# Compito 05: lotto 1 di traduzione in C delle routine del Z80 principale

Data: 2026-10-19. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Tradurre in C leggibile 15 routine foglia del Z80 principale di Solomon's Key
(set MAME `solomon`) già osservate nelle tracce, verificarle con il banco di
prova `decomp/harness/` su almeno due catture MAME ciascuna, e fissare lo
schema che tutti i lotti successivi riusano (sezione «Schema dei lotti»).
Si lavora solo in `decomp/`.

Vincoli assoluti:
- MAME solo con `-video none -sound none`, mai con `-window` (né `-video
  soft`, né `-nomaximize`): nessuna finestra deve aprirsi. Le corse MAME si
  lanciano in sottofondo (`&` o `run_in_background`), mai in primo piano
  interattivo. `-debug -debugger none` con `-autoboot_script` funziona così.
- Nessuna operazione git (niente add, commit, checkout, stash, reset): il
  commit lo fa la chat principale.
- Niente modifiche fuori da `decomp/`. Non toccare `sessioni.sh`,
  `trace.lua`, `sessione.lua`, `sched*.c/h`, `fondale.c`, `routine.c`.
- Non rigenerare tracce complete né `callgraph.csv`: i conteggi servono già
  e stanno nella tabella sotto. Tracce o file sopra i 50 MB creati durante il
  lavoro vanno cancellati a fine compito.
- Nessun file di codice sopra le 800 righe; commenti, messaggi e documenti in
  italiano. Compilatore: `gcc` nel PATH; MAME: `/usr/games/mame` 0.264.

## Cosa esiste già (fatti verificati)

| file | righe | ruolo |
|---|---:|---|
| `decomp/harness/harness.c` | 139 | banco di prova: tabella `voce` locale con 3 voci, carica `snap_<ENTRY>_prima.bin/_dopo.bin/_regs.txt`, confronta c000-e5ff e A, BC, DE, HL, IX, IY |
| `decomp/harness/ram.h` | 39 | `uint8_t ram[0x4000]` (c000-ffff), `M(a)`, `struct z80 {a,f,b,c,d,e,h,l; ix,iy,sp}`, `BC()/SET_BC()` ecc., prototipi |
| `decomp/harness/routine.c` | 67 | `sub_05b3`, `sub_2402` (stile di riferimento con fondale.c) |
| `decomp/harness/fondale.c` | 208 | `sub_326c` e callee `sub_23bb`, `sub_23cb`, `sub_23ec`, `sub_336c` |
| `decomp/harness/Makefile` | 17 | `SORGENTI = harness.c routine.c fondale.c porte.c`; bersagli `harness`, `sched_proto` |
| `decomp/harness/sched_proto.c` | 142 | definisce anche lui `uint8_t ram[0x4000]` e include `ram.h` |
| `decomp/snap.lua` | 98 | cattura prima/dopo: `SNAP_ENTRY`, `SNAP_N`, `SNAP_DIR`, `SNAP_DA_FRAME`; scarta le occorrenze interrotte dalla NMI; limite fisso 6000 frame; nessun ingresso (niente moneta) |
| `decomp/trace.lua` | 38 | sequenza d'ingresso di riferimento: moneta frame 1510-1530, start 1560-1580, dal 1900 ogni 8 frame un campo casuale di `:P1` (`P1 Left/Right/Up/Down/Button 1/Button 2`) con `math.randomseed(1)` |
| `decomp/out/main48.dasm` | 35730 | disassembly z80dasm; indirizzo nel commento finale `;30ff`, etichette `sub_3101h:` |
| `decomp/out/istruzioni.txt` | | istruzioni eseguite con conteggio (traccia di controllo, 1900 frame) |
| `decomp/out/main64.bin` | 65536 byte | immagine ROM a indirizzi reali: 0000-bfff programma, f000-ffff dati |
| `decomp/out/<sessione>_chiamate.txt` | | `sito<TAB>destinazione<TAB>numero` per sessione: `titolo` (frame 0-1500, prima della moneta), `stanza_NN` (1500-2200, moneta e input casuali dal 1600), `gameover` (7000-7700) |

Fatti sul banco (da lettura di harness.c, ram.h, Makefile):
- `ram[]` copre solo c000-ffff: **la ROM 0000-bfff non è caricata**. Le
  routine che leggono tabelle in ROM (30EF legge 3111+, 0010 e 01C5 leggono
  da puntatori) hanno bisogno di un accesso alla ROM (vedi Struttura).
- `leggi_registri` carica anche F (`z->f = af & 0xff`): una routine che usa
  il riporto in ingresso (24B1 inizia con `rra`) lo legge da `z->f & 1`. Le
  bandiere in uscita non si confrontano.
- La pila sotto lo SP d'ingresso (SP-0x40 .. SP-1) non si confronta.
- `decomp/out/` è in `.gitignore`: le catture restano locali, come le tre già
  esistenti (`snap_0x05B3_*`, `snap_0x2402_*`, `snap_0x326C_*`).
- Il `Makefile` è tracciato in git (aggiunto a forza) nonostante `.gitignore`.

Uscite attuali, misurate il 2026-10-19 (`cd decomp/harness`):
```
./harness 0x05B3 → byte C8EE-C94D coincidenti: 96 su 96
                   registri C: A=00 BC=0001 DE=0030 HL=C94E IX=C8EE IY=FFFF
                   OK 0x05B3                                   (uscita 0)
./harness 0x2402 → byte E400-E401 coincidenti: 2 su 2 ... OK 0x2402
./harness 0x326C → pila libera CFB4-CFF3: 9 byte diversi, non confrontati
                   byte D880-DFFF coincidenti: 1920 su 1920 ... OK 0x326C
./sched_proto ../out/sched_log.txt | tail -1 →
  spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131
```

## Routine del lotto 1

Scelte da `out/callgraph.csv` e `out/chiamate.txt`: nessun `call`/`rst`
interno, nessuna porta, nessuna manipolazione dell'indirizzo di ritorno,
ordinate per invocazioni e chiamanti. Estensione = dall'ingresso al `ret`
finale letto in `main48.dasm` (il CSV sbaglia 24B1, 30EF, 3A1C, 5C78).
Invocazioni: traccia di controllo a 1900 frame. Conteggi per sessione:
somma della colonna 3 di `out/<sessione>_chiamate.txt` per quella
destinazione (t = titolo, s = stanza_00, g = gameover).

| ingresso | estensione | byte | invoc. | t / s / g | siti di chiamata | aree | catture |
|---|---|---:|---:|---|---|---|---|
| 0008 | 0008-000D | 6 | 73290 | 60236 / 19560 / 19061 | 26 siti in 18 routine (`rst 08`) | registri: HL += 2·A | titolo, gioco |
| 0010 | 0010-0014 | 5 | 73592 | 60188 / 20688 / 20311 | 20 siti in 18 routine (`rst 10`) | legge (HL),(HL+1), anche ROM | titolo, gioco |
| 026C | 026C-027B | 16 | 1835 | 1435 / 700 / 700 | 015B (NMI) | c08d bit 7, c08f, c090, c091 | titolo, gioco |
| 2418 | 2418-2424 | 13 | 310 | 267 / 45 / 1 | 3A56 | solo registri | titolo, gioco |
| 237E | 237E-23A0 | 35 | 135 | 68 / 67 / 0 | 3075 | (HL)..(HL+2) | titolo, gioco |
| 23A1 | 23A1-23AB | 11 | 270 | 136 / 134 / 0 | 3080 | (DE),(HL), parola a 16 bit | titolo, gioco |
| 249E | 249E-24B0 | 19 | 1037 | 0 / 9095 / 9299 | 4DBC | (HL)..(HL+5) | gioco, tardo |
| 24B1 | 24B1-24BD | 13 | 244 | 0 / 2136 / 2188 | 4E6A | (HL+1)..(HL+3); riporto in ingresso | gioco, tardo |
| 30EF | 30EF-3100 | 18 | 4320 | 2176 / 2144 / 0 | 30AC, 30C8 | legge tabella ROM 3111 | titolo, gioco |
| 3A1C | 3A1C-3A2D | 18 | 198 | 3 / 195 / 3 | 3955, 3964, 397B, 7C10 | solo registri (cifre decimali in D, E) | titolo, gioco |
| 3F10 | 3F10-3F38 | 41 | 8 | 1 / 63 / 62 | 3ACF, 3EB4 | legge c093-c095, scrive c061-c067 | titolo, gioco |
| 4AF6 | 4AF6-4B0D | 24 | 62 | 32 / 37 / 13 | 2F48, 2F83, 2FEE, 3054, 318D, 31AE, 3772, 37C6 | IX+00,04,05,1B,1C,1D,1E | titolo, gioco |
| 4B0E | 4B0E-4B1F | 18 | 22 | 11 / 19 / 6 | 2F34, 2FB0, 2FC8, 2FCF, 300D, 35B2, 35C8, 37B7 | solo registri | titolo, gioco |
| 5C78 | 5C78-5C8D | 22 | 140 | 0 / 1616 / 1404 | 5C4E, 5C5B, 5C68, 5C73 | legge (DE) | gioco, tardo |
| 673F | 673F-6749 | 11 | 5 | 2 / 3 / 2 | 2947, 2C3A, 78E7, 7C3D, 7CA1 | azzera c000-c03f (tabella attività) | titolo, gioco |

Totale 270 byte. Significati RAM già noti (da `decomp/DATI.md` e
`decomp/SCHEDULER.md`): c000-c03f tabella delle attività; c08f valore scelto
dal record della stanza (byte 5-8 della testa 8EA4); c090-c095 tempo in BCD;
il blocco oggetti parte da c46e con voci da 0x30 (`routine.c`). Note per la
traduzione: 3A1C ha un ciclo (3A23-3A24) mai eseguito nella traccia di
controllo (A < 10); 026C esce subito (`ret z`) in 1764 casi su 1835; 0008 e
0010 sono i gestori `rst 08` e `rst 10`, routine normali (nessun `ex
(sp),hl`). Esclusi e perché: 0038 e 08CE (manipolano l'indirizzo di
ritorno), 027C (`jp nc,030B` fuori estensione), 0CE6 (continua in codice mai
eseguito), 01C5 (ramo principale 2 volte su 52), 0493 (già modellata in
`sched.c`), 7EFE e 2457 (rinviate al lotto 2).

Il disassembly di ogni routine lo legge lo sviluppatore, per esempio (da `decomp/`):
`sed -n "$(grep -n ';30ef$' out/main48.dasm | cut -d: -f1),+20p" out/main48.dasm`
e, per le sole istruzioni eseguite con i conteggi,
`awk -F'\t' '$1>="30EF" && $1<="3100"' out/istruzioni.txt`.

## Schema dei lotti (vale per questo lotto e per tutti i successivi)

### 1. Catture: almeno due stati per routine

Stati, scelti dai conteggi per sessione già su disco (nessuna traccia nuova):

| stato | ingressi (`SNAP_INGRESSI`) | `SNAP_DA_FRAME`, in ordine di tentativo | quando si usa |
|---|---|---|---|
| `titolo` | `nessuno` | 1000, 650, 0 | t > 0 |
| `gioco` | `traccia` | 2400, 2000, 1700, 1600, 1500 | sempre |
| `tardo` | `traccia` | 4000, 3000 | t = 0 (al posto di `titolo`) |

Conteggio t/s/g per una routine nuova (da `decomp/`):
`for s in titolo stanza_00 gameover; do awk -F'\t' -v d=XXXX '$2==d{n+=$3} END{print n+0}' out/${s}_chiamate.txt; done`.

Si prova il primo frame della lista; se MAME arriva al limite senza
cattura, il successivo. Regole di qualità, controllate con `cmp`:
- i due `prima.bin` della stessa routine devono differire (`cmp -s` ≠ 0);
  per le routine «solo registri» devono differire anche le righe `prima` di
  `_regs.txt`; altrimenti si rifà la seconda cattura con `SNAP_N=2`, poi 3;
- per una routine che scrive RAM almeno una cattura deve essere non banale:
  `_prima.bin` e `_dopo.bin` diversi in c000-e5ff (primi 0x2600 byte:
  `cmp -n 9728`). Se dopo `SNAP_N` 1..5 in tutti gli stati è sempre banale,
  si tiene la cattura e si annota «ramo principale non catturato» nella
  colonna note di `ROUTINE.md` (lo stato resta `tradotta`).

### 2. Script di cattura `decomp/cattura.sh` (nuovo)

Uso da `decomp/`: `sh cattura.sh <ENTRY> <stato> [N]` (es. `sh cattura.sh
0x30EF gioco`). Per ogni frame della lista dello stato: cancella
`out/snap_<ENTRY>_<stato>_*`, lancia

```
SNAP_ENTRY=$1 SNAP_TAG=$2 SNAP_INGRESSI=<da stato> SNAP_DA_FRAME=<f> SNAP_N=${3:-1} \
  mame solomon -rompath "$(cd .. && pwd)/roms" -video none -sound none \
  -skip_gameinfo -nothrottle -debug -debugger none -autoboot_script snap.lua \
  > out/snap_$1_$2.log 2>&1
```

e si ferma al primo frame per cui esiste `out/snap_<ENTRY>_<stato>_regs.txt`;
stampa `CATTURA <ENTRY> <stato> frame_da=<f> OK` o `CATTURA <ENTRY> <stato>
FALLITA`. Le chiamate si lanciano in sottofondo, al massimo 4 in parallelo
(file diversi per ENTRY e stato).

### 3. Estensione di `decomp/snap.lua`

- `SNAP_TAG`: se non vuoto i file sono `<dir>/snap_<ENTRY>_<TAG>_prima.bin`,
  `_dopo.bin`, `_regs.txt`; vuoto = nomi attuali (compatibilità).
- `SNAP_INGRESSI`: `nessuno` (predefinito, comportamento attuale) o
  `traccia`: stessi ingressi di `trace.lua` righe 7-13 e 21-31 (moneta,
  start, casuali dal 1900 con `math.randomseed(1)`), senza fine traccia.
- `SNAP_LIMITE`: limite di frame (predefinito 6000, come ora).
- Il messaggio finale aggiunge il tag: `SNAP: catturata occorrenza N di
  <ENTRY> [<TAG>] al frame F, scartate K`.

### 4. Traduzione

Un file C per intervallo di 4 KB: `decomp/harness/z80_0xxx.c`,
`z80_2xxx.c`, `z80_3xxx.c`, ... (`z80_Nxxx.c` per 0x N000-0xNFFF). Se un
file supera 600 righe si spezza in `z80_N0xx.c` / `z80_N8xx.c` (metà del
blocco). `routine.c` e `fondale.c` restano come sono. Firma unica:
`void sub_xxxx(struct z80 *z)` (esadecimale minuscolo). Stile di
`fondale.c` e `routine.c`: commento di testa con ingresso, uscita e
significato; `#define` con nomi per i campi RAM; cicli `for`; nessuna
emulazione di bandiere salvo il riporto dove serve davvero (commento che lo
dice); registri A, BC, DE, HL, IX, IY in uscita come l'originale. Letture
che possono cadere sotto c000 con `leggi()`, scritture solo con `M()`.

### 5. Registrazione

- `decomp/harness/routine.h` (nuovo): `struct voce_routine` e prototipi delle
  routine dei lotti; `ram.h` resta com'è più l'accesso alla ROM.
- `decomp/harness/routine_tab.c` (nuovo): la tabella, in ordine di indirizzo,
  con le 3 voci attuali di `harness.c` più quelle del lotto.
- `decomp/ROUTINE.md` (nuovo, aggiornato da ogni lotto): una riga per routine
  `| 0xXXXX | sub_xxxx | file | byte | descrizione | stato | catture OK | stati | note |`.
  Stato `tradotta` = C scritto, meno di 2 catture OK; `verificata` = almeno 2
  catture tutte OK e le regole di qualità rispettate. Le 3 routine
  precedenti entrano con 1 cattura (stato `base`) e stato `tradotta`.

### 6. Selezione dei lotti successivi (per l'analista)

Routine `brutta` vuota in `callgraph.csv`, tutte le destinazioni dei siti
dentro la sua estensione già `verificata` in `ROUTINE.md`, nessuna porta;
priorità a invocazioni e numero di siti; 10-15 per lotto.

## Struttura da realizzare

1. `ram.h`: aggiungere
   ```c
   extern uint8_t rom[0x10000];
   /* Lettura da tutto lo spazio del Z80: c000-ffff dalla cattura, il resto dalla ROM. */
   static inline uint8_t leggi(uint16_t a) { return a >= 0xc000 ? ram[a - 0xc000] : rom[a]; }
   ```
   `rom` è definita solo in `harness.c` (`sched_proto` non la usa: va bene).
2. `routine.h`:
   ```c
   struct voce_routine {
       const char *entry;                 /* "0x05B3", maiuscolo come nei file */
       void (*fn)(struct z80 *);
       uint16_t blocco_ini, blocco_fine;  /* 0, 0 = nessun blocco esplicito */
   };
   extern const struct voce_routine tabella_routine[];
   extern const unsigned num_routine;
   ```
   più i 15 prototipi `void sub_0008(struct z80 *z);` ecc.
3. `harness.c`: togliere la tabella locale; caricare `main64.bin` in `rom`
   (cercato in `out/` poi `../out/`); per l'ENTRY richiesto provare ogni
   cattura trovata nella prima cartella fra `SNAP_DIR`, `out`, `../out` che
   ne contiene: la cattura senza tag (`snap_<ENTRY>_regs.txt`, tag `base`)
   e quelle con tag (`glob` su `snap_<ENTRY>_*_regs.txt`; serve `#define
   _POSIX_C_SOURCE 200809L` in testa). Per ogni cattura: riga `cattura
   <tag>`, poi le righe attuali (con `blocco_ini == 0` la riga `byte ...`
   non si stampa), poi `OK <ENTRY> <tag>` o `FALLITO <ENTRY> <tag>`. In
   fondo `RIEPILOGO <ENTRY>: <n> catture, <k> OK`; uscita 0 se n ≥ 1 e k = n.
   `./harness --tutte` fa lo stesso per ogni voce di `tabella_routine`, uscita
   0 se tutte passano.
4. `Makefile`: `SORGENTI = harness.c routine.c fondale.c porte.c
   routine_tab.c $(wildcard z80_*.c)` e `routine.h` fra le dipendenze di
   `harness`. Bersaglio `sched_proto` invariato.
5. `z80_0xxx.c` (0008, 0010, 026C), `z80_2xxx.c` (2418, 237E, 23A1, 249E,
   24B1), `z80_3xxx.c` (30EF, 3A1C, 3F10), `z80_4xxx.c` (4AF6, 4B0E),
   `z80_5xxx.c` (5C78), `z80_6xxx.c` (673F).
6. `blocco_ini/blocco_fine` per le voci nuove: il primo intervallo scritto se
   fisso (026C: c08f-c091; 3F10: c061-c067; 673F: c000-c03f); 0, 0 per le
   altre.
7. `cattura.sh`, estensione di `snap.lua`, `ROUTINE.md` (18 righe).
8. Catture: 2 per ognuna delle 15 routine, stati della colonna «catture».

## Verifiche obbligatorie

Da `decomp/` salvo dove indicato.

1. `make -C harness 2>&1 | grep -ci -E 'warning|error'` → `0`; esistono
   `harness/harness` e `harness/sched_proto` (attesa, non misurata).
2. Per ognuna delle 15 routine: `(cd harness && ./harness 0xXXXX | tail -1)`
   → `RIEPILOGO 0xXXXX: 2 catture, 2 OK` (3 se ne è stata aggiunta una),
   uscita 0 (attesa, non misurata).
3. Routine precedenti, `(cd harness && ./harness 0x05B3)` → contiene `byte
   C8EE-C94D coincidenti: 96 su 96` e `registri C: A=00 BC=0001 DE=0030
   HL=C94E IX=C8EE IY=FFFF` (misurato), ultima riga `RIEPILOGO 0x05B3: 1
   catture, 1 OK` (attesa, non misurata). Idem 0x2402 (`byte E400-E401
   coincidenti: 2 su 2`) e 0x326C (`byte D880-DFFF coincidenti: 1920 su 1920`),
   righe misurate.
4. `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `18`;
   `... | grep -c FALLITO` → `0`; uscita 0 (attesa, non misurata).
5. `(cd harness && ./sched_proto ../out/sched_log.txt | tail -1)` →
   `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
6. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale"'`
   → vuoto (misurato oggi: vuoto).
7. `grep -c '| verificata |' ROUTINE.md` → `15`; `grep -c '^| 0x' ROUTINE.md`
   → `18` (attesa, non misurata).
8. `ls out/snap_0x*_titolo_regs.txt out/snap_0x*_gioco_regs.txt out/snap_0x*_tardo_regs.txt | wc -l`
   → `30` o più (attesa, non misurata).
9. Per ogni routine i due `prima.bin` differiscono:
   `for e in 0008 0010 026C 2418 237E 23A1 249E 24B1 30EF 3A1C 3F10 4AF6 4B0E 5C78 673F; do set -- out/snap_0x${e}_*_prima.bin; cmp -s "$1" "$2" && echo "UGUALI $e"; done`
   → vuoto (attesa, non misurata).
10. `grep -n -e '-window' -e 'video soft' cattura.sh snap.lua` → vuoto.
11. `find . -size +50M` → vuoto (misurato oggi: vuoto).
12. `git status --short` (sola lettura) → solo `M decomp/harness/Makefile`,
    `M decomp/harness/harness.c`, `M decomp/harness/ram.h`, `M decomp/snap.lua`
    e i nuovi `decomp/ROUTINE.md`, `decomp/cattura.sh`,
    `decomp/harness/routine.h`, `decomp/harness/routine_tab.c`,
    `decomp/harness/z80_*.c`, oltre a `compiti/05-lotto-01-routine.md`.

## Consegna

File sopra elencati, nessun commit (lo fa la chat principale con la riga
`Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`). Rapporto
finale di massimo trenta righe: uscita delle verifiche 1-12, per ogni routine
gli stati e i frame di cattura usati, routine finite `tradotta` e perché,
deviazioni dal brief.

## Domande aperte

1. Nomi C: `sub_xxxx` come ora, oppure nomi descrittivi (es.
   `indice_parola_rst08`)? Proposta: `sub_xxxx` nel codice, descrizione in
   `ROUTINE.md` e nel commento. «preferenza»
2. Seconda cattura per 05B3, 2402, 326C in questo lotto per portarle a
   `verificata`? Proposta: no, nel lotto 2 con `cattura.sh`. «preferenza»
3. `sessioni.sh` usa ancora `-video soft -window`: correggerlo a `-video none
   -sound none`? Proposta: sì, ma in un compito a parte; qui non si tocca né
   si esegue. «preferenza»
4. Le catture in `decomp/out/` non vanno in git, quindi la verifica dei lotti
   si ripete solo su questa macchina. Proposta: lasciarle fuori (circa 50 KB
   per cattura) e ricrearle con `cattura.sh` quando serve. «scelta di progetto»

## Decisioni (chat principale, 2026-10-19)

- Domanda 1 (preferenza): accettata, nomi `sub_xxxx`.
- Domanda 2 (preferenza): accettata, seconda cattura per 05B3, 2402, 326C rinviata al lotto 2.
- Domanda 3 (preferenza): accettata con aggiunta: in questo stesso lotto lo sviluppatore sostituisce `-window` con `-video none` in `sessioni.sh` (una riga), senza eseguirlo. Nessun comando MAME di questo brief apre finestre.
- Domanda 4 (scelta di progetto, decisa dalla chat principale in base alle indicazioni dell'utente di includere i dati derivati): le catture prima/dopo (`out/snap_*`) VANNO in git, così la verifica è ripetibile da un clone. Sono piccole (circa 20 KB per cattura) e coperte da NOTICE.md. Lo sviluppatore non committa: la chat principale toglierà `decomp/out/` dal .gitignore per i soli file `snap_*` a fine ciclo.

## Chiusura ciclo (chat principale, 2026-10-19)

Esito: PASSATO, 12 verifiche su 12, verificatore senza MAME (catture rilette da disco). 15 routine verificate con 2 catture; 05B3, 2402, 326C restano a 1 cattura per il lotto 2. Ramo non coperto annotato: decine in 3A1C.
Token: analista Opus 113k, sviluppatore Opus 69k, verificatore Sonnet 16k.
