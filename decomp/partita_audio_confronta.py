#!/usr/bin/env python3
"""Confronto dei flussi dei cambi AY (lotto H6, brief 50).

Uso: python3 partita_audio_confronta.py <oracolo_ay.txt> <c_ay.txt> <da> <a>
     python3 partita_audio_confronta.py --hash-wav F.wav

Per i frame da <da> a <a> stampa: frame con insieme di cambi diverso (insiemi
ordinati di righe "chip reg val"), frame con stato di fine frame diverso (stato
cumulato per (chip, reg)), primo frame diverso, ordine interno diverso
(stesso insieme ma sequenza diversa: solo informativo). Uscita 0 se i primi due
numeri sono 0, altrimenti 1.
--hash-wav: FNV-1a 64 bit dei byte del wav oltre i primi 44 (intestazione)."""
import sys
from collections import defaultdict


def leggi(percorso):
    """Ritorna {frame: [(chip, reg, val), ...]} nell'ordine del file."""
    cambi = defaultdict(list)
    with open(percorso) as f:
        for riga in f:
            if riga.startswith("#") or not riga.strip():
                continue
            p = riga.split()
            if len(p) != 4:
                continue
            cambi[int(p[0])].append((int(p[1], 16), int(p[2], 16), int(p[3], 16)))
    return cambi


def hash_wav(percorso):
    h = 0xcbf29ce484222325
    with open(percorso, "rb") as f:
        dati = f.read()[44:]
    for b in dati:
        h = ((h ^ b) * 0x100000001b3) & 0xFFFFFFFFFFFFFFFF
    print("WAV campioni %d hash %016x" % (len(dati) // 2, h))
    return 0


def main(argv):
    if len(argv) == 3 and argv[1] == "--hash-wav":
        return hash_wav(argv[2])
    if len(argv) != 5:
        print(__doc__)
        return 2
    a, b = leggi(argv[1]), leggi(argv[2])
    da, fino = int(argv[3]), int(argv[4])
    sa, sb = {}, {}
    for f in range(0, da):                 # stato cumulato prima di <da>
        for c, r, v in a.get(f, []): sa[(c, r)] = v
        for c, r, v in b.get(f, []): sb[(c, r)] = v
    n_ins = n_sta = n_ord = 0
    primo = None
    for f in range(da, fino + 1):
        ca, cb = a.get(f, []), b.get(f, [])
        diverso = False
        if sorted(ca) != sorted(cb):
            n_ins += 1
            diverso = True
        elif ca != cb:
            n_ord += 1
        for c, r, v in ca: sa[(c, r)] = v
        for c, r, v in cb: sb[(c, r)] = v
        if sa != sb:
            n_sta += 1
            diverso = True
        if diverso and primo is None:
            primo = f
    print("frame con insieme di cambi diverso %d" % n_ins)
    print("frame con stato di fine frame diverso %d" % n_sta)
    print("primo frame diverso %s" % (primo if primo is not None else "nessuno"))
    print("ordine interno diverso %d" % n_ord)
    return 0 if n_ins == 0 and n_sta == 0 else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
