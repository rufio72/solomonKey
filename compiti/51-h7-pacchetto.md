# Compito 51: H7 pacchetto finale (joystick, make gioco, opzioni, README)

Data: 2026-11-03. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Chiudere il piano hardware: (1) `gioca --joystick`, diagnostica che elenca i
joystick SDL e stampa gli eventi in diretta, più lettura joystick robusta
(hotplug, soglia assi, hat, rimappa pulsanti); (2) bersaglio `make gioco` che
prepara tutto e `gioca` eseguibile da qualunque cartella; (3) opzioni
`--volume`, `--pieno`, `--aiuto`; (4) correzione di `rst20_646B`;
(5) sezione README «How to play».

Vincoli assoluti:
- Solo dentro `/home/rufio72/OpenSolomonsKey-master`. Commenti, messaggi e
  documenti in italiano; il solo README in inglese (come il resto del README).
- Nessun file di codice sopra le 800 righe (i nuovi sotto le 250). `gioca.c`
  è a 275, `macchina.c` a 645: non farli crescere oltre 350 / 650; il codice
  nuovo va in moduli nuovi (sotto).
- INTATTI e senza cambio di comportamento: banco (`harness/harness --tutte`:
  307 `RIEPILOGO`, 1278 `^OK`, 0 fallite), `replay`, `sched_proto`,
  `giro_completo.sh`, confronto audio di H6 (`partita_audio.sh confronta`,
  hash audio `c4b40fea06623a96`), `audio_banco`, `audio_wav.sh oracoli`.
- I file `audio_*.c`, `video_rendi.c`, `macchina.c` non si modificano. Si
  possono toccare: `video_sdl.c/.h` (solo aggiunte, comportamento predefinito
  identico), `comune.c` e `audio_piattaforma.c` (solo percorsi di ricerca
  aggiunti in coda), `gioca*.c`, `gioca_audio.c`, `Makefile`, `rst20_tables.h`
  e `rst20.py`.
- Non installare nulla. MAME non serve per questo lotto. Niente git distruttivo.
- Prove fuori repo: file temporanei nella cartella scratch o `/tmp`, non nel repo.

## Cosa esiste già (fatti verificati)

Da `grafo`/lettura dei file (tutti sotto 150 righe tranne gioca.c):

| File | Righe | Contenuto rilevante |
|---|---|---|
| `decomp/harness/gioca.c` | 275 | `main` con analisi manuale degli argomenti (`--muto`, `--oracolo O D`, DSW, `--registra`, `--riproduci`, `--hash-frame`, `--flusso-ay`, `--registra-latch`, `--fase-audio`, `--mappa`, `--stampa-dsw`); il resto passa a `vsdl_leggi_opzioni`. Messaggio d'uso a riga ~236. Apre finestra con `vsdl_apri("Solomon's Key - gioca", opz.scala)`, poi `tasti_apri_joystick()`, poi `gioca_audio_apri()` se non `--muto`. `finale()` chiude joystick/finestra. `frame_libero` chiama `tasti_ingressi(SDL_GetKeyboardState(NULL), v)` e `tasti_joystick(v)`. |
| `gioca_tasti.c` | 83 | Tabella tasti, `tasti_stampa_mappa`, `tasti_ingressi`, e il joystick H5: `static SDL_Joystick *joy`; `tasti_apri_joystick()` apre solo l'indice 0 se esiste all'avvio; `tasti_joystick(uint8_t out[3])`: asse 0/1 oltre ±8000, hat 0, pulsanti 0=BUTTON1 (e600 0x20), 1=BUTTON2 (0x10), 6=COIN1 (e602 0x04), 7=START1 (e602 0x01). **Mai provato.** |
| `gioca_audio.c` | 73 | `gioca_audio_apri/spingi(const int16_t*, int n)/chiudi`; anello 8 frame, callback SDL, 44100 Hz mono int16. Il punto per il volume è `gioca_audio_spingi` (solo dispositivo: l'hash audio si calcola altrove e non deve cambiare). |
| `gioca_dsw.c` | 46 | `dsw_applica`. |
| `video_sdl.c/.h` | 163 | `vsdl_leggi_opzioni` (-s/--scala 1-8, -n/--headless, -m/--max), `vsdl_apri(titolo, scala)` crea finestra `SDL_WINDOW_SHOWN` di `VID_LARGO*scala` x `VID_ALTO*scala` (256x224), renderer accelerato+vsync o software, texture RGB24; `vsdl_presenta` gestisce SDL_QUIT ed ESC. |
| `Makefile` | 88 | Solo in `decomp/harness/` (non c'è Makefile in cima al repo; `.gitignore` riga 44 ignora `Makefile`, ma quello di harness è tracciato). `HA_SDL` da `sdl2-config`; `BERSAGLI_SDL = mostra_oracolo mostra_replay gioca` solo se SDL c'è; `all` li include; `clean` li rimuove; `.PHONY: all clean`. |
| `comune.c` | 30 | `carica_rom()` cerca solo `out/main64.bin` o `../out/main64.bin` (relativi alla cartella corrente). |
| `audio_piattaforma.c` | 156 | cerca `1.3jk` solo in `../roms/ext/solomon/` e `../../roms/ext/solomon/`. |
| `rst20_tables.h` | generato | riga 89-92: `/* 646B: statico, 3 voci ... */ rst20_646B[] = { 0x64D3, 0x64B4, 0x6539 }`. Generato da `decomp/rst20.py` (intestazione «non modificare a mano»). |

Fatti sul runtime (da lettura del codice e prove):
- Il gioco libero (`gioca` senza `--oracolo`) legge solo due cose da disco:
  `out/main64.bin` (65536 byte, costruito da `decomp/build_rom.sh` con
  `6.3f` + seconda metà di `7.3h` + prima metà di `7.3h` + 12288 zeri + primi
  4096 byte di `8.3jk`) e `1.3jk` (ROM audio). Perciò i soli file ROM
  necessari a runtime sono **`6.3f`, `7.3h`, `8.3jk`, `1.3jk`**. NON servono più
  `2.5lm 3.6lm 4.7lm 5.8lm` (grafica), `9.3m 10.3p 11.3r 12.3t` e
  `solomonj.zip`: grafica e dati audio sono C generato nel repo. (Verifica
  in 4 che sia così.)
- `decomp/out/main64.bin` NON è nel repository (`.gitignore`: `decomp/out/*`,
  salvo `snap_*` e `video/`): su un clone nuovo non esiste. Esiste ora in
  locale (65536 byte). `build_rom.sh` legge da `../roms/ext/solomon/`, che
  è `roms/solomon.zip` decompresso (`unzip` c'è in `/usr/bin/unzip`; la
  cartella `roms/ext/solomon/` c'è ora con i 12 file). `roms/solomon.zip` ha
  12 file (1.3jk … 12.3t).
- I modi `--oracolo` e il banco richiedono invece file di `decomp/out/` fatti
  con MAME (`giroboot_*`, `giroboot_602.bin`...): fuori da `make gioco`, che
  copre solo il modo libero.
- `harness/gioca -n 600` da `decomp/` (misurato): `FRAME rese 600`,
  `HASH ultimo ecd5acf694285489`, `AUDIO campioni 489510`,
  `AUDIO hash 8aedf41c0424df54`, 1,5 s. Lanciato da `/tmp` o da un'altra
  cartella (misurato): `main64.bin non trovato in out/ né in ../out/`, uscita
  non zero. Quindi oggi `gioca` funziona solo da `decomp/` o da `decomp/harness/`.
- SDL2 2.30.0 installato (`sdl2-config --version`).

**Hardware vero presente sulla macchina (misurato):** joystick «Bigben
Interactive DAIJA Arcade Stick», kernel `/dev/input/js0` (event17). Un programma
SDL minimo con `SDL_INIT_JOYSTICK` lo enumera: `n=1`, indice 0, `axes=4
btn=13 hats=1`, GUID `03008b956b1400000406000000010000` (VID 0x146b, PID
0x0604), `SDL_IsGameController` = 1. È uno stick commerciale con selettore di
modalità (D-pad / stick analogico): in modalità D-pad la leva arriva come
**hat**, quindi l'hat è il caso primario. Con
`SDL_GAMECONTROLLER_IGNORE_DEVICES=0x146b/0x0604` lo stesso programma dà
`n=0` (misurato): serve come modo per simulare «nessun joystick».
`/dev/input/js0` è leggibile dall'utente (ACL). `/dev/uinput` è scrivibile
dall'utente (ACL `user:rufio72:rw-`, misurato) senza sudo; i moduli Python
`evdev` e i programmi `evtest`/`jstest` NON ci sono.

## Struttura da realizzare

### 1. Joystick: nuovi moduli `gioca_joy.c` (+ `gioca_joy.h`) e `gioca_diag.c`

`gioca_joy.h` espone (e `gioca_tasti.c` perde le funzioni joystick, che si
spostano in `gioca_joy.c`; i prototipi in `gioca.c` si sostituiscono con
`#include "gioca_joy.h"`):

```c
typedef struct { int b1, b2, start, coin; } joy_mappa;   /* indici pulsante, -1 = nessuno */
extern joy_mappa joy_mappa_corrente;                       /* predefinita: 0, 1, 7, 6 */
int  joy_analizza_mappa(const char *s, joy_mappa *m);      /* "B1,B2,START,COIN"; 0 ok, -1 errore */
void joy_apri(void);                  /* init sottosistema, apre TUTTI i joystick presenti, registra l'event watch per ADDED/REMOVED */
void joy_chiudi(void);
void joy_ingressi(uint8_t out[3]);    /* OR sui byte e600/e602 di tutti i joystick aperti */
void joy_elenca(void);                /* stampa i dispositivi (usata dalla diagnostica) */
```

- Hotplug: `SDL_AddEventWatch` con una funzione che su `SDL_JOYDEVICEADDED`
  apre `SDL_JoystickOpen(e->jdevice.which)` e su `SDL_JOYDEVICEREMOVED` chiude
  il joystick con quell'`instance_id` (`SDL_JoystickFromInstanceID`). Gli
  eventi vengono smaltiti da `SDL_PollEvent` dentro `vsdl_presenta`, quindi il
  watch li vede anche se `gioca` non li legge. Tabella fissa di 8 joystick.
  Impostare prima di `SDL_Init`: `SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1")`
  (lo stick funziona anche con la finestra non a fuoco).
- Lettura (per ogni joystick aperto, risultato in OR; il joystick si somma
  alla tastiera, non la sostituisce; `gioca_tasti.c` e la tastiera restano
  invariati nel comportamento):
  - direzioni da assi 0 (X) e 1 (Y) con soglia ±8000 (i microswitch danno
    ±32767 pieni: la soglia basta), **e** da hat 0 (`SDL_HAT_RIGHT/LEFT/UP/DOWN`,
    anche le diagonali LEFTUP ecc.); se non c'è hat ma ci sono assi vale il solo
    asse; direzioni opposte insieme si annullano (come in H5);
  - pulsanti: `b1` → e600 0x20 (BUTTON1), `b2` → e600 0x10 (BUTTON2),
    `start` → e602 0x01 (START1), `coin` → e602 0x04 (COIN1). Indice oltre
    `SDL_JoystickNumButtons` o -1 = ignorato, mai un accesso fuori limite.
  - Ricalcolo a ogni frame con `SDL_JoystickGet*` (come H5); non servono eventi.
- `--joy-mappa B1,B2,START,COIN` (quattro interi 0-31 o -1, separati da
  virgola): esempio `--joy-mappa 2,3,9,8`. Errore italiano ed uscita 2 se non
  sono esattamente quattro numeri validi. Predefinita invariata rispetto a H5
  (0,1,7,6): da ritoccare dopo la prova dell'utente (vedi domande).
  Opzione di spegnimento: `--senza-joystick` (non apre il sottosistema).
- Diagnostica `gioca --joystick [-t SEC]` in `gioca_diag.c`:
  `int joy_diagnostica(long secondi_max)` (0 = senza limite; `-t` serve alle
  prove automatiche). Si esegue PRIMA di aprire il gioco e non apre l'emulazione.
  1. `SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK)`; se il video non parte, solo
     joystick e uscita con Ctrl-C. Finestra piccola (320x120, titolo
     «gioca --joystick: ESC per uscire») per ricevere ESC; se non si crea,
     si continua senza finestra e si esce con Ctrl-C (`SIGINT` imposta un flag).
  2. Elenca ogni dispositivo, una riga ciascuno, formato esatto:
     `JOYSTICK <indice> "<nome>" assi=<n> pulsanti=<n> cappelli=<n> guid=<32 hex>`.
     Con zero dispositivi stampa `nessun joystick rilevato` e prosegue (resta in
     attesa di hotplug).
  3. Poi per ogni evento stampa, con `fflush(stdout)` a ogni riga:
     `ASSE <indice> <asse> <valore>` (solo quando il valore cambia di più di
     2000 dall'ultimo stampato, per non inondare con il rumore degli analogici),
     `PULSANTE <indice> <numero> giu|su`,
     `CAPPELLO <indice> <hat> <su|giu|sinistra|destra|centro e combinazioni con '+'>`,
     `COLLEGATO <indice> "<nome>"` / `SCOLLEGATO <indice>`.
     Ogni riga di evento stampa in coda, tra parentesi, l'azione di gioco
     assegnata dalla mappa corrente se c'è (es. `PULSANTE 0 3 giu (BUTTON1)`,
     `CAPPELLO 0 0 destra (destra)`), così l'utente capisce senza altra tabella.
  4. Esce con 0 su ESC, chiusura finestra, Ctrl-C o scadenza di `-t`.
  Il modulo non deve superare 200 righe; `gioca.c` ne guadagna al più una
  dozzina (chiamata e opzioni).

### 2. `make gioco` e percorsi indipendenti dalla cartella

- Bersaglio `gioco` in `decomp/harness/Makefile` (e `.PHONY`), invocato
  `make -C decomp/harness gioco`; in più un `decomp/Makefile` minimo di due
  righe (`gioco: ; $(MAKE) -C harness gioco`) così funziona anche `make -C decomp gioco`
  (`.gitignore` ignora i file chiamati `Makefile`: aggiungere eccezione
  `!decomp/Makefile` come già fatto per l'altro, vedi `git ls-files`).
  Nessun Makefile in cima al repo.
- `gioco` deve: (a) se `sdl2-config` non risponde, stampare in italiano
  `ERRORE: SDL2 non trovato. Installa libsdl2-dev (sudo apt install libsdl2-dev) e riprova.` e
  fallire con uscita 2 (oggi, senza SDL, `gioca` semplicemente non è tra i
  bersagli); (b) preparare le ROM: se manca `decomp/out/main64.bin`, eseguire
  `sh decomp/build_rom.sh`, e se manca `roms/ext/solomon/6.3f` estrarre prima
  `roms/solomon.zip` con `unzip -o -d roms/ext/solomon` (o `solomonj.zip` non
  serve); se mancano sia la cartella sia lo zip, stampare
  `ERRORE: mancano le ROM: metti solomon.zip in roms/` e uscita 2;
  (c) compilare `gioca` (dipendenze già nel bersaglio `gioca`, più i nuovi
  moduli in `SORGENTI_GIOCA`); (d) stampare alla fine come si lancia
  (`decomp/harness/gioca`). `make all` non cambia significato. `clean` rimuove
  anche eventuali oggetti nuovi, mai `out/main64.bin`.
- Percorsi: `carica_rom()` e `audio_piattaforma.c` oggi dipendono dalla
  cartella corrente. Aggiungere in coda alle liste di candidati dei percorsi
  calcolati dalla cartella dell'eseguibile (`readlink("/proc/self/exe")` →
  `dirname` = `decomp/harness`): `<exe>/../out/main64.bin` e
  `<exe>/../../roms/ext/solomon/1.3jk`. Le liste attuali restano prime e
  invariate, quindi banco, replay e giro non cambiano. Funzione comune
  `const char *percorso_exe(const char *relativo, char *buf, size_t n)` in
  `comune.c` (se `/proc/self/exe` non c'è, restituisce NULL e si salta il
  candidato). Con questo `gioca` parte da qualunque cartella senza `chdir`
  (F2/`execv` e i percorsi relativi di `--registra`/`--riproduci` restano quelli
  dell'utente).
- Messaggio di errore in caso di ROM mancanti a runtime (modo libero): oltre al
  testo attuale, una riga `Esegui: make -C decomp/harness gioco`.

### 3. Opzioni finali

- `--volume N` (0-100, predefinito 100, intero): moltiplica i campioni in
  `gioca_audio_spingi` (`v * N / 100`, con arrotondamento verso zero) solo per
  il dispositivo; `AUDIO hash` e `AUDIO campioni` (calcolati prima) non
  cambiano. Fuori intervallo o non numerico: errore italiano, uscita 2.
  Funzione `void gioca_audio_volume(int percento)` in `gioca_audio.c`.
- `--pieno`: schermo intero «desktop». In `video_sdl.c` aggiungere
  `void vsdl_pieno(int on)` (da chiamare prima di `vsdl_apri`): crea la finestra
  con `SDL_WINDOW_FULLSCREEN_DESKTOP` e imposta
  `SDL_RenderSetLogicalSize(rend, VID_LARGO, VID_ALTO)` +
  `SDL_RenderSetIntegerScale(rend, SDL_TRUE)` così l'aspetto 8:7 resta e
  il resto è nero. Senza `--pieno` nulla cambia (stessa finestra di prima).
  Il cursore del mouse si nasconde in pieno schermo (`SDL_ShowCursor(SDL_DISABLE)`).
  `-s` resta la scala della finestra normale. Con `-n` (senza finestra) è
  ignorata.
- `--aiuto` (e `-h`) in italiano: stampa su stdout, uscita 0, il testo completo
  di tutte le opzioni, raggruppate: Schermo (`-s/--scala`, `--pieno`, `-n`,
  `-m`), Audio (`--muto`, `--volume`, `--flusso-ay`, `--registra-latch`,
  `--fase-audio`), Ingressi (`--joystick`, `-t`, `--joy-mappa`,
  `--senza-joystick`, `--mappa`), Partita (`--vite`, `--cabinet`, `--demo`,
  `--bonus`, `--difficolta`, `--timer`, `--extra`, `--dsw1`, `--dsw2`,
  `--stampa-dsw`), Registrazione (`--registra`, `--riproduci`, `--hash-frame`,
  `--oracolo`), Tasti in gioco (ESC, P, F2). Il testo sta in `gioca_aiuto.c`
  (funzione `void gioca_aiuto(void)`), e il messaggio d'uso dell'errore di
  `gioca.c` diventa una riga `uso: gioca [opzioni] — vedi gioca --aiuto`.
  `--aiuto` si riconosce prima di aprire SDL.
- `-t SEC` è accettato solo con `--joystick` (altrimenti errore italiano).
  Il parsing delle nuove opzioni si fa con lo stesso schema del `main`
  attuale (estrazione prima di `vsdl_leggi_opzioni`). Se `gioca.c` supererebbe
  350 righe, spostare l'analisi delle opzioni nuove in `gioca_opzioni.c`.

### 4. Manutenzione `rst20_646B`

La tabella a 646B ha 10 parole vere (da `ZONE-MAI-ESEGUITE.md` §3):
`64D3 64B4 6539 64C4 657B 65D0 64C8 6644 64CC 6655`. Correggere
`rst20_tables.h` (10 voci, commento `10 voci`). Poiché il file è generato da
`rst20.py`, correggere anche il generatore perché rigenerando ottenga lo
stesso risultato (esaminare come `rst20.py` calcola il numero di voci;
aggiungere un'eccezione esplicita per 646B con le 10 parole, documentata) e
verificare che il file rigenerato sia identico a quello corretto a mano. Le
dieci parole sono tutte già eseguite: nessun cambiamento atteso nel banco.
Annotare in `ZONE-MAI-ESEGUITE.md` §3 e nella riga di `STATO.md` ~141 che
è stata corretta. Se `rst20.py` richiede file in `out/` che mancano,
non rigenerare: correggere a mano `rst20_tables.h` e l'eccezione in `rst20.py`,
e dirlo nel rapporto.

### 5. README

Sezione `## How to play` (inglese, prima di `## Requirements` o subito dopo),
con:
- Requisiti: `libsdl2-dev` (SDL2 ≥ 2.0, provato con 2.30.0), gcc, make;
  `unzip`. ROM: mettere `solomon.zip` in `roms/` (mai committarla). A runtime il
  gioco usa solo `6.3f`, `7.3h`, `8.3jk` (CPU principale) e `1.3jk` (CPU audio),
  assemblati da `make gioco`; le altre otto ROM (grafica e `solomonj.zip`)
  non servono più perché grafica e dati audio sono C generato nel repository.
  MAME serve soltanto per rifare gli oracoli, non per giocare.
- Comandi: `make -C decomp/harness gioco`, poi `decomp/harness/gioca`; esempi con
  `-s 4`, `--pieno`, `--vite 5`, `--volume 50`, `--muto`.
- Tasti: ripresi da `gioca --mappa` (frecce/WASD, Z/Ctrl/Space jump? — nel README
  scrivere solo ciò che `--mappa` dice: BUTTON1/BUTTON2, e che quale sia salto e
  quale magia lo si annota dopo la prova manuale; 5/C gettone, 1/Invio start,
  P pausa, F2 reset, ESC esci).
- Joystick: `gioca --joystick`, uscita attesa di esempio, `--joy-mappa`,
  hotplug, joystick sommato alla tastiera; nota sugli encoder arcade: leva
  spesso come hat, i pulsanti si identificano con la diagnostica.
- Opzioni DSW e registra/riproduci in poche righe; limite dichiarato: il modo
  libero si ferma con `FERMO ...` (uscita 3) su codice non ancora tradotto.
- Aggiornare la riga «Next up» e quella H6 del README: H7 chiuso.
Aggiornare anche `compiti/PIANO-HARDWARE.md` (H7 `[x]`, con la correzione
della frase «le ROM restano fuori… carica da roms/»: solo 4 file), `STATO.md` e
una sezione «H7» in `decomp/GIRO.md`.

## Verifiche obbligatorie (da `/home/rufio72/OpenSolomonsKey-master`)

Le uscite segnate «misurata» sono state ottenute ora; le altre sono attese.

1. `make -C decomp/harness gioco 2>&1 | tail -3` e poi `make -C decomp/harness`
   → nessun errore, nessun avviso nuovo (`-Wall -Wextra`). `make -C decomp gioco`
   funziona uguale.
2. Banco: `cd decomp && harness/harness --tutte | grep -c '^OK'` → `1278`;
   `... | grep -c RIEPILOGO` → `307`; nessuna riga `FALLITO`/`DIVERSO`
   (riferimento: stessa uscita di prima del lotto, da confrontare salvando la
   uscita prima di iniziare in `/tmp`). Dopo la correzione di 646B uscita
   identica byte per byte a quella salvata.
3. Regressioni: `cd decomp && sh giro_completo.sh` → riga `GIRO COMPLETO 0-5499:
   saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13
   frame resi con hash coincidenti`, uscita 0; `harness/sched_proto out/sched_log.txt`
   e `harness/replay` come in `REPLAY.md` (stesse uscite di prima, salvate prima
   di iniziare); `harness/gioca -n 5434 --oracolo out/giroboot_oracolo.txt
   out/giroboot_disp.txt --riproduci out/ingressi_replay.txt` → `FRAME rese 5434`,
   `HASH ultimo a239d0a940bec0b9`, `FRAME ultimo 5499`, `AUDIO campioni 4042500`,
   `AUDIO hash c4b40fea06623a96`; `sh partita_audio.sh confronta` come fissato
   in H6 (604 / 312 / 1190).
4. Indipendenza dalla cartella: `cd /tmp && /home/rufio72/OpenSolomonsKey-master/decomp/harness/gioca -n 600 | tail -4`
   → `FRAME rese 600`, `HASH ultimo ecd5acf694285489`, `AUDIO campioni 489510`,
   `AUDIO hash 8aedf41c0424df54` (misurati da `decomp/`). Stessa cosa da `decomp/`.
5. Solo 4 ROM a runtime (non distruttivo): copiare in una cartella temporanea
   `/tmp/provarom` solo `6.3f 7.3h 8.3jk 1.3jk` di `roms/ext/solomon/` e
   ricreare lì `out/main64.bin` con la stessa ricetta di `build_rom.sh`,
   verificando che `cmp` con `decomp/out/main64.bin` dia silenzio; quindi
   confermare con `strace -f -e trace=openat harness/gioca -n 50 2>&1 | grep -E 'roms|out/|\.bin'`
   (se `strace` manca: `ltrace` non serve, basta `grep -n fopen decomp/harness/gioca*.c
   scheda*.c audio_piattaforma.c comune.c macchina_libera.c`) che gli unici file ROM
   aperti sono `out/main64.bin` e `1.3jk`. Riportare l'elenco.
6. Fresh build di `make gioco` (simulazione): in una copia del repo senza
   `decomp/out/main64.bin` (`git worktree` NO; usare `cp -r` in `/tmp` del solo
   necessario o spostare e rimettere `main64.bin` con cura) `make -C decomp/harness gioco`
   ricrea `out/main64.bin` identico (`cmp`). Senza `sdl2-config`:
   `make -C decomp/harness gioco SDL2_CONFIG=/nonesiste` → stampa
   `ERRORE: SDL2 non trovato. Installa libsdl2-dev ...`, uscita 2.
7. Aiuto: `decomp/harness/gioca --aiuto | head -3; echo $?` → testo italiano,
   uscita 0; `decomp/harness/gioca --aiuto | grep -c -e '--volume' -e '--pieno' -e '--joystick' -e '--joy-mappa'`
   → 4 o più (attesa, non misurata). Ogni opzione citata in 3 è nell'aiuto.
8. Volume: `harness/gioca -n 600 --volume 50` → stesso `AUDIO hash 8aedf41c0424df54`
   di `-n 600` (il volume tocca solo il dispositivo); `--volume 101`,
   `--volume abc`, `--volume -1` → messaggio italiano, uscita 2.
9. Joystick, parsing: `gioca --joy-mappa 2,3,9,8 --stampa-dsw` → `DSW1 02 DSW2 00`
   (attesa); `--joy-mappa 1,2,3` e `--joy-mappa a,b,c,d` → errore italiano,
   uscita 2.
10. Joystick, diagnostica con hardware presente (misurabile qui, lo stick è
    collegato): `SDL_VIDEODRIVER=dummy timeout 5 decomp/harness/gioca --joystick -t 2`
    → uscita 0; stdout contiene la riga
    `JOYSTICK 0 "Bigben Interactive DAIJA Arcade Stick" assi=4 pulsanti=13 cappelli=1 guid=03008b956b1400000406000000010000`
    (nome, assi, pulsanti e guid misurati con un programma SDL minimo; il
    formato della riga è definito sopra).
11. Joystick, fallback senza dispositivo: `SDL_GAMECONTROLLER_IGNORE_DEVICES=0x146b/0x0604
    SDL_VIDEODRIVER=dummy decomp/harness/gioca --joystick -t 2` → uscita 0,
    stdout `nessun joystick rilevato` (con il filtro lo stesso programma
    SDL minimo dà `n=0`, misurato; il testo esatto è quello definito sopra).
    Se il filtro non basta per la diagnostica, riportarlo e usare la
    verifica 12 o l'opzione di prova `--senza-joystick`.
12. FACOLTATIVA, joystick virtuale senza hardware, senza privilegi di root: lo script
    Python (solo libreria standard, `os`, `fcntl`, `struct`, `ctypes` non
    necessari) `decomp/prova_joystick_uinput.py` apre `/dev/uinput` (scrivibile
    dall'utente per ACL, misurato; in caso di `PermissionError` lo script
    stampa che serve `sudo -S` o l'appartenenza al gruppo `input` e termina con
    uscita 77 = «saltata»), crea un gamepad virtuale (`ABS_X`, `ABS_Y`,
    `ABS_HAT0X`, `ABS_HAT0Y`, `BTN_SOUTH`... 8 pulsanti, nome
    «prova-uinput»), attende 1 s, invia: asse X a +32767 per 0,3 s, hat destra
    per 0,3 s, pulsante 0 giù/su, poi rimuove il dispositivo. Eseguito in
    parallelo con `gioca --joystick -t 6` si attendono (non misurato) le righe
    `COLLEGATO ... "prova-uinput"`, `ASSE <i> 0 32767`, `CAPPELLO <i> 0 destra`,
    `PULSANTE <i> 0 giu`/`su`, `SCOLLEGATO <i>` — questo prova anche l'hotplug.
    Se SDL non vede il dispositivo virtuale (permessi sul nodo `event*` creato
    dal kernel) riportarlo come «non verificabile» senza insistere. Lo script
    non cambia nulla nel sistema oltre al dispositivo temporaneo.
13. File grandi: `find decomp/harness -name '*.c' | xargs wc -l | awk '$1>800 && $2!="total" && $2!="totale"'`
    → vuoto. `git status --short` mostra solo i file di questo lotto.
14. MANUALE dell'utente (non automatica, marcata come tale nel rapporto): con lo
    stick collegato, `decomp/harness/gioca --joystick`: deve elencare il DAIJA e,
    muovendo la leva in modalità D-pad, righe `CAPPELLO 0 0 ...` (in modalità
    analogica `ASSE`), e per ciascuno dei 13 pulsanti `PULSANTE 0 n giu`/`su`.
    Poi `decomp/harness/gioca`: la leva muove Dana, un pulsante salta, uno
    lancia la magia, uno dà il gettone e uno lo start; quelli che non
    corrispondono si correggono con `--joy-mappa`. Staccare e riattaccare lo
    stick durante `gioca --joystick` deve dare `SCOLLEGATO`/`COLLEGATO`.
    Questa prova è l'unica che dimostra il joystick in partita.

## Consegna

- Commit (nessun push): moduli nuovi, Makefile, `decomp/Makefile`, `.gitignore`
  se serve, README, `GIRO.md`, `STATO.md`, `PIANO-HARDWARE.md`,
  `ZONE-MAI-ESEGUITE.md`, `rst20_tables.h`, `rst20.py`, brief 51 con gli esiti in
  fondo. Messaggio: `Lotto H7: pacchetto (joystick diagnostico, make gioco, opzioni, README)` e riga
  `Co-Authored-By: <modello> <noreply@anthropic.com>`.
- Rapporto finale di massimo trenta righe: uscita delle verifiche 1-13
  (osservata contro attesa), esito della 12 (eseguita/saltata), deviazioni,
  la mappa predefinita dei pulsanti e il fatto che 14 resta all'utente.

## Domande aperte

1. Mappa pulsanti predefinita del joystick: proposta 0,1,7,6 (come H5, mai
   provata) e aggiornarla dopo la prova manuale dell'utente sul DAIJA (13
   pulsanti; quali siano salto e magia non si sa). — preferenza.
2. Posizione del bersaglio: `make -C decomp/harness gioco` più un
   `decomp/Makefile` di inoltro; nessun Makefile in cima al repo (e `.gitignore`
   lo ignorerebbe). — preferenza.
3. Percorsi indipendenti dalla cartella con `/proc/self/exe` (solo Linux, che è
   l'unica piattaforma provata) anziché `chdir`. — preferenza.
4. `make gioco` estrae da solo `roms/solomon.zip` in `roms/ext/solomon/` e
   costruisce `out/main64.bin` (due file derivati, già fuori dal repo). Se si
   preferisce che l'utente lo faccia a mano, il bersaglio si limita a
   stampare le istruzioni. Proposta: automatico. — preferenza.
5. `rst20_646B`: correggere sia `rst20_tables.h` sia il generatore `rst20.py`
   con un'eccezione esplicita (anziché lasciare un generatore che
   rigenererebbe il difetto). — preferenza.
6. Joystick: usare l'API joystick grezza di SDL (come H5), non quella
   GameController: lo stick è riconosciuto come controller (`SDL_IsGameController`
   = 1) ma la mappatura GameController nasconderebbe gli indici reali dei
   13 pulsanti che l'utente vuole conoscere. Proposta: grezza. — scelta di
   progetto (cambia la semantica degli indici in `--joy-mappa`; la scelta
   riguarda l'utente perché dovrà ragionare in indici di pulsante, e va
   confermata, anche se la proposta è quella sopra).
7. Verifica 12 (uinput): eseguirla solo se l'utente accetta di creare un
   dispositivo di input virtuale per qualche secondo sulla sua sessione
   (nessun root, ma il dispositivo compare nel desktop); altrimenti restano la
   10, la 11 e la 14. — scelta di progetto (tocca il sistema dell'utente).

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Domande 6 e 7 sono scelte di progetto, risolte dalla chat principale; da
segnalare all'utente:
6. Sì: API joystick grezza di SDL, non GameController — per uno stick
   arcade servono gli indici reali dei 13 pulsanti in `--joy-mappa`;
   GameController rimapperebbe su uno schema da gamepad e nasconderebbe
   pulsanti.
7. Sì, come verifica facoltativa, con un vincolo: il dispositivo uinput
   virtuale dev'essere SOLO un joystick (mai tastiera o mouse), creato e
   distrutto dentro il test, con nome riconoscibile (es. «solomon-test»).
Preferenze 1-5, accolte con la proposta:
1. Sì: mappa predefinita 0,1,7,6, da ritoccare dopo la prova dell'utente
   sul DAIJA.
2. Sì: bersaglio in `decomp/harness` con `decomp/Makefile` che lo
   richiama.
3. Sì: percorsi calcolati da `/proc/self/exe`, senza `chdir` (così
   `gioca` parte da qualunque cartella).
4. Sì: `make gioco` estrae da solo lo zip e costruisce `main64.bin`.
5. Sì: la tabella 646B si corregge anche nel generatore `rst20.py`.

## Stato alla sospensione (chat principale, 2026-11-03 sera)

Lo sviluppatore è stato interrotto su richiesta dell'utente A CODICE SCRITTO
MA NON VERIFICATO: le sue ultime parole sono «No warnings. Now the full
build and tests» — quindi il codice compilava senza avvisi in una prova
parziale, ma la build completa, le 14 verifiche, l'aggiornamento dei
documenti e il commit NON sono stati fatti.

Nell'albero, non committati:
- nuovi: `decomp/Makefile`, `decomp/harness/gioca_joy.c/.h`,
  `gioca_diag.c`, `gioca_aiuto.c`, `gioca_opzioni.c/.h`;
- modificati: `.gitignore`, `decomp/harness/Makefile`, `gioca.c`,
  `gioca_tasti.c`, `gioca_audio.c`, `audio_piattaforma.c`, `comune.c`,
  `video_sdl.c/.h`, `rst20_tables.h` (tabella 646B corretta),
  `decomp/rst20.py`.
- NON ancora toccati: `README.md` (sezione «Come si gioca»), `GIRO.md`,
  `STATO.md`, `PIANO-HARDWARE.md` (spunta H7).

Alla ripresa: rilanciare lo sviluppatore su questo brief dicendogli che il
codice è già nell'albero e va verificato, completato nei documenti e
committato — non riscritto da capo. Poi le prove manuali dell'utente:
`gioca --joystick` col DAIJA collegato (SDL lo vede: 4 assi, 13 pulsanti,
1 hat) e la partita vera.
