#!/usr/bin/env python3
"""Byte diversi fra due dump c000-ffff (16384 byte), raggruppati per zona.

Uso (da decomp/): python3 replay_diff.py <dump_a.bin> <dump_b.bin>
I dump sono quelli di replay.lua (out/replay_<f>.bin) o di snap.lua. Per
ogni zona nominata stampa il numero di byte diversi e i primi indirizzi con
i due valori; negli slot oggetti c46e-c8ed dice anche il numero di slot e lo
scostamento nel record. Con la zona e l'ultima attività spedita (dal rapporto
di harness/replay) si attribuisce la divergenza alla routine responsabile.
"""
import sys

BASE = 0xC000

# (inizio, fine compresa, nome): la prima che contiene l'indirizzo vince
ZONE = [
    (0xC000, 0xC03F, "tabella attività"),
    (0xC040, 0xC043, "IX/IY salvati dalla NMI"),
    (0xC046, 0xC04D, "monete/crediti"),
    (0xC04E, 0xC04F, "indici spedidore"),
    (0xC050, 0xC053, "ingressi e comando"),
    (0xC054, 0xC057, "coda suoni"),
    (0xC463, 0xC468, "contatori"),
    (0xC2A9, 0xC2A9, "contatori"),
    (0xC46E, 0xC8ED, "slot oggetti"),
    (0xC8EE, 0xC94D, "tabella sprite"),
    (0xC994, 0xC9B3, "palette"),
    (0xC9CA, 0xCA49, "palette"),
    (0xCF00, 0xCFFF, "pila"),
    (0xD880, 0xDFFF, "fondale"),
    (0xE020, 0xE07F, "registri sprite"),
    (0xE400, 0xE5FF, "registri palette"),
]
SLOT_INIZIO, SLOT_PASSO = 0xC46E, 0x30
MOSTRATI = 8


def zona(a):
    for ini, fine, nome in ZONE:
        if ini <= a <= fine:
            return nome
    return "non mappata"


def carica(nome):
    with open(nome, "rb") as f:
        d = f.read()
    if len(d) != 0x4000:
        sys.exit(f"{nome}: {len(d)} byte invece di 16384")
    return d


def main():
    if len(sys.argv) != 3:
        sys.exit("uso: python3 replay_diff.py <dump_a.bin> <dump_b.bin>")
    a, b = carica(sys.argv[1]), carica(sys.argv[2])
    gruppi = {}
    for i in range(0x4000):
        if a[i] != b[i]:
            gruppi.setdefault(zona(BASE + i), []).append(BASE + i)
    if not gruppi:
        print("nessun byte diverso")
        return
    totale = sum(len(v) for v in gruppi.values())
    print(f"byte diversi: {totale} (a = {sys.argv[1]}, b = {sys.argv[2]})")
    ordine = sorted(gruppi, key=lambda z: gruppi[z][0])
    for z in ordine:
        ind = gruppi[z]
        print(f"{z}: {len(ind)} byte, {ind[0]:04X}-{ind[-1]:04X}")
        for x in ind[:MOSTRATI]:
            extra = ""
            if z == "slot oggetti":
                s, o = divmod(x - SLOT_INIZIO, SLOT_PASSO)
                extra = f"  slot {s} (+{o:02X})"
            print(f"  {x:04X}: a {a[x - BASE]:02X} b {b[x - BASE]:02X}{extra}")
        if len(ind) > MOSTRATI:
            print(f"  ... altri {len(ind) - MOSTRATI}")


if __name__ == "__main__":
    main()
