#!/usr/bin/env python3
"""Copertura di esecuzione del Z80 principale (o audio) dalla traccia MAME.

Legge out/trace.txt in streaming e produce:
  out/eseguiti.txt   PC distinti eseguiti (uno per riga, esadecimale, ordinati)
  out/istruzioni.txt PC, numero di esecuzioni e testo dell'istruzione
  out/chiamate.txt   chiamate eseguite: sito, destinazione, numero
  out/copertura.txt  intervalli contigui codice|dati su 0000-bfff e f000-ffff
  out/numeri.txt     i numeri del sondaggio
Con --cancella elimina la traccia alla fine. Con --prefisso NOME i file
diventano out/NOME_eseguiti.txt, out/NOME_istruzioni.txt e così via (il prefisso può
contenere una sottocartella, es. audio/cod_00_07 -> out/audio/cod_00_07_*).
Con --cpu audio la ROM è ../roms/ext/solomon/1.3jk (16 KB, riempita a 64 KB
con zeri in memoria), la zona di ROM è 0000-3fff e il contatore RAM conta i
PC in 4000-47ff.
"""
import argparse
import os
import re
import sys

RIGA = re.compile(r'^([0-9A-F]{4}): (.*)$')
OPERANDO16 = re.compile(r'\$([0-9A-F]{4})\b')

# lunghezze delle istruzioni senza prefisso
DUE = {0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x36, 0x3E,
       0xC6, 0xCE, 0xD6, 0xDE, 0xE6, 0xEE, 0xF6, 0xFE,
       0xD3, 0xDB, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38}
TRE = {0x01, 0x11, 0x21, 0x31, 0x22, 0x2A, 0x32, 0x3A,
       0xC3, 0xC2, 0xCA, 0xD2, 0xDA, 0xE2, 0xEA, 0xF2, 0xFA,
       0xCD, 0xC4, 0xCC, 0xD4, 0xDC, 0xE4, 0xEC, 0xF4, 0xFC}
ED_QUATTRO = {0x43, 0x4B, 0x53, 0x5B, 0x63, 0x6B, 0x73, 0x7B}
CON_SPIAZZAMENTO = {0x34, 0x35, 0x36, 0x46, 0x4E, 0x56, 0x5E, 0x66, 0x6E, 0x7E,
                    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x77,
                    0x86, 0x8E, 0x96, 0x9E, 0xA6, 0xAE, 0xB6, 0xBE}
# istruzioni che cambiano il flusso: escluse dall'autoverifica di lunghezza
SALTI = {'jp', 'jr', 'call', 'ret', 'rst', 'djnz', 'retn', 'reti'}
# istruzioni a blocco che si ripetono sullo stesso PC
RIPETUTE = {'ldir', 'lddr', 'cpir', 'cpdr', 'inir', 'indr', 'otir', 'otdr', 'halt'}
NMI = 0x0066
# zone di ROM della CPU analizzata (impostate da principale() secondo --cpu)
ROM = [(0x0000, 0xBFFF), (0xF000, 0xFFFF)]
ROM_MAIN = [(0x0000, 0xBFFF), (0xF000, 0xFFFF)]
ROM_AUDIO = [(0x0000, 0x3FFF)]
# ingresso dell'IRQ (modo 1) escluso dall'autoverifica: solo per l'audio, il
# principale resta com'era
IRQ = None


def lunghezza_base(op):
    if op in DUE:
        return 2
    if op in TRE:
        return 3
    return 1


def lunghezza(rom, pc):
    """Lunghezza in byte dell'istruzione a pc secondo le regole del brief."""
    b0 = rom[pc]
    b1 = rom[(pc + 1) & 0xFFFF]
    if b0 == 0xCB:
        return 2
    if b0 == 0xED:
        return 4 if b1 in ED_QUATTRO else 2
    if b0 in (0xDD, 0xFD):
        if b1 == 0xCB:
            return 4
        n = lunghezza_base(b1) + 1
        if b1 in CON_SPIAZZAMENTO:
            n += 1
        return n
    return lunghezza_base(b0)


def in_rom(a):
    return any(i <= a <= f for i, f in ROM)


def destinazione(mnem, operandi):
    """Destinazione di un call/rst dal testo, o None."""
    if mnem == 'rst':
        return int(operandi.strip().lstrip('$'), 16)
    if mnem == 'call':
        return int(operandi.split(',')[-1].strip().lstrip('$'), 16)
    return None


def principale():
    global ROM, IRQ
    ap = argparse.ArgumentParser(description='Copertura dalla traccia MAME')
    ap.add_argument('--traccia', default='out/trace.txt')
    ap.add_argument('--cpu', choices=('main', 'audio'), default='main',
                    help='Z80 principale (predefinito) o Z80 audio')
    ap.add_argument('--rom', default=None,
                    help='immagine ROM (predefinito out/main64.bin o '
                    '../roms/ext/solomon/1.3jk con --cpu audio)')
    ap.add_argument('--cancella', action='store_true',
                    help='cancella la traccia dopo l\'elaborazione')
    ap.add_argument('--prefisso', default='',
                    help='prefisso dei file in out/ (es. stanza_03)')
    arg = ap.parse_args()

    if arg.cpu == 'audio':
        ROM = ROM_AUDIO
        IRQ = 0x0038
        rom = open(arg.rom or '../roms/ext/solomon/1.3jk', 'rb').read()
        assert len(rom) == 0x4000, '1.3jk deve essere di 16384 byte'
        rom = rom + bytes(0x10000 - len(rom))
        ram_da, ram_a, ram_etichetta = 0x4000, 0x47FF, 'PC in RAM 4000-47ff'
    else:
        ROM = ROM_MAIN
        rom = open(arg.rom or 'out/main64.bin', 'rb').read()
        assert len(rom) == 0x10000, 'main64.bin deve essere di 65536 byte'
        ram_da, ram_a, ram_etichetta = 0xC000, 0xFFFF, 'PC >= c000'

    cache = {}            # riga -> (pc, mnem, operandi)
    conteggio = {}        # pc -> esecuzioni
    testo = {}            # pc -> testo istruzione
    chiamate = {}         # (sito, destinazione) -> numero
    destinazioni = set()  # definizione del brief: PC dopo un call/rst
    violazioni = []
    righe = 0
    rst20 = 0
    jp_hl = 0
    jp_ixiy = 0
    pc_ram = 0
    nmi = 0
    prec = None           # (pc, mnem, operandi) della riga precedente

    with open(arg.traccia, 'r', buffering=1 << 20) as f:
        for riga in f:
            v = cache.get(riga)
            if v is None:
                m = RIGA.match(riga.rstrip('\n'))
                if not m:
                    continue
                parti = m.group(2).split(None, 1)
                v = (int(m.group(1), 16), parti[0],
                     parti[1].strip() if len(parti) > 1 else '')
                cache[riga] = v
            pc, mnem, oper = v
            righe += 1
            conteggio[pc] = conteggio.get(pc, 0) + 1
            if ram_da <= pc <= ram_a:
                pc_ram += 1
            if pc == NMI:
                nmi += 1
            if prec is not None:
                ppc, pmn, pop = prec
                if pmn in ('call', 'rst'):
                    destinazioni.add(pc)
                    dest = destinazione(pmn, pop)
                    # la chiamata è avvenuta se si arriva alla destinazione,
                    # o se è incondizionata (anche se la NMI si mette in mezzo)
                    if pc == dest or (pmn == 'rst' or ',' not in pop):
                        k = (ppc, dest)
                        chiamate[k] = chiamate.get(k, 0) + 1
                elif pmn not in SALTI:
                    atteso = (ppc + lunghezza(rom, ppc)) & 0xFFFF
                    ok = (pc == atteso or pc == NMI or pc == IRQ or
                          (pmn in RIPETUTE and pc == ppc))
                    if not ok and len(violazioni) < 1000:
                        violazioni.append((ppc, pc))
                    elif not ok:
                        violazioni.append(None)
            if mnem == 'rst' and oper == '$20':
                rst20 += 1
            elif mnem == 'jp' and oper == '(hl)':
                jp_hl += 1
            elif mnem == 'jp' and oper in ('(ix)', '(iy)'):
                jp_ixiy += 1
            prec = v

    for (pc, mnem, oper) in cache.values():
        testo[pc] = (mnem + ' ' + oper).strip()

    # byte eseguiti
    eseguito = bytearray(0x10000)
    for pc in conteggio:
        for i in range(lunghezza(rom, pc)):
            eseguito[(pc + i) & 0xFFFF] = 1
    totale_rom = sum(f - i + 1 for i, f in ROM)
    byte_eseguiti = sum(eseguito[a] for i, f in ROM for a in range(i, f + 1))

    # intervalli contigui
    intervalli = []
    for i, f in ROM:
        inizio = i
        stato = eseguito[i]
        for a in range(i + 1, f + 2):
            s = eseguito[a] if a <= f else None
            if s != stato:
                intervalli.append((inizio, a - 1, 'codice' if stato else 'dati'))
                inizio, stato = a, s

    # tabelle dati stimate: operandi a 16 bit (non destinazioni di salto)
    # che cadono su byte di ROM mai eseguiti
    tabelle = set()
    for pc, t in testo.items():
        mn = t.split(None, 1)[0]
        if mn in ('jp', 'jr', 'call', 'djnz', 'rst'):
            continue
        for x in OPERANDO16.findall(t):
            a = int(x, 16)
            if in_rom(a) and not eseguito[a]:
                tabelle.add(a)

    os.makedirs('out', exist_ok=True)
    pre = arg.prefisso + '_' if arg.prefisso else ''

    def uscita(nome):
        percorso = os.path.join('out', pre + nome)
        os.makedirs(os.path.dirname(percorso), exist_ok=True)
        return percorso
    with open(uscita('eseguiti.txt'), 'w') as f:
        for pc in sorted(conteggio):
            f.write('%04X\n' % pc)
    with open(uscita('istruzioni.txt'), 'w') as f:
        f.write('# pc\tesecuzioni\tistruzione\n')
        for pc in sorted(conteggio):
            f.write('%04X\t%d\t%s\n' % (pc, conteggio[pc], testo[pc]))
    with open(uscita('chiamate.txt'), 'w') as f:
        f.write('# sito\tdestinazione\tnumero\n')
        for (s, d), n in sorted(chiamate.items()):
            f.write('%04X\t%04X\t%d\n' % (s, d, n))
    with open(uscita('copertura.txt'), 'w') as f:
        f.write('# intervalli contigui inizio-fine codice|dati (%s)\n'
                % ', '.join('%04x-%04x' % z for z in ROM))
        for i, fi, s in intervalli:
            f.write('%04x-%04x %s\n' % (i, fi, s))
    dest_vere = {d for (_, d) in chiamate}
    with open(uscita('numeri.txt'), 'w') as f:
        def scrivi(nome, valore):
            f.write('%s: %s\n' % (nome, valore))
        scrivi('righe di traccia', righe)
        scrivi('byte eseguiti', byte_eseguiti)
        scrivi('byte mai eseguiti (su %d di ROM)' % totale_rom, totale_rom - byte_eseguiti)
        scrivi('PC distinti', len(conteggio))
        scrivi('destinazioni call/rst distinte (PC dopo un call/rst)', len(destinazioni))
        scrivi('destinazioni call/rst distinte (operando, chiamata avvenuta)', len(dest_vere))
        scrivi('ingressi NMI 0066', nmi)
        scrivi('rst $20 eseguiti (tabelle inline)', rst20)
        scrivi('jp (hl) eseguiti', jp_hl)
        scrivi('jp (ix)/(iy) eseguiti', jp_ixiy)
        scrivi(ram_etichetta, pc_ram)
        scrivi('tabelle dati stimate', len(tabelle))
        scrivi('intervalli codice', sum(1 for x in intervalli if x[2] == 'codice'))
        scrivi('intervalli dati', sum(1 for x in intervalli if x[2] == 'dati'))
        scrivi('violazioni autoverifica lunghezza', len(violazioni))
    for v in violazioni[:10]:
        if v:
            print('violazione: %04X -> %04X' % v, file=sys.stderr)
    if arg.cancella:
        os.remove(arg.traccia)
        print('traccia cancellata')


if __name__ == '__main__':
    principale()
