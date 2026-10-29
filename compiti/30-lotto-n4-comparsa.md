# Compito 30: lotto N4 del motore dei nemici, comparsa (tipi 0, 4, 5 di 4F1B)

Data: 2026-10-29. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere.

## Contesto: file da leggere

Da leggere (e basta):
1. `decomp/out/nemici_comparsa.dasm`, da generare (vedi sotto): il
   disassemblato delle 28 routine del lotto.
2. `decomp/harness/rst38_tables.h`: solo `rst38_4FAC` (r. 22-25),
   `rst38_728B` (r. 101-104) e `rst38_73B6` (r. 106-109).
3. `decomp/harness/nemici.h` (207 righe): prototipi, `nemici_rst38`,
   `NEMICI_PONI_RIPORTO`.
4. Le convenzioni qui sotto («Convenzioni di traduzione»).

Da aprire perché si modificano, solo per la parte indicata:
`decomp/harness/nemici_comparsa.c` (50 righe, contiene 734D e 80A5 già
tradotte), `nemici_stub.c`, `routine_tab.c`, `ram.h` (macro, r. 9-30),
`routine.h` (solo prototipi, con `grep`), `nemici_tipo09.c` (solo
`prendi_portato`, r. 21-30, esempio di stile per `rst 8; rst 10h`),
`decomp/ROUTINE.md` (solo dalla r. 280 alla fine, formato della sezione N3),
`decomp/CENSIMENTO-NEMICI.md` (solo il paragrafo «Lotto N3 fatto»).

È vietato leggere `STATO.md`, `README.md`, `ROUTINE.md` per intero, gli
altri brief in `compiti/` e `decomp/harness/nemici_tipi_08_14.c`.

Generazione del disassemblato (da `/home/rufio72/OpenSolomonsKey-master`,
`awk` è mawk: il confronto è fra stringhe esadecimali minuscole):
```
cd decomp && awk 'BEGIN{n=split("134b 139c 13ab 13b9 13fc 1413 14d4 14fc 1549 2548 4f40 4fc5 7288 7294 72b6 72ce 72da 738f 73a3 73c7 7412 741d 7456 7470 74ab 756c 758e 759f",lo," "); split("135a 13aa 13b8 13c2 1412 1423 14fb 150b 1556 2550 4fc4 4fd3 728b 72b5 72cd 72d9 734c 73a2 73b6 7411 741c 7455 7467 74aa 754b 758d 759e 75a9",hi," ")}
/^[A-Za-z_0-9]+:/ {lab=$0; next}
/;[0-9a-f][0-9a-f][0-9a-f][0-9a-f][ \t]*$/ {a=substr($0,length($0)-3); ok=0; for(i=1;i<=n;i++) if(a>=lo[i] && a<=hi[i]) ok=1; if(ok){ if(lab!="") print lab; print} lab=""; next}
{lab=""}' out/main64.dasm > out/nemici_comparsa.dasm; cd ..
```
Uscita attesa (misurata): `wc -l < decomp/out/nemici_comparsa.dasm` → `519`.
Nel file: i byte delle tabelle inline 4FAD-4FB8, 728C-7293, 73B7-73C6
compaiono come istruzioni finte (le tabelle vere sono in `rst38_tables.h`);
4FB9-4FC1 è codice mal decodificato (vedi sotto). Le tabelle di dati ROM
150C-1548, 7468-746F, 754C-756B non sono nel file: si leggono con `M(addr)`.

## Obiettivo

Tradurre in C le 28 routine del lotto N4 (tema «comparsa»: tipi 0, 4 e 5 di
4F1B, cioè ingressi 4F40, 73A3, 7288, più le routine fuori zona 13xx/14xx/2548).
80A5 (N2) e 734D (N1) sono già tradotte e non si toccano. Verifica: catture
MAME nel banco (`harness --tutte`) con `cattura.sh` (stato `stanzaNN`,
`finale`), con il nuovo script `cattura_oggetti.sh` (stato «oggetti») e,
dove un ramo non si vede, `SNAP_FORZA`. Replay e `sched_proto` non devono
cambiare.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `cattura.sh`, `snap.lua`, `sessioni.sh`, `replay.*`, `sched*.c`,
  `rst20*`, `rst38_tables.h`, `censimento_nemici.py`, `STATO.md`,
  `README.md` né le catture esistenti in `decomp/out/`.
- Niente installazioni. MAME gira solo come fa `cattura.sh` (senza finestra,
  senza audio, provider a `none`): niente partite a mano. Una cattura dura
  circa 2-3 secondi.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe e nessun `nemici*.c` sopra le 700.
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`); si
  ripristina un file solo con `git checkout -- <file>`.
- Un solo agente alla volta su `decomp/harness`.

## Cosa esiste già (fatti verificati)

Misure del 2026-10-29 sui binari già compilati (da `decomp/harness`):
- `./harness --tutte`: exit 0, 412 righe `^cattura`, 0 `FALLITO`, 165 righe
  `^RIEPILOGO` (formato `RIEPILOGO 0x6EB7: 2 catture, 2 OK`).
- `grep -c STUB_NEMICI nemici_stub.c` → 25 (la `#define` più 24 righe, tra cui
  `4f40`, `7288`, `73a3`).
- Nessun file di codice di `decomp` (fuori da `out/`) sopra le 700 righe;
  `nemici_tipi_08_14.c` ne ha 697.
- Nessuna routine del lotto contiene `ld a,r` (`grep -c 'ld a,r'` sul
  disassemblato → 0): non serve nessun wrapper `banco_r_*`. Se ne comparisse
  una, la regola vale: si verifica con un wrapper `banco_r_xxxx` in
  `nemici_r.c`, come 54C5 e 5995 (intestazione di `nemici_r.c`).

Il banco `harness` include già `$(wildcard nemici*.c)`: i moduli nuovi
entrano senza toccare il Makefile.

Routine del lotto (da `CENSIMENTO-NEMICI.md` e `out/censimento_nemici.txt`).
Nessuna è coperta dal replay (colonna replay = NO; gli stub 4f40, 7288, 73a3
non fermano il replay, exit 0). «Sessioni» = `grep -lix <ADDR> decomp/out/*_eseguiti.txt`.

| ingresso | byte | chiamanti | sessioni storiche | note |
|---|---:|---|---|---|
| 4F40 | 121 | 4F0F (`rst 38` 4F1B[0]) | oggetti | tipo 0; chiama 738F, 80A5, 4FC5, `rst 38` su `rst38_4FAC` |
| 4FC5 | 15 | 4F40, 73C7 | nessuna | vedi «Routine non catturabili» |
| 134B | 16 | 13B9 | oggetti | |
| 139C 13AB | 15 14 | 4F40 (4FAC[2], [1]) | oggetti | chiamano 0493 (già tradotta) e 13FC |
| 13B9 | 10 | 4F40 (4FAC[3]) | oggetti | chiama 134B e 13FC |
| 13FC | 23 | 139C 13AB 13B9 1413 14D4 14FC 4F40 73C7 | oggetti, stanza 06 | `jp z` a 14FC; tabella dati 150C; chiama 1549, 0493 |
| 1413 | 17 | 4F40 (4FAC[4]) | oggetti | chiama 13FC, 349F |
| 14D4 | 40 | 4F40 (4FAC[5]) | oggetti | chiama 13FC, 349F |
| 14FC | 16 | 13FC, 4F40 (4FAC[0]) | nessuna | vedi «Routine non catturabili» |
| 1549 | 14 | 13FC | oggetti, stanza 06 | |
| 2548 | 9 | 759F | stanze 03 12 21 31 40 50 e finale | |
| 7288 | 4 | 4F0F (`rst 38` 4F1B[5]) | stanze 02 11 17 20 28 38 50 e finale | tipo 5: `ld e,(iy+1d); rst 38` su `rst38_728B` |
| 7294 72B6 72CE | 34 24 12 | 7288 ([0] [2] [3]) | come 7288 | 72B6 chiama 734D |
| 72DA | 115 | 7288 ([1]) | stanze 11 17 28 38 50 e finale | chiama 4938, 4AF6, 734D, 80A5 |
| 738F | 20 | 4F40, 73A3 | stanze 03 12 21 31 40 50, finale, oggetti | uscita: riporto |
| 73A3 | 20 | 4F0F (`rst 38` 4F1B[4]) | stanze 03 12 21 31 40 50, finale, oggetti | tipo 4; `rst 38` su `rst38_73B6` (tail: la destinazione ritorna al chiamante) |
| 73C7 | 75 | 73A3 ([7]) | nessuna | vedi «Routine non catturabili» |
| 7412 | 11 | 73A3 ([0]) | finale (e altre sessioni storiche) | catturabile solo da `finale` |
| 741D 7470 | 57 59 | 73A3 ([1] [2]) | come 73A3 | tabella dati 7468; chiamano la coda 7456 |
| 7456 | 18 | 741D (cade), 7470 (`jr`) | come 73A3 | ingresso interno (coda condivisa) |
| 74AB | 161 | 73A3 ([6]) | come 73A3 | tabelle dati 754C e 755C; chiama 756C, 758E, 759F |
| 756C 758E 759F | 34 17 11 | 74AB | come 73A3 | 756C e 758E: riporto in uscita; 759F chiama 2548 |

Totale: 982 B. Già tradotte e da usare (prototipi in `routine.h` o
`nemici.h`, da grep): `sub_0008`, `sub_0010`, `sub_0028` (preserva A, B, C,
H, L; da `grafo symbol`), `sub_0493`, `sub_349f`, `sub_31ce`, `sub_31e9`,
`sub_36f0`, `sub_3b62`, `sub_4938`, `sub_4af6`, `sub_734d`, `sub_80a5`.

Dati utili (da `out/main64.bin`, misurati):
- Sito 4FAC: `ld hl,4fc2; push hl; rst 38h` a 4FA8-4FAC; tabella
  `rst38_4FAC` = 14FC, 13AB, 139C, 13B9, 1413, 14D4. Le destinazioni
  ritornano a 4FC2 (`jp 80a5`). L'indice E è
  `((IY+1D) - 1) * 2 + ((IY+1E) >> 1)` e se è >= 6 il salto 4FA0 va a 4FB9.
- 4FB9-4FC1 (byte `C6 23 0E 10 EF 4F CD FC 13`, mal decodificati nel
  disassemblato): `add a,23h; ld c,10h; rst 28h; ld c,a; call 13FC`, poi
  cade in 4FC2.
- `rst38_728B` = 7294, 72DA, 72B6, 72CE (indice (IY+1D)).
- `rst38_73B6` = 7412, 741D, 7470, 3B62, 3B62, 3B62, 74AB, 73C7 (indice
  (IY+1D); 73A3 lo pone a 7 se 738F dà riporto).

### Routine non catturabili (14FC, 4FC5, 73C7)

Nessuna sessione storica le ha mai eseguite (colonna unione = NO) e il
log `out/partita_viva_20261027*.log` mostra solo `DEST 4FC5`, cioè la
chiamata di 4F40 a 4F8F: nelle catture `snap.lua` il breakpoint d'ingresso
non scatta mai, quindi `SNAP_FORZA` (applicata all'ingresso catturato) non
può aprirle direttamente. Sono però raggiungibili forzando i chiamanti, e
la cattura del chiamante confronta lo stato finale: le routine interne sono
verificate lì. Misure del 2026-10-29 (cattura con `SNAP_FORZA`, esito sul
«dopo» rispetto al «prima»):
- `4F40` in stato «oggetti» con `iy+1d:01,iy+1e:00,ix+4:00,ix+5:00`:
  c05a 00 → 80 (4FC5 eseguita) e c237 3 → 4 (14FC eseguita). Anche
  `iy+1d:02,iy+1e:02,ix+4:00,ix+5:00` e `iy+1d:03,iy+1e:02,ix+4:00,ix+5:00`
  mettono c05a = 80.
- `73A3` in `stanza03` con `iy+1d:07,ix+1:00`: c250 0 → 1 e c05a = 80 (73C7
  primo ramo, che chiama 4FC5 e 13FC); `iy+1d:07,ix+1:00,iy+4:0d` idem;
  `iy+1d:07,ix+1:10,ix+8:20` esce da 73C7 per il ramo 7409/80A5 (c250
  invariato); `iy+1d:07,ix+1:10,ix+8:05` per `ret c`.

Decisione: 14FC è `static` in `nemici_comparsa.c` (chiamanti solo lì); 73C7
è `static` in `nemici_comparsab.c`; 4FC5 non è statica (la chiamano 4F40 e
73C7 in moduli diversi), ha il prototipo in `nemici.h` ma nessuna voce in
`routine_tab.c`. Nessuna delle tre ha voce nel banco: in `ROUTINE.md` stato
`tradotta`, nota «senza cattura diretta: coperta dalle catture forzate di
4F40 / 73A3». Non si usa la categoria 5B09 («mai eseguita, non raggiungibile»):
qui il ramo si raggiunge, per questo la verifica 11 misura la copertura.

### Convenzioni di traduzione

- Ogni routine è `void sub_xxxx(struct z80 *z)`. Registri `z->a b c d e h l`,
  `z->ix`, `z->iy` (uint16_t), `z->f`. Memoria con `M(indirizzo)` (RAM c000-ffff),
  coppie con `BC(z) DE(z) HL(z)` e `SET_BC SET_DE SET_HL` (`ram.h`).
- Il banco confronta RAM (c000-e5ff) e A, BC, DE, HL, IX, IY all'uscita, non F:
  anche i registri «sporchi» lasciati dal codice originale devono coincidere.
  Il riporto, dove un chiamante lo legge (738F, 756C, 758E), sta nel bit 0
  di `z->f` (`NEMICI_PONI_RIPORTO(z,c)`, `NEMICI_RIPORTO_ACCESO(z)`).
- `rst 38h` a un sito con tabella inline: `nemici_rst38(z, sito, tab, voci)`
  restituisce la destinazione (0 fuori tabella) e mette DE = sito+2+2E; HL e
  A restano intatti. Poi si smista con uno `switch` (esempio: `salta_4f1b` in
  `nemici.c`). Siti qui: 0x4FAC con `rst38_4FAC` (6 voci; prima
  `SET_HL(z, 0x4FC2)` per il `push hl`; dopo la destinazione si esegue 4FC2,
  cioè `sub_80a5`), 0x728B con `rst38_728B` (4 voci), 0x73B6 con
  `rst38_73B6` (8 voci; la [3], [4], [5] sono `sub_3b62`).
- `jp`/`jr` verso un'altra routine = chiamata e `return`. Un ingresso interno
  senza cattura propria (7456) è una `static` condivisa.
- `ld hl,tab; rst 8; rst 10h` (7422, 7475, 74F5, 1403) segue il pattern di
  `prendi_portato`: `sub_0008`, poi `sub_0010`. Le tabelle ROM si leggono con
  `M(addr + indice)`; sotto c000 la scrittura è ignorata.
- `rst 28h` = `sub_0028` (suono, C = codice).
- Nessun `NEMICI_PUNTO`: il replay non raggiunge queste routine.
- Catture `forza*` valgono come catture OK ma non contano per lo stato
  `verificata`, che richiede almeno 2 catture non forzate.

## Struttura da realizzare

**1. Moduli C.** Stima righe a 0,6-0,68 per byte (N3: 446 righe per 597+
byte): 982 B danno 589-668 righe più le 50 esistenti = 639-718, troppo vicino
al limite di 700. Quindi:
- `nemici_comparsa.c` (50 righe oggi, ~245-270 dopo): 13xx/14xx (134B, 139C,
  13AB, 13B9, 13FC, 1413, 14D4, 14FC `static`, 1549), 4F40, 4FC5, 738F, più
  734D e 80A5 esistenti. Aggiornare il commento di intestazione.
- `nemici_comparsab.c` (nuovo, ~400-450 righe): 7288, 7294, 72B6, 72CE, 72DA,
  73A3, 73C7 `static`, 7412, 741D, 7456 `static`, 7470, 74AB, 756C, 758E,
  759F, 2548. Intestazione con `#include "routine.h"`, `#include "nemici.h"`,
  `#include "rst38_tables.h"`.
Se dopo la scrittura un file supera le 700 righe, si sposta 72DA o 74AB nel
modulo più corto.

**2. `nemici.h`**: sezione «Lotto N4» con i prototipi non statici: 134B, 139C,
13AB, 13B9, 13FC, 1413, 14D4, 1549, 2548, 4F40, 4FC5, 7288, 7294, 72B6, 72CE,
72DA, 738F, 73A3, 7412, 741D, 7470, 74AB, 756C, 758E, 759F (25). Si tolgono
`sub_4f40`, `sub_7288`, `sub_73a3` dal blocco degli stub e il blocco «Lotto N1,
nemici_comparsa.c» si lascia. `nemici_stub.c`: si tolgono `STUB_NEMICI(4f40)`,
`(7288)`, `(73a3)`. In `nemici.c` non cambia niente.

**3. `routine_tab.c`**: una voce `{ "0xXXXX", sub_xxxx, 0, 0 }` per ognuno
dei 24 ingressi con voce (i 25 del punto 2 meno 4FC5), in ordine di
indirizzo: 134B 139C 13AB 13B9 13FC 1413 14D4 1549 2548 4F40 7288 7294 72B6
72CE 72DA 738F 73A3 7412 741D 7470 74AB 756C 758E 759F. 80A5 c'è già.

**4. `decomp/cattura_oggetti.sh`** (nuovo, da copiare esatto). Replica lo
stato della sessione «oggetti» di `sessioni.sh` (oggetti 08-1B in coda c05b
e posta 22), ripetuto due volte perché le routine 13AB, 13B9, 14D4 si
eseguano almeno due volte; passa per `cattura.sh` (stato `gioco`, che non
cambia). Misurato: con questo script 13AB e 14D4 danno N=1 e N=2.
```
#!/bin/sh
# Cattura prima/dopo nello stato «oggetti» (brief 30): lo stato gioco di
# cattura.sh con gli oggetti 08-1B in coda c05b e la posta 22, come la
# sessione «oggetti» di sessioni.sh, ripetuti due volte (14 terne, una posta
# ogni 100 frame da 2300 a 3600). Uso (da decomp/):
#   sh cattura_oggetti.sh <ENTRY> [N] [SUFFISSO]
# I file sono out/snap_<ENTRY>_gioco_oggetti[_SUFFISSO]_*.
set -u
POKE=
POSTA=
f=2250
for terna in "08 09 0a" "0b 0c 0d" "0e 0f 10" "11 12 13" "14 15 16" "17 18 19" "1a 1b 00" \
             "08 09 0a" "0b 0c 0d" "0e 0f 10" "11 12 13" "14 15 16" "17 18 19" "1a 1b 00"; do
    a=$((0xc05b))
    for o in $terna; do
        POKE="$POKE$f:$(printf 0x%x $a):0x$o,$f:$(printf 0x%x $((a + 1))):0,"
        a=$((a + 2))
    done
    POSTA="$POSTA$((f + 50)):0x22,"
    f=$((f + 100))
done
export SNAP_POKE="$POKE" SNAP_POSTA="$POSTA"
exec sh cattura.sh "$1" gioco "${2:-1}" "oggetti${3:+_$3}"
```

**5. Catture.** Da `decomp/`. Almeno 2 catture non forzate per voce. Piano
(misurato il 2026-10-29: ogni cattura indicata riesce; una seconda cattura
dello stesso stato usa N=2 e un suffisso `n2`, altrimenti sovrascrive la
prima):

| voci | catture |
|---|---|
| 0x4F40 0x134B 0x139C 0x13AB 0x13B9 0x13FC 0x1413 0x14D4 0x1549 | `sh cattura_oggetti.sh <ENTRY> 1` e `sh cattura_oggetti.sh <ENTRY> 2 n2` |
| 0x7288 0x7294 0x72B6 0x72CE | `stanza02`, `stanza11` (N=1) |
| 0x72DA | `stanza11`, `stanza17` (N=1; `stanza02` non cattura) |
| 0x2548 0x738F 0x73A3 0x741D 0x7470 0x74AB 0x756C 0x758E 0x759F | `stanza03`, `stanza12` (N=1) |
| 0x7412 | `finale` N=1 e `sh cattura.sh 0x7412 finale 2 n2` (nelle stanze non scatta) |

Dopo il primo giro si misura la copertura (punto 6) e si aggiungono
forzature per i rami non eseguiti, con `SNAP_FORZA` (voci `ix+HH:VV`,
`iy+HH:VV`, `r:VV`; la forzatura è applicata all'ingresso voluto, quindi vale
solo per rami decisi dentro la routine catturata). Suffisso `forza` o
`forzaNNNN`, es. `SNAP_FORZA='iy+1d:07,ix+1:00' sh cattura.sh 0x73A3 stanza03 1 forza`,
`SNAP_FORZA='iy+1d:01,iy+1e:00,ix+4:00,ix+5:00' sh cattura_oggetti.sh 0x4F40 1 forza`.
Forzature **obbligatorie**, che aprono 14FC, 4FC5, 73C7:
- 4F40: le due misurate sopra (indice 4FAC 0 per 14FC; 4FC5 in tutte) più
  `iy+1d:01,iy+1e:02` (13AB), `iy+1d:02,iy+1e:00` (139C), `iy+1d:02,iy+1e:02`
  (13B9), `iy+1d:03,iy+1e:00` (1413), `iy+1d:03,iy+1e:02` (14D4) e
  `iy+1d:04,iy+1e:00` (indice 6: ramo 4FB9), tutte con `ix+4:00,ix+5:00`
  (queste sei: attese, non misurate); ramo con `iy+1d:00`: `iy+1d:00,ix+8:30,ix+e:00`
  (misurato: esce per 80A5).
- 73A3: le quattro misurate sopra (primo ramo di 73C7, ramo con
  `iy+4:0d`, ramo 7409, `ret c`).
- Altri esempi dal disassemblato (non misurati): `7412` con `ix+8:20` e
  `ix+8:00`; `13FC` con `a:36` (apre 14FC per `jp z`).
Il ramo di 73C7 con (c238)+1 >= 10 (73E0-73EB) dipende da una cella assoluta
che `snap.lua` non forza: può restare scoperto, da elencare nel rapporto.

**6. Copertura del C** (gcov, fuori dal repository; correggere il prefisso
`h-` del `.gcda`):
```
D=/tmp/cov30 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
 && ln -s "$PWD/decomp/out" $D/out && (cd $D && gcc -std=c11 -O0 --coverage -o h \
 harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c z80_*.c \
 nemici*.c attivita.c attivita_01_10.c && ./h --tutte > /dev/null; \
 for f in nemici_comparsa nemici_comparsab; do \
 gcov -n h-$f.gcda | grep -A1 "File '$f.c'" | tail -1; done)
```
(Formato dell'uscita, misurato su `nemici_tipo09`: `Lines executed:98.98% of 197`.)
Nel rapporto: le righe non eseguite di ogni file e il motivo.

**7. Documenti.** `ROUTINE.md`: in fondo, sezione «## Motore dei nemici (lotto
N4)» con la stessa tabella della N3 (ingresso, funzione, file, byte,
descrizione, stato, catture OK, stati, note), una riga per ognuno dei 24
ingressi nel banco più 14FC, 4FC5, 73C7 e 7456 (tradotte, senza voce
propria); elenco delle catture forzate e nota sullo stato «oggetti»
(`cattura_oggetti.sh`; le routine 13xx/14xx sono eseguite dalla sessione
«oggetti» perché l'attività 22 / 12D6 le chiama dalla tabella 130A: le
catture hanno «dopo» a PC=12E8). `CENSIMENTO-NEMICI.md`: sotto «Piano dei lotti»
un paragrafo «**Lotto N4 fatto** (brief 30)» con la divisione in
`nemici_comparsa.c`/`nemici_comparsab.c`, le tre routine senza voce nel banco
e `cattura_oggetti.sh`. Si aggiunge anche una riga all'elenco dei moduli se
manca `nemici_comparsab.c`.

## Verifiche obbligatorie

Tutte da `/home/rufio72/OpenSolomonsKey-master`; ogni uscita attesa è esatta
salvo dove è scritto `>=` o «attesa, non misurata». Abbreviazione usata nei
cicli: `V="134B 139C 13AB 13B9 13FC 1413 14D4 1549 2548 4F40 7288 7294 72B6 72CE 72DA 738F 73A3 7412 741D 7470 74AB 756C 758E 759F"`.

1. Disassemblato: `wc -l < decomp/out/nemici_comparsa.dasm` → `519` (misurata).
2. Compilazione: `cd decomp/harness && make harness replay sched_proto > /tmp/m30.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/m30.txt`
   → `make 0` e `0`.
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h30.txt; echo "exit $?"; grep -c FALLITO /tmp/h30.txt; grep -c '^RIEPILOGO' /tmp/h30.txt`
   → `exit 0`, `0`, `189` (165 + 24 voci nuove).
4. Catture per voce, riepilogo del banco:
   `V="..."; for a in $V; do grep "^RIEPILOGO 0x$a:" /tmp/h30.txt; done | awk '{if($3<2||$3!=$5)b++} END{print NR, b+0}'`
   → `24 0`.
5. Catture non forzate per voce:
   `cd decomp && V="..."; for a in $V; do ls out/snap_0x${a}_*_regs.txt | grep -vc forza; done | awk '$1<2{b++} END{print NR, b+0}'`
   → `24 0`.
6. Totale catture: `grep -c '^cattura' /tmp/h30.txt | awk '{print ($1>=460)?"OK":"POCHE"}'` → `OK` (412 + 2 per voce).
7. Voci singole: `cd decomp/harness && for a in 0x4F40 0x73A3 0x7288; do ./harness $a > /dev/null; echo -n "$? "; done; echo`
   → `0 0 0 `.
8. `sched_proto`: `cd decomp && harness/sched_proto out/sched_log.txt | tail -1`
   → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
9. Replay: `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r30.txt; echo "exit $?"; tail -4 /tmp/r30.txt`
   → `exit 0` e le quattro righe (misurate):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
10. Stub: `grep -c STUB_NEMICI decomp/harness/nemici_stub.c; grep -c -E 'STUB_NEMICI\((4f40|7288|73a3)\)' decomp/harness/nemici_stub.c`
    → `22` e `0` (attesa, non misurata).
11. Routine senza voce, coperte dal chiamante: `grep -c -E '"0x(14FC|4FC5|73C7)"' decomp/harness/routine_tab.c` → `0`; poi, dopo il comando del punto 6 di «Struttura»:
    `cd /tmp/cov30 && gcov -n -f h-nemici_comparsa.gcda h-nemici_comparsab.gcda | grep -A1 -E "Function '(sub_4fc5|sub_14fc|sub_73c7)'" | grep 'Lines executed' | awk -F'[:%]' '{if($2+0<75)b++} END{print NR, b+0}'`
    → `3 0` (attesa, non misurata: soglia 75% per ognuna delle tre funzioni).
12. Copertura: il comando del punto 6 di «Struttura» seguito da `| awk -F'[:%]' '{if($2+0<85)b++} END{print NR, b+0}'`
    → `2 0` (soglia 85% per ognuno dei 2 file; attesa, non misurata).
13. Dimensioni: `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/nemici*.c | awk '$1>700 && $2!="totale" && $2!="total"'` → vuoto.
14. File vietati intatti: `git diff --name-only HEAD | grep -E 'cattura\.sh|snap\.lua|sessioni|replay\.|sched|rst20|rst38_tables|censimento_nemici\.py|STATO\.md|README\.md'` → vuoto
    (nota: `cattura_oggetti.sh` non corrisponde al pattern `cattura\.sh`).
15. Ambito: `git status --short | grep -v -E '^( M|\?\?|A ) (compiti/30-|decomp/(cattura_oggetti\.sh|harness/(nemici[a-z0-9_]*\.[ch]|routine_tab\.c)|ROUTINE\.md|CENSIMENTO-NEMICI\.md|out/(snap_0x[0-9A-F]{4}_|nemici_comparsa\.dasm)))'`
    → vuoto; `git status --short | grep -c -E '\.(gcda|gcno)$'` → `0`.
16. Script e documenti: `sh -n decomp/cattura_oggetti.sh; echo $?; grep -c '^## Motore dei nemici (lotto N4)$' decomp/ROUTINE.md; grep -c 'Lotto N4 fatto' decomp/CENSIMENTO-NEMICI.md`
    → `0`, `1` e `1`.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto N4 del motore dei nemici: comparsa (tipi 0, 4, 5), catture nel banco`
  e riga finale `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`
  (se il lotto passa a Opus per escalation, la riga diventa
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`).
  Il commit comprende il disassemblato `decomp/out/nemici_comparsa.dasm`,
  `decomp/cattura_oggetti.sh` e le catture nuove (`prima.bin`, `dopo.bin`,
  `regs.txt`, `.log`).
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-16; l'elenco
  delle voci con numero di catture e stato (`verificata`/`tradotta`); le
  forzature che hanno aperto o non hanno aperto il ramo voluto (in
  particolare per 14FC, 4FC5, 73C7); le righe gcov non coperte; le deviazioni
  dal brief.

## Domande aperte

1. **Divisione in `nemici_comparsa.c` e `nemici_comparsab.c`**: con il metro
   0,6-0,68 righe per byte il solo `nemici_comparsa.c` arriverebbe a 639-718
   righe contro il limite di 700. Proposta: dividere come prescritto; 4F40 e
   13xx/14xx restano nel modulo esistente. — «preferenza»
2. **14FC, 4FC5, 73C7 senza voce nel banco**: mai eseguite in nessuna sessione,
   il breakpoint d'ingresso non scatta; sono però raggiungibili forzando 4F40
   e 73A3 (misurato). Proposta: tradotte (14FC e 73C7 `static`, 4FC5 con
   prototipo in `nemici.h` perché usata da due moduli), stato `tradotta` con
   nota «senza cattura diretta», verificate dalla copertura gcov (verifica 11)
   e dalle catture forzate dei chiamanti. Applica la regola già ratificata per
   le routine non catturabili. — «preferenza»
3. **Nuovo `decomp/cattura_oggetti.sh`**: le routine 4F40 e 13xx/14xx si
   eseguono solo nella sessione «oggetti» (oggetti 08-1B, posta 22), per cui
   `cattura.sh stanzaNN` non basta. Proposta: uno script nuovo che passa
   `SNAP_POKE`/`SNAP_POSTA` a `cattura.sh` (che non si modifica). Alternativa:
   aggiungere uno stato `oggetti` a `cattura.sh`, fuori dal perimetro. —
   «preferenza»
4. **7412**: nelle stanze non scatta mai in cattura, solo in `finale`.
   Proposta: seconda cattura non forzata da `finale` N=2 (suffisso `n2`). —
   «preferenza»
5. **Soglia di copertura 85%** per i due moduli e 75% per le tre routine
   senza voce: il ramo 73E0-73EB di 73C7 dipende da (c238) assoluta e non è
   forzabile. Proposta: soglie come scritto, righe scoperte elencate nel
   rapporto. — «preferenza»
6. **Aggiornamento di `STATO.md` e `README.md`**: lo sviluppatore non li
   legge né li tocca. Proposta: li aggiorna la chat principale dopo la
   verifica. — «preferenza»
