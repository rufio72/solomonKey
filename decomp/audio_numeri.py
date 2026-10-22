#!/usr/bin/env python3
"""Numeri del rapporto AUDIO.md dai derivati in out/audio/.

Uso (da decomp/, dopo sh audio.sh): python3 audio_numeri.py
Legge out/audio/*_istruzioni.txt, *_chiamate.txt, *_ram.txt, *_ay.txt e
stampa: byte della ROM audio eseguiti e no, intervalli codice/dati, routine
(destinazioni di call/rst eseguite più 0038 e 0066), PC distinti, rst usati,
RAM scritta, codici 00-40 che producono cambi AY, codici «lunghi».
Non stampa byte della ROM, solo indirizzi e conteggi.
"""
import glob
import os
import re

from coverage import lunghezza

CARTELLA = 'out/audio'
ROM = '../roms/ext/solomon/1.3jk'
PASSO = 90          # frame per codice nelle corse cod_*
RST = re.compile(r'^rst\s+\$([0-9A-F]{2})')


def file_di(suffisso):
    """File out/audio/*_<suffisso> (esclusa l'unione di copertura_call.py)."""
    return sorted(f for f in glob.glob(os.path.join(CARTELLA, '*_' + suffisso))
                  if 'unione' not in os.path.basename(f))


def righe_dati(nome):
    with open(nome) as f:
        for r in f:
            if r.strip() and not r.startswith('#'):
                yield r.rstrip('\n').split('\t')


def intervalli(segni, da, a):
    """Intervalli contigui (inizio, fine, valore) di segni[da..a]."""
    out = []
    inizio, stato = da, segni[da]
    for x in range(da + 1, a + 2):
        s = segni[x] if x <= a else None
        if s != stato:
            out.append((inizio, x - 1, stato))
            inizio, stato = x, s
    return out


def cambi_per_codice(nome):
    """Codice -> (frame del codice, numero di cambi AY, frame dei cambi)."""
    cod = {}
    corrente = None
    with open(nome) as f:
        for r in f:
            if r.startswith('# codice '):
                _, _, fr, cc = r.split()
                corrente = int(cc, 16)
                cod[corrente] = (int(fr), [])
            elif not r.startswith('#') and corrente is not None and r.strip():
                cod[corrente][1].append(int(r.split('\t')[0]))
    return cod


def principale():
    rom = open(ROM, 'rb').read()
    assert len(rom) == 0x4000
    rom = rom + bytes(0x10000 - len(rom))

    # PC eseguiti e conteggi, testo delle istruzioni
    pc_tot = {}
    testo = {}
    for nome in file_di('istruzioni.txt'):
        for pc, n, t in righe_dati(nome):
            p = int(pc, 16)
            pc_tot[p] = pc_tot.get(p, 0) + int(n)
            testo[p] = t
    eseguito = bytearray(0x10000)
    for pc in pc_tot:
        for i in range(lunghezza(rom, pc)):
            eseguito[(pc + i) & 0xFFFF] = 1
    byte_es = sum(eseguito[:0x4000])
    print('file di istruzioni letti: %d' % len(file_di('istruzioni.txt')))
    print('PC distinti eseguiti: %d (max %04X, PC >= 4000: %d)'
          % (len(pc_tot), max(pc_tot), sum(1 for p in pc_tot if p >= 0x4000)))
    print('byte di ROM eseguiti: %d su 16384 (%.1f%%), mai eseguiti: %d'
          % (byte_es, 100.0 * byte_es / 0x4000, 0x4000 - byte_es))
    ultimo = max(a for a in range(0x4000) if eseguito[a])
    print('ultimo byte eseguito: %04X; byte eseguiti oltre 0AF3: %d'
          % (ultimo, sum(eseguito[0x0AF3:0x4000])))
    iv = intervalli(eseguito, 0, 0x3FFF)
    cod = [(i, f) for i, f, s in iv if s]
    print('intervalli codice: %d, intervalli dati: %d'
          % (len(cod), sum(1 for x in iv if not x[2])))
    print('intervalli codice: ' + ' '.join('%04x-%04x' % x for x in cod))
    # riempimento in fondo alla ROM: byte uguali all'ultimo (solo indirizzo)
    fondo = 0x3FFF
    while fondo > 0 and rom[fondo - 1] == rom[0x3FFF]:
        fondo -= 1
    print('riempimento uniforme in fondo alla ROM: %04X-3FFF (%d byte)'
          % (fondo, 0x4000 - fondo))

    # routine: destinazioni di call/rst eseguite più IRQ e NMI
    dest = {}
    for nome in file_di('chiamate.txt'):
        for s, d, n in righe_dati(nome):
            d = int(d, 16)
            dest[d] = dest.get(d, 0) + int(n)
    routine = set(dest) | {0x0038, 0x0066}
    print('destinazioni call/rst eseguite: %d; routine (con 0038 e 0066): %d'
          % (len(dest), len(routine)))
    print('routine: ' + ' '.join('%04X' % d for d in sorted(routine)))
    rst = {}
    for pc, t in testo.items():
        m = RST.match(t)
        if m:
            rst[m.group(1)] = rst.get(m.group(1), 0) + pc_tot[pc]
    print('rst usati (esecuzioni): ' + ' '.join('%s=%d' % kv for kv in sorted(rst.items())))
    jp_hl = sum(pc_tot[p] for p, t in testo.items() if t.replace(' ', '') == 'jp(hl)')
    print('jp (hl) eseguiti: %d' % jp_hl)
    # gestori del jp (hl) a 0216: tabella di 16 parole a 03CE, indice =
    # byte di sequenza F0-FF & 0F; esecuzioni = conteggio del primo PC
    gestori = [rom[0x03CE + 2 * i] | (rom[0x03CF + 2 * i] << 8) for i in range(16)]
    esec = [pc_tot.get(g, 0) for g in gestori]
    print('gestori 0216: ' + ' '.join('F%X=%04X:%d' % (i, g, n)
                                      for i, (g, n) in enumerate(zip(gestori, esec)))
          + ' somma %d' % sum(esec))

    # RAM audio scritta dal frame 110
    ram = {}
    for nome in file_di('ram.txt'):
        for a, n, _, _ in righe_dati(nome):
            a = int(a, 16)
            ram[a] = ram.get(a, 0) + int(n)
    segni = bytearray(0x4800)
    for a in ram:
        segni[a] = 1
    zone = [(i, f) for i, f, s in intervalli(segni, 0x4000, 0x47FF) if s]
    print('RAM audio scritta (byte distinti, dal frame 110): %d in %d zone'
          % (len(ram), len(zone)))
    print('zone RAM scritte: ' + ' '.join('%04x-%04x' % z for z in zone))
    piu = sorted(ram.items(), key=lambda kv: -kv[1])[:10]
    print('RAM più scritta: ' + ' '.join('%04X=%d' % kv for kv in piu))

    # codici 00-40 e cambi AY nelle corse cod_*
    tutti = {}
    for nome in file_di('ay.txt'):
        b = os.path.basename(nome)
        if b.startswith('cod_') and not b.startswith('cod_41_ff'):
            tutti.update(cambi_per_codice(nome))
    con = {c: len(v[1]) for c, v in tutti.items() if v[1]}
    print('codici 00-40 osservati: %d; con cambi AY: %d; senza: %s'
          % (len(tutti), len(con),
             ' '.join('%02X' % c for c in sorted(tutti) if c not in con) or 'nessuno'))
    print('cambi AY per codice: ' + ' '.join('%02X=%d' % kv for kv in sorted(con.items())))
    lunghi = sorted(c for c, (f0, fr) in tutti.items()
                    if any(f0 + PASSO - 10 <= x < f0 + PASSO for x in fr))
    print('codici lunghi (cambi negli ultimi 10 di %d frame): %d: %s'
          % (PASSO, len(lunghi), ' '.join('%02X' % c for c in lunghi)))
    # durata nelle corse lungo_<cc>: ultimo cambio AY dopo il codice
    durate = []
    for nome in file_di('ay.txt'):
        b = os.path.basename(nome)
        if b.startswith('lungo_'):
            for c, (f0, fr) in cambi_per_codice(nome).items():
                durate.append((c, (max(fr) - f0) if fr else 0, len(fr)))
    if durate:
        print('corse lunghe (codice=frame dell\'ultimo cambio dopo il codice/cambi): '
              + ' '.join('%02X=%d/%d' % d for d in sorted(durate)))
        print('ancora attivi a fine corsa lunga (cambi negli ultimi 10 di 900): %s'
              % (' '.join('%02X' % c for c, d, _ in sorted(durate) if d >= 890) or 'nessuno'))
    ign = file_di('ay.txt')
    ign = [n for n in ign if os.path.basename(n).startswith('cod_41_ff')]
    for nome in ign:
        n = sum(len(v[1]) for v in cambi_per_codice(nome).values())
        print('cambi AY con i codici 41-FF: %d' % n)


if __name__ == '__main__':
    principale()
