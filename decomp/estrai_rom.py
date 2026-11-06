#!/usr/bin/env python3
"""Estrae in tabelle C i dati del Z80 principale letti a runtime (brief 62)
e il codice del driver del Z80 audio (brief 63).

Uso (da decomp/): python3 -I estrai_rom.py
Legge le ROM 6.3f, 7.3h, 8.3jk e 1.3jk da ../roms/ext/solomon/ (o relativo
allo script), NON out/main64.bin, e scrive in ../tecmo/:
  rom_dati_{varie,testi,stanze,oggetti,fondali}.c   una tabella per voce
  rom_dati.h                                         dichiarazioni e RomVoce
  rom_voci.c                                         rom_voci[] per indirizzo
  audio_driver.{c,h}                                 0000-05C5 di 1.3jk
Le voci sono quelle con spazio 'P' di VOCI in censimento_rom.py (letto
dalla ROM più tab_8089 già estratta) e il riempimento F997-FFFF, che
completa la finestra F000-FFFF copiata in ram[] da macchina.c e
macchina_libera.c. Prima di scrivere rilegge i valori dal testo prodotto e
li confronta con la ROM; se differiscono esce con 2 (anche per il driver). Un file si riscrive
solo se il contenuto cambia, quindi rieseguirlo non cambia nulla.
"""
import os
import re
import sys

QUI = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, QUI)
from censimento_rom import VOCI, L, file_generato  # noqa: E402

TECMO = os.path.join(QUI, '..', 'tecmo')
CARTELLE = ['../roms/ext/solomon', '../../roms/ext/solomon',
            os.path.join(QUI, '../roms/ext/solomon')]
PER_RIGA = 24
# 88 voci e 17739 byte dal brief 61; il brief 62 ne aggiunge 2 e ne allarga 7,
# il collaudo del 2026-11-06 allarga tab_3709_8661, il brief 65
# copione_demo_767c (CENSIMENTO-ROM.md, «Esito del lotto 62»).
VOCI_ATTESE, BYTE_L_ATTESI = 91, 20118

# Voce locale: riempimento della finestra F000-FFFF dopo i flussi dei fondali.
# Il codice del driver audio finisce dove cominciano i dati (estrai_dati.py).
DRIVER_FINE = 0x05C6

RIEMPIMENTO = ('P', 'riempimento_f997', 0xF997, 0xFFFF, 'riempimento',
               'memcpy F000 (macchina.c, macchina_libera.c)', 'byte', 'statica')

FILE_ORDINE = ['rom_dati_varie.c', 'rom_dati_testi.c', 'rom_dati_stanze.c',
               'rom_dati_oggetti.c', 'rom_dati_fondali.c']

AVVISO = ('Generato da decomp/estrai_rom.py: non modificare a mano. Dati derivati\n'
          ' * dalla ROM Tecmo (tecmo/README.md, NOTICE.md).')


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


def origine(a):
    """File ROM e posizione del byte a (build_rom.sh)."""
    if a < 0x4000:
        return '6.3f', a
    if a < 0x8000:
        return '7.3h', a            # seconda metà di 7.3h
    if a < 0xC000:
        return '7.3h', a - 0x8000   # prima metà di 7.3h
    if a >= 0xF000:
        return '8.3jk', a - 0xF000
    return None, None


def byte_voce(chip, v):
    f, off = origine(v[2])
    f2, _ = origine(v[3])
    if f is None or f != f2:
        print('ERRORE: %s fuori dalla ROM o a cavallo di un confine' % v[1], file=sys.stderr)
        sys.exit(2)
    return f, chip[f][off:off + v[3] - v[2] + 1]


def testo_file(voci):
    righe = ['/* ' + AVVISO + ' */', '#include "rom_dati.h"']
    for v, f, dati in voci:
        righe.append('')
        righe.append('/* %04X-%04X da %s (%s) */' % (v[2], v[3], f, v[6]))
        righe.append('const uint8_t %s[%d] = {' % (v[1], len(dati)))
        for i in range(0, len(dati), PER_RIGA):
            righe.append('    ' + ''.join('0x%02X,' % b for b in dati[i:i + PER_RIGA]))
        righe.append('};')
    return '\n'.join(righe) + '\n'


def testo_h(voci):
    righe = ['/* ' + AVVISO + ' */',
             '#ifndef ROM_DATI_H',
             '#define ROM_DATI_H',
             '',
             '#include <stdint.h>',
             '']
    for v, _, dati in voci:
        righe.append('extern const uint8_t %s[%d];  /* %04X-%04X */' % (v[1], len(dati), v[2], v[3]))
    righe += ['',
              '/* Una voce: indirizzo Z80 d\'inizio, lunghezza, byte. */',
              'typedef struct { uint16_t inizio; uint16_t lung; const uint8_t *dati; } RomVoce;',
              'extern const RomVoce rom_voci[];',
              'extern const unsigned rom_voci_n;',
              '',
              '#endif']
    return '\n'.join(righe) + '\n'


def testo_voci(voci):
    righe = ['/* ' + AVVISO + ' */',
             '#include "rom_dati.h"',
             '',
             '/* Tutte le voci, ordinate per indirizzo: carica_rom() le ricompone in rom[]. */',
             'const RomVoce rom_voci[] = {']
    for v, _, dati in voci:
        righe.append('    { 0x%04X, %d, %s },' % (v[2], len(dati), v[1]))
    righe += ['};', 'const unsigned rom_voci_n = sizeof rom_voci / sizeof rom_voci[0];']
    return '\n'.join(righe) + '\n'


def testo_driver(dati):
    righe = ['/* ' + AVVISO,
             ' * Codice del driver del Z80 audio, 0000-05C5 di 1.3jk. */',
             '#include "audio_driver.h"',
             '',
             'const uint8_t aud_driver[%d] = {' % len(dati)]
    for i in range(0, len(dati), PER_RIGA):
        righe.append('    ' + ''.join('0x%02X,' % b for b in dati[i:i + PER_RIGA]))
    righe.append('};')
    return '\n'.join(righe) + '\n'


def testo_driver_h(dati):
    righe = ['/* ' + AVVISO + ' */',
             '#ifndef AUDIO_DRIVER_H',
             '#define AUDIO_DRIVER_H',
             '',
             '#include <stdint.h>',
             '',
             'extern const uint8_t aud_driver[%d];  /* 0000-05C5 di 1.3jk */' % len(dati),
             '',
             '#endif']
    return '\n'.join(righe) + '\n'


def driver_audio():
    """Blob 0000-05C5 di 1.3jk: controlla le voci 'A' del censimento, scrive
    audio_driver.{c,h} dopo l'andata e ritorno."""
    voci_a = {v[1]: v for v in VOCI if v[0] == 'A'}
    g, d = voci_a.get('aud_gestori_03ce'), voci_a.get('aud_dati_05c6')
    if (g is None or d is None or g[2] < 0 or g[3] >= DRIVER_FINE
            or d[2] != DRIVER_FINE):
        print('ERRORE: voci A del censimento incoerenti con il driver 0000-05C5',
              file=sys.stderr)
        sys.exit(2)
    dati = leggi_rom('1.3jk', 0x4000)[:DRIVER_FINE]
    testo = testo_driver(dati)
    riletti = bytes(int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]{2}),', testo))
    if riletti != dati:
        print('ERRORE: andata e ritorno fallita per audio_driver.c', file=sys.stderr)
        sys.exit(2)
    scrivi('audio_driver.h', testo_driver_h(dati))
    scrivi('audio_driver.c', testo)
    print('audio: driver 0000-%04X, %d byte, andata e ritorno ok' % (DRIVER_FINE - 1, len(dati)))


def scrivi(nome, t):
    percorso = os.path.join(TECMO, nome)
    vecchio = None
    if os.path.exists(percorso):
        with open(percorso, encoding='utf-8') as f:
            vecchio = f.read()
    if vecchio != t:
        with open(percorso, 'w', encoding='utf-8') as f:
            f.write(t)


def main():
    voci_p = [v for v in VOCI if v[0] == 'P']
    byte_l = sum(v[3] - v[2] + 1 for v in voci_p if v[4] == L)
    if len(voci_p) != VOCI_ATTESE or byte_l != BYTE_L_ATTESI:
        print('ERRORE: censimento cambiato: %d voci P (attese %d), %d byte letti (attesi %d)'
              % (len(voci_p), VOCI_ATTESE, byte_l, BYTE_L_ATTESI), file=sys.stderr)
        sys.exit(2)
    chip = {'6.3f': leggi_rom('6.3f', 0x4000), '7.3h': leggi_rom('7.3h', 0x8000),
            '8.3jk': leggi_rom('8.3jk', 0x8000)}
    tutte = sorted(voci_p + [RIEMPIMENTO], key=lambda v: v[2])
    for a, b in zip(tutte, tutte[1:]):
        if b[2] <= a[3]:
            print('ERRORE: voci sovrapposte %s e %s' % (a[1], b[1]), file=sys.stderr)
            sys.exit(2)
    estratte = [(v,) + byte_voce(chip, v) for v in tutte]
    per_file = {n: [] for n in FILE_ORDINE}
    for e in estratte:
        per_file[file_generato(e[0])].append(e)
    testi = {n: testo_file(per_file[n]) for n in FILE_ORDINE}
    # andata e ritorno: i byte riletti dal testo, file per file, uguali alla ROM
    for n in FILE_ORDINE:
        riletti = bytes(int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]{2}),', testi[n]))
        attesi = b''.join(d for _, _, d in per_file[n])
        if riletti != attesi:
            print('ERRORE: andata e ritorno fallita per %s' % n, file=sys.stderr)
            sys.exit(2)
    os.makedirs(TECMO, exist_ok=True)
    for n in FILE_ORDINE:
        scrivi(n, testi[n])
    scrivi('rom_dati.h', testo_h(estratte))
    scrivi('rom_voci.c', testo_voci(estratte))
    totale = sum(len(d) for _, _, d in estratte)
    print('rom: %d voci, %d byte, andata e ritorno ok' % (len(estratte), totale))
    driver_audio()


main()
