"""Mappe delle stanze con gli oggetti nascosti («special»), compito 60.

Per ogni stanza (c243 = 0-51) rende fondale e blocchi come in gioco, disegna
gli oggetti nascosti (celle 0x40-0x76, classe = v & 0x3f, rivelati da 4938
con la tabella di 3A2E = stanze.tile_fg) con contorno verde e la cella E
della testa del record con contorno arancio; legenda in inglese sotto
l'immagine (le immagini vanno nelle pagine pubbliche del repo).

Uso (da decomp/):
  python3 mappe_special.py [--rom out/main64.bin] [--rom-grafica ../roms/ext/solomon]
      [--dump-dir out] [--uscita ../docs/special] [--scala 3] [--nei-blocchi]
      [--stanza N] [--confronta-mame] [--rendi-dump FILE.bin --ppm FILE.ppm]
Legge soltanto da out/ (palette dai dump di MAME al frame 2300).
"""
import argparse
import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

import render_py
import stanze

VERDE, ARANCIO, AZZURRO = (0, 255, 0), (255, 128, 0), (0, 200, 255)
GRIGIO = (40, 40, 40)
TABULAZIONI = (6, 80, 110, 300, 410)    # x dei campi della legenda
TAB_C24A = 0x8dd4                 # byte 0 del record di 4 byte: c24a iniziale

# classe -> (nome inglese descrittivo dal tile, punti; None = nessun punteggio
# dalla tabella 150C). Tabella completa in DATI.md, «Oggetti nascosti».
OGGETTI = {
    0x08: ('blue diamond', 500), 0x09: ('small blue jar', 200),
    0x0A: ('yellow coin', 2000), 0x0B: ('red orb', 200), 0x0C: ('gem', 2000),
    0x0D: ('blue jar', 500), 0x0E: ('scroll', None), 0x0F: ('bell', 200),
    0x10: ('green potion A', 100), 0x11: ('green potion B', 500),
    0x12: ('red hourglass', 200), 0x13: ('blue hourglass', 500),
    0x14: ('green orb', 100), 0x15: ('blue jar A', 200), 0x16: ('blue jar B', 500),
    0x17: ('scroll', 200), 0x18: ('orange bell', 200), 0x19: ('yellow bell', 500),
    0x1A: ('red jar (fire bottle)', 500), 0x1B: ('red jar', 200),
    0x29: ('gold bar', 100), 0x2A: ('yellow diamond', 200), 0x2B: ('sapphire', 500),
    0x2C: ('gold coin', 1000), 0x2D: ('coins', 2000), 0x2E: ('ruby', 5000),
    0x2F: ('star coin A', 10000), 0x30: ('star coin B', 20000),
    0x31: ('green gem', 50000), 0x32: ('yellow ornament', 100000),
    0x33: ('seal', 200000), 0x34: ('winged ornament', 500000),
    0x35: ('tiger', 1000000), 0x36: ('medal', None),
}


def nome(classe):
    return OGGETTI.get(classe, ('class %02X' % classe, None))[0]


def punti(classe):
    p = OGGETTI.get(classe, (None, None))[1]
    return '%d pts' % p if p is not None else 'special'


def celle(griglia, condizione):
    """(r, k, valore) delle celle della griglia 12x15 che soddisfano condizione."""
    return [(r, k, griglia[0x10 + 16 * r + k]) for r in range(12) for k in range(1, 16)
            if condizione(griglia[0x10 + 16 * r + k])]


def nascosta(v):
    return 0x40 <= v <= 0x76


def nel_blocco(v):
    return v >= 0x80 and 0x08 <= (v & 0x3f) <= 0x36


def metti_blocco(video, colore, rom, r, k, classe):
    """Blocco 2x2 come 3A54: codici c, c+1 / c+2, c+3, un solo colore."""
    codice, attr = stanze.tile_fg(rom, classe)
    pos = (4 + 2 * r) * 32 + 1 + 2 * (k - 1)
    for i, d in enumerate((0, 1, 32, 33)):
        video[pos + d] = (codice + i) & 0xff
        colore[pos + d] = attr


def leggi_dump(cartella, n):
    percorso = os.path.join(cartella, 'dump_stanza_%d_2300.bin' % n)
    if not os.path.exists(percorso):
        sys.exit('ERRORE: manca %s; rigenerarlo da decomp/ con\n  ROOM=%d DUMP_FRAME=2000,2300 '
                 'mame solomon ... -video none -autoboot_script dump_stanza.lua'
                 % (percorso, n))
    with open(percorso, 'rb') as f:
        return f.read()


def rendi_stanza(rom, n, grafica, palette, nei_blocchi):
    """Immagine 192x240 della zona di gioco (righe di tile 4-27, colonne 1-30)."""
    c24a = rom[TAB_C24A + 4 * n]
    griglia, testa = stanze.griglia_fg(rom, n, c24a)
    bg = stanze.fondale(rom, n)
    fg_video, fg_colore, _ = stanze.primo_piano(rom, n, c24a)
    fg_video, fg_colore = bytearray(fg_video), bytearray(fg_colore)
    for r, k, v in celle(griglia, nascosta):
        metti_blocco(fg_video, fg_colore, rom, r, k, v & 0x3f)
    rgb = render_py.rendi_frame(fg_colore, fg_video, bytes(bg.colore), bytes(bg.video),
                                bytes(128), palette, grafica)[16:208, 8:248].copy()
    if nei_blocchi:   # icona dell'oggetto sopra il blocco (pen 0 trasparente)
        vuoto = bytes(0x400)
        sv, sc = bytearray(0x400), bytearray(0x400)
        for r, k, v in celle(griglia, nel_blocco):
            metti_blocco(sv, sc, rom, r, k, v & 0x3f)
        icone = render_py.rendi_frame(sc, sv, vuoto, vuoto, bytes(128), palette,
                                      grafica)[16:208, 8:248]
        sola = render_py.rendi_frame(vuoto, vuoto, vuoto, vuoto, bytes(128), palette,
                                     grafica)[16:208, 8:248]
        diversi = np.any(icone != sola, axis=2)
        rgb[diversi] = icone[diversi]
    return rgb, griglia, testa, c24a


def voce(tipo, v, r, k):
    """Riga di legenda a campi separati da tabulazioni (font proporzionale)."""
    c = v & 0x3f
    return '%s\t%02X\t%s\trow %d col %d\t%s' % (tipo, c, nome(c), r, k, punti(c))


def contorno(disegno, r, k, scala, colore):
    x0, y0 = 16 * (k - 1) * scala, 16 * r * scala
    lato = 16 * scala
    for i in range(scala):
        disegno.rectangle([x0 + i, y0 + i, x0 + lato - 1 - i, y0 + lato - 1 - i],
                          outline=colore)


def font():
    try:
        return ImageFont.load_default(size=14)
    except TypeError:                     # Pillow senza FreeType o vecchio
        return ImageFont.load_default()


def immagine(rgb, griglia, testa, c24a, n, scala, nei_blocchi):
    alto, largo = rgb.shape[0] * scala, rgb.shape[1] * scala
    mappa = Image.fromarray(rgb, 'RGB').resize((largo, alto), Image.NEAREST)
    d = ImageDraw.Draw(mappa)
    righe = [('Room %d (c243=%d)' % (n, n), (255, 255, 255))]
    nasc = celle(griglia, nascosta)
    for r, k, v in nasc:
        contorno(d, r, k, scala, VERDE)
        righe.append((voce('hidden', v, r, k), VERDE))
    if not nasc:
        righe.append(('no hidden objects', VERDE))
    e = testa[4]
    if 0x10 <= e < 0xd0 and 1 <= (e & 15) <= 15:
        r, k = (e - 0x10) // 16, e & 15
        contorno(d, r, k, scala, ARANCIO)
        righe.append(('cell E: class c24a+5 = %02X (05/06/07 with c24a=0/1/2), row %d col %d'
                      % (c24a + 5, r, k), ARANCIO))
    if nei_blocchi:
        for r, k, v in celle(griglia, nel_blocco):
            contorno(d, r, k, scala, AZZURRO)
            righe.append((voce('in block', v, r, k), AZZURRO))
    righe.append(('time step (record bytes 5-8): %02X,%02X,%02X,%02X, chosen with (e605>>2)&3'
                  % tuple(testa[5:9]),
                  (200, 200, 200)))
    f = font()
    passo = 18
    finale = Image.new('RGB', (largo, alto + 8 + passo * len(righe)), GRIGIO)
    finale.paste(mappa, (0, 0))
    dl = ImageDraw.Draw(finale)
    for i, (testo, colore) in enumerate(righe):
        for x, campo in zip(TABULAZIONI, testo.split('\t')):
            dl.text((x, alto + 4 + passo * i), campo, fill=colore, font=f)
    return finale


def confronta_mame(rom, cartella):
    """Per ogni classe nascosta: tile e colore di una cella visibile nel dump."""
    nascoste, viste = set(), {}
    for n in range(stanze.NUM_STANZE):
        g, _ = stanze.griglia_fg(rom, n, rom[TAB_C24A + 4 * n])
        nascoste |= {v & 0x3f for _, _, v in celle(g, nascosta)}
        for r, k, v in celle(g, lambda v: 0x08 <= v <= 0x36):
            viste.setdefault(v, []).append((n, r, k))
    for classe in sorted(nascoste):
        if classe not in viste:
            print('classe 0x%02X: non visibile in chiaro in nessuna stanza' % classe)
            continue
        codice, attr = stanze.tile_fg(rom, classe)
        esito = None
        for n, r, k in viste[classe]:
            pos = (4 + 2 * r) * 32 + 1 + 2 * (k - 1)
            for frame in (2300, 2000):
                p = os.path.join(cartella, 'dump_stanza_%d_%d.bin' % (n, frame))
                with open(p, 'rb') as f:
                    dump = f.read()
                ok = all(dump[0x1400 + pos + dd] == (codice + i) & 0xff and
                         dump[0x1000 + pos + dd] == attr
                         for i, dd in enumerate((0, 1, 32, 33)))
                if ok:
                    esito = (n, frame)
                    break
            if esito:
                break
        if esito is None:
            print('classe 0x%02X: DIVERSA (stanze %s)'
                  % (classe, ','.join(str(n) for n, _, _ in viste[classe])))
        elif esito[1] == 2300:
            print('classe 0x%02X: MAME ok (stanza %d)' % (classe, esito[0]))
        else:
            print('classe 0x%02X: MAME ok (stanza %d, dump al frame 2000: animato)'
                  % (classe, esito[0]))


def main():
    ap = argparse.ArgumentParser(description='Mappe delle stanze con gli oggetti nascosti')
    ap.add_argument('--rom', default='out/main64.bin')
    ap.add_argument('--rom-grafica', default='../roms/ext/solomon')
    ap.add_argument('--dump-dir', default='out')
    ap.add_argument('--uscita', default='../docs/special')
    ap.add_argument('--scala', type=int, default=3)
    ap.add_argument('--nei-blocchi', action='store_true',
                    help='anche gli oggetti dentro i blocchi, in USCITA/nei_blocchi/')
    ap.add_argument('--stanza', type=int, help='solo la stanza N (c243, 0-51)')
    ap.add_argument('--confronta-mame', action='store_true')
    ap.add_argument('--rendi-dump', help='dump c000-ffff da rendere intero (con sprite)')
    ap.add_argument('--ppm', help='uscita PPM P6 di --rendi-dump')
    arg = ap.parse_args()
    with open(arg.rom, 'rb') as f:
        rom = f.read()
    if arg.confronta_mame:
        confronta_mame(rom, arg.dump_dir)
        return
    grafica = render_py.carica_grafica(arg.rom_grafica)
    if arg.rendi_dump:
        if not arg.ppm:
            ap.error('--rendi-dump vuole --ppm')
        with open(arg.rendi_dump, 'rb') as f:
            ram = render_py.ram_da_dump(f.read())
        render_py.scrivi_ppm(arg.ppm, render_py.rendi_frame(*ram, grafica))
        return
    if arg.stanza is not None and not 0 <= arg.stanza < stanze.NUM_STANZE:
        ap.error('stanza fra 0 e %d' % (stanze.NUM_STANZE - 1))
    elenco = [arg.stanza] if arg.stanza is not None else range(stanze.NUM_STANZE)
    cartella = os.path.join(arg.uscita, 'nei_blocchi') if arg.nei_blocchi else arg.uscita
    os.makedirs(cartella, exist_ok=True)
    totale, scritte, conteggio = 0, 0, {}
    for n in elenco:
        palette = render_py.ram_da_dump(leggi_dump(arg.dump_dir, n))[5]
        rgb, griglia, testa, c24a = rendi_stanza(rom, n, grafica, palette, arg.nei_blocchi)
        classi = [v & 0x3f for _, _, v in celle(griglia, nascosta)]
        for c in classi:
            conteggio[c] = conteggio.get(c, 0) + 1
        totale += len(classi)
        immagine(rgb, griglia, testa, c24a, n, arg.scala, arg.nei_blocchi).save(
            os.path.join(cartella, 'stanza_%02d.png' % n))
        scritte += 1
        print('stanza %02d: nascosti %d classi %s'
              % (n, len(classi), ','.join('0x%02X' % c for c in classi) or '-'))
    print('classe nome conteggio')
    for c in sorted(conteggio):
        print('0x%02X %s %d' % (c, nome(c), conteggio[c]))
    print('TOTALE nascosti %d' % totale)
    print('PNG scritte %d' % scritte)


if __name__ == '__main__':
    main()
