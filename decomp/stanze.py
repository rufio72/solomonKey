#!/usr/bin/env python3
"""Decodificatore delle stanze di Solomon's Key dalla ROM (set solomon).

Ricostruisce il fondale (layer BG, videoram dc00 e colorram d800) di una
stanza con la stessa grammatica della routine 0x326C chiamata da 0x3231:
- F068 + 4*N: parola 1 = flusso base (decodificato con BC = 0x1000),
  parola 2 = H:L, indici (1-based, 0 = assente) di due sovrapposizioni;
- F138 + 4*(i-1): parola = flusso della sovrapposizione i, poi C, B;
- F000 + 2*N: record della palette BG (0x33BA, copiata a c9ca e poi e500).
Il primo piano (FG, blocchi e oggetti fissi) segue 0x37D7, 0x358F e 0x3829:
- 1557 + 48*N: due mappe di bit da 24 byte (12 righe x 16 celle, bit 7 per
  primo, cella 0 di ogni riga = bordo): la prima mette 0xB7 (blocco
  marrone), la seconda 0xF8 (blocco bianco) nella griglia c0cd;
- 8EA4 + 2*N: record della stanza (10 byte di testa, poi coppie
  posizione/valore fino a FF), posizioni relative a c0bd;
- ogni cella diversa da 0x37 e fuori da 0x40-0x76 diventa un blocco 2x2
  con codice e colore dalle tabelle 8578 e 858E (0x3A2E, 0x3A54).
Due valori dipendono dallo stato del gioco e non dalla ROM: c24a (oggetto
mostrato in posizione E, valore c24a+5) e c248 (se uguale a D l'oggetto
non viene messo); con --confronta si legge c24a dal dump.

Uso (da decomp/):
  python3 stanze.py --rom out/main64.bin --stanza N [--confronta ram_room_M.bin]
Scrive out/stanza_N.txt con la griglia dei codici; con --confronta stampa i
tile diversi rispetto al dump di MAME (c000-ffff) nelle righe 4-27,
colonne 1-30, la zona scritta da 0x326C; per il FG si escludono le celle
che cambiano fra il dump e out/dump_stanza_N_2300.bin (dump_stanza.lua).
"""
import argparse
import os

NUM_STANZE = 52
TAB_PALETTE = 0xf000
TAB_STANZE = 0xf068
TAB_SOVRAPPOSIZIONI = 0xf138
BG_COLORE = 0xd800
BG_VIDEO = 0xdc00
FG_COLORE = 0xd000
FG_VIDEO = 0xd400
PALETTE_RAM = 0xc9ca
MAPPE_BLOCCHI = 0x1557         # 48 byte per stanza
TAB_RECORD = 0x8ea4            # parola per stanza -> record
TAB_TILE_BASSI = 0x8578        # valori 0-7 e blocchi: (codice, colore)
TAB_TILE_OGGETTI = 0x858e      # valori 8-0x3f
GRIGLIA = 0xc0bd               # 0xe0 celle, righe da 16, la riga 0 è a c0cd
CELLA_VUOTA = 0x37
RIGHE = range(4, 28)
COLONNE = range(1, 31)


def parola(rom, a):
    return rom[a] | (rom[a + 1] << 8)


def passo(valore, indietro):
    return (valore - 1) & 0xff if indietro else (valore + 1) & 0xff


class Decodificatore:
    """Stato di 0x326C: le variabili c082-c089 e i due piani BG (1 KB ciascuno)."""

    def __init__(self, rom):
        self.rom = rom
        self.video = bytearray(0x400)
        self.colore = bytearray(0x400)
        self.scritte = set()          # celle toccate (indice riga*32+colonna)

    def blocco(self, base, attributi, riga, colonna):
        """0x336C: blocco 2x2 a riga 4+2*riga, colonna 1+2*colonna."""
        if riga > 11 or colonna > 14:
            return
        cella = 0x80 + riga * 64 + 2 * colonna + 1
        for i in range(4):
            indice = i
            if attributi & 0x08:
                indice ^= 2
            if attributi & 0x80:
                indice ^= 1
            pos = (cella + (32 if i >= 2 else 0) + (i & 1)) & 0x3ff
            self.video[pos] = (base + indice) & 0xff
            self.colore[pos] = attributi
            self.scritte.add(pos)

    def flusso(self, p, b, c):
        """Decodifica un flusso a partire dall'indirizzo p con il parametro BC."""
        larghezza = b & 0x1f
        riga_b = (((c >> 4) << 1) | (b >> 7)) & 0xff
        a_destra = (b >> 6) & 1
        col = ((c & 0x0f) + larghezza - 2) if a_destra else (c & 0x0f)
        col_b = ((col << 1) | a_destra) & 0xff
        inizio_b, contatore, ricarica_riga = col_b, larghezza, larghezza
        attr_c087, ribalta_una, rip = 0, 0, 0
        pila = []
        while True:
            c = self.rom[p]
            p += 1
            tipo = c & 3
            if tipo == 1:                         # 333C / 23CB: attributi
                if c & 0x80:
                    attr_c087 = (attr_c087 & 0x88) | (c & 0x70) | ((c & 0x0c) >> 2)
                else:
                    attr_c087 = (attr_c087 & 0x8b) | (c & 0x70)
                    ribalta_una = (0x80 if c & 4 else 0) | (0x08 if c & 8 else 0)
                continue
            if tipo == 2:                         # 335A: inizio ripetizione
                c &= 0xfc
                rip = c | ((c & 0x0c) >> 2)
                pila.append(p)
                continue
            if tipo == 3:                         # 336A: fine
                return p
            base = c & 0xfc                       # 3297: blocco
            su = riga_b & 1
            riga = riga_b >> 1
            attributi = 0x08 if su else 0
            sinistra = col_b & 1
            colonna = col_b >> 1
            if sinistra:
                attributi |= 0x80
            col_b = ((passo(colonna, sinistra) << 1) | sinistra) & 0xff
            contatore = (contatore - 1) & 0xff
            if contatore == 0:                    # a capo
                riga = passo(riga, su)
                riga_b = ((riga << 1) | su) & 0xff
                verso = inizio_b & 1
                colonna = (riga & 0x80) | (inizio_b >> 1)
                col_b = ((passo(colonna, verso) << 1) | verso) & 0xff
                contatore = (ricarica_riga - 1) & 0xff
            scambiati = ((attr_c087 >> 4) | (attr_c087 << 4)) & 0xff
            attributi ^= scambiati ^ ribalta_una
            ribalta_una = 0
            self.blocco(base, attributi, riga, colonna)
            if (rip & 0x0c) == 0 and (rip & 0xf0) == 0:
                continue
            ricarica = rip & 0x0c
            ricarica |= ricarica >> 2
            if rip & 3:
                rip -= 1
                continue
            giri = ((rip & 0xf0) - 0x10) & 0xff
            if giri == 0:
                rip = 0
                if pila:
                    pila.pop()
                continue
            if pila:
                p = pila[-1]
            rip = giri | ricarica


def fondale(rom, n):
    """Come 0x3231: flusso base più le sovrapposizioni facoltative."""
    d = Decodificatore(rom)
    voce = TAB_STANZE + 4 * n
    d.flusso(parola(rom, voce), 0x10, 0x00)
    sovr = parola(rom, voce + 2)
    for indice in (sovr >> 8, sovr & 0xff):
        if indice:
            s = TAB_SOVRAPPOSIZIONI + 4 * (indice - 1)
            d.flusso(parola(rom, s), rom[s + 3], rom[s + 2])
    return d


def palette(rom, n, c=0):
    """0x33BA: record a F000[N], testa h e 2*(h&0x7f) byte copiati a c9ca
    (c9ca-ca49 azzerati prima da 0x33AA). Il riporto di `rrc c` (bit 0 di C)
    decide un passo successivo che rielabora i nibble con rld: la chiamata
    da 3258 ha C = 1, ma nei dump al frame 2000 c9ca contiene la copia
    semplice (C = 0, chiamate successive da 7906/7CD8), che è il predefinito."""
    p = parola(rom, TAB_PALETTE + 2 * n)
    h = rom[p]
    a = (h << 1) & 0xff
    quanti = a
    a = ((c & 1) << 7) | (a >> 1)             # rra con il riporto di rrc c
    buf = bytearray(256)
    buf[:quanti] = rom[p + 1:p + 1 + quanti]
    if not a & 0x80:
        return bytes(buf[:0x80])
    giri = (a << 1) & 0xff or 256             # djnz: B = 0 vale 256
    a = 0
    for hl in range(giri):
        for passo_rld in range(3):
            t = buf[hl]
            buf[hl] = ((t << 4) | (a & 0x0f)) & 0xff
            a = (a & 0xf0) | (t >> 4)
            if passo_rld < 2:
                a >>= 1
    return bytes(buf[:0x80])


def griglia_fg(rom, n, c24a=2, c248=None, prima_volta=True):
    """Griglia c0bd-c19c dopo 0x37D7 (mappe di bit) e 0x358F (record).
    Restituisce (griglia, testa) con griglia indicizzata da c0bd."""
    g = bytearray(0xe0)
    g[0x00:0x10] = b'\xff' * 16                # 0x380A su c0bd e c18d
    g[0xd0:0xe0] = b'\xff' * 16
    p = MAPPE_BLOCCHI + 48 * n
    for valore, mappa in ((0xb7, p), (0xf8, p + 24)):   # 0x3812
        for i in range(24 * 8):
            if rom[mappa + i // 8] & (0x80 >> (i % 8)):
                g[0x10 + i] = valore
    for i in range(0xe0):                       # 37FA: celle vuote = 0x37
        if g[i] == 0:
            g[i] = CELLA_VUOTA
    r = parola(rom, TAB_RECORD + 2 * n)
    testa = rom[r:r + 10]
    g[testa[0]] = 0x01                          # 359C: porta
    g[testa[1]] = 0x04                          # 35A7, 35BD: specchi di partenza
    g[testa[2]] = 0x04
    if c248 != testa[3]:                        # 35D2: oggetto in posizione E
        g[testa[4]] = (g[testa[4]] & 0x80) + c24a + 5
    q = r + 10
    while rom[q] != 0xff:                       # 3642: coppie posizione, valore
        pos, h = rom[q], rom[q + 1]
        q += 2
        if (h & 0x3f) >= 0x32 and not prima_volta:
            h = (h & 0xc0) + 0x29
        if pos < len(g):
            g[pos] = h
    return g, testa


def tile_fg(rom, v):
    """0x3A2E: valore di cella -> (codice, colore)."""
    if v < 0x08:
        a = TAB_TILE_BASSI + 2 * v
    elif v < 0x40:
        a = TAB_TILE_OGGETTI + 2 * (v - 8)
    elif v < 0xc0:
        a = TAB_TILE_BASSI + 2 * 8
    elif v < 0xf8:
        a = TAB_TILE_BASSI + 2 * 9
    else:
        a = TAB_TILE_BASSI + 2 * 10
    return rom[a], rom[a + 1]


def primo_piano(rom, n, c24a=2):
    """0x3829: disegna i blocchi della griglia nei piani FG (d400 e d000)."""
    g, _ = griglia_fg(rom, n, c24a)
    video = bytearray(0x400)
    colore = bytearray(0x400)
    for r in range(12):
        for k in range(1, 16):
            v = g[0x10 + 16 * r + k]
            if v == CELLA_VUOTA or 0x40 <= v < 0x77:
                continue
            codice, attr = tile_fg(rom, v)
            pos = (4 + 2 * r) * 32 + 1 + 2 * (k - 1)    # 0x3A54
            for i, d in enumerate((0, 1, 32, 33)):
                video[pos + d] = (codice + i) & 0xff
                colore[pos + d] = attr
    return video, colore, g


def confronta(nome, ricostruito, dump, base, celle):
    diversi = sum(1 for i in celle if ricostruito[i] != dump[base - 0xc000 + i])
    print('%s: %d tile diversi' % (nome, diversi))
    return diversi


def main():
    ap = argparse.ArgumentParser(description='Decodificatore delle stanze dalla ROM')
    ap.add_argument('--rom', default='out/main64.bin')
    ap.add_argument('--stanza', type=int, required=True, help='indice 0-based (c243)')
    ap.add_argument('--confronta', help='dump c000-ffff di MAME (ram_room_M.bin)')
    ap.add_argument('--c24a', type=int, help='stato c24a (predefinito: dal dump, o 2)')
    ap.add_argument('--escludi', help='dump della stessa stanza a un altro frame '
                    '(predefinito out/dump_stanza_N_2300.bin se esiste): le celle '
                    'FG che cambiano fra i due dump sono animate e non si confrontano')
    arg = ap.parse_args()
    if not 0 <= arg.stanza < NUM_STANZE:
        ap.error('stanza fra 0 e %d' % (NUM_STANZE - 1))
    with open(arg.rom, 'rb') as f:
        rom = f.read()
    dump = None
    if arg.confronta:
        with open(arg.confronta, 'rb') as f:
            dump = f.read()
    c24a = arg.c24a if arg.c24a is not None else (dump[0x24a] if dump else 2)

    d = fondale(rom, arg.stanza)
    pal = palette(rom, arg.stanza)
    fg_video, fg_colore, griglia = primo_piano(rom, arg.stanza, c24a)
    celle = [r * 32 + c for r in RIGHE for c in COLONNE]
    with open('out/stanza_%d.txt' % arg.stanza, 'w') as f:
        f.write('# stanza %d (c243=%d), righe 4-27 colonne 1-30\n' % (arg.stanza, arg.stanza))
        for nome, piano in (('BG videoram', d.video), ('BG colorram', d.colore),
                            ('FG videoram', fg_video), ('FG colorram', fg_colore)):
            f.write('# %s\n' % nome)
            for r in RIGHE:
                f.write(' '.join('%02X' % piano[r * 32 + c] for c in COLONNE) + '\n')
        f.write('# griglia c0cd (12 righe x 16 celle, c24a=%d)\n' % c24a)
        for r in range(12):
            f.write(' '.join('%02X' % griglia[0x10 + 16 * r + k] for k in range(16)) + '\n')
        f.write('# palette c9ca (128 byte)\n')
        for i in range(0, 0x80, 32):
            f.write(' '.join('%02X' % x for x in pal[i:i + 32]) + '\n')
    print('scritta out/stanza_%d.txt (%d celle BG scritte su %d)'
          % (arg.stanza, len(d.scritte & set(celle)), len(celle)))
    if dump is None:
        return
    confronta('BG videoram', d.video, dump, BG_VIDEO, celle)
    confronta('BG colorram', d.colore, dump, BG_COLORE, celle)
    pal_diversi = sum(1 for i in range(0x80) if pal[i] != dump[PALETTE_RAM - 0xc000 + i])
    print('palette c9ca: %d byte diversi' % pal_diversi)
    # riferimento a 2300 e non a 1700: i blocchi FG compaiono fra i frame
    # 1850 e 1900, quindi al 1700 il primo piano è ancora vuoto (misurato)
    escludi = arg.escludi or 'out/dump_stanza_%d_2300.bin' % arg.stanza
    celle_fg = celle
    if os.path.exists(escludi):
        with open(escludi, 'rb') as f:
            prima = f.read()
        animate = set()
        for base in (FG_VIDEO, FG_COLORE):
            for i in celle:
                if prima[base - 0xc000 + i] != dump[base - 0xc000 + i]:
                    animate.add(i)
        celle_fg = [i for i in celle if i not in animate]
        print('FG: %d celle escluse (cambiano fra %s e il dump)' % (len(animate), escludi))
    else:
        print('FG: nessuna cella esclusa (%s assente)' % escludi)
    confronta('FG videoram', fg_video, dump, FG_VIDEO, celle_fg)
    confronta('FG colorram', fg_colore, dump, FG_COLORE, celle_fg)


if __name__ == '__main__':
    main()
