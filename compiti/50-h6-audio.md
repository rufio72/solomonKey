# Compito 50: H6 il gioco suona (raccordo audio)

Data: 2026-11-03. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Collegare il driver audio già tradotto (`decomp/harness/audio_*.c`) a `gioca`:
le scritture del gioco al latch `e800` diventano comandi al driver (latch +
NMI audio), il driver gira due IRQ per frame in passo col gioco, i cambi dei
registri YM2149 vanno a emu2149 che produce PCM a 44100 Hz mono; il PCM va al
callback SDL audio, oppure (modo senza finestra) a un hash. In più un oracolo
MAME nuovo della stessa partita (latch e cambi AY) e un confronto misurabile.

Vincoli assoluti:
- Solo dentro `/home/rufio72/OpenSolomonsKey-master`. Commenti, messaggi e
  documenti in italiano. Nessun file di codice sopra le 800 righe (i nuovi
  restano sotto le 250).
- INTATTI e senza modifiche di comportamento: banco (307 voci, 1278 catture),
  banco audio, `replay`, `sched_proto`, `giro_completo.sh`. I file `audio_*.c`
  e `audio.h` non si modificano (si aggiunge l'adattatore). `video_rendi.c` e
  `video_sdl.c/.h` non si toccano. `gioco_libero`, `replay`, `harness` non
  devono linkare né chiamare il nuovo codice.
- MAME sempre senza finestra e senza audio: `-video none -sound none`,
  `SDL_VIDEODRIVER=dummy`, tastiera/mouse/joystick/lightgun spenti (come
  `giro_boot.sh`). Non modificare `giro_boot.lua`, `replay.lua`, `audio.lua`.
- Non installare nulla. Build: `make -C decomp/harness` (SDL2 c'è:
  `sdl2-config` risponde).
- Niente git distruttivo (stash, filter-branch, reset).

## Cosa esiste già (fatti verificati)

Meccanismo vero (da `/home/rufio72/mame-src/solomon.cpp`, sola lettura):
- Il principale scrive `e800` (riga 222) → `sh_command_w` (160-173): scrive il
  `soundlatch` e fa `pulse_input_line(INPUT_LINE_NMI)` sull'audio. L'audio
  legge il latch a `8000`; NMI audio = `0066`.
- Z80 audio a 3,072 MHz con interrupt periodico `irq0_line_hold` a
  2*60 = 120 Hz (riga 361); schermo a 60 Hz (riga 366); YM2149 a 1,5 MHz,
  tre chip, uscita mono con peso 0,12 ciascuno.
- Quindi 2 IRQ audio per frame, e la NMI audio cade nel frame senza legame
  con la fase degli IRQ.

Misure fatte per questo brief (MAME, partita dei 5434 frame 66-5499, prove in
`/tmp/claude-1000/-home-rufio72-OpenSolomonsKey-master/b40ceafb-2c3b-41e9-86ed-809de85cf21e/scratchpad/proto/`,
file `pa.lua` = prototipo funzionante dell'oracolo, 3,7 s reali):
- Stessa partita di `giro_boot.sh` (-debug, `bpset 66`, contatore `frames` da
  `schermo:frame_number()`, ingressi di `replay.lua`): i frame NMI coincidono
  con le righe `F` di `out/giroboot_oracolo.txt` (cmp identico, 5434 righe).
  Deterministico: due corse danno file identici.
- Scritture a `e800` dal principale: 165 in tutta la partita, la prima al
  frame 1189 (valore 3F), poi 1305 (38), 1371 (36), 1399 (34), 1464 (35),
  1517 (06), 1531 (18), 1534 (3E), 1564 (3F); l'ultima al 5499. Tutte avvengono
  nella NMI del frame etichettato (`frames` al momento della scrittura).
- Cambi AY (formato di `audio.lua`): 20409 righe di dati, frame 0-5499.
- Fase: tempo MAME del latch dentro il frame etichettato = 0,38-0,54 di un
  periodo IRQ (1/120 s) dopo l'IRQ che apre il frame; il secondo IRQ del frame
  arriva dopo il latch. Ordine reale nel frame etichettato F:
  **IRQ A (0038 + passata 029E), latch/NMI 0066, IRQ B (0038 + 029E)**.
  Il banco audio (`audio_banco.c`) invece assume latch, IRQ, IRQ: per questo
  il suo oracolo `gioco` diverge dal frame 1189 (AUDIO.md, riga ~221).
- Misura del vecchio modello su questa partita (copia di `audio_banco` in una
  cartella temporanea, latch a inizio frame): primo frame diverso 1190; frame
  con insieme di cambi diverso 2565; frame con stato dei registri a fine frame
  diverso 1870. Con il latch spostato di +1 frame: 3828 e 3344. Il modello
  attuale non basta: serve la fase "A, latch, B".
- Nel C oggi: `nmi.c:259` fa `scrivi_porta(0xe800, suono)`; `porte.c`
  `scrivi_porta` chiama `porte_gancio_scrittura` se non nullo; il giro lo
  imposta a `gancio_porta` (`macchina.c:170`, imposta a 610; libera a
  `macchina_libera.c:109`), che per `e800` fa solo `ram[a-0xc000] = v`. Nessun
  comando audio è oggi registrato né usato.

Interfaccia del driver (da `grafo`/lettura di `audio.h`, `audio_piattaforma.c`,
`audio_suona.c`):
```
void aud_carica_rom(void);                 /* 1.3jk da ../roms/ext/solomon/ o ../../roms/ext/solomon/ */
void aud_usa_dati_estratti(int modo);      /* 1 = dati da audio_dati_*.c */
void aud_azzera(void);   void aud_avvio(void);
void aud_0038(void);     void aud_029e(void);    void aud_0066(uint8_t latch);
extern int aud_frame;    extern uint8_t aud_latch;
int  aud_ombra(int chip, int reg);         /* -1 = mai scritto; chip 1-3 */
int  aud_flusso_apri(const char *p);  void aud_flusso_chiudi(void);
extern void (*aud_osservatore)(int chip, int reg, int val);
```
Ciclo di `audio_suona.c`: `aud_azzera(); aud_frame=0; aud_avvio();` poi per
ogni frame `aud_frame=f;` `aud_0066(cc)` al frame del codice, e due volte
`aud_0038(); aud_029e();`. Il flusso scritto da `aud_out` ha righe
`frame\tchip\treg\tval` (chip hex, reg/val hex a 2 cifre) solo ai cambi.

Resa PCM esistente (`audio_wav.c`, solo `main`, non riutilizzabile come
libreria): `PSG_new(1500000, 44100)`, `PSG_setVolumeMode(p,1)`,
`PSG_setQuality(p,1)`, `PSG_reset`, 3 chip; per ogni frame lo stato di fine
frame si scrive con `PSG_writeReg(p, r, v)` solo dove cambia, registri 0-13,
registro 6 escluso dal giro e scritto solo se `(r7 & 0x38) != 0x38` e cambia
(r7 = stato scritto, `0xFF` se mai scritto); poi 735 campioni
`s = (calc0+calc1+calc2) * 7 / 8` saturati a int16. emu2149 sta in
`harness/emu2149/` (non modificare).

Frame: la macchina chiama `passo(frame, v)` in `gioca.c` (riga ~101) a ogni
frame, PRIMA della NMI di quel frame (modo libero da `frame_libero`, modo
oracolo da `frame_oracolo` via `macchina_prima_nmi`); la scrittura a `e800`
avviene durante la NMI, cioè dopo `passo` dello stesso frame. Il primo frame è
il 66. `gioca.c` 246 righe, `macchina.c` 640, `macchina.h` 27. Makefile:
`SORGENTI_GIOCA` (riga 80) e regola `gioca` (82); `SORGENTI_DRIVER` (riga 14)
è l'elenco audio da riusare; `audio_wav` mostra come si linka emu2149
(`emu2149/emu2149.c`).

Baseline delle regressioni (misurate oggi, da `decomp/`):
- `harness/harness --tutte` → 1278 righe `^OK`, 307 `RIEPILOGO`, nessuna
  FALL/DIFF.
- `harness/sched_proto out/sched_log.txt` → ultime righe «spedizioni
  confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti:
  131» e «nessuna divergenza nello spedidore».
- `harness/replay out/replay_oracolo.txt out/replay_disp.txt
  out/replay_2410.bin 2410` → termina con «nessuna divergenza nello
  spedidore».
- `sh giro_completo.sh` → «GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0
  divergenze, 30 a metà attività (attese), 13 frame resi con hash
  coincidenti».
- `harness/audio_banco cod_00_07` → «avvio 85 cambi, sequenza uguale» e «frame
  13-759 confrontati 747, primo frame diverso: nessuno».
- `harness/gioca -n 5434 --oracolo out/giroboot_oracolo.txt
  out/giroboot_disp.txt --riproduci out/ingressi_replay.txt` (7,8 s) →
  `FRAME rese 5434`, `HASH ultimo a239d0a940bec0b9`, `FRAME ultimo 5499`.
- `decomp/out/*` è ignorato da git: gli oracoli nuovi non si committano, si
  rigenerano con lo script.

## Struttura da realizzare

### 1. Oracolo MAME della partita (`decomp/partita_audio.lua`, `decomp/partita_audio.sh`)
Parti da `/…/scratchpad/proto/pa.lua` (se manca, ricostruiscilo da questa
descrizione) e da `giro_boot.lua` per contatore e ingressi. Lo script lancia
MAME come `giro_boot.sh` (stessi flag) con `-autoboot_script
partita_audio.lua`, `REPLAY_FINE` predefinito 5500, e scrive:
- `out/audio/partita_latch.txt`: intestazione `# frame\tvalore\torigine`, righe
  `%d\t%02X\tgioco` (tap di scrittura su `e800` del principale; `frames` come
  in `pa.lua`).
- `out/audio/partita_ay.txt`: intestazione `# frame\tchip\treg\tval`, righe
  `%d\t%X\t%02X\t%02X` (tap `io` dell'audiocpu, copia ombra per chip, solo ai
  cambi: stessa logica di `audio.lua` righe 71-91). Stesso nome-formato degli
  oracoli di `audio.lua`, quindi `audio_banco partita` e `audio_wav` lo leggono.
- Chiusura a `frames == 5500` con `manager.machine:exit()`; l'esito sullo
  stdout: `PARTITA latch 165 ay 20410`; uscita 1 se un file è vuoto.
Il `bpset 66` e il ciclo `register_periodic` servono solo a mantenere la
numerazione dei frame di `giro_boot.lua`: copiali uguali.

### 2. Adattatore scheda (`decomp/harness/scheda_audio.h/.c`, nuovi)
```
void scheda_audio_apri(const char *flusso_ay, const char *registro_latch); /* NULL = non scrivere */
void scheda_audio_latch(uint8_t v);          /* chiamata dal gancio di e800 */
void scheda_audio_frame(long frame);         /* da passo(): chiude frame-1, fa IRQ A di frame */
void scheda_audio_chiudi(void);              /* chiude l'ultimo frame, chiude i file */
int  scheda_audio_pcm(int16_t *buf);         /* vedi 3 */
extern int scheda_audio_fase;                /* 1 predefinita (A, latch, B); 0 = latch, A, B */
```
Semantica (per un frame etichettato F, con `aud_frame = F`):
`aud_0038(); aud_029e();` (A) — poi i latch scritti nella NMI del frame F in
ordine: `aud_latch = v; aud_0066(v);` e una riga nel registro latch — poi
`aud_0038(); aud_029e();` (B). Poiché i latch di F arrivano dopo `passo(F)`,
`scheda_audio_frame(F)` completa prima il frame F-1 (latch in coda, B) e poi
esegue A di F. Alla prima chiamata (F = 66): `aud_carica_rom()`,
`aud_usa_dati_estratti(1)`, `aud_azzera()`, `aud_frame=0; aud_avvio();` e
recupero dei frame 0..F-1 con soli A e B (nessun latch), come l'oracolo MAME
(avvio 85 cambi nei frame 0-13). `scheda_audio_chiudi()` completa il frame
corrente (latch + B). Con `scheda_audio_fase = 0` l'ordine è latch, A, B (per
la scansione delle fasi, passo 2 delle verifiche).

### 3. Resa PCM (`decomp/harness/scheda_pcm.c`, nuovo, dichiarato in `scheda_audio.h`)
Dopo ogni frame completo (dopo B), `scheda_audio_pcm` ritorna 735 campioni
int16 con la stessa procedura di `audio_wav.c` (vedi sopra), lo stato letto da
`aud_ombra(chip, reg)` (chip 1-3, reg 0-13; -1 = mai scritto). Mantiene
inoltre un FNV-1a 64 bit (offset `0xcbf29ce484222325`, primo `0x100000001b3`)
sui byte dei campioni little-endian di tutti i frame da 0 a quello chiuso,
catch-up compreso, e il contatore dei campioni. Funzioni:
`uint64_t scheda_audio_hash(void); long scheda_audio_campioni(void);`.

### 4. Gancio e800 (modifica minima)
`macchina.h`: `extern void (*macchina_latch_audio)(uint8_t v);` (dichiarazione
sotto `macchina_prima_nmi`). `macchina.c`: definizione a NULL e in
`gancio_porta` un ramo `else if (a == 0xe800) { ram[a - 0xc000] = v; if
(macchina_latch_audio) macchina_latch_audio(v); }` (mantiene il comportamento
attuale). Resta sotto le 650 righe.

### 5. gioca (`gioca.c`, `Makefile`, nuovo `gioca_audio.c`)
- `gioca.c`: in `passo()` (all'inizio) `scheda_audio_frame(frame)`, e subito
  dopo il pcm: `gioca_audio_spingi(buf, n)` solo per i frame da quello in cui
  si è iniziato a rendere (i 66 frame di recupero non vanno al dispositivo ma
  entrano nell'hash). In `finale()` prima delle stampe: `scheda_audio_chiudi()`
  e le righe `AUDIO campioni <n>` e `AUDIO hash <hex16>`. Imposta
  `macchina_latch_audio = scheda_audio_latch`.
- Opzioni nuove (in `gioca.c`, estratte da argv come `--registra`):
  `--muto` (nessun dispositivo SDL audio, il PCM si calcola comunque),
  `--flusso-ay F` (flusso dei cambi, formato di `audio.lua` senza `# codice`),
  `--registra-latch F` (formato di `partita_latch.txt`),
  `--fase-audio N` (0 o 1). Con `-n N` (senza finestra) l'audio è sempre muto.
  Aggiornare la riga `uso:`.
- `gioca_audio.c` (nuovo, SDL): `int gioca_audio_apri(void)` →
  `SDL_InitSubSystem(SDL_INIT_AUDIO)` + `SDL_OpenAudioDevice(NULL, 0, &want,
  &have, 0)` con `want.freq = 44100`, `AUDIO_S16SYS`, 1 canale, `samples =
  1024`, callback; ritorna 0 ok / -1 (in tal caso messaggio su stderr e si
  continua muti). Anello di 8 frame (5880 campioni) protetto da
  `SDL_LockAudioDevice`; il callback consuma e riempie di zeri se vuoto;
  `gioca_audio_spingi` scarta i campioni più vecchi se l'anello è pieno;
  `gioca_audio_chiudi`. Chiusura in `finale()` prima di `vsdl_chiudi()`.
  Durante la pausa (P) nessuna NMI: l'anello si svuota, silenzio.
- `Makefile`: `SORGENTI_GIOCA` += `scheda_audio.c scheda_pcm.c gioca_audio.c
  $(SORGENTI_DRIVER) emu2149/emu2149.c`; dipendenze `scheda_audio.h audio.h
  $(INTESTAZIONI_DATI) emu2149/emu2149.h`. Verificare che non ci siano simboli
  doppi (`audio_dati*` sono già in `SORGENTI_DRIVER`).

### 6. Confronto (`decomp/partita_audio_confronta.py`, nuovo, Python 3 senza dipendenze)
`python3 partita_audio_confronta.py <oracolo_ay.txt> <c_ay.txt> <da> <a>`.
Stampa, per i frame da `da` a `a`: `frame con insieme di cambi diverso N`
(insiemi di righe `chip reg val` ordinati, per frame), `frame con stato di
fine frame diverso M` (stato cumulato per `(chip,reg)`), `primo frame diverso
F|nessuno`, e `ordine interno diverso K` (frame con stesso insieme ma
sequenza diversa, solo informativo). Uscita 0 se N = M = 0, altrimenti 1.

### Metro scelto e perché
- Comandi: identità esatta della sequenza `(frame, valore)` col log MAME
  (165 righe): è l'ingresso del driver e non dipende da modelli del tempo.
- Cambi AY: confronto per frame sull'insieme dei cambi e sullo stato di fine
  frame (non sull'ordine interno: la passata 029E può essere spezzata dalla NMI
  e MAME scrive i chip in ordine 3,1,2 dove il C scrive 1,2,3, come già visto
  nell'oracolo `gioco`). Il metro è lo stesso del banco audio (frame per frame)
  ma contro un oracolo che nasce dalla stessa partita e contiene l'istante
  vero del latch. I `_ay.txt` esistenti non servono per il confronto diretto:
  nessuno coincide con la sequenza di latch di questa partita (`gioco` in
  `out/audio/gioco_*` è la corsa audio.lua di 2200 frame, con numerazione
  diversa di un frame).
- Audio reso: l'hash del PCM prodotto in `gioca` coincide con quello che
  `audio_wav` produce dal flusso scritto da `gioca` (stessa procedura, due
  strade); la corrispondenza col suono di MAME discende dal punto sopra.

## Verifiche obbligatorie (da `decomp/`)

1. `make -C harness` → nessun errore né avviso nuovo (CFLAGS `-Wall -Wextra`).
2. Oracolo: `sh partita_audio.sh` → riga `PARTITA latch 165 ay 20410`;
   `md5sum out/audio/partita_latch.txt` = `1480738c823a9b1c5aaa65a9e3269db5`;
   `md5sum out/audio/partita_ay.txt` = `c2db4b09eaf421ba95a11f3155764fe6`
   (misurati con `pa.lua`; lo script deve produrre byte identici; se le
   intestazioni differiscono, correggere lo script, non l'atteso). Ripetuto
   due volte: md5 identici.
3. Comandi: `harness/gioca -n 5434 --oracolo out/giroboot_oracolo.txt
   out/giroboot_disp.txt --riproduci out/ingressi_replay.txt --registra-latch
   /tmp/l.txt` poi `cmp /tmp/l.txt out/audio/partita_latch.txt` → silenzio
   (uguali). Stdout: `FRAME rese 5434`, `HASH ultimo a239d0a940bec0b9`,
   `FRAME ultimo 5499` (invariati). Se il C scrive una sequenza diversa dal
   latch di MAME, fermarsi e riportare la prima riga diversa.
4. Cambi AY: stessa corsa con `--flusso-ay /tmp/c_ay.txt`, poi per `--fase-audio`
   in 0 e 1 (e 2 se aggiunta dallo sviluppatore): `python3
   partita_audio_confronta.py out/audio/partita_ay.txt /tmp/c_ay.txt 14 5499`.
   Baseline misurata del modello vecchio (fase 0): insiemi diversi 2565, stati
   diversi 1870, primo frame 1190. Atteso con la fase 1 (A, latch, B): non
   misurato; l'obiettivo è N = M = 0 (uscita 0). Riportare i tre numeri per
   ogni fase provata e fissare nello script `partita_audio.sh` (modo
   `confronta`) la fase scelta e i valori misurati come atteso esatto.
5. Hash audio headless e ripetibilità: `harness/gioca -n 5434 --oracolo …
   --riproduci …` stampa `AUDIO campioni 4042500` (5500 frame × 735;
   attesa, non misurata) e `AUDIO hash <hex16>`; due corse → hash identico.
   Il valore va annotato in `decomp/GIRO.md` (sezione H6) come regressione.
6. Coerenza con `audio_wav`: `harness/audio_wav /tmp/c_ay.txt /tmp/c.wav 0
   5499`, poi hash FNV-1a 64 dei byte del file oltre i primi 44 (header
   canonico; verificare con `xxd` la dimensione `44 + 4042500*2`) uguale a
   `AUDIO hash` del punto 5. Scrivere il calcolo in un piccolo script Python
   (`decomp/partita_audio_confronta.py --hash-wav F.wav`).
7. Dispositivo SDL: `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy harness/gioca
   -m 300 --oracolo … --riproduci …` → esce 0; stderr senza errori audio e
   riga `AUDIO dispositivo 44100 Hz mono` (attesa, non misurata). Con
   `--muto` la riga non compare. Ascolto dal vivo (`harness/gioca` con
   finestra e audio dell'host): prova manuale facoltativa, non automatica.
8. Regressioni (uscite di baseline sopra): `harness/harness --tutte` (1278
   `^OK`, 307 `RIEPILOGO`), `harness/sched_proto out/sched_log.txt`,
   `harness/replay …` e `sh giro_completo.sh` (riga GIRO COMPLETO invariata),
   `harness/audio_banco cod_00_07` (riga invariata), `sh audio_wav.sh oracoli`
   (stesso esito di prima: eseguirlo prima di cominciare per averlo come
   riferimento).
9. Ambito: `git status --short` elenca solo file nuovi e le modifiche a
   `gioca.c`, `macchina.c`, `macchina.h`, `Makefile`, `GIRO.md`, `STATO.md`,
   `.gitignore` se serve per i binari; `git diff --stat` non tocca `audio_*.c`,
   `audio.h`, `video_*`.
10. Dimensioni: `find decomp -name '*.c' -o -name '*.h' | xargs wc -l | awk
    '$1>800 && $2!="totale"'` → vuoto, oppure solo i file dati già oltre
    prima di cominciare (misurarlo prima e riportarlo).

## Consegna

- File nuovi: `partita_audio.lua`, `partita_audio.sh`,
  `partita_audio_confronta.py`, `harness/scheda_audio.h/.c`,
  `harness/scheda_pcm.c`, `harness/gioca_audio.c`; modificati `gioca.c`,
  `macchina.c/.h`, `Makefile`; documenti: sezione «H6» in `decomp/GIRO.md`
  (meccanismo latch/NMI, fase scelta con i numeri, hash audio, opzioni nuove),
  riga in `STATO.md`, casella H6 in `compiti/PIANO-HARDWARE.md`, riga
  nell'elenco dei bersagli di `README.md` se descrive `gioca`.
- Commit con messaggio «Lotto H6: il gioco suona (…)» e riga
  `Co-Authored-By: <modello> <noreply@anthropic.com>`. Nessun push.
- Rapporto finale: massimo trenta righe, con l'uscita delle dieci verifiche, i
  numeri delle fasi provate e le deviazioni.

## Domande aperte

1. **Scelta di progetto.** Se nessuna fase (0, 1, 2) azzera i frame diversi
   del confronto AY, che cosa si accetta? Proposta: il lotto si chiude con il
   numero minimo misurato fissato come atteso esatto, elenco dei residui per
   classe in GIRO.md, e un secondo passo (H6b) per modellare la durata della
   passata 029E; il suono resta giocabile. Alternativa: non chiudere H6.
2. **Preferenza.** Lotto unico (oracolo + adattatore + PCM + SDL)? Proposta:
   sì, circa 600 righe nuove totali; se la fase non converge, consegnare
   comunque oracolo, adattatore, hash e confronto e rimandare SDL a H6b.
3. **Preferenza.** Dati del driver: `aud_usa_dati_estratti(1)` (sorgente C
   generato) in `gioca`, la ROM `1.3jk` serve comunque per il codice
   0000-05C5 e si legge da `roms/ext/solomon/`. Proposta: sì.
4. **Preferenza.** Catch-up dei 66 frame iniziali: nell'hash sì, al
   dispositivo SDL no. Proposta: sì.
5. **Preferenza.** Anello audio di 8 frame, buffer SDL da 1024 campioni,
   `--muto` e niente opzione di volume (H7 la prevede). Proposta: sì.
6. **Preferenza.** Oracolo in script nuovo (`partita_audio.lua`) invece di
   estendere `giro_boot.lua`, per non toccare l'oracolo del giro. Proposta: sì.

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Domanda 1 (scelta di progetto, risolta dalla chat principale col criterio
già ratificato dall'utente per la catena audio — il giudizio finale è
all'ascolto, niente modello a cicli salvo differenze udibili; da segnalare
all'utente): se nessuna fase azzera i frame diversi, H6 si chiude col
minimo misurato come atteso esatto (così la regressione resta
deterministica) e la durata della passata 029E va a un eventuale H6b;
se all'ascolto il risultato stona, H6b si apre davvero.
Preferenze 2-6, accolte con la proposta:
2. Sì: lotto unico (~600 righe); SDL slitta a H6b solo se la fase non
   converge.
3. Sì: dati del driver estratti (`aud_usa_dati_estratti(1)`), ROM `1.3jk`
   solo per il codice.
4. Sì: i 66 frame di recupero iniziali entrano nell'hash ma non nel
   dispositivo SDL.
5. Sì: anello audio di 8 frame, buffer SDL da 1024 campioni, `--muto`;
   il volume arriva con H7.
6. Sì: oracolo in uno script Lua nuovo, `giro_boot.lua` intatto.
