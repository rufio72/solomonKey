#!/usr/bin/env python3
"""Censimento del motore dei nemici dietro il corpo 0A (ingresso 4DA2).

Il gestore a 0x0038 equivale a `jp tabella[E]`, con la tabella di parole
little-endian subito dopo il byte FF (ret del gestore a 0042). Lo script:
a) cammina staticamente sulla ROM dal 4DA2: caduta, jp/jr/djnz/call statici,
   rst 20h (voci da out/rst20.txt) e rst 38h (voci calcolate qui). Si ferma
   sulle routine già tradotte (frontiera: regex su harness/routine.h più le
   statiche di harness/attivita.c) e su jp (hl)/(ix)/(iy), segnalati come
   punti aperti. I siti rst 38h si scoprono così, non col grep;
b) per ogni sito rst 38h scoperto o dinamico stima le voci con
   voci_euristiche (due passate, come rst20.py) e con il limite dedotto
   dall'indice in E (`ld e,n`, oppure `ld e,a` con `and m` o `cp n; jr nc`
   prima); dalla traccia legge l'indice massimo osservato;
c) spezza il cammino in routine: ingressi = 4DA2, destinazioni di call e
   voci di tabella; ogni routine sono i PC raggiunti dal suo ingresso senza
   attraversarne un altro (le code condivise diventano ingressi interni).

Uscite: out/rst38.txt (formato di out/rst20.txt), harness/rst38_tables.h,
out/censimento_nemici.txt, out/rst38_dinamici.txt (cache della traccia).

Uso (da decomp/):
  python3 censimento_nemici.py --traccia out/replay_gioco_trace.txt
  python3 censimento_nemici.py      # senza traccia: usa la cache e
                                    # out/replay_gioco_eseguiti.txt
"""
import argparse
import os
import re

from coverage import lunghezza
from rst20 import voci_euristiche

INGRESSO = 0x4DA2
SITO_RST38 = 0xFF
SITO_RST20 = 0xE7
RET_GESTORE38 = 0x0042
NMI = 0x0066
SALTO_COND = {0xC2, 0xCA, 0xD2, 0xDA, 0xE2, 0xEA, 0xF2, 0xFA}
CALL_COND = {0xC4, 0xCC, 0xD4, 0xDC, 0xE4, 0xEC, 0xF4, 0xFC}
RET_COND = {0xC0, 0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8}
JR_COND = {0x20, 0x28, 0x30, 0x38, 0x10}
RST_CHIAMATA = {0xC7: 0x00, 0xCF: 0x08, 0xD7: 0x10, 0xDF: 0x18, 0xEF: 0x28, 0xF7: 0x30}


def frontiera(routine_h, attivita_c):
    """Indirizzi delle routine già tradotte."""
    with open(routine_h) as f:
        fr = {int(x, 16) for x in re.findall(r'sub_([0-9a-f]{4})', f.read())}
    with open(attivita_c) as f:
        fr |= {int(x, 16) for x in
               re.findall(r'^static\s+\w+\s+sub_([0-9a-f]{4})\(', f.read(), re.M)}
    return fr


def relativo(pc, b):
    return (pc + 2 + (b - 256 if b > 127 else b)) & 0xFFFF


def flusso(rom, pc):
    """(lunghezza, tipo, salti, caduta, chiamata) dell'istruzione a pc.
    tipo: normale, ret, aperto (jp (hl)/(ix)/(iy)), rst20, rst38."""
    n = lunghezza(rom, pc)
    b0, b1 = rom[pc], rom[(pc + 1) & 0xFFFF]
    nn = b1 | (rom[(pc + 2) & 0xFFFF] << 8)
    if b0 == 0xC3:
        return n, 'normale', [nn], False, None
    if b0 == 0x21 and rom[(pc + 3) & 0xFFFF] == 0xE5:
        # `ld hl,nn; push hl` prima di un salto in tabella (4FA8, 6F18): nn è
        # il ritorno della voce chiamata, quindi codice della stessa routine
        return n, 'normale', [nn], True, None
    if b0 in SALTO_COND:
        return n, 'normale', [nn], True, None
    if b0 == 0x18:
        return n, 'normale', [relativo(pc, b1)], False, None
    if b0 in JR_COND:
        return n, 'normale', [relativo(pc, b1)], True, None
    if b0 == 0xCD or b0 in CALL_COND:
        return n, 'normale', [], True, nn
    if b0 in RST_CHIAMATA:
        return n, 'normale', [], True, RST_CHIAMATA[b0]
    if b0 == 0xC9:
        return n, 'ret', [], False, None
    if b0 in RET_COND:
        return n, 'normale', [], True, None
    if b0 == 0xE9 or (b0 in (0xDD, 0xFD) and b1 == 0xE9):
        return n, 'aperto', [], False, None
    if b0 == 0xED and b1 in (0x45, 0x4D, 0x55, 0x5D, 0x65, 0x6D, 0x75, 0x7D):
        return n, 'ret', [], False, None
    if b0 == SITO_RST20:
        return n, 'rst20', [], False, None
    if b0 == SITO_RST38:
        return n, 'rst38', [], False, None
    return n, 'normale', [], True, None


# opcode senza prefisso che scrivono E o A (per il limite dell'indice)
SCRIVE_E = set(range(0x58, 0x60)) | {0x1E, 0x1C, 0x1D, 0x11, 0xD1, 0xEB, 0xD9}
SCRIVE_A = (set(range(0x78, 0x80)) | set(range(0x80, 0xB8)) |
            {0x3E, 0x0A, 0x1A, 0x3A, 0x3C, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F,
             0xF1, 0x08, 0xDB, 0xC6, 0xCE, 0xD6, 0xDE, 0xEE, 0xF6, 0xED})


# routine che conservano A ed E (0028: push af/bc/hl, call 0043, pop): il
# limite si deduce anche attraverso la loro chiamata (es. 4FA4 prima di 4FAC)
PRESERVA = {0x0028}


def limite38(rom, sito, pred):
    """Limite alle voci dedotto dall'indice in E prima del sito, o None.
    pred: pc -> lista dei predecessori statici (caduta o salto)."""
    pc = sito
    registro = 'e'
    correzione = 0
    for _ in range(12):
        p = pred.get(pc, [])
        if len(p) != 1:
            return None
        pc = p[0]
        b0, b1 = rom[pc], rom[(pc + 1) & 0xFFFF]
        if b0 == 0xCD and b1 | (rom[(pc + 2) & 0xFFFF] << 8) in PRESERVA:
            continue
        if b0 in RST_CHIAMATA and RST_CHIAMATA[b0] in PRESERVA:
            continue
        if b0 in (0xCB, 0xDD, 0xFD, 0xCD) or b0 in CALL_COND or b0 in RST_CHIAMATA:
            return None
        if registro == 'e':
            if b0 == 0x1E:
                return b1 + 1
            if b0 == 0x5F:
                registro = 'a'
                continue
            if b0 in SCRIVE_E or b0 == 0xED:
                return None
            continue
        if b0 == 0x3D:
            correzione -= 1
        elif b0 == 0xFE:
            succ = pc + 2
            if rom[succ] == 0x30 or rom[succ] == 0xD2:
                return b1 + correzione
            return None
        elif b0 == 0xE6:
            return b1 + 1 + correzione
        elif b0 == 0x3E:
            return b1 + 1 + correzione
        elif b0 in SCRIVE_A or 0xB8 <= b0 <= 0xBF:
            return None
    return None


def leggi_rst20(nome):
    tab = {}
    with open(nome) as f:
        for r in f:
            c = r.split()
            if len(c) >= 4:
                tab[int(c[0], 16)] = [int(x, 16) for x in c[4:]]
    return tab


def cammina(rom, fr, tab20_fn, tab38_fn):
    """Cammino statico dal 4DA2. Restituisce un dizionario con i PC visitati
    e i loro dati di flusso, i siti rst e le destinazioni."""
    visti = {}
    siti38, siti20 = {}, {}
    aperti, fuori_rom = [], []
    pila = [INGRESSO]
    while pila:
        pc = pila.pop()
        if pc in visti or pc in fr:
            continue
        if pc >= 0xC000:
            fuori_rom.append(pc)
            continue
        info = flusso(rom, pc)
        n, tipo, salti, caduta, chiamata = info
        tabella = []
        if tipo == 'rst38':
            tabella = tab38_fn(pc)
            siti38[pc] = tabella
        elif tipo == 'rst20':
            tabella = tab20_fn(pc)
            siti20[pc] = tabella
        elif tipo == 'aperto':
            aperti.append(pc)
        visti[pc] = (n, tipo, salti, caduta, chiamata, tabella)
        succ = list(salti) + list(tabella)
        if caduta:
            succ.append((pc + n) & 0xFFFF)
        if chiamata is not None:
            succ.append(chiamata)
        pila.extend(s for s in succ if s not in visti)
    return visti, siti38, siti20, aperti, fuori_rom


def operando_dati(rom, pc):
    """Operando di `ld bc/de/hl,nn` o `ld ix/iy,nn` in ROM: indirizzo di dati
    o di codice, comunque la fine di una tabella in linea che lo precede
    (6F3D, dati letti da 6ECF, chiude la tabella di 6F1C)."""
    b0 = rom[pc]
    if b0 in (0x01, 0x11, 0x21):
        nn = rom[pc + 1] | (rom[pc + 2] << 8)
    elif b0 in (0xDD, 0xFD) and rom[pc + 1] == 0x21:
        nn = rom[pc + 2] | (rom[pc + 3] << 8)
    else:
        return None
    return nn if nn < 0xC000 else None


def predecessori(visti):
    pred = {}
    for pc, (n, tipo, salti, caduta, chiamata, tab) in visti.items():
        for s in salti:
            pred.setdefault(s, []).append(pc)
        if caduta:
            pred.setdefault((pc + n) & 0xFFFF, []).append(pc)
    return pred


def leggi_traccia(nome):
    """(dinamici, eseguiti): dinamici = {sito: {dest: conteggio}} dei rst 38,
    con la pila per gli annidamenti come rst20.leggi_traccia."""
    dinamici = {}
    eseguiti = set()
    pila = []
    in_attesa = None
    with open(nome, buffering=1 << 20) as f:
        for r in f:
            if len(r) < 6 or r[4:6] != ': ':
                continue
            try:
                pc = int(r[:4], 16)
            except ValueError:
                continue
            eseguiti.add(pc)
            if in_attesa is not None:
                if pc != NMI:
                    d = dinamici.setdefault(in_attesa, {})
                    d[pc] = d.get(pc, 0) + 1
                in_attesa = None
            if r.startswith('rst  $38', 6):
                pila.append(pc)
            elif pc == RET_GESTORE38 and pila:
                in_attesa = pila.pop()
    return dinamici, eseguiti


def regioni(visti, ingressi, fr):
    """Assegna ogni PC visitato a un ingresso; le code condivise da più
    ingressi diventano ingressi interni. Restituisce (ingressi, proprietario)."""
    ingressi = set(ingressi)
    pred = predecessori(visti)
    while True:
        reclami = {}
        for e in ingressi:
            pila = [e]
            while pila:
                pc = pila.pop()
                if pc not in visti or pc in fr:
                    continue
                if pc != e and pc in ingressi:
                    continue
                s = reclami.setdefault(pc, set())
                if e in s:
                    continue
                s.add(e)
                n, tipo, salti, caduta, chiamata, tab = visti[pc]
                pila.extend(salti)
                if caduta:
                    pila.append((pc + n) & 0xFFFF)
        nuovi = set()
        for pc, s in reclami.items():
            if len(s) > 1 and any(reclami.get(p) != s for p in pred.get(pc, [])):
                nuovi.add(pc)
        if not nuovi:
            return ingressi, {pc: next(iter(s)) for pc, s in reclami.items()}
        ingressi |= nuovi


# ---------------------------------------------------------------------------
# Piano dei lotti e dei moduli C (CENSIMENTO-NEMICI.md, sezione «Piano»)

SMISTATORE_TIPI = 0x4F1B      # rst 38 sul tipo di entità: E = (iy+03) - D
RIGHE_PER_BYTE = 0.6          # metro di attivita.c (7 corpi, ~600 B di Z80)
SOGLIA_MODULO = 700           # righe C stimate per modulo (margine sulle 800)
SOGLIA_N1 = 1200              # oltre, N1 si spezza in N1a/N1b (risoluzione 3)
# routine del perimetro dei corpi 01 e 10 (REPLAY.md, «Perimetro misurato»)
COMUNI_01_10 = {0x2474, 0x2482, 0x248D, 0x24BE, 0x4592, 0x4938, 0x49AE,
                0x4ADA, 0x4B20, 0x4C11}
# moduli: (file, tema, gruppi); gruppo = 'corpo', 'comuni' o 't<indice di 4F1B>'
# con suffisso a/b se il tipo da solo supera la soglia
MODULI = [
    ('nemici.c', 'corpo 0A (4DA2), smistatori 4DA6 e 4F1B, ramo 4632, voce nulla 3B62',
     ['corpo']),
    ('nemici_comuni.c', 'routine condivise con i corpi 01 e 10', ['comuni']),
    ('nemici_comparsa.c', 'tipi 0, 4 e 5 di 4F1B; 4FC5 (comparsa) condivisa da 0 e 4',
     ['t0', 't4', 't5']),
    ('nemici_tipo06a.c', 'tipo 6 di 4F1B (4FD4), prima metà per indirizzo', ['t6a']),
    ('nemici_tipo06b.c', 'tipo 6 di 4F1B (4FD4), seconda metà', ['t6b']),
    ('nemici_tipo07a.c', 'tipo 7 di 4F1B (5CFB), prima metà per indirizzo', ['t7a']),
    ('nemici_tipo07b.c', 'tipo 7 di 4F1B (5CFB), seconda metà', ['t7b']),
    ('nemici_tipi_08_14.c', 'tipi 8 (6ECA) e 14 (6CE8) di 4F1B', ['t8', 't14']),
    ('nemici_tipo09.c', 'tipo 9 di 4F1B (57CD)', ['t9']),
    ('nemici_tipi_10_13.c', 'tipi 10-13 di 4F1B (714B, 56E9, 55B2, 54D8)',
     ['t10', 't11', 't12', 't13']),
    ('nemici_tipi_15_17.c', 'tipi 15-17 di 4F1B (6C0B, 6B43, 7F68)', ['t15', 't16', 't17']),
]


def righe_c(byte):
    return int(round(byte * RIGHE_PER_BYTE))


def chiusura(routine, semi, stop):
    """Routine raggiunte dai semi lungo le chiamate, senza entrare in stop."""
    pila, visti = list(semi), set()
    while pila:
        x = pila.pop()
        if x in visti or x in stop or x not in routine:
            continue
        visti.add(x)
        pila.extend(routine[x]['chiamate'])
    return visti


def gruppi(routine, voci_tipi):
    """Gruppo di ogni routine: comuni (01/10), corpo (dal 4DA2 senza passare
    per le voci di 4F1B), poi tipo t = indice più basso di 4F1B che la raggiunge."""
    gruppo = {x: 'comuni' for x in COMUNI_01_10 if x in routine}
    for x in chiusura(routine, [INGRESSO], set(voci_tipi) | set(gruppo)):
        gruppo[x] = 'corpo'
    for x in set(voci_tipi):              # voci nulle (3B62: ret) e simili
        if x in routine and routine[x]['byte'] <= 1:
            gruppo[x] = 'corpo'
    for i, v in enumerate(voci_tipi):
        for x in chiusura(routine, [v], set(gruppo)):
            gruppo.setdefault(x, 't%d' % i)
    # un tipo oltre la soglia si spezza per indirizzo in due metà
    for g in sorted(set(gruppo.values())):
        membri = sorted(x for x in gruppo if gruppo[x] == g)
        tot = sum(routine[x]['byte'] for x in membri)
        if righe_c(tot) > SOGLIA_MODULO:
            somma = 0
            for x in membri:
                gruppo[x] = g + ('a' if somma < tot / 2 else 'b')
                somma += routine[x]['byte']
    return gruppo


def piano(routine, voci_tipi, fr):
    """Lotti di traduzione e moduli: scrive out/censimento_nemici_lotti.txt,
    stampa i controlli di coerenza e restituisce i lotti."""
    gruppo = gruppi(routine, voci_tipi)
    modulo = {}
    for nome, tema, gs in MODULI:
        for x, g in gruppo.items():
            if g in gs:
                modulo[x] = nome
    senza = sorted(x for x in routine if x not in modulo)
    for x in routine:
        routine[x]['gruppo'] = gruppo.get(x, '?')
        routine[x]['modulo'] = modulo.get(x, '?')
    n1 = sorted(x for x in routine if routine[x]['coperta'] != 'NO' or x == INGRESSO)
    lotti = []
    righe_n1 = righe_c(sum(routine[x]['byte'] for x in n1))
    if righe_n1 > SOGLIA_N1:
        meta = sum(routine[x]['byte'] for x in n1) / 2
        somma, a, b = 0, [], []
        for x in n1:
            (a if somma < meta else b).append(x)
            somma += routine[x]['byte']
        lotti += [('N1a', a), ('N1b', b)]
    else:
        lotti.append(('N1', n1))
    # N2+: un lotto per modulo con le routine non coperte, in ordine di
    # utilità = byte eseguiti nell'unione storica delle sessioni
    resto = []
    for nome, tema, gs in MODULI:
        xs = sorted(x for x in routine if modulo.get(x) == nome and x not in n1)
        if xs:
            util = sum(routine[x]['byte_unione'] for x in xs)
            resto.append((-util, nome, xs))
    for k, (util, nome, xs) in enumerate(sorted(resto)):
        lotti.append(('N%d' % (k + 2), xs))
    for nome, xs in lotti:
        for x in xs:
            routine[x]['lotto'] = nome
    # stub del lotto N1: routine non coperte chiamate o smistate da N1
    stub = sorted({c for x in n1 for c in routine[x]['chiamate']
                   if c in routine and c not in n1})

    with open('out/censimento_nemici_lotti.txt', 'w') as f:
        f.write('# lotto byte righe_stimate moduli ingressi\n')
        for nome, xs in lotti:
            b = sum(routine[x]['byte'] for x in xs)
            mods = sorted({modulo.get(x, '?') for x in xs})
            f.write('%s %d %d %s %s\n' % (nome, b, righe_c(b), ','.join(mods),
                                          ','.join('%04X' % x for x in xs)))
        f.write('# stub di N1 (routine mancanti segnalate dal replay): %s\n'
                % ','.join('%04X' % x for x in stub))
        f.write('# modulo byte righe_stimate\n')
        for nome, tema, gs in MODULI:
            b = sum(routine[x]['byte'] for x in routine if modulo.get(x) == nome)
            f.write('%s %d %d\n' % (nome, b, righe_c(b)))

    # controlli di coerenza (verifica 7 del brief 24)
    conti = {}
    for nome, xs in lotti:
        for x in xs:
            conti[x] = conti.get(x, 0) + 1
    una_volta = all(conti.get(x, 0) == 1 for x in routine)
    coperte = {x for x in routine if routine[x]['coperta'] != 'NO'} | {INGRESSO}
    n1_esatto = set(x for nome, xs in lotti if nome in ('N1', 'N1a', 'N1b') for x in xs) == coperte
    somma = sum(routine[x]['byte'] for nome, xs in lotti for x in xs)
    totale = sum(r['byte'] for r in routine.values())
    righe_mod = [righe_c(sum(routine[x]['byte'] for x in routine if modulo.get(x) == m))
                 for m, _, _ in MODULI]
    print('piano: %d lotti, %d moduli; routine in esattamente un lotto: %s; '
          'N1 = coperte + 4DA2: %s (%d routine, %d righe stimate); '
          'byte dei lotti %d = totale %d: %s; righe per modulo max %d <= %d: %s; '
          'routine senza modulo: %s' % (
              len(lotti), len(MODULI), 'SI' if una_volta else 'NO',
              'SI' if n1_esatto else 'NO', len(n1), righe_n1, somma, totale,
              'SI' if somma == totale else 'NO', max(righe_mod), SOGLIA_MODULO,
              'SI' if max(righe_mod) <= SOGLIA_MODULO else 'NO',
              ' '.join('%04X' % x for x in senza) or 'nessuna'))
    print('stub di N1: %d routine' % len(stub))
    return lotti, stub


# note fisse del censimento (osservazioni fuori dalla traccia del replay)
NOTE = {
    0x4FC5: 'osservata dal vivo: 6 DEST in partita_viva_20261027.log, 1 nel 27b; '
            'probabile comparsa di un\'entità (fata)',
    0x7F68: 'salta (jr z) a 7F39, sito rst 38 dentro sub_7f2d già tradotta',
    0x3B62: 'solo `ret`: voce nulla delle tabelle',
    0x4F0F: 'smistatore dei tipi 4F1B: E = (iy+03) - D',
}


def scrivi_markdown(nome, tabelle, routine, lotti_stub, fr):
    """Tabelle del censimento in markdown, da incollare in CENSIMENTO-NEMICI.md."""
    lotti, stub = lotti_stub
    with open(nome, 'w') as f:
        f.write('| sito | voci | indice max | stato | tabella |\n|---|---:|---:|---|---|\n')
        for s in sorted(tabelle):
            t = tabelle[s]
            f.write('| %04X | %d | %s | %s | %s |\n' % (s, len(t['voci']), t['indice_max'],
                                                       t['stato'],
                                                       ' '.join('%04X' % w for w in t['voci'])))
        f.write('\n| ingresso | estensione | byte | chiamanti | replay | unione | lotto | '
                'modulo | note |\n|---|---|---:|---|---|---|---|---|---|\n')
        for x in sorted(routine):
            r = routine[x]
            note = []
            if r['interno']:
                note.append('ingresso interno (coda condivisa)')
            if x in COMUNI_01_10:
                note.append('anche corpi 01/10')
            if x < 0x4000:
                note.append('fuori dalla zona 4xxx-8xxx')
            if x in NOTE:
                note.append(NOTE[x])
            f.write('| %04X | %04X-%04X | %d | %s | %s | %s | %s | %s | %s |\n' % (
                x, r['da'], r['a'], r['byte'],
                ' '.join('%04X' % c for c in sorted(r['chiamanti'])) or '-',
                r['coperta'], r['unione'], r['lotto'], r['modulo'].replace('.c', ''),
                '; '.join(note) or '-'))
        f.write('\n| lotto | routine | byte | righe C stimate | moduli |\n'
                '|---|---:|---:|---:|---|\n')
        for nome_l, xs in lotti:
            b = sum(routine[x]['byte'] for x in xs)
            f.write('| %s | %d | %d | %d | %s |\n' % (
                nome_l, len(xs), b, righe_c(b),
                ', '.join(sorted({routine[x]['modulo'] for x in xs}))))
        f.write('\n| modulo | tema | routine | byte | righe C stimate |\n'
                '|---|---|---:|---:|---:|\n')
        for m, tema, gs in MODULI:
            xs = [x for x in routine if routine[x]['modulo'] == m]
            b = sum(routine[x]['byte'] for x in xs)
            f.write('| `%s` | %s | %d | %d | %d |\n' % (m, tema, len(xs), b, righe_c(b)))
        f.write('\nStub di N1: %s\n' % ' '.join('%04X' % x for x in stub))


def principale():
    ap = argparse.ArgumentParser(description='Censimento del motore dei nemici (corpo 0A)')
    ap.add_argument('--rom', default='out/main64.bin')
    ap.add_argument('--traccia', default=None)
    ap.add_argument('--eseguiti', default='out/replay_gioco_eseguiti.txt')
    ap.add_argument('--cache', default='out/rst38_dinamici.txt')
    ap.add_argument('--rst20', default='out/rst20.txt')
    ap.add_argument('--intestazione', default='harness/rst38_tables.h')
    ap.add_argument('--unione', default='out/eseguiti_unione.txt',
                    help='PC eseguiti in tutte le sessioni storiche (utilità dei lotti)')
    arg = ap.parse_args()

    with open(arg.rom, 'rb') as f:
        rom = f.read()
    fr = frontiera('harness/routine.h', 'harness/attivita.c')
    tab20 = leggi_rst20(arg.rst20)

    # traccia (o cache): siti dinamici ed eseguiti
    if arg.traccia:
        dinamici, eseguiti = leggi_traccia(arg.traccia)
        with open(arg.cache, 'w') as f:
            for s in sorted(dinamici):
                for d, c in sorted(dinamici[s].items()):
                    f.write('%04X %04X %d\n' % (s, d, c))
    else:
        dinamici = {}
        if os.path.exists(arg.cache):
            with open(arg.cache) as f:
                for r in f:
                    s, d, c = r.split()
                    dinamici.setdefault(int(s, 16), {})[int(d, 16)] = int(c)
        eseguiti = set()
        if os.path.exists(arg.eseguiti):
            with open(arg.eseguiti) as f:
                eseguiti = {int(r, 16) for r in f if r.strip()}

    # cammino iterato: tabelle troncate dalle voci note (seconda passata di
    # rst20.py) e dal limite dell'indice, finché il cammino non cambia
    ingressi_noti = frozenset()
    pred = {}
    limiti = {}
    tronche20 = {}
    prec = None
    for passata in range(8):
        def tab20_fn(sito, ingr=ingressi_noti):
            # voci di out/rst20.txt, troncate dove l'euristica con le
            # destinazioni note del cammino si ferma prima (6F1C: 16 su 22)
            euristica = voci_euristiche(rom, sito, ingr)
            voci = tab20.get(sito)
            if voci is None:
                return euristica
            if len(euristica) < len(voci):
                tronche20[sito] = (len(voci), len(euristica))
                return voci[:len(euristica)]
            return voci

        def tab38(sito, ingr=ingressi_noti, pr=pred):
            voci = voci_euristiche(rom, sito, ingr)
            lim = limite38(rom, sito, pr)
            limiti[sito] = lim
            return voci if lim is None else voci[:lim]
        tronche20.clear()
        visti, siti38, siti20, aperti, fuori_rom = cammina(rom, fr, tab20_fn, tab38)
        pred = predecessori(visti)
        # destinazioni note: voci di tabella e bersagli di salti e call (una
        # tabella finisce dove inizia codice noto, es. 4FB9 dopo quella di 4FAC)
        ingressi_noti = frozenset(
            [w for v in list(siti38.values()) + list(siti20.values()) for w in v] +
            [d for (n, t, salti, c, chiamata, tab) in visti.values()
             for d in salti + ([chiamata] if chiamata is not None else [])] +
            [operando_dati(rom, pc) for pc in visti if operando_dati(rom, pc) is not None])
        firma = (frozenset(visti), tuple(sorted((s, tuple(v)) for s, v in siti38.items())))
        if firma == prec:
            break
        prec = firma
    # limite finale con i predecessori dell'ultimo cammino
    for s in siti38:
        limiti[s] = limite38(rom, s, pred)

    # tabelle anche per i siti dinamici fuori dal cammino
    tabelle = {}
    for s, v in siti38.items():
        tabelle[s] = {'voci': v, 'stato': 'eseguito' if s in dinamici else 'statico',
                      'limite': limiti.get(s)}
    for s in dinamici:
        if s not in tabelle:
            tabelle[s] = {'voci': voci_euristiche(rom, s, ingressi_noti),
                          'stato': 'fuori-cammino', 'limite': None}
    fuori = 0
    for s, t in tabelle.items():
        t['indice_max'] = '-'
        if s in dinamici:
            massimo = -1
            for d in dinamici[s]:
                if d in t['voci']:
                    massimo = max(massimo, t['voci'].index(d))
                else:
                    fuori += 1
                    print('sito %04X: destinazione %04X fuori tabella' % (s, d))
            t['indice_max'] = str(massimo)
    print('siti rst 38 nel cammino: %d, dinamici: %d (fuori dal cammino: %d); '
          'destinazioni fuori tabella: %d' % (
              len(siti38), len(dinamici), sum(1 for s in dinamici if s not in siti38), fuori))

    with open('out/rst38.txt', 'w') as f:
        for s in sorted(tabelle):
            t = tabelle[s]
            f.write('%04X %d %s %s %s\n' % (s, len(t['voci']), t['stato'], t['indice_max'],
                                             ' '.join('%04X' % w for w in t['voci'])))
    with open(arg.intestazione, 'w') as f:
        f.write('/* Generato da censimento_nemici.py: non modificare a mano.\n'
                ' * Tabelle inline di rst $38 (indice in E): rst38_XXXX[i] =\n'
                ' * destinazione per E = i. */\n'
                '#ifndef RST38_TABLES_H\n#define RST38_TABLES_H\n\n#include <stdint.h>\n\n')
        for s in sorted(tabelle):
            t = tabelle[s]
            if not t['voci']:
                continue
            f.write('/* %04X: %s, %d voci, indice massimo osservato %s */\n'
                    % (s, t['stato'], len(t['voci']), t['indice_max']))
            f.write('static const uint16_t rst38_%04X[] = {\n' % s)
            for j in range(0, len(t['voci']), 8):
                f.write('    ' + ', '.join('0x%04X' % w for w in t['voci'][j:j + 8]) + ',\n')
            f.write('};\n\n')
        f.write('#endif\n')

    # routine
    ingressi = {INGRESSO}
    for pc, (n, tipo, salti, caduta, chiamata, tab) in visti.items():
        if chiamata is not None:
            ingressi.add(chiamata)
        ingressi.update(tab)
    ingressi = {e for e in ingressi if e in visti and e not in fr}
    base = set(ingressi)
    ingressi, padrone = regioni(visti, ingressi, fr)
    pcs = {e: [] for e in ingressi}
    for pc, e in padrone.items():
        pcs[e].append(pc)
    chiamate = {e: set() for e in ingressi}
    for pc, e in padrone.items():
        n, tipo, salti, caduta, chiamata, tab = visti[pc]
        dest = list(salti) + list(tab) + ([chiamata] if chiamata is not None else [])
        if caduta:
            dest.append((pc + n) & 0xFFFF)
        for d in dest:
            if d in fr:
                chiamate[e].add(d)
            elif d in ingressi and d != e:
                chiamate[e].add(d)
    chiamanti = {e: set() for e in ingressi}
    for e, cs in chiamate.items():
        for c in cs:
            if c in chiamanti:
                chiamanti[c].add(e)
    unione = set()
    if os.path.exists(arg.unione):
        with open(arg.unione) as f:
            unione = {int(r, 16) for r in f if r.strip()}

    def copertura(lista, insieme):
        fatti = sum(1 for p in lista if p in insieme)
        return 'SI' if fatti == len(lista) else ('NO' if fatti == 0 else 'PARZIALE')
    routine = {}
    totale = 0
    coperte = {'SI': 0, 'PARZIALE': 0, 'NO': 0}
    with open('out/censimento_nemici.txt', 'w') as f:
        f.write('# ingresso estensione byte coperta chiamate chiamanti '
                '(chiamate: (t) = già tradotta; ingresso con * = interno, coda condivisa)\n')
        for e in sorted(ingressi):
            lista = sorted(pcs[e])
            byte = sum(visti[p][0] for p in lista)
            fine = max(p + visti[p][0] - 1 for p in lista)
            coperta = copertura(lista, eseguiti)
            coperte[coperta] += 1
            routine[e] = {'da': min(lista), 'a': fine, 'byte': byte, 'coperta': coperta,
                          'unione': copertura(lista, unione), 'interno': e not in base,
                          'byte_unione': sum(visti[p][0] for p in lista if p in unione),
                          'chiamate': chiamate[e], 'chiamanti': chiamanti[e]}
            totale += byte
            cs = ','.join('%04X%s' % (c, '(t)' if c in fr else '') for c in sorted(chiamate[e])) or '-'
            ch = ','.join('%04X' % c for c in sorted(chiamanti[e])) or '-'
            f.write('%04X%s %04X-%04X %d %s %s %s\n' % (e, '' if e in base else '*', min(lista),
                                                       fine, byte, coperta, cs, ch))
    print('routine: %d (interne: %d), byte: %d, PC: %d; coperte SI %d, PARZIALE %d, NO %d' % (
        len(ingressi), len(ingressi - base), totale, len(visti),
        coperte['SI'], coperte['PARZIALE'], coperte['NO']))
    print('siti rst 20 nel cammino: %s' % (' '.join('%04X' % s for s in sorted(siti20)) or '-'))
    for s, (prima, dopo) in sorted(tronche20.items()):
        print('rst 20 %04X: %d voci in out/rst20.txt, %d dopo il troncamento' % (s, prima, dopo))
    print('punti aperti (jp (hl)/(ix)/(iy)): %s' % (' '.join('%04X' % s for s in sorted(aperti)) or '-'))
    if fuori_rom:
        print('destinazioni in RAM: %s' % ' '.join('%04X' % s for s in sorted(set(fuori_rom))))
    print('PC del cammino nell\'unione storica (%s): %d su %d' % (
        arg.unione, sum(1 for p in visti if p in unione), len(visti)))
    lotti = piano(routine, tabelle[SMISTATORE_TIPI]['voci'], fr)
    scrivi_markdown('out/censimento_nemici_tabelle.md', tabelle, routine, lotti, fr)
    print('scritti out/rst38.txt, %s, out/censimento_nemici.txt, '
          'out/censimento_nemici_lotti.txt, out/censimento_nemici_tabelle.md' % arg.intestazione)


if __name__ == '__main__':
    principale()
