#!/usr/bin/env python3
"""Censimento degli accessi a porte hardware del Z80 principale (brief 12).

Il Z80 principale non ha spazio di I/O (nessuna io map in solomon.cpp):
le porte sono mappate in memoria a e600-e606 (letture), e600/e604/e800
(scritture) e indirizzi non mappati (e646, e802, e803). Lo script trova nel
disassembly statico:
  - le istruzioni `in`/`out` letterali (tutte in zone dati, mai eseguite);
  - gli accessi diretti con operando (0e6xxh)/(0e8xxh);
  - i caricamenti di e6xx in una coppia di registri (accessi indiretti).
Per ogni sito riporta direzione e conteggi nelle tracce di sessione.

Uso (da decomp/): python3 censimento_porte.py
Richiede out/main48.dasm e, se presenti, out/<sessione>_istruzioni.txt
per le sessioni titolo, stanza_00, gameover.
"""
import os
import re

SESSIONI = ['titolo', 'stanza_00', 'gameover']
DIRETTO = re.compile(r'^\t(ld .*)\t+;([0-9a-f]{4})$')
ACCESSO = re.compile(r'\((0e[68][0-9a-f]{2})h\)')
PUNTATORE = re.compile(r'^ld (hl|de|bc|ix|iy),(0e[68][0-9a-f]{2})h$')
IN_OUT = re.compile(r'^\t((?:in|out|ini|ind|outi|outd|inir|indr|otir|otdr) .*)\t+;([0-9a-f]{4})$')


def conteggi_sessione(nome):
    """pc -> esecuzioni dalla out/<nome>_istruzioni.txt (vuoto se assente)."""
    percorso = os.path.join('out', f'{nome}_istruzioni.txt')
    conta = {}
    if not os.path.exists(percorso):
        return conta
    with open(percorso) as f:
        for riga in f:
            if riga.startswith('#'):
                continue
            campi = riga.rstrip('\n').split('\t')
            conta[int(campi[0], 16)] = int(campi[1])
    return conta


def principale():
    sessioni = {n: conteggi_sessione(n) for n in SESSIONI}
    diretti, puntatori, letterali = [], [], []
    with open('out/main48.dasm') as f:
        for riga in f:
            riga = riga.rstrip('\n')
            m = DIRETTO.match(riga)
            if m:
                testo, pc = m.group(1).rstrip(), int(m.group(2), 16)
                acc = ACCESSO.search(testo)
                if acc:
                    porta = int(acc.group(1), 16)
                    # direzione: ld (porta),x = scrittura; ld x,(porta) = lettura
                    direzione = 'W' if testo.startswith(f'ld ({acc.group(1)}h)') else 'R'
                    diretti.append((pc, porta, direzione, testo))
                    continue
                p = PUNTATORE.match(testo)
                if p:
                    puntatori.append((pc, int(p.group(2), 16), p.group(1), testo))
                    continue
            m = IN_OUT.match(riga)
            if m:
                letterali.append((int(m.group(2), 16), m.group(1)))

    def frequenze(pc):
        return '  '.join('%s=%d' % (n[0], sessioni[n].get(pc, 0)) for n in SESSIONI)

    print('# Accessi diretti (operando e6xx/e8xx):', len(diretti))
    for pc, porta, direzione, testo in diretti:
        print('%04X  %04X  %s  %-18s  %s' % (pc, porta, direzione, testo, frequenze(pc)))
    print()
    print('# Accessi indiretti (porta in coppia di registri):', len(puntatori))
    for pc, porta, registro, testo in puntatori:
        print('%04X  %04X  via %s  %-14s  %s' % (pc, porta, registro, testo, frequenze(pc)))
    print()
    print('# Istruzioni in/out letterali (attese tutte in zone dati, mai eseguite):',
          len(letterali))
    for pc, testo in letterali:
        n = sum(s.get(pc, 0) for s in sessioni.values())
        print('%04X  %-18s  esecuzioni totali=%d' % (pc, testo, n))


if __name__ == '__main__':
    principale()
