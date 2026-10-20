# Compito 06: lotto 2 di traduzione in C delle routine del Z80 principale

Data: 2026-10-20. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Tradurre in C 15 nuove routine del Z80 principale di Solomon's Key (set MAME
`solomon`), verificarle con il banco di prova `decomp/harness/` su due catture
MAME ciascuna, portare a `verificata` le tre routine rimaste a una cattura
(05B3, 2402, 326C) e coprire il ramo «decine» di 3A1C. Schema, convenzioni e
strumenti sono quelli del lotto 1: `compiti/05-lotto-01-routine.md` (sezione
«Schema dei lotti»), già realizzati in `decomp/`. Si lavora solo in `decomp/`.

Vincoli assoluti:
- MAME solo con `-video none -sound none`, mai `-window` né `-video soft`; le
  corse MAME in sottofondo (`&` o `run_in_background`), al massimo 4 in
  parallelo, file diversi per ENTRY e stato. `cattura.sh` già lo rispetta.
- Nessuna operazione git (add, commit, checkout, stash, reset): commit della
  chat principale.
- Niente modifiche fuori da `decomp/`. Non toccare `sessioni.sh`, `trace.lua`,
  `sessione.lua`, `sched*.c/h`, `fondale.c`, `routine.c`, `porte.c` e le
  traduzioni del lotto 1 (salvo l'aggiunta di `sub_0018` in `z80_0xxx.c`).
- Non rigenerare tracce complete né `callgraph.csv`. File sopra 50 MB creati
  durante il lavoro vanno cancellati.
- Nessun file di codice sopra le 800 righe; commenti, messaggi e documenti in
  italiano. Compilatore `gcc` nel PATH; MAME `/usr/games/mame` 0.264.
- Nomi C `sub_xxxx` (esadecimale minuscolo), descrizione in `ROUTINE.md`.

## Cosa esiste già (fatti verificati)

Da lettura diretta del repository (commit 8e0b438):

| file | righe | ruolo |
|---|---:|---|
| `decomp/harness/harness.c` | 214 | banco: per ENTRY prova ogni cattura (senza tag = `base`, con tag via glob `snap_<ENTRY>_*_regs.txt`), `--tutte` per tutta la tabella |
| `decomp/harness/routine.h` | | `struct voce_routine`, prototipi `sub_xxxx` (lotto 1 e precedenti) |
| `decomp/harness/routine_tab.c` | 27 | tabella `tabella_routine[]` in ordine di indirizzo, 18 voci; riga tipo `{ "0x3F10", sub_3f10, 0xc061, 0xc067 },` |
| `decomp/harness/z80_0xxx.c` | 44 | `sub_0008`, `sub_0010`, `sub_026c` |
| `decomp/harness/z80_2xxx.c` | 78 | `sub_237e`, `sub_23a1`, `sub_2418`, `sub_249e`, `sub_24b1`; testa del file e stile: commento di testa con ingresso/uscita |
| `decomp/harness/z80_3xxx.c` | 63 | `sub_30ef`, `sub_3a1c`, `sub_3f10` |
| `decomp/harness/z80_4xxx.c` | 30 | `sub_4af6`, `sub_4b0e` |
| `decomp/harness/z80_6xxx.c` | 18 | `sub_673f` |
| `decomp/harness/Makefile` | | `SORGENTI = harness.c routine.c fondale.c porte.c routine_tab.c $(wildcard z80_*.c)`: un nuovo `z80_7xxx.c` è raccolto da solo |
| `decomp/cattura.sh` | 36 | `sh cattura.sh <ENTRY> <stato> [N]`, stati `titolo` (frame 1000, 650, 0), `gioco` (2400, 2000, 1700, 1600, 1500), `tardo` (4000, 3000); scrive `out/snap_<ENTRY>_<stato>_*` |
| `decomp/snap.lua` | | variabili `SNAP_ENTRY`, `SNAP_N`, `SNAP_DIR`, `SNAP_DA_FRAME`, `SNAP_TAG`, `SNAP_INGRESSI`, `SNAP_LIMITE`; funzione `arma_ingresso()` fa `bpset <entry>` |
| `decomp/ROUTINE.md` | 28 | 18 righe `| 0x… |`; 15 `verificata`, 3 `tradotta` (05B3, 2402, 326C, cattura `base`) |
| `decomp/out/main48.dasm` | 35730 | disassembly; indirizzo nel commento finale `;3a2e` (minuscolo) |
| `decomp/out/main64.bin` | 65536 | ROM a indirizzi reali (il banco la carica in `rom[]`, `leggi(a)` la legge) |

Firma unica `void sub_xxxx(struct z80 *z)`. Accessi: `M(a)` scrittura/lettura RAM
c000-ffff, `leggi(a)` lettura su tutto lo spazio, `HL(z)/SET_HL(z,v)`,
`BC/DE` analoghi, registri `z->a z->b …`, riporto in ingresso `z->f & 1`.
Le bandiere in uscita non si confrontano; la pila sotto lo SP d'ingresso
non si confronta. Una routine che ne chiama un'altra la chiama in C
(`sub_0010(z);`) con gli stessi `z`.

Uscite attuali, misurate il 2026-10-20 (da `decomp/harness`):
```
./harness --tutte | grep -c '^RIEPILOGO'   → 18
./harness --tutte | grep -c FALLITO        → 0
./harness 0x05B3 | tail -2                 → OK 0x05B3 base / RIEPILOGO 0x05B3: 1 catture, 1 OK
./harness 0x2402 | tail -1                 → RIEPILOGO 0x2402: 1 catture, 1 OK
./harness 0x326C | tail -1                 → RIEPILOGO 0x326C: 1 catture, 1 OK
./sched_proto ../out/sched_log.txt | tail -1 →
  spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131
ls out/snap_0x*_*_regs.txt | wc -l (da decomp) → 30
grep -c '| verificata |' ROUTINE.md        → 15
```
Le tre catture `base` (prima.bin 16384 byte, `_regs.txt`): 05B3 con
`prima PC=05B3 AF=2008 BC=0000 DE=0030 HL=C8EE`; 2402 con `BC=0001 DE=0000
HL=E400`; 326C con `BC=042C DE=F7A3 HL=F13F`. Le nuove catture tagged devono
essere diverse da queste.

## Routine del lotto 2

Selezione secondo lo schema (sezione 6 del brief 05): `brutta` vuota in
`callgraph.csv`, nessuna porta (controllato su `main48.dasm`: nessun
`in`/`out`/`ex (sp),hl`/`ldir` nelle estensioni sotto), tutte le destinazioni
interne già tradotte (`rst 08`/`rst 10` → `sub_0008`/`sub_0010`; 4E6A →
`sub_24b1`). Conteggi per sessione da `out/<sessione>_chiamate.txt` (t =
titolo, s = stanza_00, g = gameover). Estensione letta in `main48.dasm`
(il CSV non è affidabile sulle lunghezze). Stato di cattura scelto in base
ai conteggi: `titolo` se t>0, altrimenti `tardo` (g>0) al posto di `titolo`.

| ingresso | estensione | byte | t / s / g | siti | effetto (da dasm) | catture |
|---|---|---:|---|---|---|---|
| 0018 | 0018-001A | 3 | 35629 / 19174 / 18638 | 0023, 2B11, 323B, 33C5, 485E, 487C | `rst 10; ex de,hl; ret`: come `rst 10` ma DE e HL scambiati; chiama `sub_0010` | titolo, gioco |
| 2368 | 2368-2375 | 14 | 2 / 2 / 0 | 2EA5 | HL = 6·tabella(HL+A): `l=(HL+A)`, ×2, ×6; solo registri | titolo, gioco |
| 23AC | 23AC-23BA | 15 | 7 / 7 / 4 | 3218 | azzera (HL) e un byte a D:(E+…); `inc de` | titolo, gioco |
| 23F8 | 23F8-2401 | 10 | 4 / 5 / 4 | 356D | `rst 08` su HL, poi DE=parola, C=byte, B=0 | titolo, gioco |
| 240B | 240B-2417 | 13 | 1 / 1 / 1 | 37DA | HL = 24·A (via ×3 e ×8); solo registri | titolo, gioco |
| 2457 | 2457-2473 | 29 | 2 / 4 / 2 | 2F4C, 2F93 | D, E da E (colonna/riga), solo registri | titolo, gioco |
| 34D4 | 34D4-34E0 | 13 | 4 / 6 / 2 | 34A2, 34A8 | B=0A byte C=00 a HL e a HL con bit 10 azzerato | titolo, gioco |
| 380A | 380A-3811 | 8 | 2 / 2 / 2 | 37E7, 37ED | riempie 16 byte da HL con il valore FF (BC=10FF) | titolo, gioco |
| 3A2E | 3A2E-3A53 | 38 | 41 / 41 / 0 | 3844 | indice A in tabelle ROM 8578/858E con `rst 08`; B,C = coppia di byte | titolo, gioco |
| 4722 | 4722-472D | 12 | 4 / 4 / 0 | 37CA | C = 7 + numero di elementi della tabella ROM 9484 minori o uguali ad A | titolo, gioco |
| 4DEE | 4DEE-4E30 | 67 | 0 / 534 / 547 | 4DE5 | usa `rst 08`, `rst 10`; cicla sui 17 slot a c2c1 (passo 0x18); `ret z` iniziale | gioco, tardo |
| 4E31 | 4E31-4E86 | 86 | 0 / 535 / 547 | 4DDD | cicla sui slot c2c1 / c4fe; chiama `sub_24b1` con il riporto di `sub d` (4E65) | gioco, tardo |
| 4E87 | 4E87-4EE5 | 95 | 0 / 535 / 547 | 4DDA | usa `rst 10` due volte, `rr e`, c459; tre `ret` condizionali | gioco, tardo |
| 6771 | 6771-677B | 11 | 11 / 209 / 5 | 31D4 e altri 17 (7 chiamanti) | copia byte da (DE) fino a FF in due piani (HL e HL con bit 10 azzerato); HL e DE in uscita | titolo, gioco |
| 7EFE | 7EFE-7F2C | 47 | 231 / 231 / 0 | 7EE1, 7EF2 | calcola indirizzo in d486/d496 da D,E,C e scrive A e C in due piani | titolo, gioco |

Le colonne «effetto» sono sintesi dall'analista: la traduzione si fa sul
disassembly. In particolare:
- In 4E31 il riporto passato a `sub_24b1` è quello di `sub d` a 4E65
  (`set 1,c` non lo modifica): impostare `z->f = (z->f & ~1) | prestito` prima
  della chiamata. In 4E87 `rl e` / `rr e` usano il riporto dell'ultimo `add a,a`
  / `rr e` precedente: tradurlo esplicito.
- 4DEE, 4E31, 4E87 sono tre routine consecutive (4DEE-4E30, 4E31-4E86,
  4E87-4EE5) chiamate da 4D87: ognuna termina al proprio `ret`; il codice a
  4EE6 non fa parte di 4E87.
- 7EFE e 6771 scrivono in due piani (`res 2,h` / `set 2,h`): sono scritture
  RAM normali sotto e5ff; la cattura è non banale se `prima.bin` e
  `dopo.bin` differiscono in c000-e5ff.
- Se in una routine si trova un'istruzione non coperta dal dasm di cui sopra
  (porta, `ex (sp),hl`), fermarsi su quella routine, non tradurla e annotarla
  in `ROUTINE.md` (stato `scartata`) invece di indovinare.

Scartate, con ragione: 234A (t=2, s=0, g=0, entra a metà routine), 23BB /
23EC (già callee di `sub_326c` in `fondale.c`), 7379 (`jp 2540` fuori
estensione), 03FB (chiama 0469 non tradotta), 34E1 / 5507 / 36F0 (callee non
verificate), 4D87 (cinque callee).

## Struttura da realizzare

1. Funzioni, file di destinazione (si aggiunge in fondo, stile del lotto 1:
   commento di testa con ingresso, uscita, significato; `#define` per i campi
   RAM; nessuna emulazione di bandiere salvo il riporto dove serve):
   - `z80_0xxx.c`: `sub_0018`
   - `z80_2xxx.c`: `sub_2368`, `sub_23ac`, `sub_23f8`, `sub_240b`, `sub_2457`
   - `z80_3xxx.c`: `sub_34d4`, `sub_380a`, `sub_3a2e`
   - `z80_4xxx.c`: `sub_4722`, `sub_4dee`, `sub_4e31`, `sub_4e87`
   - `z80_6xxx.c`: `sub_6771`
   - `z80_7xxx.c` (nuovo): `sub_7efe`
   Se un file supera 600 righe lo si spezza come da schema del brief 05.
2. `routine.h`: prototipi del lotto 2 (blocco «Lotto 2 (compito 06)»).
3. `routine_tab.c`: 15 voci in ordine di indirizzo; `blocco_ini/blocco_fine`
   = 0, 0 per tutte (la RAM c000-e5ff si confronta comunque).
4. `snap.lua`: nuova variabile `SNAP_COND` (condizione di breakpoint del
   debugger, vuota = nessuna). In `arma_ingresso()`: se non vuota,
   `bpset <entry>,<cond>`. Documentarla nel commento di testa. Per il ramo
   «decine» di 3A1C la condizione è `a>=#10` (decimale con `#`; se MAME non
   la accetta provare `a>=0a`).
5. `cattura.sh`: quarto argomento facoltativo SUFFISSO; se presente il tag è
   `<stato>_<SUFFISSO>` (file `out/snap_<ENTRY>_<stato>_<SUFFISSO>_*`, glob
   del banco già compatibile). Gli altri argomenti e il comportamento senza
   quarto argomento restano invariati. `SNAP_COND` si passa dall'ambiente.
6. Catture (tutte con `sh cattura.sh`, da `decomp/`):
   - le 15 nuove routine: due stati ciascuna, come nell'ultima colonna;
   - seconde catture: `05B3 gioco`, `326C gioco`, `2402 titolo` (con N=2 o
     3 se la cattura coincide con la `base`, controllo con `cmp -s`);
   - ramo decine: `SNAP_COND='a>=#10' sh cattura.sh 0x3A1C gioco 1 decine`;
     se fallisce provare `titolo`, poi `tardo`. Se in nessuno stato esiste
     una occorrenza con A ≥ 10, annotare in `ROUTINE.md` «ramo decine non
     catturato: A≥10 mai osservato entro 6000 frame» e proseguire (non
     bloccante).
7. Regole di qualità (brief 05, sezione 1): i due `prima.bin` della stessa
   routine devono differire; per le «solo registri» (0018, 2368, 240B, 2457)
   devono differire le righe `prima` di `_regs.txt`; per le routine che
   scrivono RAM almeno una cattura con `prima`/`dopo` diversi in c000-e5ff
   (`cmp -n 9728`), altrimenti `SNAP_N` 2..5 e in caso estremo nota
   «ramo principale non catturato» e stato `tradotta`.
8. `ROUTINE.md`: 15 nuove righe in ordine di indirizzo (tra le esistenti,
   formato attuale), 05B3 / 2402 / 326C aggiornate (2 catture, stati
   `base, gioco` ecc., stato `verificata`, tolta la nota «seconda cattura
   nel lotto 2»); 3A1C: nel campo note aggiungere l'esito della cattura
   decine. Aggiornare la riga di testa se serve (nessun cambio di schema).

## Verifiche obbligatorie

Da `decomp/` salvo dove indicato. «Attesa, non misurata» = dedotta: le
uscite dipendono dal codice che si scrive.

1. `make -C harness 2>&1 | grep -ci -E 'warning|error'` → `0`; esistono
   `harness/harness` e `harness/sched_proto` (attesa, non misurata).
2. Per ognuna delle 15 routine: `(cd harness && ./harness 0xXXXX | tail -1)` →
   `RIEPILOGO 0xXXXX: 2 catture, 2 OK`, uscita 0 (attesa, non misurata).
3. Le tre routine rimaste a una cattura:
   `for e in 05B3 2402 326C; do (cd harness && ./harness 0x$e | tail -1); done`
   → `RIEPILOGO 0x05B3: 2 catture, 2 OK` e idem per 2402 e 326C (attesa, non
   misurata); le righe `byte C8EE-C94D coincidenti: 96 su 96` per 05B3 e
   `byte D880-DFFF coincidenti: 1920 su 1920` per 326C restano per la
   cattura `base` (misurato oggi) e compaiono anche per la nuova.
4. 3A1C: `(cd harness && ./harness 0x3A1C | tail -1)` → `RIEPILOGO 0x3A1C: 3
   catture, 3 OK` se la cattura decine riesce, `2 catture, 2 OK` altrimenti
   (attesa, non misurata). Nel primo caso il test deve davvero percorrere il
   ramo: `grep prima out/snap_0x3A1C_*decine_regs.txt` mostra `AF=` con byte
   alto ≥ 0x0A.
5. `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `33`;
   `... | grep -c FALLITO` → `0`; uscita 0 (attesa, non misurata).
6. `(cd harness && ./sched_proto ../out/sched_log.txt | tail -1)` →
   `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
7. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale"'`
   → vuoto (misurato oggi: vuoto).
8. `grep -c '| verificata |' ROUTINE.md` → `33`; `grep -c '^| 0x' ROUTINE.md`
   → `33` (attesa, non misurata; meno di 33 `verificata` solo per routine
   rimaste `tradotta` o `scartata`, da spiegare nel rapporto).
9. `ls out/snap_0x*_*_regs.txt | wc -l` → `64` (30 attuali + 30 nuove + 3
   seconde catture + 1 decine; 63 senza decine) (attesa, non misurata).
10. I due `prima.bin` differiscono:
    `for e in 0018 2368 23AC 23F8 240B 2457 34D4 380A 3A2E 4722 4DEE 4E31 4E87 6771 7EFE; do set -- out/snap_0x${e}_*_prima.bin; cmp -s "$1" "$2" && echo "UGUALI $e"; done`
    → vuoto; per le tre seconde catture:
    `for e in 05B3 2402 326C; do for f in out/snap_0x${e}_*_prima.bin; do cmp -s out/snap_0x${e}_prima.bin "$f" && echo "UGUALI $e"; done; done`
    → vuoto (attese, non misurate).
11. `grep -n -e '-window' -e 'video soft' cattura.sh snap.lua` → vuoto
    (misurato oggi: vuoto).
12. `find .. -size +50M` → vuoto (misurato oggi: vuoto).
13. `git status --short` (sola lettura) → solo `M decomp/ROUTINE.md`,
    `M decomp/cattura.sh`, `M decomp/snap.lua`, `M decomp/harness/routine.h`,
    `M decomp/harness/routine_tab.c`, `M decomp/harness/z80_0xxx.c`, `z80_2xxx.c`,
    `z80_3xxx.c`, `z80_4xxx.c`, `z80_6xxx.c`, i nuovi
    `decomp/harness/z80_7xxx.c`, `decomp/out/snap_*` (file non tracciati),
    `compiti/06-lotto-02-routine.md`. Nessun processo `mame` residuo:
    `pgrep -c mame` → `0`.

## Consegna

File sopra elencati, nessun commit (lo fa la chat principale con la riga
`Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`). Rapporto finale
di massimo trenta righe: uscita delle verifiche 1-13, per ogni routine gli
stati e i frame di cattura usati, routine rimaste `tradotta` o `scartata` e
perché, esito del ramo decine di 3A1C, deviazioni dal brief.

## Domande aperte

1. Il ramo decine di 3A1C si cattura con una condizione di breakpoint
   (`SNAP_COND`) e un suffisso nel tag, anziché cercarlo con `SNAP_N` alti.
   Proposta: come descritto, perché `SNAP_N` non garantisce A ≥ 10.
   «preferenza»
2. Alcune routine hanno 1-2 invocazioni per sessione (240B, 2368, 2457,
   380A): la cattura potrebbe non riuscire in tutti i frame di partenza.
   Proposta: se uno stato fallisce, provare l'altro stato disponibile
   (`titolo`/`gioco`/`tardo`) e annotare lo stato usato; se resta una sola
   cattura la routine resta `tradotta` con nota. «preferenza»
3. `234A` (entra a metà di una routine, t=2) è esclusa; si può riprendere in
   un lotto successivo. Proposta: lasciarla fuori. «preferenza»

## Decisioni (chat principale, 2026-10-20)

- Domanda 1 (preferenza): accettata, ramo decine di 3A1C con `SNAP_COND='a>=#10'` e suffisso del tag; se non compare mai, nota in ROUTINE.md senza bloccare.
- Domanda 2 (preferenza): accettata, per 240B, 2368, 2457, 380A si prova l'altro stato; con una sola cattura la routine resta `tradotta` con nota.
- Domanda 3 (preferenza): accettata, 234A resta fuori dal lotto.

## Chiusura ciclo (chat principale, 2026-10-20)

Esito: PASSATO, 13 verifiche su 13 rifatte dal verificatore. 15 routine nuove tutte `verificata` con 2 catture; 05B3, 2402, 326C portate a 2 catture; ramo decine di 3A1C catturato nel titolo (A=0C, f3406), assente in gioco e tardo entro 6000 frame. Fatti nuovi annotati in ROUTINE.md: 240B calcola HL=48·A; 6771 prosegue oltre il ret fino a 6784 (20 byte). Totale routine verificate: 33.
Token: analista Sonnet 74k, sviluppatore Opus 99k, verificatore Sonnet 20k.
