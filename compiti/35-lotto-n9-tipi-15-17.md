# Compito 35: lotto N9 del motore dei nemici, tipi 15-17 (6B43, 6C0B, 7F48-809C)

Data: 2026-11-03. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere.

## Contesto: file da leggere

Da leggere (e basta):
1. `decomp/out/nemici_n9.dasm`, da generare (vedi sotto): 286 righe di
   disassemblato delle 16 routine del lotto. Dentro ci sono tre tabelle di dati
   disassemblate come istruzioni, che non sono codice: 6B59-6B5E (`rst 38`
   di 6B43), 6BC5-6BCC (tabella di 6B9B) e 6C0F-6C1C (`rst 38` di 6C0B);
   più 8089-808C (tabella di 8042: `04 08 08 0C`).
2. `decomp/harness/nemici.h` (299 righe): r. 1-60 (riporto, `nemici_rst38`),
   r. 130-150 (N2), r. 262-299 (registro R e stub).
3. Le convenzioni qui sotto.

Da aprire perché si modificano o servono da modello, solo per la parte indicata:
`harness/nemici_tipi_15_17.c` (59 righe: tutto, è il file che cresce),
`harness/nemici_stub.c` (26 righe: tutto), `harness/routine_tab.c` (solo gli
intorni di `0x6BCD`-`0x6CE8` r. 177-184, `0x7F2D` e `0x80A5` r. 234-235),
`harness/nemici_tipi_08_14.c` (r. 236-287: `sub_6e55` e `sub_6e90`, modello
per 6C8E, che ne ripete l'inizio e usa 6E83 e 6E8B),
`harness/nemici_tipo06b.c` (r. 1-60: modello di modulo con `IXB`/`IYB`),
`decomp/ROUTINE.md` (solo r. 534-587: sezione N8; e la r. 208 con 7F68),
`decomp/CENSIMENTO-NEMICI.md` (solo il paragrafo «**Lotto N8 fatto**»).

È vietato leggere `STATO.md`, `README.md`, `ROUTINE.md` per intero, gli
altri brief in `compiti/` e i moduli `nemici_*.c` non elencati sopra.

Generazione del disassemblato (da `/home/rufio72/OpenSolomonsKey-master`;
`awk` è mawk; i dati sopra elencati restano dentro i range e sono
disassemblati, i range escludono 6BCD, 6C1D e 7F51/7F68 già tradotte):
```
cd decomp && awk 'BEGIN{n=split("6b43 6c0b 6c5f 7f48 7f81",lo," "); split("6bcc 6c1c 6ce7 7f50 80a4",hi," ")}
/^[A-Za-z_0-9]+:/ {lab=$0; next}
/;[0-9a-f][0-9a-f][0-9a-f][0-9a-f][ \t]*$/ {a=substr($0,length($0)-3); ok=0; for(i=1;i<=n;i++) if(a>=lo[i] && a<=hi[i]) ok=1; if(ok){ if(lab!="") print lab; print} lab=""; next}
{lab=""}' out/main64.dasm > out/nemici_n9.dasm; cd ..
```
Uscita attesa (misurata): `wc -l < decomp/out/nemici_n9.dasm` → `286`;
`grep -c 'ld a,r' decomp/out/nemici_n9.dasm` → `0` (nessuna routine del
lotto legge R: niente wrapper `banco_r_*`). Il file è ignorato da git
(`decomp/out/*`): si aggiunge al commit con `git add -f`.

## Obiettivo

Tradurre in C le 16 routine del lotto N9 (6B43, 6B5F, 6B8F, 6B9B, 6C0B, 6C5F,
6C8E, 7F48, 7F81, 7F8A, 7FA6, 7FCF, 801D, 8042, 808D, 809C: 561 byte), nel
modulo esistente `nemici_tipi_15_17.c` (59 righe, stima finale circa 400);
togliere 10 stub; portare 12 voci nel banco con 84 catture MAME (11 voci del
lotto più la 7F68, già tradotta in N1, vedi «Catture»). Il replay e `sched_proto`
non devono cambiare: il replay non raggiunge nessuna routine del lotto
(altrimenti lo stub avrebbe fermato la spedizione; misurato: finisce al frame
5499 con exit 0).

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `cattura.sh`, `snap.lua`, `sessioni.sh`, `replay.*`, `sched*.c`,
  `rst20*`, `rst38_tables.h`, `censimento_nemici.py`, `STATO.md`,
  `README.md`, `compiti/` (tranne questo file non si scrive nulla), né le
  catture esistenti in `decomp/out/`.
- Niente installazioni. MAME gira solo come fa `cattura.sh` (senza finestra,
  senza audio, provider a `none`): niente partite a mano, niente giri su
  tutte le stanze. Una cattura dura 3-100 secondi (fino a 6 in parallelo con
  `xargs -P6`, file di uscita distinti).
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe e nessun `nemici*.c` sopra le 700.
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`); si
  ripristina un file solo con `git checkout -- <file>`.
- Un solo agente alla volta su `decomp/harness`.

## Cosa esiste già (fatti verificati)

Misure del 2026-11-03 sui binari già compilati (da `decomp/harness`):
- `./harness --tutte`: exit 0, 796 righe `^cattura`, 0 `FALLITO`, 228 righe
  `^RIEPILOGO`. Replay: exit 0, fino al frame 5499. `sched_proto`:
  `106124, 0, 131`.
- `grep -c STUB_NEMICI nemici_stub.c` → 17 (16 stub più la riga `#define`).
  Stub del lotto: 6b43, 6c0b, 7f48, 7f81, 7f8a, 7fa6, 7fcf, 801d, 8042,
  809c (10). Gli altri 6 (2474, 4ada, 4fd4, 5fc8, 610c, 627f) restano.
  6B5F, 6B8F, 6B9B, 6C5F, 6C8E e 808D non hanno stub (nessuno le chiama
  ancora).
- Chiamanti già scritti in C (non cambiano): `nemici.c` r. 74-76 (`salta_4f1b`:
  `sub_6c0b`, `sub_6b43`, `sub_7f68`); `nemici_tipi_15_17.c` r. 12-28
  (`salta_7f68`, smista su 7FA6, 7F48, 7FCF, 7F51, 7F81, 7F8A, 801D, 8042) e
  r. 33-43 (`sub_7f51` chiama `sub_809c`).
- Le tabelle `rst 38` sono già in `rst38_tables.h`: `rst38_6B58` (3 voci:
  6B5F, 6B8F, 6B9B), `rst38_6C0E` (7 voci: 3B62, 6C1D, 6D37, 6C5F, 6DCE,
  6C8E, 6EA1). La tabella di 6B9B (6BC5: parole 0080, 00C0, 0100, 0140)
  e quella di 8042 (8089: 04 08 08 0C) non ci sono: si leggono con `leggi()`
  o si copiano come `static const` locali (libero).
- Nessun `ld a,r` nel lotto.

Grafo delle chiamate (dal dasm e da `grafo symbol`; `(t)` = già tradotta con
prototipo in `routine.h` o `nemici.h`):

| ingresso | byte | chiamanti | chiama |
|---|---:|---|---|
| 6B43 | 22 | 4F0F (`rst 38`) | `rst 38` a 6B58: 6B5F, 6B8F, 6B9B |
| 6B5F | 48 | 6B43 | 6BCD(t), `rst 28`(t) (`sub_0028`) |
| 6B8F | 11 | 6B43 | - |
| 6B9B | 42 | 6B43 | `rst 8`(t), `rst 10`(t), 57AA(t), 7379(t), 57B3(t) |
| 6C0B | 4 | 4F0F | `rst 38` a 6C0E: 3B62(t), 6C1D(t), 6D37(t), 6C5F, 6DCE(t), 6C8E, 6EA1(t) |
| 6C5F | 47 | 6C0B | - |
| 6C8E | 90 | 6C0B | 6E90(t), 5C32(t), tail 6E83 e 6E8B (già in `sub_6e55`) |
| 7F48 | 9 | 7F68 | - |
| 7F81 | 9 | 7F68 | 80A5(t) |
| 7F8A | 28 | 7F68 | 24BE(t), 3A54(t) |
| 7FA6 | 41 | 7F68 | - |
| 7FCF | 78 | 7F68 | tail 808D, tail 809C |
| 801D | 37 | 7F68 | tail 809C |
| 8042 | 71 | 7F68 | tail 808D |
| 808D | 15 | 7FCF, 8042 | 24BE(t), 4C11(t), cade in 809C |
| 809C | 9 | 7F51, 7FCF, 801D, 808D | - |

Firme (da `grafo`/`grep`, tutte `void sub_xxxx(struct z80 *z)`): `sub_0008`
(DE = A, HL += 2·A), `sub_0010` (DE = parola a HL, HL += 2), `sub_0028`
(`rst 28`: suono C, conserva AF, BC, HL), `sub_4af6`, `sub_3a54`,
`sub_5c32`, `sub_6c2c`, `sub_7379` (routine.h); `sub_24be`, `sub_4c11`,
`sub_57aa`, `sub_57b3`, `sub_6bcd`, `sub_6e90`, `sub_80a5`, `sub_3b62`,
`sub_6c1d`, `sub_6d37`, `sub_6dce`, `sub_6ea1` (nemici.h).

**Il riporto.** Dove una routine lascia un riporto che il chiamante legge
si usa `NEMICI_PONI_RIPORTO(z, c)` / `NEMICI_RIPORTO_ACCESO(z)`
(nemici.h r. 16-19). Casi del lotto, ricavati dal dasm:
- 6B9B: `sub_57aa` legge il riporto entrante (`sbc hl,bc`). L'ultima istruzione
  che lo tocca è l'`add hl,de` di `rst 8` (6BAB), con HL = 6BC5 + 2·(0..3):
  nessun riporto. Prima di `sub_57aa(z)` scrivere `NEMICI_PONI_RIPORTO(z, 0)`.
  Poi `ret c` dopo 57AA, `ret nc` dopo 7379: il riporto di uscita sta in
  `z->f` (convenzione di entrambe).
- 6C8E: dopo `sub_6e90(z)` (6C91) il riporto è in `z->f` (come in `sub_6e55`).
  I salti di 6CAE-6CD1 usano flag locali: `jr nz` a 6CAE è sul risultato
  dell'`and (iy+1ah)` (6CA8); `cp 002h` a 6CB2 e `bit 0,a` a 6CC7 danno lo
  Z/C con cui si arriva a 6CC9 (`ld` e `set` non toccano i flag), poi
  `jp z,6E8B` a 6CD1. Usare una variabile locale per Z.
- 6C5F, a 6C75-6C7A: `rra; ccf; rla` sul riporto locale (variabile `c`).
- Le altre routine non passano il riporto fuori.

### Convenzioni di traduzione

- Ogni routine è `void sub_xxxx(struct z80 *z)`, tutte non statiche.
  Registri `z->a b c d e h l`, `z->ix` (slot), `z->iy` (oggetto), `z->f`.
  Memoria con `M(indirizzo)`; ROM con `leggi(indirizzo)`; coppie con
  `BC(z) DE(z) HL(z)` e `SET_BC SET_DE SET_HL`. Per le celle classe
  C080/C081 c'è `CLASSI_C080` (vedi `sub_6e55`). Il modulo ha già
  `#include "routine.h"`, `"nemici.h"`, `"rst38_tables.h"`; aggiungere
  `#define IXB(o) M(z->ix + (o))` e `IYB(o)` come in `nemici_tipo06b.c`.
- Il banco confronta RAM (c000-e5ff, salvo 0x40 byte sotto SP) e A, BC, DE,
  HL, IX, IY all'uscita, non F: anche i registri «sporchi» devono coincidere
  (B, C, DE, HL lasciati dalle sottoroutine). Le catture di `rst 38`
  lasciano DE = sito + 2 + 2·E (lo fa `nemici_rst38`).
- `rst 38` a 6B58: `nemici_rst38(z, 0x6B58, rst38_6B58, 3)`; a 6C0E:
  `nemici_rst38(z, 0x6C0E, rst38_6C0E, 7)`; poi `switch` sulla destinazione
  come `salta_7f68` (r. 12-28): `0` = indice fuori tabella (non fare nulla),
  il resto `nemici_manca(dest)`.
- 6B43: A = ((IY+4) & 0C) >> 2 (due `rra`, riporto entrante nullo), C = A, B =
  (IX+4) se il bit 1 di A è acceso, altrimenti (IX+5); E = (IY+1D); A resta
  quello di C all'ingresso del `rst 38`.
- Le code `jp 808D` (7FFA), `jp 809C` (7FFD, 8040 `jr`, 7F65 già fatta),
  `jp 80A5` (7F87) sono salti di coda: si traducono `sub_xxxx(z); return;`.
  808D finisce cadendo in 809C (non c'è `ret`): `sub_808d` termina con
  `sub_809c(z);`. 8011-801C (fine di 7FCF) è un blocco raggiunto dal
  `jr`/caduta di 8005: tradurlo inline in `sub_7fcf`.
- A 6CC3 (`ld (0001h),a`) l'originale scrive in ROM: nessun effetto, non si
  scrive `M(1)`.
- Catture `forza*` valgono come catture OK ma non contano per lo stato
  `verificata`, che richiede almeno 2 catture non forzate.

## Struttura da realizzare

**1. `harness/nemici_tipi_15_17.c`** (esistente, 59 righe): aggiornare
l'intestazione (aggiungere «Lotto N9 (brief 35): 6B43, 6B5F, 6B8F, 6B9B,
6C0B, 6C5F, 6C8E, 7F48, 7F81, 7F8A, 7FA6, 7FCF, 801D, 8042, 808D, 809C;
tipo 15 = 6C0B, tipo 16 = 6B43, tipo 17 = 7F68») e aggiungere in ordine
libero (prototipi in `nemici.h`) le 16 funzioni `sub_6b43`, `sub_6b5f`,
`sub_6b8f`, `sub_6b9b`, `sub_6c0b`, `sub_6c5f`, `sub_6c8e`, `sub_7f48`,
`sub_7f81`, `sub_7f8a`, `sub_7fa6`, `sub_7fcf`, `sub_801d`, `sub_8042`,
`sub_808d`, `sub_809c`. Due `static` d'appoggio per i dispatch, `salta_6b43`
e `salta_6c0b`, come `salta_7f68`. Commenti come nei moduli esistenti:
indirizzo del dasm accanto a ogni gruppo di istruzioni. Se a lavoro finito
il file supera 650 righe, spostare 6C5F, 6C8E in `nemici_tipi_15_17b.c`
(intestazione e `IXB/IYB` ripetuti).

**2. `harness/nemici_stub.c`**: togliere le 10 righe `STUB_NEMICI(6b43)`,
`(6c0b)`, `(7f48)`, `(7f81)`, `(7f8a)`, `(7fa6)`, `(7fcf)`, `(801d)`,
`(8042)`, `(809c)`. Restano 2474, 4ada, 4fd4, 5fc8, 610c, 627f. Il commento
di intestazione resta.

**3. `harness/nemici.h`**: togliere i 10 prototipi dalla sezione «Stub»
(r. 281-290). Dopo la sezione N8 (e prima di «Il registro R») una sezione
«Lotto N9 (brief 35)» con i prototipi delle 16 funzioni (segnare: 6B9B e 6C8E
non lasciano il riporto fuori; nessuna lo lascia).

**4. `harness/routine_tab.c`**: 12 voci `{ "0xXXXX", sub_xxxx, 0, 0 }`, in
ordine di indirizzo: `0x6B43`, `0x6B5F`, `0x6B9B` prima di `0x6BCD`;
`0x6C0B` prima di `0x6C1D`; `0x6C5F` e `0x6C8E` dopo `0x6C2C` e prima di
`0x6CE8`; `0x7F48`, `0x7F68`, `0x7FA6`, `0x7FCF` dopo `0x7F2D`; `0x8042`,
`0x809C` prima di `0x80A5`. 6B8F, 7F81, 7F8A, 801D e 808D non hanno voce
(ingresso non catturabile, vedi sotto).

**5. Catture.** Da `decomp/`. Misurate il 2026-11-03 in una copia di lavoro con
questi stessi comandi (`cattura.sh` e `snap.lua` originali): tutte riescono,
84 file `_regs.txt` in tutto. Stati: `finale` parte dal frame 2300,
`stanzaNN` dal 1951.

Non forzate (38). Un file `/tmp/n9_nat.txt` con una cattura per riga
(`ENTRY stato N [suffisso]`) e poi `xargs -P6 -L1 sh cattura.sh < /tmp/n9_nat.txt`
(tutte le righe: nessuna virgola né virgolette):
```
0x6B43 finale 1
0x6B43 finale 2 n2
0x6B43 stanza50 1
0x6B43 stanza50 2 n2
```
e lo stesso blocco di quattro righe per 6B5F, 6B9B, 6C0B, 6C5F, 6C8E (24
righe in tutto), più:
```
0x7F48 stanza07 1
0x7F48 stanza07 2 n2
0x7F48 stanza18 1
0x7F48 stanza18 2 n2
0x7F68 stanza07 1
0x7F68 stanza07 2 n2
0x7FA6 stanza07 1
0x7FA6 stanza07 2 n2
0x7FCF stanza45 1
0x7FCF stanza45 2 n2
0x8042 stanza43 1
0x8042 stanza43 2 n2
0x809C stanza07 1
0x809C stanza07 2 n2
```
Misurato: 7F48 in `stanza00` e 7F8A in `stanza42`, `stanza43`, `stanza51` (anche
con `SNAP_LIMITE=15000`) **non producono cattura**: per questo 7F48 usa
`stanza07`/`stanza18` e 7F8A non ha ingresso catturabile. 6B8F, 7F81, 801D e
808D non sono mai raggiunte neppure nelle sessioni di censimento (colonna
«sessioni» NO): non catturabili.

Forzate (46), stato come indicato, N=1, `SNAP_FORZA` applicato all'ingresso
(sintassi di `snap.lua`: `ix+HH:VV`, `iy+HH:VV`, registri; valori
esadecimali). Tag `forzaNN`. Tutte riescono (misurato). Il ramo aperto è
dedotto dal dasm (attesa, non misurata l'apertura; misurato che la cattura
riesce). Funzione d'aiuto, poi un ciclo per riga (`xargs` toglie le
virgolette ma qui non ce ne sono):
```
cd decomp
cat > /tmp/go35.sh <<'EOF'
SNAP_FORZA="$4" sh cattura.sh $1 $2 1 $3
EOF
xargs -P6 -L1 sh /tmp/go35.sh < /tmp/n9_forze.txt
```
con `/tmp/n9_forze.txt` (formato `ENTRY stato tag valori`):
```
0x6B43 finale forza01 iy+1d:01,ix+08:2d
0x6B43 finale forza02 iy+1d:01,ix+08:2e
0x6B43 finale forza03 iy+1d:02
0x6B43 finale forza04 iy+1d:00,iy+04:00,ix+01:00
0x6B43 finale forza05 iy+1d:00,iy+04:10,ix+01:00
0x6B43 finale forza06 iy+1d:00,iy+04:08,ix+01:01,ix+0d:22
0x6B43 finale forza07 iy+1d:00,iy+04:08,ix+01:01,ix+0d:23
0x6C0B finale forza00 iy+1d:00
0x6C0B finale forza01 iy+1d:01
0x6C0B finale forza02 iy+1d:02
0x6C0B finale forza03 iy+1d:03
0x6C0B finale forza04 iy+1d:04
0x6C0B finale forza05 iy+1d:05
0x6C0B finale forza06 iy+1d:06
0x6C5F finale forza01 iy+1e:00
0x6C5F finale forza02 iy+1e:02,ix+0d:05
0x6C5F finale forza03 iy+1e:02,ix+0d:06,ix+01:10
0x6C5F finale forza04 iy+1e:02,ix+0d:06,ix+01:00
0x6C8E finale forza01 ix+05:00,ix+04:00
0x6C8E finale forza02 ix+05:40,iy+1e:02,ix+0d:2f
0x6C8E finale forza03 ix+05:40,iy+1e:02,ix+0d:30
0x6C8E finale forza04 ix+05:40,iy+1e:00,iy+1a:00
0x6C8E finale forza05 ix+05:40,iy+1e:00,iy+1a:10,iy+12:00,iy+13:00
0x6C8E finale forza06 ix+05:40,iy+1e:01,iy+1a:20,iy+12:00,iy+13:00
0x6C8E finale forza07 ix+05:40,iy+1e:01,iy+1a:20,iy+12:01
0x7FA6 stanza07 forza01 ix+01:00
0x7FA6 stanza07 forza02 ix+01:01,ix+08:0f
0x7FA6 stanza07 forza03 ix+01:01,ix+08:10
0x7FCF stanza45 forza01 ix+01:00
0x7FCF stanza45 forza02 ix+01:04,iy+1a:00,ix+0e:05
0x7FCF stanza45 forza03 ix+01:04,iy+1a:00,ix+0e:00
0x7FCF stanza45 forza04 ix+01:04,iy+1a:30,ix+08:d1
0x7FCF stanza45 forza05 ix+01:04,iy+1a:30,ix+08:d2
0x8042 stanza43 forza01 iy+01:02
0x8042 stanza43 forza02 iy+01:00,iy+1e:01,ix+08:00
0x8042 stanza43 forza03 iy+01:00,iy+1e:01,ix+08:ff
0x8042 stanza43 forza04 iy+01:00,iy+1e:00,iy+1a:00
0x8042 stanza43 forza05 iy+01:00,iy+1e:00,iy+1a:30,ix+08:00
0x7F68 stanza07 forza00 iy+04:00,iy+1d:00
0x7F68 stanza07 forza01 iy+04:00,iy+1d:00,ix+08:78
0x7F68 stanza07 forza02 iy+04:00,iy+1d:02
0x7F68 stanza07 forza03 iy+04:00,iy+1d:04,iy+01:00,ix+08:00,iy+1a:30
0x7F68 stanza07 forza04 iy+04:00,iy+1d:04,iy+01:02
0x7F68 stanza07 forza05 iy+04:00,iy+1d:04,iy+01:00,ix+08:78
0x7F68 stanza07 forza06 iy+04:00,iy+1d:04,iy+01:00,ix+08:00,iy+1a:00
0x7F68 stanza07 forza07 iy+04:00,iy+1d:05
```
Rami aperti (dedotti): 6B43 forza01-02 → 6B8F (`ret c` / 6B95), forza03 →
6B9B, forza04-07 → 6B5F (prima volta con B = 0E/0F, poi `ret c` a 6B82, poi
suono e fase 2); 6C0B forza00-06 → un indice per destinazione; 7F68 forza00-01 →
7F81 (`ret c` / 80A5), forza02 → 7F8A, forza03-06 → 801D (ret nz, bit 1
acceso, (IX+8) ≥ 78, 809C), forza07 → 8042. Per i rami di 6C8E che dipendono
dalla cella davanti (C080/C081, da 5C32) e per 6B9B (celle di 57AA/7379) le
forzate con `ix`/`iy` non bastano: coperti solo se le catture naturali li
percorrono.

Non fare forzate con un indice fuori tabella (`iy+1d` ≥ 3 per 6B43, ≥ 7 per
6C0B): l'originale salterebbe in aria e `nemici_rst38` registra il sito
come errore. Se una cattura, nel banco, fallisce perché entra in uno stub che
resta (2474, 4ada, 4fd4, 5fc8, 610c, 627f), si scarta e si dice nel rapporto.

Totali: 38 + 46 = 84 catture, 12 voci. Per voce (non forzate + forzate):
6B43 4+7, 6B5F 4, 6B9B 4, 6C0B 4+7, 6C5F 4+4, 6C8E 4+7, 7F48 4,
7F68 2+8, 7FA6 2+3, 7FCF 2+5, 8042 2+5, 809C 2.

**6. Copertura del C** (gcov, fuori dal repository; prefisso `h-`):
```
D=/tmp/cov35 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
 && ln -s "$PWD/decomp/out" $D/out && (cd $D && gcc -std=c11 -O0 --coverage -o h \
 harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c z80_*.c \
 nemici*.c attivita.c attivita_01_10.c && ./h --tutte > /dev/null; \
 gcov h-nemici_tipi_15_17.gcda > /dev/null 2>&1; \
 awk -F: '$1 ~ /#####/' nemici_tipi_15_17.c.gcov)
```
Nel rapporto: le righe non eseguite e il motivo. Attese e note (stima, non
misurata): i casi `0` e `default` dei tre `switch` (gli indici fuori tabella),
alcuni rami di 6B9B (dipendono da 57AA/7379) e di 6C8E (celle davanti). Altre
righe scoperte: aggiungere una forzata (stesso schema, tag `forzaNN`
successivo, stato come sopra, N=1, ingresso della routine con byte scelti per
aprire il ramo) solo se il ramo è raggiungibile senza passare dagli stub;
altrimenti elencarle.

**7. Documenti.** `ROUTINE.md`: in fondo (dopo la r. 587), sezione
`## Motore dei nemici (lotto N9)` con un paragrafo (modulo
`nemici_tipi_15_17.c`; stub rimasti; 7F68 entra nel banco per coprire 7F81,
7F8A, 801D; 6B8F e 808D coperte dalle forzate di 6B43 e di 7FCF/8042; nessun
ingresso catturabile per 6B8F, 7F81, 7F8A, 801D, 808D) e la tabella come quella
della N8 (ingresso, funzione, file, byte, descrizione, stato, catture OK,
stati, note), una riga per routine (16) più una riga per 7F68 (già tradotta
in N1, ora `verificata`). Aggiornare la riga della r. 208 (`0x7F68`):
stato «verificata (banco, lotto N9)». Stato `verificata` per le 11 voci del
lotto con voce nel banco; `tradotta` per 6B8F, 7F81, 7F8A, 801D, 808D; poi
l'elenco delle forzate. `CENSIMENTO-NEMICI.md`: dopo il paragrafo «Lotto N8
fatto» un paragrafo «**Lotto N9 fatto** (brief 35)» con le 16 routine, gli
stub di `nemici_stub.c` da 16 a 6 (tolti 10), 12 voci nuove nel banco, 84
catture. Le righe della tabella dei moduli non cambiano.

## Verifiche obbligatorie

Tutte da `/home/rufio72/OpenSolomonsKey-master`; uscita attesa esatta salvo
dove è scritto «attesa, dedotta». Abbreviazione:
`V="6B43 6B5F 6B9B 6C0B 6C5F 6C8E 7F48 7F68 7FA6 7FCF 8042 809C"`.

1. Disassemblato: `wc -l < decomp/out/nemici_n9.dasm; grep -c 'ld a,r' decomp/out/nemici_n9.dasm`
   → `286` e `0` (misurate).
2. Compilazione: `cd decomp/harness && make harness replay sched_proto > /tmp/m35.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/m35.txt`
   → `make 0` e `0`.
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h35.txt; echo "exit $?"; grep -c FALLITO /tmp/h35.txt; grep -c '^RIEPILOGO' /tmp/h35.txt; grep -c '^cattura' /tmp/h35.txt`
   → `exit 0`, `0`, `240`, `880` (228 + 12 voci; 796 + 84; attese, dedotte).
4. Catture per voce: `for a in $V; do grep "^RIEPILOGO 0x$a:" /tmp/h35.txt; done`
   → (attese, dedotte dai file elencati sopra)
   ```
   RIEPILOGO 0x6B43: 11 catture, 11 OK
   RIEPILOGO 0x6B5F: 4 catture, 4 OK
   RIEPILOGO 0x6B9B: 4 catture, 4 OK
   RIEPILOGO 0x6C0B: 11 catture, 11 OK
   RIEPILOGO 0x6C5F: 8 catture, 8 OK
   RIEPILOGO 0x6C8E: 11 catture, 11 OK
   RIEPILOGO 0x7F48: 4 catture, 4 OK
   RIEPILOGO 0x7F68: 10 catture, 10 OK
   RIEPILOGO 0x7FA6: 5 catture, 5 OK
   RIEPILOGO 0x7FCF: 7 catture, 7 OK
   RIEPILOGO 0x8042: 7 catture, 7 OK
   RIEPILOGO 0x809C: 2 catture, 2 OK
   ```
5. Catture non forzate per voce: `cd decomp && for a in $V; do ls out/snap_0x${a}_*_regs.txt | grep -vc forza; done | tr '\n' ' '`
   → `4 4 4 4 4 4 4 2 2 2 2 2 ` (misurate nella copia di lavoro).
6. Totale file: `cd decomp && for a in $V; do ls out/snap_0x${a}_*_regs.txt; done | wc -l`
   → `84` (misurata).
7. Voci precedenti invariate: `for a in 4767 4E87 3569 7379 6C2C 54C5 51FA 6C1D 6BCD 57AA 6E90 7F2D 80A5; do grep "^RIEPILOGO 0x$a:" /tmp/h35.txt; done`
   → `4767: 22 catture, 22 OK`, `4E87`, `3569`, `7379`, `6C2C`, `54C5`: `2 catture, 2 OK`,
   `51FA: 30…30 OK`, `6C1D: 6…6 OK`, `6BCD: 6…6 OK`, `57AA`, `6E90`,
   `7F2D`, `80A5`: `2…2 OK` (misurate prima della modifica).
8. `sched_proto`: `cd decomp && harness/sched_proto out/sched_log.txt | tail -1`
   → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
9. Replay: `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r35.txt; echo "exit $?"; tail -4 /tmp/r35.txt`
   → `exit 0` e (misurate):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
10. Stub: `grep -c STUB_NEMICI decomp/harness/nemici_stub.c; grep -c -E '6b43|6c0b|7f48|7f81|7f8a|7fa6|7fcf|801d|8042|809c' decomp/harness/nemici_stub.c`
    → `7`, `0` (attese, dedotte: oggi 17; si tolgono 10).
11. Copertura: il comando del punto 6 di «Struttura». Uscita attesa (attesa,
    non misurata): ogni riga `#####` spiegata nel rapporto (indici fuori
    tabella e rami che dipendono dalle celle).
12. Dimensioni: `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/nemici*.c | awk '$1>700 && $2!="totale" && $2!="total"'` → vuoto.
13. File vietati intatti: `git diff --name-only HEAD | grep -E 'cattura\.sh|snap\.lua|sessioni|replay\.|sched|rst20|rst38_tables|censimento_nemici\.py|STATO\.md|README\.md|compiti/'` → vuoto.
14. Ambito: `git status --short | grep -v -E '^( M|\?\?|A ) (compiti/35-|decomp/(harness/(nemici_tipi_15_17\.c|nemici_tipi_15_17b\.c|nemici\.h|nemici_stub\.c|routine_tab\.c)|ROUTINE\.md|CENSIMENTO-NEMICI\.md|out/(snap_0x(6B43|6B5F|6B9B|6C0B|6C5F|6C8E|7F48|7F68|7FA6|7FCF|8042|809C)_|nemici_n9\.dasm)))'`
    → vuoto; `git status --short | grep -c -E '\.(gcda|gcno)$'` → `0`.
15. Documenti: `grep -c '^## Motore dei nemici (lotto N9)$' decomp/ROUTINE.md; grep -c 'Lotto N9 fatto' decomp/CENSIMENTO-NEMICI.md`
    → `1` e `1`.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto N9 del motore dei nemici: tipi 15-17 (6B43, 6C0B, 7F48-809C), catture nel banco`
  e riga finale `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`
  (se il lotto passa a Opus per escalation, `Claude Opus 5.5`). Il commit
  comprende `decomp/out/nemici_n9.dasm` (`git add -f`) e le catture nuove
  (`prima.bin`, `dopo.bin`, `regs.txt`, `.log`; con `git add -f`).
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-15; l'elenco
  delle voci con numero di catture e stato; le righe gcov non coperte; le
  deviazioni dal brief (cattura che fallisce o scartata, cambio di N o di
  stato).

## Domande aperte

1. **7F68 nel banco**: 7F68 è del lotto N1, non del N9, ma è l'unico
   ingresso che raggiunge 7F81, 7F8A, 801D (e, via 8042/7FCF, 808D) con le
   forzate. Proposta: aggiungerla al banco (10 catture, +1 voce, 12 voci in
   tutto) e segnare 7F81, 7F8A, 801D come `tradotta` coperte dalle forzate
   di 7F68. — «preferenza»
2. **5 routine senza ingresso catturabile** (6B8F, 7F81, 7F8A, 801D, 808D).
   Proposta: `tradotta`, coperte dalle forzate dei genitori (6B43, 7F68,
   7FCF/8042); 7F8A compare nelle sessioni di censimento ma `cattura.sh` non
   la raggiunge in nessuno stato provato. — «preferenza»
3. **Forzate con indice fuori tabella** (6B43, 6C0B, 7F68) non fatte;
   i casi `0` e `default` degli `switch` restano scoperti in gcov. Proposta:
   come scritto. — «preferenza»
4. **Rami di 6C8E e 6B9B** che dipendono dalle celle (C080/C081) e da
   57AA/7379: coperti solo se le catture naturali li percorrono. Proposta:
   elencarli in `ROUTINE.md` e nel rapporto, senza altre catture. —
   «preferenza»
5. **Spezzatura**: nessuna prevista (circa 400 righe). Se oltre 650,
   `nemici_tipi_15_17b.c` come descritto. — «preferenza»
6. **`STATO.md` e `README.md`**: li aggiorna la chat principale dopo la
   verifica (banco a 880 catture e 240 voci). — «preferenza»

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Tutte le domande sono preferenze, accolte con la proposta dell'analista:
1. Sì: 7F68 (lotto N1) entra nel banco con le sue 10 catture, come via per 7F81, 7F8A e 801D.
2. Sì: le 5 routine senza ingresso catturabile (6B8F, 7F81, 7F8A, 801D, 808D) si segnano `tradotta`, coperte dalle forzate dei genitori.
3. Sì: niente forzate con indice fuori tabella; i casi `0` e `default` degli switch restano scoperti in gcov e si annotano.
4. Sì: i rami di 6C8E e 6B9B dipendenti dalle celle di griglia si elencano in ROUTINE.md e nel rapporto, senza altre catture.
5. Sì: nessuna spezzatura preventiva; se il modulo supera le 650 righe si usa `nemici_tipi_15_17b.c`.
6. Sì: STATO.md e README.md li aggiorna la chat principale dopo la verifica.

## Esito (chat principale, 2026-11-03)

Passato al primo colpo, settimo ciclo con lo schema economico (analista
Sonnet ~123k token, sviluppatore Sonnet ~81k, verifica a script senza
modello). Le 16 routine dei tipi 15-17 sono in `nemici_tipi_15_17.c`
(416 righe), 10 stub tolti (restano 7), banco a 240 voci e 886 catture
con 0 fallite, replay e `sched_proto` invariati. Deviazione migliorativa:
6 forzate in più su 7F68 (`forza08-13`) per coprire le destinazioni della
tabella 7F39, da cui 886 catture invece delle 880 attese e 16 catture su
7F68 invece di 10. Scoperture gcov annotate in `ROUTINE.md`: casi `0` e
`default` dei tre switch (indici fuori tabella non forzati), ramo dopo
`sub_7379` in 6B9B, ramo 6CC7/6E8B in 6C8E (dipendono dalle celle).
STATO.md e README.md si aggiornano insieme all'esito del lotto N10.
