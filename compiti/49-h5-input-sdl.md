# Compito 49: H5 — il gioco si gioca (programma `gioca`, tastiera, DSW, registrazione)

Data: 2026-11-03. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Voce H5 di `compiti/PIANO-HARDWARE.md`. Un programma `decomp/harness/gioca` che
unisce la macchina del giro libero (boot dal reset, NMI, spedidore, corpi
tradotti) alla finestra SDL2 di H3, prende gli ingressi dalla tastiera (e dal
joystick SDL se c'è), accetta i DSW da riga di comando, registra e riproduce
gli ingressi nel formato di `out/ingressi_replay.txt`. Due modi: **libero**
(senza oracolo, il gioco si gioca) e **`--oracolo`** (la macchina di H4 con
oracolo, ripresa in finestra: riproduce esattamente la partita registrata).

**Esito dell'analisi sulla cadenza delle NMI (misurato sul registro, vedi
fatti): una NMI per frame al vblank NON basta a riprodurre l'hash del frame
5499.** Il gioco C non ha un modello dei cicli Z80: la posizione delle NMI
dentro i corpi viene dall'oracolo. Perciò la verifica «hash a239d0a940bec0b9»
si fa nel modo `--oracolo`; il modo libero si verifica con determinismo, ciclo
registra/riproduci e schermate stabili (sezione Verifiche).

## Vincoli assoluti

- Nessuna modifica fuori da `/home/rufio72/OpenSolomonsKey-master`. Commenti,
  messaggi e documenti in italiano. `gcc`/`make` nel PATH; SDL2 via `sdl2-config`
  (presente in `/usr/bin`).
- NON si modificano: `video_sdl.c`, `video_sdl.h`, `video_rendi.c`, `rendi_frame`,
  `replay.c`, `sched.c`, `sched_proto.c`, `mostra_*.c`, `gioco_libero.c`, i
  corpi tradotti (`attivita*.c`, `nemici*.c`, `z80_*.c`). Banco (307 voci,
  1278 catture, 0 fallite), replay, sched_proto e `giro_completo.sh` restano
  identici nell'uscita. `macchina.c` si tocca solo come scritto in «Struttura».
- Nessun file di codice sopra 800 righe (macchina.c è a 635: dopo le modifiche
  resta sotto 700; ogni modulo nuovo sotto 400).
- MAME non serve: nessuna finestra MAME (`pgrep -c mame` → `0`). Le prove con
  SDL senza schermo usano `SDL_VIDEODRIVER=dummy` o `-n`.
- La prova di gioco con la finestra vera è facoltativa e marcata (m-man).
- Prototipi e file temporanei fuori dal repository (`/tmp/h49/`).

## Cosa esiste già (fatti verificati)

**Layout.** I sorgenti stanno in `decomp/harness/`, gli script e `out/` in
`decomp/` (da lì: `harness/gioco_libero ...`). `out/` non è versionata ma
contiene (da `ls`): `ingressi_replay.txt` (5434 righe, frame 66-5499),
`giroboot_oracolo.txt`, `giroboot_disp.txt`, `giroboot_*.bin`; se mancano:
`sh giro_boot.sh`.

**Ingressi** (da `ingressi_replay.txt`, misurato): riga `<frame> <e600> <e601>
<e602>` (frame decimale, byte in esadecimale a due cifre), una per NMI, frame
da 66 a 5499. Valori distinti: e600 ∈ {00,01,02,04,08,10,20} (mai due bit),
e601 sempre 00, e602 ∈ {00,01 (20 frame), 04 (20 frame)}: moneta poi start.
`macchina.c` (`carica_ingressi`, riga 126) lo legge in `ingressi[MAX_FRAME][3]`
(MAX_FRAME 8192) indicizzato per numero di frame e a ogni NMI scrive i tre byte
in `ram[0xe600..0xe602 - 0xc000]` (`esegui_nmi`, riga 252).

**Bit delle porte** (da `/home/rufio72/mame-src/solomon.cpp` righe 245-273,
tutti `IP_ACTIVE_HIGH`: premuto = 1):

| Porta | bit 0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40, 0x80 |
|---|---|---|---|---|---|---|---|
| e600 P1 | destra | sinistra | su | giù | BUTTON2 | BUTTON1 | non usati |
| e601 P2 (cocktail) | destra | sinistra | su | giù | BUTTON2 | BUTTON1 | non usati |
| e602 SYSTEM | START1 | START2 | COIN1 | COIN2 | non usati | | |

**DSW** (da solomon.cpp 275-318; `ram[0xe604]` = DSW1, `ram[0xe605]` = DSW2;
sono scritti nel boot, `macchina.c` righe 329-330: `0x02` e `0x00`):

| Bit | Opzione | Valori |
|---|---|---|
| DSW1 0x01 | Demo Sounds | 0 = on, 1 = off |
| DSW1 0x02 | Cabinet | 2 = Upright, 0 = Cocktail |
| DSW1 0x0c | Vite | 0x0c=2, 0x00=3, 0x08=4, 0x04=5 |
| DSW1 0x30 / 0xc0 | Coin B / Coin A | 0x00 1C_1C, 0x20 2C_1C, 0x10 1C_2C, 0x30 1C_3C (idem 0x80/0x40/0xc0) |
| DSW2 0x03 | Difficoltà | 0x02 facile, 0x00 normale, 0x01 più dura, 0x03 difficile |
| DSW2 0x0c | Timer | 0x08 lento, 0x00 normale, 0x04 veloce, 0x0c velocissimo |
| DSW2 0x10 | Extra | 0 normale, 0x10 difficile |
| DSW2 0xe0 | Bonus Life | 0x00 30k-200k-500k; 0x80 100k-300k-800k; 0x40 30k-200k; 0xc0 100k-300k; 0x20 30k; 0xa0 100k; 0x60 200k; 0xe0 nessuno |

Il codice tradotto legge solo `e605 & 3` (attract07c.c:271), `(e605>>2)&3`
(attract07b.c:79), `e604 & 2` (attract07a.c:78) e `e605` con bit 5-7 a 0
(attract08.c:147): con DSW diversi dal default il gioco può raggiungere codice
non tradotto (vedi «Limite»). I DSW arrivano alla macchina SOLO per la scrittura
nel boot (non c'è altra sorgente): `gancio_porta` (macchina.c:167) manda e604 in
scrittura a `ribalta`, non in ram.

**Macchina** (da `grafo`/lettura di macchina.c, 635 righe): statici in
macchina.c: `maschera_nmi` (e600 bit 0 in scrittura), `ribalta`, `gancio_porta`,
`boot_2833()` (riga 325, fino a `posta_attivita(0x1f); posta_attivita(0x06)`),
`esegui_nmi(const struct riga_f *r)` (riga 238: sanità/saldatura,
`giro_video_campione(r->frame, ram, prefisso)`, porte vive, `sub_0066(&z)`,
`k_ff++`), `punto()` (riga 267, MACCHINA_PUNTO: inietta la NMI se il ritorno
dell'oracolo cade nell'intervallo e l'impronta coincide), `involucro()` (riga 406+,
sceglie il corpo da `corpo_attivita_replay[codice]`; se manca imposta
`corpo_mancante`), `giro_spedidore()` (ciclo a eventi D/N dell'oracolo),
`macchina_esegui(oracolo, disp, ingr, fine)` (riga ~590: configura
`macchina_punto`, `porte_gancio_scrittura`, `nemici_punto`,
`attract_punto_largo`, `sched_gancio_scansione`, `nemici_r_gancio`,
`corpo_attivita[i]`). Fuori da macchina.c: `spedisci_una(&slot,&codice)` (sched.c:24,
chiama `sched_gancio_scansione(lettura)` a ogni lettura della scansione, poi
`corpo_attivita[codice](slot)`), `corpo_attivita_replay[SCHED_CODICI]` (40),
`sub_0066(struct z80*)` (nmi.c:144), `carica_rom()`, `giro_dal_boot` (nemici.h:68,
gioco_libero.c lo accende prima di `macchina_esegui`), `nemici_r_gancio`
(nemici.h:360, sentinella del registro R: oggi ferma il giro).

**Hash e video** (da `giro_video.c`, `video_sdl.h`): `rendi_frame(const vid_ram*,
uint8_t rgb[224][256][3])`; `vid_ram` = sei puntatori a d000, d400, d800, dc00,
e000, e400 (si costruisce come `rendi_hash` in giro_video.c:28-38 o `dopo_nmi`
in mostra_replay.c:46). `vsdl_leggi_opzioni` gestisce `-s N`, `-n N` (headless),
`-m N`; `vsdl_apri(titolo, scala)`; `vsdl_presenta(fb)` rende, **fa il ritmo a
60 Hz** (vsync o ripiego, video_sdl.c:105-131), smaltisce gli eventi con
`SDL_PollEvent` e ritorna 1 su ESC/chiusura finestra; `vsdl_hash(fb)` FNV-1a
(non usa SDL); `SDL_Init(SDL_INIT_VIDEO)` è in vsdl_apri (video_sdl.c:71) e basta.
Conseguenza: la tastiera si legge con `SDL_GetKeyboardState(NULL)` DOPO
`vsdl_presenta` (il pompaggio eventi è già fatto lì); il joystick richiede
`SDL_InitSubSystem(SDL_INIT_JOYSTICK)` dopo `vsdl_apri`.

**Cadenza delle NMI** (misurato con script Python su `out/giroboot_disp.txt`,
5434 NMI, 106104 spedizioni):
- dopo il frame 700 ogni frame contiene da 1 a 30 spedizioni di corpi, media
  21,67; ma 2928 delle 4800 NMI dopo il frame 700 cadono FUORI dallo spedidore
  (3AF3-3B10), cioè a metà di un corpo (nel giro libero con oracolo 30 su queste
  sono ancora «a metà attività» divergenti, le altre si riproducono perché il
  corpo ha un punto di sincronia dove l'impronta coincide);
- il boot (frame 66-602) ha 537 NMI dentro `boot_2833` (test RAM, azzeramenti).
Quindi senza oracolo la NMI si può collocare solo ai confini (scansione, prima
del corpo) o ai punti `MACCHINA_PUNTO` del boot: l'istante vero dipende dai cicli
Z80, non modellati. La partita risultante è plausibile, non identica a MAME: è
la semantica scelta per il modo libero.

**Limite di copertura.** I corpi tradotti sono quelli che la partita registrata
ha eseguito (`corpo_attivita_replay`); un percorso diverso (altre stanze, altri
nemici, lettura del registro R, DSW diversi) può raggiungere codice non tradotto.
Il modo libero deve fermarsi in modo pulito (messaggio, rendi l'ultimo frame,
registrazione chiusa), non crollare. Il piano H5 non richiede di tradurre altro.

## Struttura da realizzare

### 1. Modifiche minime a `macchina.c` + nuovo `macchina.h`

- `macchina.h` (nuovo, ~25 righe): dichiara `macchina_esegui`, `boot_2833`,
  `gancio_porta`, `extern uint8_t maschera_nmi, ribalta;`,
  `extern uint8_t macchina_dsw1, macchina_dsw2;` (default 0x02, 0x00),
  `extern void (*macchina_prima_nmi)(long frame);` (default NULL).
- Rendere non statici `boot_2833`, `gancio_porta`, `maschera_nmi`, `ribalta`.
- Righe 329-330: `ram[0xe604 - 0xc000] = macchina_dsw1; ram[0xe605 - 0xc000] = macchina_dsw2;`.
- In `esegui_nmi`, subito dopo `giro_video_campione(...)` e dopo la scrittura
  delle porte vive, prima di `sub_0066`: `if (macchina_prima_nmi) macchina_prima_nmi(r->frame);`.
  Così il frame si rende nello stesso istante dell'hash di `giro_video.c` (RAM
  prima della NMI del frame). Nient'altro cambia: l'uscita di `gioco_libero` e
  di `giro_completo.sh` resta identica.

### 2. `macchina_libera.c` (nuovo, < 300 righe): macchina senza oracolo

Interfaccia (in `macchina.h`):
```c
struct libera_opz { long frame_max; /* 0 = illimitato */ };
/* Chiamata a ogni NMI PRIMA di sub_0066: scrive in v[0..2] gli ingressi del
 * frame (e600,e601,e602), rende/presenta. Ritorna 0 = continua, 1 = esci. */
extern int (*libera_frame)(long frame, uint8_t v[3]);
int macchina_libera_esegui(void);   /* 0 fine pulita, 3 fermo per codice non tradotto */
```
- Numero di frame: la prima NMI è il frame 66, poi 67, ... (stessa numerazione
  dei file di ingressi e di MAME).
- Prepara come `macchina_esegui`: `carica_rom()`, copia di f000-ffff in `ram[]`
  (macchina.c:603), `porte_gancio_scrittura = gancio_porta`, `giro_dal_boot = 1`.
- **Boot**: `macchina_punto = punto_libero` e `boot_2833()`. `punto_libero(da,a)`
  inietta la NMI (se `maschera_nmi` e non già in NMI) ogni `PUNTI_PER_NMI_BOOT`
  chiamate (costante da calibrare: obiettivo ~537 NMI nel boot come in MAME,
  tolleranza larga; deve solo terminare). La NMI libera = scrive le porte da
  `libera_frame`, `struct z80 z = {0}; sub_0066(&z);`, incrementa il frame.
- **Dopo il boot**: `macchina_punto = NULL`, `nemici_punto = NULL`,
  `sched_gancio_scansione = scansione_libera`, `nemici_r_gancio = r_letto_libero`
  (ferma pulito), `corpo_attivita[i] = involucro_libero` (stessa logica di
  `involucro`: se `codice >= SCHED_CODICI || !corpo_attivita_replay[codice]` →
  `fermo(...)`, altrimenti chiama il corpo). Ciclo: `while (1)` di
  `spedisci_una`; esito -1 (codice fuori tabella) o 0 prolungato → fermo.
- **Cadenza dopo il boot**: NMI al confine, in `scansione_libera` (a ogni lettura)
  e in `involucro_libero` (prima del corpo), quando `corpi_dal_frame >=
  CORPI_PER_FRAME` (22: media misurata 21,67) oppure `letture_dal_frame >=
  LETTURE_PER_FRAME` (a vuoto, tabella senza attività: scegliere ~ 2000). Le due
  costanti stanno in testa al file, commentate con la misura. NMI solo se
  `maschera_nmi`; mai dentro una NMI.
- `fermo(motivo)`: stampa `FERMO <motivo> al frame <f>` su stdout, imposta il
  codice 3, e ritorna al chiamante di `gioca` tramite `longjmp` o, più semplice,
  `exit(3)` dopo aver chiamato una funzione di chiusura registrata con `atexit`
  (chiude la registrazione, `vsdl_chiudi`, stampa il riepilogo). Scelta libera.
- Non usa `ev[]`, `ff[]`, `ingressi[]`, `k_ff` di macchina.c.

### 3. `gioca.c` (nuovo, < 350 righe): opzioni, ritmo, riepilogo

Opzioni (le proprie si tolgono da argv, il resto va a `vsdl_leggi_opzioni`):
- `--registra FILE` (scrive `"%ld %02X %02X %02X\n"` per ogni NMI renduta, nella
  stessa forma del file esistente; i test confrontano con `cmp`, quindi stesso
  formato esatto: frame decimale, byte maiuscoli a due cifre. Se il file di
  riferimento usa minuscole, si adegua: verificare con `head -3`: `66 00 00 00`);
  `--riproduci FILE` (ingressi dal file; frame mancante = `00 00 00`; finito il file
  gli ingressi sono 0); le due si possono dare insieme.
- `--oracolo ORACOLO.txt DISP.txt`: usa `macchina_esegui(oracolo, disp,
  ingressi, -1)` con `--riproduci` obbligatorio (qui gli ingressi li carica
  `macchina.c`; `macchina_prima_nmi` rende, stampa `HASH` e fa -n/-m). Serve
  anche `out/giroboot_NNN.bin` accanto all'oracolo (già prodotti da giro_boot.sh).
- `--hash-frame F[,F...]`: a quei frame stampa `HASH <F> <hex16>` (hash di
  `vsdl_hash` del frame reso).
- DSW: `--vite 2|3|4|5`, `--cabinet upright|cocktail`, `--demo on|off`,
  `--bonus 30k-200k-500k|100k-300k-800k|30k-200k|100k-300k|30k|100k|200k|nessuno`,
  `--difficolta facile|normale|dura|difficile`,
  `--timer lento|normale|veloce|velocissimo`, `--extra normale|difficile`,
  `--dsw1 HEX`, `--dsw2 HEX` (grezzi, vincono sui precedenti). Default `02`/`00`.
  `--stampa-dsw` stampa `DSW1 xx DSW2 yy` (maiuscole) ed esce 0 senza avviare. Il
  parsing dei DSW sta in `gioca_dsw.c` (nuovo, < 120 righe), funzione pura.
- `--mappa`: stampa la tabella tasti (sotto) ed esce 0 senza SDL.
- Con `-n N` (headless): N NMI rese, poi esce; senza finestra, quindi senza
  tastiera: gli ingressi vengono solo da `--riproduci` (altrimenti 00).
- Riepilogo finale su stdout (tutti i modi, anche su `FERMO`):
  ```
  FRAME rese <N>
  HASH ultimo <hex16>
  ```
  e, se c'è `--oracolo`, anche `FRAME ultimo <f>`. In modo oracolo il numero di
  frame della riga `HASH <F>` è quello dell'oracolo (66-5499).

### 4. `gioca_tasti.c` (nuovo, < 250 righe): tastiera e joystick

Funzione pura `uint8_t tasti_ingressi(const uint8_t *stato_sdl, uint8_t out[3])`
(nome libero) che usa gli scancode SDL; ingressi attivi alti. Tabella (anche
l'output di `--mappa`, una riga per tasto, formato `TASTO PORTA MASCHERA NOME`,
25 righe in questo ordine):

```
Right e600 01 destra      D e600 01 destra
Left  e600 02 sinistra    A e600 02 sinistra
Up    e600 04 su          W e600 04 su
Down  e600 08 giu         S e600 08 giu
X     e600 10 BUTTON2     LeftAlt e600 10 BUTTON2
Z     e600 20 BUTTON1     LeftCtrl e600 20 BUTTON1    Space e600 20 BUTTON1
1     e602 01 START1      Return  e602 01 START1
2     e602 02 START2
5     e602 04 COIN1       C       e602 04 COIN1
6     e602 08 COIN2
L e601 01 destra  J e601 02 sinistra  I e601 04 su  K e601 08 giu
M e601 10 BUTTON2  N e601 20 BUTTON1
```
(una riga per tasto: 13 + 6 + 6 = 25.) Opposti (destra+sinistra, su+giù):
si annullano entrambi (8WAY semplificato).
Joystick (facoltativo, senza obbligo, non verificabile headless): primo
joystick, asse X/Y oltre ±8000 e cappello = direzioni; pulsante 0 → BUTTON1 (0x20),
1 → BUTTON2 (0x10), 6 → COIN1, 7 → START1.
Quale dei due pulsanti è il salto e quale la magia non risulta dal sorgente
MAME (solo BUTTON1/BUTTON2): lo si annota in `GIRO.md` dopo la prova manuale.

Comandi: ESC (gestito da `vsdl_presenta`), **P** pausa (fronte di discesa; in
pausa si ripresenta l'ultimo frame con `SDL_Delay(16)` e nessuna NMI, la
registrazione non avanza), **F2** reset = `execv(argv[0], argv)` con gli stessi
argomenti (la registrazione riparte da zero; in modo `--riproduci` il reset non
si fa). Il callback `libera_frame` fa: lettura ingressi (tastiera/joystick o file),
registrazione, `rendi_frame` + `vsdl_hash` + (se non headless) `vsdl_presenta`.
Titolo finestra `"Solomon's Key - gioca"`; alla fine `vsdl_chiudi` e riepilogo.

### 5. Makefile

Aggiungere `gioca` a `BERSAGLI_SDL`, e:
```
SORGENTI_GIOCA = $(filter-out gioco_libero.c,$(SORGENTI_LIBERO)) gioca.c gioca_tasti.c gioca_dsw.c macchina_libera.c video_sdl.c
gioca: $(SORGENTI_GIOCA) macchina.h ram.h routine.h sched.h nemici.h e603.h rst20_tables.h rst38_tables.h video.h video_sdl.h giro_video.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ $(SORGENTI_GIOCA) $(SDL_LIBS)
```
e `gioca` in `clean`. Il binario non si committa (`git status --short | grep -c 'gioca$'` → `0`;
controllare `.gitignore`, aggiungere `decomp/harness/gioca` se gli altri binari vi compaiono).

### 6. Documenti

`decomp/GIRO.md`: sezione «H5: gioca» (uso, mappa tasti dai bit veri, DSW, cadenza
libera con le costanti scelte e i numeri misurati, differenza dal modo oracolo, limite
di copertura, quale pulsante è il salto se verificato). `compiti/PIANO-HARDWARE.md`:
H5 a `[x]` con una riga di esito. `decomp/harness` non ha README.

## Verifiche obbligatorie

Da `decomp/` salvo diversa indicazione. (m) = misurato dall'analista su strumenti
esistenti; (a) = attesa, non misurata (lo sviluppatore riporta l'uscita vera e,
se diversa, la spiega); (m-man) = manuale facoltativa. `mkdir -p /tmp/h49`.

1. Compilazione: `cd harness && make -B all gioca 2>&1 | grep -c -i -E 'warning|error'` → `0`.
2. Mappa: `harness/gioca --mappa | wc -l` → `25`; `harness/gioca --mappa | grep -c 'e600'` → `13`;
   `harness/gioca --mappa | grep -E '^(Z|Return|5) '` → tre righe con `e600 20 BUTTON1`,
   `e602 01 START1`, `e602 04 COIN1` (a).
3. DSW: `harness/gioca --stampa-dsw` → `DSW1 02 DSW2 00` (m: i valori del boot);
   `harness/gioca --vite 5 --cabinet cocktail --demo off --bonus 100k --stampa-dsw` →
   `DSW1 05 DSW2 A0`; `harness/gioca --vite 2 --bonus nessuno --timer velocissimo
   --difficolta difficile --extra difficile --stampa-dsw` → `DSW1 0E DSW2 FF`;
   `harness/gioca --dsw1 0F --dsw2 3 --stampa-dsw` → `DSW1 0F DSW2 03` (tutte a, calcolate
   dalla tabella dei bit sopra).
4. Modo oracolo, hash del giro completo: `harness/gioca --oracolo out/giroboot_oracolo.txt
   out/giroboot_disp.txt --riproduci out/ingressi_replay.txt -n 5500 --hash-frame 700,1300,5499
   | grep -E '^(HASH|FRAME)'` → `HASH 700 ecd5acf694285489`, `HASH 1300 72dcb8156116ea1a`,
   `HASH 5499 a239d0a940bec0b9` (valori m dalla tabella `campione[]` di giro_video.c, che il
   giro già verifica; resa nello stesso istante), poi `FRAME rese 5434`, `HASH ultimo
   a239d0a940bec0b9`, `FRAME ultimo 5499`; `echo $?` → `0`.
5. Modo libero, determinismo: `harness/gioca --riproduci out/ingressi_replay.txt -n 1500
   --hash-frame 700,1000 > /tmp/h49/a.txt; echo $?; harness/gioca ... > /tmp/h49/b.txt;
   cmp /tmp/h49/a.txt /tmp/h49/b.txt && echo identici` → `0` e `identici`; contiene
   `FRAME rese 1500` e nessuna riga `FERMO` (a). Poi con `-n 5434`: riportare
   `FRAME rese`, `HASH ultimo`, o la riga `FERMO <motivo> al frame <f>` con `echo $?` = `3`
   (a: il modo libero può fermarsi su un corpo non tradotto o sul registro R; è un esito
   ammesso purché pulito e riportato).
6. Schermata stabile del titolo: nell'esecuzione del punto 5 `HASH 700` e `HASH 1000`
   coincidono fra loro (a); confrontarli con `ecd5acf694285489` (hash MAME-oracolo dei frame
   700 e 1000, schermata del titolo) e riportare se coincidono (a: può coincidere solo se
   l'attesa del boot finisce prima di 700 con la stessa schermata).
7. Registra/riproduci a cerchio: `harness/gioca --riproduci out/ingressi_replay.txt --registra
   /tmp/h49/r.txt -n 1000 > /dev/null; head -1000 out/ingressi_replay.txt | cmp - /tmp/h49/r.txt
   && echo identici` → `identici` (a); `wc -l < /tmp/h49/r.txt` → `1000`; `head -1 /tmp/h49/r.txt`
   → `66 00 00 00`.
8. Ingressi sintetici: scrivere in `/tmp/h49/s.txt` le righe `100 00 00 04`, `101 00 00 04`,
   `130 00 00 01`, `131 00 00 01`, `200 01 00 00`, `201 02 00 00`; `harness/gioca --riproduci
   /tmp/h49/s.txt --registra /tmp/h49/s2.txt -n 300 >/dev/null`; `grep -E '^(100|130|200|201) '
   /tmp/h49/s2.txt` → le stesse quattro righe con tre byte; `wc -l < /tmp/h49/s2.txt` → `300` (a).
9. Finestra senza schermo: `SDL_VIDEODRIVER=dummy harness/gioca -m 120 --riproduci
   out/ingressi_replay.txt | tail -2` → `FRAME rese 120` e una riga `HASH ultimo`, exit 0 (a).
10. Regressioni invariate: `sh giro_completo.sh; echo "exit $?"` → `GIRO COMPLETO 0-5499:
    saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con
    hash coincidenti` e `exit 0` (m, brief 48); `cd harness && ./harness --tutte > /tmp/h49/b.txt;
    echo "exit $?"; grep -c FALLITO /tmp/h49/b.txt; grep -c '^RIEPILOGO' /tmp/h49/b.txt;
    grep -c '^cattura' /tmp/h49/b.txt` → `exit 0`, `0`, `307`, `1278` (m, brief 48);
    `harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 2>&1 |
    tail -4` → quattro righe di brief 48 (885 divergenze a metà attività, nessuna nello
    spedidore); `harness/sched_proto out/sched_log.txt | tail -1` → `spedizioni confrontate:
    106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`.
11. Ambito: `git diff --stat -- decomp/harness/replay.c decomp/harness/sched.c
    decomp/harness/video_sdl.c decomp/harness/video_sdl.h decomp/harness/video_rendi.c
    decomp/harness/mostra_replay.c decomp/harness/gioco_libero.c decomp/harness/giro_video.c`
    (dalla radice) → vuoto; `git diff --stat -- decomp/harness/macchina.c` → solo poche righe
    (< 25 inserzioni); `sha1sum out/ingressi_replay.txt` → `43934a8eeed8d44a071d7ab958d32ea74fbd437f`.
12. Dimensioni: `wc -l harness/*.c | awk '$1>800 && $2!="totale"'` → vuoto; `wc -l
    harness/macchina.c harness/macchina_libera.c harness/gioca.c harness/gioca_tasti.c
    harness/gioca_dsw.c` → ciascuno sotto 700.
13. `pgrep -c mame` → `0`. `git status --short` dalla radice → solo: `compiti/49-h5-input-sdl.md`,
    `compiti/PIANO-HARDWARE.md`, `decomp/GIRO.md`, `decomp/harness/{macchina.c,macchina.h,
    macchina_libera.c,gioca.c,gioca_tasti.c,gioca_dsw.c,Makefile}` (e `.gitignore` se toccato).
14. (m-man, facoltativa) `harness/gioca` senza argomenti: si apre la finestra a 3x, 60 Hz;
    5 inserisce la moneta, 1 avvia, frecce/WASD muovono, Z e X agiscono, P pausa, F2 riparte,
    ESC esce. Con `--registra /tmp/h49/m.txt` la partita si registra; `harness/gioca
    --riproduci /tmp/h49/m.txt -n 600` ne dà lo stesso hash due volte. Riportare quale tasto è il
    salto e fin dove si arriva prima di un `FERMO`.

## Consegna

Commit unico con messaggio «Lotto H5: gioca, tastiera, DSW, registra e riproduci» e riga
`Co-Authored-By: <modello> <noreply@anthropic.com>`. Rapporto finale, al massimo trenta righe:
uscita delle 14 verifiche, valori scelti di `PUNTI_PER_NMI_BOOT`, `CORPI_PER_FRAME`,
`LETTURE_PER_FRAME` e perché; frame a cui il modo libero si ferma (se si ferma) e il motivo;
hash liberi misurati dei frame 700, 1000 e ultimo; deviazioni. Se il boot libero non termina
con nessuna cadenza ragionevole: fermarsi, riportare l'evidenza (ultimo `punto` raggiunto, RAM
di c044/c04e/c04f) e non forzare.

## Domande aperte

1. **Scelta di progetto — cadenza delle NMI senza oracolo.** La NMI per frame al vblank non
   riproduce MAME (a metà corpo in 2928 casi su 4800). Proposta: ai confini (scansione e prima
   del corpo) con budget di 22 corpi per frame; la verifica dell'hash a239... si fa nel modo
   `--oracolo`. Alternativa più fedele ma costosa: modello a cicli Z80 per corpo (fuori portata
   di H5, tradurre il costo di ogni routine). Alternativa intermedia: usare anche i punti
   `NEMICI_PUNTO` come orologio (NMI a metà corpo ogni K punti), più vicina a MAME ma più fragile.
2. **Scelta di progetto — il modo `--oracolo` entra in H5.** Proposta: sì (costa un hook di
   quattro righe in `macchina.c` e dà la verifica esatta dell'hash richiesto). Senza, H5 non ha
   verifica a hash esatto.
3. **Scelta di progetto — copertura.** Il modo libero si ferma al primo corpo/lettura R non
   tradotti. Proposta: accettarlo in H5 (esito ammesso, riportato) e aprire un lotto separato di
   traduzione guidato dai punti di arresto trovati giocando.
4. **Preferenza — tasti.** Proposta: la tabella sopra (BUTTON1 = Z/Ctrl/Spazio, BUTTON2 = X/Alt,
   come i default di MAME).
5. **Preferenza — nomi delle opzioni DSW** in italiano come sopra, più `--dsw1/--dsw2` grezzi.
6. **Preferenza — reset F2 con `execv`** (riavvia il processo) invece di reinizializzare lo stato:
   gli statici dei corpi tradotti non hanno un reset esplicito.
7. **Preferenza — P2 (e601) mappato su IJKL/N/M** anche se il default DSW è Upright.
8. **Preferenza — resa prima della NMI** (stesso istante di `giro_video.c`), non dopo.

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Domande 1-3 sono scelte di progetto, risolte dalla chat principale con le
proposte dell'analista, coerenti coi precedenti ratificati; da segnalare
all'utente:
1. Sì: nel gioco libero le NMI cadono ai confini della scansione con un
   budget di ~22 corpi per frame (media misurata 21,67). È lo stesso
   criterio dell'audio: niente modello a cicli finché la differenza non
   si percepisce giocando; se giocando si sentisse sbagliato, si riapre.
2. Sì: il modo `--oracolo` entra in H5 — serve alla verifica esatta
   dell'hash del frame 5499.
3. Sì: il modo libero si ferma in modo pulito su codice non tradotto o
   lettura di R; le traduzioni ulteriori sono lotti a parte.
Preferenze 4-8, accolte con la proposta:
4. Sì: tabella dei tasti come proposta (frecce/WASD, Z/Ctrl/Spazio B1,
   X/Alt B2, 5 moneta, 1 start).
5. Sì: opzioni DSW leggibili in italiano più `--dsw1`/`--dsw2` grezzi.
6. Sì: F2 riavvia con `execv`.
7. Sì: giocatore 2 su IJKL con N/M.
8. Sì: resa prima della NMI, come `giro_video.c`.
