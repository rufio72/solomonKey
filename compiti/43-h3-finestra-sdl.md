# Compito 43: H3, finestra SDL2 (modulo video_sdl, mostra_oracolo, mostra_replay)

Data: 2026-11-03. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Prerequisito: SDL2 va installato PRIMA dello sviluppo (non dallo sviluppatore)

Misurato: `sdl2-config` assente, `/usr/include/SDL2/SDL.h` assente,
`pkg-config sdl2` assente. È installata solo la libreria a runtime
(`libsdl2-2.0-0 2.30.0+dfsg-1ubuntu3.1`); manca `libsdl2-dev` (candidato
apt: `2.30.0+dfsg-1ubuntu3.1`). Lo installa la chat principale, con sudo:
`sudo -S apt-get install -y libsdl2-dev < ~/.claude/sudo.pass`.
Lo sviluppatore NON installa nulla: come prima azione esegue
`sdl2-config --version`; se non risponde `2.30.0` si ferma e lo riporta.

## Obiettivo

Lotto H3 di `compiti/PIANO-HARDWARE.md`. Un modulo `video_sdl.c` che mostra in
una finestra SDL2 il framebuffer 256x224 RGB di `rendi_frame` (texture
streaming, 60 Hz con vsync, ripiego a temporizzazione software, scala intera
configurabile), e due programmi dimostrativi: `mostra_oracolo <stato>` (stato
campione di `decomp/out/video/`) e `mostra_replay` (i 3090 frame del giro
replay 2410-5499, rendendo la RAM video a ogni NMI). Entrambi hanno una
modalità senza finestra che stampa gli hash del primo e dell'ultimo frame, così
le verifiche restano a script e a uscita esatta.

## Vincoli assoluti

- Lavoro solo in `/home/rufio72/OpenSolomonsKey-master`. File da creare o
  toccare: quelli di «Struttura da realizzare», più `decomp/VIDEO.md` (sezione
  nuova «Finestra SDL (H3)»), `decomp/harness/Makefile`, `.gitignore`, una
  riga in `STATO.md` (punto 3, strato hardware, dopo la riga di H2) e
  l'esito in fondo a questo brief. `compiti/PIANO-HARDWARE.md`: spuntare H3.
- `rendi_frame` (`video_rendi.c`), `video.h`, `rendi_oracolo.c`: NON si
  toccano. Banco (304 voci, 1270 catture, 0 fallite), replay e sched_proto
  INTATTI nelle uscite (verifiche 8-10). L'unica modifica ammessa a un
  sorgente esistente del motore è il gancio di `replay.c` descritto sotto.
- Il Makefile compila i bersagli SDL solo se SDL2 c'è; senza SDL gli altri
  bersagli (`harness sched_proto replay audio_* rendi_oracolo video_immagine`)
  devono costruirsi come prima.
- MAME non serve e non si lancia. Non installare pacchetti. Niente git
  distruttivo (stash, reset, rebase, checkout di file).
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe (obiettivo 650). `gcc` con `-std=c11 -Wall -Wextra -O0`, zero
  warning; con `-std=c11` stretto `getopt_long` non è dichiarato: l'analisi
  delle opzioni si scrive a mano.
- PROVA fuori dal repository (in `/tmp`) quello che si può provare prima di
  inserirlo, in particolare la sequenza SDL (finestra, texture, present) con
  `SDL_VIDEODRIVER=dummy`.

## Cosa esiste già (fatti verificati)

| Cosa | Dove |
|---|---|
| `rendi_frame(const vid_ram *ram, uint8_t rgb[VID_ALTO][VID_LARGO][3])`; `vid_ram` = sei puntatori `fg_colorram fg_videoram bg_colorram bg_videoram spriteram palette` (1024,1024,1024,1024,128,512 B); `VID_LARGO 256`, `VID_ALTO 224` | `decomp/harness/video.h` (39 righe, letto) |
| Indirizzi Z80: FG colorram d000, FG videoram d400, BG colorram d800, BG videoram dc00, spriteram e000, palette e400 | `decomp/VIDEO.md` |
| Lettura dei sei `.bin` di uno stato + PPM: funzioni `static` `leggi_bin` ecc. (da copiare, non da condividere) | `decomp/harness/rendi_oracolo.c` (87 righe, letto) |
| 7 stati: `titolo gioco stanza10 bonus finale finale2 prove`, file `out/video/<S>_{fg_colorram,fg_videoram,bg_colorram,bg_videoram,spriteram,palette}.bin` | `decomp/out/video/` |
| RAM del gioco: `extern uint8_t ram[0x4000]` (c000-ffff), `M(a)` = `ram[(uint16_t)(a)-0xc000]`; quindi d000 = `&ram[0x1000]`, e000 = `&ram[0x2000]`, e400 = `&ram[0x2400]` | `decomp/harness/ram.h` |
| La RAM del replay copre c000-e5ff, video RAM compresa: l'impronta FNV-1a confrontata con MAME a ogni NMI è calcolata su c000-e5ff (esclusi c040-c043, cf00-cfff) | `replay.c:32-47` |
| La NMI tradotta (`sub_0066`) copia palette (c994→e400, c9ca→e500, code e4c6/e5d4) e tabella sprite (c8ee→e020, 0x60 byte) nella video RAM | `nmi.c:153-169` |
| Punto unico di esecuzione della NMI nel replay: `static void esegui_nmi(const struct evento *n)` (`replay.c:195`); in coda scrive e600-e602 da `r->p[]` e chiama `sub_0066(&z);` (penultima istruzione, prima della `}`); `r` = `&ff[k_ff++]`, `r->frame` è il numero di frame | `replay.c:195-247` (letto) |
| `int main(int argc, char **argv)` del replay (riga 323, file 457 righe); uso `replay <oracolo.txt> <disp.txt> <dump_F0.bin> <F0> [frame_fine] [--oltre]`; ritorna 0 se nessuna divergenza nello spedidore | `replay.c:323-457` |
| `esegui_nmi` è chiamata 3090 volte in un giro 2410-5499 («NMI confrontate: 3090»): una per frame | uscita misurata di `replay` |
| Makefile: `CC = gcc`, `CFLAGS = -std=c11 -Wall -Wextra -O0`; `SORGENTI_REPLAY = $(filter-out harness.c,$(SORGENTI)) sched.c replay.c`; `SORGENTI_GRAFICA = $(wildcard video_dati_*.c)`; `all:` e `clean:` elencano i binari a mano; regola `rendi_oracolo: rendi_oracolo.c video_rendi.c $(SORGENTI_GRAFICA) video.h` | `decomp/harness/Makefile` (letto) |
| `.gitignore`: i binari sono righe `decomp/harness/<nome>` (ultime: `rendi_oracolo`, `video_immagine`) | `.gitignore:76-77` |
| Dati del replay in `decomp/out/`: `replay_oracolo.txt`, `replay_disp.txt`, `replay_2410.bin` | `ls` |
| `README.md` risulta già modificato (`M`) prima di questo lotto: non è vostro, non toccarlo | `git status --short` |

Hash dei pixel dei sette stati (FNV-1a 64 bit sui 172032 byte RGB riga per
riga, calcolati con PIL dal PNG di MAME, quindi attesi per `rendi_frame` che è
identico pixel per pixel; misurati):
`titolo ecd5acf694285489`, `gioco 00d246e511baf342`,
`stanza10 58e83a3b9ab4aadb`, `bonus 03c860c73b428be3`,
`finale 9a80a5a4eda714ae`, `finale2 624f26bc6c885510`,
`prove 8dd4ee80a955743a`.
Calcolo di riferimento in Python, da usare nella verifica 3:
`h=0xcbf29ce484222325; per ogni byte b: h=((h^b)*0x100000001b3)&(2**64-1)`.

## Struttura da realizzare

Tutti in `decomp/harness/`.

### 1. `video_sdl.h` / `video_sdl.c` (~200 righe)

```c
#include "video.h"
typedef struct { int scala; long headless; long massimo; } vsdl_opz;
/* -s/--scala N (1-8, predefinita 3), -n/--headless N (N>=1, senza finestra,
 * SDL non viene nemmeno inizializzata), -m/--max N (con finestra, esce dopo
 * N frame; serve alle prove con SDL_VIDEODRIVER=dummy). -n e -m si escludono.
 * Ritorna l'indice del primo argomento non opzione, -1 se errore (messaggio
 * in italiano su stderr). Azzerata = finestra, scala 3, nessun limite. */
int vsdl_leggi_opzioni(int argc, char **argv, vsdl_opz *o);
int vsdl_apri(const char *titolo, int scala);   /* 0 ok, -1 errore */
/* Mostra il frame (SDL_UpdateTexture + RenderCopy + RenderPresent), smaltisce
 * gli eventi. 0 = continua; 1 = l'utente vuole uscire (ESC o chiusura finestra);
 * -1 = errore. */
int vsdl_presenta(const uint8_t fb[VID_ALTO][VID_LARGO][3]);
int vsdl_vsync(void);                            /* 1 se il vsync è attivo */
void vsdl_chiudi(void);
/* FNV-1a 64 bit sui VID_ALTO*VID_LARGO*3 byte, riga per riga. Non usa SDL. */
uint64_t vsdl_hash(const uint8_t fb[VID_ALTO][VID_LARGO][3]);
```

Dettagli di `vsdl_apri`: `SDL_Init(SDL_INIT_VIDEO)`; hint
`SDL_HINT_RENDER_SCALE_QUALITY` = `"0"` (pixel netti); finestra
`256*scala x 224*scala`; renderer con `SDL_RENDERER_ACCELERATED |
SDL_RENDERER_PRESENTVSYNC`; se la creazione fallisce, secondo tentativo con
`SDL_RENDERER_SOFTWARE`. Dopo la creazione `SDL_GetRendererInfo`: se manca
`SDL_RENDERER_PRESENTVSYNC` nei flag, il vsync non c'è e `vsdl_presenta`
applica il ripiego: tiene il tempo con `SDL_GetPerformanceCounter` e attende
(`SDL_Delay`) fino a 1/60 s dal present precedente (nessuna deriva: la
scadenza avanza di 1/60 s, non di «ora + 1/60»). Texture
`SDL_PIXELFORMAT_RGB24`, `SDL_TEXTUREACCESS_STREAMING`, 256x224; si aggiorna con
`SDL_UpdateTexture(tex, NULL, fb, VID_LARGO*3)`. SDL non cambia i pixel: il
framebuffer passato è esattamente quello che `rendi_frame` ha prodotto.

### 2. `mostra_oracolo.c` (~110 righe)

`mostra_oracolo [-s N] [-n N | -m N] [-d cartella] <stato>`; `-d` predefinita
`out/video` (si lancia da `decomp/` come `rendi_oracolo`). Legge i sei `.bin`
(copia di `leggi_bin`, uscita 2 con messaggio in caso di errore), rende UNA
volta con `rendi_frame`, poi:
- con finestra: ripresenta lo stesso frame a 60 Hz finché ESC/chiusura (o
  `-m N` frame);
- `-n N`: nessuna finestra, nessuna chiamata SDL; N "frame" = stesso frame
  contato N volte.
In entrambe le modalità, all'uscita stampa su stdout esattamente tre righe:
```
HASH primo <16 cifre esadecimali minuscole>
HASH ultimo <16 cifre esadecimali minuscole>
FRAME rese <N>
```
(per `mostra_oracolo` primo = ultimo). Exit 0; 2 per errori d'uso o di lettura.

### 3. Gancio in `replay.c` (modifica minima, solo aggiunte)

Prima di `esegui_nmi` aggiungere:
```c
/* Gancio opzionale (lotto H3): chiamato a fine NMI, con la RAM del frame
 * completa. NULL nel replay normale: nessun effetto sull'uscita. */
void (*replay_dopo_nmi)(long frame) = NULL;
```
e subito dopo `sub_0066(&z);` in `esegui_nmi`:
`if (replay_dopo_nmi) replay_dopo_nmi(r->frame);`. Nient'altro: niente
`static`, nessun cambio di semantica, ordine delle chiamate invariato.

### 4. `mostra_replay.c` (~150 righe)

`mostra_replay [-s N] [-n N | -m N] [-o cartella]`; `-o` predefinita `out`
(contiene `replay_oracolo.txt replay_disp.txt replay_2410.bin`).
`main` costruisce `argv` per `replay_main`:
`{"replay", "<o>/replay_oracolo.txt", "<o>/replay_disp.txt", "<o>/replay_2410.bin", "2410"}`
(argc 5), installa `replay_dopo_nmi` e chiama `replay_main`. L'unico `main`
del file è quello di `mostra_replay.c`: `replay.c` si compila a parte con
`-Dmain=replay_main` (vedi Makefile). Il gancio:
1. costruisce `vid_ram` con i puntatori dentro `ram` (`&ram[0x1000]` FG
   colorram, `&ram[0x1400]` FG videoram, `&ram[0x1800]` BG colorram,
   `&ram[0x1c00]` BG videoram, `&ram[0x2000]` spriteram, `&ram[0x2400]`
   palette — da `d000-c000` ecc.), chiama `rendi_frame`;
2. conta i frame resi, memorizza hash e numero di frame del primo e
   dell'ultimo (`vsdl_hash`);
3. con finestra: `vsdl_presenta`; se restituisce 1 o -1, o se si raggiunge
   `-m`, riepilogo e `exit(0)`; con `-n N`: raggiunto N, riepilogo ed `exit(0)`
   (senza finestra e senza SDL).
Il riepilogo (stesso dopo `replay_main`, se il giro finisce prima del limite
o senza limite) stampa su stdout tre righe come sopra più, subito prima di
`FRAME rese`, `NMI primo <f> ultimo <f>` con i numeri di frame veri. Il
replay stampa anche le sue righe (`SANITA F0 …`, ecc.): le verifiche filtrano
con `grep -E '^(HASH|FRAME|NMI primo)'`. Il valore di ritorno di `replay_main`
è l'exit di `mostra_replay` quando il giro finisce da solo.

Cosa può non essere completo (documentarlo in `VIDEO.md`, non risolverlo): le
1604 NMI «a metà attività» (885 con impronta divergente da MAME) possono
mostrare la RAM del C in uno stato appena diverso da quello di MAME; non è un
difetto del renderizzatore. Se all'ispezione manuale mancano elementi, verifica
che `ram[]` copra l'area (sì: c000-ffff) e annota cosa manca.

### 5. Makefile

Aggiungere, prima di `all:`:
```make
# strato video SDL2 (lotto H3): i bersagli SDL esistono solo se sdl2-config risponde
SDL2_CONFIG ?= sdl2-config
HA_SDL := $(shell $(SDL2_CONFIG) --version 2>/dev/null)
ifneq ($(HA_SDL),)
BERSAGLI_SDL = mostra_oracolo mostra_replay
SDL_CFLAGS := $(shell $(SDL2_CONFIG) --cflags)
SDL_LIBS := $(shell $(SDL2_CONFIG) --libs)
endif
```
`all:` aggiunge `$(BERSAGLI_SDL)`; `clean:` rimuove sempre
`mostra_oracolo mostra_replay replay_lib.o`. Regole:
```make
mostra_oracolo: mostra_oracolo.c video_sdl.c video_sdl.h video_rendi.c $(SORGENTI_GRAFICA) video.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ mostra_oracolo.c video_sdl.c video_rendi.c $(SORGENTI_GRAFICA) $(SDL_LIBS)

replay_lib.o: replay.c ram.h routine.h sched.h nemici.h e603.h rst20_tables.h rst38_tables.h
	$(CC) $(CFLAGS) -Dmain=replay_main -c -o $@ replay.c

mostra_replay: mostra_replay.c video_sdl.c video_sdl.h video_rendi.c $(SORGENTI_GRAFICA) replay_lib.o $(filter-out replay.c,$(SORGENTI_REPLAY)) video.h ram.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ mostra_replay.c video_sdl.c video_rendi.c $(SORGENTI_GRAFICA) replay_lib.o $(filter-out replay.c,$(SORGENTI_REPLAY)) $(SDL_LIBS)
```
(le regole si scrivono sempre; senza SDL non sono in `all`, quindi non
vengono tentate). In `.gitignore` aggiungere `decomp/harness/mostra_oracolo`,
`decomp/harness/mostra_replay`, `decomp/harness/replay_lib.o`.

## Verifiche obbligatorie

Comandi da `/home/rufio72/OpenSolomonsKey-master/decomp` salvo dove indicato.

1. SDL presente: `sdl2-config --version` → `2.30.0` (dopo l'installazione
   della chat principale; attesa, versione candidata apt).
2. Compilazione: `cd harness && make all 2>&1 | tee /tmp/m43.txt; echo "exit ${PIPESTATUS[0]}"; grep -c -i -E 'warning|error' /tmp/m43.txt` →
   `exit 0` e `0`. Poi `ls mostra_oracolo mostra_replay` → entrambi presenti.
3. Hash dei sette stati (senza finestra, a uscita esatta):
   `for S in titolo gioco stanza10 bonus finale finale2 prove; do harness/mostra_oracolo -n 1 $S | sed -n 1p; done` →
   ```
   HASH primo ecd5acf694285489
   HASH primo 00d246e511baf342
   HASH primo 58e83a3b9ab4aadb
   HASH primo 03c860c73b428be3
   HASH primo 9a80a5a4eda714ae
   HASH primo 624f26bc6c885510
   HASH primo 8dd4ee80a955743a
   ```
   (misurati dal PNG con PIL; quindi lo stesso `rendi_frame` identico a MAME
   deve dare questi). Inoltre `harness/mostra_oracolo -n 3 titolo; echo "exit $?"` →
   ```
   HASH primo ecd5acf694285489
   HASH ultimo ecd5acf694285489
   FRAME rese 3
   exit 0
   ```
4. Percorso SDL completo senza schermo:
   `SDL_VIDEODRIVER=dummy harness/mostra_oracolo -m 5 -s 2 gioco; echo "exit $?"` →
   ```
   HASH primo 00d246e511baf342
   HASH ultimo 00d246e511baf342
   FRAME rese 5
   exit 0
   ```
   (apre finestra 512x448, texture streaming, 5 present; con il driver dummy
   il vsync normalmente manca e passa dal ripiego software: se lo sviluppatore
   misura il contrario lo riporta, l'uscita non cambia.) Errori d'uso:
   `harness/mostra_oracolo -n 1 inesistente; echo "exit $?"` → messaggio su
   stderr e `exit 2`; `harness/mostra_oracolo -s 0 -n 1 gioco; echo "exit $?"` →
   `exit 2`.
5. Tempo del ripiego a 60 Hz: `SDL_VIDEODRIVER=dummy` con `-m 60` deve durare
   da 0,9 a 1,2 secondi se il vsync manca:
   `s=$(date +%s.%N); SDL_VIDEODRIVER=dummy harness/mostra_oracolo -m 60 gioco >/dev/null; e=$(date +%s.%N); echo "$e - $s" | bc` → valore fra `0.9` e `1.2`
   (attesa, non misurata). Se il driver dummy ha il vsync (risulta da
   `vsdl_vsync()`) la durata comunque non deve scendere sotto 0,9 s; se lo fa,
   è un difetto del ripiego.
6. Replay senza finestra:
   `harness/mostra_replay -n 100 | grep -E '^(HASH|FRAME|NMI primo)'` → quattro
   righe: `HASH primo <h1>`, `HASH ultimo <h2>`, `NMI primo <f1> ultimo <f2>`,
   `FRAME rese 100`, con `h1 != h2` e `f2 > f1 >= 2410` (hash e frame non
   misurabili prima di costruire il programma: lo sviluppatore li copia nel
   rapporto). Ripetuto due volte le due uscite sono identiche
   (`diff` vuoto). `harness/mostra_replay -n 1 | grep -E '^HASH'` → due righe
   con lo stesso valore.
7. Replay intero: `harness/mostra_replay -n 100000 | grep -E '^(FRAME|NMI primo)'; echo "exit ${PIPESTATUS[0]}"` →
   `NMI primo <f1> ultimo 5499`, `FRAME rese 3090`, `exit 0` (attesa: una
   resa per NMI, 3090 come «NMI confrontate»; non misurata). Il primo hash
   di questa corsa è uguale a quello della verifica 6 (`diff` vuoto sulle
   righe `HASH primo`).
8. Replay storico intatto (il binario `replay`, ricostruito, non cambia
   uscita): `harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r43.txt 2>&1; echo "exit $?"; tail -4 /tmp/r43.txt` → `exit 0` e
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
   (misurato oggi; `sha1sum` dell'intero `/tmp/r43.txt` lanciato da `harness/`
   con percorsi `../out/...`: `77925e67f3a60bb3b2a3acbe73dbae979869e55b`.)
9. Banco: `cd harness && ./harness --tutte > /tmp/h43.txt; echo "exit $?"; grep -c FALLITO /tmp/h43.txt; grep -c '^RIEPILOGO' /tmp/h43.txt; grep -c '^cattura' /tmp/h43.txt` →
   `exit 0`, `0`, `304`, `1270`.
10. `harness/sched_proto out/sched_log.txt | tail -1` →
    `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`.
11. Il renderizzatore è intatto e ancora 7/7: `git diff --stat -- decomp/harness/video_rendi.c decomp/harness/video.h decomp/harness/rendi_oracolo.c` → vuoto; `sh rendi_video.sh; echo "exit $?"` → `RENDI 7/7 identici`, `exit 0`.
12. Modifica a `replay.c` minima: `git diff --numstat -- decomp/harness/replay.c` → `<a>	0	decomp/harness/replay.c` con `a` tra 4 e 6 e zero righe rimosse.
13. Guardia SDL: `cd harness && make -n all SDL2_CONFIG=/bin/false | grep -c mostra_` → `0`; `make -n -B all | grep -c 'mostra_oracolo'` → almeno `1`. Inoltre `make -B rendi_oracolo video_immagine SDL2_CONFIG=/bin/false 2>&1 | grep -c -i -E 'warning|error'` → `0`.
14. Righe: `wc -l harness/video_sdl.c harness/video_sdl.h harness/mostra_oracolo.c harness/mostra_replay.c harness/replay.c | awk '$1>800 && $2!="totale"'` → vuoto.
15. Ambito (dalla radice): `git status --short` → solo `M`: `.gitignore`,
    `STATO.md`, `compiti/PIANO-HARDWARE.md`, `decomp/VIDEO.md`,
    `decomp/harness/Makefile`, `decomp/harness/replay.c`, e `??`: `compiti/43-…`,
    `decomp/harness/video_sdl.c`, `video_sdl.h`, `mostra_oracolo.c`,
    `mostra_replay.c`; più il `M README.md` preesistente. Nessun binario né `.o`:
    `git status --short | grep -c -E 'mostra_(oracolo|replay)$|\.o$'` → `0`.
16. `pgrep -c mame` → `0`.

Verifica manuale FACOLTATIVA (non fa parte dell'esito, richiede schermo):
`harness/mostra_oracolo -s 3 gioco` mostra la schermata di gioco nitida a
768x672, ESC chiude; `harness/mostra_replay -s 3` fa scorrere il giro per circa
52 secondi (3090 frame a 60 Hz) e si chiude da solo o con ESC.

## Consegna

Commit unico «Lotto H3: finestra SDL2 (video_sdl, mostra_oracolo,
mostra_replay)» con riga finale `Co-Authored-By: <modello vero>
<noreply@anthropic.com>`. Non fare push. In fondo a questo brief: esito e
uscita delle verifiche. Rapporto finale: massimo trenta righe con l'uscita
delle 16 verifiche, i valori misurati di hash/frame della verifica 6 e 7, se
il vsync era presente col driver dummy, e le deviazioni.

## Domande aperte

1. **Scelta di progetto.** Aggancio del replay: serve un gancio in `replay.c`
   (4-6 righe aggiunte: puntatore a funzione globale `replay_dopo_nmi`,
   NULL nel replay normale; `replay.c` compilato anche come oggetto con
   `-Dmain=replay_main`). Cambia un sorgente che il vincolo chiama «intatto»
   (le uscite restano identiche, verifiche 8 e 12). Alternativa senza toccare
   `replay.c`: nessuna pulita (la resa dovrebbe leggere `ram[]` da un altro
   thread o sostituire `sub_0066` a link). Proposta: il gancio.
2. **Preferenza.** Scala predefinita 3 (768x672), massimo 8. Proposta: così.
3. **Preferenza.** Hash FNV-1a 64 bit scritto a mano (nessuna libreria nuova;
   riproducibile in Python in tre righe). Proposta: così, invece di SHA-1.
4. **Preferenza.** La resa del replay avviene a fine NMI (dopo `sub_0066`),
   perché la NMI è il momento in cui il C pubblica palette e sprite nella
   video RAM; il confronto con MAME frame per frame non è in questo lotto
   (H4 lo riprende col giro libero). Proposta: così, e dichiararlo in `VIDEO.md`.
5. **Preferenza.** `-m N` (finestra con limite di frame) in più del `-n/--headless N`
   richiesto, per provare il percorso SDL sotto `SDL_VIDEODRIVER=dummy`.
   Proposta: tenerlo.
6. **Preferenza.** `libsdl2-dev` installato a livello di sistema con apt
   (versione 2.30.0). Proposta: così, dalla chat principale prima di lanciare
   lo sviluppatore.

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

1. (Scelta di progetto, risolta dalla chat principale sotto il mandato
   dell'utente «fai l'H3», da segnalare nel rapporto:) sì, il gancio
   `replay_dopo_nmi` in `replay.c` si fa — non c'è alternativa pulita, e
   le verifiche 8 e 12 garantiscono che le uscite del replay restino
   identiche bit a bit.
2. Sì (preferenza): scala predefinita 3, massimo 8.
3. Sì (preferenza): hash FNV-1a 64 scritto a mano al posto di SHA-1.
4. Sì (preferenza): resa a fine NMI; il confronto frame per frame con
   MAME non è in questo lotto, le differenze possibili dalle 885 NMI
   divergenti a metà attività si documentano in VIDEO.md.
5. Sì (preferenza): opzione `-m N` oltre a `-n/--headless N`.
6. Fatto (preferenza): `libsdl2-dev` 2.30.0 installato a sistema dalla
   chat principale con apt, il 2026-11-03, prima dello sviluppo.

Nota per lo sviluppatore: il `README.md` modificato che il brief dichiara
fuori ambito è già stato committato dalla chat principale (5cc3acb);
l'albero è pulito.

## Esito (sviluppatore, 2026-11-03)

Tutte le 16 verifiche passate. Misurati: verifica 6 `HASH primo cc144b3badc192df`,
`HASH ultimo 8da91567376f1f0e`, `NMI primo 2410 ultimo 2509`; verifica 7
3090 frame, ultimo 5499, exit 0. Verifica 5: 1,008 s. Deviazione: il renderer
software (dummy) dichiara il vsync ma non attende, quindi `vsdl_apri` considera
il vsync valido solo se il renderer non è software (altrimenti verifica 5 durava 0,05 s).
sha1 del replay storico 77925e67... invariato.
