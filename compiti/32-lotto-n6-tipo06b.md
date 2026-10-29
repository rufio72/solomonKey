# Compito 32: lotto N6 del motore dei nemici, tipo 6 seconda metà (5274-531C)

Data: 2026-10-29. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere.

## Contesto: file da leggere

Da leggere (e basta):
1. `decomp/out/nemici_n6.dasm`, da generare (vedi sotto): 134 righe di
   disassemblato, le 8 routine del lotto più le 5 anticipate da N10.
2. `decomp/harness/rst38_tables.h`: solo `rst38_5207` (r. 38-41, 14 voci).
3. `decomp/harness/nemici.h` (244 righe): prototipi, `nemici_rst38`
   (r. 28-40), `NEMICI_PONI_RIPORTO`, `NEMICI_RIPORTO_ACCESO` (r. 16-19).
4. Le convenzioni qui sotto («Convenzioni di traduzione»).

Da aprire perché si modificano, solo per la parte indicata:
`decomp/harness/nemici_tipo06a.c` (59 righe: tutto), `nemici_tipo06b.c`
(173 righe: r. 1-45 per intestazione, `sub_52fd` e `sub_54c5`),
`nemici_tipi_10_13b.c` (solo r. 18-33, esempio di `sub_54d8`: `rst 38` con
`switch`), `nemici_r.c` (solo r. 155-161, i quattro wrapper `banco_r_*`),
`routine_tab.c` (solo r. 113-121), `routine.h` (solo r. 76, `sub_36f0`),
`ram.h` (macro `M`, `BC`, `SET_HL`…), `decomp/ROUTINE.md` (solo dalla r. 417
alla fine: formato della sezione N5), `decomp/CENSIMENTO-NEMICI.md` (solo il
paragrafo «Lotto N5 fatto», r. 422-427).

È vietato leggere `STATO.md`, `README.md`, `ROUTINE.md` per intero, gli
altri brief in `compiti/` e i moduli `nemici_*.c` diversi da quelli elencati.

Generazione del disassemblato (da `/home/rufio72/OpenSolomonsKey-master`,
`awk` è mawk: il confronto è fra stringhe esadecimali minuscole):
```
cd decomp && awk 'BEGIN{n=split("5068 51fa 5224 531c",lo," "); split("5091 5207 52fc 5344",hi," ")}
/^[A-Za-z_0-9]+:/ {lab=$0; next}
/;[0-9a-f][0-9a-f][0-9a-f][0-9a-f][ \t]*$/ {a=substr($0,length($0)-3); ok=0; for(i=1;i<=n;i++) if(a>=lo[i] && a<=hi[i]) ok=1; if(ok){ if(lab!="") print lab; print} lab=""; next}
{lab=""}' out/main64.dasm > out/nemici_n6.dasm; cd ..
```
Uscita attesa (misurata): `wc -l < decomp/out/nemici_n6.dasm` → `134`;
`grep -c 'ld a,r' decomp/out/nemici_n6.dasm` → `0`. Il file è ignorato da
git (`decomp/out/*`): si aggiunge al commit con `git add -f`. La tabella
inline 5208-5223 non è nel file (sta in `rst38_5207`); 5066-5067 (`ld b,03`)
non si traduce. 52FD, 5345, 54C5 (N3) non sono nel file e non si toccano.

## Obiettivo

Tradurre in C le 8 routine ancora mancanti del lotto N6 (5274, 5289, 5292,
52AA, 52C2, 52DB, 52EC, 531C: 178 byte) e le 5 routine del tipo 6 che esse
richiedono e che il censimento assegna a N10 (51FA, 5224, 523C, 5254, 5068:
136 byte). 52FD, 5345 e 54C5 sono già tradotte (N3, `nemici_tipo06b.c`) e
non si toccano. Le 8 routine sono raggiunte solo da 51FA con `rst 38` a
5207 (tranne 531C, chiamata da 50BA e 50DF, e la sua coda 5068): 51FA è
quindi l'ingresso del banco per la maggior parte di esse. Verifica: catture
MAME nel banco (`harness --tutte`) con `cattura.sh` (stati `finale`,
`stanza50`) e catture forzate con `SNAP_FORZA`. Replay e `sched_proto` non
devono cambiare.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `cattura.sh`, `snap.lua`, `sessioni.sh`, `replay.*`, `sched*.c`,
  `rst20*`, `rst38_tables.h`, `censimento_nemici.py`, `STATO.md`,
  `README.md` né le catture esistenti in `decomp/out/`.
- Niente installazioni. MAME gira solo come fa `cattura.sh` (senza finestra,
  senza audio, provider a `none`): niente partite a mano, niente giri su
  tutte le stanze. Una cattura dura 25-100 secondi (si possono lanciare
  fino a 6 in parallelo con `xargs -P6`: file di uscita distinti).
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe e nessun `nemici*.c` sopra le 700.
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`); si
  ripristina un file solo con `git checkout -- <file>`.
- Un solo agente alla volta su `decomp/harness`.

## Cosa esiste già (fatti verificati)

Misure del 2026-10-29 sui binari già compilati (da `decomp/harness`):
- `./harness --tutte`: exit 0, 626 righe `^cattura`, 0 `FALLITO`, 203 righe
  `^RIEPILOGO`; `RIEPILOGO 0x51D7`, `0x52FD`, `0x5345`, `0x54C5`: `2 catture,
  2 OK` ciascuna.
- `grep -c STUB_NEMICI nemici_stub.c` → 17: nessuna delle routine di questo
  lotto ha uno stub (4FD4, 50BA, 50DF restano stub/non tradotte).
- Nessun file di codice di `decomp` (fuori da `out/`) sopra le 800 righe;
  `nemici*.c` tutti sotto le 700 (il più lungo: `nemici_tipi_08_14.c`, 697).
- gcov di partenza (comando al punto 6): `nemici_tipo06a.c` 100,00% di 32
  righe, `nemici_tipo06b.c` 99,14% di 116.
- Il Makefile prende `nemici*.c` con un `wildcard`: nessun modulo nuovo.
- `.gitignore` non ignora `decomp/out/snap_*`.
- Nessuna routine del dasm legge `ld a,r` direttamente; ma 51FA chiama 51D7,
  che chiama 54C5 (due `ld a,r`): serve il wrapper `banco_r_51fa` (vedi sotto).

Routine (dal dasm; «replay» = NO per tutte: il replay non le raggiunge):

| ingresso | byte | chiamanti | cosa fa |
|---|---:|---|---|
| 51FA | 14 | 50DF (non tradotta) | `call 51D7`; se il bit 0 di (IX+1) è spento (IX+10) = A; E = A; `rst 38` al sito 5207 su `rst38_5207` (14 voci) |
| 5224 | 24 | 51FA[0] | C = 0; se il bit 0 di (IX+1): C = 1 se (IX+10) > 6; (IY+1E) = C; set 0 di (IX+1) |
| 523C | 24 | 51FA[2] | come 5224 (stesso C) ma (IY+1E) = C e res 0 di (IX+1) invece di set 0 |
| 5254 | 32 | 51FA[4] | C come 5224; (IY+1D) = 0, (IY+1E) = C; res 2 di (IX+0); set 0 di (IX+1) |
| 5274 | 21 | 51FA[1,3,5] | (IY+1D) = 0, (IY+1E) = 0; res 0 di (IX+1); res 2 di (IX+0); set 7 di (IX+1) |
| 5289 | 9 | 51FA[6] | set 1 di (IY+1E); res 0 di (IX+1) |
| 5292 | 24 | 51FA[7] | C = 1, con bit 0 di (IX+1) e 6 >= (IX+10) C = 0; (IY+1E) = C; set 0 di (IX+1) |
| 52AA | 24 | 51FA[9] | identica a 5292 |
| 52C2 | 25 | 51FA[11] | C = 1 (0 se il bit 0 di (IX+1)); (IY+1D) = 0, (IY+1E) = C; set 0 di (IX+1); res 2 di (IX+0) |
| 52DB | 17 | 51FA[8,10,12] | (IY+1D) = 0, (IY+1E) = 1; res 0 di (IX+1); res 2 di (IX+0) |
| 52EC | 17 | 51FA[13] | (IY+1D) = 0; set 1 di (IY+1E); res 0 di (IX+1); res 2 di (IX+0) |
| 531C | 41 | 50BA, 50DF | H = (IY+0E)+8, L = (IY+15)+5; due `call 36F0` (L e L+6): HL = cella in c0bd, bit 7 = piena; entrambe libere → `or a; ret`; una piena → `pop hl` (scarta l'indirizzo di ritorno) e `jp 5068` con B = (IY+1A) |
| 5068 | 42 | 531C, 5092, 51B5, 5066 (non tradotte) | coda condivisa: (IY+0) = ((IY+0) & 9F) \| 10; (IY+4) += 3; (IY+3) = 0D; (IY+1E) = 2; (IY+1A) = B; (IX+0E) = FF; set 6 di (IY+0); res 0 di (IX+1) |

Le descrizioni sono un riassunto: fa fede il dasm. Già tradotte e da usare
(prototipi in `routine.h` o `nemici.h`): `sub_51d7` (BC = maschera a 14 bit;
con un solo bit acceso, E = A = indice del bit, senza dipendere da R: C = 1
e 54C5 dà 0), `sub_36f0` (H, L → HL = c0bd + indice cella, DE, A come
l'originale), `sub_52fd`, `sub_5345`, `sub_54c5`. 5345 non è chiamata da
nessuna routine di questo lotto.

I 14 valori della tabella `rst38_5207` (indice E → destinazione): 5224,
5274, 523C, 5274, 5254, 5274, 5289, 5292, 52DB, 52AA, 52DB, 52C2, 52DB,
52EC. Misura (catture forzate di 51FA con BC = 1 << j): E = j, cioè
DE di uscita = 5209 + 2j, per j = 0..13.

### Convenzioni di traduzione

- Ogni routine è `void sub_xxxx(struct z80 *z)`. Registri `z->a b c d e h l`,
  `z->ix`, `z->iy`, `z->f`. Memoria con `M(indirizzo)`; coppie con
  `BC(z) DE(z) HL(z)` e `SET_BC SET_DE SET_HL`. In `nemici_tipo06b.c` ci sono
  già le macro `IXB(o)`, `IYB(o)`, `BIT7(hl)`, `RES`, `SET`.
- Il banco confronta RAM (c000-e5ff, salvo 0x40 byte sotto SP) e A, BC, DE,
  HL, IX, IY all'uscita, non F: anche i registri «sporchi» devono coincidere
  (C e A in particolare). Il riporto, dove un chiamante lo legge, sta nel
  bit 0 di `z->f` (`NEMICI_PONI_RIPORTO`, `NEMICI_RIPORTO_ACCESO`).
- `rst 38h` con tabella inline: `z->e = ...` (qui `z->e = z->a`, il `ld e,a`
  di 5206), poi `nemici_rst38(z, 0x5207, rst38_5207, 14)` restituisce la
  destinazione (0 fuori tabella) e mette DE = sito+2+2E; `switch` come in
  `sub_54d8` (`nemici_tipi_10_13b.c` r. 19-33). Le destinazioni ritornano al
  chiamante di 51FA (sono `jp` impliciti).
- **531C e `pop hl; jp 5068`.** Nell'originale il ramo «cella piena» scarta
  l'indirizzo di ritorno e termina in 5068, che ritorna al chiamante del
  chiamante (4EFB). In C: `sub_531c` chiama `sub_5068(z)` (con
  `z->b = IYB(0x1a)`) e poi ritorna; convenzione per i futuri chiamanti
  (50BA, 50DF, N10): riporto 1 in `z->f` = uscita anomala, il chiamante deve
  ritornare subito; riporto 0 = uscita normale (`or a`). In MAME HL esce con
  l'indirizzo di ritorno scartato (50CB o 50E2): il C non lo riproduce (non
  esiste una pila); è nel commento della funzione.
- Il ramo anomalo di 531C **non è catturabile**: `snap.lua` aspetta il
  ritorno all'indirizzo letto in cima alla pila (con SP+2) e l'abbandono lo
  salta (misurato: le catture forzate con cella piena danno «limite di 6000
  frame raggiunto senza cattura»). Resta scoperto da gcov e va elencato.
- Wrapper R (regola ratificata): `banco_r_51fa` in `nemici_r.c`, una riga
  come le altre: `void banco_r_51fa(struct z80 *z) { prova_con_r(z, "0x51FA",
  sub_51fa); }`; prototipo in `nemici.h`; la voce di `routine_tab.c` per 51FA
  punta al wrapper. Le altre routine non leggono R.
- Catture `forza*` valgono come catture OK ma non contano per lo stato
  `verificata`, che richiede almeno 2 catture non forzate.
- Routine non catturabili direttamente → tradotte con nota «senza cattura
  propria» e coperte dalle catture forzate di 51FA più gcov (soglia 75%).

## Struttura da realizzare

**1. Moduli C.** Stima a 0,6-0,68 righe per byte: 178 B → 107-121 righe, da
sommare alle 173 di `nemici_tipo06b.c` = 280-294; 136 B → 82-92 righe, da
sommare alle 59 di `nemici_tipo06a.c` = 141-151. Entrambi molto sotto le 700:
nessuna spezzatura.
- `nemici_tipo06b.c`: in coda, `sub_5274`, `sub_5289`, `sub_5292`,
  `sub_52aa`, `sub_52c2`, `sub_52db`, `sub_52ec`, `sub_531c`, tutte non
  statiche (le chiama 51FA, in un altro file). Aggiornare il commento di
  intestazione (ingressi aggiunti; 531C e il riporto di uscita anomala).
- `nemici_tipo06a.c`: aggiungere `#include "rst38_tables.h"`, poi `sub_5068`,
  `sub_51fa`, `sub_5224` (non statiche) e `sub_523c`, `sub_5254` (`static`,
  chiamate solo da 51FA, prima di 51FA nel file). Aggiornare l'intestazione.
- `nemici_r.c`: `banco_r_51fa` (vedi sopra).

**2. `nemici.h`**: sezione «Lotto N6 (brief 32)» con i prototipi (non
statiche) di 5068, 51FA, 5224, 5274, 5289, 5292, 52AA, 52C2, 52DB, 52EC,
531C e di `banco_r_51fa` (accanto agli altri `banco_r_*`). `nemici_stub.c`
non cambia.

**3. `routine_tab.c`**: voci `{ "0xXXXX", sub_xxxx, 0, 0 }` in ordine di
indirizzo per le sole routine con catture: 0x5068, 0x51FA (con
`banco_r_51fa`), 0x5224, 0x5274, 0x5292, 0x531C. Nessuna voce per 5289, 52AA,
52C2, 52DB, 52EC, 523C, 5254.

**4. Catture.** Da `decomp/`. Misurate il 2026-10-29 in una copia di lavoro:
tutte riescono. La seconda occorrenza dello stesso stato usa `N=2` e il
suffisso `n2`. «Dopo» a PC=510D (ritorno a 50DF dopo 51FA) per 51FA, 5224,
5274, 5292 (5114 per `5274 finale n2`); PC=4EFB per 5068; PC=50E2/50CB per
531C. Non catturabili in nessuno stato (misurato: nessuna occorrenza fino al
frame 9600 in `finale`, nessuna in `stanza33`, `stanza45`, `stanza50`): 5289,
52AA, 52C2, 52DB, 52EC, 523C, 5254, e 5292 e 5224 in `stanza50`.
Comandi (non forzate, 16):
```
cd decomp
sh cattura.sh 0x5068 finale 1;        sh cattura.sh 0x5068 finale 2 n2
sh cattura.sh 0x51FA finale 1;        sh cattura.sh 0x51FA finale 2 n2
sh cattura.sh 0x51FA stanza50 1;      sh cattura.sh 0x51FA stanza50 2 n2
sh cattura.sh 0x5224 finale 1;        sh cattura.sh 0x5224 finale 2 n2
sh cattura.sh 0x5224 stanza50 1
sh cattura.sh 0x5274 finale 1;        sh cattura.sh 0x5274 finale 2 n2
sh cattura.sh 0x5292 finale 1
sh cattura.sh 0x531C finale 1;        sh cattura.sh 0x531C finale 2 n2
sh cattura.sh 0x531C stanza50 1;      sh cattura.sh 0x531C stanza50 2 n2
```
Forzate di 51FA (29 in tutto), stato `finale`, `N=1`. In 51FA la maschera BC
con un solo bit acceso dà E = indice del bit; per j = 0..13 `BC = 1 << j`
(`b:00,c:<1<<j>` per j < 8, `b:<1<<(j-8)>,c:00` per j >= 8), tag `forzaJJ`:
```
for j in 0 1 2 3 4 5 6 7 8 9 10 11 12 13; do
  if [ $j -lt 8 ]; then b=00; c=$(printf %02x $((1<<j))); else b=$(printf %02x $((1<<(j-8)))); c=00; fi
  SNAP_FORZA="b:$b,c:$c" sh cattura.sh 0x51FA finale 1 forza$(printf %02d $j)
done
```
Varianti per i sei ingressi che leggono il bit 0 di (IX+1) e (IX+10)
(j = 0, 2, 4, 7, 9, 11, cioè 5224, 523C, 5254, 5292, 52AA, 52C2), con `b:`/`c:`
come sopra: tag `forzaJJb` con `ix+1:01,ix+10:07` (confronto: 6 < (IX+10)) e
`forzaJJc` con `ix+1:01,ix+10:03` (6 >= (IX+10)), 12 catture.
Forzate dirette di 5292, `finale`, 3 catture: `forza01` = `ix+1:00`,
`forza02` = `ix+1:01,ix+10:07`, `forza03` = `ix+1:01,ix+10:03`.

Forzature **misurate** (tutte eseguite; effetto letto dal confronto
prima/dopo, A e BC in uscita, DE = 5209 + 2j in tutte le 51FA):

| voce | SNAP_FORZA | ramo aperto; dopo (AF, BC) |
|---|---|---|
| 51FA | `b:00,c:01` … `b:20,c:00` (j = 0..13) | le 14 voci di 5207; A = j in uscita; BC = 0001 per j = 7, 9, 11, altrimenti 0000 |
| 51FA, j = 0 | `…,ix+1:01,ix+10:07` / `…,ix+10:03` | 5224 con confronto: AF = 0601 BC = 0001 / AF = 0602 BC = 0000 |
| 51FA, j = 2 | stesse | 523C: AF = 0601 BC = 0001 / AF = 0602 BC = 0000 |
| 51FA, j = 4 | stesse | 5254: AF = 0601 BC = 0001 / AF = 0602 BC = 0000 |
| 51FA, j = 7 | stesse | 5292: AF = 0693 BC = 0001 / AF = 0642 BC = 0000 |
| 51FA, j = 9 | stesse | 52AA: AF = 0693 BC = 0001 / AF = 0642 BC = 0000 |
| 51FA, j = 11 | stesse | 52C2 con bit 0 acceso: AF = 0B42 BC = 0000 (entrambe) |
| 5292 | `ix+1:00` / `ix+1:01,ix+10:07` / `ix+1:01,ix+10:03` | C = 1 / carry, C = 1 / senza carry, C = 0 (AF = 0754, 0693, 0642) |

Naturali (misurati; A/E di 51FA): `51FA finale` ha BC = 0021 (due bit
accesi: 51D7 sceglie a caso, E = 5, 5274; richiede il wrapper R),
`finale n2` e `stanza50` E = 0 (5224), `stanza50 n2` E = 0 (5224).

Restano non esercitabili: il ramo anomalo di 531C (non catturabile, vedi
sopra) e il caso `default`/0 di `nemici_rst38` in 51FA (E <= 13 sempre).

**5. Copertura del C** (gcov, fuori dal repository; prefisso `h-`):
```
D=/tmp/cov32 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
 && ln -s "$PWD/decomp/out" $D/out && (cd $D && gcc -std=c11 -O0 --coverage -o h \
 harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c z80_*.c \
 nemici*.c attivita.c attivita_01_10.c && ./h --tutte > /dev/null; \
 for f in nemici_tipo06a nemici_tipo06b; do gcov -n h-$f.gcda | grep -A1 "File '$f.c'" | tail -1; done)
```
Formato: `Lines executed:99.14% of 116`. Nel rapporto: le righe non eseguite di
ogni file e il motivo.

**6. Documenti.** `ROUTINE.md`: in fondo (dopo la r. 466), sezione
`## Motore dei nemici (lotto N6)` con un paragrafo (moduli, 51FA con il
wrapper R, anticipazioni da N10, convenzione di 531C), la tabella come
quella della N5 (ingresso, funzione, file, byte, descrizione, stato, catture
OK, stati, note), una riga per ognuna delle 13 routine, l'elenco delle
catture forzate. 5289, 52AA, 52C2, 52DB, 52EC, 523C, 5254: stato `tradotta`,
nota «senza cattura propria, coperta dalle forzate di 51FA». 5292: `tradotta`
(una sola cattura non forzata). `CENSIMENTO-NEMICI.md`: dopo il paragrafo
«Lotto N5 fatto» un paragrafo «**Lotto N6 fatto** (brief 32)» con le 8
routine, le 5 anticipate da N10 (che non vanno più fatte in N10) e la
convenzione del riporto di 531C. Le righe della tabella dei moduli non
cambiano.

## Verifiche obbligatorie

Tutte da `/home/rufio72/OpenSolomonsKey-master`; uscita attesa esatta salvo
dove è scritto `>=` o «attesa, non misurata». Abbreviazione:
`V="5068 51FA 5224 5274 5292 531C"`.

1. Disassemblato: `wc -l < decomp/out/nemici_n6.dasm; grep -c 'ld a,r' decomp/out/nemici_n6.dasm`
   → `134` e `0` (misurate).
2. Compilazione: `cd decomp/harness && make harness replay sched_proto > /tmp/m32.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/m32.txt`
   → `make 0` e `0`.
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h32.txt; echo "exit $?"; grep -c FALLITO /tmp/h32.txt; grep -c '^RIEPILOGO' /tmp/h32.txt`
   → `exit 0`, `0`, `209` (203 + 6 voci nuove).
4. Catture per voce: `for a in $V; do grep "^RIEPILOGO 0x$a:" /tmp/h32.txt; done`
   → esattamente (attesa, dedotta dal numero di catture elencate):
   ```
   RIEPILOGO 0x5068: 2 catture, 2 OK
   RIEPILOGO 0x51FA: 30 catture, 30 OK
   RIEPILOGO 0x5224: 3 catture, 3 OK
   RIEPILOGO 0x5274: 2 catture, 2 OK
   RIEPILOGO 0x5292: 4 catture, 4 OK
   RIEPILOGO 0x531C: 4 catture, 4 OK
   ```
5. Catture non forzate per voce: `cd decomp && for a in $V; do ls out/snap_0x${a}_*_regs.txt | grep -vc forza; done | tr '\n' ' '`
   → `2 4 3 2 1 4 ` (5292 ha una sola cattura non forzata: stato `tradotta`).
6. Totale catture: `grep -c '^cattura' /tmp/h32.txt` → `671` (626 + 45).
7. Voci precedenti invariate: `for a in 51D7 52FD 5345 54C5 6C2C; do grep "^RIEPILOGO 0x$a:" /tmp/h32.txt; done`
   → cinque righe `RIEPILOGO 0x…: 2 catture, 2 OK` (misurato prima della modifica).
8. `sched_proto`: `cd decomp && harness/sched_proto out/sched_log.txt | tail -1`
   → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
9. Replay: `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r32.txt; echo "exit $?"; tail -4 /tmp/r32.txt`
   → `exit 0` e (misurate):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
10. Stub: `grep -c STUB_NEMICI decomp/harness/nemici_stub.c` → `17` (misurato, invariato).
11. Copertura: il comando del punto 5 di «Struttura». Uscita attesa (attesa,
    non misurata): due righe `Lines executed:NN.NN% of N` con NN >= 90.
    Controllo: `… | awk -F'[:%]' '{n++; if($2+0<90)b++} END{print n, b+0}'` → `2 0`.
12. Dimensioni: `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/nemici*.c | awk '$1>700 && $2!="totale" && $2!="total"'` → vuoto.
13. File vietati intatti: `git diff --name-only HEAD | grep -E 'cattura\.sh|snap\.lua|sessioni|replay\.|sched|rst20|rst38_tables|censimento_nemici\.py|STATO\.md|README\.md'` → vuoto.
14. Ambito: `git status --short | grep -v -E '^( M|\?\?|A ) (compiti/32-|decomp/(harness/(nemici[a-z0-9_]*\.[ch]|routine_tab\.c)|ROUTINE\.md|CENSIMENTO-NEMICI\.md|out/(snap_0x(5068|51FA|5224|5274|5292|531C)_|nemici_n6\.dasm)))'`
    → vuoto; `git status --short | grep -c -E '\.(gcda|gcno)$'` → `0`.
15. Documenti: `grep -c '^## Motore dei nemici (lotto N6)$' decomp/ROUTINE.md; grep -c 'Lotto N6 fatto' decomp/CENSIMENTO-NEMICI.md`
    → `1` e `1`.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto N6 del motore dei nemici: tipo 6 seconda metà (5274-531C), con 51FA, 5224, 523C, 5254 e 5068 anticipate da N10, catture nel banco`
  e riga finale `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`
  (se il lotto passa a Opus per escalation, `Claude Opus 5.5`). Il commit
  comprende il disassemblato `decomp/out/nemici_n6.dasm` (`git add -f`) e le
  catture nuove (`prima.bin`, `dopo.bin`, `regs.txt`, `.log`; con `git add -f`
  per sicurezza).
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-15; l'elenco
  delle voci con numero di catture e stato (`verificata`/`tradotta`); le
  forzature della tabella che non hanno aperto il ramo voluto; le righe gcov
  non coperte; le deviazioni dal brief.

## Domande aperte

1. **Anticipare da N10 cinque routine** (51FA, 5224, 523C, 5254, 5068,
   136 B in `nemici_tipo06a.c`). Servono: 5068 è la coda di 531C; 51FA con la
   sua tabella è l'unico modo di coprire con il banco 5289, 52AA, 52C2, 52DB,
   52EC (mai raggiungibili da `cattura.sh`) e 5292 (una sola occorrenza); le
   altre tre destinazioni della tabella (5224, 523C, 5254) servono a 51FA per
   non lasciare voci non tradotte. Come nel lotto N3 (51D7, 24DF), con N10 che
   scende da 27 a 22 routine. Alternativa: tradurre solo 5068 e lasciare le
   cinque routine N6 senza alcuna copertura (nessuna cattura, gcov 0%).
   Proposta: l'anticipazione. — «scelta di progetto»
2. **Riporto di uscita di 531C** (1 = uscita anomala, il chiamante ritorna;
   0 = normale) in `z->f`, e HL dopo il ramo anomalo non riprodotto.
   Proposta: come scritto. — «preferenza»
3. **Ramo anomalo di 531C non catturabile** (il «dopo» di `snap.lua` non lo
   vede). Proposta: tradotto con nota, scoperto da gcov, elencato nel
   rapporto; non si modifica `snap.lua`. — «preferenza»
4. **5292 con una sola cattura non forzata** (nessuna seconda occorrenza né
   in `finale` fino a 9600 frame né in `stanza50`). Proposta: stato
   `tradotta`, con 3 forzate dirette e le 4 di 51FA (j = 7). — «preferenza»
5. **Soglia gcov** 90% per i due file (non 75/85 come nei lotti precedenti:
   i file sono piccoli e le uniche righe scoperte attese sono quelle del ramo
   anomalo di 531C). Proposta: come scritto. — «preferenza»
6. **`STATO.md` e `README.md`**: lo sviluppatore non li legge né li tocca.
   Proposta: li aggiorna la chat principale dopo la verifica. — «preferenza»

## Risoluzioni (chat principale, 2026-10-29)

1. **Accolta la proposta** (scelta di progetto risolta col precedente di
   N3, brief 29 risoluzione 1): le 5 routine di N10 (51FA, 5224, 523C,
   5254, 5068) si anticipano qui, nel modulo del censimento
   (`nemici_tipo06a.c`); N10 le troverà fatte. L'alternativa lascerebbe
   cinque routine N6 senza copertura, contro la regola «cattura nel banco».
2. Riporto di 531C come scritto: sì.
3. Ramo anomalo di 531C tradotto con nota, `snap.lua` intatto: sì.
4. 5292 con una sola cattura non forzata, stato `tradotta`: sì.
5. Soglia gcov 90% sui due file: sì.
6. `STATO.md` e `README.md` li aggiorna la chat principale: sì.
