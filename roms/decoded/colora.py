"""Fogli di chars, tiles e sprite a colori con la palette di RAM di una stanza.

Uso: python3 colora.py <solomon.zip o cartella che lo contiene> <cartella_uscita> [stanza]

Legge le ROM da solomon.zip con zipfile (non le copia) e la palette da
mame/rooms/ram_room_<stanza>.bin (RAM c000-ffff, relativa alla cartella dello
script). Produce, per ogni set (chars, tiles, sprites), un foglio col banco di
riferimento di ogni tile nella stanza e otto fogli a banco fisso 0..7.

Duplicato: rd_zip, bit e decode sono copie di rd, bit e decode di decode.py
(che non è importabile perché legge sys.argv a livello di modulo); vanno tenute
identiche. decode.py e i suoi fogli in grigi non si toccano.
"""
import os
import sys
import zipfile
from collections import Counter
from PIL import Image

QUI = os.path.dirname(os.path.abspath(__file__))


# --- copie da decode.py (vedi commento di testa) ---
def rd_zip(z, nome):
    return z.read(nome)

def bit(data, off):
    return (data[off>>3] >> (7-(off&7))) & 1

def decode(data, w, h, planes, xoff, yoff, charsize, count):
    tiles=[]
    for n in range(count):
        base=n*charsize
        t=[[0]*w for _ in range(h)]
        for y in range(h):
            for x in range(w):
                v=0
                for p,po in enumerate(planes):
                    v |= bit(data, base+po+xoff[x]+yoff[y]) << p
                t[y][x]=v
        tiles.append(t)
    return tiles
# --- fine copie ---

# decode() dà il nibble con i piani invertiti: il pen vero è il nibble rovesciato
ROVESCIA = [int(f'{v:04b}'[::-1], 2) for v in range(16)]


def tile8(z, a, b):
    """Tile 8x8 a 4 piani, stesso layout di tile8 in decode.py; valori già pen."""
    data = rd_zip(z, a) + rd_zip(z, b)
    planes=[0,1,2,3]; xo=[i*4 for i in range(8)]; yo=[i*32 for i in range(8)]
    t = decode(data, 8, 8, planes, xo, yo, 32*8, len(data)//32)
    return [[[ROVESCIA[v] for v in r] for r in tt] for tt in t]


def spr16(z, nomi):
    """Sprite 16x16, stesso layout di spr16 in decode.py; valori già pen."""
    data = b''.join(rd_zip(z, f) for f in nomi); q = len(data)//4*8
    planes=[0,q,2*q,3*q]; xo=list(range(8))+[64+i for i in range(8)]
    yo=[i*8 for i in range(8)]+[128+i*8 for i in range(8)]
    t = decode(data, 16, 16, planes, xo, yo, 32*8, len(data)//4//32)
    return [[[ROVESCIA[v] for v in r] for r in tt] for tt in t]


def leggi_palette(ram):
    """256 voci (R,G,B) da e400; parola little endian xxxxBBBBGGGGRRRR."""
    pal = []
    for i in range(256):
        w = ram[0x2400+2*i] | (ram[0x2400+2*i+1] << 8)
        pal.append(((w & 15)*17, ((w >> 4) & 15)*17, ((w >> 8) & 15)*17))
    return pal


def _piano(ram, colore, video):
    """1024 coppie (code, banco, flip_righe) di un piano 32x32."""
    celle = []
    for i in range(1024):
        a = ram[colore+i]
        celle.append((ram[video+i] + 256*(a & 7), (a >> 4) & 7, bool(a & 0x80)))
    return celle


def leggi_piani(ram):
    """Piani FG (d000/d400), BG (d800/dc00) e le 32 voci sprite da e000.

    Sprite: dizionari con code, banco, fx, fy, x, y (y già in coordinate
    dello schermo intero, prima del ritaglio delle 16 righe in alto)."""
    fg = _piano(ram, 0x1000, 0x1400)
    bg = _piano(ram, 0x1800, 0x1c00)
    spr = []
    for i in range(32):
        b0, b1, b2, b3 = ram[0x2000+4*i:0x2000+4*i+4]
        spr.append(dict(code=b0 + 16*(b1 & 0x10), banco=(b1 & 0x0e) >> 1,
                        fx=bool(b1 & 0x40), fy=bool(b1 & 0x80),
                        x=b3, y=241-b2))
    return fg, bg, spr


def banco_riferimento(usi, conteggio):
    """Per ogni codice il banco più frequente in usi (coppie code, banco);
    a parità il più basso; 0 se il codice non è mai usato."""
    cont = {}
    for code, banco in usi:
        cont.setdefault(code, Counter())[banco] += 1
    out = [0]*conteggio
    for code, c in cont.items():
        if 0 <= code < conteggio:
            out[code] = min(c, key=lambda b: (-c[b], b))
    return out


def foglio_rgba(tiles, w, h, colonne, palette, base, banchi, trasparente0,
                percorso, scala=2):
    """Salva un PNG RGBA; colore = palette[base + 16*banco + pen]."""
    righe = (len(tiles)+colonne-1)//colonne
    img = Image.new('RGBA', (colonne*w, righe*h))
    px = img.load()
    for n, t in enumerate(tiles):
        ox = (n % colonne)*w; oy = (n//colonne)*h
        voce0 = base + 16*banchi[n]
        for y in range(h):
            for x in range(w):
                pen = t[y][x]
                alfa = 0 if (trasparente0 and pen == 0) else 255
                px[ox+x, oy+y] = palette[voce0+pen] + (alfa,)
    img = img.resize((img.width*scala, img.height*scala), Image.NEAREST)
    img.save(percorso)
    print(percorso, len(tiles), "tile", img.size)


def main():
    if len(sys.argv) < 3:
        sys.exit("uso: python3 colora.py <solomon.zip> <cartella_uscita> [stanza]")
    rom, out = sys.argv[1], sys.argv[2]
    stanza = int(sys.argv[3]) if len(sys.argv) > 3 else 1
    if os.path.isdir(rom):
        rom = os.path.join(rom, 'solomon.zip')
    with open(os.path.join(QUI, 'mame', 'rooms', f'ram_room_{stanza}.bin'), 'rb') as f:
        ram = f.read()
    pal = leggi_palette(ram)
    fg, bg, spr = leggi_piani(ram)
    with zipfile.ZipFile(rom) as z:
        chars = tile8(z, '12.3t', '11.3r')
        tiles = tile8(z, '10.3p', '9.3m')
        sprites = spr16(z, ['2.5lm', '3.6lm', '4.7lm', '5.8lm'])
    insiemi = [
        ('chars', chars, 8, 64, 0, True, [(c, b) for c, b, _ in fg]),
        ('tiles', tiles, 8, 64, 128, False, [(c, b) for c, b, _ in bg]),
        ('sprites', sprites, 16, 32, 0, True, [(s['code'], s['banco']) for s in spr]),
    ]
    for nome, ts, lato, col, base, tr, usi in insiemi:
        rif = banco_riferimento(usi, len(ts))
        pref = os.path.join(out, f'us_{nome}_stanza{stanza}')
        foglio_rgba(ts, lato, lato, col, pal, base, rif, tr, pref + '.png')
        for k in range(8):
            foglio_rgba(ts, lato, lato, col, pal, base, [k]*len(ts), tr,
                        f'{pref}_b{k}.png')


if __name__ == '__main__':
    main()
