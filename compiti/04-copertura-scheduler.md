# Compito 04: copertura con partite guidate e modello a funzioni dello scheduler

Data: 2026-10-19. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

Fase 1 della decompilazione del Z80 principale di Solomon's Key (set
`solomon`). Va eseguito DOPO il brief 03 (`compiti/03-rst20-e603-livelli.md`),
che produce `decomp/rst20.py`, `decomp/e603.lua`, `decomp/harness/e603.h` e
la regola del tap globale riusata qui. Lavora solo in `decomp/`.

## Obiettivo

d) Alzare la copertura del codice eseguito con sessioni MAME guidate
   (stanza forzata, partita senza input fino al game over, titolo e demo,
   eventuale partita umana registrata) e misurare prima/dopo la percentuale
   delle 299 destinazioni statiche di `call` osservate.
e) Documentare in `decomp/SCHEDULER.md` lo spedidore delle attività a 0x3AF3
   e la sua interazione con la NMI, e costruire in `decomp/harness/` un
   prototipo C **a funzioni** (decisione già presa: niente coroutine né
   cambio di contesto; una funzione C per ogni codice di attività, chiamata
   da un ciclo sui 64 slot che riproduce uno a uno lo spedidore, con lo
   stato delle attività in RAM come nell'originale) che riproduca la
   sequenza di spedizione misurata in MAME. Le sessioni `bonus` e `finale`
   sono fuori perimetro (rinviate, decisione dell'utente in sospeso).

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/`. Non toccare `src/`, `tools/`, `res/`,
  `roms/`, `compiti/`.
- Niente da installare: `mame` 0.264, `gcc` 13.3, `python3`. Mai `-debugger imgui`.
- Commenti, messaggi, nomi di file e documenti in italiano.
- Nessun file di codice sopra le 800 righe; script rieseguibili da `decomp/`.
- Tracce MAME cancellate dopo l'elaborazione (ogni traccia di 700 frame pesa
  circa 90 MB; restano solo i derivati in `out/`).
- MAME con `-debug` e traccia fa circa 6 frame al secondo (15 minuti per
  5500 frame): dimensionare le finestre di traccia, mai tracciare l'intera
  sessione. Senza debugger e con `-video none` fa 5500 frame in 6 s.
- `sudo` disponibile con `sudo -S <cmd> < ~/.claude/sudo.pass`, senza mai
  stampare la password; non dovrebbe servire.

## Cosa esiste già (fatti verificati)

### Strumenti in `decomp/` (da brief 02 e 03)

- `trace.lua` (38 righe): moneta `":SYSTEM"`/`"Coin 1"` ai frame 1510-1530,
  start `"1 Player Start"` 1560-1580, input casuali da 1900 (`math.randomseed(1)`,
  ogni 8 frame un campo fra `P1 Left/Right/Up/Down/Button 1/Button 2`),
  `dbg:command("trace out/trace.txt,0,noloop")` all'avvio, `trace off` e
  `manager.machine:exit()` al frame `TRACE_FRAMES`. `dbg:command` funziona
  anche dentro `emu.register_frame_done` (snap.lua lo usa per `bpset`).
- `coverage.py` (225 righe): `--traccia`, `--rom`, `--cancella`; scrive
  sempre `out/eseguiti.txt` (un PC esadecimale per riga), `out/istruzioni.txt`
  (`pc\tesecuzioni\tistruzione`), `out/chiamate.txt`, `out/copertura.txt`,
  `out/numeri.txt`. Va esteso con un'opzione di prefisso per non
  sovrascrivere (vedi struttura).
- `snap.lua`, `harness/` (ram.h, routine.c, harness.c, Makefile, e dal brief
  03 fondale.c, rst20_tables.h, e603.h, porte.c).
- Tap Lua: handle in variabile **globale** (`TAP = mem:install_read_tap(...)`),
  altrimenti il tap sparisce dopo pochi frame; nel callback
  `cpu.state["PC"].value` è il PC dopo l'istruzione (3AFA `or (hl)` → 3AFB).
- Stanza forzata: tap in lettura su c243 dal frame 1500 al 1950 che
  restituisce N (0-based), poi `TAPC:remove()`.
- Riga MAME veloce: `mame solomon -rompath /home/rufio72/OpenSolomonsKey-master/roms -video none -sound none -skip_gameinfo -nothrottle -autoboot_script X.lua`
  (`DISPLAY=:1`); con traccia: `-video soft -window -nomaximize -debug -debugger none` al posto di `-video none`.
- `-record nome.inp` registra gli input (cartella `-input_directory`,
  default `inp/` sotto la cartella corrente), `-playback nome.inp` li
  riproduce; `-seconds_to_run N` esce dopo N secondi emulati (da `mame -showusage`).

### Copertura (misurata)

- Destinazioni statiche di `call` in 0000-bfff: **299** (339 contando RAM e
  spazzatura). Regex sul dasm: `^\tcall (?:[a-z]+,)?(?:0([0-9a-f]{4})h|(?:l|sub_)([0-9a-f]{4})h)(?:\+\d+)?\t`,
  tenere `d < 0xc000`.
- Osservate (PC presente in `out/eseguiti.txt`): **123 su 299 (41,1%)** con
  la traccia a 1900 frame oggi in `out/`; 129 (43,1%) a 5500 frame con
  input casuali (RAPPORTO.md).
- Grandi zone mai eseguite (RAPPORTO.md): 80a5-bfff (dati), 12ab-2349,
  5cfb-63a3, 4f1c-5506, 0cf3-1287, 6c48-714a, 6785-6c1c, 3f93-4397.

### Fasi del gioco senza input (misurate: dump di c000-c03f ogni 100 frame per 20000 frame, moneta/start come trace.lua)

Codici attività presenti nella tabella c000-c03f (solo quando cambiano):
```
100  (vuota)          700  14 06           1200 14 07
1900 14 0A 0D 20      7400 14 15           7600 14 07
8000 14 0A 0D 20      13400 14 15          13700 14 07
14000 14 0A 0D 20     19500 14 15          19700 14 16
```
Lettura: 14 0A 0D 20 = partita o demo in corso; 15 = fine partita/demo
(compare fra il frame 7300 e il 7400 senza input, cioè la partita senza
input finisce circa 97 s dopo lo start); 07 = sequenza titolo/attract; 16 =
schermata successiva del ciclo attract (da identificare). `c243` resta 0 e
`c246` 0 per tutta la corsa. Il contatore vite non è in c000-c2ff con un
andamento 3→2→1→0 a passi di 100 frame: va cercato (diff di RAM fra frame
1900 e 7400, oppure tap in scrittura).

### Lo spedidore 0x3AF3 (misurato su disassembly, `istruzioni.txt` e tap a 5500 frame)

```
3AF3 ld a,(c04f)      ; indice corrente (0..63)
3AF6 ld l,a ; ld h,c0 ; HL = c000 + indice: tabella attività di 64 byte
3AF9 xor a
3AFA or (hl)          ; A = codice dell'attività nello slot (0 = vuoto)
3AFB jr nz,3B02
3AFD inc l ; res 6,l  ; indice ciclico su 64
3B00 jr 3AFA
3B02 ld b,a
3B03 ld de,3AF3 ; push de   ; ritorno allo spedidore
3B07 push hl                ; puntatore allo slot, per l'attività
3B08 inc l ; res 6,l ; ld a,l ; ld (c04f),a   ; prossimo indice
3B0F ld a,b
3B10 rst 20h          ; salto a tabella_3B11[codice], 40 voci (brief 03)
```
- Tabella 3B11 (codice → ingresso): `0:3B61 1:48D1 2:4B84 3:4BC6 4:4C30 5:4C42 6:75E7 7:2CCD 8:7C98 9:4D29 A:4DA2 B:2C27 C:7BF6 D:5C8E E:63A4 F:640E 10:457B 11:456E 12:426A 13:6480 14:44B5 15:6466 16:6496 17:6429 18:4124 19:3792 1A:411D 1B:4111 1C:3F40 1D:3F39 1E:3B61 1F:43C8 20:3E90 21:3B61 22:12D6 23:3E4E 24:2C72 25:3B77 26:3B63 27:62B6` (3B61 = `pop hl; ret`).
- Un'attività entra con A = codice, (SP) = puntatore allo slot, (SP+2) =
  3AF3. Modi di uscita visti: `pop hl; ret` (resta nello slot, rieseguita al
  giro dopo: 3E98, 4B8B, 4BCD); `pop hl; ld (hl),n; ret` (cambia il proprio
  codice: 4B95 → 3, 4C3F → 3, 4C48 → 9); azzeramento dello slot da parte
  di chi la termina (scrittori misurati sotto).
- Attività a passi: 2CCD (codice 07) `ld hl,c19d; ld a,(hl); inc (hl); rst
  20h` spedisce sul passo c19d (26 voci); i passi fanno `dec (hl)` per
  restare sul passo (attesa) e `inc (hl)` per avanzare; sotto-passo in c19e
  (2DAD: `cp 6`, 2DCE: `cp 7`). Stesso schema a 3B63 (codice 26). In C il
  contatore resta in RAM e la funzione dell'attività spedisce su di esso
  (`switch`), un passo per chiamata, come l'originale.
- Primitiva «posta attività» 0x0493 (A = codice): `ld hl,c04e; ld d,a;`
  ciclo 0496: `ld a,3f; and (hl); inc (hl)` (indice = c04e & 3f, c04e++
  senza maschera), se il byte a c000+indice è diverso da 0 ripete, altrimenti
  scrive il codice a c000+indice (04AA) e `ret`. Chiamanti (chiamate.txt):
  0267 (1), 2951, 2956, 2C52, 2C6E, 2E35 (6), 78EC, 7979, 7C53, 7CA6, 7CE0,
  e 017B nella NMI (`ld a,27h; call 0493`, solo se e603 bit 0 = 1: mai in
  MAME). Azzeramento della tabella: 6742 (`ld hl,c000; ld (hl),c; inc hl;
  djnz`, in 673F chiamata da 2C3A nell'attività 0B) e 2868 (init).
- **Una sola pila**: `ld sp` eseguiti solo a 283A (`dc00`, init) e 286E
  (`d000`); nessun `ld sp,(nn)` né `ld (nn),sp` nella ROM; `ex (sp),hl`
  solo nei gestori rst (0020/0025/0038/0041) e a 2A23-2A38, 323C. Le «pile
  cff0-cffa e dbfa» del rapporto 02 sono profondità di chiamata sulla pila
  d000 e la pila dc00 dell'init (cattura al frame 66, durante 2858-286C).
  Lo spedidore è un polling round-robin, le attività conservano lo stato
  in RAM e cedono ritornando a 3AF3.
- Numeri a 5500 frame (tap su c000-c03f e c04f): letture della tabella a
  3AFB 2.073.166; spedizioni (letture a 3AFB con valore ≠ 0) **106.124**,
  di cui **30.725** nei frame ≤ 1900 (la traccia a 1900 frame conta 30.704
  esecuzioni di 3B10: differenza di un frame di allineamento); per frame
  (1900-5500) min 15, max 29, media 21; giri (`ld a,(c04f)` a 3AF3) uguali
  alle spedizioni. Scritture nella tabella **1298**, per PC: 2C1B 512 (test
  RAM, valore 80), 04AB 110 (0493), 6746 64, 2868 64, 4C10 57, 63C2 46,
  4B97 38, 4937 26, 4925 14, 4591 11, 457A 2, 43E4 2, 2E58 2, 7619 1.
- Campioni (frame slot codice): frame 1600: `0B 14; 0C 07; 0B 14; 0C 07;`
  ... (14 coppie); frame 1899 finisce `0B 14; 0D 0A; 0E 0D; 0F 20;`, frame
  1900 inizia `10 0E; 0B 14; 0D 0A; 0E 0D; 0F 20; 10 03; 0B 14; 0D 0A;`
  (l'indice c04f sopravvive al cambio di frame: il giro non è allineato
  alla NMI).
- NMI 0066 (318 byte, 1 per frame, 1835 in 1900 frame, 5435 in 5500: è
  disabilitata nei primi ~65 frame): `push af; exx; ld (c040),ix; ld
  (c042),iy; ld hl,c044; ld a,(hl); bit 2,a; jp z,00C4; bit 6,a; jp z,0098;
  res 6,(hl); ldir c994→e400 (32) e c9ca→e500 (128)`; altri `ldir` a 00A1,
  00AC, 00B7, 00C2 (sprite), `call 05B3` (tabella sprite), 0150 `call nz,01C5`,
  0156 `inc (c2b7)`, 015A `inc (c29a)` (contatori di frame letti dalle
  attività: 4DA2 legge c2b8, 5C8E legge c29a), 015B `call 026C`, 015E `ld
  a,(e603)`, 019A `ld a,(e606)`, 01A4 `ld (e800),a` (audio), 01BB/01BF `ld
  (e600),a`, 01C3 `retn`. La NMI non scrive la tabella attività (salvo
  017B, mai eseguito).

## Struttura da realizzare

Tutto in `/home/rufio72/OpenSolomonsKey-master/decomp/`.

### d) Sessioni guidate e copertura

1. `coverage.py`: aggiungere `--prefisso NOME` che scrive `out/NOME_eseguiti.txt`,
   `out/NOME_istruzioni.txt`, `out/NOME_chiamate.txt`, `out/NOME_copertura.txt`,
   `out/NOME_numeri.txt` (senza prefisso, comportamento invariato).
2. `sessione.lua` (nuovo, modello trace.lua + snap.lua per `dbg:command` nei
   frame): variabili d'ambiente `SESS_NOME` (prefisso dei file), `SESS_ROOM`
   (stanza forzata 0-based, vuota = nessun tap), `SESS_TRACE_DA` e
   `SESS_TRACE_A` (finestra della traccia: `trace out/<nome>_trace.txt,0,noloop`
   al frame DA, `trace off` e uscita al frame A), `SESS_INPUT` =
   `nessuno | casuale | file.lua` (un file Lua che ritorna una lista di
   `{frame, campo, valore}`), `SESS_SEED`. Moneta e start sempre ai frame
   1510-1580. La sessione stampa a ogni cambio il contenuto non nullo di
   c000-c03f con il frame (`FASE <frame> <codici>`), così si vede quando
   compare il codice 15.
3. `sessioni.sh` (nuovo): esegue in sequenza, ognuna seguita da `python3
   coverage.py --traccia out/<nome>_trace.txt --prefisso <nome> --cancella`:
   - `stanza_NN` per NN = 0..51 tranne 49 (brief 01: la stanza 50 non carica):
     `SESS_ROOM=NN`, input casuale dal 1600, traccia 1500-2200 (circa 2
     minuti ciascuna con la traccia, 51 sessioni ≈ 2 ore: lanciare in
     background e proseguire con e) nel frattempo);
   - `gameover`: nessun input, traccia 7000-7700 (fine partita, codice 15);
   - `titolo`: traccia 0-1500 (test RAM, titolo, demo iniziale);
   - `attract`: nessuna moneta, traccia 7400-8200 e 19400-19900 (fasi 07/16
     dopo la demo: punteggi, titolo);
   - `bonus` e `finale`: **fuori perimetro**, rinviate (decisione
     dell'utente in sospeso): non vanno tentate, non entrano nei cancelli.
4. `copertura_call.py` (nuovo): legge il dasm per le 299 destinazioni e uno
   o più file `*_eseguiti.txt`; stampa `destinazioni statiche osservate: N su
   299 (P%)` per ogni file e per l'unione, e scrive `out/eseguiti_unione.txt`
   e `out/call_mai_osservate.txt` (le destinazioni restanti con l'indirizzo
   della prima `call` statica che le raggiunge, per capire che cosa manca).
5. Istruzioni per la partita umana (nel rapporto, non da eseguire):
   `cd decomp && DISPLAY=:1 mame solomon -rompath ../roms -video soft -window -skip_gameinfo -record umana.inp`
   (gioca, poi esce con Esc; il file va in `decomp/inp/umana.inp`), replica
   con `mame solomon ... -playback umana.inp -debug -debugger none
   -autoboot_script sessione.lua` e `SESS_INPUT=nessuno` (gli input vengono
   dal file), finestre di traccia scelte dai frame delle fasi stampate.
6. Estensione di e603: ogni sessione tiene anche il tap su e603 del brief
   03 (`out/<nome>_e603.txt`); se compare un PC diverso da 0161, aggiornare
   `harness/e603.h` e dirlo nel rapporto.

### e) Scheduler: documento, misura e prototipo a funzioni

7. `SCHEDULER.md` (nuovo): la tabella c000-c03f, c04e/c04f, 0493, 3AF3-3B10,
   la tabella 3B11 con, per ogni codice, l'ingresso e il modo di uscita
   osservato; le attività a passi (c19d/c19e); la NMI e i contatori c2b7/c29a;
   la prova della pila unica; i numeri misurati sopra; il modello C a
   funzioni adottato e la corrispondenza uno a uno (slot della tabella →
   voce di `tabella[64]`, codice → funzione `attivita_XX(uint8_t slot)`,
   `pop hl; ret` → `return`, `pop hl; ld (hl),n; ret` → `tabella[slot] = n;
   return`, contatore di passo c19d → `switch` sul byte in RAM). Motivare
   in dieci righe perché non servono coroutine: nessuna attività sospende
   uno stato sulla pila, tutto lo stato è in RAM e ogni ingresso riparte
   dalla testa della routine.
8. `sched_log.lua` (nuovo): tap in lettura su c000-c03f (PC 3AFB, valore ≠ 0
   → riga `D <frame> <slot> <codice>`), tap in scrittura su c000-c03f (riga
   `W <frame> <pc> <slot> <valore>`), nell'ordine in cui avvengono, in
   `out/sched_log.txt`; stessi input di trace.lua; frame finale
   `SCHED_FRAMES` (default 5500). Dura circa 6 s.
9. `harness/sched.h`, `harness/sched.c` (nuovi): `uint8_t *tabella` (=
   `&M(0xc000)`, 64 byte), `indice` (= M(0xc04f)), `prossimo_libero` (=
   M(0xc04e)); `uint8_t posta_attivita(uint8_t codice)` con la semantica
   esatta di 0493 (restituisce lo slot scelto); `int spedisci_una(uint8_t
   *slot, uint8_t *codice)` = 3AF3-3B10: cerca dal `indice` il primo slot
   non nullo (ciclico su 64), aggiorna `indice` come 3B08-3B0C, chiama
   `corpo_attivita[codice](slot)` e ritorna; `typedef void (*attivita_fn)(uint8_t slot)`
   e `attivita_fn corpo_attivita[40]` = tabella 3B11 in C (per ora stub;
   le routine tradotte vi si agganceranno con lo stesso prototipo). Nessun
   cambio di contesto, nessuna pila separata.
10. `harness/sched_proto.c` (nuovo, eseguibile `harness/sched_proto`):
    `./harness/sched_proto out/sched_log.txt [frame_max]`. Legge il log;
    le righe `W` prima della prima `D` si applicano subito (test RAM e
    init); poi, a ogni spedizione del prototipo, lo stub dell'attività
    chiamata applica le righe `W` del log che seguono la `D` corrente fino
    alla `D` successiva (le `W` con pc 04AB passano per `posta_attivita`
    e si confronta lo slot scelto con quello del log; le altre scrivono
    direttamente), poi ritorna. Si confrontano slot e codice di ogni `D`
    del log con quelli prodotti da `spedisci_una`; alla prima differenza
    si stampa frame, numero di spedizione, atteso e ottenuto. Uscita:
    `spedizioni confrontate: N, differenze: 0; posta_attivita: M, slot
    coincidenti: M` e codice di uscita 0/1. Le attività sono tutte stub in
    questo brief: il confronto verifica lo spedidore, la primitiva di posta
    e il ciclo di chiamata, non le attività.
11. `harness/Makefile`: bersagli `harness` (invariato più i file del brief
    03) e `sched_proto` (sched.c sched_proto.c, stesso `CFLAGS`); `make`
    costruisce entrambi.

Libere: nomi delle funzioni, formato esatto del log purché stabile, se
`corpo_attivita` è un array statico o una `switch`.

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master/decomp/` con `export DISPLAY=:1`,
`MOPT` e `MFAST` come nel brief 03. Prima di tutto `touch .inizio`.

1. `python3 copertura_call.py out/eseguiti.txt` → `destinazioni statiche osservate: 123 su 299 (41.1%)` (misurato sulla traccia a 1900 frame presente in `out/`).
2. `SESS_NOME=prova SESS_ROOM=1 SESS_TRACE_DA=1500 SESS_TRACE_A=1700 SESS_INPUT=casuale mame solomon $MOPT -debug -debugger none -autoboot_script sessione.lua && python3 coverage.py --traccia out/prova_trace.txt --prefisso prova --cancella && wc -l < out/prova_eseguiti.txt` → file presente con più di 1000 righe (attesa, non misurata); `ls out/prova_trace.txt` → assente.
3. `sh sessioni.sh > out/sessioni.log 2>&1` (in background, circa 2-3 ore) e poi `python3 copertura_call.py out/*_eseguiti.txt | tail -1` → `unione: N su 299 (P%)` con N > 129 (attesa, non misurata; obiettivo dichiarato N ≥ 270, cioè oltre il 90%, senza le sessioni `bonus` e `finale` che sono fuori perimetro; il numero reale è attesa, non misurata). Riportare la riga per ogni sessione.
4. `grep '^FASE' out/sessioni.log | grep -m1 ' 14 15'` → una riga con frame F, 7300 < F ≤ 7400 nella sessione `gameover` (misurato con passo 100; con il passo 1 il frame esatto è attesa, non misurata).
5. `ls out/*trace*.txt 2>/dev/null` → vuoto (tracce cancellate).
6. `mame solomon $MFAST -autoboot_script sched_log.lua && grep -c '^D' out/sched_log.txt && awk '$1=="D" && $2<=1900' out/sched_log.txt | wc -l && grep -c '^W' out/sched_log.txt && grep -c '^W .* 04AB ' out/sched_log.txt` → `106124`, `30725`, `1298`, `110` (misurati con tap equivalenti; ±21 sulle spedizioni e ±1 sulle scritture se il frame finale è contato diversamente; il formato del campo pc deve far funzionare l'ultimo grep o un equivalente dichiarato).
7. `head -3 out/sched_log.txt` → le prime righe sono `W` con pc 2C1B e valore 80 (test RAM) (misurato: 512 scritture di 2C1B precedono ogni spedizione).
8. `make -C harness && ./harness/sched_proto out/sched_log.txt` → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 110, slot coincidenti: 110`, codice di uscita 0 (attesa, non misurata). Anche `./harness/sched_proto out/sched_log.txt 1900` → `spedizioni confrontate: 30725, differenze: 0` (attesa, non misurata).
9. `./harness/harness 0x05B3 && ./harness/harness 0x2402 && ./harness/harness 0x326C` → tre `OK` (regressione del brief 03; attesa per 0x326C).
10. `test -s SCHEDULER.md && grep -c -E '0493|3AF3|c04f|c04e|spedisci_una' SCHEDULER.md` → ≥ 5.
11. `cat out/*_e603.txt | awk '{print $1}' | sort -u` → `0161` soltanto, oppure anche `4CF0`/`62C1` con i valori riportati nel rapporto e `harness/e603.h` aggiornato (attesa, non misurata).
12. `find . -name '*.c' -o -name '*.h' -o -name '*.py' -o -name '*.lua' -o -name '*.sh' | grep -v rst20_tables.h | xargs wc -l | awk '$1>800 && $2!="totale"'` → vuoto.
13. `cd /home/rufio72/OpenSolomonsKey-master && find . -path ./decomp -prune -o -newer decomp/.inizio -type f -print` → vuoto.
14. `cd /home/rufio72/OpenSolomonsKey-master && git status --short | grep -v ' decomp/'` → vuoto.

## Consegna

- File nuovi e modificati solo in `decomp/`; `out/` con i derivati delle
  sessioni (`*_eseguiti.txt`, `*_numeri.txt`, `sched_log.txt`,
  `eseguiti_unione.txt`, `call_mai_osservate.txt`), nessuna traccia.
- Nessun commit (lo fa la chat principale); se richiesto, messaggio in
  italiano con riga finale `Co-Authored-By: Claude Opus <noreply@anthropic.com>`
  (o il modello usato).
- Rapporto finale: massimo trenta righe con l'uscita delle verifiche 1-14,
  la copertura per sessione e in unione, le prime cinque voci di
  `call_mai_osservate.txt`, il frame del game over, le istruzioni per la
  partita umana (punto 5), le deviazioni dal brief e il tempo MAME speso.

## Domande aperte

1. «scelta di progetto» Modello dello scheduler: risolta dall'utente,
   niente coroutine; una funzione C per codice di attività chiamata dal
   ciclo sui 64 slot (vedi struttura, punti 7-11).
2. «scelta di progetto» Sessioni `bonus` e `finale`: fuori perimetro,
   rinviate; decisione dell'utente in sospeso; non entrano nelle verifiche
   né nei cancelli.
3. «preferenza» Finestra di traccia per stanza: 1500-2200 (700 frame, circa
   2 minuti): copre caricamento, inizio e 600 frame di input casuali.
   Alternativa 1500-2500 (3 minuti, 51 stanze ≈ 2,5 ore). Proposta 1500-2200.
4. «preferenza» Partita umana con `-record/-playback`: proposta di
   limitarsi a scrivere le istruzioni nel rapporto, senza eseguirla, e di
   predisporre `sessione.lua` a girare con `-playback`.
5. «preferenza» Le attività stub nel prototipo applicano le scritture del
   log prese dalla misura: il confronto prova lo spedidore e `posta_attivita`,
   non il contenuto delle attività; proposta accettarlo come perimetro di
   questo brief (le attività vere arrivano con le traduzioni).

## Decisioni (chat principale, 2026-10-19)

- Domanda 3 (preferenza): accettata, finestra 1500-2200.
- Domanda 4 (preferenza): accettata, solo istruzioni per la partita umana, `sessione.lua` predisposto per `-playback`.
- Domanda 5 (preferenza): accettata, stub che applicano le scritture del log.
- Domanda 1 (scelta di progetto, utente): niente coroutine; una funzione C per codice di attività, ciclo sui 64 slot uno a uno con 3AF3 (stesso ordine, stessa posta 0493, stesso indice c04f), stato in RAM; `coro.*` tolti, `sched_proto` con ciclo e stub, verifica sulla sequenza per frame invariata.
- Domanda 2 (scelta di progetto, utente): sessioni `bonus` e `finale` fuori perimetro, rinviate; cancello di copertura senza di esse (atteso invariato: N > 129, obiettivo ≥ 270, attesa, non misurata).
