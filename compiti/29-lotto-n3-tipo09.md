# Compito 29: lotto N3 del motore dei nemici, tipo 9 (ingresso 57CD)

Data: 2026-10-29. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere.

## Contesto: file da leggere

Da leggere (e basta):
1. `decomp/out/nemici_tipo09.dasm`, da generare (vedi sotto): il
   disassemblato delle 16 routine del lotto e delle 5 dipendenze.
2. `decomp/harness/rst38_tables.h`: solo `rst38_57D3` (r. 58-61) e
   `rst38_5BFC` (r. 63-67).
3. `decomp/harness/nemici.h` (177 righe): prototipi, `nemici_rst38`,
   `NEMICI_PONI_RIPORTO`.
4. Le convenzioni qui sotto («Convenzioni di traduzione»).

Da aprire perché si modificano, solo per la parte indicata:
`decomp/harness/nemici_tipo09.c` (132 righe, serve da esempio di stile:
`prendi_portato` r. 21-30), `nemici_stub.c`, `routine_tab.c`, `ram.h`
(macro, r. 9-30), `routine.h` (solo prototipi, con `grep`),
`decomp/ROUTINE.md` (solo dalla r. 210 alla fine, formato della sezione N2),
`decomp/CENSIMENTO-NEMICI.md` (solo il paragrafo «Lotto N2 fatto»).

È vietato leggere `STATO.md`, `README.md`, `ROUTINE.md` per intero, gli
altri brief in `compiti/` e `decomp/harness/nemici_tipi_08_14.c`.

Generazione del disassemblato (da `/home/rufio72/OpenSolomonsKey-master`,
`awk` è mawk: il confronto è fra stringhe esadecimali minuscole):
```
cd decomp && awk 'BEGIN{n=split("24df 51d7 52fd 5345 54c5 57cd",lo," "); split("250d 51f9 531b 54c4 54d7 5c31",hi," ")}
/^[A-Za-z_0-9]+:/ {lab=$0; next}
/;[0-9a-f][0-9a-f][0-9a-f][0-9a-f][ \t]*$/ {a=substr($0,length($0)-3); ok=0; for(i=1;i<=n;i++) if(a>=lo[i] && a<=hi[i]) ok=1; if(ok){ if(lab!="") print lab; print} lab=""; next}
{lab=""}' out/main64.dasm > out/nemici_tipo09.dasm; cd ..
```
Uscita attesa (misurata): `wc -l < decomp/out/nemici_tipo09.dasm` → `958`.
Nel file: i byte delle tabelle 57D4-57E3 e 5BFD-5C18 compaiono come istruzioni
finte (le tabelle vere sono in `rst38_tables.h`); 58CF-594E è già tradotto
(lotto N1) e non si tocca.

## Obiettivo

Tradurre in C le 16 routine rimaste del lotto N3 (tipo 9 di 4F1B, ingresso
57CD) più 5 dipendenze che non stanno nel lotto ma sono eseguite a ogni
ingresso (57CD chiama 52FD, 5995 chiama 5345 e 51D7). Verifica: catture MAME
nel banco (`harness --tutte`) con `cattura.sh` stato `stanzaNN` e, dove un ramo
non si vede, `SNAP_FORZA`. Replay e `sched_proto` non devono cambiare.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `cattura.sh`, `snap.lua`, `sessioni.sh`, `replay.*`, `sched*.c`,
  `rst20*`, `rst38_tables.h`, `censimento_nemici.py`, `STATO.md`,
  `README.md` né le catture esistenti in `decomp/out/`.
- Niente installazioni. MAME gira solo come fa `cattura.sh` (senza finestra,
  senza audio, provider a `none`): niente partite a mano.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe e nessun `nemici*.c` sopra le 700.
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`); si
  ripristina un file solo con `git checkout -- <file>`.
- Un solo agente alla volta su `decomp/harness`.

## Cosa esiste già (fatti verificati)

Misure del 2026-10-29 sui binari già compilati (da `decomp/harness`):
- `./harness --tutte`: exit 0, 321 righe `^cattura`, 0 `FALLITO`, 147 righe
  `^RIEPILOGO` (formato `RIEPILOGO 0x6EB7: 2 catture, 2 OK`).
- `grep -c STUB_NEMICI nemici_stub.c` → 26 (la `#define` più 25 righe, tra cui
  `57cd`).
- Nessun file di codice di `decomp` (fuori da `out/`) sopra le 700 righe;
  `nemici_tipi_08_14.c` ne ha 697.

Il banco `harness` include già `$(wildcard nemici*.c)`: i moduli nuovi
entrano senza toccare il Makefile.

Routine del lotto (da `CENSIMENTO-NEMICI.md` e `out/censimento_nemici.txt`).
Stanze (0-based) = sessioni storiche che eseguono l'ingresso
(`grep -lx <ADDR> decomp/out/*_eseguiti.txt`); `finale` = sessione finale.
Il replay non le raggiunge (colonna replay = NO; oggi lo stub `57cd` non
ferma il replay, exit 0).

| ingresso | byte | chiamanti | stanze | note |
|---|---:|---|---|---|
| 57CD | 7 | 4F0F (`rst 38` 4F1B[9]) | 33 45 50 finale | `call 52FD; ld e,(iy+1d); rst 38` su `rst38_57D3` |
| 57E4 | 23 | 57CD [4] | 33 finale | |
| 57FB | 15 | 57CD [0] | 33 45 50 finale | |
| 580A | 33 | 57CD [2] | 33 45 50 finale | `jp 5894` |
| 582B | 105 | 57CD [1] | 33 45 50 finale | chiama 7379, 4592, `jp`/cade in 5894 |
| 5894 | 15 | 580A, 582B | 33 45 finale | ingresso interno: chiama 6EB7 (già in `nemici_tipo09.c`) |
| 58A3 | 44 | 57CD [7] | 33 45 50 finale | chiama 58CF, 5917 (già tradotte) |
| 594F | 70 | 57CD [6] | 33 45 50 finale | |
| 5995 | 597 | 57CD [5] | 33 45 50 finale | chiama 5345, 51D7, 5928, 5B12, 5C32, 7379, 3B62; termina con `rst 38` su `rst38_5BFC` (5BFC) |
| 5B09 | 9 | 5995 (`jr`/`jp` da 5A94, 5BB2, 5BD0 e voce [13]) | nessuna | mai eseguita |
| 5B12 | 10 | 5995 (`call`) | 33 45 50 finale | uscita: riporto (`cp 5`) |
| 5C19 5C1E | 5 5 | 5995 (voci [0][2] e [7][9]) | 33 50 / 33 45 | |
| 5C23 5C29 | 6 4 | 5995 (voci [1][3][4][5] e [8][10][11][12]) | 33 45 50 / 33 45 | 5C23 cade in 5C2D, 5C29 pure |
| 5C2D | 5 | 5C23, 5C29 | 33 45 50 finale | ingresso interno (coda condivisa) |

Totale delle 16 routine: 953 B. 6EB7 e 6EC1 sono già tradotte e catturate
(2 catture ciascuna).

Dipendenze fuori lotto, tutte non tradotte, tutte eseguite nelle stanze
33 45 50 e `finale`, da tradurre qui nel modulo che il censimento prescrive:

| ingresso | byte | modulo | chiamanti | chiama |
|---|---:|---|---|---|
| 52FD | 31 | `nemici_tipo06b.c` | 4FD4, 57CD, 5CFB | - |
| 5345 | 384 | `nemici_tipo06b.c` | 50DF, 5995 | 24DF |
| 54C5 | 19 | `nemici_tipo06b.c` | 51D7 | - |
| 51D7 | 35 | `nemici_tipo06a.c` | 51FA, 5995 | 54C5 |
| 24DF | 47 | `nemici_tipo06a.c` | 5345 | - |

Già tradotte e da usare (prototipi in `routine.h` o `nemici.h`, da grep):
`sub_0008`, `sub_0010`, `sub_0018`, `sub_36f0`, `sub_5c32`, `sub_7379`,
`sub_3b62`, `sub_4592`, `sub_4938`, `sub_49ae`, `sub_58cf`, `sub_5917`,
`sub_5928`, `sub_6eb7`, `sub_6ec1`, `sub_4b98(uint16_t ix)`.

### Convenzioni di traduzione

- Ogni routine è `void sub_xxxx(struct z80 *z)`. Registri `z->a b c d e h l`,
  `z->ix`, `z->iy` (uint16_t), `z->f`. Memoria con `M(indirizzo)` (RAM c000-ffff),
  coppie con `BC(z) DE(z) HL(z)` e `SET_BC SET_DE SET_HL` (`ram.h`).
- Il banco confronta RAM (c000-e5ff) e A, BC, DE, HL, IX, IY all'uscita, non F:
  anche i registri «sporchi» lasciati dal codice originale devono coincidere.
  Il riporto, dove un chiamante lo legge (5B12, 58CF, 7379), sta nel bit 0 di
  `z->f` (`NEMICI_PONI_RIPORTO(z,c)`, `NEMICI_RIPORTO_ACCESO(z)`).
- `rst 38h` a un sito con tabella inline: `nemici_rst38(z, sito, tab, voci)`
  restituisce la destinazione (0 fuori tabella) e mette DE = sito+2+2E; poi
  si smista con uno `switch` sulla destinazione (esempio: `salta_4f1b` in
  `nemici.c`). Qui: sito 0x57D3 con `rst38_57D3` (8 voci, la [3] è `sub_3b62`)
  e sito 0x5BFC con `rst38_5BFC` (14 voci, la [6] è `sub_3b62`).
- `jp`/`jr` verso un'altra routine = chiamata e `return`. Un ingresso interno
  senza cattura propria (5894, 5C2D) è una `static` condivisa; le routine con
  voce nel banco sono non statiche.
- `ld hl,48af; rst 8; rst 10h; push de; ex (sp),ix; ...; pop ix` è il
  pattern di `prendi_portato` (static in `nemici_tipo09.c`, r. 21): in 582B
  (5870-5878) A è già l'indice, per questo il pattern coincide solo se A =
  (IX+0E): controllarlo sul disassemblato prima di riusarlo.
- Le tabelle ROM si leggono con `M(addr + indice)`; sotto c000 la scrittura
  è ignorata (`scrivi_ram` in `nemici_tipo09.c`).
- Nessun `NEMICI_PUNTO`: il replay non raggiunge queste routine.
- Catture `forza*` valgono come catture OK ma non contano per lo stato
  `verificata`, che richiede almeno 2 catture non forzate.

## Struttura da realizzare

**1. Moduli C.** Stima righe a 0,6-0,68 righe per byte (N2: 697 righe per
1030 B): mettere tutto in `nemici_tipo09.c` darebbe 704-780 righe, troppo
vicino al limite. Quindi:
- `nemici_tipo09.c` (oggi 132 righe, ~340 dopo): 57CD, 57E4, 57FB, 580A, 582B,
  `static` 5894, 58A3, 594F, più le routine esistenti.
- `nemici_tipo09b.c` (nuovo, ~435 righe): 5995, 5B12, 5C19, 5C1E, 5C23, 5C29,
  `static` 5C2D, e 5B09 come `static` (vedi verifica 11). Intestazione con
  `#include "routine.h"`, `#include "nemici.h"`, `#include "rst38_tables.h"`.
- `nemici_tipo06b.c` (nuovo, ~295 righe): 52FD, 5345, 54C5.
- `nemici_tipo06a.c` (nuovo, ~56 righe): 51D7, 24DF.

**2. `nemici.h`**: sezione «Lotto N3» con i prototipi non statici di 57CD,
57E4, 57FB, 580A, 582B, 58A3, 594F, 5995, 5B12, 5C19, 5C1E, 5C23, 5C29,
52FD, 5345, 54C5, 51D7, 24DF. Si toglie `sub_57cd` dal blocco degli stub.
`nemici_stub.c`: si toglie `STUB_NEMICI(57cd)`. In `nemici.c` non cambia
niente (`salta_4f1b` chiama già `sub_57cd`).

**3. `routine_tab.c`**: una voce `{ "0xXXXX", sub_xxxx, 0, 0 }` per ognuno
dei 18 ingressi non statici (13 del lotto più 52FD, 5345, 54C5, 51D7, 24DF),
in ordine di indirizzo. 6EB7 e 6EC1 ci sono già.

**4. Catture.** Da `decomp/`, `sh cattura.sh <ENTRY> <stato> [N] [SUFFISSO]`.
Almeno 2 catture non forzate per voce. Piano:

| voci | catture |
|---|---|
| 0x57CD, 0x57FB, 0x580A, 0x582B, 0x58A3, 0x594F, 0x5995, 0x5B12, 0x5C23, 0x52FD, 0x5345, 0x54C5, 0x51D7, 0x24DF | `stanza33` N=1 e `stanza45` N=1 (o `stanza33` N=2, `stanza50`) |
| 0x57E4 | `stanza33` N=1, N=2; se manca, `finale` |
| 0x5C19 | `stanza33`, `stanza50` |
| 0x5C1E, 0x5C29 | `stanza33`, `stanza45` |

Per le voci eseguite poche volte per sessione si provano N=2 e N=3 e lo stato
`finale`. Dopo il primo giro si misura la copertura (punto 5) e si aggiungono
forzature per i rami non eseguiti, con `SNAP_FORZA` (voci `ix+HH:VV`,
`iy+HH:VV`, `r:VV`; la forzatura è applicata all'ingresso voluto, quindi vale
solo per rami decisi dentro la routine catturata). Esempi dal disassemblato
(non misurati): `57E4` con `ix+d:1f` e `ix+d:20`; `57FB` con `ix+d:03` e
`ix+d:04`; `580A` con `iy+1e:02,ix+d:08,ix+1:08` (apre 5894); `58A3` con
`ix+1:08` e `ix+d:16`; `594F` con `iy+c:ff`. Suffisso `forza` o `forzaNNNN`.

5B09 non si può catturare: nessuna sessione l'ha eseguita e `snap.lua` forza
solo IX, IY e registri dell'ingresso catturato, non le celle che 5C32 scrive
in c07e-c081 prima del salto. Si traduce senza voce nel banco.

**5. Copertura del C** (gcov, fuori dal repository):
```
D=/tmp/cov29 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
 && ln -s "$PWD/decomp/out" $D/out && (cd $D && gcc -std=c11 -O0 --coverage -o h \
 harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c z80_*.c \
 nemici*.c attivita.c attivita_01_10.c && ./h --tutte > /dev/null; \
 for f in nemici_tipo09.c nemici_tipo09b.c nemici_tipo06a.c nemici_tipo06b.c; do \
 gcov -n $f | grep -A1 "File '$f'" | tail -1; done)
```
Nel rapporto: le righe non eseguite di ogni file e il motivo.

**6. Documenti.** `ROUTINE.md`: in fondo, sezione «## Motore dei nemici (lotto
N3)» con la stessa tabella della N2 (ingresso, funzione, file, byte,
descrizione, stato, catture OK, stati, note), una riga per ognuno dei 18
ingressi nel banco più 5B09, 5894 e 5C2D (tradotte, senza voce propria);
elenco delle catture forzate. `CENSIMENTO-NEMICI.md`: sotto «Piano dei lotti»
un paragrafo «**Lotto N3 fatto** (brief 29)» con le 5 dipendenze anticipate da
N6 (52FD, 5345, 54C5) e N10 (51D7, 24DF) e la divisione in
`nemici_tipo09.c`/`nemici_tipo09b.c`.

## Verifiche obbligatorie

Tutte da `/home/rufio72/OpenSolomonsKey-master`; ogni uscita attesa è esatta
salvo dove è scritto `>=`.

1. Disassemblato: `wc -l < decomp/out/nemici_tipo09.dasm` → `958` (misurata).
2. Compilazione: `cd decomp/harness && make harness replay sched_proto > /tmp/m29.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/m29.txt`
   → `make 0` e `0`.
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h29.txt; echo "exit $?"; grep -c FALLITO /tmp/h29.txt; grep -c '^RIEPILOGO' /tmp/h29.txt`
   → `exit 0`, `0`, `165` (147 + 18 voci nuove).
4. Catture per voce, riepilogo del banco:
   `for a in 57CD 57E4 57FB 580A 582B 58A3 594F 5995 5B12 5C19 5C1E 5C23 5C29 52FD 5345 54C5 51D7 24DF; do grep "^RIEPILOGO 0x$a:" /tmp/h29.txt; done | awk '{if($3<2||$3!=$5)b++} END{print NR, b+0}'`
   → `18 0`.
5. Catture non forzate per voce:
   `cd decomp && for a in 57CD 57E4 57FB 580A 582B 58A3 594F 5995 5B12 5C19 5C1E 5C23 5C29 52FD 5345 54C5 51D7 24DF; do ls out/snap_0x${a}_*_regs.txt | grep -vc forza; done | awk '$1<2{b++} END{print NR, b+0}'`
   → `18 0`.
6. Totale catture: `grep -c '^cattura' /tmp/h29.txt | awk '{print ($1>=357)?"OK":"POCHE"}'` → `OK` (321 + 2 per voce).
7. Voci singole: `cd decomp/harness && for a in 0x57CD 0x5995 0x5345; do ./harness $a > /dev/null; echo -n "$? "; done; echo`
   → `0 0 0 `.
8. `sched_proto`: `cd decomp && harness/sched_proto out/sched_log.txt | tail -1`
   → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
9. Replay: `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r29.txt; echo "exit $?"; tail -4 /tmp/r29.txt`
   → `exit 0` e le quattro righe (misurate):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
10. Stub: `grep -c STUB_NEMICI decomp/harness/nemici_stub.c; grep -c 'STUB_NEMICI(57cd)' decomp/harness/nemici_stub.c`
    → `25` e `0` (attesa, non misurata).
11. 5B09 senza voce nel banco: `grep -c '"0x5B09"' decomp/harness/routine_tab.c; grep -c 'sub_5b09\|5b09' decomp/harness/nemici_tipo09b.c | awk '{print ($1>=1)?"OK":"MANCA"}'`
    → `0` e `OK`.
12. Copertura: il comando del punto 5 seguito da `| awk -F'[:%]' '{if($2+0<85)b++} END{print NR, b+0}'`
    → `4 0` (soglia 85% per ognuno dei 4 file; attesa, non misurata).
13. Dimensioni: `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/nemici*.c | awk '$1>700 && $2!="totale" && $2!="total"'` → vuoto.
14. File vietati intatti: `git diff --name-only HEAD | grep -E 'cattura\.sh|snap\.lua|sessioni|replay\.|sched|rst20|rst38_tables|censimento_nemici\.py|STATO\.md|README\.md'` → vuoto.
15. Ambito: `git status --short | grep -v -E '^( M|\?\?|A ) (compiti/29-|decomp/(harness/(nemici[a-z0-9_]*\.[ch]|routine_tab\.c)|ROUTINE\.md|CENSIMENTO-NEMICI\.md|out/(snap_0x[0-9A-F]{4}_|nemici_tipo09\.dasm)))'`
    → vuoto; `git status --short | grep -c -E '\.(gcda|gcno)$'` → `0`.
16. Documenti: `grep -c '^## Motore dei nemici (lotto N3)$' decomp/ROUTINE.md; grep -c 'Lotto N3 fatto' decomp/CENSIMENTO-NEMICI.md`
    → `1` e `1`.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto N3 del motore dei nemici: tipo 9 con 5 dipendenze di N6 e N10, catture nel banco`
  e riga finale `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`
  (schema in prova: lo sviluppatore gira su Sonnet; se il lotto passa a Opus
  per escalation, la riga diventa `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`).
  Il commit comprende il disassemblato `decomp/out/nemici_tipo09.dasm` e le
  catture nuove (`prima.bin`, `dopo.bin`, `regs.txt`, `.log`).
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-16; l'elenco
  delle voci con numero di catture e stato (`verificata`/`tradotta`); le
  forzature che hanno aperto o non hanno aperto il ramo voluto; le righe gcov
  non coperte; le deviazioni dal brief.

## Domande aperte

1. **Le 5 dipendenze fuori lotto** (52FD, 5345, 54C5 da N6; 51D7, 24DF da
   N10: 516 B, +54% sul lotto, 5345 da sola 384 B). Senza di esse nessuna
   cattura di 57CD o 5995 può passare, perché il banco confronta lo stato
   finale. Proposta: tradurle qui, ognuna nel modulo del censimento, come
   fatto in N2 per 571D e compagne; N6 e N10 le troveranno fatte. —
   «scelta di progetto»
2. **Divisione in `nemici_tipo09.c` e `nemici_tipo09b.c`**: tutto in un file
   darebbe 704-780 righe (non ~660) contro il limite di 700 dei `nemici*.c`.
   Proposta: 5995 e le sue code in `nemici_tipo09b.c`, come prescritto sopra. —
   «preferenza»
3. **5B09 tradotta ma senza catture**: mai eseguita nelle sessioni storiche e
   non raggiungibile con le forzature di `snap.lua` (dipende dalle celle che
   5C32 scrive in c07e-c081). Proposta: `static` in `nemici_tipo09b.c`, riga
   in `ROUTINE.md` con stato `tradotta` e nota «senza cattura»; il lotto
   resta passato. Alternativa: estendere `snap.lua` con forzature di RAM
   assoluta, fuori dal perimetro. — «scelta di progetto»
4. **Voci con una sola stanza utile** (57E4: solo stanza 33 e `finale`).
   Proposta: se `stanza33` N=2 non esiste, la seconda cattura viene da
   `finale`. — «preferenza»
5. **Aggiornamento di `STATO.md` e `README.md`**: lo sviluppatore non li
   legge né li tocca. Proposta: li aggiorna la chat principale dopo la
   verifica. — «preferenza»

## Risoluzioni (chat principale, 2026-10-29)

1. **Accolta la proposta**: le 5 dipendenze si traducono qui, ognuna nel
   modulo del censimento. Stesso precedente di N2 (6 dipendenze anticipate);
   senza di esse nessuna cattura passa, quindi non c'è alternativa praticabile.
2. **Accolta la proposta**: divisione in `nemici_tipo09.c` e
   `nemici_tipo09b.c` come prescritto.
3. **Accolta la proposta**: 5B09 `static`, stato `tradotta` con nota «senza
   cattura». Categoria già esistente in `ROUTINE.md`; l'estensione di
   `snap.lua` resta fuori perimetro.
4. **Accolta la proposta**: seconda cattura di 57E4 da `finale` se
   `stanza33` N=2 non esiste.
5. **Accolta la proposta**: `STATO.md` e `README.md` li aggiorna la chat
   principale dopo la verifica.
