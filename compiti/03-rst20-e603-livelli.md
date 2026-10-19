# Compito 03: tabelle `rst $20`, protezione `e603`, dati dei livelli

Data: 2026-10-19. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

Fase 1 della decompilazione in C del Z80 principale di Solomon's Key (set
`solomon`). Va eseguito DOPO il brief 02 (già chiuso: `decomp/` esiste con
gli script e `decomp/RAPPORTO.md`) e PRIMA del brief 04
(`compiti/04-copertura-scheduler.md`), che riusa i file prodotti qui.

## Obiettivo

Tre lavori meccanici sui dati dentro la ROM, ognuno con uscita in C o Python
e verifica contro MAME:
a) estrarre tutte le tabelle inline di `rst $20` (salto su tabella che segue
   l'istruzione) in `decomp/harness/rst20_tables.h` e come annotazioni nel
   disassembly, e tradurre in C la routine 0x326C (decodificatore del
   fondale, usa `rst $20`) verificandola nel banco di prova;
b) emettere la tabella della protezione `e603` (PC di lettura → valore) in C;
c) trovare formato e indirizzo dei dati delle 52 stanze e scrivere un
   decodificatore Python che ricostruisce i layer BG e FG di una stanza
   uguali ai dump di MAME. Questa parte serve al port in C del gioco anche
   indipendentemente dalla decompilazione: i dati delle stanze letti dalla
   ROM sostituiscono i `level_N.osk` scritti a mano.

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/`. Non toccare `src/`, `tools/`, `res/`,
  `roms/`, `compiti/`. I dump in `roms/decoded/mame/rooms/` si leggono.
- Niente da installare: `z80dasm` 1.1.6, `mame` 0.264 (`/usr/games/mame`, nel
  PATH come `mame`), `gcc` 13.3, `python3`. Mai `-debugger imgui` (segfault).
- Commenti, messaggi, nomi di file e documenti in italiano.
- Nessun file di codice sopra le 800 righe (`.c .h .py .lua .sh`); i file
  generati (`out/*.dasm`, `rst20_tables.h` se generato) non contano ma vanno
  comunque prodotti da uno script rieseguibile.
- Ogni script rieseguibile da `decomp/` (percorsi relativi a quella cartella).
- Tracce MAME (`out/trace.txt`, 700 MB a 5500 frame) cancellate a fine lavoro.
- `sudo` disponibile con `sudo -S <cmd> < ~/.claude/sudo.pass`, mai stampare
  la password; qui non dovrebbe servire.

## Cosa esiste già (fatti verificati)

### File in `decomp/` (da `wc -l` e lettura)

| file | righe | ruolo |
|---|---|---|
| `build_rom.sh` | 19 | `out/main48.bin` (0000-bfff) e `out/main64.bin` (+ f000-ffff da 8.3jk) |
| `disasm.sh` | 6 | `z80dasm -a -l -g 0` → `out/main48.dasm` (35732 righe), `out/main64.dasm` |
| `trace.lua` | 38 | traccia `noloop` con moneta 1510/1530, start 1560/1580, input casuali da 1900; `TRACE_FRAMES` |
| `coverage.py` | 225 | legge `out/trace.txt`; scrive `out/eseguiti.txt`, `out/istruzioni.txt` (`pc\tesecuzioni\tistruzione`), `out/chiamate.txt` (`sito\tdest\tn`), `out/copertura.txt`, `out/numeri.txt`; opzioni `--traccia --rom --cancella` |
| `callgraph.py` | 144 | `out/callgraph.csv` da istruzioni.txt e chiamate.txt |
| `snap.lua` | 98 | cattura prima/dopo di una routine: `SNAP_ENTRY`, `SNAP_N`, `SNAP_DIR`, `SNAP_DA_FRAME`; scarta le catture interrotte dalla NMI |
| `harness/ram.h` | 29 | `ram[0x4000]` = c000-ffff, `M(a)`, `struct z80 {a,f,b,c,d,e,h,l; ix,iy,sp}`, macro `BC/DE/HL`, `SET_*` |
| `harness/routine.c` | 67 | `sub_05b3`, `sub_2402` tradotte |
| `harness/harness.c` | 128 | `./harness <ENTRY>`: tabella `{"0x05B3", sub_05b3, blocco_ini, blocco_fine}`, confronta c000-e5ff e A,BC,DE,HL,IX,IY |
| `harness/Makefile` | 8 | `gcc -std=c11 -Wall -Wextra -O0` |

`out/` contiene oggi i derivati della traccia a **1900 frame** (il
verificatore del brief 02 li ha sovrascritti): `istruzioni.txt` 4490 PC,
`numeri.txt` con `rst $20 eseguiti: 48410`, `ingressi NMI 0066: 1835`.
`out/trace.txt` non esiste: va rigenerata (`TRACE_FRAMES=1900` ≈ 1 minuto
più l'elaborazione; 5500 frame circa 15 minuti). Le catture
`out/snap_0x05B3_*`, `out/snap_0x2402_*` esistono.

Formato della traccia (da brief 02): `3B10: rst  $20`, `0026: ret`, mnemonici
minuscoli, operandi `$XXXX` maiuscoli. Nota: in `grep` con virgolette doppie
`\$` diventa `$` (ancora di fine riga): usare `'rst  [$]20'`.

### MAME: velocità e tap (misurati)

- Senza debugger e con `-video none` MAME fa 5500 frame in 5,8 s (3192%).
  Riga: `mame solomon -rompath /home/rufio72/OpenSolomonsKey-master/roms -video none -sound none -skip_gameinfo -nothrottle -autoboot_script X.lua`
  con `DISPLAY=:1`. L'avviso «-video none doesn't make much sense» è innocuo.
- Con `-debug -debugger none` e traccia: circa 15 minuti per 5500 frame.
- L'emulazione è deterministica: i miei dump c000-ffff al frame 2000 con il
  tap su c243 sono identici byte per byte a `roms/decoded/mame/rooms/ram_room_1.bin`
  (ROOM=0) e `ram_room_2.bin` (ROOM=1): `cmp -l | wc -l` → `0` e `0`.
- **Tap**: `mem:install_read_tap(a, b, nome, function(offset, data, mask) ... end)`
  e `install_write_tap`; dentro il callback `cpu.state["PC"].value` è il PC
  **dopo** l'istruzione (per `ld a,($e603)` a 015E vale 0161; per `or (hl)` a
  3AFA vale 3AFB). L'handle restituito va tenuto in una **variabile globale**
  (`TAP1 = mem:install_read_tap(...)`): con una `local` del chunk il tap viene
  rimosso dal garbage collector dopo poche decine di frame (misurato: 59
  letture di e603 invece di 5435). `ldir` fa contare 2 letture per byte al tap.
- Forzare la stanza (brief 01): tap in lettura su c243 attivo dal frame 1500
  al 1950 che restituisce N (0-based): `TAPC = mem:install_read_tap(0xc243, 0xc243, "room", function(o, d, m) return N end)`;
  `TAPC:remove()` al 1950. Moneta `":SYSTEM"`/`"Coin 1"` 1510-1530, start
  `"1 Player Start"` 1560-1580. Il dump `ram_room_N.bin` è c000-ffff al frame
  2000 con ROOM=N-1 (51 file: 1-49, 51, 52).

### a) `rst $20`: semantica e siti

Gestore a 0x0020, 7 byte (da `out/main48.dasm`):
```
0020 ex (sp),hl   ; HL = indirizzo di ritorno = inizio tabella
0021 push de
0022 rst 8        ; 0008: ld e,a; ld d,0; add hl,de; add hl,de; ret  → HL += 2*A
0023 rst 18h      ; 0018: rst 10h; ex de,hl; ret   (0010: DE = parola a HL)  → HL = tabella[A]
0024 pop de
0025 ex (sp),hl   ; HL originale ripristinato, destinazione sullo stack
0026 ret          ; salto a tabella[A]
```
Effetto: `jp tabella[A]` con tabella di parole little-endian subito dopo
l'istruzione; A, BC, DE, HL conservati (le bandiere no). Nella traccia la
destinazione è il PC della riga che segue `0026: ret` dopo `SITO: rst  $20`.

Siti `rst 20h` nel disassembly statico 0000-bfff (25, da grep `^\trst 20h`):
`1833 2b4f 2c77 2cd2 2db8 2dd9 3335 3b10 3b1d 3b68 3b7c 3f45 4170 4644 62b9 646b 6485 649b 6f1c 75eb 7b1f 7cec 8c93 a497 a4a8`.
Eseguiti (traccia 1900 e 5500 frame, da `istruzioni.txt` e callgraph): **7**:
`2CD2 2DB8 2DD9 3335 3B10 75EB 7CEC`. `3B1D` e `3B7C` cadono dentro tabelle
(falsi positivi); `8C93 A497 A4A8` sono in 80a5-bfff (dati). Byte E7 nelle
zone marcate codice in `copertura.txt` (1900 frame): 8, cioè i 7 eseguiti più
`791C` (non eseguito).

Dimensione delle tabelle. Euristica misurata: leggere parole da sito+1
finché la parola sta in 0000-bfff e sito+1+2k è minore della più piccola
destinazione maggiore del sito; poi applicare il limite dato dalle
istruzioni che precedono il `rst`: `cp n; jr nc` → n voci, `and m` → m+1
voci. Risultati (euristica / con limite):
- 2CD2 (in 2CCD: `ld hl,c19d; ld a,(hl); inc (hl); rst 20h`, contatore di passo senza limite): 26 voci `3861 34FA 2DEE 37D7 358F 2D07 3225 3A7D 2E89 2F2D 2E5D 2DFA 2DAD 2DCE 31C5 2E0A 2D9D 2E9C 3709 3829 321E 2E9C 2EF5 2E6D 2F1A 2E1E`.
- 2DB8 (`cp 006h; jr nc` a 2DB4): 8 / **6** `2FC1 2E79 2FA9 2E71 2F8F 2E7D`.
- 2DD9 (`cp 007h` a 2DD5): 9 / **7** `3009 305F 3131 2E65 3161 2E81 31B7`.
- 3335 (`and 003h; jp nz,332d` a 3292, poi `dec a`): **3** `333C 335A 336A`.
- 3B10 (spedidore delle attività, A = codice 1..39): **40** `3B61 48D1 4B84 4BC6 4C30 4C42 75E7 2CCD 7C98 4D29 4DA2 2C27 7BF6 5C8E 63A4 640E 457B 456E 426A 6480 44B5 6466 6496 6429 4124 3792 411D 4111 3F40 3F39 3B61 43C8 3E90 3B61 12D6 3E4E 2C72 3B77 3B63 62B6` (3B61 = `pop hl; ret`, voce vuota).
- 75EB (`ld hl,c1ae; ld a,(hl); rst 20h`): 12 `78E0 763E 760B 760B 760B 7A0B 763E 760B 7B3B 763E 760B 7604` (7604 è l'ultima voce e il codice riprende lì).
- 7CEC (`ld a,(e605); rlca×3; and 007h`): 9 / **8** `7D6A 7DC6 7DA0 7DDE 7D85 7DD2 7DB3 3B62`.
- 62B9 (`ld a,(c1ae); rst 20h`, non eseguito): 2 `62E0 62BE` (62BE è codice: legge e603).
- 3B68 (`ld hl,c19d; ld a,(hl); inc (hl); rst 20h`, attività 3B63, non eseguita).
Il numero di voci osservato dalla traccia (indice massimo raggiunto) è un
limite inferiore; dove le voci si ripetono (760B) la traccia non distingue
l'indice.

### a) Routine 0x326C da tradurre (decodificatore del fondale)

Da `out/main48.dasm` 326C-33A9 e dalle misure con tap. Chiamata con `call`
da 3241 (4 volte in 1900 frame) e 3268 (8), estensione 326C-3353 nel
callgraph (231 byte, aree `ram`), più le callee `23BB` (12 chiamate da
328B), `336C` (960 da 32F4, scrive i tile), `23EC` (960 da 3374, HL =
D*64). Ingresso: DE = puntatore al flusso in ROM, BC = parametro (B&1f e
C nibble: posizione iniziale), lavora su c082-c089. Ciclo a 328F: `ld a,(de);
inc de; ld c,a; and 3; jp nz,332d`; con (c&3)≠0 spinge 328F come ritorno e
fa `rst 20h` su (c&3)-1 → 333C (attributi in c087/c088 da C), 335A (segna
ripetizione: `pop hl; push de; push hl; ret` lascia DE sotto l'indirizzo di
ritorno), 336A (`pop hl; ret` = fine flusso, ritorna al chiamante di 326C).
Con (c&3)=0 è un blocco: B = c&fc, aggiorna il cursore in c082-c084, `call
336C`: se D>11 o E>14 non disegna, altrimenti HL = dc80 + D*64 + 2*E + 1 e
scrive 4 tile (2x2) in videoram BG (`ld (hl),a`) e colorram BG (`res 2,h;
ld (hl),c; set 2,h`) con codici `base + (0..3) ^ ribaltamenti`. Scritture
misurate per stanza: 960 a 3397 (D881-DB7E) e 960 a 339A (DC81-DF7E), cioè
righe 4-27, colonne 1-30 di entrambi i piani BG.

Cattura per il harness: `SNAP_ENTRY=0x326C SNAP_N=1` (prima chiamata, al
frame 1563 dopo lo start); `snap.lua` scarica c000-ffff, quindi il flusso
in f000-ffff è dentro `snap_0x326C_prima.bin` e il C lo legge con `M(de)`.
Le callee 23BB/23CB/23EC non leggono ROM sotto c000 (da disassembly).
Uscita attesa dei registri: `attesa, non misurata` (misurarla con snap.lua
prima di tradurre e riportarla nel rapporto).

### b) Protezione `e603` (misurata con tap, 5500 frame, input casuali di trace.lua)

- Handler MAME `solomon_state::_0xe603_r`, valore dipendente dal PC. Letture
  nella ROM (da `grep '($E603)'` su istruzioni.txt e dasm): **015E** (nella
  NMI: `ld a,(e603); rrca; jr nc,0183`: con bit 0 = 1 la NMI posta
  l'attività 0x27 → 62B6), **4CED** (`ld a,(c1b9); ld c,a; ld a,(e603); xor
  c; and 8; jr nz,4d00`), **62BE** (attività 62B6, raggiunta solo via
  0x27, cioè solo se 015E legge bit 0 = 1).
- Tap su e603 per 5500 frame: una sola chiave, `0161:00` con 5435 letture
  (PC del tap = 015E+3). 4CED e 62BE **non** letti in questa sessione; 62BE
  non è raggiungibile finché 015E legge 0.
- Letto da Lua fuori da quei PC il valore è 0 (brief 02).

### c) Dati delle stanze (misurati con tap sulle letture ROM 8000-bfff/f000-ffff e scritture d000-dfff, ROOM=0 e ROOM=1, frame 1560-2000)

Indice di stanza: `c243` (0-based). Fondale (BG), tutto in f000-ffff
(primi 4096 byte di 8.3jk, in `out/main64.bin` agli stessi indirizzi):
- 33BA: `ld a,(c243); ld hl,f000; rst 8; rst 18h` → **F000 = tabella di 52
  parole**, una per stanza, verso un record palette: byte di testa `h`, poi
  `2*(h&0x7f)` byte copiati a c9ca (`ldir` a 33D2); se bit 7 di `h` è 1 i
  byte sono nibble espansi con `rld` (33D8-33E7). La NMI copia c9ca → e500
  (banchi palette 8-15) quando c044 bit 6 è 1 (33B4 `set 6,(hl)`; NMI
  008B-0093). Prime parole a F000: `F30F F1E8 F22A F3D3 F22A F2AC F2EE ...`
  (F30F: h=20 → 64 byte; F1E8: h=10 → 32 byte). Letture misurate: ROOM=0
  F30F e F310-F34F; ROOM=1 F1E8 e F1E9-F208.
- 3231: `ld a,(c243); add a,a; ld hl,f068; rst 8; rst 10h; push de; rst 18h;
  ex (sp),hl; ex de,hl; ld bc,1000h; call 326C; pop hl` → **F068 = tabella di
  52 voci da 4 byte**: parola 1 = puntatore al flusso base, parola 2 = H:L
  indici di due sovrapposizioni facoltative. Se H≠0 `call 325E` con A=H, se
  L≠0 con A=L: 325E fa `dec a; add a,a; ld hl,f138; rst 8; rst 10h; ld c,(hl);
  inc hl; ld b,(hl); call 326C` → **F138 = tabella di voci da 4 byte**
  (puntatore al flusso, BC iniziale), indice A-1. Poi `ld c,1; call 33BA`.
  Voci misurate: ROOM=0 → `F4C3,0201` (flussi F4C3 letto 270 byte, poi
  F13C→F7A3 con BC=042C, poi F138→? con BC=0420); ROOM=1 → `F415,0003`
  (F415 220 byte, poi F140→F6B2 con BC=069C). Byte a F13A: `20 04 A3 F7 2C 04
  27 F6 08 05 A3 F7 8C 04 B2 F6 9C 06 B2 F6 A2 06 15 F8`.
  I puntatori si ripetono fra stanze (F4C3 usato da 4 stanze, F415 da 14):
  stanze con lo stesso fondale (brief 01: {6,16}, {13,22,32}, ...).
- Chiamanti di 33BA: 3220, 325A, 7906, 7CD8; di 326C: 3241, 3268 (da chiamate.txt).

Primo piano (FG, blocchi e oggetti): **formato non ancora individuato**;
fatti misurati per arrivarci:
- Record per stanza a **8EA4**: tabella di parole `8F0C 8F29 8F40 8F53 8F5E
  8F7B 8F9E 8FBF ...` (contigui, lunghezza variabile, terminatore FF),
  letto al frame 1563 da 359D-3651 (`ld hl,8EA4` a 3592). Stanza 0:
  `28 18 18 A5 AB 2F 39 2F 47 10 | 18 53 57 15 59 15 63 14 6D 14 58 18 67 AA 69 AA 68 AB FF`
  (10 byte di testa, poi coppie lette a 3646/3651). Stanza 1: `9E 22 24 92
  3E 2F 39 2B 47 10 | 38 18 69 95 B6 6B 86 9A B8 48 BA 6B FF`.
- I blocchi FG sono disegnati al frame 1665 (105 frame dopo lo start) da
  3A5E/3A61/3A67/3A6A (2x2 tile, 43 blocchi per ROOM=0, 35 per ROOM=1),
  chiamati dal motore oggetti 0602-06A7 (catena di ritorno sullo stack:
  060F, 2F55 = passo 2F2D dell'attività 07, slot C00C). Il motore legge per
  ogni oggetto le tabelle `A746` (parola per indice → record di 3 byte a
  A8A0+), `AEE3` (record di 9 byte, offset +0..+8 letti a 065E-06A0),
  `9555` (parola → record di 3 byte a 96AF+); 43 iterazioni per ROOM=0, 34
  per ROOM=1. Le prime letture dopo il frame 1655 sono identiche per le
  due stanze (animazioni), quindi la lista degli oggetti della stanza è
  già in RAM a 1655: va cercata con un tap in scrittura sulla tabella
  oggetti (c46e, 24 voci da 0x30; c82e) fra i frame 1560 e 1665, oppure
  con un tap in scrittura su d480-d77f che registri registri e catena di
  ritorno escludendo PC 2F64 (96 scritture di azzeramento di D5C0-D61F).
- Altre letture ROM al caricamento (ROOM=0 | ROOM=1): 3573 ldir 80B3-80F2
  (stato iniziale, 32+32 byte, uguale per tutte); 3516 8DD4-8DD7 | 8DD8-8DDB;
  38FC 82D3; 3937/393F 85EC-85ED; 3A51/3A53 857A-85AF (coppie di tile dei
  blocchi, `ld hl,8578` a 3A45, dal frame 1854); 4729 9484-9494 (solo ROOM=0,
  frame 1853); 0746 ldir 9495-9550 (4 byte per voce, `ld bc,9495` a 073D,
  7248 letture: animazioni sprite).
- Formato RAM dei piani (brief 01): FG colorram d000 / videoram d400, BG
  colorram d800 / videoram dc00, 1 KB ciascuna, indice = riga*32+colonna;
  attributo: bit 0-2 bit alti del codice, bit 4-6 banco colore, bit 7 flip
  X, bit 3 flip Y. Palette e400-e5ff (BG = voci 128-255).

## Struttura da realizzare

Tutto in `/home/rufio72/OpenSolomonsKey-master/decomp/`.

1. `rst20.py` (nuovo): legge `out/main64.bin`, `out/main48.dasm` e, se esiste,
   `out/trace.txt` in streaming (mai `read()` intero). Siti statici = righe
   `rst 20h` del dasm in 0000-bfff; siti dinamici = righe `XXXX: rst  $20`
   della traccia, con destinazione = PC della riga dopo `0026: ret`. Per ogni
   sito: voci con l'euristica e il limite da `cp n`/`and m` descritti sopra,
   stato `eseguito`/`statico`/`dati` (80a5-bfff) /`dentro-tabella`, indice
   massimo osservato. Uscite: `out/rst20.txt` (una riga per sito: `sito
   n_voci stato indice_max voci...`), `harness/rst20_tables.h` generato
   (`static const uint16_t rst20_XXXX[] = {...};` per ogni sito con n_voci
   ≥ 1, più un commento con lo stato) e `out/main48_annotato.dasm` =
   `main48.dasm` con, dopo ogni riga `rst 20h` di un sito, righe di
   commento `; rst20: n voci` e `; dw XXXX  -> etichetta` per ogni voce.
   Nome del file `.h` e formato delle righe liberi, purché stabili.
2. `e603.lua` (nuovo, modello: `trace.lua` per moneta/start/input casuali
   e la regola del tap globale): tap in lettura su e603 per `E603_FRAMES`
   frame (default 5500) che accumula `PC:valore → conteggio` e scrive
   `out/e603.txt` (righe `PC valore conteggio`, PC del tap = PC dopo
   l'istruzione) e `harness/e603.h` generato (`static const struct { uint16_t
   pc_lettura; uint8_t valore; } e603_tabella[] = {{0x015E, 0x00}, ...};`
   con il PC riportato all'istruzione, cioè tap-3), più una funzione
   dichiarata `uint8_t leggi_e603(uint16_t pc)` in `harness/porte.c` che
   cerca nella tabella e, se il PC manca, scrive un avviso su stderr e
   restituisce 0 (comportamento di MAME). Gli script devono accettare
   `E603_ROOM` facoltativo (forza la stanza come sopra) così il brief 04
   può estendere la tabella.
3. Traduzione di 0x326C: `harness/fondale.c` (nuovo, per non far crescere
   `routine.c` oltre misura) con `void sub_326c(struct z80 *z)` e le callee
   `sub_23bb`, `sub_23cb`, `sub_23ec`, `sub_336c` come funzioni statiche o
   esportate; dichiarazioni in `ram.h`; voce `{"0x326C", sub_326c, 0xd880,
   0xdfff}` in `harness.c`. Il salto `rst 20h` a 3335 diventa uno `switch`
   sulle tre voci (commentare che viene da `rst20_3335`); la pila interna
   (335A spinge DE, 3320 `pop de; push de`, 3329 `pop hl`) diventa una
   piccola pila C locale. Il C deve essere leggibile (nomi per c082-c089:
   cursore x/y, attributi, contatore di ripetizione), non un emulatore.
4. `stanze.py` (nuovo): decodificatore Python dalla ROM. `python3 stanze.py
   --rom out/main64.bin --stanza N --confronta roms/decoded/mame/rooms/ram_room_M.bin`
   ricostruisce i due piani BG (videoram+colorram, righe 4-27 colonne 1-30)
   con la stessa grammatica di 326C (base + sovrapposizioni + palette) e,
   una volta trovato il formato, il FG; stampa il numero di tile diversi
   per piano rispetto al dump (BG videoram, BG colorram, FG videoram, FG
   colorram) e scrive `out/stanza_N.txt` con la griglia dei codici. Senza
   `--confronta` stampa solo la griglia. Il confronto va fatto sulle sole
   righe 4-27 colonne 1-30 (zona scritta da 326C: D881-DB7E e DC81-DF7E).
   Per il FG, celle che cambiano fra il frame 1700 e il 2000 (oggetti
   animati) si escludono: produrre `out/dump_stanza_N_1700.bin` con uno
   script `dump_stanza.lua` (modello: il Lua del brief 01, con dump a un
   frame `DUMP_FRAME`) e documentare quante celle sono escluse.
5. `DATI.md` (nuovo): tabella delle strutture trovate (F000, F068, F138,
   8EA4, 326C, siti rst20, e603) con indirizzi, formati e limiti noti; serve
   al port.

Libere: nomi delle funzioni, formato di `out/rst20.txt`, uso di `argparse`,
se generare `rst20_tables.h` e `e603.h` sotto `harness/` o `out/` (purché
il Makefile li trovi).

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master/decomp/` con `export DISPLAY=:1` e
`MOPT="-rompath /home/rufio72/OpenSolomonsKey-master/roms -video soft -sound none -window -nomaximize -skip_gameinfo -nothrottle"`
per le corse con debugger, `MFAST="-rompath /home/rufio72/OpenSolomonsKey-master/roms -video none -sound none -skip_gameinfo -nothrottle"`
per quelle con tap. Prima di tutto `touch .inizio`.

1. `TRACE_FRAMES=1900 mame solomon $MOPT -debug -debugger none -autoboot_script trace.lua && python3 coverage.py && grep -E '^(rst|ingressi)' out/numeri.txt` → `ingressi NMI 0066: 1835`, `rst $20 eseguiti (tabelle inline): 48410` (misurati).
2. `python3 rst20.py && grep -c . out/rst20.txt` → 25 righe (una per sito statico; misurato il conteggio dei siti). `grep -E '^(2CD2|2DB8|2DD9|3335|3B10|75EB|7CEC) ' out/rst20.txt | awk '{print $1, $2, $3}'` → `2CD2 26 eseguito`, `2DB8 6 eseguito`, `2DD9 7 eseguito`, `3335 3 eseguito`, `3B10 40 eseguito`, `75EB 12 eseguito`, `7CEC 8 eseguito` (voci misurate dall'euristica con limite; formato riga a scelta ma con questi numeri).
3. Ogni sito dinamico della traccia è fra i 25 statici e ogni destinazione osservata è una voce della sua tabella: `rst20.py` deve stamparlo (`siti dinamici: 7, tutti statici; destinazioni fuori tabella: 0`), uscita attesa esattamente quei numeri (7 misurato; 0 attesa, non misurata).
4. `grep -c 'rst20_' harness/rst20_tables.h` → ≥ 10 (attesa, non misurata); `grep -c 'rst20:' out/main48_annotato.dasm` → 25.
5. `rm -f out/trace.txt` e poi `ls out/trace.txt` → assente.
6. `mame solomon $MFAST -autoboot_script e603.lua && cat out/e603.txt` → una sola riga `0161 00 5435` (misurata con la stessa sequenza di input di trace.lua a 5500 frame; conteggio ±1 ammesso se il frame finale è contato diversamente). `harness/e603.h` contiene `{0x015E, 0x00}`.
7. `SNAP_ENTRY=0x326C mame solomon $MOPT -debug -debugger none -autoboot_script snap.lua && head -1 out/snap_0x326C_regs.txt` → riga `prima PC=326C ... DE=F4C3 BC=1000 ...` (DE e BC attesi dal codice a 3231-3241 con c243=0; il resto attesa, non misurata); `cmp -l out/snap_0x326C_prima.bin out/snap_0x326C_dopo.bin | wc -l` → > 0 (attesa, non misurata; scrive dc80-dfff/d880-dbff).
8. `make -C harness && ./harness/harness 0x326C && ./harness/harness 0x05B3 && ./harness/harness 0x2402` → `OK 0x326C`, `OK 0x05B3`, `OK 0x2402`, codice di uscita 0 (attesa, non misurata).
9. `python3 stanze.py --rom out/main64.bin --stanza 0 --confronta ../roms/decoded/mame/rooms/ram_room_1.bin` e idem `--stanza 1` con `ram_room_2.bin` → `BG videoram: 0 tile diversi`, `BG colorram: 0 tile diversi` per entrambe (obiettivo; attesa, non misurata). Per il FG: `FG videoram: 0`, `FG colorram: 0` al netto delle celle escluse (attesa, non misurata); se il formato FG non è trovato, stampare `FG: non decodificato` e dirlo nel rapporto (vedi domanda 3).
10. `mame solomon $MFAST -autoboot_script dump_stanza.lua` con `DUMP_FRAME=2000 ROOM=0` → `cmp out/dump_stanza_0_2000.bin ../roms/decoded/mame/rooms/ram_room_1.bin` senza uscita (misurato: 0 differenze con lo stesso meccanismo).
11. `test -s DATI.md && grep -c -E 'F000|F068|8EA4' DATI.md` → ≥ 3.
12. `find . -name '*.c' -o -name '*.h' -o -name '*.py' -o -name '*.lua' -o -name '*.sh' | grep -v rst20_tables.h | xargs wc -l | awk '$1>800 && $2!="totale"'` → vuoto.
13. `cd /home/rufio72/OpenSolomonsKey-master && find . -path ./decomp -prune -o -newer decomp/.inizio -type f -print` → vuoto.
14. `cd /home/rufio72/OpenSolomonsKey-master && git status --short | grep -v '^?? decomp/' | grep -v '^ M decomp/' ` → vuoto (nessuna modifica fuori da decomp/).

## Consegna

- File nuovi e modificati solo in `decomp/`. Nessun commit (lo fa la chat
  principale); se si committa su richiesta, messaggio in italiano con riga
  finale `Co-Authored-By: Claude Opus <noreply@anthropic.com>` (o il modello
  usato).
- Rapporto finale: massimo trenta righe con l'uscita delle verifiche 1-14,
  i registri misurati di 0x326C (prima/dopo), il numero di celle FG escluse,
  le deviazioni dal brief e il tempo MAME speso.

## Domande aperte

1. «preferenza» Routine da tradurre con `rst $20`: 0x326C (chiamata con
   `call`, solo RAM, è il decodificatore del fondale che `stanze.py` deve
   riprodurre comunque). Le alternative 0x2D8A e 0x3ADF non contengono
   `rst $20` (i siti 2DB8/2DD9 stanno nei passi 2DAD/2DCE dell'attività 07,
   raggiunti via tabella e non via `call`, e 3B10 è lo spedidore stesso).
   Proposta: 0x326C.
2. «preferenza» Formato di `rst20_tables.h`: un array `uint16_t` per sito
   con nome `rst20_XXXX` e commento di stato; proposta accettarlo.
3. «scelta di progetto» Perimetro del FG in `stanze.py`: il formato della
   lista oggetti non è ancora individuato; proposta: il BG è obbligatorio
   (verifica 9 a 0 differenze), il FG si consegna se trovato entro il
   compito, altrimenti si documenta in `DATI.md` ciò che si è misurato e il
   FG passa a un brief successivo, senza bloccare il brief 04.
4. «preferenza» Valori di e603 per i PC 4CF0 e 62C1 non osservati: la
   tabella resta con il solo 015E e `leggi_e603` restituisce 0 con avviso
   per PC ignoti, come MAME; il brief 04 estende la tabella se le sessioni
   guidate raggiungono 4CED.

## Decisioni (chat principale, 2026-10-19)

- Domanda 1 (preferenza): accettata, routine 0x326C.
- Domanda 2 (preferenza): accettata, un array `uint16_t` per sito.
- Domanda 4 (preferenza): accettata, tabella e603 con il solo 015E, avviso per PC ignoti.
- Domanda 3 (scelta di progetto, confermata dall'utente): BG obbligatorio, FG se trovato entro il compito, altrimenti documentato in DATI.md e rimandato.

## Chiusura ciclo (chat principale, 2026-10-19)

Esito: PASSATO, 14 verifiche su 14. Nota: i registri attesi nella verifica 7 erano un fatto sbagliato del brief (cattura in demo senza moneta); il harness passa sulla cattura reale. FG dei livelli consegnato, 38 stanze su 52 a 0 differenze, dettagli in decomp/DATI.md.
Token: analista Fable 185k (brief 03+04 insieme) più 195k per la revisione del 04, sviluppatore Opus 167k, verificatore Sonnet 18k.
