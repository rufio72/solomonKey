#!/usr/bin/env python3
"""Estrae le ROM grafiche Tecmo in sorgente C (lotto H2).

Uso (da decomp/): python3 -I estrai_grafica.py
Legge da ../roms/ext/solomon/ (o relativo allo script) le otto ROM
grafiche e scrive in ../tecmo/ dodici file video_dati_*.c da 16384 byte:
  video_dati_car_{a,b,c,d}.c  caratteri FG  (12.3t, 11.3r)
  video_dati_fon_{a,b,c,d}.c  fondale BG    (10.3p, 9.3m)
  video_dati_spr_{a,b,c,d}.c  sprite        (2.5lm 3.6lm 4.7lm 5.8lm)
Prima di scrivere rilegge i valori dal testo prodotto e li confronta
con la ROM; se differiscono esce con 2. Un file si riscrive solo se il
contenuto cambia, quindi rieseguirlo non cambia nulla.
"""
import os
import re
import sys

QUI = os.path.dirname(os.path.abspath(__file__))
TECMO = os.path.join(QUI, '..', 'tecmo')
CARTELLE = ['../roms/ext/solomon', '../../roms/ext/solomon',
            os.path.join(QUI, '../roms/ext/solomon')]
PEZZO = 0x4000
PER_RIGA = 24

# (file ROM, dimensione attesa, [(array, suffisso del file C)])
ROM = [
    ('12.3t', 0x8000, [('vid_car_a', 'car_a'), ('vid_car_b', 'car_b')]),
    ('11.3r', 0x8000, [('vid_car_c', 'car_c'), ('vid_car_d', 'car_d')]),
    ('10.3p', 0x8000, [('vid_fon_a', 'fon_a'), ('vid_fon_b', 'fon_b')]),
    ('9.3m', 0x8000, [('vid_fon_c', 'fon_c'), ('vid_fon_d', 'fon_d')]),
    ('2.5lm', 0x4000, [('vid_spr_a', 'spr_a')]),
    ('3.6lm', 0x4000, [('vid_spr_b', 'spr_b')]),
    ('4.7lm', 0x4000, [('vid_spr_c', 'spr_c')]),
    ('5.8lm', 0x4000, [('vid_spr_d', 'spr_d')]),
]


def leggi_rom(nome, lung):
    for c in CARTELLE:
        p = os.path.join(c, nome)
        if os.path.exists(p):
            with open(p, 'rb') as f:
                dati = f.read()
            if len(dati) != lung:
                sys.exit('%s: lunghezza sbagliata (attesi %d byte)' % (p, lung))
            return dati
    sys.exit('%s non trovato in ../roms/ext/solomon/' % nome)


def testo(nome_rom, intervallo, array, dati):
    righe = ['/* Generato da decomp/estrai_grafica.py dalla ROM %s %s: non modificare'
             % (nome_rom, intervallo),
             ' * a mano. Grafica derivata dalle ROM Tecmo, non di proprietà dell\'autore del',
             ' * repository (NOTICE.md). */',
             '#include "video.h"',
             'const uint8_t %s[VID_PEZZO] = {' % array]
    for i in range(0, len(dati), PER_RIGA):
        righe.append('    ' + ''.join('0x%02X,' % b for b in dati[i:i + PER_RIGA]))
    righe.append('};')
    return '\n'.join(righe) + '\n'


def main():
    totale = 0
    for nome, lung, pezzi in ROM:
        rom = leggi_rom(nome, lung)
        for k, (array, suff) in enumerate(pezzi):
            dati = rom[k * PEZZO:(k + 1) * PEZZO]
            intervallo = '%04X-%04X' % (k * PEZZO, (k + 1) * PEZZO - 1)
            t = testo(nome, intervallo, array, dati)
            riletti = bytes(int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]{2}),', t))
            if riletti != dati:
                print('ERRORE: andata e ritorno fallita per %s' % array, file=sys.stderr)
                sys.exit(2)
            percorso = os.path.join(TECMO, 'video_dati_%s.c' % suff)
            vecchio = None
            if os.path.exists(percorso):
                with open(percorso, encoding='utf-8') as f:
                    vecchio = f.read()
            if vecchio != t:
                with open(percorso, 'w', encoding='utf-8') as f:
                    f.write(t)
            totale += len(dati)
    print('grafica: %d file, %d byte, andata e ritorno ok'
          % (sum(len(p) for _, _, p in ROM), totale))


main()
