"""Verifica dei fogli a colori della stanza 1 contro snap_room_1.png.

Uso (dalla cartella roms/decoded, dopo colora.py ... 1):
    python3 verifica_colori.py

Ricostruisce lo schermo della stanza 1 dai soli fogli a banco fisso
(us_<set>_stanza1_bK.png, ridotti a scala 1) e dagli attributi letti dal dump
mame/rooms/ram_room_1.bin: BG tiles opaco, poi FG chars con pen 0
trasparente, poi gli sprite. Confronta il ritaglio (0,16,256,240) con lo
screenshot. Esce con 1 se la cella del blocco marrone (riga 10, colonna 14)
differisce o se i pixel uguali con gli sprite sono meno di 54326.
"""
import os
import sys
from PIL import Image

QUI = os.path.dirname(os.path.abspath(__file__))
RITAGLIO = (0, 16, 256, 240)
MINIMO_SPRITE = 54326


def carica_banchi(nome):
    """Gli 8 fogli a banco fisso di un set, ridotti a scala 1."""
    fogli = []
    for k in range(8):
        im = Image.open(os.path.join(QUI, f'us_{nome}_stanza1_b{k}.png')).convert('RGBA')
        fogli.append(im.resize((im.width//2, im.height//2), Image.NEAREST))
    return fogli


def ritaglia_tile(fogli, banco, code, lato, colonne):
    ox = (code % colonne)*lato; oy = (code//colonne)*lato
    return fogli[banco].crop((ox, oy, ox+lato, oy+lato))


def disegna_piano(schermo, fogli, ram, colore, video):
    """Piano 32x32 di tile 8x8; bit 7 dell'attributo ribalta le righe."""
    for i in range(1024):
        a = ram[colore+i]
        code = ram[video+i] + 256*(a & 7)
        t = ritaglia_tile(fogli, (a >> 4) & 7, code, 8, 64)
        if a & 0x80:
            t = t.transpose(Image.FLIP_TOP_BOTTOM)
        schermo.alpha_composite(t, ((i % 32)*8, (i//32)*8))


def disegna_sprite(schermo, fogli, ram):
    """Le 32 voci di spriteram a e000 (b0 b1 b2 b3)."""
    for i in range(32):
        b0, b1, b2, b3 = ram[0x2000+4*i:0x2000+4*i+4]
        code = b0 + 16*(b1 & 0x10)
        t = ritaglia_tile(fogli, (b1 & 0x0e) >> 1, code, 16, 32)
        if b1 & 0x40:
            t = t.transpose(Image.FLIP_LEFT_RIGHT)
        if b1 & 0x80:
            t = t.transpose(Image.FLIP_TOP_BOTTOM)
        # paste con maschera ritaglia ai bordi (alfa solo 0 o 255)
        schermo.paste(t, (b3, 241-b2), t)


def uguali(a, b):
    return sum(1 for p, q in zip(a.getdata(), b.getdata()) if p == q)


def main():
    with open(os.path.join(QUI, 'mame', 'rooms', 'ram_room_1.bin'), 'rb') as f:
        ram = f.read()
    snap = Image.open(os.path.join(QUI, 'mame', 'rooms', 'snap_room_1.png')).convert('RGB')
    schermo = Image.new('RGBA', (256, 256 + 16), (0, 0, 0, 255))
    disegna_piano(schermo, carica_banchi('tiles'), ram, 0x1800, 0x1c00)
    disegna_piano(schermo, carica_banchi('chars'), ram, 0x1000, 0x1400)
    bgfg = schermo.crop(RITAGLIO).convert('RGB')

    # cella del blocco marrone: riga 10, colonna 14 (y schermo = 80 - 16)
    cella = (14*8, 10*8, 14*8+8, 10*8+8)
    snap_cella = snap.crop((cella[0], cella[1]-16, cella[2], cella[3]-16))
    diversi_cella = 64 - uguali(schermo.crop(cella).convert('RGB'), snap_cella)

    disegna_sprite(schermo, carica_banchi('sprites'), ram)
    tutto = schermo.crop(RITAGLIO).convert('RGB')

    n_bgfg = uguali(bgfg, snap)
    n_tutto = uguali(tutto, snap)
    totale = snap.width*snap.height
    print(f'cella blocco r10 c14 pixel diversi: {diversi_cella}')
    print(f'BG+FG pixel uguali: {n_bgfg} / {totale}')
    print(f'BG+FG+sprite pixel uguali: {n_tutto} / {totale}')
    if diversi_cella > 0 or n_tutto < MINIMO_SPRITE:
        sys.exit(1)


if __name__ == '__main__':
    main()
