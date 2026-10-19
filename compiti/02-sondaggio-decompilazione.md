# Compito 02: sondaggio di fattibilità della decompilazione del Z80 principale

Data: 2026-10-19. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

**Ordine di esecuzione: questo brief va eseguito DOPO il brief 01
(`compiti/01-asset-da-rom.md`), mai in parallelo.** Il brief 01 lavora su
`res/`, `tools/`, `src/`; questo lavora solo in `decomp/`.

## Obiettivo

Misurare, senza decompilare il gioco, se conviene decompilare a mano in C
leggibile il programma del Z80 principale di Solomon's Key arcade (set USA
`solomon`). Prodotto: una cartella nuova `decomp/` con disassembly, copertura
di esecuzione reale presa da MAME, grafo delle chiamate, una prova di metodo
(due routine tradotte in C e verificate byte per byte contro MAME) e il
rapporto `decomp/RAPPORTO.md` con numeri e giudizio.

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/` (da creare). Non toccare `compiti/`,
  `roms/`, `res/`, `tools/`, `src/`. I dump e gli script in
  `roms/decoded/mame/` si leggono, non si cambiano.
- Niente da installare: `z80dasm` 1.1.6 è già installato (`/usr/bin/z80dasm`),
  `mame` 0.264 in `/usr/games/mame`, `gcc` e `python3` nel PATH. Non usare
  `-debugger imgui` (va in segfault, verificato).
- Commenti, messaggi, nomi di file e documenti in italiano.
- Nessun file di codice sopra le 800 righe (`.c`, `.h`, `.py`, `.lua`, `.sh`).
  Il disassembly generato (`decomp/out/*.dasm`) non è codice scritto a mano e
  non conta; i file di uscita grandi stanno in `decomp/out/`.
- Il file di traccia MAME (centinaia di MB) va cancellato dopo l'elaborazione;
  in `decomp/out/` restano solo gli elenchi derivati.
- Non modificare `roms/decoded/mame/dump2.lua`: copiarne le parti utili.
- Il sondaggio dura poche ore di lavoro agente: non tradurre più di tre routine,
  non annotare il disassembly a mano.

## Cosa esiste già (fatti verificati)

### Mappa di memoria del Z80 principale (misurata via Lua `space.map.entries`, MAME 0.264)

| Intervallo | Tipo | Nome MAME | Note |
|---|---|---|---|
| 0000-bfff | rom | region `:maincpu` | 6.3f a 0000-3fff; 7.3h seconda metà a 4000-7fff, prima metà a 8000-bfff |
| c000-cfff | ram | | RAM di lavoro, azzerata all'avvio dal codice a 0x2858-0x286c |
| d000-d3ff | ram / w delegate | `colorram_w<0>`, share `colorram1` | |
| d400-d7ff | ram / w delegate | `videoram_w<0>`, share `videoram1` | |
| d800-dbff | ram / w delegate | `colorram_w<1>`, share `colorram2` | |
| dc00-dfff | ram / w delegate | `videoram_w<1>`, share `videoram2` | |
| e000-e07f | ram | share `spriteram` | |
| e400-e5ff | ram / w delegate | `palette_device::write8`, share `palette` | |
| e600 | r port `P1` / w `nmi_mask_w` | | scrittura: 0 disabilita NMI, 1 abilita |
| e601 | r port `P2` | | |
| e602 | r port `SYSTEM` | | moneta e start |
| e603 | r delegate `solomon_state::_0xe603_r` | | handler dipendente dal PC (protezione); letto da 015E, 4CED, 62BE; letto da Lua fuori da quei PC vale 0 |
| e604 | r port `DSW1` / w `flipscreen_w` | | scritture eseguite a 28F1, 38DC, 760C, 7CB0 |
| e605 | r port `DSW2` | | |
| e606 | r nop | | letto a 019A |
| e800 | w `sh_command_w` | | latch audio; scritto a 01A4 (dentro la NMI) e 2ACB |
| f000-ffff | rom | region `:maincpu` | **primi 4096 byte di 8.3jk** (verificato: uguali a `ram_title.bin[0x3000:]`); mai eseguito in 1900 frame; contiene tabelle di puntatori f1e8, f22a, f2ac... |

Z80 audio: `:audiocpu`, 1.3jk 16384 byte a 0000-3fff, RAM 4000-47ff, latch a 8000; fuori dal sondaggio, da citare nel rapporto solo con la dimensione.

Non esiste istruzione `im` nel disassembly: l'unico interrupt è la NMI (vettore 0x0066), abilitata da `ld (e600),a` con a=1.

### Immagini ROM (misurate)

- Chip in `/home/rufio72/OpenSolomonsKey-master/roms/ext/solomon/`: 6.3f 16384 byte (md5 82b258c78adf5bb852b7d1874532cf19), 7.3h 32768 (444ebb46c7a2e46f0fa7b99ca4cfee90), 8.3jk 32768, 1.3jk 16384 (43b065c0ca0bae238fec504ccfee55c0).
- Immagine 48 KB `main48.bin` = 6.3f + 7.3h[16384:] + 7.3h[:16384]: md5 `daaf50c3682c824a746c0a75c566119a`.
- Immagine 64 KB `main64.bin` = main48 + 12288 byte zero + 8.3jk[:4096]: md5 `0bca296adf15411c009cf0d13718400d`.
- `z80dasm -a -l -g 0 main48.bin` produce 35732 righe, avvisi «Code might not be 8080 compatible» e «Self modifying code detected» (falso positivo: scrive su indirizzi che z80dasm crede codice ma sono RAM). Formato: `\tld (0e600h),a\t\t;0001` con etichette `lXXXXh:` e `sub_XXXXh:`. Il disassembly statico scambia dati per codice: contiene `call 0d513h`, `jp 0c0e1h`, `call nc,0d4c1h` (RAM video) e 14 `jp (hl)` a 0d17, 4f32, 7b19, 862d, 966d-9693 (8), a593, a5b7, di cui **nessuno eseguito** in 1900 frame.

### Punti fissi del programma (da disassembly e traccia)

- Reset 0x0000: `xor a; ld (e600),a; jp 2833` (init). A 0x2858-0x286c: azzera c000-cfff, `ld sp,d000`.
- NMI: 0x0066 `push af; exx; ld (c040),ix; ld (c042),iy; ld hl,c044; ld a,(hl); bit 2,a ...`; unico `retn` eseguito a **0x01C3**. Dentro la NMI: `ldir` a 0089, 0093, 00A1, 00AC, 00B7, 00C2, 01AD (copie verso spriteram/palette), lettura e603 a 015E, scrittura e800 a 01A4, `ld (e600),a` a 01BB e 01BF.
- Ciclo principale (PC più frequenti in 1200 frame, 530k esecuzioni): **0x3AFA** `or (hl); jr nz,$3B02; inc l; res 6,l; jr $3AFA` (scansione di una tabella di attività in RAM).
- Vettori RST (verificati sul disassembly): 0008 `ld e,a; ld d,0; add hl,de; add hl,de; ret` (HL += 2*A); 0010 `ld e,(hl); inc hl; ld d,(hl); inc hl; ret` (DE = parola a HL); 0018 `rst 10h; ex de,hl; ret` (HL = parola a HL); 0020 `ex (sp),hl; push de; rst 8; rst 18h; pop de; ex (sp),hl; ...` = **salto su tabella inline che segue il `rst 20h`** (dati dentro al codice: pattern da contare). Eseguiti in traccia: rst $08, $10, $18 (es. a 0018, 0022, 0023, 2B10-2B18) e rst $28 a 0541.
- Nessun PC ≥ 0xc000 in 1900 frame: non c'è codice copiato in RAM ed eseguito da lì.

### Traccia MAME (misurata)

Strada verificata: `-debug -debugger none` più script Lua `-autoboot_script` che manda comandi al debugger con `manager.machine.debugger:command(...)`. In Lua con `-debug`, `cpu.debug` è `sol.device_debug*` e `manager.machine.debugger` è `sol.debugger_manager*` (senza `-debug` entrambi nil). Funziona anche `-debugger qt -debugscript file` (file con `trace t.txt,0,noloop` e `go`), ma la strada Lua non apre finestre. `noloop` = traccia completa senza compattare i cicli. Formato riga: `0000: xor  a`, `01A4: ld   ($E800),a`, `0018: rst  $10`, `0066: push af`, `2B1C: ret  z`, mnemonici minuscoli, operandi esadecimali con `$`, maiuscoli.

Sessione di 1900 frame (avvio, demo, moneta a 1510-1530, start a 1560-1580, livello 1), opzioni `-rompath /home/rufio72/OpenSolomonsKey-master/roms -video soft -sound none -window -nomaximize -skip_gameinfo -nothrottle`, `DISPLAY=:1`:
- 15.610.624 righe, 244.091.593 byte, 34 s reali (velocità 95%).
- **4490 PC distinti**, **115 destinazioni di CALL/RST distinte**, 0 PC ≥ c000, 0 `jp (hl)` eseguiti, `ldir` eseguiti a 25 indirizzi (0089 ... 43DF).
- 600 frame (solo titolo): 5.359.895 righe, 26 destinazioni di CALL; 1200 frame: 2090 PC distinti.
- Campi delle porte (nomi esatti per `field:set_value`): `:P1` → `P1 Up`, `P1 Down`, `P1 Left`, `P1 Right`, `P1 Button 1`, `P1 Button 2`; `:SYSTEM` → `Coin 1`, `Coin 2`, `1 Player Start`, `2 Players Start`. Accesso: `manager.machine.ioport.ports[":P1"].fields["P1 Right"]`.
- L'emulazione è deterministica con gli stessi input: le misure sotto si ripetono identiche.

### Stato prima/dopo di una routine (meccanismo verificato)

Breakpoint all'ingresso; allo stop si leggono i registri (`cpu.state["PC"].value`, nomi `PC AF BC DE HL IX IY SP`), si scarica c000-ffff con `mem:read_u8`, si legge l'indirizzo di ritorno `ret = mem:read_u16(SP)` e si mette un breakpoint `bpset ret,sp==SP+2` che scatta solo al ritorno di quell'invocazione. Lo stop si rileva in `emu.register_periodic` con `dbg.execution_state == "stop"`; `os.getenv` funziona nel Lua di MAME. Misure (prima occorrenza):

```
SNAP_ENTRY=0x05B3: before PC=05B3 AF=2008 BC=0000 DE=0030 HL=C8EE IX=C8EE IY=FFFF SP=DBFA frame=66
                   ritorno a 0136 con sp==DBFC
                   after  PC=0136 AF=004C BC=0001 DE=0030 HL=C94E IX=C8EE IY=FFFF SP=DBFC
                   byte diversi fra prima e dopo (c000-ffff): 96
SNAP_ENTRY=0x2402: before PC=2402 AF=1054 BC=0001 DE=0000 HL=E400 IX=0000 IY=FFFF SP=CFFA frame=151
                   ritorno a 36C0 con sp==CFFC
                   after  PC=36C0 AF=0F1A BC=0001 DE=0001 HL=E402 IX=0000 IY=FFFF SP=CFFC
                   byte diversi: 0 (scrive 0000 su e400-e401 già a zero)
NMI 0x0066:        ritorno a 2867 con sp==DC00, byte diversi: 117
```

### Routine candidate per la prova di metodo (da traccia di 600 frame, foglia, senza porte)

- **0x05B3-0x0601** (`ret` a 0601; 05B3 `ld c,0`, ingresso alternativo 05B5): costruisce la tabella sprite a c8ee (0x18 voci da 4 byte = 96 byte) leggendo gli oggetti a c46e (passo 0x30, campi +00, +0e, +15, +27, +28) e il flag `(c045) & 0x20`; usa `jr`, `jp`, `djnz`, `add ix,de`. Chiamata 535 volte in 600 frame. Pura RAM→RAM: **prima routine obbligatoria**.
- **0x2402-0x240A**: `ld (hl),e; inc hl; ld (hl),d; inc hl; ex de,hl; add hl,bc; ex de,hl; dec a; ret` (scrive una parola a HL, DE += BC). Chiamata 64 volte. Seconda routine, a livello di registri.
- 0x0008, 0x0010, 0x0018 (vettori RST, 5 istruzioni) e 0x240B-0x2417 (HL = A*48): alternative a livello di registri.
- Per una terza routine da gioco (facoltativa) usare il classificatore descritto sotto sulla traccia completa.

## Struttura da realizzare

Tutto in `/home/rufio72/OpenSolomonsKey-master/decomp/`:

1. `build_rom.sh` — crea `out/main48.bin` e `out/main64.bin` con `cat`, `dd bs=16384 skip=1 count=1`, `head -c`; stampa gli md5.
2. `disasm.sh` — `z80dasm -a -l -g 0 out/main48.bin > out/main48.dasm` e, facoltativo, main64.
3. `trace.lua` — come `roms/decoded/mame/dump2.lua` per moneta/start (stessi frame 1510/1530/1560/1580) più: `dbg:command("trace out/trace.txt,0,noloop")` e `dbg:command("go")` all'avvio; dal frame 1900 al 5500 input casuali con `math.randomseed(1)`: ogni 8 frame scegliere un campo fra `P1 Left`, `P1 Right`, `P1 Up`, `P1 Down`, `P1 Button 1`, `P1 Button 2`, tenerlo premuto 8 frame; al frame finale `dbg:command("trace off")` e `manager.machine:exit()`; il frame finale è la variabile d'ambiente `TRACE_FRAMES` (default 5500; con 1900 niente input casuali). Lanciare da `decomp/` (i percorsi Lua sono relativi alla cartella corrente). Stima: ~700 MB, ~100 s.
4. `coverage.py` — legge `out/trace.txt` in streaming (mai `read()` intero), estrae i PC distinti (`^([0-9A-F]{4}): (.*)$`) in `out/eseguiti.txt` (un indirizzo esadecimale per riga, ordinati) e cancella poi la traccia (o la cancella lo sviluppatore). Decodifica la lunghezza di ogni istruzione eseguita leggendo `out/main64.bin`:
   - senza prefisso: 2 byte per 06,0E,16,1E,26,2E,36,3E, C6,CE,D6,DE,E6,EE,F6,FE, D3, DB, 10, 18,20,28,30,38; 3 byte per 01,11,21,31, 22,2A,32,3A, C3, C2,CA,D2,DA,E2,EA,F2,FA, CD, C4,CC,D4,DC,E4,EC,F4,FC; 1 byte il resto;
   - CB → 2; ED → 2, tranne ED 43,4B,53,5B,63,6B,73,7B → 4;
   - DD/FD: se segue CB → 4; altrimenti lunghezza del secondo byte come senza prefisso + 1, e ancora +1 (spiazzamento) se il secondo byte è uno di 34,35,36, 46,4E,56,5E,66,6E,7E, 70-75,77, 86,8E,96,9E,A6,AE,B6,BE.
   - Autoverifica: per ogni coppia di righe consecutive della traccia la cui prima istruzione non è `jp/jr/call/ret/rst/djnz/retn`, deve valere `PC2 == PC1 + len(PC1)`; contare le violazioni (attese 0).
   - Uscite in `out/copertura.txt`: intervalli contigui `inizio-fine codice|dati` su 0000-bfff e f000-ffff; e `out/numeri.txt` con: byte eseguiti, byte mai eseguiti (su 53248 byte di ROM), PC distinti, numero di destinazioni di `call`/`rst` distinte (PC della riga successiva a un `call` o `rst` eseguito), numero di `rst $20` eseguiti (tabelle inline), numero di `jp (hl)` eseguiti, PC ≥ c000, tabelle dati stimate = operandi immediati a 16 bit distinti (`ld hl/de/bc/ix/iy,$nnnn` e `ld a,($nnnn)` ecc.) in 0000-bfff/f000-ffff che cadono su byte mai eseguiti.
5. `callgraph.py` — da `out/eseguiti.txt` più una seconda passata sulla traccia (o dalla stessa passata di coverage.py, a scelta): per ogni destinazione di call/rst: estensione = istruzioni eseguite da lì fino alla destinazione successiva in ordine di indirizzo; colonne CSV `entry,byte,invocazioni,chiamati,chiamanti,aree,indiretto,brutta`. `aree` = insieme fra `ram` (c000-cfff), `video` (d000-dfff), `sprite` (e000-e07f), `palette` (e400-e5ff), `porte` (e600-e606), `audio` (e800) dagli operandi assoluti; `indiretto` = 1 se scrive via `(hl)/(ix+d)/(iy+d)/(de)/(bc)/ldir`; `brutta` = motivo fra `jp(hl)`, `rst20`, `ram-exec`, `porte` o vuoto. Uscita `out/callgraph.csv`.
6. `snap.lua` — il meccanismo prima/dopo: variabili d'ambiente `SNAP_ENTRY` (es. `0x05B3`), `SNAP_N` (occorrenza da catturare, default 1), `SNAP_DIR` (default `out`). Scrive `out/snap_<ENTRY>_prima.bin`, `_dopo.bin` (16384 byte c000-ffff) e `_regs.txt` con due righe `prima PC=.. AF=.. BC=.. DE=.. HL=.. IX=.. IY=.. SP=..` e `dopo ...`. Durante la fase «dopo» tenere anche un breakpoint a 0066: se scatta prima del ritorno, la cattura è contaminata dalla NMI; scartarla e passare all'occorrenza successiva (riportare quante scartate). Uscire con `manager.machine:exit()` dopo la cattura; limite di sicurezza 6000 frame.
7. `harness/` — `ram.h` (`extern uint8_t ram[0x4000]; #define M(a) ram[(a) - 0xc000]`, `struct z80 { uint8_t a,f,b,c,d,e,h,l; uint16_t ix,iy,sp; }` più macro `HL(z)` ecc.), `routine.c` (`void sub_05b3(struct z80 *z)`, `void sub_2402(struct z80 *z)`, eventuale terza), `harness.c` (`./harness <ENTRY>`: carica `out/snap_<ENTRY>_prima.bin` e i registri «prima», chiama la routine scelta da una tabella nome→funzione, confronta `ram[0x0000..0x25ff]` (c000-e5ff: esclude le porte e la ROM f000) byte per byte con `_dopo.bin` stampando le prime 8 differenze come `XXXX: prima->c dopo`, confronta A, BC, DE, HL, IX, IY con la riga «dopo» (non F, non SP, non PC); stampa `OK <ENTRY>` o `FALLITO <ENTRY>` e ritorna 0/1), `Makefile` (`gcc -std=c11 -Wall -Wextra -O0`). Scrivere il C come traduzione leggibile (nomi delle variabili per gli offset: `obj->x`), non come emulatore.
8. `RAPPORTO.md` — numeri di `out/numeri.txt`, tabella delle routine «brutte» da callgraph.csv, stima delle routine normali (destinazioni senza `brutta`), esito del harness, costo stimato (routine normali × tempo per routine osservato sulla prova di metodo), giudizio: fattibile / fattibile con riserve / sconsigliato, con motivazione in dieci righe. Citare che 0xf000-0xffff è dati (8.3jk) e la dimensione del Z80 audio.

Libere: nomi delle funzioni Python, formato esatto di `copertura.txt`, uso di `argparse`.

## Verifiche obbligatorie

Eseguire da `/home/rufio72/OpenSolomonsKey-master/decomp/` con `export DISPLAY=:1` e `MOPT="-rompath /home/rufio72/OpenSolomonsKey-master/roms -video soft -sound none -window -nomaximize -skip_gameinfo -nothrottle"`. Prima di tutto `mkdir -p decomp && touch decomp/.inizio`.

1. `sh build_rom.sh && md5sum out/main48.bin out/main64.bin` → `daaf50c3682c824a746c0a75c566119a  out/main48.bin` e `0bca296adf15411c009cf0d13718400d  out/main64.bin` (misurati).
2. `sh disasm.sh && wc -l < out/main48.dasm` → `35732` (misurato).
3. `mame solomon $MOPT -debug -debugger none -autoboot_script trace.lua` → esce da solo; `ls -l out/trace.txt` esiste. Con `TRACE_FRAMES=1900` (sessione identica a quella misurata, senza input casuali) le misure sono `wc -l` = `15610624`, PC distinti `4490`, destinazioni call/rst `115` (misurati). Con 5500 frame e input casuali: righe > 15610624, PC distinti ≥ 4490, destinazioni ≥ 115 (attesa, non misurata).
4. `python3 coverage.py && cat out/numeri.txt` → violazioni dell'autoverifica di lunghezza = 0; PC ≥ c000 = 0; `jp (hl)` eseguiti = 0 (misurato a 1900 frame; con input casuali attesa, non misurata); byte eseguiti fra 9000 e 20000 (attesa, non misurata).
5. `python3 callgraph.py && wc -l < out/callgraph.csv` → numero di destinazioni + 1 di intestazione (attesa, non misurata); `grep -c ',porte' out/callgraph.csv` ≥ 1 (la NMI legge e603/e606).
6. `SNAP_ENTRY=0x05B3 mame solomon $MOPT -debug -debugger none -autoboot_script snap.lua && cat out/snap_0x05B3_regs.txt && cmp -l out/snap_0x05B3_prima.bin out/snap_0x05B3_dopo.bin | wc -l` → righe `prima PC=05B3 AF=2008 BC=0000 DE=0030 HL=C8EE IX=C8EE IY=FFFF SP=DBFA` e `dopo PC=0136 AF=004C BC=0001 DE=0030 HL=C94E IX=C8EE IY=FFFF SP=DBFC`, differenze `96` (misurato).
7. Idem con `SNAP_ENTRY=0x2402` → `prima PC=2402 AF=1054 BC=0001 DE=0000 HL=E400 ...`, `dopo PC=36C0 AF=0F1A BC=0001 DE=0001 HL=E402 ...`, differenze `0` (misurato). Poiché 0 differenze non prova nulla sulla RAM, per 0x2402 il harness deve verificare anche HL=E402 e DE=0001.
8. `make -C harness && ./harness/harness 0x05B3 && ./harness/harness 0x2402` → `OK 0x05B3`, `OK 0x2402`, codice di uscita 0 (attesa, non misurata). Per 0x05B3 controllare anche che i 96 byte a c8ee-c94d coincidano.
9. `test -s RAPPORTO.md && grep -cE 'fattibile|sconsigliato' RAPPORTO.md` → ≥ 1.
10. `find . -name '*.c' -o -name '*.h' -o -name '*.py' -o -name '*.lua' -o -name '*.sh' | xargs wc -l | awk '$1>800 && $2!="totale"'` → vuoto.
11. `cd /home/rufio72/OpenSolomonsKey-master && find . -path ./decomp -prune -o -newer decomp/.inizio -type f -print` → vuoto (nessuna modifica fuori da `decomp/`).
12. `ls -l decomp/out/trace.txt` → file assente (cancellato dopo l'elaborazione).

## Consegna

- Cartella `decomp/` come sopra; nessun commit (la cartella non è un repository git).
- Rapporto finale: massimo trenta righe con l'uscita delle verifiche 1-12, i numeri di `out/numeri.txt`, il giudizio di RAPPORTO.md in una riga e le deviazioni dal brief.

## Domande aperte

1. «preferenza» Confronto dei registri nel harness: proposta A, BC, DE, HL, IX, IY, escludendo F (le bandiere sono costose da riprodurre in C leggibile) e SP/PC.
2. «preferenza» Traccia con `noloop` (completa, ~700 MB per 5500 frame) oppure senza (compattata, cicli stretti segnati una volta): proposta `noloop`, perché il conteggio delle invocazioni nel grafo resta esatto; il disco ha 196 GB liberi.
3. «preferenza» Estensione di una routine nel grafo = fino alla destinazione di call successiva in ordine di indirizzo: sovrastima le routine con code condivise; proposta accettarla e segnalarla nel rapporto come limite.
4. «scelta di progetto» Terza routine della prova di metodo: proposta facoltativa, scelta dal classificatore fra le foglie senza porte della traccia completa con almeno 10 istruzioni e scritture su `ram`; se il tempo manca, bastano 0x05B3 e 0x2402.
5. «scelta di progetto» Durata della sessione con input casuali: proposta 5500 frame (un minuto di gioco dopo lo start); un valore più alto allarga la copertura ma non cambia il metodo.

## Decisioni (chat principale, 2026-10-19)

- Domanda 1 (preferenza): accettata la proposta, confronto di A, BC, DE, HL, IX, IY; esclusi F, SP, PC.
- Domanda 2 (preferenza): accettata la proposta, traccia con `noloop`.
- Domanda 3 (preferenza): accettata la proposta, estensione fino alla call successiva, limite segnalato nel rapporto.
- Domanda 4 (scelta di progetto, confermata dall'utente): terza routine facoltativa, scelta dal classificatore; bastano 0x05B3 e 0x2402 se il tempo manca.
- Domanda 5 (scelta di progetto, confermata dall'utente): sessione di 5500 frame.

## Chiusura ciclo (chat principale, 2026-10-19)

Esito: PASSATO con tre note del verificatore, nessuna imputabile al lavoro:
- Verifica 3: destinazioni call/rst 119 o 124 contro le 115 del brief; differenza di definizione del conteggio, documentata dallo sviluppatore.
- Verifica 4: 7815 byte eseguiti contro l'intervallo 9000-20000: il verificatore ha misurato sulla traccia da 1900 frame, l'intervallo del brief si riferisce ai 5500 frame (misurati 10183). La rilettura ha sovrascritto `out/numeri.txt` con i numeri a 1900 frame; i numeri a 5500 frame restano in RAPPORTO.md.
- Verifica 12: la traccia da 244 MB era stata rigenerata dal verificatore stesso; cancellata dalla chat principale.
Token: analista Fable 76k, sviluppatore Opus 74k, verificatore Sonnet 22k.
