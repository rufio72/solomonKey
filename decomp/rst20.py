#!/usr/bin/env python3
"""Tabelle inline di `rst $20` (salto su tabella che segue l'istruzione).

Il gestore a 0x0020 equivale a `jp tabella[A]`, con la tabella di parole
little-endian subito dopo il byte E7. Lo script:
- prende i siti statici dalle righe `rst 20h` di out/main48.dasm (0000-bfff);
- stima il numero di voci con un'euristica (parole in 0000-bfff, la tabella
  finisce prima della più piccola destinazione maggiore del sito) e con il
  limite dato dalle istruzioni che precedono il salto (`cp n; jr nc` -> n
  voci, `and m` -> m+1 voci, ogni `dec a` toglie una voce);
- se esiste out/trace.txt la legge riga per riga: sito dinamico = riga
  `XXXX: rst  $20`, destinazione = PC della riga che segue `0026: ret`;
- scrive out/rst20.txt, harness/rst20_tables.h e out/main48_annotato.dasm.

Uso (da decomp/): python3 rst20.py [--rom out/main64.bin] [--traccia out/trace.txt]
"""
import argparse
import os
import re

INIZIO_DATI = 0x80a5          # da qui a bfff la ROM contiene dati
RIGA_ISTR = re.compile(r'^\t(.+?)\t+;([0-9a-f]{4})$')
RIGA_ETICHETTA = re.compile(r'^(l|sub_)([0-9a-f]{4})h:$')
RIGA_TRACCIA = re.compile(r'^([0-9A-F]{4}): (.*)$')


def leggi_dasm(nome):
    """Restituisce (righe, istruzioni) con istruzioni = lista di
    (indice_riga, indirizzo, testo) in ordine di file."""
    with open(nome) as f:
        righe = f.read().split('\n')
    istr = []
    for i, r in enumerate(righe):
        m = RIGA_ISTR.match(r)
        if m:
            istr.append((i, int(m.group(2), 16), m.group(1).strip()))
    return righe, istr


def trasferimento_incondizionato(t):
    return (t == 'ret' or t.startswith('jp ') and ',' not in t
            or t.startswith('jr ') and ',' not in t or t == 'jp (hl)')


def scrive_a(t):
    """Vero se l'istruzione cambia A in modo che il limite non si possa dedurre."""
    if t.startswith('ld a,') or t in ('pop af', 'ex af,af\''):
        return True
    for p in ('add a,', 'adc a,', 'sub ', 'sbc a,', 'or ', 'xor ', 'rla', 'rra',
              'rlca', 'rrca', 'inc a', 'neg', 'cpl', 'in a,'):
        if t.startswith(p):
            return True
    return False


def limite(istr, pos, righe, salti):
    """Limite al numero di voci dedotto dalle istruzioni prima di istr[pos]."""
    correzione = 0
    k = pos - 1
    passi = 0
    while k >= 0 and passi < 12:
        i_riga, ind, t = istr[k]
        # etichetta fra questa istruzione e la successiva: se qui il flusso
        # non prosegue, si continua dal salto che porta all'etichetta
        etich = [RIGA_ETICHETTA.match(righe[j]) for j in range(i_riga + 1, istr[k + 1][0])]
        etich = [int(m.group(2), 16) for m in etich if m]
        if etich and trasferimento_incondizionato(t):
            origini = salti.get(etich[0], [])
            if len(origini) != 1:
                return None
            k = origini[0]
            passi += 1
            continue
        if t == 'dec a':
            correzione -= 1
        elif t.startswith('cp 0') and t.endswith('h'):
            succ = istr[k + 1][2]
            if succ.startswith(('jr nc,', 'jp nc,')):
                return int(t[3:-1], 16) + correzione
            return None
        elif t.startswith('and 0') and t.endswith('h'):
            return int(t[4:-1], 16) + 1 + correzione
        elif scrive_a(t) or trasferimento_incondizionato(t):
            return None
        k -= 1
        passi += 1
    return None


def voci_euristiche(rom, sito, ingressi=frozenset()):
    """Parole da sito+1 finché stanno in 0000-bfff e la posizione precede la
    più piccola destinazione maggiore del sito; con `ingressi` (destinazioni
    note di tutte le tabelle) la tabella finisce anche dove inizia codice noto."""
    voci = []
    minima = 0x10000
    p = sito + 1
    while p + 1 < 0xc000 and p < minima and p not in ingressi:
        w = rom[p] | (rom[p + 1] << 8)
        if w > 0xbfff:
            break
        voci.append(w)
        if w > sito:
            minima = min(minima, w)
        p += 2
    return voci


def leggi_traccia(nome):
    """Siti dinamici: {sito: {destinazione: conteggio}}. Pila per gli annidamenti
    (una NMI fra il rst e il ret del gestore può fare il suo rst $20)."""
    dinamici = {}
    pila = []
    in_attesa = None
    with open(nome) as f:
        for r in f:
            m = RIGA_TRACCIA.match(r)
            if not m:
                continue
            pc = int(m.group(1), 16)
            if in_attesa is not None:
                if pc != 0x0066:
                    d = dinamici.setdefault(in_attesa, {})
                    d[pc] = d.get(pc, 0) + 1
                in_attesa = None
            istr = m.group(2)
            if istr.startswith('rst  $20'):
                pila.append(pc)
            elif pc == 0x0026 and pila:
                in_attesa = pila.pop()
    return dinamici


def main():
    ap = argparse.ArgumentParser(description='Tabelle inline di rst $20')
    ap.add_argument('--rom', default='out/main64.bin')
    ap.add_argument('--dasm', default='out/main48.dasm')
    ap.add_argument('--traccia', default='out/trace.txt')
    ap.add_argument('--intestazione', default='harness/rst20_tables.h')
    arg = ap.parse_args()

    with open(arg.rom, 'rb') as f:
        rom = f.read()
    righe, istr = leggi_dasm(arg.dasm)
    # salti verso etichette: indirizzo -> posizioni in istr
    salti = {}
    for k, (_, _, t) in enumerate(istr):
        m = re.search(r'l([0-9a-f]{4})h$', t)
        if m and t.startswith(('jp ', 'jr ', 'djnz ')):
            salti.setdefault(int(m.group(1), 16), []).append(k)

    # due passate: la seconda ferma ogni tabella sulle destinazioni trovate
    # nella prima (es. 3B77, voce della tabella di 3B10, chiude quella di 3B68)
    ingressi = frozenset()
    for passata in range(2):
        siti = {}
        for k, (i_riga, ind, t) in enumerate(istr):
            if t == 'rst 20h' and ind < 0xc000:
                euristica = voci_euristiche(rom, ind, ingressi)
                lim = limite(istr, k, righe, salti)
                voci = euristica if lim is None else euristica[:lim]
                siti[ind] = {'riga': i_riga, 'voci': voci, 'limite': lim}
        ingressi = frozenset(w for s, v in siti.items() if s < INIZIO_DATI
                             for w in v['voci'])

    dinamici = leggi_traccia(arg.traccia) if os.path.exists(arg.traccia) else None

    for s, v in siti.items():
        dentro = any(a != s and a + 1 <= s < a + 1 + 2 * len(w['voci'])
                     for a, w in siti.items())
        if dinamici and s in dinamici:
            v['stato'] = 'eseguito'
        elif s >= INIZIO_DATI:
            v['stato'] = 'dati'
        elif dentro:
            v['stato'] = 'dentro-tabella'
        else:
            v['stato'] = 'statico'
        v['indice_max'] = '-'

    if dinamici is not None:
        fuori = 0
        for s, dest in dinamici.items():
            if s not in siti:
                continue
            voci = siti[s]['voci']
            massimo = -1
            for d in dest:
                if d in voci:
                    massimo = max(massimo, voci.index(d))
                else:
                    fuori += 1
                    print('sito %04X: destinazione %04X fuori tabella' % (s, d))
            siti[s]['indice_max'] = str(massimo)
        non_statici = [s for s in dinamici if s not in siti]
        print('siti dinamici: %d, %s; destinazioni fuori tabella: %d' % (
            len(dinamici), 'tutti statici' if not non_statici else
            'non statici: ' + ' '.join('%04X' % s for s in non_statici), fuori))
    else:
        print('traccia %s assente: niente siti dinamici' % arg.traccia)

    with open('out/rst20.txt', 'w') as f:
        for s in sorted(siti):
            v = siti[s]
            f.write('%04X %d %s %s %s\n' % (s, len(v['voci']), v['stato'], v['indice_max'],
                                             ' '.join('%04X' % w for w in v['voci'])))

    with open(arg.intestazione, 'w') as f:
        f.write('/* Generato da rst20.py: non modificare a mano.\n'
                ' * Tabelle inline di rst $20: rst20_XXXX[i] = destinazione per A = i. */\n'
                '#ifndef RST20_TABLES_H\n#define RST20_TABLES_H\n\n#include <stdint.h>\n\n')
        for s in sorted(siti):
            v = siti[s]
            if not v['voci']:
                continue
            f.write('/* %04X: %s, %d voci, indice massimo osservato %s */\n'
                    % (s, v['stato'], len(v['voci']), v['indice_max']))
            f.write('static const uint16_t rst20_%04X[] = {\n' % s)
            for j in range(0, len(v['voci']), 8):
                f.write('    ' + ', '.join('0x%04X' % w for w in v['voci'][j:j + 8]) + ',\n')
            f.write('};\n\n')
        f.write('#endif\n')

    etichette = {}
    for r in righe:
        m = RIGA_ETICHETTA.match(r)
        if m:
            etichette[int(m.group(2), 16)] = r[:-1]
    aggiunte = {v['riga']: v for v in siti.values()}
    with open('out/main48_annotato.dasm', 'w') as f:
        for i, r in enumerate(righe):
            f.write(r + ('\n' if i < len(righe) - 1 else ''))
            if i in aggiunte:
                v = aggiunte[i]
                f.write('; rst20: %d voci (%s)\n' % (len(v['voci']), v['stato']))
                for w in v['voci']:
                    f.write(';   dw %04X  -> %s\n' % (w, etichette.get(w, '?')))
    print('scritti out/rst20.txt, %s, out/main48_annotato.dasm' % arg.intestazione)


if __name__ == '__main__':
    main()
