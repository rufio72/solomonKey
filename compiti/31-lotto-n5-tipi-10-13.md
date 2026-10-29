# Compito 31: lotto N5 del motore dei nemici, tipi 10-13 (714B, 56E9, 55B2, 54D8)

Data: 2026-10-29. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere.

## Contesto: file da leggere

Da leggere (e basta):
1. `decomp/out/nemici_tipi_10_13.dasm`, da generare (vedi sotto): il
   disassemblato delle 14 routine del lotto.
2. `decomp/harness/rst38_tables.h`: solo `rst38_54DB` (r. 43-46) e
   `rst38_5704` (r. 53-56) (`rst38_55B5`, r. 48-51, è già usata).
3. `decomp/harness/nemici.h` (232 righe): prototipi, `nemici_rst38`,
   `NEMICI_PONI_RIPORTO`, `NEMICI_RIPORTO_ACCESO`.
4. Le convenzioni qui sotto («Convenzioni di traduzione»).

Da aprire perché si modificano, solo per la parte indicata:
`decomp/harness/nemici_tipi_10_13.c` (315 righe: r. 1-43 per intestazione e
stile del `switch` su `rst 38`, r. 190-210 per `sub_714b` e il `case 0x719E`,
r. 278-315 per 571D/57AA/57B3 già tradotte), `nemici_stub.c`,
`routine_tab.c`, `z80_6xxx.c` (solo `sub_6c2c`, r. 52-77), `ram.h` (macro,
r. 9-30), `nemici_tipo09.c` (solo `prendi_portato`, r. 25-34, esempio di
stile per `rst 8; rst 10h` con `push de; ex (sp),ix`), `decomp/ROUTINE.md`
(solo dalla r. 349 alla fine, formato della sezione N4),
`decomp/CENSIMENTO-NEMICI.md` (solo il paragrafo «Lotto N4 fatto», r. 413-420,
e le r. 465-470 della tabella dei moduli).

È vietato leggere `STATO.md`, `README.md`, `ROUTINE.md` per intero, gli
altri brief in `compiti/` e i moduli `nemici_*.c` diversi da quelli elencati.

Generazione del disassemblato (da `/home/rufio72/OpenSolomonsKey-master`,
`awk` è mawk: il confronto è fra stringhe esadecimali minuscole):
```
cd decomp && awk 'BEGIN{n=split("54d8 54ea 551a 5529 5584 55c4 56e9 5713 5726 573c 5749 6bcd 6c1d 719e",lo," "); split("54db 5506 5528 5583 55b1 55c8 5704 571c 573b 5748 57a9 6c0a 6c2b 7234",hi," ")}
/^[A-Za-z_0-9]+:/ {lab=$0; next}
/;[0-9a-f][0-9a-f][0-9a-f][0-9a-f][ \t]*$/ {a=substr($0,length($0)-3); ok=0; for(i=1;i<=n;i++) if(a>=lo[i] && a<=hi[i]) ok=1; if(ok){ if(lab!="") print lab; print} lab=""; next}
{lab=""}' out/main64.dasm > out/nemici_tipi_10_13.dasm; cd ..
```
Uscita attesa (misurata): `wc -l < decomp/out/nemici_tipi_10_13.dasm` → `272`.
Il file è ignorato da git (`decomp/out/*`): si aggiunge al commit con
`git add -f`. Le tabelle inline 54DC-54E9 e 5705-5712 non sono nel file (sono
in `rst38_tables.h`); 6BCD ha la sola etichetta `sub_6bcdh:`.

## Obiettivo

Tradurre in C le 14 routine ancora mancanti del lotto N5 (tipi 10-13 di
4F1B: ingressi 714B, 56E9, 55B2, 54D8). 571D, 57AA, 57B3 (N2) e 55B2, 55C9,
561B, 5640, 5693, 714B, 7155, 7235 (N1) sono già tradotte e non si toccano.
Verifica: catture MAME nel banco (`harness --tutte`) con `cattura.sh` (stati
`stanzaNN`, `finale`) e, dove un ramo non si vede, `SNAP_FORZA`. Replay e
`sched_proto` non devono cambiare.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `cattura.sh`, `snap.lua`, `sessioni.sh`, `replay.*`, `sched*.c`,
  `rst20*`, `rst38_tables.h`, `censimento_nemici.py`, `STATO.md`,
  `README.md` né le catture esistenti in `decomp/out/`.
- Niente installazioni. MAME gira solo come fa `cattura.sh` (senza finestra,
  senza audio, provider a `none`): niente partite a mano. Una cattura dura
  alcuni secondi; non lanciare giri su tutte le stanze.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe e nessun `nemici*.c` sopra le 700.
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`); si
  ripristina un file solo con `git checkout -- <file>`.
- Un solo agente alla volta su `decomp/harness`.

## Cosa esiste già (fatti verificati)

Misure del 2026-10-29 sui binari già compilati (da `decomp/harness`):
- `./harness --tutte`: exit 0, 539 righe `^cattura`, 0 `FALLITO`, 189 righe
  `^RIEPILOGO` (formato `RIEPILOGO 0x6EB7: 2 catture, 2 OK`).
- `grep -c STUB_NEMICI nemici_stub.c` → 22 (la `#define` più 21 righe, tra cui
  `54d8`, `55c4`, `56e9`, `6c1d`, `719e`).
- Nessun file di codice di `decomp` (fuori da `out/`) sopra le 800 righe;
  nessun `nemici*.c` sopra le 700 (`nemici_tipi_08_14.c` ne ha 697).
- Nessuna routine del lotto contiene `ld a,r` (`grep -c 'ld a,r'` sul
  disassemblato → 0): non servono wrapper `banco_r_*`.
- Il Makefile prende `nemici*.c` con un `wildcard`: il modulo nuovo entra
  senza toccarlo.

Routine del lotto (da `CENSIMENTO-NEMICI.md`; colonna replay = NO per tutte:
gli stub non fermano il replay, che resta a exit 0). Il C esistente le chiama
già: `nemici.c` smista 54D8 e 56E9 da 4F1B (`case 0x54D8`, `0x56E9`),
`nemici_tipi_10_13.c` chiama 55C4 e 6C1D da 55B2 (r. 29-31) e 719E da 714B
(r. 199).

| ingresso | byte | chiamanti | note |
|---|---:|---|---|
| 54D8 | 4 | 4F0F (4F1B[13]) | tipo 13: `ld e,(iy+1d); rst 38` al sito 0x54DB su `rst38_54DB` (7 voci: 5584, 551A, 54EA, 3B62, 3B62, 5529, 3B62) |
| 54EA | 29 | 54D8[2] | `ld c,18; call 5507`; NC → `jp 561B` (esistente) |
| 551A | 15 | 54D8[1] | `ld a,2; call 6C2C; ret nc`; (IX+0D) ≥ 18 → `jp 80A5` |
| 5529 | 91 | 54D8[5] | tre chiamate a 5507; fase 0 con suono 27 (`rst 28`) |
| 5584 | 46 | 54D8[0] | chiama 7379 (slot libero) e 6BCD |
| 6BCD | 62 | 5584, 6B5F (N9, non tradotta) | `ld hl,48AF; rst 8; rst 10h; push de; ex (sp),ix`; chiama 4AF6 |
| 55C4 | 5 | 55B5[3] | `ld (iy+1d),2; ret` |
| 6C1D | 15 | 55B5[1] | `ld a,3; call 6C2C; ret nc`; (IX+0D) ≥ 0D → `jp 80A5` |
| 56E9 | 28 | 4F0F (4F1B[11]) | tipo 11: BC = (C1A2) o (C1A4) per il bit 3 di (IY+4); 57AA; `ld a,1; call nc,57B3`; `ld e,(iy+1d); rst 38` al sito 0x5704 su `rst38_5704` (7 voci: 571D, 5726, 573C, 573C, 3B62, 5749, 5713) |
| 5713 | 10 | 56E9[6] | |
| 5726 | 22 | 56E9[1] | |
| 573C | 13 | 56E9[2] e [3] | |
| 5749 | 97 | 56E9[5] | chiama 5928, 7379, 58CF, 5917, 6EB7 |
| 719E | 151 | 714B (`rst38_714E`[1]) | chiama 6EB7, 4AF6, 734D, 7379, 4938; `push iy; ex (sp),ix` |

Totale: 588 B (14 routine). Già tradotte e da usare (prototipi in
`routine.h` o `nemici.h`, da `grafo symbol`): `sub_0008`, `sub_0010`,
`sub_0028` (suono, preserva AF, BC, HL), `sub_3b62` (voce nulla),
`sub_4938`, `sub_4af6` (scrive (IX+04), +05=ff, +1B, +1C=ff, +1D=B, +1E=C,
+00=A; usa E, B, C, A), `sub_5507` (C = soglia, riporto in `z->f` bit 0; A
all'uscita), `sub_561b`, `sub_57aa` (HL = (IX+0B..0C) − BC − riporto,
riporto del prestito in `z->f`), `sub_57b3`, `sub_58cf` (riporto in uscita),
`sub_5917`, `sub_5928`, `sub_6eb7`, `sub_734d`, `sub_7379` (HL = slot libero,
A = indice, riporto in `z->f` bit 0), `sub_80a5`.

**6C2C non imposta il riporto.** Il disassemblato mostra che 551A e 6C1D
leggono il riporto di 6C2C (`ret nc`), ma `sub_6c2c` (`z80_6xxx.c`, r. 52-77,
verificata dal banco senza F) non lo scrive. Dal disassemblato (6C2C-6C5E,
da `grep ';6c' out/main64.dasm`): al primo passaggio (bit 5 di (IX+1) spento)
esce con riporto 0 (`and 04; rrca` su zero); al secondo, `rrca` su (IX+1)
mette il riporto = bit 0 di (IX+1) prima della modifica: se 0 → `set 0` e
riporto 0; se 1 → `rst 28` (preserva AF) e riporto 1. Quindi: riporto 1
se e solo se (IX+1) & 0x21 == 0x21 all'ingresso. Va aggiunto a `sub_6c2c`
con la convenzione di 5507: `z->f = (uint8_t)((z->f & ~1) | riporto)`
(riporto 0 sul primo ramo e sul `set 0`, 1 sul ramo del suono, scritto dopo
la chiamata a `sub_0028`). `sub_6c2c` non ha altri chiamanti; il banco non
confronta F, quindi le sue 2 catture restano OK.

### Convenzioni di traduzione

- Ogni routine è `void sub_xxxx(struct z80 *z)`. Registri `z->a b c d e h l`,
  `z->ix`, `z->iy` (uint16_t), `z->f`. Memoria con `M(indirizzo)` (RAM c000-ffff),
  coppie con `BC(z) DE(z) HL(z)` e `SET_BC SET_DE SET_HL` (`ram.h`).
- Il banco confronta RAM (c000-e5ff) e A, BC, DE, HL, IX, IY all'uscita, non F:
  anche i registri «sporchi» lasciati dal codice originale devono coincidere
  (per esempio C = 18 dopo 5507, A = (IY+1A) & 30 in uscita da 54EA). Il
  riporto, dove un chiamante lo legge (7379, 57AA, 58CF, 6C2C), sta nel bit 0
  di `z->f` (`NEMICI_PONI_RIPORTO(z,c)`, `NEMICI_RIPORTO_ACCESO(z)`).
- `rst 38h` a un sito con tabella inline: `z->e = M(z->iy + 0x1d)` (il `ld e`
  precede il `rst`), poi `nemici_rst38(z, sito, tab, voci)` restituisce la
  destinazione (0 fuori tabella) e mette DE = sito+2+2E; HL e A restano
  intatti. Si smista con uno `switch` come in `sub_55b2` (r. 22-43 di
  `nemici_tipi_10_13.c`); `sub_3b62` per le voci 3B62; le destinazioni
  ritornano al chiamante di 54D8/56E9 (sono `jp` impliciti). Siti qui:
  0x54DB con `rst38_54DB` (7 voci) e 0x5704 con `rst38_5704` (7 voci).
- `jp`/`jr` verso un'altra routine = chiamata e `return` (`jp nc,561B` →
  `if (!riporto) { sub_561b(z); return; }`).
- `ld hl,48AF; rst 8; rst 10h` (6BCD, 7221): `sub_0008`, poi `sub_0010`, con
  `#define PORTATI_48AF 0x48af` locale in ogni modulo (come
  `nemici_tipo09.c` r. 18). `push de; ex (sp),ix` (6BCD): `ix0 = z->ix;
  z->ix = DE(z);` … `z->ix = ix0;` (`pop ix`), DE invariato. In 719E
  `push iy; ex (sp),ix` (71D8-71E6): `ix0 = z->ix; z->ix = z->iy;` … `z->ix =
  ix0;`; IY non cambia.
- `rst 28h` = `sub_0028` con `z->c` = codice suono (5565: C = 27).
- Letture di costanti: `ld bc,(c1a2)` (56E9) = `M(0xc1a2) | M(0xc1a3) << 8`;
  `ld hl,c08a; ld (hl),0` (7219) = `M(0xc08a) = 0`; `ld (c835),a` = `M(0xc835)`.
  `ld bc,0e01` (71F9) è un immediato (B = 0E, C = 01).
- Nessun `NEMICI_PUNTO`: il replay non raggiunge queste routine.
- Catture `forza*` valgono come catture OK ma non contano per lo stato
  `verificata`, che richiede almeno 2 catture non forzate.

## Struttura da realizzare

**1. Moduli C.** Stima a 0,6-0,68 righe per byte: 588 B danno 353-400 righe;
sommate alle 315 esistenti fanno 668-715, troppo vicino al limite di 700.
Quindi si divide per tema:
- `nemici_tipi_10_13.c` (315 righe, ~420-435 dopo): si aggiungono `sub_55c4`
  (tipo 12), `sub_6c1d` (tipo 12) e `sub_719e` (tipo 10, 151 B) con
  `#define PORTATI_48AF`. Si aggiorna il commento di intestazione (ingressi,
  e che il resto del lotto sta in `nemici_tipi_10_13b.c`).
- `nemici_tipi_10_13b.c` (nuovo, ~270-310 righe): tipo 13 e tipo 11 —
  54D8, 54EA, 551A, 5529, 5584, 6BCD, 56E9, 5713, 5726, 573C, 5749.
  Intestazione di commento in italiano (tema, ingressi, «IX = slot, IY =
  oggetto»), poi `#include "routine.h"`, `#include "nemici.h"`,
  `#include "rst38_tables.h"` e `#define PORTATI_48AF 0x48af`.
Se dopo la scrittura un file supera le 700 righe, si sposta 5749 o 5529 nel
modulo più corto.

**2. `z80_6xxx.c`**: il riporto di `sub_6c2c` (vedi sopra).

**3. `nemici.h`**: sezione «Lotto N5» con i prototipi di tutte e 14 le
routine (tutte non statiche: le usa `routine_tab.c`): 54D8, 54EA, 551A,
5529, 5584, 55C4, 56E9, 5713, 5726, 573C, 5749, 6BCD, 6C1D, 719E. Si
tolgono dal blocco degli stub i prototipi `sub_54d8`, `sub_55c4`,
`sub_56e9`, `sub_6c1d`, `sub_719e`. `nemici_stub.c`: si tolgono
`STUB_NEMICI(54d8)`, `(55c4)`, `(56e9)`, `(6c1d)`, `(719e)`. In `nemici.c`
non cambia niente.

**4. `routine_tab.c`**: una voce `{ "0xXXXX", sub_xxxx, 0, 0 }` per ognuna
delle 14 routine, ciascuna al suo posto in ordine di indirizzo (le voci
esistenti 0x5507, 0x571D, 0x6C2C… servono da riferimento). Nessuna voce per
le routine statiche: non ce ne sono in questo lotto.

**5. Catture.** Da `decomp/`. Almeno 2 catture non forzate per voce. Piano
(misurato il 2026-10-29: ogni cattura indicata riesce con
`sh cattura.sh <ENTRY> <stato> 1`; la seconda cattura di uno stato identico
usa `N=2` e il suffisso `n2`, altrimenti sovrascrive la prima):

| voci | catture |
|---|---|
| 0x54D8 0x54EA 0x5529 | `stanza02`, `stanza06` |
| 0x551A | `finale` e `sh cattura.sh 0x551A finale 2 n2` (nelle stanze non scatta; con `N=3` suffisso `n3` una terza, diversa) |
| 0x5584 | `stanza30` e `sh cattura.sh 0x5584 stanza30 2 n2` (unica stanza) |
| 0x55C4 | `stanza07`, `stanza10` |
| 0x56E9 0x5713 0x5726 0x573C 0x5749 | `stanza01`, `stanza06` |
| 0x6BCD | `stanza02`, `stanza11` |
| 0x6C1D | `stanza07`, `stanza19` (`stanza09` e `stanza12` non catturano) |
| 0x719E | `stanza02`, `stanza05` |

Le catture hanno «dopo» a PC=4EFB (ritorno a 4F0F); quelle di 6BCD a
PC=6B78 (ritorno a 6B5F). Dopo il primo giro si misura la copertura (punto
7) e si aggiungono forzature per i rami non eseguiti, con `SNAP_FORZA`
(voci `ix+HH:VV`, `iy+HH:VV`, registri `a..l:VV`; la forzatura è applicata
all'ingresso voluto, prima del dump, quindi vale per i rami decisi dentro la
routine catturata) e suffisso `forza` o `forzaNN`, es.
`SNAP_FORZA='iy+1d:01' sh cattura.sh 0x54D8 stanza02 1 forza01`.
Forzature **misurate** il 2026-10-29 (tutte eseguite; l'effetto è letto dal
confronto prima/dopo):

| voce, stato | SNAP_FORZA | ramo aperto |
|---|---|---|
| 54D8 stanza02 | `iy+1d:00` / `01` / `02` / `03` / `05` | 5584 (ret c) / 551A / 54EA (fase 5) / 3B62 / 5529 (ret c) |
| 54EA stanza02 | `ix+4:00,ix+5:00,iy+1e:00` | NC → 561B |
| 54EA stanza02 | `ix+4:00,ix+5:00,iy+1e:01,iy+1a:30` | fase 5, bit 1 di (IY+1E) acceso |
| 54EA stanza02 | `ix+4:00,ix+5:00,iy+1e:01,iy+1a:00` | fase 5, bit 1 spento |
| 551A finale | `ix+1:21,ix+d:20` / `ix+1:21,ix+d:00` | riporto, 80A5 / riporto, ret c (suono) |
| 551A finale | `ix+1:20` / `ix+1:00` | 6C2C senza riporto, ret nc, secondo / primo passaggio |
| 5529 stanza02 | `iy+1a:00,ix+4:7f` | 5573 (fase 2, set 4 di (IX+1)) |
| 5529 stanza02 | `iy+1a:00,ix+4:00,ix+5:00,iy+1e:01,ix+d:00` | 5549, ret c |
| 5529 stanza02 | `iy+1a:00,ix+4:00,ix+5:00,iy+1e:01,ix+d:ff,iy+4:00` | fine attesa 0x64, suono 27 |
| 5529 stanza02 | `iy+1a:00,ix+4:00,ix+5:00,iy+1e:01,ix+d:40,iy+4:10` | fine attesa 0x32, suono 27 |
| 5529 stanza02 | `iy+1a:30,ix+4:7f,iy+12:00` / `iy+12:01` | 5573 / ret nz |
| 5529 stanza02 | `iy+1a:30,ix+4:00,ix+5:00,iy+1e:01,ix+d:00` | 5567 → riporto → 5549, ret c |
| 5584 stanza30 | `iy+1e:00,ix+d:1c` | 7379 + 6BCD, bit 1 acceso |
| 5584 stanza30 | `iy+1e:00,ix+d:00` / `iy+1e:02,ix+d:00` | ret c |
| 5584 stanza30 | `iy+1e:02,ix+d:48` | 55A3: azzera (IX+0D), fase 2 |
| 6BCD stanza02 | `c:00` / `c:01` / `c:02` / `c:03` | le quattro combinazioni dei bit 0-1 di C |
| 6C1D stanza07 | `ix+1:21,ix+d:10` | riporto, 80A5 |
| 6C1D stanza07 | `ix+1:21,ix+d:00` / `ix+1:20` / `ix+1:00` | ret c con suono / ret nc / primo passaggio |
| 56E9 stanza01 | `iy+1d:00` … `iy+1d:06`, con `iy+4:00` | le sette voci di 5704 (4 è 3B62) |
| 56E9 stanza01 | `iy+1d:00,iy+4:08` | ramo (C1A4) |
| 56E9 stanza01 | `iy+1d:04,ix+b:ff,ix+c:ff,ix+1:00` | 57AA senza prestito → 57B3 (fase 0, azzera (IX+0B..0D)) |
| 5713 stanza01 | `iy+0:20` / `iy+0:00` | ret nz / fase 5 |
| 5726 stanza01 | `iy+4:08` / `iy+4:00` | senza / con azzeramento di (IX+0B..0C) |
| 5749 stanza01 | `iy+12:01` | res 6 di (IX+1) |
| 5749 stanza01 | `iy+12:00,ix+1:08,ix+d:00` / `…,ix+d:20` | ret c / 5917 e 6EB7 |
| 5749 stanza01 | `iy+12:00,ix+1:48` | 5917 e 6EB7 (bit 3 vince sul 6) |
| 5749 stanza01 | `iy+12:00,ix+1:00` | 5763: dipende dalla cella (5928), fase 2 |
| 719E stanza02 | `ix+1:00,ix+8:00` / `ix+1:00,ix+8:0b` | ret c / set 0 di (IX+1) |
| 719E stanza02 | `ix+1:05,ix+8:00` / `ix+1:05,ix+8:16` | ret c / senza 6EB7, oggetto riagganciato (4AF6) |
| 719E stanza02 | `ix+1:09,ix+8:16` | con 6EB7 (A = (IX+0E)), 4AF6 |
| 719E stanza02 | `ix+1:01,iy+1a:00` | 71D4, 4AF6 con E = (IY+4) |
| 719E stanza02 | `ix+1:01,iy+1a:01` (o `08`) | 734D, cella < F8, 7379, 4938, c835 = ff |
| 719E stanza02 | `ix+1:01,iy+1a:02` (o `04`) | 734D, cella ≥ F8: set 2 di (IX+1) |

Restano non forzabili (celle assolute o stato dei 17 slot): `ret nc` di 7379
(5590, 5772, 720D) e i rami di 5749 oltre 5763. Possono restare scoperti:
da elencare nel rapporto.

**6. Copertura del C** (gcov, fuori dal repository; il prefisso del `.gcda`
è `h-`):
```
D=/tmp/cov31 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
 && ln -s "$PWD/decomp/out" $D/out && (cd $D && gcc -std=c11 -O0 --coverage -o h \
 harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c z80_*.c \
 nemici*.c attivita.c attivita_01_10.c && ./h --tutte > /dev/null; \
 gcov -n h-nemici_tipi_10_13b.gcda | grep -A1 "File 'nemici_tipi_10_13b.c'" | tail -1; \
 gcov -n -f h-nemici_tipi_10_13.gcda | grep -A1 -E "Function '(sub_55c4|sub_6c1d|sub_719e)'" | grep 'Lines executed')
```
(Formato dell'uscita: `Lines executed:98.98% of 197`.) Il vecchio
`nemici_tipi_10_13.c` ha altre routine coperte solo dal replay: la copertura
si misura quindi sul file nuovo e sulle tre funzioni, non sul file intero.
Nel rapporto: le righe non eseguite di ogni file e il motivo.

**7. Documenti.** `ROUTINE.md`: in fondo (dopo la r. 415), sezione
`## Motore dei nemici (lotto N5)` con un paragrafo (moduli, nessun `ld a,r`,
stato dei tre tipi), la tabella come quella della N4 (ingresso, funzione,
file, byte, descrizione, stato, catture OK, stati, note), una riga per
ognuna delle 14 routine, l'elenco delle catture forzate e la nota che
`sub_6c2c` ora imposta il riporto. Nelle sezioni N1/N2 non si cambia niente.
`CENSIMENTO-NEMICI.md`: dopo il paragrafo «Lotto N4 fatto» un paragrafo
«**Lotto N5 fatto** (brief 31)» con la divisione fra `nemici_tipi_10_13.c`
(55C4, 6C1D, 719E) e `nemici_tipi_10_13b.c` (le altre 11) e la correzione
del riporto di 6C2C; nella tabella dei moduli, sotto la riga di
`nemici_tipi_10_13.c`, una riga `nemici_tipi_10_13b.c` (stesso schema della
riga `nemici_comparsab.c`).

## Verifiche obbligatorie

Tutte da `/home/rufio72/OpenSolomonsKey-master`; ogni uscita attesa è esatta
salvo dove è scritto `>=` o «attesa, non misurata». Abbreviazione usata nei
cicli: `V="54D8 54EA 551A 5529 5584 55C4 56E9 5713 5726 573C 5749 6BCD 6C1D 719E"`.

1. Disassemblato: `wc -l < decomp/out/nemici_tipi_10_13.dasm` → `272` (misurata).
2. Compilazione: `cd decomp/harness && make harness replay sched_proto > /tmp/m31.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/m31.txt`
   → `make 0` e `0`.
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h31.txt; echo "exit $?"; grep -c FALLITO /tmp/h31.txt; grep -c '^RIEPILOGO' /tmp/h31.txt`
   → `exit 0`, `0`, `203` (189 + 14 voci nuove).
4. Catture per voce, riepilogo del banco:
   `V="..."; for a in $V; do grep "^RIEPILOGO 0x$a:" /tmp/h31.txt; done | awk '{if($3<2||$3!=$5)b++} END{print NR, b+0}'`
   → `14 0`.
5. Catture non forzate per voce:
   `cd decomp && V="..."; for a in $V; do ls out/snap_0x${a}_*_regs.txt | grep -vc forza; done | awk '$1<2{b++} END{print NR, b+0}'`
   → `14 0`.
6. Totale catture: `grep -c '^cattura' /tmp/h31.txt | awk '{print ($1>=567)?"OK":"POCHE"}'` → `OK` (539 + 2 per voce, più le forzate).
7. Voci singole e 6C2C: `cd decomp/harness && for a in 0x54D8 0x56E9 0x719E 0x6C2C; do ./harness $a > /dev/null; echo -n "$? "; done; echo; grep '^RIEPILOGO 0x6C2C:' /tmp/h31.txt`
   → `0 0 0 0 ` e `RIEPILOGO 0x6C2C: 2 catture, 2 OK` (misurato prima della modifica).
8. `sched_proto`: `cd decomp && harness/sched_proto out/sched_log.txt | tail -1`
   → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
9. Replay: `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r31.txt; echo "exit $?"; tail -4 /tmp/r31.txt`
   → `exit 0` e le quattro righe (misurate):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
10. Stub: `grep -c STUB_NEMICI decomp/harness/nemici_stub.c; grep -c -E 'STUB_NEMICI\((54d8|55c4|56e9|6c1d|719e)\)' decomp/harness/nemici_stub.c`
    → `17` e `0` (attesa, dedotta: 22 − 5).
11. Copertura: il comando del punto 6 di «Struttura». Uscita attesa (attesa,
    non misurata): una riga `Lines executed:NN.NN% of N` per il file nuovo
    con NN ≥ 85 e tre righe per le funzioni con NN ≥ 75. Controllo:
    `… | awk -F'[:%]' '{n++; if($2+0<(n==1?85:75))b++} END{print n, b+0}'` → `4 0`.
12. Dimensioni: `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/nemici*.c | awk '$1>700 && $2!="totale" && $2!="total"'` → vuoto.
13. File vietati intatti: `git diff --name-only HEAD | grep -E 'cattura\.sh|snap\.lua|sessioni|replay\.|sched|rst20|rst38_tables|censimento_nemici\.py|STATO\.md|README\.md'` → vuoto.
14. Ambito: `git status --short | grep -v -E '^( M|\?\?|A ) (compiti/31-|decomp/(harness/(nemici[a-z0-9_]*\.[ch]|routine_tab\.c|z80_6xxx\.c)|ROUTINE\.md|CENSIMENTO-NEMICI\.md|out/(snap_0x[0-9A-F]{4}_|nemici_tipi_10_13\.dasm)))'`
    → vuoto; `git status --short | grep -c -E '\.(gcda|gcno)$'` → `0`.
15. Documenti: `grep -c '^## Motore dei nemici (lotto N5)$' decomp/ROUTINE.md; grep -c 'Lotto N5 fatto' decomp/CENSIMENTO-NEMICI.md; grep -c '^| `nemici_tipi_10_13b.c`' decomp/CENSIMENTO-NEMICI.md`
    → `1`, `1` e `1`.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto N5 del motore dei nemici: tipi 10-13 (54D8, 56E9, 719E), catture nel banco`
  e riga finale `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`
  (se il lotto passa a Opus per escalation, la riga diventa
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`).
  Il commit comprende il disassemblato `decomp/out/nemici_tipi_10_13.dasm`
  (`git add -f`) e le catture nuove (`prima.bin`, `dopo.bin`, `regs.txt`,
  `.log`, anch'esse da aggiungere con `-f`).
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-15; l'elenco
  delle voci con numero di catture e stato (`verificata`/`tradotta`); le
  forzature della tabella che non hanno aperto il ramo voluto; le righe gcov
  non coperte; le deviazioni dal brief.

## Domande aperte

1. **Divisione dei moduli**: con 0,6-0,68 righe per byte tutto in
   `nemici_tipi_10_13.c` darebbe 668-715 righe contro il limite di 700.
   Proposta: dividere per tema; 55C4, 6C1D e 719E (tipi 12 e 10) nel file
   esistente, le altre 11 (tipi 13 e 11) in `nemici_tipi_10_13b.c`. —
   «preferenza»
2. **Riporto di 6C2C**: 551A e 6C1D lo leggono, `sub_6c2c` non lo scrive.
   Proposta: aggiungerlo in `sub_6c2c` (`z80_6xxx.c`) con la convenzione di
   5507 e 7379 (bit 0 di `z->f`), invece di calcolarlo nei chiamanti; il
   banco non confronta F e le catture di 6C2C restano OK. Tocca una routine
   già `verificata`, per 3-4 righe. — «preferenza»
3. **551A catturabile solo da `finale`** e 5584 solo da `stanza30`: nelle
   stanze 02, 06, 07, 08, 20, 22, 26, 30, 35, 38, 46, 50 la 551A non scatta.
   Proposta: le seconde catture non forzate sono `finale N=2 n2` e `stanza30
   N=2 n2` (prima/dopo diversi dalla prima occorrenza, misurato), come per
   7412 nel lotto N4. — «preferenza»
4. **Soglie di copertura**: 85% sul file nuovo, 75% sulle tre funzioni nel
   file vecchio; `ret nc` di 7379 (3 punti) e il `default` dei due `switch`
   non si raggiungono. Proposta: soglie come scritto, righe scoperte
   elencate nel rapporto. — «preferenza»
5. **`STATO.md` e `README.md`**: lo sviluppatore non li legge né li tocca.
   Proposta: li aggiorna la chat principale dopo la verifica. —
   «preferenza»

Scelte di progetto: nessuna.

## Risoluzioni (chat principale, 2026-10-29)

Tutte le domande sono «preferenza»: accolte tutte con la proposta
dell'analista.
1. Divisione dei moduli come prescritto (55C4, 6C1D e 719E nel file
   esistente, le altre 11 in `nemici_tipi_10_13b.c`): sì.
2. Riporto di 6C2C scritto in `sub_6c2c` (z80_6xxx.c, convenzione di
   5507/7379): sì — il banco protegge la routine già verificata.
3. Seconde catture `finale N=2 n2` e `stanza30 N=2 n2` per 551A e 5584: sì.
4. Soglie gcov 85% (file nuovo) e 75% (le tre funzioni nel vecchio): sì.
5. `STATO.md` e `README.md` li aggiorna la chat principale: sì.

## Esito (chat principale, 2026-10-29)

Terzo ciclo dello schema economico, passato al primo colpo.
- Le 15 verifiche rieseguite dalla chat principale passano tutte; la
  copertura (11), che lo sviluppatore aveva misurato a mano, confermata
  col comando del brief: `4 0` (91,53% file nuovo; 98,46/100/100% le tre
  funzioni). Banco a 626 catture (203 voci, 0 fallite), replay a fine
  oracolo, 6C2C ancora 2/2 dopo la modifica del riporto.
- Tutte e 14 le voci «verificata». Rami residui scoperti documentati nel
  rapporto (5572 e 5545 di 5529 non apribili con le forzature provate).
- **Token**: analista Sonnet ~142k, sviluppatore Sonnet ~83k (minimo
  storico: le catture pre-misurate nel brief pagano), verifica 0.
  Totale ~225k ≈ 0,5-0,8 $.
