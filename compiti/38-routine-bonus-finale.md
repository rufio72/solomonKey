# Compito 38: routine dei quadri bonus e del finale (censimento del lotto 12, rifatto)

Data: 2026-11-03. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere. Rifà e supera `compiti/28-routine-bonus-finale.md` (mai
eseguito): non va letto.

## Obiettivo

Tradurre in C le 23 routine di `decomp/CENSIMENTO-12.md` che oggi mancano
(quadri bonus, oggetti speciali, attività 25/26/16, finale), ciascuna con
catture MAME nel banco (`harness --tutte`), e scrivere il destino delle
altre 17 destinazioni del censimento. 23 voci nuove nel banco, 61 catture
nuove. Banco storico, `sched_proto` e replay non devono cambiare.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `snap.lua`, `sessioni.sh`, `replay.*`, `sched*.c`, `rst20*`,
  `rst38_tables.h`, `censimento_nemici.py`, `STATO.md`, `README.md`,
  `compiti/` (tranne questo file, niente), né le catture già in `decomp/out/`
  (`git ls-files decomp/out | grep snap_`), compresa `snap_0x3FCE_porta_*`.
- Niente installazioni. MAME (`/usr/games/mame`) solo senza finestra, come
  `cattura.sh` (`-video none -sound none`, provider a `none`). Nessuna partita
  a mano. Una cattura dura 3-100 s; fino a 6 in parallelo con `xargs -P6`.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra
  le 800 righe (i moduli `z80_*.c` toccati restano sotto le 650: stima
  `z80_4xxx.c` ~520, `z80_2xxx.c` ~570, `z80_38xx.c` ~290).
- Niente registro R nelle catture: la sola routine del lotto che legge R
  (4302) si verifica con un wrapper `banco_r_4302` (regola ratificata).
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`); si
  ripristina un file solo con `git checkout -- <file>`.
- Un solo agente alla volta su `decomp/harness`.

## Perimetro (verificato sul codice di oggi)

40 destinazioni del censimento = 23 da fare + 17 fuori. Fuori, con destino da
scrivere in `CENSIMENTO-12.md`:
- già tradotte dai lotti N3-N12 (grep su `harness/*.c` e `ROUTINE.md`): 134B
  (`nemici_comparsa.c`, verificata), 4ADA (`nemici_comuni.c`), 5DDF e 5E09
  (`nemici_tipo07a.c`, 5DDF col wrapper `banco_r_5ddf`), 5FDC e 6248
  (`nemici_tipo07b.c`), 5115 (`nemici_tipo06a.c`, tradotta senza ingresso
  catturabile), 5124 e 51FA (`nemici_tipo06a.c`, verificate);
- interni, solo documentati: 02A2, 02A4, 02A5 (etichette interne di
  `sub_027c`, `nmi_oggetti.c`); 40FF (interno del ciclo `40FE` del corpo
  dell'attività 1C, spedidore a passi, non tradotto); 7B64 (interno del passo
  7B3B-7B94, `djnz 7B64`);
- 0CFD, 0E0B, 11A5: gestori della tabella 0D18 (chiave, continuazione 0CF3 di
  `sub_0ce6`), lotto a parte futuro (1278 B); 0E0B sta nel gestore 0D8E, 11A5
  in 113C.

## Cosa esiste già (fatti verificati)

Baseline misurata il 2026-11-03 sui binari già compilati (`decomp/harness`,
senza modifiche): `./harness --tutte` exit 0, 0 `FALLITO`, 264 `^RIEPILOGO`,
1110 `^cattura`; `harness/sched_proto out/sched_log.txt | tail -1` →
`spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`;
replay: vedi verifica 8; `grep -c 03ee` sull'uscita del replay → 0 (03EE non è
mai raggiunta dal replay).

| file | righe | ruolo |
|---|---:|---|
| `decomp/cattura.sh` | 79 | stati `titolo gioco tardo porta bonus finale stanzaNN`; `SNAP_POKE`, `SNAP_POSTA`, `SNAP_COND`, `SNAP_FORZA` dall'ambiente |
| `decomp/cattura_oggetti.sh` | 24 | stato `gioco` + 14 terne di oggetti 08-1B nella coda c05b e posta 22 ogni 100 frame da 2300; tag `gioco_oggetti[_SUFFISSO]`; fa `exec sh cattura.sh` |
| `harness/z80_1xxx.c` | 32 | contiene solo `sub_1288`; include solo `<stdio.h>` e `routine.h` |
| `harness/z80_2xxx.c` | 502 | 234A-2551 |
| `harness/z80_38xx.c` | 132 | `sub_3a1c`, `sub_3f10`, `sub_3f88`... (3Cxx-3Fxx vanno qui, `z80_3xxx.c` 551 righe non si tocca) |
| `harness/z80_4xxx.c` | 439 | include solo `routine.h`; contiene `sub_4d87` (r. 367) e `sub_4398` (r. 399, routine diversa, non confondere con 4319) |
| `harness/z80_6xxx.c` | 81 | include `routine.h` e `nemici.h` |
| `harness/z80_7xxx.c` | 229 | include `routine.h` |
| `harness/nmi.c` | 264 | stub `static void sub_03ee` (r. 26-30, avviso su stderr), chiamato a r. 193 con `SET_HL(&n, 0xc078); n.a = M(0xc078);` |
| `harness/attivita.c` | 354 | `static uint16_t sub_250e(uint16_t hl, int *riporto)` (r. 88), usata a r. 118 da `attivita_0d_5c8e` |
| `harness/nemici_r.c` | 175 | `prova_con_r` (r. 130) e i `banco_r_*` (r. 166-175) |
| `harness/routine.h` | 163 | prototipi; commento «Lotto 10» con la frase «Lo stub 03EE è statico in nmi.c» |
| `harness/routine_tab.c` | 274 | `{ "0xXXXX", sub_xxxx, 0, 0 },` in ordine di indirizzo |
| `harness/nemici.h` | 342 | prototipi nemici e `banco_r_*` (r. 331-340); `void sub_4b98(uint16_t ix);` (r. 72), `sub_24be` (81), `sub_4c11` (86), `sub_6eb7` (175), tutte `(struct z80 *z)` salvo 4b98 |

(da `wc -l`, `grafo symbol`, grep). Il Makefile prende `z80_*.c` e `nemici*.c`
con `wildcard`: nessuna modifica. Routine già tradotte da chiamare, tutte
`void sub_xxxx(struct z80 *z)`: `sub_0008` (rst 8), `sub_0010` (rst 10),
`sub_0018` (rst 18), `sub_3a54` (routine.h r. 156), `sub_4d87`, `sub_4af6`
(routine.h), `sub_24be`, `sub_4c11`, `sub_6eb7`, `sub_4b98(uint16_t ix)`
(nemici.h).

Convenzioni (da `ROUTINE.md` e dai moduli): `z->a b c d e h l`, `z->ix`, `z->iy`,
`z->f`; `M(a)` solo RAM (c000-ffff); **ROM con `leggi(a)`** (ram.h r. 16: una
tabella ROM letta con `M()` ha già dato un bug nel lotto N11); coppie
`BC(z) DE(z) HL(z)`, `SET_BC SET_DE SET_HL`. Il banco confronta RAM e A, BC,
DE, HL, IX, IY all'uscita, non F: anche i registri «sporchi» devono coincidere.
Z e riporto che il chiamante legge stanno nel bit 6 e nel bit 0 di `z->f`
(es. `z->f = (uint8_t)((z->f & ~1) | prestito)` in `z80_2xxx.c` r. 207); `F`
in ingresso arriva dalla cattura (`AF=` di `_regs.txt`).

### Disassemblato

Da `/home/rufio72/OpenSolomonsKey-master` (mawk). Si commenta ogni gruppo di
istruzioni con l'indirizzo del dasm, come nei moduli esistenti. Dentro ci sono
byte di dati o di routine vicine disassemblati come codice (dopo i `ret`
finali di ogni routine): non sono da tradurre.
```
cd decomp && awk 'BEGIN{n=split("03ee 1471 1533 2425 250e 3cc1 3d20 3da6 3fb2 4302 6976 7b95",lo," "); split("03fa 1491 1549 244a 251d 3cc9 3d5f 3e0f 3fe7 4398 69a7 7baf",hi," ")}
/^[A-Za-z_0-9]+:/ {lab=$0; next}
/;[0-9a-f][0-9a-f][0-9a-f][0-9a-f][ \t]*$/ {a=substr($0,length($0)-3); ok=0; for(i=1;i<=n;i++) if(a>=lo[i] && a<=hi[i]) ok=1; if(ok){ if(lab!="") print lab; print} lab=""; next}
{lab=""}' out/main64.dasm > out/bonus_n38.dasm; cd ..
```
Uscita attesa (misurata): `wc -l < decomp/out/bonus_n38.dasm` → `419`;
`grep -c 'ld a,r' decomp/out/bonus_n38.dasm` → `2` (righe 430A e 430D, solo in
4302). Il file è ignorato da git: `git add -f`.

## Le 23 routine

Byte da cammino statico (stima righe C: 0,6 per byte). «→» = cade nella
successiva senza `ret`.

| ingresso | byte | modulo | chiama | note dal dasm |
|---|---:|---|---|---|
| 03EE | 13 | `nmi.c` | 3A54 | B = A, (HL) = 0, C D E da HL+1..HL+3, `sub_3a54`; sostituisce lo stub statico |
| 1471 | 3 | `z80_1xxx.c` | → 1474 | HL = c093 e `sub_1474(z)` |
| 1474 | 11 | `z80_1xxx.c` | | raddoppio BCD di 3 byte a HL: `or a`, tre volte `adc a,a; daa`; il riporto passa fra i byte |
| 147F | 18 | `z80_1xxx.c` | rst 10 | A = 2·(c243), L = A + A4 (`jr nc` altrimenti `inc h`), HL = 8EA4 + ..., `rst 10` (ROM: `sub_0010`), HL = 0009 + DE |
| 1533 | 22 | `z80_1xxx.c` | | B = (c259), HL = c25d, A = c0; ciclo: `and (hl)` → `ret z` (Z); due `rrca`, `dec hl` se riporto, `djnz`; esaurito: `scf` (riporto in `z->f` bit 0) |
| 2425 | 11 | `z80_2xxx.c` | | (HL) += (DE) con riporto sul byte seguente, HL e DE += 2 |
| 2430 | 17 | `z80_2xxx.c` | | azzera 7 byte con D ed E interposti, A = B |
| 2441 | 9 | `z80_2xxx.c` | | HL = 16·A (A invariato) |
| 250E | 15 | `z80_2xxx.c` | rst 10 | vedi sotto |
| 3CC1 | 1 | `z80_38xx.c` | → 3CC2 | `M(HL) = A` e `sub_3cc2(z)` |
| 3CC2 | 8 | `z80_38xx.c` | | `res 2,h; ld (hl),65; set 2,h; inc hl` |
| 3D20 | 15 | `z80_38xx.c` | → 3D2F | ciclo B volte (`djnz` su 3D20): somma a 32 bit con costante 000186A0, poi `sub_3d2f(z)` in coda |
| 3D2F | 20 | `z80_38xx.c` | | somma HL e A al valore a 24 bit c1aa-c1ac |
| 3D43 | 9 | `z80_38xx.c` | 4D87 | DE = c061, C = 0, `sub_4d87` |
| 3DA6 | 13 | `z80_38xx.c` | | (IX+0B/0C) = L/H, (IX+12/13) = E/D |
| 3DEA | 36 | `z80_38xx.c` | 4AF6 | scrive (c87b) = B(08), (c87c) = C; **`ld ix,c88e` non viene ripristinato**; (IX+0E) = (c86c); (IX+15) = (c873) − 10; `sub_4af6` |
| 3FB2 | 28 | `z80_38xx.c` | 3FCE | 3 volte: `rld` su (HL) da c095 verso c093 con A = 0, `sub_3fce` per ciascuna delle 2 cifre; Z = (A == 0) dopo `rld` va messo in `z->f` bit 6 prima di ogni `sub_3fce`; DE = 3FE8 (tabella ROM) |
| 3FCE | 26 | `z80_38xx.c` | | se Z (bit 6 di `z->f` in ingresso): salta a `inc de` ×3 e `ret`; altrimenti C volte somma a 24 bit di 3 byte ROM (DE) con c1aa-c1ac (tre `adc a,(hl)`), poi DE += 3 |
| 4302 | 23 | `z80_4xxx.c` | 2441 | HL = 82DA + 16·A; due `ld a,r`: `nemici_r_somma()`; A = ROM[HL + ((somma & 1E) >> 1)] (con `leggi`) |
| 4319 | 127 | `z80_4xxx.c` | 24BE, 4B98, 4C11, 6EB7, rst 8/18 | A−0F: <4 → 432B; 4 → `ret`; 5-6 → 4360; 7 → `ret`; 8 → 437C; altrimenti `ret`. Serve `#include "nemici.h"` in `z80_4xxx.c`. A 4342: `push hl; ex (sp),ix` = IX ← HL con IX vecchio salvato, `sub_4b98(ix)`, `pop ix` |
| 6976 | 49 | `z80_6xxx.c` | rst 10 | incrementa tre contatori (c1ba+h, c1ca+offset, c1fc+(c243)); `rst 10` legge la parola RAM c23a |
| 7B95 | 9 | `z80_7xxx.c` | | `(HL) = C; res 2,h; (HL) = 04; set 2,h; dec hl` |
| 7B9E | 19 | `z80_7xxx.c` | 3A54 | ciclo fino al byte `ff` (ROM, `leggi`): B = byte, C = byte seguente, `sub_3a54` con HL e DE salvati, D += 2 |

Dettagli:
- **250E**: nuova `void sub_250e(struct z80 *z)` esportata in `z80_2xxx.c`.
  Uscita (misurata nelle catture): A = 0, B = 0, C = vecchio (HL), DE = HL_in + 1,
  HL = parola nuova, riporto di `add hl,bc` in `z->f` bit 0; scrive
  (HL_in+1) = E, (HL_in+2) = D. La statica di `attivita.c` diventa un involucro
  (rinominarla `conta_250e`, stessa firma): costruisce una `struct z80` locale
  azzerata, `SET_HL`, chiama `sub_250e`, restituisce `HL` e `*riporto = z.f & 1`.
  Il comportamento di `attivita_0d_5c8e` non cambia (il replay lo prova).
- **03EE**: togliere `static` e l'avviso in `nmi.c` (restano gli include se
  servono altrove; controllare se `fprintf` è usato ancora), definirla con il
  corpo di 03EE; in `routine.h` dichiararla e togliere «Lo stub 03EE è statico
  in nmi.c» dal commento «Lotto 10». Il chiamante (r. 191-193) resta uguale.
- **4302 e il registro R**: le due `ld a,r` sono come quelle di 5DDF
  (R1, poi R2 = R1 + 3 con un `ld` in mezzo): `nemici_r_somma()` (nemici.h)
  restituisce già R1 + R2. In `nemici_r.c` aggiungere in coda
  `void banco_r_4302(struct z80 *z) { prova_con_r(z, "0x4302", sub_4302); }`,
  il prototipo accanto ai `banco_r_*` di `nemici.h` e la voce `0x4302` di
  `routine_tab.c` su `banco_r_4302`. `sub_4302` va dichiarata in `routine.h`.
- **Z e riporto di uscita** (1533: `ret z` con Z, `scf` con riporto; 3FB2→3FCE:
  Z in ingresso): impostarli in `z->f` anche dove il banco non li confronta,
  così i chiamanti futuri li trovano.
- **Prototipi**: tutte le nuove in `routine.h` in una sezione «Lotto 38
  (compito 38): quadri bonus e finale», tranne `sub_4302` (anche lì).
- **Interni**: 1471→1474, 3CC1→3CC2, 3D20→3D2F cadono; 3D43 e 3FCE sono ingressi
  veri.

## Strumenti di cattura

Modifiche provate in una copia di lavoro fuori dal repository (stato di
partenza `decomp/out/` vuota): tutte le 61 catture sotto riescono.

**`decomp/cattura.sh`**: due stati nuovi, da inserire prima di
`stanza[0-9]|stanza[0-9][0-9])` nel primo `case` (e le righe corrispondenti nel
secondo `case`, accanto a `porta|bonus|finale`); aggiornare il commento di
testa e il messaggio dello stato sconosciuto (`... finale, finale2, attNN, stanzaNN`):
```
    finale2) INGRESSI=traccia; FRAMES="4500"; SNAP_LIMITE=${SNAP_LIMITE:-9500}; export SNAP_LIMITE
            POKE_DEF=2250:0xc19d:0,2250:0xc19e:0,2250:0xc24b:0x32,2250:0xc24c:0x32,2250:0xc24d:0x32 ;;
    att[0-9A-F][0-9A-F])
            INGRESSI=traccia; FRAMES="2300"
            POKE_DEF=2250:0xc19d:0,2250:0xc19e:0 ;;
```
e nel secondo `case`:
```
    finale2)
        SNAP_POKE=${SNAP_POKE:-$POKE_DEF}
        SNAP_POSTA=${SNAP_POSTA:-2300:0x1c} ;;
    att[0-9A-F][0-9A-F])
        SNAP_POKE=${SNAP_POKE:-$POKE_DEF}
        SNAP_POSTA=${SNAP_POSTA:-2300:0x${STATO#att}} ;;
```
`finale2` = inneschi di `finale`, partenza al frame 4500 (la stanza finale);
`attNN` (es. `att25`) = c19d/c19e azzerati e posta dell'attività NN al 2300
(inneschi di `att_XX` di `sessioni.sh`). `sh -n cattura.sh` → nessun errore.

**`decomp/cattura_oggetti.sh`**: riga 22 diventa
`export SNAP_POKE="${SNAP_POKE_EXTRA:+$SNAP_POKE_EXTRA,}$POKE" SNAP_POSTA="$POSTA"`
(poke extra opzionali «frame:addr:val» davanti alle terne); aggiungere al
commento di testa `SNAP_POKE_EXTRA`.

## Catture (da `decomp/`)

Una cattura riuscita stampa `CATTURA <ENTRY> <tag> frame_da=<F> OK`;
`frame_da` è 1951 per `stanzaNN`, 2300 per porta/bonus/finale/attNN, 4500 per
`finale2`, 2400 per `cattura_oggetti.sh`. Non fallire tutto se una cattura
fallisce (misurato: nessuna fallisce nell'elenco sotto).

**A. Non forzate con `cattura.sh` (34)**, file `/tmp/n38_a.txt` (`ENTRY stato N [suffisso]`),
`xargs -P6 -L1 sh cattura.sh < /tmp/n38_a.txt`:
```
0x03EE stanza17 1
0x03EE stanza17 2 n2
0x2425 porta 1
0x2425 porta 2 n2
0x2425 bonus 1
0x2430 porta 1
0x2430 finale 1
0x3CC1 att25 1
0x3CC1 att25 2 n2
0x3CC2 att25 1
0x3CC2 att25 2 n2
0x3D20 att25 1
0x3D20 att25 2 n2
0x3D2F att25 1
0x3D2F att25 2 n2
0x3D43 att25 1
0x3D43 att25 2 n2
0x3DA6 att25 1
0x3DA6 att25 2 n2
0x3DA6 att26 1
0x3DEA att25 1
0x3DEA att26 1
0x3FB2 porta 1
0x3FB2 finale 1
0x3FB2 att25 1
0x3FCE porta 2 n2
0x3FCE porta 3 n3
0x3FCE porta 4 n4
0x6976 att16 1
0x6976 finale2 1
0x7B95 finale2 1
0x7B95 finale2 2 n2
0x7B9E finale2 1
0x7B9E finale2 2 n2
```
(`0x3FCE porta 1` esiste già nel repository: non rifarla.)

**B. Non forzate con `cattura_oggetti.sh` (13)**, file `/tmp/n38_b.txt` (`ENTRY N [suffisso]`),
`xargs -P5 -L1 sh cattura_oggetti.sh < /tmp/n38_b.txt`:
```
0x1471 1
0x1471 2 n2
0x1474 1
0x1474 2 n2
0x147F 1
0x147F 2 n2
0x1533 1
0x1533 2 n2
0x250E 1
0x250E 2 n2
0x2441 1
0x4302 1
0x4319 1
```
(misurato: `0x4302 2 n2` e `0x4319 2 n2` danno FALLITA, c'è una sola occorrenza:
2441, 4302 e 4319 hanno una sola cattura non forzata.)

**C. Forzate (14)**, una per volta (tag `forzaNN`, N=1). `SNAP_FORZA` = registri/(IX+n)
all'ingresso; `SNAP_POKE_EXTRA`/`SNAP_POKE` = byte di RAM scritti un frame
prima (il frame è quello dell'occorrenza meno 1, dalla tabella dei frame).
Ognuna riesce (misurato); il ramo aperto è dedotto dal dasm.
```
SNAP_POKE_EXTRA=2401:0xc25d:0xff sh cattura_oggetti.sh 0x1533 1 forza01   # ciclo esaurito: scf
SNAP_POKE_EXTRA=2601:0xc243:0x2f sh cattura_oggetti.sh 0x147F 1 forza01   # c243 alto: riporto, inc h
SNAP_FORZA=a:0x03 sh cattura_oggetti.sh 0x4302 1 forza01
SNAP_FORZA=a:0x0b sh cattura_oggetti.sh 0x4302 1 forza02
SNAP_FORZA=a:0x13 sh cattura_oggetti.sh 0x4319 1 forza01                  # A-0F = 4: ret c
SNAP_FORZA=a:0x14 sh cattura_oggetti.sh 0x4319 1 forza02                  # 4360
SNAP_FORZA=a:0x15 sh cattura_oggetti.sh 0x4319 1 forza03                  # 4360
SNAP_FORZA=a:0x16 sh cattura_oggetti.sh 0x4319 1 forza04                  # A-0F = 7: ret
SNAP_FORZA=a:0x17 sh cattura_oggetti.sh 0x4319 1 forza05                  # 437C
SNAP_POKE_EXTRA=2455:0xc42a:0x38 sh cattura_oggetti.sh 0x4319 1 forza06   # 432B, bit 4 acceso
SNAP_POKE_EXTRA=2455:0xc42a:0x00 sh cattura_oggetti.sh 0x4319 1 forza07   # 432B, bit 3 spento
SNAP_POKE_EXTRA=2455:0xc42a:0x00 SNAP_FORZA=a:0x15 sh cattura_oggetti.sh 0x4319 1 forza08   # 4360, ret z
SNAP_POKE=2250:0xc19d:0,2250:0xc19e:0,2429:0xc23c:0x01 sh cattura.sh 0x6976 att16 1 forza01   # (c23c) non zero
SNAP_POKE=2250:0xc19d:0,2250:0xc19e:0,2429:0xc23b:0x0e sh cattura.sh 0x6976 att16 1 forza02   # parola >= 0E00: jr c
```
(provate con tag diversi nella copia di lavoro; stesse forzature, stessi esiti.)
Se `gcov` (sotto) mostra righe non coperte raggiungibili cambiando un registro,
(IX+n)/(IY+n) o un byte di RAM un frame prima, aggiungere una forzata con il
tag `forzaNN` successivo; altrimenti spiegarlo nel rapporto.

**Frame di cattura (misurati)**: 03EE stanza17 1975, n2 2047 · 1471 2502, n2 2602 ·
1474 2502, n2 2503 · 147F 2602, n2 2603 · 1533 2402, n2 2702 · 2425 porta 2303,
n2 2304, bonus 2303 · 2430 porta 2302, finale 2302 · 2441 2456 · 250E 2422,
n2 2423 · 3CC1 e 3CC2 att25 2501, n2 2502 · 3D20, 3D2F, 3D43 att25 2502, n2 2503 ·
3DA6 att25 2368, n2 2369, att26 2302 · 3DEA att25 2302, att26 2302 · 3FB2 porta
2386, finale 2386, att25 2502 · 3FCE porta 2386 (esistente), n2 2387, n3 2388,
n4 2389 (F di n3 e n4 con Z spento: coprono il ciclo di somma; quelle di porta
e n2 hanno Z acceso) · 4302 e 4319 2456 · 6976 att16 2430, finale2 5988 ·
7B95 e 7B9E finale2 8663, n2 8664.

Totale 34 + 13 + 14 = 61 catture nuove.

## Struttura da realizzare

1. Strumenti di cattura (sopra) e le 61 catture, `git add -f` dei file
   `decomp/out/snap_<ENTRY>_<tag>_{prima.bin,dopo.bin,regs.txt}` e `.log`.
2. Le 23 funzioni nei moduli della tabella, ciascuna con commento in italiano
   (indirizzo, effetto, registri d'uscita) e indirizzi del dasm accanto ai gruppi.
   Ordine delle funzioni nel file: definire prima di usare, o dichiarare in `routine.h`.
3. `routine.h`: prototipi (sezione nuova) e ritocco del commento «Lotto 10».
   `nmi.c` (03EE), `attivita.c` (involucro `conta_250e`), `nemici_r.c` e `nemici.h`
   (`banco_r_4302`), `z80_4xxx.c` (`#include "nemici.h"`).
4. `routine_tab.c`: 23 voci nuove `{ "0x....", sub_xxxx, 0, 0 }` in ordine di indirizzo
   (0x4302 con `banco_r_4302`). 3FCE entra con la sua cattura esistente e le 3 nuove.
5. Documenti (sotto).

### Documenti

- `decomp/ROUTINE.md`: in fondo, sezione `## Routine dei quadri bonus e del finale (lotto 38)`
  con un paragrafo (moduli toccati, involucro 250E, wrapper `banco_r_4302`, stati
  `finale2`/`attNN` e `SNAP_POKE_EXTRA`, forzate con poke di RAM) e la tabella a
  colonne standard (`| ingresso | funzione | file | byte | descrizione | stato | catture OK | stati | note |`),
  una riga per routine. Stato `verificata` per chi ha 2 o più non forzate
  (tutte tranne 2441, 4302, 4319 che restano `tradotta`); nelle note «N non
  forzate e M forzate»; elenco delle forzate e righe gcov non coperte in coda.
  Aggiornare anche la riga di 0x250E se già citata altrove (non c'è: grep).
- `decomp/CENSIMENTO-12.md`: correggere la frase iniziale «nessuna di queste
  routine è tradotta» e aggiungere in fondo la sezione «Destino delle 40
  destinazioni (brief 38)»: tabella `| ingresso | destino |` con le 23 (tradotta
  nel lotto 38, modulo), le 9 già tradotte (lotto e modulo) e le 8 documentate
  (interno di 027C; interno del ciclo 40FE; interno di 7B3B; chiave 0D18 = lotto
  futuro).
- `STATO.md` e `README.md` non si toccano (li aggiorna chi orchestra).

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master`. Prima di modificare qualunque file:
`cd decomp/harness && ./harness --tutte > /tmp/h38_prima.txt` (riferimento).
«attesa, dedotta» = non misurabile prima di scrivere il codice.

1. Disassemblato: `wc -l < decomp/out/bonus_n38.dasm; grep -c 'ld a,r' decomp/out/bonus_n38.dasm`
   → `419` e `2` (misurate).
2. Script: `cd decomp && sh -n cattura.sh && sh -n cattura_oggetti.sh && echo ok` → `ok`.
3. Catture: eseguiti A, B, C, ogni uscita in `/tmp/n38_esito.txt` →
   `grep -c ' OK$' /tmp/n38_esito.txt` → `61`; `grep -c FALLITA /tmp/n38_esito.txt` → `0`;
   `cd decomp && git ls-files out | grep -c snap_0x3FCE_porta_regs` → `1` e
   `git status --short out/snap_0x3FCE_porta_regs.txt out/snap_0x3FCE_porta_prima.bin` vuoto.
4. Compilazione: `cd decomp/harness && make harness replay sched_proto > /tmp/m38.txt 2>&1; echo "make $?"; grep -c -E 'warning|error|undefined' /tmp/m38.txt`
   → `make 0` e `0`.
5. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h38.txt; echo "exit $?"; grep -c FALLITO /tmp/h38.txt; grep -c '^RIEPILOGO' /tmp/h38.txt; grep -c '^cattura' /tmp/h38.txt`
   → `exit 0`, `0`, `287`, `1172` (264+23; 1110+61+1 della 3FCE esistente; attese, dedotte).
6. Catture per voce: `for a in 03EE 1471 1474 147F 1533 2425 2430 2441 250E 3CC1 3CC2 3D20 3D2F 3D43 3DA6 3DEA 3FB2 3FCE 4302 4319 6976 7B95 7B9E; do grep "^RIEPILOGO 0x$a:" /tmp/h38.txt; done`
   → `N catture, N OK` con N = 2, 2, 2, 3, 3, 3, 2, 1, 2, 2, 2, 2, 2, 2, 3, 2, 3, 4, 3, 9, 4, 2, 2
   (in quest'ordine; somma 62; attese, dedotte dalle catture elencate).
7. Voci precedenti invariate: `diff <(grep '^RIEPILOGO' /tmp/h38_prima.txt) <(grep '^RIEPILOGO' /tmp/h38.txt)`
   → solo 23 righe `>` (le nuove), nessuna riga `<`.
8. `cd decomp && harness/sched_proto out/sched_log.txt | tail -1` →
   `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
9. Replay: `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r38.txt 2>&1; echo "exit $?"; tail -4 /tmp/r38.txt; grep -c -i -E 'manca|03ee' /tmp/r38.txt`
   → `exit 0`, (misurato oggi):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
   e `0`. Se i numeri cambiano (250E è usata dal replay via `attivita_0d_5c8e`),
   correggere finché tornano uguali.
10. Stub tolto: `grep -c 'static void sub_03ee' decomp/harness/nmi.c; grep -c '03ee: c078' decomp/harness/nmi.c`
    → `0` e `0` (oggi 1 e 1).
11. Una funzione per ingresso: `for a in 03ee 1471 1474 147f 1533 2425 2430 2441 250e 3cc1 3cc2 3d20 3d2f 3d43 3da6 3dea 3fb2 3fce 4302 4319 6976 7b95 7b9e; do grep -c -E "^void sub_$a\(struct z80 \*z\)$" decomp/harness/*.c | awk -F: -v a=$a '{s+=$2} END{printf "%s:%d ", a, s}'; done; echo`
    → ogni voce `:1` (attesa, dedotta).
12. Copertura (gcov, fuori dal repository):
    ```
    D=/tmp/cov38 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
     && ln -s "$PWD/decomp/out" $D/out && (cd $D && gcc -std=c11 -O0 --coverage -o h \
     harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c z80_*.c \
     nemici*.c attivita.c attivita_01_10.c && ./h --tutte > /dev/null; \
     for f in z80_1xxx z80_2xxx z80_38xx z80_4xxx z80_6xxx z80_7xxx nmi attivita; do gcov h-$f.gcda > /dev/null 2>&1; \
     echo "== $f"; awk -F: '$1 ~ /#####/' $f.c.gcov; done)
    ```
    (attesa, non misurata) nelle 23 funzioni nessuna riga `#####` tranne i casi
    spiegati nel rapporto; il resto dei file ha righe scoperte preesistenti, da
    ignorare.
13. Dimensioni: `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/z80_*.c | awk '$1>650 && $2!="totale" && $2!="total"'` → vuoto.
14. Vietati intatti: `git diff --name-only HEAD | grep -E 'snap\.lua|sessioni|replay\.|sched|rst20|rst38_tables|censimento_nemici\.py|STATO\.md|README\.md|compiti/'` → vuoto.
15. Ambito: `git status --short | grep -v -E '^( M|\?\?|A ) (compiti/38-|decomp/(cattura\.sh|cattura_oggetti\.sh|ROUTINE\.md|CENSIMENTO-12\.md|harness/(z80_(1xxx|2xxx|38xx|4xxx|6xxx|7xxx)\.c|nmi\.c|attivita\.c|nemici_r\.c|nemici\.h|routine\.h|routine_tab\.c)|out/(snap_0x(03EE|1471|1474|147F|1533|2425|2430|2441|250E|3CC1|3CC2|3D20|3D2F|3D43|3DA6|3DEA|3FB2|3FCE|4302|4319|6976|7B95|7B9E)_|bonus_n38\.dasm)))'`
    → vuoto; `git status --short | grep -c -E '\.(gcda|gcno)$'` → `0`.
16. Documenti: `grep -c '^## Routine dei quadri bonus e del finale (lotto 38)$' decomp/ROUTINE.md; grep -c 'Destino delle 40 destinazioni' decomp/CENSIMENTO-12.md`
    → `1` e `1`.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto 38: routine dei quadri bonus e del finale, 23 voci nel banco`
  e riga finale `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
  (se il lavoro lo fa Sonnet: `Claude Sonnet 5.5`). Il commit comprende
  `decomp/out/bonus_n38.dasm` e le 61 catture nuove (`git add -f`).
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-16; elenco
  delle voci con numero di catture e stato; righe gcov non coperte con il
  motivo; deviazioni dal brief (cattura fallita o scartata, forzata aggiunta,
  cambio di stato).

## Domande aperte

1. **Un solo lotto.** 23 routine, circa 500 B e 300 righe C, tutte brevi e
   senza dipendenze nuove: nessuna spezzatura. Proposta: come scritto. — «preferenza»
2. **Strumenti di cattura**: gli oggetti si catturano con `cattura_oggetti.sh`
   (già nel repository) più `SNAP_POKE_EXTRA`, non con un nuovo stato `oggetti`
   di `cattura.sh`; solo `finale2` e `attNN` sono stati nuovi di `cattura.sh`.
   Proposta: come scritto. — «preferenza»
3. **Forzate con poke di RAM** un frame prima dell'ingresso (1533, 147F, 4319,
   6976), oltre a `SNAP_FORZA`; contano come forzate (tag `forzaNN`, non per
   `verificata`). Proposta: sì. — «preferenza»
4. **2441, 4302, 4319 restano `tradotta`**: un'unica occorrenza naturale
   (la seconda non esiste: FALLITA misurata). Proposta: accettarlo. — «preferenza»
5. **`censimento_12.py` del vecchio brief non si ricostruisce**: il destino
   delle 40 si scrive a mano in `CENSIMENTO-12.md`. Proposta: come scritto. — «preferenza»
6. **03EE in `nmi.c`** (dove sta lo stub) e **250E in `z80_2xxx.c`** con
   involucro `conta_250e` in `attivita.c`. Proposta: come scritto. — «preferenza»
7. **Il lotto della chiave 0D18** (0CFD, 0E0B, 11A5 e i 16 gestori, 1278 B) resta
   fuori; da pianificare dopo questo. — «scelta di progetto» (già ratificata:
   solo da ricordare a chi orchestra per il punto successivo di `STATO.md`)

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Domande 1-6 sono preferenze, accolte con la proposta dell'analista:
1. Sì: un solo lotto da 23 routine, senza spezzatura (~500 byte).
2. Sì: oggetti catturati con `cattura_oggetti.sh` più `SNAP_POKE_EXTRA`, nessuno stato nuovo `oggetti`.
3. Sì: forzate con poke di RAM un frame prima dell'ingresso (1533, 147F, 4319, 6976) oltre a `SNAP_FORZA`; non contano per `verificata`.
4. Sì: 2441, 4302 e 4319 restano `tradotta` (una sola cattura naturale, la seconda occorrenza non esiste).
5. Sì: niente ricostruzione di `censimento_12.py`; il destino delle 40 destinazioni si scrive a mano in `CENSIMENTO-12.md`.
6. Sì: 03EE resta in `nmi.c`; 250E in `z80_2xxx.c` con involucro `conta_250e` in `attivita.c`.

Domanda 7 (scelta di progetto): già ratificata dall'utente il 2026-11-03 —
la chiave 0D18 (1278 byte) resta fuori, lotto a parte da pianificare dopo;
la chat principale la annota in STATO.md a fine ciclo.

## Esito (chat principale, 2026-11-03)

Passato al primo colpo, decimo ciclo con lo schema economico (analista
Sonnet ~163k token, sviluppatore Sonnet ~113k, verifica a script senza
modello). Le 23 routine di bonus e finale sono tradotte e nel banco:
287 voci e 1173 catture con 0 fallite (1110 + 62 nuove + la pilota
preesistente di 3FCE che ora entra nel conteggio; il «1172» nel
rapporto dello sviluppatore era un refuso, la somma per voce dà 1173),
replay a fine oracolo senza divergenze nello spedidore, `sched_proto`
invariato. Deviazioni: una forzata in più su 1533 (`forza02`) per la
riga `dec hl`; un errore di primo giro su 4319 (HL non ripristinato nel
ramo 4360) trovato dal banco e corretto. Stati: 20 `verificata`,
3 `tradotta` (2441, 4302, 4319: una sola cattura naturale). Il destino
delle 40 destinazioni del lotto 12 è scritto in `CENSIMENTO-12.md`.
Il brief 28 è superato da questo. Resta fuori, da pianificare: la
chiave 0D18 (1278 byte).
