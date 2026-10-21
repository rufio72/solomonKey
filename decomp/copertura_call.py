#!/usr/bin/env python3
"""Copertura delle destinazioni statiche di call osservate nelle sessioni.

Uso (da decomp/): python3 copertura_call.py out/*_eseguiti.txt
Legge out/main48.dasm (z80dasm) per le destinazioni statiche di call in
0000-bfff e uno o più file di PC eseguiti (uno esadecimale per riga).
Stampa la copertura per ogni file e per l'unione e scrive
  out/eseguiti_unione.txt    PC eseguiti in almeno una sessione
  out/call_mai_osservate.txt destinazioni mai osservate, con il sito della
                             prima call statica che le raggiunge
Con --classifica aggiunge a out/call_mai_osservate.txt la colonna
sito_eseguito (SI/NO: il PC della prima call statica è nell'unione); NO vuol
dire probabile dato disassemblato come codice, o codice raggiungibile solo da
zone a loro volta mai eseguite.
"""
import argparse
import re

CALL = re.compile(r'^\tcall (?:[a-z]+,)?(?:0([0-9a-f]{4})h|(?:l|sub_)([0-9a-f]{4})h)(?:\+\d+)?\t')
SITO = re.compile(r';([0-9a-f]{4})\s*$')
UNIONE = 'out/eseguiti_unione.txt'


def destinazioni(dasm):
    """Destinazione -> primo sito di call (indirizzo nel commento z80dasm)."""
    dest = {}
    with open(dasm) as f:
        for riga in f:
            m = CALL.match(riga)
            if not m:
                continue
            d = int(m.group(1) or m.group(2), 16)
            if d >= 0xC000:
                continue
            s = SITO.search(riga)
            sito = int(s.group(1), 16) if s else -1
            if d not in dest or (sito >= 0 and sito < dest[d]):
                dest[d] = sito
    return dest


def leggi_pc(nome):
    with open(nome) as f:
        return {int(r, 16) for r in (x.strip() for x in f) if r}


def riga(etichetta, oss, tot):
    return '%s: %d su %d (%.1f%%)' % (etichetta, oss, tot, 100.0 * oss / tot)


def principale():
    ap = argparse.ArgumentParser(description='Copertura delle destinazioni di call')
    ap.add_argument('eseguiti', nargs='+', help='file *_eseguiti.txt')
    ap.add_argument('--dasm', default='out/main48.dasm')
    ap.add_argument('--classifica', action='store_true',
                    help='colonna sito_eseguito SI/NO in call_mai_osservate.txt')
    arg = ap.parse_args()

    dest = destinazioni(arg.dasm)
    tot = len(dest)
    unione = set()
    file_letti = 0
    for nome in arg.eseguiti:
        if nome == UNIONE:
            continue          # l'unione di una corsa precedente non conta
        pc = leggi_pc(nome)
        unione |= pc
        file_letti += 1
        oss = sum(1 for d in dest if d in pc)
        print(riga('%s destinazioni statiche osservate' % nome if len(arg.eseguiti) > 1
                   else 'destinazioni statiche osservate', oss, tot))
    with open(UNIONE, 'w') as f:
        for p in sorted(unione):
            f.write('%04X\n' % p)
    mancano = sorted(d for d in dest if d not in unione)
    with open('out/call_mai_osservate.txt', 'w') as f:
        if arg.classifica:
            f.write('# destinazione\tprima call statica\tsito_eseguito\n')
        else:
            f.write('# destinazione\tprima call statica\n')
        for d in mancano:
            if arg.classifica:
                f.write('%04X\t%04X\t%s\n' % (d, dest[d], 'SI' if dest[d] in unione else 'NO'))
            else:
                f.write('%04X\t%04X\n' % (d, dest[d]))
    if file_letti > 1:
        print(riga('unione', tot - len(mancano), tot))


if __name__ == '__main__':
    principale()
