# Compito 34: lotto N8 del motore dei nemici, tipo 07 prima metà (5CFB-5F82, più 4FEF e 5FAC)

Data: 2026-10-30. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere.

## Contesto: file da leggere

Da leggere (e basta):
1. `decomp/out/nemici_n8.dasm`, da generare (vedi sotto): 388 righe di
   disassemblato. Contiene le 15 routine del lotto (5CFB-5FAB) più 4FEF e
   5FAC. Dentro 5E1A, a 5E41-5E4C, ci sono 12 byte di dati (tabella del
   `rst 38`) disassemblati come istruzioni: non sono codice.
2. `decomp/harness/nemici.h` (266 righe): r. 1-60 (riporto, `nemici_rst38`),
   r. 232-266 (sezioni N7, registro R, stub).
3. Le convenzioni qui sotto.

Da aprire perché si modificano, solo per la parte indicata:
`harness/nemici_stub.c` (24 righe: tutto), `harness/routine_tab.c` (solo gli
intorni di `0x4F40` e `0x5C78`), `harness/nemici_r.c` (162 righe: r. 1-35 e
r. 130-162), `harness/nemici_tipo06a.c` (155 righe: r. 1-15, la coda, e r. 128-155 come
modello di `rst 38` con `switch`), `harness/nemici_tipo06b.c` (r. 1-60:
modello di modulo con `IXB`, riporto in `z->f` e `nemici_r_somma`), `decomp/ROUTINE.md` (solo r. 508-532:
sezione N7), `decomp/CENSIMENTO-NEMICI.md` (solo il paragrafo «Lotto N7
fatto», r. 438-443).

È vietato leggere `STATO.md`, `README.md`, `ROUTINE.md` per intero, gli
altri brief in `compiti/` e i moduli `nemici_*.c` non elencati sopra.

Generazione del disassemblato (da `/home/rufio72/OpenSolomonsKey-master`;
`awk` è mawk: il confronto è fra stringhe esadecimali minuscole):
```
cd decomp && awk 'BEGIN{n=split("4fef 5cfb",lo," "); split("4ff8 5fc7",hi," ")}
/^[A-Za-z_0-9]+:/ {lab=$0; next}
/;[0-9a-f][0-9a-f][0-9a-f][0-9a-f][ \t]*$/ {a=substr($0,length($0)-3); ok=0; for(i=1;i<=n;i++) if(a>=lo[i] && a<=hi[i]) ok=1; if(ok){ if(lab!="") print lab; print} lab=""; next}
{lab=""}' out/main64.dasm > out/nemici_n8.dasm; cd ..
```
Uscita attesa (misurata): `wc -l < decomp/out/nemici_n8.dasm` → `388`;
`grep -c 'ld a,r' decomp/out/nemici_n8.dasm` → `2` (5DDF e 5DE2). Il file è
ignorato da git (`decomp/out/*`): si aggiunge al commit con `git add -f`.

## Obiettivo

Tradurre in C le 15 routine del lotto N8 (5CFB, 5D29, 5D76, 5DDF, 5E09, 5E1A,
5E4D, 5E60, 5E8F, 5E9D, 5EDD, 5F0C, 5F33, 5F4B, 5F82: 677 byte) più due
dipendenze piccole anticipate da N10 e N11 (4FEF, 10 byte; 5FAC, 28 byte),
in un modulo nuovo `nemici_tipo07a.c`; togliere lo stub di 5CFB; portare 14
voci nel banco con 84 catture MAME. Replay e `sched_proto` non devono
cambiare (misurato: il replay non raggiunge 5CFB, altrimenti lo stub avrebbe
fermato la spedizione; finisce al frame 5499 con exit 0).

Fuori lotto, che restano stub (nuovi): **610C** (chiamata da 5CFB, spedisce
con `rst 38` a 12 routine di N11) e **5FC8** (destinazione 2 della tabella di
5E1A). **627F** (N11), tail-call di 5E9D, resta non tradotta: si chiama
come `sub_627f(z)` dichiarata in `nemici.h` e coperta da un terzo stub (vedi
sotto).

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `cattura.sh`, `snap.lua`, `sessioni.sh`, `replay.*`, `sched*.c`,
  `rst20*`, `rst38_tables.h`, `censimento_nemici.py`, `STATO.md`,
  `README.md`, `compiti/` (tranne questo file non si scrive nulla), né le
  catture esistenti in `decomp/out/`.
- Niente installazioni. MAME gira solo come fa `cattura.sh` (senza finestra,
  senza audio, provider a `none`): niente partite a mano, niente giri su
  tutte le stanze. Una cattura dura 3-100 secondi (si possono lanciare fino a
  6 in parallelo con `xargs -P6`, file di uscita distinti; attenzione: `xargs`
  toglie le virgolette, per `SNAP_COND` e `SNAP_FORZA` con virgole usare un
  ciclo `for` o `sh -c`).
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe e nessun `nemici*.c` sopra le 700.
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`); si
  ripristina un file solo con `git checkout -- <file>`.
- Un solo agente alla volta su `decomp/harness`.

## Cosa esiste già (fatti verificati)

Misure del 2026-10-30 sui binari già compilati (da `decomp/harness`):
- `./harness --tutte`: exit 0, 706 righe `^cattura`, 0 `FALLITO`, 214 righe
  `^RIEPILOGO`. Replay: exit 0, fino al frame 5499. `sched_proto`:
  `106124, 0, 131`.
- `grep -c STUB_NEMICI nemici_stub.c` → 15 (14 stub più la riga `#define`):
  `STUB_NEMICI(5cfb)` è l'unico di questo lotto.
- Nessun `nemici_tipo07*.c` esiste. Il Makefile prende `nemici*.c` con un
  `wildcard` (sia `harness` sia `replay`): nessuna modifica al Makefile.
- Stima: 705 byte × 0,6 = circa 420 righe per `nemici_tipo07a.c` (sotto
  le 700). Se a lavoro finito supera 650 righe, spostare 5F0C, 5F33, 5F4B,
  5F82 e 5FAC in `nemici_tipo07ab.c` (intestazione e `IXB/IYB` ripetuti).
- `nemici.c` r. 66 chiama già `sub_5cfb(z)` dal `switch` di `salta_4f1b`
  (4F0F, tipo 07): non cambia. Il solo chiamante di 5CFB è quello.

Grafo delle chiamate (da `out/censimento_nemici.txt` e dal dasm; `(t)` =
già tradotta con prototipo in `routine.h`/`nemici.h`):

| ingresso | byte | chiamanti | chiama |
|---|---:|---|---|
| 5CFB | 46 | 4F0F (`rst 38`) | 52FD(t), 5D29, 610C (stub), 4FEF, 5E1A |
| 5D29 | 172 | 5CFB | `rst 8`(t), 4AF6(t), 5D76 |
| 5D76 | 10 | 5D29 (due volte) | - |
| 5DDF | 42 | 5E1A | 2 × `ld a,r`, 5E09 (due volte) |
| 5E09 | 17 | 5DDF | - |
| 5E1A | 43 | 5CFB | 5DDF, `rst 38` a 5E40: 5E60, 5E9D, 5FC8 (stub), 5F33, 5EDD, 5EDD |
| 5E4D | 15 | 5E60, 5E9D, 5EDD, 5F33 (e 5FC8, 61xx, fuori lotto) | - |
| 5E60 | 47 | 5E1A | 4AF6(t), 5E4D |
| 5E8F | 14 | 5E9D, 5EDD, 5F4B, 5F82 | - |
| 5E9D | 64 | 5E1A | 5E8F, 36F0(t), 5E4D, tail 627F (non tradotta) |
| 5EDD | 47 | 5E1A | 5E8F, 5E4D |
| 5F0C | 39 | 5F33 (e 5FC8) | 36F0(t) |
| 5F33 | 37 | 5E1A | 5F0C, 5F4B, 5F82, 5E4D |
| 5F4B | 42 | 5F33 | `rst 8`(t), 5E8F, tail 5FAC |
| 5F82 | 42 | 5F33 | `rst 8`(t), 5E8F, tail 5FAC |
| 4FEF | 10 | 5CFB, 5FAC (e 4FD4, 51B5, non tradotte) | `rst 8`(t) |
| 5FAC | 28 | 5F4B, 5F82 (e 5FDC, 6046, fuori lotto) | 4FEF |

Già tradotte e da usare: `sub_0008` (DE = A, HL += 2·A), `sub_4af6` (A, BC, E
di ingresso; inizializza IX), `sub_36f0` (routine.h r. 76), `sub_52fd`
(nemici.h r. 194). Le tabelle ROM 84C8 (5D29), 608C (5F4B), 60AC (5F82) e
4FF9 (4FEF) si leggono con `leggi()`. `rst38_5E40` (6 voci: 5E60, 5E9D,
5FC8, 5F33, 5EDD, 5EDD) è già in `rst38_tables.h`.

**Il riporto.** Dove una routine lascia un riporto che il chiamante legge
si usa `NEMICI_PONI_RIPORTO(z, c)` / `NEMICI_RIPORTO_ACCESO(z)`
(nemici.h r. 16-19). Casi di questo lotto, ricavati dal dasm:
- 5E09 esce con riporto = (A < 0x18) (`cp 018h`); 5DDF lo legge a 5DF3 e
  5E04. Dentro 5E09 il riporto di `sub b` (5E0F) entra in `rra` (5E10) e
  quello di `rra` entra nel `rla` (5E12): scriverlo con una variabile `c`.
- 5D29, a 5D4E-5D54: il riporto di `rra` attraversa `dec a` (che non lo
  tocca) ed entra in `rla` a 5D54. Il riporto entrante in `rra` è 0: viene
  dall'ultimo `add hl,de` (5D45), con HL = 84C8 + 4·A e A al più 0xFF:
  mai oltre 0xFFFF.
- 5CFB, a 5D1E: `sbc hl,bc` ha riporto entrante 0 (l'ultimo flag toccato è
  l'`add hl,de` di `rst 8` in 4FEF, senza riporto): HL = (IX+0B/0C) − BC, e
  si esce senza altro (HL resta la differenza) se HL < BC.
- 5E4D, 5E8F, 5F0C, 5F4B, 5F82: i riporti sono tutti locali alla routine.

**Registro R (5DDF).** 5DDF fa `ld a,r; ld c,a; ld a,r; add a,c; ld d,a`: stessa
coppia di letture di 54C5, quindi `nemici_r_somma()` dà A = D = R1 + R2
(R2 = R1 + 3 sui 7 bassi). Ma C resta uguale a R1 sui percorsi che escono
dal primo `ret nz` (5DFA) senza ricaricare C. `nemici_r_somma` non dà R1:
aggiungere in `nemici_r.c` e dichiarare in `nemici.h`
`uint8_t nemici_r_somma_r1(uint8_t *r1);` (restituisce la somma e scrive
R1), e far chiamare `nemici_r_somma` a quella. Nel banco R1 è il valore
trovato dalla ricerca; fuori dal banco (replay) vale 0. Regola ratificata:
niente R nelle catture; la ricerca è nei wrapper `banco_r_*` di
`nemici_r.c` (r. 99-162: `prova_con_r(z, "0xXXXX", sub_xxxx)`). 5E1A chiama 5DDF
quando il bit 5 di (IX+0) è spento: servono due wrapper, `banco_r_5ddf` e
`banco_r_5e1a`, dichiarati in `nemici.h` (sezione «Il registro R») e usati
come funzioni di `routine_tab.c` al posto di `sub_5ddf` e `sub_5e1a`. Le
catture di 5CFB nel banco non toccano 5E1A (percorso del bit 2 acceso),
quindi 5CFB non ha wrapper. Aggiornare il commento d'intestazione di
`nemici_r.c` (ora dice «quattro wrapper»: sono sette).

Nell'originale 5DDF e 5E1A non usano altro dal registro. Le routine del
lotto non leggono R altrove (`grep -c 'ld a,r'` → 2).

### Convenzioni di traduzione

- Ogni routine è `void sub_xxxx(struct z80 *z)`, tutte non statiche.
  Registri `z->a b c d e h l`, `z->ix` (slot, c411), `z->iy` (oggetto, c79e),
  `z->f`. Memoria con `M(indirizzo)`; ROM con `leggi(indirizzo)`; coppie con
  `BC(z) DE(z) HL(z)` e `SET_BC SET_DE SET_HL`. Definire in testa
  `#define IXB(o) M(z->ix + (o))` e `IYB(o)` (come `nemici_tipo06b.c`).
- Il banco confronta RAM (c000-e5ff, salvo 0x40 byte sotto SP) e A, BC, DE,
  HL, IX, IY all'uscita, non F: anche i registri «sporchi» devono coincidere
  (in particolare B, C, DE e HL lasciati dalle sottoroutine). Le catture di
  `rst 38` lasciano DE = sito + 2 + 2·E (lo fa `nemici_rst38`).
- `rst 38` a 5E40: `nemici_rst38(z, 0x5E40, rst38_5E40, 6)`, poi `switch` sulla
  destinazione come in `nemici_tipo06a.c` r. 139-155: `0x5E60`, `0x5E9D`,
  `0x5F33`, `0x5EDD` chiamano la routine; `0x5FC8` chiama lo stub
  `sub_5fc8(z)`; `0` = indice fuori tabella (non fare nulla); il resto
  `nemici_manca(dest)`. I `rst 8` sono `sub_0008(z)`.
- Le due code `jr z,5FAC` (5F57 in 5F4B e 5F8E in 5F82) e il `jp 627F`
  (5EDA in 5E9D) sono salti di coda: si traducono `sub_5fac(z); return;` e
  `sub_627f(z); return;`. 627F finisce con `pop hl; ret` (scarta un
  indirizzo di ritorno): è la ragione per cui non si traduce qui; il
  chiamante-C non fa nulla di speciale, lo gestirà N11.
- A 5E5C (`ld (0000h),a`) l'originale scrive in ROM: nessun effetto, non
  si scrive `M(0)`; A resta il risultato di `sub 0ch`.
- Catture `forza*` valgono come catture OK ma non contano per lo stato
  `verificata`, che richiede almeno 2 catture non forzate. Una cattura con
  `SNAP_COND` (filtro dell'occorrenza, nessuna modifica di stato) è non
  forzata (precedenti: 4E87, 4649).

## Struttura da realizzare

**1. `harness/nemici_tipo07a.c`** (nuovo): intestazione in italiano (lotto N8,
brief 34; 4FEF ne è fuori e sta in `nemici_tipo06a.c`; 5FAC anticipata da N11),
`#include "routine.h"`, `"nemici.h"`, `"rst38_tables.h"`, poi in ordine
libero (i prototipi stanno in `nemici.h`): `sub_5cfb`, `sub_5d29`,
`sub_5d76`, `sub_5ddf`, `sub_5e09`, `sub_5e1a`, `sub_5e4d`, `sub_5e60`,
`sub_5e8f`, `sub_5e9d`, `sub_5edd`, `sub_5f0c`, `sub_5f33`, `sub_5f4b`,
`sub_5f82`, `sub_5fac`. Una funzione per ingresso (5E4D, 5E8F, 5F0C sono
sottoroutine con un solo ingresso: funzioni normali). Commenti come nei
moduli esistenti: indirizzo del dasm accanto a ogni gruppo di istruzioni.
Uscite delle sottoroutine con valore (A, E di 5E8F; A di 5F0C e 5D76; B, C
di 5F4B e 5F82): in `z->a`, `z->e`, `z->b`, `z->c` come nell'originale.

**2. `harness/nemici_tipo06a.c`**: in coda `sub_4fef` (anticipata da N10, 10
byte: A − 0x10, HL = 4FF9 + 2·A tramite `sub_0008`, BC = parola letta, HL
sul secondo byte). Aggiornare l'intestazione (una riga: «Lotto N8 (brief
34): 4FEF, che 5CFB e 5FAC richiedono»).

**3. `harness/nemici_stub.c`**: togliere `STUB_NEMICI(5cfb)`; aggiungere
`STUB_NEMICI(5fc8)`, `STUB_NEMICI(610c)` e `STUB_NEMICI(627f)` (in ordine di
indirizzo, con le altre). Il commento di intestazione resta.

**4. `harness/nemici.h`**: togliere `sub_5cfb` dalla sezione «Stub» (r. 253);
aggiungere `sub_5fc8`, `sub_610c`, `sub_627f` alla stessa sezione. Dopo la
sezione N7 (r. 232-238) una sezione «Lotto N8 (brief 34)» con i prototipi
delle 16 funzioni di `nemici_tipo07a.c` più `sub_4fef`. Nella sezione del
registro R: `nemici_r_somma_r1`, `banco_r_5ddf`, `banco_r_5e1a`.

**5. `harness/nemici_r.c`**: `nemici_r_somma_r1`, due wrapper in coda
(`banco_r_5ddf` → `prova_con_r(z, "0x5DDF", sub_5ddf)`; `banco_r_5e1a` →
`"0x5E1A"`, `sub_5e1a`), commento aggiornato.

**6. `harness/routine_tab.c`**: 14 voci, in ordine di indirizzo:
`{ "0x4FEF", sub_4fef, 0, 0 }` fra `0x4F40` e `0x5068`; dopo `0x5C78`:
`0x5CFB`, `0x5D29`, `0x5D76`, `0x5DDF` (`banco_r_5ddf`), `0x5E09`, `0x5E1A`
(`banco_r_5e1a`), `0x5E4D`, `0x5E8F`, `0x5E9D`, `0x5EDD`, `0x5F0C`, `0x5F33`,
`0x5F4B` (tutte `sub_xxxx`, `0, 0`). 5E60, 5F82 e 5FAC non hanno voce: nessun
ingresso catturabile (vedi sotto).

**7. Catture.** Da `decomp/`. Misurate il 2026-10-30 in una copia di lavoro con
questi stessi comandi (snap.lua originale): tutte riescono (84 `_regs.txt`).
Per ognuna ho anche misurato, con un breakpoint su 610C, 5FC8, 5FAC, 627F e
4FEF durante la finestra ingresso→ritorno, che **non entra in nessuno stub**
(solo le quattro 5F4B che passano da 5FAC/4FEF, anticipate qui).
```
cd decomp
sh cattura.sh 0x4FEF finale 1; sh cattura.sh 0x4FEF stanza50 1
for s in finale stanza50; do
  SNAP_COND='(b@c411)&4' sh cattura.sh 0x5CFB $s 1 c
  SNAP_COND='(b@c411)&4' sh cattura.sh 0x5CFB $s 2 c2
  for e in 5D29 5D76 5E4D 5E8F 5F0C 5F33 5F4B; do
    sh cattura.sh 0x$e $s 1; sh cattura.sh 0x$e $s 2 n2
  done
done
sh cattura.sh 0x5DDF finale 1; sh cattura.sh 0x5DDF finale 2 n2
for n in 1 2 3; do sh cattura.sh 0x5E09 finale $n $( [ $n = 1 ] || echo n$n ); done
for n in 1 2 3 4; do sh cattura.sh 0x5E1A finale $n $( [ $n = 1 ] || echo n$n ); done
sh cattura.sh 0x5E1A stanza50 1; sh cattura.sh 0x5E1A stanza50 2 n2
sh cattura.sh 0x5E9D finale 1; sh cattura.sh 0x5E9D finale 4 n4
sh cattura.sh 0x5EDD finale 1; sh cattura.sh 0x5EDD finale 2 n2
```
Sono 49 catture non forzate (`4FEF` 2, `5CFB` 4, `5D29` 4, `5D76` 4, `5DDF` 2,
`5E09` 3, `5E1A` 6, `5E4D` 4, `5E8F` 4, `5E9D` 2, `5EDD` 2, `5F0C` 4, `5F33`
4, `5F4B` 4). Le 4 di 5CFB hanno il bit 2 di (IX+0) acceso (`SNAP_COND`):
percorrono 5D29; **tutte le altre occorrenze di 5CFB (N=1-4 senza filtro)
passano per 610C** e non sono usabili finché N11 non traduce 610C. Lo stato
`finale` parte dal frame 2300, `stanza50` dal 1951. 5E9D finale N=1 e N=4
sono le sole occorrenze pulite (la N=3 passa da 627F e non va fatta).

Catture **forzate**, stato `finale`, N=1, `SNAP_FORZA` applicato all'ingresso
(sintassi di `snap.lua`, valori esadecimali); tag `forzaNN`. Tutte
riescono (35). Il ramo aperto è dedotto dal dasm (attesa, non misurata
l'apertura; misurato che la cattura riesce e che non entra negli stub):

| ingresso | tag | SNAP_FORZA | ramo aperto |
|---|---|---|---|
| 5E1A | forza01 | `iy+1d:0b` | (IY+1D) < 0C: `ld (0),a; ret` |
| 5E1A | forza02 | `iy+1d:0c,iy+1e:0,ix+04:00` | E=0, 5E60 con bit 7 spento → 5E4D |
| 5E1A | forza03 | `iy+1d:0c,iy+1e:0,ix+04:80` | E=0, 5E60 con bit 7 acceso → 4AF6 |
| 5E1A | forza04 | `iy+1d:0c,iy+1e:1` | E=1, 5E9D |
| 5E1A | forza05 | `iy+1d:0c,iy+1e:3,ix+05:00` | E=3, 5F33 → 5F75 → 5F82 |
| 5E1A | forza06 | `iy+1d:0c,iy+1e:3,ix+05:80` | E=3, 5F33 → 5F4B |
| 5E1A | forza07 | `iy+1d:0d,iy+1e:0` | E=4, 5EDD |
| 5E1A | forza08 | `iy+1d:0d,iy+1e:1` | E=5, 5EDD |
| 5F33 | forza01 / 02 | `ix+05:00` / `ix+05:80` | ramo 5F75 (5F82) / 5F4B |
| 5D29 | forza01 | `ix+0d:02` | (IX+0D) < 3: `ret c` |
| 5D29 | forza02 / 03 | `iy+1d:0,iy+1e:0,ix+0d:3` / `…iy+1e:1…` | 5DC3, bit 0 di (IY+1E) spento / acceso |
| 5D29 | forza04 | `iy+24:02,iy+1d:09,iy+1e:00,ix+0d:3` | 5D80, `ret c` a 5D8D |
| 5D29 | forza05 | `iy+24:02,iy+1d:0a,iy+1e:00,ix+0d:3` | 5D80, `ret z` a 5D95 |
| 5D29 | forza06 | `iy+24:02,iy+1d:0a,iy+1e:02,ix+0d:3` | 5D80 → 5D96 (4AF6, bit 0 di D spento) |
| 5D29 | forza07 | `iy+24:02,iy+1d:0b,iy+1e:01,ix+0d:3` | 5D80 → 5D96 (bit 0 di D acceso) |
| 5D29 | forza08 | `iy+24:02,iy+1d:0b,iy+1e:00,ix+0d:3` | 5D80 → 5D96 (A=(IY+1E)) |
| 5D29 | forza09 | `iy+24:04,iy+1d:0b,iy+1e:03,ix+0d:3` | percorso generale 5D40-5D75 |
| 5F4B | forza00-forza0f | `a:00` … `a:0f` (esadecimale, due cifre) | voce A della tabella 608C; per A = 00, 01, 04, 05 la coda 5FAC (+ 4FEF) |

Per 5D29 `forza01`: dopo = `AF 0293 BC 0300 DE 4F2B HL C081` (PC=5D07); per 5F4B
`forza00`: `AF 0108 BC 0C03 DE 0001 HL FFF8`, `forza0d`: `AF 2C3A BC 0400 DE 001C
HL 60A7`. Per i comandi usare un ciclo:
```
cd decomp
f() { SNAP_FORZA="$3" sh cattura.sh $1 finale 1 $2; }
f 0x5E1A forza01 iy+1d:0b
f 0x5E1A forza02 iy+1d:0c,iy+1e:0,ix+04:00
f 0x5E1A forza03 iy+1d:0c,iy+1e:0,ix+04:80
f 0x5E1A forza04 iy+1d:0c,iy+1e:1
f 0x5E1A forza05 iy+1d:0c,iy+1e:3,ix+05:00
f 0x5E1A forza06 iy+1d:0c,iy+1e:3,ix+05:80
f 0x5E1A forza07 iy+1d:0d,iy+1e:0
f 0x5E1A forza08 iy+1d:0d,iy+1e:1
f 0x5F33 forza01 ix+05:00
f 0x5F33 forza02 ix+05:80
f 0x5D29 forza01 ix+0d:02
f 0x5D29 forza02 iy+1d:0,iy+1e:0,ix+0d:3
f 0x5D29 forza03 iy+1d:0,iy+1e:1,ix+0d:3
f 0x5D29 forza04 iy+24:02,iy+1d:09,iy+1e:00,ix+0d:3
f 0x5D29 forza05 iy+24:02,iy+1d:0a,iy+1e:00,ix+0d:3
f 0x5D29 forza06 iy+24:02,iy+1d:0a,iy+1e:02,ix+0d:3
f 0x5D29 forza07 iy+24:02,iy+1d:0b,iy+1e:01,ix+0d:3
f 0x5D29 forza08 iy+24:02,iy+1d:0b,iy+1e:00,ix+0d:3
f 0x5D29 forza09 iy+24:04,iy+1d:0b,iy+1e:03,ix+0d:3
for v in 0 1 2 3 4 5 6 7 8 9 a b c d e f; do f 0x5F4B forza0$v a:0$v; done
```
Non fare la forzata di 5E1A con `iy+1e:2` (E=2): chiama 5FC8, ancora stub.
5E60, 5F82 e 5FAC non hanno un ingresso catturabile: 5E60 e 5F82 non sono
mai raggiunte da MAME dallo stato `finale` o `stanza50` se non forzando 5E1A
o 5F33 (una `poke` di (IY+1D/1E) non ha dato una cattura), 5FAC solo da 5F4B
e 5F82. Sono coperte dalle forzate sopra e il loro stato è `tradotta`.
Totale: 49 + 35 = 84 catture, 14 voci.

**8. Copertura del C** (gcov, fuori dal repository; prefisso `h-`):
```
D=/tmp/cov34 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
 && ln -s "$PWD/decomp/out" $D/out && (cd $D && gcc -std=c11 -O0 --coverage -o h \
 harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c z80_*.c \
 nemici*.c attivita.c attivita_01_10.c && ./h --tutte > /dev/null; \
 gcov h-nemici_tipo07a.gcda > /dev/null 2>&1; \
 awk -F: '$1 ~ /#####/' nemici_tipo07a.c.gcov)
```
Nel rapporto: le righe non eseguite e il motivo. Attese e note (stima, non
misurata): il ramo di 5CFB a 5D08-5D28 (res 4, 610C, 4FEF, sbc, set 4, 5E1A:
fino a 10 righe), la chiamata a `sub_5fc8` nello `switch` di 5E1A (1) e a
`sub_627f` in 5E9D (1-2). Altre righe scoperte: aggiungere una forzata (stesso
schema, tag `forzaNN` successivo, stato `finale`, N=1, ingresso della routine
con registri o byte scelti per aprire il ramo) solo se il ramo è raggiungibile
senza passare dagli stub; altrimenti elencarle.

**9. Documenti.** `ROUTINE.md`: in fondo (dopo la r. 532), sezione
`## Motore dei nemici (lotto N8)` con un paragrafo (modulo
`nemici_tipo07a.c`, 4FEF in `nemici_tipo06a.c`, wrapper `banco_r_5ddf` e
`banco_r_5e1a`, stub 610C/5FC8/627F, ramo 5D08-5D28 di 5CFB non verificato
finché N11 non traduce 610C, 5E60/5F82/5FAC `tradotta` senza ingresso
proprio) e la tabella come quella della N7 (ingresso, funzione, file, byte,
descrizione, stato, catture OK, stati, note), una riga per routine (17),
più l'elenco delle forzate. Stato `verificata` per le 14 con voce nel banco;
`tradotta` per 5E60, 5F82 e 5FAC. `CENSIMENTO-NEMICI.md`: dopo il paragrafo
«Lotto N7 fatto» un paragrafo «**Lotto N8 fatto** (brief 34)» con le 15
routine, le 2 anticipate (4FEF da N10, 5FAC da N11), gli stub di
`nemici_stub.c` da 14 a 16 (5cfb tolto; 610c, 5fc8 e 627f aggiunti). Le righe della tabella dei moduli non
cambiano; N10 e N11 non rifanno 4FEF e 5FAC.

## Verifiche obbligatorie

Tutte da `/home/rufio72/OpenSolomonsKey-master`; uscita attesa esatta salvo
dove è scritto «attesa, dedotta». Abbreviazione:
`V="4FEF 5CFB 5D29 5D76 5DDF 5E09 5E1A 5E4D 5E8F 5E9D 5EDD 5F0C 5F33 5F4B"`.

1. Disassemblato: `wc -l < decomp/out/nemici_n8.dasm; grep -c 'ld a,r' decomp/out/nemici_n8.dasm`
   → `388` e `2` (misurate).
2. Compilazione: `cd decomp/harness && make harness replay sched_proto > /tmp/m34.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/m34.txt`
   → `make 0` e `0`.
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h34.txt; echo "exit $?"; grep -c FALLITO /tmp/h34.txt; grep -c '^RIEPILOGO' /tmp/h34.txt`
   → `exit 0`, `0`, `228` (214 + 14 voci; attesa, dedotta).
4. Catture per voce: `for a in $V; do grep "^RIEPILOGO 0x$a:" /tmp/h34.txt; done`
   → (attese, dedotte dai file elencati sopra)
   ```
   RIEPILOGO 0x4FEF: 2 catture, 2 OK
   RIEPILOGO 0x5CFB: 4 catture, 4 OK
   RIEPILOGO 0x5D29: 13 catture, 13 OK
   RIEPILOGO 0x5D76: 4 catture, 4 OK
   RIEPILOGO 0x5DDF: 2 catture, 2 OK
   RIEPILOGO 0x5E09: 3 catture, 3 OK
   RIEPILOGO 0x5E1A: 14 catture, 14 OK
   RIEPILOGO 0x5E4D: 4 catture, 4 OK
   RIEPILOGO 0x5E8F: 4 catture, 4 OK
   RIEPILOGO 0x5E9D: 2 catture, 2 OK
   RIEPILOGO 0x5EDD: 2 catture, 2 OK
   RIEPILOGO 0x5F0C: 4 catture, 4 OK
   RIEPILOGO 0x5F33: 6 catture, 6 OK
   RIEPILOGO 0x5F4B: 20 catture, 20 OK
   ```
5. Catture non forzate per voce: `cd decomp && for a in $V; do ls out/snap_0x${a}_*_regs.txt | grep -vc forza; done | tr '\n' ' '`
   → `2 4 4 4 2 3 6 4 4 2 2 4 4 4 ` (misurate nella copia di lavoro).
6. Totale file: `cd decomp && ls out/snap_0x{4FEF,5CFB,5D29,5D76,5DDF,5E09,5E1A,5E4D,5E8F,5E9D,5EDD,5F0C,5F33,5F4B}_*_regs.txt | wc -l`
   → `84` (misurata). Totale catture del banco: `grep -c '^cattura' /tmp/h34.txt` → `790` (706 + 84; attesa, dedotta).
7. Voci precedenti invariate: `for a in 4767 4E87 3569 7379 6C2C 54C5 51FA; do grep "^RIEPILOGO 0x$a:" /tmp/h34.txt; done`
   → `4767: 22 catture, 22 OK`, `4E87: 2…2 OK`, `3569: 2…2 OK`, `7379: 2…2 OK`,
   `6C2C: 2…2 OK`, `54C5: 2…2 OK`, `51FA: 30 catture, 30 OK` (misurate prima della modifica).
8. `sched_proto`: `cd decomp && harness/sched_proto out/sched_log.txt | tail -1`
   → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
9. Replay: `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r34.txt; echo "exit $?"; tail -4 /tmp/r34.txt`
   → `exit 0` e (misurate):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
10. Stub: `grep -c STUB_NEMICI decomp/harness/nemici_stub.c; grep -c 5cfb decomp/harness/nemici_stub.c; grep -c -E '610c|5fc8|627f' decomp/harness/nemici_stub.c`
    → `17`, `0`, `3` (attese, dedotte: oggi 15 = 14 stub + `#define`; si
    toglie 1 e si aggiungono 3: 16 stub + `#define`).
11. Copertura: il comando del punto 8 di «Struttura». Uscita attesa (attesa,
    non misurata): al più 14 righe `#####`, tutte spiegate nel rapporto.
    Controllo: `… | wc -l` → numero ≤ 14.
12. Dimensioni: `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/nemici*.c | awk '$1>700 && $2!="totale" && $2!="total"'` → vuoto.
13. File vietati intatti: `git diff --name-only HEAD | grep -E 'cattura\.sh|snap\.lua|sessioni|replay\.|sched|rst20|rst38_tables|censimento_nemici\.py|STATO\.md|README\.md|compiti/'` → vuoto.
14. Ambito: `git status --short | grep -v -E '^( M|\?\?|A ) (compiti/34-|decomp/(harness/(nemici_tipo07a\.c|nemici_tipo06a\.c|nemici\.h|nemici_stub\.c|nemici_r\.c|routine_tab\.c)|ROUTINE\.md|CENSIMENTO-NEMICI\.md|out/(snap_0x(4FEF|5CFB|5D29|5D76|5DDF|5E09|5E1A|5E4D|5E8F|5E9D|5EDD|5F0C|5F33|5F4B)_|nemici_n8\.dasm)))'`
    → vuoto; `git status --short | grep -c -E '\.(gcda|gcno)$'` → `0`.
15. Documenti: `grep -c '^## Motore dei nemici (lotto N8)$' decomp/ROUTINE.md; grep -c 'Lotto N8 fatto' decomp/CENSIMENTO-NEMICI.md`
    → `1` e `1`.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto N8 del motore dei nemici: tipo 07 prima metà (5CFB-5F82, 4FEF, 5FAC), catture nel banco`
  e riga finale `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`
  (se il lotto passa a Opus per escalation, `Claude Opus 5.5`). Il commit
  comprende `decomp/out/nemici_n8.dasm` (`git add -f`) e le catture nuove
  (`prima.bin`, `dopo.bin`, `regs.txt`, `.log`; con `git add -f`).
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-15; l'elenco
  delle voci con numero di catture e stato; le righe gcov non coperte; le
  deviazioni dal brief (cattura che fallisce: se per un diverso numero di
  occorrenza si è dovuto cambiare N o si è scartata una cattura, dirlo).

## Domande aperte

1. **Perimetro**: il lotto prende le 15 routine più 4FEF e 5FAC (38 byte
   anticipati); 610C (e le 11 routine di N11 che spedisce), 5FC8 (con 5FDC e
   6046) e 627F restano stub. Alternativa: accorpare N8 e N11 in un solo
   lotto (1305 byte, circa 780 righe, due moduli). Proposta: come scritto;
   N11 rifà i test di 5CFB sul ramo 610C (le occorrenze N=1-4 di 5CFB senza
   filtro in `finale` e `stanza50` lo percorrono e hanno già misurato che
   toccano 610C e 4FEF) e traduce 627F con la sua semantica `pop hl; ret`. —
   «scelta di progetto»
2. **Stato di 5CFB**: `verificata` (4 non forzate) benché il ramo 5D08-5D28
   non sia nel banco. Proposta: sì, con la nota in `ROUTINE.md`. — «preferenza»
3. **Coerenza col brief 28** (non eseguito): lì 5DDF e 5E09 sono anticipate in
   `nemici_tipo07a.c` e R entra nelle catture (`z->r`, `R=` in `snap.lua`);
   qui 5DDF/5E09 si fanno col wrapper (regola ratificata: niente R nelle
   catture). Proposta: lo sviluppatore non tocca il brief 28; la chat
   principale lo annota (5DDF e 5E09 fatte in N8, `nemici_tipo07a.c` esiste
   già, niente `z->r`). — «preferenza»
4. **`nemici_r_somma_r1`**: serve R1 perché 5DDF può uscire con C = R1.
   Proposta: la funzione nuova, `nemici_r_somma` come involucro. — «preferenza»
5. **5E60, 5F82, 5FAC senza voce di banco** (nessun ingresso catturabile).
   Proposta: `tradotta`, coperte dalle forzate di 5E1A, 5F33 e 5F4B. —
   «preferenza»
6. **Tre stub nuovi** (610C, 5FC8, 627F) al posto di quello di 5CFB. Proposta:
   sì; il replay non li raggiunge (misurato). — «preferenza»
7. **Spezzatura**: nessuna prevista (circa 420 righe). Proposta: se oltre 650,
   `nemici_tipo07ab.c` come descritto. — «preferenza»
8. **`STATO.md` e `README.md`**: li aggiorna la chat principale dopo la
   verifica. — «preferenza»

## Risoluzioni (2026-10-30)

1. **Come scritto**: N8 da solo, niente accorpamento con N11. L'utente ha
   già fissato il perimetro («vai con N8 poi ti fermi») e l'ordine
   N8 → N9 → N10 → N11+N12; il ramo 610C e 627F restano a N11.
2-8. Preferenze accolte con la proposta dell'analista. Per la 3: la
   coerenza col brief 28 la annota la chat principale a fine ciclo
   (5DDF e 5E09 fatte in N8, `nemici_tipo07a.c` esiste già, niente
   `z->r` nelle catture).

## Esito (2026-10-30)

Ciclo passato al primo colpo, sesto con lo schema economico (analista
Sonnet ~195k token, sviluppatore Sonnet ~100k, verifica a script senza
modello; ~1 $). Commit dello sviluppatore `7aaa4bd`. Verifiche 1-15
rieseguite dalla chat principale: tutte passate con le attese corrette
per le 6 catture forzate aggiunte dallo sviluppatore (banco a 796
catture e 228 voci, 90 file del lotto; 5DDF 3, 5E09 4, 5E1A 15, 5E9D 3,
5EDD 4). Unica eccezione la verifica 11: 27 righe `#####` contro il
tetto di 14, che però era «attesa, non misurata»; tutte spiegate in
`ROUTINE.md` (rami dietro gli stub 610C/5FC8/627F e ingressi non
catturabili), si chiudono con N11. Deviazione corretta in corso d'opera:
prima versione di 5EDD con A sbagliato (l'originale ricarica A da
(IY+1E) a 5EEC prima del `cp c`).
