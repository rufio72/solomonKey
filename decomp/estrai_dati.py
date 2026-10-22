#!/usr/bin/env python3
"""Estrae la regione dati della ROM audio 1.3jk (05C6-3FFF) in sorgente C.

Uso (da decomp/): python3 estrai_dati.py
Legge ../roms/ext/solomon/1.3jk (o ../../roms/..., come audio_piattaforma.c)
e scrive in harness/:
  audio_dati_indirizzi.h   un #define ADR_<nome> per ogni etichetta
  audio_dati_gruppi.h      dichiarazioni ed elenco dei gruppi di pezzi
  audio_dati_tabelle.c     riempimenti, toni, durate, inviluppi, tabella
                           dei comandi, residuo 3811
  audio_dati_spartiti_*.c  elenchi e flussi dei comandi 01-3F
e SPARTITI.md (descrizione funzionale dei comandi).

Ogni etichetta (inizio di tabella, elenco di un comando, flusso puntato da
un record, bersaglio di F4/F7) apre un pezzo che arriva all'etichetta
seguente. Prima di scrivere, il generatore espande le macro del testo C
prodotto (stessa semantica di harness/audio_dati.h) e controlla che diano
i byte della ROM; altrimenti esce con 2. Rieseguirlo non cambia nulla.
"""
import os
import re
import sys

QUI = os.path.dirname(os.path.abspath(__file__))
HARNESS = os.path.join(QUI, 'harness')
ROM_PERCORSI = ['../roms/ext/solomon/1.3jk', '../../roms/ext/solomon/1.3jk',
                os.path.join(QUI, '../roms/ext/solomon/1.3jk')]

INIZIO, FINE_ROM = 0x05C6, 0x4000
TAB_TONI, TAB_TONI_B, TAB_DURATE = 0x0700, 0x0760, 0x0790
TAB_INV, INV_FINE, TAB_CMD, FLUSSI = 0x0850, 0x0AF3, 0x0AF3, 0x0B73
RESIDUO, RIEMPI_FINE = 0x3811, 0x385F
RIGHE_MAX_FILE = 560     # righe per file di spartiti (limite del progetto 800)
LARGHEZZA = 96           # colonne massime di una riga di flusso

NOTE = ['DO', 'DOd', 'RE', 'REd', 'MI', 'FA', 'FAd', 'SOL', 'SOLd', 'LA', 'LAd', 'SI']
# lunghezza dei gestori senza argomento variabile (F0 a parte)
LUNG = {0xF1: 2, 0xF2: 2, 0xF3: 2, 0xF9: 2, 0xFD: 2, 0xF5: 2, 0xF4: 3, 0xF7: 3,
        0xFC: 3, 0xF6: 1, 0xF8: 1, 0xFA: 1, 0xFE: 1, 0xFF: 1, 0xFB: 7}
INTESTAZIONE = ('/* Generato da decomp/estrai_dati.py dalla ROM audio 1.3jk: non modificare\n'
                ' * a mano. Dati musicali derivati dal programma Tecmo (NOTICE.md). */\n')


def leggi_rom():
    for p in ROM_PERCORSI:
        if os.path.exists(p):
            with open(p, 'rb') as f:
                rom = f.read()
            if len(rom) != 0x4000:
                sys.exit('%s: lunghezza sbagliata (attesi 16384 byte)' % p)
            return rom
    sys.exit('1.3jk non trovato in ../roms/ext/solomon/ né in ../../roms/ext/solomon/')


def parola(rom, a):
    return rom[a] | rom[a + 1] << 8


def lung_istr(rom, a):
    """Byte occupati dall'unità di flusso che inizia ad a."""
    b = rom[a]
    if b < 0xF0:
        return 1
    if b == 0xF0:
        n = rom[a + 1]
        if n == 0xFF:
            return 5
        if n == 0xFE:
            return 4 if rom[a + 3] == 0xFF else 3
        return 2
    return LUNG[b]


def segnato(x):
    return x - 256 if x >= 0x80 else x


# ---------------------------------------------------------------- analisi

class Comando:
    def __init__(self, num, elenco, rom):
        self.num, self.elenco, self.record = num, elenco, []
        a = elenco
        while rom[a] != 0xFF:
            self.record.append((rom[a], parola(rom, a + 1)))
            a += 3
        self.fine_elenco = a + 1

    @property
    def sigla(self):
        return 'cmd_%02X' % self.num if self.num else 'residuo_%04X' % self.elenco


def comandi(rom):
    cmd = [Comando(n, parola(rom, TAB_CMD + 2 * n), rom) for n in range(1, 64)]
    return cmd + [Comando(0, RESIDUO, rom)]


def percorri(rom, inizio):
    """Byte raggiungibili da un ingresso: (insieme dei byte, unità, bersagli)."""
    visti, byte, bersagli, da_fare = set(), set(), set(), [inizio]
    while da_fare:
        a = da_fare.pop()
        while a not in visti:
            visti.add(a)
            n = lung_istr(rom, a)
            byte.update(range(a, a + n))
            b = rom[a]
            if b in (0xF4, 0xF7):
                bersagli.add(parola(rom, a + 1))
                da_fare.append(parola(rom, a + 1))
            if b in (0xF4, 0xF8, 0xFF):
                break
            a += n
    return byte, visti, bersagli


def etichette(rom, cmd):
    """Indirizzo -> nome di ogni ingresso (elenchi, flussi, bersagli)."""
    nomi = {}
    zona = sorted(c.elenco for c in cmd)

    def area(a):                      # comando nella cui zona cade a
        prima = max(z for z in zona if z <= a)
        return next(c for c in cmd if c.elenco == prima)

    for c in cmd:
        nomi[c.elenco] = c.sigla
    candidati = {}
    for c in cmd:
        for k, p in c.record:
            candidati.setdefault(p, []).append((c, k))
    usati = set()
    for p, usi in sorted(candidati.items()):
        c, k = next(((c, k) for c, k in usi if area(p) is c), usi[0])
        nome = base_nome = '%s_b%d_s%d' % (c.sigla, k, 1 if k < 9 else 2)
        giro = 1
        while nome in usati:          # stesso blocco in due record: _r2, _r3
            giro += 1
            nome = '%s_r%d' % (base_nome, giro)
        usati.add(nome)
        nomi[p] = nome
    interni = set()
    for c in cmd:
        for _, p in c.record:
            interni |= percorri(rom, p)[2]
    base = {}
    for a in sorted(interni - set(nomi)):
        padre = max(x for x in nomi if x < a and not re.search(r'_p\d+$', nomi[x]))
        base[padre] = base.get(padre, 1) + 1
        nomi[a] = '%s_p%d' % (nomi[padre], base[padre])
    return nomi


# ---------------------------------------------------------------- testo C

def adr(nomi, a):
    return 'ADR_%s' % nomi[a] if a in nomi else '0x%04X' % a


def unita(rom, a, fine, nomi):
    """(testo della macro, byte occupati) dell'unità ad a, entro fine."""
    b, n = rom[a], lung_istr(rom, a)
    if a + n > fine:
        return 'B(0x%02X)' % b, 1
    x = rom[a + 1] if n > 1 else 0
    if b < 0x80:
        if b & 15 >= 12:
            return ('PAUSA' if b == 0x0C else 'B(0x%02X)' % b), 1
        return 'NOTA(%d, %s)' % (b >> 4, NOTE[b & 15]), 1
    if b < 0xF0:
        return 'DUR(%d)' % (b & 0x7F), 1
    if b == 0xF0:
        if x == 0xFF:
            return 'INV_HW(0x%02X, 0x%04X)' % (rom[a + 2], parola(rom, a + 3)), 5
        if x == 0xFE:
            return ('INV_HW_FORMA' if n == 4 else 'INV_HW_NOTA') + '(0x%02X)' % rom[a + 2], n
        return 'STRUM(%d)' % x, 2
    semplici = {0xF1: 'ATTEN(%d)', 0xF3: 'SCOST(%d)', 0xF5: 'RIP_INIZIO(%d)',
                0xF9: 'RUMORE(%d)', 0xFD: 'COMANDO(0x%02X)'}
    if b in semplici:
        return semplici[b] % x, 2
    if b == 0xF2:
        return 'TRASP(%d)' % segnato(x), 2
    if b in (0xF4, 0xF7):
        return '%s(%s)' % ('SALTA' if b == 0xF4 else 'CHIAMA', adr(nomi, parola(rom, a + 1))), 3
    if b == 0xFB:
        return 'GLISSA(0x%04X, 0x%04X, %d)' % tuple(parola(rom, a + 1 + 2 * i) for i in range(3)), 7
    if b == 0xFC:
        return 'PERIODO(0x%04X)' % parola(rom, a + 1), 3
    return {0xF6: 'RIP_FINE', 0xF8: 'RITORNA', 0xFA: 'RUMORE_OFF', 0xFE: 'ATTEN_PIU',
            0xFF: 'FINE'}[b], 1


def impagina(voci, a_capo_dopo=()):
    """Righe di massimo LARGHEZZA colonne con le voci separate da virgole."""
    righe, riga = [], ''
    for v in voci:
        if riga and len('    ' + riga + v + ',') > LARGHEZZA:
            righe.append(riga.rstrip())
            riga = ''
        riga += v + ', '
        if v.split('(')[0] in a_capo_dopo:
            righe.append(riga.rstrip())
            riga = ''
    if riga:
        righe.append(riga.rstrip())
    return ['    ' + r for r in righe]


def corpo_flusso(rom, a, fine, nomi):
    """Righe del flusso da a a fine: dopo un'interruzione (F4, F8, FF) i
    byte non raggiunti escono come B() (separatori FF)."""
    voci, chiuso = [], False
    while a < fine:
        if chiuso:
            voci.append('B(0x%02X)' % rom[a])
            a += 1
            continue
        testo, n = unita(rom, a, fine, nomi)
        voci.append(testo)
        chiuso = rom[a] in (0xF4, 0xF8, 0xFF) and n == LUNG[rom[a]]
        a += n
    return impagina(voci, ('SALTA', 'RITORNA', 'FINE', 'RIP_FINE'))


def corpo_elenco(rom, c, fine, nomi):
    voci = ['REC(%d, %s)' % (k, adr(nomi, p)) for k, p in c.record] + ['FINE_ELENCO']
    voci += ['B(0x%02X)' % rom[a] for a in range(c.fine_elenco, fine)]
    return impagina(voci)


def corpo_parole(rom, a, n, per_riga=8):
    voci = ['W(0x%04X)' % parola(rom, a + 2 * i) for i in range(n)]
    return ['    ' + ', '.join(voci[i:i + per_riga]) + ',' for i in range(0, n, per_riga)]


def corpo_inviluppo(rom, a, fine):
    voci = []
    while a < fine:
        if rom[a] == 0xFF:
            voci.append('RIPARTI')
            a += 1
        elif a + 1 < fine:
            voci.append('PASSO(%d, %d)' % (rom[a], rom[a + 1]))
            a += 2
        else:
            voci.append('B(0x%02X)' % rom[a])
            a += 1
    return impagina(voci)


class Pezzo:
    def __init__(self, ind, lung, nome, righe, nota):
        self.ind, self.lung, self.nome, self.righe, self.nota = ind, lung, nome, righe, nota


def pezzi_tabelle(rom, nomi_inv):
    p = [Pezzo(INIZIO, TAB_TONI - INIZIO, 'riempimento_05C6', None, 'riempimento FF'),
         Pezzo(TAB_TONI, 0x60, 'tab_periodi_toni', corpo_parole(rom, TAB_TONI, 48),
               'periodi del tono per indice 00-2F (nota + 12 * ottava + trasposizione)'),
         Pezzo(TAB_TONI_B, 0x30, 'tab_periodi_toni_b',
               impagina(['B(0x%02X)' % rom[a] for a in range(TAB_TONI_B, TAB_DURATE)]),
               'periodi del tono (byte alto 0) per indice 30-5F'),
         Pezzo(TAB_DURATE, 0xC0, 'tab_durate', corpo_parole(rom, TAB_DURATE, 96),
               'durate in IRQ (2 per frame) dei byte DUR(0)-DUR(95)'),
         Pezzo(TAB_INV, 0x20, 'tab_inviluppi',
               impagina(['W(ADR_%s)' % nomi_inv[i] for i in range(16)]),
               'inizi degli inviluppi software STRUM(0)-STRUM(15)')]
    inizi = [parola(rom, TAB_INV + 2 * i) for i in range(16)] + [INV_FINE]
    for i in range(16):
        p.append(Pezzo(inizi[i], inizi[i + 1] - inizi[i], nomi_inv[i],
                       corpo_inviluppo(rom, inizi[i], inizi[i + 1]),
                       'inviluppo software %d: coppie (volume, IRQ)' % i))
    return p


def pezzi_flussi(rom, cmd, nomi, da, a):
    """Pezzi fra gli indirizzi da e a, uno per etichetta."""
    elenchi = {c.elenco: c for c in cmd}
    tagli = sorted(x for x in nomi if da <= x < a) + [a]
    out = []
    for x, y in zip(tagli, tagli[1:]):
        if x in elenchi:
            c = elenchi[x]
            nota = ('comando %02X: elenco dei record (blocco, flusso)' % c.num if c.num
                    else 'residuo mai collegato: elenco dei record')
            out.append(Pezzo(x, y - x, nomi[x], corpo_elenco(rom, c, y, nomi), nota))
        else:
            m = re.search(r'_b(\d+)_s(\d)', nomi[x])
            k = int(m.group(1)) if m else 0
            nota = 'flusso, blocco %d, strato %s (chip %d voce %d)' % (
                k, m.group(2), k // 3 % 3 + 1, 3 - k % 3) if m else 'flusso'
            if nomi[x].endswith(tuple('_p%d' % i for i in range(2, 99))):
                nota = 'ingresso interno (bersaglio di F4/F7)'
            out.append(Pezzo(x, y - x, nomi[x], corpo_flusso(rom, x, y, nomi), nota))
    return out


def testo_pezzi(pezzi, gruppo):
    righe = []
    for p in pezzi:
        if p.righe is None:
            continue
        righe.append('/* %04X-%04X %s */' % (p.ind, p.ind + p.lung - 1, p.nota))
        righe.append('static const uint8_t %s[] = {' % p.nome)
        righe += p.righe
        righe.append('};')
    righe.append('')
    righe.append('const struct aud_pezzo aud_pezzi_%s[] = {' % gruppo)
    for p in pezzi:
        if p.righe is None:
            righe.append('    RIEMPIMENTO(%s, 0x%04X),' % (p.nome, p.lung))
        else:
            righe.append('    PEZZO(%s),' % p.nome)
    righe.append('};')
    righe.append('const unsigned aud_n_pezzi_%s =' % gruppo)
    righe.append('    sizeof aud_pezzi_%s / sizeof aud_pezzi_%s[0];' % (gruppo, gruppo))
    return righe


def file_c(gruppo, descrizione, pezzi):
    return (INTESTAZIONE + '/* %s */\n#include <stddef.h>\n#include "audio_dati.h"\n\n'
            % descrizione + '\n'.join(testo_pezzi(pezzi, gruppo)) + '\n')


# ---------------------------------------------------------------- controllo

def espandi(righe, indirizzi):
    """Byte delle macro del testo C, con la semantica di audio_dati.h."""
    b8 = lambda x: [x & 0xFF]
    w = lambda x: [x & 0xFF, (x >> 8) & 0xFF]
    spazio = dict(indirizzi)
    spazio.update({n: i for i, n in enumerate(NOTE)})
    spazio.update(
        B=b8, W=w, NOTA=lambda o, n: [(o << 4) | n], PAUSA=[0x0C], DUR=lambda i: [0x80 | i],
        STRUM=lambda n: [0xF0, n], INV_HW=lambda f, p: [0xF0, 0xFF, f] + w(p),
        INV_HW_NOTA=lambda f: [0xF0, 0xFE, f], INV_HW_FORMA=lambda f: [0xF0, 0xFE, f, 0xFF],
        ATTEN=lambda x: [0xF1, x], TRASP=lambda x: [0xF2, x & 0xFF], SCOST=lambda x: [0xF3, x],
        SALTA=lambda a: [0xF4] + w(a), RIP_INIZIO=lambda n: [0xF5, n], RIP_FINE=[0xF6],
        CHIAMA=lambda a: [0xF7] + w(a), RITORNA=[0xF8], RUMORE=lambda x: [0xF9, x],
        RUMORE_OFF=[0xFA], GLISSA=lambda p0, p1, d: [0xFB] + w(p0) + w(p1) + w(d),
        PERIODO=lambda p: [0xFC] + w(p), COMANDO=lambda x: [0xFD, x], ATTEN_PIU=[0xFE],
        FINE=[0xFF], REC=lambda k, a: [k] + w(a), FINE_ELENCO=[0xFF],
        PASSO=lambda v, n: [v, n], RIPARTI=[0xFF])
    testo = re.sub(r'/\*.*?\*/', '', ' '.join(righe))
    out = []
    for v in eval('[' + testo + ']', {'__builtins__': {}}, spazio):
        out += v
    return bytes(out)


def controlla(rom, pezzi, nomi):
    indirizzi = {'ADR_' + n: a for a, n in nomi.items()}
    img = bytearray(b'\xAA' * 0x4000)
    coperto = [False] * 0x4000
    for p in pezzi:
        dati = b'\xFF' * p.lung if p.righe is None else espandi(p.righe, indirizzi)
        if len(dati) != p.lung:
            sys.exit('ERRORE: %s espande %d byte, attesi %d' % (p.nome, len(dati), p.lung))
        for i in range(p.lung):
            if coperto[p.ind + i]:
                sys.exit('ERRORE: %s si sovrappone a un altro pezzo' % p.nome)
            coperto[p.ind + i] = True
        img[p.ind:p.ind + p.lung] = dati
    if not all(coperto[INIZIO:]) or img[INIZIO:] != rom[INIZIO:]:
        sys.exit('ERRORE: le macro espanse non ridanno la regione 05C6-3FFF')


# ---------------------------------------------------------------- documento

def simula(rom, inizio, passi_max=500000):
    """Percorso principale di un flusso con F5/F6, F7/F8 e F4 seguiti.
    Restituisce (IRQ totali, None) o (None, 'ciclo') se F4 riporta allo
    stesso stato di controllo; il primo valore conta d + 1 IRQ per nota."""
    a, dur, rip_cont, rip_punt, rit, irq, visti = inizio, 0, 0, 0, 0, 0, set()
    for _ in range(passi_max):
        b = rom[a]
        if b < 0x80:
            irq += dur + 1
            a += 1
        elif b < 0xF0:
            dur = parola(rom, TAB_DURATE + 2 * (b & 0x7F)) - 1
            a += 1
        elif b == 0xFB:
            dur = parola(rom, a + 5)
            irq += dur + 1
            a += 7
        elif b == 0xFC:
            irq += dur + 1
            a += 3
        elif b == 0xF4:
            chiave = (a, rip_cont, rip_punt, rit)
            if chiave in visti:
                return None, 'ciclo'
            visti.add(chiave)
            a = parola(rom, a + 1)
        elif b == 0xF5:
            rip_cont, rip_punt = rom[a + 1], a + 1
            a += 2
        elif b == 0xF6:
            rip_cont = (rip_cont - 1) & 0xFF
            a = rip_punt + 1 if rip_cont else a + 1
        elif b == 0xF7:
            rit, a = a + 2, parola(rom, a + 1)
        elif b == 0xF8:
            a = rit + 1
        elif b == 0xFF:
            return irq, None
        else:
            a += lung_istr(rom, a)
    return None, 'ciclo'


def strumenti(rom, unita_lette):
    """Strumenti delle unità di flusso: nomi e numeri d'inviluppo software."""
    s = set()
    for a in unita_lette:
        b = rom[a]
        if b == 0xF0:
            s.add('inviluppo hardware' if rom[a + 1] >= 0xFE else rom[a + 1])
        s.update({0xF9: ['rumore'], 0xFB: ['glissando'], 0xFC: ['periodo diretto'],
                  0xF2: ['trasposizione']}.get(b, []))
    return s


def testo_strumenti(s):
    soft = sorted(x for x in s if isinstance(x, int))
    nomi = sorted(x for x in s if isinstance(x, str))
    if soft:
        nomi.insert(0, 'inviluppo software ' + ', '.join(str(n) for n in soft))
    return '; '.join(nomi) or 'solo tono'


def frame_testo(irq):
    """Durata da IRQ (2 per frame) a frame e secondi a 60 frame/s."""
    f = ('%d' % (irq // 2)) + (',5' if irq % 2 else '')
    return '%s frame (%s s)' % (f, ('%.2f' % (irq / 120)).replace('.', ','))


def descrivi(rom, c, nomi):
    """Righe del record e riassunto (voci, strati, durata, strumenti)."""
    righe, voci, strati, durate, ciclo, strum = [], set(), set(), [], False, set()
    for k, p in c.record:
        byte, lette, _ = percorri(rom, p)
        note = [rom[a] for a in lette if rom[a] < 0x80 and rom[a] & 15 < 12]
        irq, stato = simula(rom, p)
        chip, voce, strato = k // 3 % 3 + 1, 3 - k % 3, 1 if k < 9 else 2
        voci.add((chip, voce))
        strati.add(strato)
        st = strumenti(rom, lette)
        strum |= st
        if stato:
            ciclo = True
        else:
            durate.append(irq)
        ottave = ('ottave %d-%d' % (min(n >> 4 for n in note), max(n >> 4 for n in note))
                  if note else 'nessuna nota')
        righe.append('- blocco %d, strato %d, chip %d voce %d, flusso `%s` a %04X: %d byte, '
                     '%d note, %s; %s; %s' % (
                         k, strato, chip, voce, nomi[p], p, len(byte), len(note), ottave,
                         testo_strumenti(st),
                         'ciclo' if stato else frame_testo(irq)))
    durata = 'ciclo' if ciclo else frame_testo(max(durate))
    sintesi = '%d %s, %d %s, %s' % (len(voci), 'voce' if len(voci) == 1 else 'voci',
                                     len(strati), 'strato' if len(strati) == 1 else 'strati',
                                     durata)
    return righe, sintesi, durata, ciclo, testo_strumenti(strum), voci, strati


def leggi_nomi_suoni():
    """Nomi dei suoni da audio_nomi.tsv (righe «cc<TAB>nome», # = commento),
    se il file esiste: {comando: nome}; «|» diventa «/» per la tabella."""
    nomi = {}
    p = os.path.join(QUI, 'audio_nomi.tsv')
    if not os.path.exists(p):
        return nomi
    with open(p, encoding='utf-8') as f:
        for riga in f:
            riga = riga.rstrip('\n')
            if not riga.strip() or riga.lstrip().startswith('#'):
                continue
            cc, _, nome = riga.partition('\t')
            nome = nome.strip().replace('|', '/')
            if nome:
                nomi[int(cc.strip(), 16)] = nome
    return nomi


def documento(rom, cmd, nomi):
    principali = [c for c in cmd if c.num]
    nomi_suoni = leggi_nomi_suoni()
    info = {c.num: descrivi(rom, c, nomi) for c in principali}
    in_ciclo = [c.num for c in principali if info[c.num][3]]
    usi = {}
    for c in cmd:
        for _, p in c.record:
            usi.setdefault(p, []).append(c)
    t = ['# Spartiti del Z80 audio: i comandi 01-3F',
         '',
         'Generato da `estrai_dati.py` dalla ROM `1.3jk` (brief 21): non modificare a',
         'mano. I dati stessi stanno in `harness/audio_dati_*.c` come macro leggibili',
         '(`NOTA`, `DUR`, `STRUM`, `SALTA`, `CHIAMA`, ...; definite in',
         '`harness/audio_dati.h`); la semantica dei byte è in `AUDIO-SEQUENZE.md`.',
         '',
         'Legenda. Un comando (valore 01-3F scritto nel latch) ha un elenco di record',
         '(blocco, flusso). Il blocco k (0-17) suona sul chip k/3 mod 3 + 1, voce 3 − k',
         'mod 3; i blocchi 0-8 sono il primo strato, 9-17 il secondo, che copre la',
         'stessa voce del blocco k − 9 finché non finisce. Byte e note contano ciò che',
         'è raggiungibile dal flusso (F4 e F7 seguiti); le ottave sono quelle scritte',
         'nei byte, prima della trasposizione F2. La durata nominale somma le durate',
         'delle note lungo il percorso (ripetizioni F5/F6 coi loro giri, chiamate',
         'F7/F8), d + 1 IRQ per nota, 2 IRQ per frame, secondi a 60 frame/s',
         '(attesa, non misurata); per il comando è quella della voce più lunga, il',
         'primo IRQ di avvio escluso. «ciclo» = un salto F4 riporta il flusso in un',
         'punto già suonato con lo stesso stato: il comando non finisce da solo (lo',
         'ferma un altro comando o il latch 00). La colonna «nome» resta vuota: i nomi',
         'si scrivono in `audio_nomi.tsv` ascoltando `out/wav/codici/<cc>.wav`.',
         '',
         '## Riassunto',
         '',
         '| comando | nome | elenco | record | voci | strati | durata | strumenti |',
         '|---|---|---|---|---|---|---|---|']
    for c in principali:
        r, _, durata, _, st, voci, strati = info[c.num]
        nome = nomi_suoni.get(c.num, '')
        t.append('| %02X | %s| %04X | %d | %d | %d | %s | %s |' % (
            c.num, nome + ' ' if nome else '', c.elenco, len(c.record),
            len(voci), len(strati), durata, st))
    t += ['', 'Comandi in ciclo: ' + ' '.join('%02X' % n for n in in_ciclo), '',
          'Comandi: %d; record: %d; flussi distinti: %d.' % (
              len(principali), sum(len(c.record) for c in principali),
              len({p for c in principali for _, p in c.record})), '']
    for c in principali:
        righe, sintesi, _, _, st, _, _ = info[c.num]
        t += ['### Comando %02X' % c.num, '',
              'Elenco a %04X, %d record. %s; %s.' % (c.elenco, len(c.record), sintesi, st), '']
        t += righe
        condivisi = sorted({'`%s` anche nel comando %s' % (nomi[p], ', '.join(
            '%02X' % d.num for d in usi[p] if d is not c)) for _, p in c.record
            if len(usi[p]) > 1})
        if condivisi:
            t += ['', 'Flussi condivisi: ' + '; '.join(condivisi) + '.']
        t.append('')
    res = next(c for c in cmd if not c.num)
    righe, sintesi, _, _, st, _, _ = descrivi(rom, res, nomi)
    t += ['## Residuo 3811 (mai collegato)', '',
          'Elenco a %04X con %d record, che nessuna voce della tabella dei comandi' % (
              res.elenco, len(res.record)),
          'raggiunge: resto di un comando tolto. Estratto come `residuo_3811` per',
          'ricomporre la ROM. %s; %s.' % (sintesi, st), ''] + righe + ['']
    return '\n'.join(t)


# ---------------------------------------------------------------- scrittura

def scrivi(nome, testo):
    """Scrive solo se il contenuto cambia (idempotente)."""
    vecchio = None
    if os.path.exists(nome):
        with open(nome) as f:
            vecchio = f.read()
    if vecchio != testo:
        with open(nome, 'w') as f:
            f.write(testo)


def raggruppa(cmd, pezzi_cmd):
    """Gruppi consecutivi di comandi con al massimo RIGHE_MAX_FILE righe."""
    gruppi, corrente, righe = [], [], 0
    for c in cmd:
        n = sum(len(p.righe) + 4 for p in pezzi_cmd[c.num])
        if corrente and righe + n > RIGHE_MAX_FILE:
            gruppi.append(corrente)
            corrente, righe = [], 0
        corrente.append(c)
        righe += n
    gruppi.append(corrente)
    return gruppi


def main():
    rom = leggi_rom()
    for a in list(range(INIZIO, TAB_TONI)) + list(range(RIEMPI_FINE, FINE_ROM)):
        if rom[a] != 0xFF:
            sys.exit('ERRORE: byte %04X del riempimento non è FF' % a)
    cmd = comandi(rom)
    nomi = etichette(rom, cmd)
    nomi_inv = ['inviluppo_%02d' % i for i in range(16)]
    for i in range(16):
        nomi[parola(rom, TAB_INV + 2 * i)] = nomi_inv[i]
    tabelle = pezzi_tabelle(rom, nomi_inv)
    voci = ['W(ADR_cmd_%02X)' % n for n in range(1, 64)]
    tabelle.append(Pezzo(TAB_CMD, 0x80, 'tab_comandi', ['    W(0x0000),'] + impagina(voci),
                         'tabella dei comandi: elenco per latch 01-3F (00 = arresto)'))
    tabelle += pezzi_flussi(rom, cmd, nomi, RESIDUO, RIEMPI_FINE)
    tabelle.append(Pezzo(RIEMPI_FINE, FINE_ROM - RIEMPI_FINE, 'riempimento_385F', None,
                         'riempimento FF'))
    for p in tabelle:
        nomi.setdefault(p.ind, p.nome)
    principali = [c for c in cmd if c.num]
    confini = [c.elenco for c in principali] + [RESIDUO]
    pezzi_cmd = {c.num: pezzi_flussi(rom, cmd, nomi, confini[i], confini[i + 1])
                 for i, c in enumerate(principali)}
    tutti = tabelle + [p for c in principali for p in pezzi_cmd[c.num]]
    controlla(rom, tutti, nomi)

    file_gen = {'audio_dati_tabelle.c': file_c(
        'tabelle', 'Tabelle, inviluppi, tabella dei comandi, residuo 3811 e riempimenti.',
        tabelle)}
    gruppi = ['tabelle']
    for i, g in enumerate(raggruppa(principali, pezzi_cmd)):
        lettera = chr(ord('a') + i)
        gruppi.append('spartiti_' + lettera)
        file_gen['audio_dati_spartiti_%s.c' % lettera] = file_c(
            gruppi[-1], 'Elenchi e flussi dei comandi %02X-%02X.' % (g[0].num, g[-1].num),
            [p for c in g for p in pezzi_cmd[c.num]])
    ind = [INTESTAZIONE, '/* Indirizzi ROM delle etichette dei dati audio. */',
           '#ifndef AUDIO_DATI_INDIRIZZI_H', '#define AUDIO_DATI_INDIRIZZI_H', '']
    ind += ['#define ADR_%-28s 0x%04X' % (nomi[a], a) for a in sorted(nomi)]
    file_gen['audio_dati_indirizzi.h'] = '\n'.join(ind + ['', '#endif', ''])
    gr = [INTESTAZIONE, '/* Gruppi di pezzi: uno per file audio_dati_*.c. */',
          '#ifndef AUDIO_DATI_GRUPPI_H', '#define AUDIO_DATI_GRUPPI_H', '',
          '#include "audio_dati.h"', '']
    for g in gruppi:
        gr.append('extern const struct aud_pezzo aud_pezzi_%s[];' % g)
        gr.append('extern const unsigned aud_n_pezzi_%s;' % g)
    gr += ['', '#define AUD_DATI_GRUPPI \\']
    gr += ['    { aud_pezzi_%s, &aud_n_pezzi_%s }, \\' % (g, g) for g in gruppi]
    gr += ['', '#endif', '']
    file_gen['audio_dati_gruppi.h'] = '\n'.join(gr)
    for nome, testo in sorted(file_gen.items()):
        scrivi(os.path.join(HARNESS, nome), testo)

    scrivi(os.path.join(QUI, 'SPARTITI.md'), documento(rom, cmd, nomi))
    print('pezzi: %d, file di spartiti: %d, etichette: %d'
          % (len(tutti), len(gruppi) - 1, len(nomi)))


if __name__ == '__main__':
    main()
