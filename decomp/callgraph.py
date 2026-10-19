#!/usr/bin/env python3
"""Grafo delle chiamate del Z80 principale dai file derivati di coverage.py.

Ingressi: out/istruzioni.txt, out/chiamate.txt, out/main64.bin.
Uscita: out/callgraph.csv con colonne
  entry,byte,invocazioni,chiamati,chiamanti,aree,indiretto,brutta
Le routine sono le destinazioni di call/rst eseguite più la NMI (0066).
Estensione di una routine: istruzioni eseguite dall'ingresso fino al
prossimo ingresso in ordine di indirizzo (sovrastima le code condivise).
Stampa anche le candidate per la prova di metodo (foglie senza porte,
almeno 10 istruzioni, scritture in ram).
"""
import bisect
import re

from coverage import lunghezza

OPERANDO16 = re.compile(r'\$([0-9A-F]{4})\b')
NMI = 0x0066
AREE = [('ram', 0xC000, 0xCFFF), ('video', 0xD000, 0xDFFF),
        ('sprite', 0xE000, 0xE07F), ('palette', 0xE400, 0xE5FF),
        ('porte', 0xE600, 0xE606), ('audio', 0xE800, 0xE800)]
# istruzioni che scrivono sul primo operando quando è in memoria
SCRIVONO = {'ld', 'inc', 'dec', 'set', 'res', 'rlc', 'rrc', 'rl', 'rr',
            'sla', 'sra', 'srl', 'sll'}
BLOCCO = {'ldir', 'lddr', 'ldi', 'ldd', 'rld', 'rrd'}
INDIRETTI = ('(hl)', '(ix', '(iy', '(de)', '(bc)')


def leggi_istruzioni():
    istr = {}
    for riga in open('out/istruzioni.txt'):
        if riga.startswith('#'):
            continue
        pc, n, t = riga.rstrip('\n').split('\t')
        istr[int(pc, 16)] = (int(n), t)
    return istr


def leggi_chiamate():
    ch = []
    for riga in open('out/chiamate.txt'):
        if riga.startswith('#'):
            continue
        s, d, n = riga.split('\t')
        ch.append((int(s, 16), int(d, 16), int(n)))
    return ch


def area(a):
    for nome, i, f in AREE:
        if i <= a <= f:
            return nome
    return None


def scrive_indiretto(t):
    parti = t.split(None, 1)
    mn = parti[0]
    if mn in BLOCCO:
        return True
    if mn in SCRIVONO and len(parti) > 1:
        primo = parti[1].split(',')[0] if mn not in ('set', 'res') \
            else parti[1].split(',')[-1]
        return primo.startswith(INDIRETTI)
    return False


def principale():
    rom = open('out/main64.bin', 'rb').read()
    istr = leggi_istruzioni()
    chiamate = leggi_chiamate()
    ingressi = sorted({d for _, d, _ in chiamate} | {NMI})
    pcs = sorted(istr)

    def routine_di(pc):
        i = bisect.bisect_right(ingressi, pc) - 1
        return ingressi[i] if i >= 0 else None

    invocazioni = {e: 0 for e in ingressi}
    chiamati = {e: set() for e in ingressi}
    chiamanti = {e: set() for e in ingressi}
    for s, d, n in chiamate:
        invocazioni[d] += n
        r = routine_di(s)
        if r is not None:
            chiamati[r].add(d)
            chiamanti[d].add(r)
        else:
            chiamanti[d].add(s)
    invocazioni[NMI] = istr.get(NMI, (0, ''))[0]

    righe = []
    candidate = []
    for k, e in enumerate(ingressi):
        fine = ingressi[k + 1] if k + 1 < len(ingressi) else 0x10000
        dentro = pcs[bisect.bisect_left(pcs, e):bisect.bisect_left(pcs, fine)]
        nbyte = sum(lunghezza(rom, p) for p in dentro)
        aree = set()
        indiretto = 0
        motivi = set()
        for p in dentro:
            t = istr[p][1]
            mn = t.split(None, 1)[0]
            if mn not in ('jp', 'jr', 'call', 'djnz', 'rst'):
                for x in OPERANDO16.findall(t):
                    a = area(int(x, 16))
                    if a:
                        aree.add(a)
            if scrive_indiretto(t):
                indiretto = 1
            if t == 'rst $20':
                motivi.add('rst20')
            if t in ('jp (hl)', 'jp (ix)', 'jp (iy)'):
                motivi.add('jp(hl)')
            if p >= 0xC000 and p < 0xF000:
                motivi.add('ram-exec')
        if e == 0x0020:
            motivi.add('rst20')
        if 'porte' in aree:
            motivi.add('porte')
        ordine = ['porte', 'rst20', 'jp(hl)', 'ram-exec']
        brutta = '+'.join(m for m in ordine if m in motivi)
        nomi_aree = ' '.join(n for n, _, _ in AREE if n in aree)
        righe.append('%04X,%d,%d,%d,%d,%s,%d,%s' % (
            e, nbyte, invocazioni[e], len(chiamati[e]), len(chiamanti[e]),
            nomi_aree, indiretto, brutta))
        if (not chiamati[e] and not motivi and len(dentro) >= 10
                and ('ram' in aree or indiretto)):
            candidate.append((e, len(dentro), nbyte, invocazioni[e], nomi_aree))

    with open('out/callgraph.csv', 'w') as f:
        f.write('entry,byte,invocazioni,chiamati,chiamanti,aree,indiretto,brutta\n')
        for r in righe:
            f.write(r + '\n')
    brutte = sum(1 for r in righe if not r.endswith(','))
    print('routine: %d, brutte: %d, normali: %d' % (len(righe), brutte, len(righe) - brutte))
    print('candidate per la prova di metodo (entry, istruzioni, byte, invocazioni, aree):')
    for c in candidate:
        print('  %04X %d %d %d %s' % c)


if __name__ == '__main__':
    principale()
