# Compito 27: lotto N2 del motore dei nemici, con catture forzate

Data: 2026-10-28. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere.

## Obiettivo

Tradurre in C le 33 routine del lotto N2 (tipi 8 e 14 della tabella 4F1B,
modulo nuovo `decomp/harness/nemici_tipi_08_14.c`) più le 6 dipendenze
piccole che servono a verificarle. La verifica sono le catture MAME nel banco
(`harness --tutte`), ottenute forzando la stanza o pochi byte di RAM/registri.
Replay e `sched_proto` servono da regressione e non devono cambiare.
Prima di tutto si sistemano `cattura.sh`/`sessioni.sh`: provider di input a
`none` e stato `bonus`.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `replay.lua`, `replay.sh`, `replay.c`, `sched*.c`, `rst20.py`,
  `rst20_tables.h`, `rst38_tables.h`, `censimento_nemici.py` né le catture
  esistenti in `decomp/out/`.
- Niente installazioni. MAME (`mame`, in `/usr/games`) gira solo senza
  finestra: `SDL_VIDEODRIVER=dummy -video none -sound none` più i quattro
  provider a `none`. Nessuna partita a mano e nessuna sessione con finestra.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra
  le 800 righe, e nessun modulo `nemici*.c` sopra le ~700.
- Niente git distruttivo: niente `stash`, `reset --hard` o `filter-branch`.
  Si ripristina solo il singolo file con `git checkout -- <file>`.

## Cosa esiste già (fatti verificati)

| file | righe | ruolo |
|---|---:|---|
| `decomp/cattura.sh` | 67 | cattura prima/dopo; stati titolo, gioco, tardo, porta, bonus, finale; MAME a r. 58-59 **senza** provider |
| `decomp/snap.lua` | 176 | breakpoint su SNAP_ENTRY, dump c000-ffff prima/dopo, scarta le catture interrotte dalla NMI; SNAP_POKE/SNAP_POSTA ai frame; nessun forzamento della stanza |
| `decomp/sessioni.sh` | 132 | `MOPT` a r. 14 senza provider; `bonus_04` usa `SESS_ROOM=4` più il poke `c24a=0` (r. 65-67) |
| `decomp/sessione.lua` | 189 | stanza forzata a r. 133-136: read tap su c243 dal frame 1500 al 1950 |
| `decomp/replay.sh` | 32 | riga MAME giusta (r. 20-24) |
| `decomp/harness/Makefile` | 42 | `harness` = `SORGENTI` senza `nemici*.c`; `replay` = `SORGENTI_REPLAY` con `$(wildcard nemici*.c)` |
| `decomp/harness/harness.c` | 193 | `prova_routine` cerca `snap_<ENTRY>_regs.txt` e `snap_<ENTRY>_*_regs.txt` in `$SNAP_DIR`, `out`, `../out`; con 0 catture la voce fallisce |
| `decomp/harness/routine_tab.c` | 121 | `tabella_routine[]` `{ "0xXXXX", sub_xxxx, blocco_ini, blocco_fine }` in ordine di indirizzo |
| `decomp/harness/nemici.h` | 139 | prototipi N1 e stub, `NEMICI_PONI_RIPORTO`, `nemici_rst38`, `nemici_rst20`, `NEMICI_PUNTO` |
| `decomp/harness/nemici_stub.c` | 37 | 27 `STUB_NEMICI(x)`, tra cui `6ce8` (r. 25) e `6eca` (r. 26) |
| `decomp/harness/nemici.c` | 204 | `salta_4f1b` (r. 56) chiama `sub_6ce8` e `sub_6eca`; `sub_4f0f` (r. 79) |
| `decomp/harness/nemici_tipi_10_13.c` | 276 | modulo N5 (modulo di 571D, 57AA, 57B3) |
| `decomp/harness/nemici_tipo09.c` | 96 | modulo N3 (modulo di 6EB7, 6EC1); usa `sub_4b98` di `attivita.c` |
| `decomp/harness/nemici_comparsa.c` | 38 | modulo N4 (modulo di 80A5) |
| `decomp/ROUTINE.md` | 196 | tabella principale (r. 10) e sezione «Motore dei nemici (lotto N1)» (r. 161) |

Strutture (commento di `nemici.c`): 17 slot da 0x18 byte a c2c1 (IX) e 17
oggetti da 0x30 byte a c4fe (IY). `sub_4f0f` (da `grafo symbol`) ritorna se
(IY+4) < 6, altrimenti E = (IY+3) − 6 e fa `rst 38` a 4F1B. Il tipo 8 è
6ECA ((IY+3) = 0x0E), il tipo 14 è 6CE8 ((IY+3) = 0x14).

Firme e helper (da `grafo symbol`): tutte le routine sono
`void sub_xxxx(struct z80 *z)`, con il riporto nel bit 0 di `z->f`
(`NEMICI_PONI_RIPORTO(z,c)` e `NEMICI_RIPORTO_ACCESO(z)`).
`nemici_rst38(z, sito, tab, voci)` restituisce `tab[E]` e mette
DE = sito+2+2E. `nemici_rst20(z, sito, tab, voci)` restituisce `tab[A]`.
Fuori tabella entrambe restituiscono 0 e registrano il sito. Le routine già
tradotte che N2 chiama sono `sub_0008`, `sub_0010`, `sub_0018`, `sub_0028`,
`sub_36f0`, `sub_4af6`, `sub_5c32`, `sub_7379` (in `routine.h`) e
`sub_4938` (`nemici_comuni.c`, N1).

Composizione di N2 (da `decomp/out/censimento_nemici_lotti.txt`): 33
ingressi, 1030 B, ~618 righe stimate. «Unione» = copertura nelle sessioni
storiche. Le sessioni che hanno eseguito ogni ingresso stanno in
`decomp/out/stanza_NN_eseguiti.txt` (NN = stanza 0-based). Misurate con
`grep -lx <ADDR> out/*_eseguiti.txt`:

| ingresso | byte | unione | come ci si arriva | stanze che lo eseguono |
|---|---:|---|---|---|
| 6CE8 | 28 | PARZ | tipo 14: chiama 57AA (BC = c1a6 o c1a8 su bit 3 di IY+4), 57B3 se NC; poi `rst 38` a 6D03 su (IY+1D) | 10 14 16 23 24 34 48 |
| 6D12 | 9 | SI | 6D03[3] | 10 14 24 34 48 |
| 6D1B | 28 | PARZ | 6D03[1] | 10 14 24 34 48 |
| 6D37 | 151 | PARZ | 6D03[2], anche dal tipo 15 (6C0B) | 42 (finale) |
| 6DCE | 135 | PARZ | 6D03[4], anche 6C0B; chiama 4938, 6EB7, 6EC1 | 42 (finale) |
| 6E55 | 46 | PARZ | 6D03[5]; 6E90 con HL = 0816, 5C32 | 10 14 24 34 48 |
| 6E83 | 8 | NO | interno a 6E55: se 6E90 dà riporto, cioè \|(IX+5)\| < 0x16 e (IX+4) < 0x08 | nessuna |
| 6E8B | 5 | SI | coda di 6E55 | 42 (finale) |
| 6E90 | 17 | SI | `call` da 6E55 e 6C8E | 10 14 02 22 … |
| 6EA1 | 22 | PARZ | 6D03[6], anche 6C0B; chiama 6EB7 | 10 02 22 23 … |
| 6ECA | 165 | PARZ | tipo 8: velocità da 6F3D[(IY+4)−0x16] in c1a0, 70FE, B = 70EE[A], `push 7056`, `rst 20` a 6F1C su A | 08 11 19 22 27 32 … |
| 6F49 | 2 | SI | 6F1C[0] e [15] (`or a; ret`) | 08 11 19 … |
| 6F4B 6F5E 6F6E 6F81 | 19 16 19 16 | SI | 6F1C[1] [2] [4] [8] | 08 11 (6F4B, 6F81); 11 02 22 (6F5E, 6F6E) |
| 6F91 | 34 | PARZ | `jp` da 6F4B/5E/6E/81: 6FB3 se bit 1 di C, altrimenti 6FD1 | 08 11 … |
| 6FB3 6FD1 | 16 14 | PARZ | `call` da 6F91; il ramo `set 7,c` non è mai preso | 08 11 … |
| 6FC3 | 14 | NO | `call` a 6FAA: bit 1 di C = 0 e 6FD1 ha messo il bit 7 ((E+8) & 0F ≠ B & 0F) | nessuna |
| 6FDF | 14 | NO | `call` a 6F9C: bit 1 di C = 1 e 6FB3 ha messo il bit 7 (D & 0F ≠ B >> 4) | nessuna |
| 6FED 6FF7 | 10 10 | SI/PARZ | 6F1C[3] [12] e [6] [9] | 08 11 … |
| 7001 700B | 10 10 | SI | 6F1C[5] e [10]; `jr` a 7015/7036 e 7025/7045 | 11 27 e 22 24 27 33 |
| 7015 7025 7036 7045 | 16 17 15 17 | SI | 6F1C[7] [11] [13] [14] e dai `jr` | 08 11 02 22 … |
| 70A8 70B0 70CE | 8 30 32 | SI | `call` | 08 11 … |
| 70FE | 77 | SI | `call` da 6ECA (36F0) | 08 11 … |

Dipendenze esterne di N2 non tradotte (da `out/censimento_nemici.txt`):
571D (9 B, 6D03[0]; se (IX+0D) ≥ 0x40 salta a 80A5), 80A5 (14 B: azzera
IX+0, IX+1, IY+0, IY+1), 57AA (9 B: HL = (IX+0B..0C) − BC − riporto),
57B3 (26 B), 6EB7 (10 B, cade in 6EC1) e 6EC1 (9 B). Le prime quattro sono
in N5/N4, le ultime due in N3. 57AA viene eseguita a ogni ingresso in 6CE8,
quindi con uno stub nessuna cattura di 6CE8 può passare.

Particolarità del codice (da `out/main64.dasm`):
- 6D03: tabella `rst38_6D03` = 571D 6D1B 6D37 6D12 6DCE 6E55 6EA1.
- 6F1C: `rst 20` con `push 7056` prima. Le voci vere sono le prime **16**
  di `rst20_6F1C` (l'indice A viene da 70FE con `and 0Fh`); le altre 6 sono
  la tabella dati 6F3D (009A 00E6 0133 ×2). Si passa `voci = 16` a
  `nemici_rst20`. Alla destinazione HL = 7056 e in pila c'è 7056: la coda
  7056-70A7 la esegue il C di 6ECA dopo la destinazione.
- 70EE: 16 byte dati (00 4C 44 40 C4 00 44 CC 4C C0 CC C4 …), da leggere
  dalla ROM con `M(0x70EE + a)`.
- Le destinazioni di `rst 38`/`rst 20` e 6F91 hanno in cima alla pila
  l'indirizzo di ritorno del chiamante (4EFB, cioè il ritorno della `call`
  a 4F0F, oppure 7056). Per questo `snap.lua` le cattura come routine
  normali: ritorno = parola in cima alla pila all'ingresso.

Misure di oggi (2026-10-28, binari già compilati in `decomp/harness`):
- `./harness --tutte`: exit 0, 220 righe `^cattura`, 0 `FALLITO`, 112 `RIEPILOGO`.
- `harness/sched_proto out/sched_log.txt | tail -1` (da `decomp`):
  `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`.
- `harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410`:
  exit 0; ultime quattro righe:
  ```
  spedizioni eseguite: 64686, fino al frame 5499
  NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
  prima divergenza a metà attività (attesa): frame 2415
  nessuna divergenza nello spedidore
  ```
- Nessun file `.c/.h/.lua/.py/.sh` di `decomp` (escluso `out/`) supera le 800 righe.

Convenzioni: `ROUTINE.md` marca `verificata` una routine con almeno 2
catture OK e `tradotta` una con meno di 2. Le catture si versionano
(`prima.bin`, `dopo.bin`, `regs.txt`, `.log`). Rigenerare una cattura
esistente deve dare `.bin` e `regs` identici bit a bit; i `.log` cambiano e
vanno ripristinati.

## Struttura da realizzare

**1. Manutenzione degli script** (prima di ogni cattura nuova):
- `cattura.sh` r. 58-59 e `sessioni.sh` `MOPT` r. 14: aggiungere
  `-keyboardprovider none -mouseprovider none -joystickprovider none -lightgunprovider none`
  (come `replay.sh` r. 22-23). Aggiornare il commento di testa.
- `snap.lua`: aggiungere `SNAP_ROOM` (stanza 0-based). Si copia il
  meccanismo di `sessione.lua` r. 133-136: tap in lettura su c243 dal frame
  1500 al 1950, installato e tolto dentro `register_frame_done`.
- `snap.lua`: aggiungere `SNAP_FORZA`, una lista separata da virgole
  applicata **nell'istante in cui si accetta l'occorrenza voluta, prima di
  `registri("prima")` e del dump**. Voci ammesse: `ix+HH:VV` e `iy+HH:VV`
  (byte in RAM a IX/IY + HH esadecimale) e `r:VV` con r in a, b, c, d, e,
  h, l (registro a 8 bit, `cpu.state["E"].value = VV`). Ogni forzatura si
  stampa nel log (`FORZA ...`). Le catture forzate restano originali,
  perché MAME esegue il codice vero a partire dallo stato forzato.
- `cattura.sh`: stato nuovo `stanzaNN` (es. `stanza10`) con
  `SNAP_ROOM=NN`, `INGRESSI=traccia`, `FRAMES="1700 1900 2100"`, senza
  poke né poste. Lo stato `bonus` passa a `SNAP_ROOM=4` più i poke
  `2250:0xc19d:0,2250:0xc19e:0,2250:0xc24a:0` (si toglie `c24b=0x85`),
  come `bonus_04` di `sessioni.sh`. Non esistono catture `bonus` da
  rigenerare: in `out/` ci sono solo `snap_0x3FCE_porta_*` e
  `snap_0x51FA_finale_*`.

**2. Banco con i moduli dei nemici.** Nel Makefile si aggiunge al bersaglio
`harness` `$(wildcard nemici*.c) attivita.c attivita_01_10.c sched.c` (con
`nemici.h rst38_tables.h sched.h` nelle dipendenze). `nemici_tipo09.c` usa
`sub_4b98` di `attivita.c`, e `sched.c` usa `corpo_attivita`, che è
definita solo in `replay.c` e `sched_proto.c`. Se il collegamento la chiede,
si definisce in `harness.c` come `attivita_fn corpo_attivita[SCHED_CODICI];`
(stessa riga di `replay.c:29`). Questo elenco è dedotto, non provato: va
ridotto o allargato finché `make harness` collega senza errori. Il banco
storico deve restare 220/0.

**3. C del lotto.**
- Nuovo `harness/nemici_tipi_08_14.c` con le 33 routine N2. Sono
  **non statiche** tutte quelle registrate nel banco (elenco al punto 4).
  Gli ingressi interni senza cattura propria (6E83, 6E8B) possono restare
  etichette dentro `sub_6e55`. La coda 7056-70A7 è una `static` chiamata
  da `sub_6eca` dopo lo smistamento di 6F1C. Le tabelle stanno in
  `rst38_6D03` e `rst20_6F1C` (16 voci). Le PARZIALE si traducono intere.
- Dipendenze, ognuna nel modulo prescritto dal censimento e non statica:
  `sub_571d`, `sub_57aa`, `sub_57b3` in `nemici_tipi_10_13.c`, `sub_80a5`
  in `nemici_comparsa.c`, `sub_6eb7` e `sub_6ec1` in `nemici_tipo09.c`.
- `nemici.h`: prototipi nuovi in una sezione «Lotto N2». Si tolgono
  `sub_6ce8` e `sub_6eca` dagli stub, e le righe 25-26 da `nemici_stub.c`.
- `routine_tab.c`: le voci N2 e le 6 dipendenze, in ordine di indirizzo,
  con `#include "nemici.h"`.

**4. Catture.** Da `decomp/`, `sh cattura.sh <ENTRY> <stato> [N] [SUFFISSO]`.
L'obiettivo è almeno 2 catture OK per voce (stato `verificata`). Dove
basta una stanza, conviene provare prima le stanze della tabella sopra.

| voce banco | catture proposte |
|---|---|
| 0x6CE8, 0x6D12, 0x6D1B, 0x6E55, 0x6E90 | `stanza10`, `stanza14` (N=1; se manca, N=2 o `stanza24`) |
| 0x6E55 (6E83) | `SNAP_FORZA=ix+5:0,ix+4:0 sh cattura.sh 0x6E55 stanza10 1 forza` |
| 0x6D37, 0x6DCE | `stanza42` N=1 e N=2 (o `finale`) |
| 0x6EA1 | `stanza10`, `stanza02` |
| 0x571D, 0x57AA, 0x57B3, 0x80A5, 0x6EB7, 0x6EC1 | stanze del tipo 14 (`stanza10`, `stanza14`); 571D con (IY+1D) = 0 dopo 57B3; per 80A5 provare `SNAP_FORZA=ix+d:0x40` su 0x571D |
| 0x6ECA, 0x70FE, 0x70A8, 0x70B0, 0x70CE | `stanza08`, `stanza11` |
| 0x6F49 … 0x7045 (le 12 destinazioni di 6F1C), 0x6F91, 0x6FB3, 0x6FD1 | la stanza con più sessioni nella tabella sopra (`stanza08`, `stanza11`, `stanza22`, `stanza27`), N=1 e N=2 |
| 0x6F91 (6FC3) | `SNAP_COND='(c&2)==0' SNAP_FORZA=e:0x89 sh cattura.sh 0x6F91 stanza08 1 forza6fc3` |
| 0x6F91 (6FDF) | `SNAP_COND='(c&2)==2' SNAP_FORZA=d:0x81 sh cattura.sh 0x6F91 stanza08 1 forza6fdf` |

Il ramo 6FC3 si apre perché (0x89+8) & 0F = 1, che non è mai 0, 4 o C,
gli unici valori di B & 0F in 70EE. Il ramo 6FDF si apre perché 0x81 & 0F
= 1 ≠ B >> 4. Se una forzatura non apre il ramo, lo si dice nel rapporto e
in `ROUTINE.md`: in quel caso la strada è un breakpoint su 6FAA/6F9C in una
sessione headless con `SNAP_COND`, senza ricorrere a partite a mano. Se una
cattura nella stanza non arriva entro `SNAP_LIMITE` (6000 frame), si prova
un'altra stanza della tabella. Ogni cattura MAME dura da qualche decina di
secondi a un minuto.

**5. Copertura del C.** Un build gcov del banco, fuori dal repository,
dice quali righe di `nemici_tipi_08_14.c` le catture hanno eseguito:
```
D=/tmp/cov27 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
 && ln -s "$PWD/decomp/out" $D/out && cd $D && gcc -std=c11 -O0 --coverage -o h <sorgenti di harness> \
 && ./h --tutte > /dev/null; gcov -n nemici_tipi_08_14.c | tail -2
```
(`<sorgenti di harness>` è l'elenco del bersaglio `harness` del Makefile.)
Nel rapporto vanno le righe non eseguite e il motivo per ciascuna.

**6. Documenti.** In `ROUTINE.md`, una sezione «Motore dei nemici (lotto
N2)» con le colonne della tabella principale (ingresso, funzione, file,
byte, descrizione, stato, catture OK, stati, note), più le catture forzate
segnate come tali. Si aggiorna la sezione N1: i moduli `nemici*.c` ora
entrano anche nel banco. In `CENSIMENTO-NEMICI.md`, sotto «Piano dei
lotti», si segna N2 fatto, con le 6 dipendenze anticipate da N3/N4/N5. In
`STATO.md` si aggiorna il punto 0 e si toglie la «Trappola scoperta» come
lavoro in sospeso, dato che ora è sistemata.

## Verifiche obbligatorie

Tutte da `/home/rufio72/OpenSolomonsKey-master`.

1. `grep -c 'provider none' decomp/cattura.sh decomp/sessioni.sh`: ≥ 1 per
   file, e ognuna delle due righe MAME ha i quattro provider (attesa, non
   misurata).
2. Rigenerazione della cattura pilota con i provider spenti:
   `cd decomp && sh cattura.sh 0x3FCE porta 1` stampa `CATTURA 0x3FCE porta frame_da=2300 OK`
   (attesa, non misurata). Poi
   `git status --short decomp/out/snap_0x3FCE_porta_prima.bin decomp/out/snap_0x3FCE_porta_dopo.bin decomp/out/snap_0x3FCE_porta_regs.txt`
   esce vuoto, e `git checkout -- decomp/out/snap_0x3FCE_porta.log`.
3. `cd decomp/harness && make harness replay sched_proto`: nessun errore e
   nessun warning nei file nuovi o toccati.
4. `cd decomp/harness && ./harness --tutte; echo "exit $?"`: `exit 0`,
   `grep -c FALLITO` = 0, `grep -c '^cattura'` = 220 + numero delle catture
   nuove (attesa: circa 70), e una riga `RIEPILOGO 0x<ingresso>: n catture, n OK`
   per ogni voce nuova (oggi le righe `RIEPILOGO` sono 112).
5. `cd decomp/harness && ./harness 0x6CE8 && ./harness 0x6ECA`: exit 0 per
   ciascuna, con almeno 2 catture.
6. `cd decomp && harness/sched_proto out/sched_log.txt | tail -1` →
   `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`
   (misurato).
7. `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 | tail -4; echo "exit $?"`
   → le quattro righe misurate sopra, identiche, ed `exit 0`.
8. `grep -c STUB_NEMICI decomp/harness/nemici_stub.c` → 26 (oggi 28
   occorrenze, cioè la `#define` più 27 righe; ne restano 25 + 1),
   senza `6ce8` né `6eca`.
9. Copertura gcov (punto 5 della struttura): almeno il 90% delle righe di
   `nemici_tipi_08_14.c` eseguite, e ognuna delle 33 routine con almeno una
   riga eseguita (attesa, non misurata). Le eccezioni vanno motivate nel
   rapporto.
10. `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/nemici_tipi_08_14.c` < 700.
11. `git status --short` mostra solo i file di questo compito: script,
    `harness/*.c|h|Makefile`, `ROUTINE.md`, `CENSIMENTO-NEMICI.md`,
    `STATO.md`, le catture nuove `decomp/out/snap_0x....` e il brief.
    Nessun binario compilato (`harness`, `replay`, `sched_proto`) e nessun
    `.gcda`/`.gcno` nel repository.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto N2 del motore dei nemici: tipi 8 e 14 con catture forzate, provider MAME spenti`
  e riga finale `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Rapporto finale di massimo 30 righe: uscita delle verifiche 4-10;
  l'elenco delle voci con il numero di catture e lo stato
  (`verificata`/`tradotta`); le forzature che hanno aperto o non hanno
  aperto il ramo voluto; le righe gcov non coperte; le deviazioni dal brief
  (elenco dei sorgenti del banco, stanze cambiate).

## Domande aperte

1. **Le 6 dipendenze di altri lotti** (571D, 57AA, 57B3, 80A5 da N5/N4;
   6EB7, 6EC1 da N3), 77 B in tutto. Proposta: tradurle in N2, ognuna nel
   modulo prescritto dal censimento, perché senza 57AA nessuna cattura di
   6CE8 passa. N3, N4 e N5 le troveranno fatte. — «preferenza»
2. **Moduli dei nemici nel banco**: oggi `nemici*.c` entrano solo in
   `replay`, e `ROUTINE.md` dice che si verificano col giro. Proposta:
   collegarli anche a `harness`, con `attivita*.c` e `sched.c`, così la
   regola vale per tutti i lotti N2-N12. Cambia il perimetro del bersaglio
   del banco. — «scelta di progetto»
3. **Catture da stato forzato come prova di `verificata`** (6E83 via RAM,
   6FC3/6FDF via registri E/D: rami mai visti in 68 sessioni, forse codice
   difensivo). Proposta: valgono come catture OK, con tag `forza*` e nota
   «stato forzato» in `ROUTINE.md`. Non contano per lo stato
   `verificata` se sono le uniche di quella voce. — «scelta di progetto»
4. **Destinazioni di 6F1C e 6F91 come funzioni esportate e voci del banco**
   (invece di un solo `sub_6eca` con lo smistamento dentro). Proposta: sì,
   perché ogni ramo diventa catturabile da solo. — «preferenza»
5. **Frame di partenza dello stato `stanzaNN`**: proposta `1700 1900 2100`,
   da cambiare se le catture non arrivano. — «preferenza»
6. **Stato `bonus` corretto a `SNAP_ROOM=4` senza `c24b=0x85`**: nessuna
   cattura esistente dipende da esso. — «preferenza»

## Risoluzioni (2026-10-28)

- Domanda 1: ACCETTATA la proposta — le 6 dipendenze (571D, 57AA, 57B3,
  80A5, 6EB7, 6EC1) si traducono in N2, ognuna nel modulo prescritto dal
  censimento.
- Domanda 2 (scelta di progetto, decisa dall'utente): SÌ — `nemici*.c`,
  `attivita*.c` e `sched.c` si collegano anche al banco `harness`; la
  regola «cattura nel banco» vale per tutti i lotti N2-N12.
- Domanda 3 (scelta di progetto, decisa dall'utente): ACCETTATA la
  proposta — le catture da stato forzato valgono come catture OK (tag
  `forza*`, nota «stato forzato» in ROUTINE.md) ma da sole non bastano per
  lo stato `verificata`.
- Domanda 4: ACCETTATA la proposta — destinazioni di 6F1C e 6F91 esportate
  e voci proprie del banco.
- Domanda 5: ACCETTATA la proposta — frame di partenza 1700/1900/2100.
- Domanda 6: ACCETTATA la proposta — stato `bonus` a `SNAP_ROOM=4` senza
  `c24b=0x85`.

Resta valida la decisione dell'utente dal brief 25: se il replay diverge
si corregge tutto, annotando in REPLAY.md.

## Esito (2026-10-28)

PASSATO. Sviluppatore: 11/11; verificatore: tutte passate (la 3 senza
ricompilare perché i binari erano aggiornati; la 9, gcov, non rieseguita —
misurata dallo sviluppatore al 96,08% con nessuna routine a zero; la 11
vuota perché già committato, 764699e). Le catture nuove sono 101 invece
delle ~70 attese: copertura in più, non un difetto.

Risultato: 33 routine dei tipi 8 e 14 in nemici_tipi_08_14.c (697 righe)
più 6 dipendenze nei moduli prescritti; stub a 26; banco a 321 catture e
147 voci, 35 nuove tutte «verificata» con almeno 2 catture non forzate;
replay 2410-5499 e sched_proto identici. Manutenzione chiusa: provider a
none in cattura.sh e sessioni.sh, stato bonus a SNAP_ROOM=4, nuovi
SNAP_ROOM/SNAP_FORZA in snap.lua, stato stanzaNN (frame 1951/2100/2300,
non 1700+: sotto il 1951 snap.lua non arma il breakpoint — deviazione
documentata). Altre deviazioni sane: sched.c non serve nel banco; sub_4f0f
spegne il riporto dopo rst 38 (letto da 57AA), replay invariato; quattro
forzature aggiunte per rami mai visti.

Token: analista Opus ~134k, sviluppatore Opus ~218k (84 chiamate, ~1 h con
le catture MAME), verificatore Sonnet ~16k. Nulla su OpenRouter.
