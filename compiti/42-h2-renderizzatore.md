# Compito 42: H2, renderizzatore software puro (fondale, testo, sprite, palette)

Data: 2026-11-03. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Lotto H2 di `compiti/PIANO-HARDWARE.md`. Una funzione C `rendi_frame` che, dai
sei blocchi di RAM video (puntatori, senza dipendere dall'harness), produce il
framebuffer RGB 256x224 del frame: fondale BG, piano FG con trasparenza,
sprite con flip e ordine, palette dalla RAM. La grafica delle ROM diventa C
generato da `decomp/estrai_grafica.py` con prova di andata e ritorno. Un
programma `rendi_oracolo` confronta il frame reso con il PNG di MAME pixel per
pixel: **7 stati su 7 identici** (i sei di H1 più lo stato sintetico `prove`).

Il modello qui sotto è già stato provato fuori dal repository con un
prototipo Python: 0 pixel diversi su tutti e sette gli stati. Non serve
spezzare in H2a/H2b: stima ~450 righe scritte a mano più 12 file generati da
~690 righe.

## Vincoli assoluti

- Lavoro solo in `/home/rufio72/OpenSolomonsKey-master`. File da creare o
  toccare: quelli elencati in «Struttura da realizzare», più `decomp/VIDEO.md`,
  `NOTICE.md`, `.gitignore`, `decomp/harness/Makefile`, una riga in `STATO.md`
  (punto 3, strato hardware) e l'esito in fondo a questo brief.
- NON modificare nessun altro file di `decomp/harness/`, né `video.lua`,
  `snap.lua`, `cattura.sh`, `replay*`. Banco (304 voci, 1270 catture, 0
  fallite), replay e sched_proto INTATTI (verifiche 9-11).
- NON rigenerare i sei stati di H1 (`titolo gioco stanza10 bonus finale
  finale2`): si genera solo lo stato nuovo `prove`.
- MAME solo senza finestra, come fa già `video.sh` (`-video none -sound
  none`, provider di input a `none`, `SDL_VIDEODRIVER=dummy`).
- Le ROM non entrano nel repository: si leggono da `roms/ext/solomon/`
  (ignorata da `.gitignore:48`, `roms/ext/`). Entra il C generato, con la
  nota di non proprietà (NOTICE.md).
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe. Niente installazioni. Niente git distruttivo (stash, reset,
  filter-branch, rebase, checkout di file). Python sempre con `python3 -I`
  (PIL 10.2.0 e numpy 1.26.4 installati; numpy non serve).
- `gcc` con i flag del Makefile: `-std=c11 -Wall -Wextra -O0`, zero warning.

## Cosa esiste già (fatti verificati)

| Cosa | Dove |
|---|---|
| Oracolo H1: per stato `<S>_{bg,fg}_{colorram,videoram}.bin` (1024 B), `<S>_spriteram.bin` (128 B), `<S>_palette.bin` (512 B), `<S>.png` 256x224 RGB | `decomp/out/video/`, formato in `decomp/VIDEO.md` (52 righe) |
| Indirizzi: FG colorram d000, FG videoram d400, BG colorram d800, BG videoram dc00, spriteram e000, palette e400 | `decomp/VIDEO.md` |
| Generatore stati: `sh video.sh [stato...]` da `decomp/`, `case` per stato in `lancia()`, controllo 7 file e `VIDEO <S> OK` | `decomp/video.sh` (49 righe) |
| Poke: `VIDEO_POKE` «frame:addr:val» separati da virgola, scritti a inizio frame | `decomp/video.lua:5-6,23,58-60` |
| Hash dei PNG (sha1 dei soli pixel RGB) e dei .bin | `python3 -I decomp/verifica_video.py [DIR]` |
| ROM grafiche (tutte presenti, lette con `ls`) | `roms/ext/solomon/`: `12.3t` `11.3r` (FG, 32 KB l'una), `10.3p` `9.3m` (BG, 32 KB), `2.5lm` `3.6lm` `4.7lm` `5.8lm` (sprite, 16 KB l'una) |
| Decodifica Python esistente (stesso formato, piani rovesciati) | `roms/decoded/colora.py` (162 righe: `leggi_palette`, `_piano`, `leggi_piani`) |
| Modello di generatore con andata e ritorno: percorsi ROM `ROM_PERCORSI`, intestazione «Generato da … non modificare», controllo che esce con 2 | `decomp/estrai_dati.py:24-44` |
| Modello di programma di andata e ritorno | `decomp/harness/audio_immagine.c` (28 righe) |
| Makefile: `CC = gcc`, `CFLAGS = -std=c11 -Wall -Wextra -O0`; i jolly sono `z80_*.c`, `nemici*.c`, `audio_dati*.c` (un nome `video_*.c` non entra in nessun bersaglio esistente); `all:` e `clean:` elencano i binari | `decomp/harness/Makefile` (tracciato anche se `.gitignore` ha `Makefile`) |
| Binari ignorati: righe `decomp/harness/<nome>` | `.gitignore:53-57,73-75` |

### Formato hardware (provato: 0 pixel diversi su 7 stati)

- Schermo: tilemap 32x32 celle 8x8 = 256x256, cella `i = riga*32 + colonna`,
  nessuno scorrimento. Il PNG di MAME è l'area visibile, righe 16-239:
  framebuffer riga `y` = schermo riga `y+16`. Niente bordo né altro ritaglio.
- Palette: voce `i` (0-255), `w = pal[2i] | pal[2i+1]<<8`;
  `R = (w&15)*17`, `G = ((w>>4)&15)*17`, `B = ((w>>8)&15)*17`.
- Tile 8x8 (FG e BG): grafica FG = `12.3t` seguita da `11.3r` (64 KB, 2048
  tile da 32 byte); BG = `10.3p` seguita da `9.3m`. Pen del pixel (x,y) del
  tile c: byte `g[c*32 + y*4 + x/2]`, nibble alto se x pari, basso se dispari.
- FG: `a = fg_colorram[i]`, codice `fg_videoram[i] + 256*(a&7)`, colore
  `16*((a>>4)&7) + pen` (voci 0-127), pen 0 trasparente; bit 0x80 e 0x08
  **ignorati** (provato su `prove`: trattarli come flip dà 58 pixel diversi).
- BG: `a = bg_colorram[i]`, codice `bg_videoram[i] + 256*(a&7)`, colore
  `128 + 16*((a>>4)&7) + pen`, opaco; `a&0x80` flip orizzontale (senza: 2796
  pixel diversi in titolo/gioco/finale), `a&0x08` flip verticale (senza: 264
  pixel diversi in `prove`).
- Sprite 16x16, 32 voci da 4 byte `b0 b1 b2 b3`: codice `b0 + ((b1&0x10)<<4)`
  (0-511), colore `16*((b1>>1)&7) + pen` (voci 0-127), `b1&0x40` flip
  orizzontale, `b1&0x80` flip verticale, `sx = b3`, `sy = 241 - b2` (in righe
  di schermo, prima del -16), pen 0 trasparente. Grafica: quattro piani, un
  file per piano; pen = `p(2.5lm)<<3 | p(3.6lm)<<2 | p(4.7lm)<<1 | p(5.8lm)`,
  dove p(file) è il bit `7-(x&7)` del byte
  `s*32 + ((y>=8)?16:0) + ((x>=8)?8:0) + (y&7)` del file.
- Ordine: BG, poi FG, poi sprite dalla voce 31 (offset 0x7C) alla voce 0: la
  voce 0 resta sopra (ordine inverso: 45 pixel diversi in finale2). Nessun bit
  di priorità sprite/FG.
- Bordi: pixel fuori da x 0-255 o righe di schermo 16-239 si scartano, niente
  avvolgimento (con avvolgimento in x: 74 pixel diversi in `prove`, sprite a
  x=250). `sy` può essere negativo (b2 > 241): si scarta la parte fuori.
- Coordinate: con `sy = 240-b2` o `242-b2` gioco ha 413/415 pixel diversi.
- Non modellato: ribaltamento dello schermo (latch di sola scrittura, non è
  nella RAM; vedi domanda 2).
- La corrispondenza PNG/RAM nello stesso frame (dubbio di VIDEO.md) è
  confermata: 0 pixel diversi in tutti gli stati.

### Stato sintetico `prove` (misurato)

Parte da `titolo` (frame 1000, ingressi `nessuno`) e al frame 995 scrive:
BG colorram d942-d94b = 0x0a (flip verticale), FG colorram d2aa,d2ab = 0x80 e
d2ac,d2ad = 0x08 (bit da ignorare), quattro sprite codice 0x88:
voce 0 `88 80 8d 64` (flip v), voce 1 `88 00 8d fa` (x=250, ritaglio),
voce 2 `88 c0 78 3c` (flip h+v), voce 3 `88 00 05 1e`. Riga POKE esatta:

```
POKE_P=995:0xd942:0x0a,995:0xd943:0x0a,995:0xd944:0x0a,995:0xd945:0x0a,995:0xd946:0x0a,995:0xd947:0x0a,995:0xd948:0x0a,995:0xd949:0x0a,995:0xd94a:0x0a,995:0xd94b:0x0a,995:0xd2aa:0x80,995:0xd2ab:0x80,995:0xd2ac:0x08,995:0xd2ad:0x08,995:0xe000:0x88,995:0xe001:0x80,995:0xe002:141,995:0xe003:100,995:0xe004:0x88,995:0xe005:0x00,995:0xe006:141,995:0xe007:250,995:0xe008:0x88,995:0xe009:0xc0,995:0xe00a:120,995:0xe00b:60,995:0xe00c:0x88,995:0xe00d:0,995:0xe00e:5,995:0xe00f:30
```

Due corse MAME danno file identici (misurato).

## Struttura da realizzare

### 1. `decomp/estrai_grafica.py` (~120 righe)

Uso da `decomp/`: `python3 -I estrai_grafica.py`. Cerca le ROM come
`estrai_dati.py` (`../roms/ext/solomon/`, poi relativo allo script). Scrive in
`harness/` dodici file, ognuno con 16384 byte, 24 byte per riga
(`0x%02X,` senza spazi, rientro di 4: 683 righe di dati, ~690 totali):

| File | Array | Origine |
|---|---|---|
| `video_dati_car_a.c` / `_b.c` | `vid_car_a` / `vid_car_b` | `12.3t` 0000-3FFF / 4000-7FFF |
| `video_dati_car_c.c` / `_d.c` | `vid_car_c` / `vid_car_d` | `11.3r` 0000-3FFF / 4000-7FFF |
| `video_dati_fon_a.c` … `_d.c` | `vid_fon_a` … `vid_fon_d` | `10.3p` (a, b), `9.3m` (c, d) |
| `video_dati_spr_a.c` … `_d.c` | `vid_spr_a` … `vid_spr_d` | `2.5lm`, `3.6lm`, `4.7lm`, `5.8lm` |

Ogni file: intestazione
`/* Generato da decomp/estrai_grafica.py dalla ROM <file> <intervallo>: non modificare`
` * a mano. Grafica derivata dalle ROM Tecmo, non di proprietà dell'autore del`
` * repository (NOTICE.md). */`, `#include "video.h"`, poi
`const uint8_t vid_xxx_y[VID_PEZZO] = { ... };`. Prima di scrivere, il
generatore rilegge i valori dal testo prodotto e li confronta con la ROM;
se differiscono esce con 2. Scrive il file solo se il contenuto cambia;
stampa una riga finale `grafica: 12 file, 196608 byte, andata e ritorno ok`.

### 2. `decomp/harness/video.h` (~50 righe)

Dipende solo da `<stdint.h>`. Da copiare:

```c
#define VID_LARGO 256
#define VID_ALTO 224          /* righe di schermo 16-239 */
#define VID_PEZZO 0x4000      /* byte di un file video_dati_*.c */
typedef struct {
    const uint8_t *fg_colorram;   /* d000-d3ff, 1024 byte */
    const uint8_t *fg_videoram;   /* d400-d7ff, 1024 byte */
    const uint8_t *bg_colorram;   /* d800-dbff, 1024 byte */
    const uint8_t *bg_videoram;   /* dc00-dfff, 1024 byte */
    const uint8_t *spriteram;     /* e000-e07f, 128 byte */
    const uint8_t *palette;       /* e400-e5ff, 512 byte */
} vid_ram;
void rendi_frame(const vid_ram *ram, uint8_t rgb[VID_ALTO][VID_LARGO][3]);
```

più le dodici `extern const uint8_t vid_car_a[VID_PEZZO];` ecc.

### 3. `decomp/harness/video_rendi.c` (~170 righe)

`rendi_frame` e funzioni `static`: palette (256 RGB), pen di tile
(`static int pen_tile(const uint8_t *const g[4], int codice, int x, int y)`,
con `g[o>>14][o&0x3FFF]`), pen di sprite, disegno BG, FG, sprite, secondo il
formato sopra. Lavora in un buffer di indici 256x256 o direttamente sulle 224
righe visibili: scelta libera, il risultato deve essere identico.

### 4. `decomp/harness/rendi_oracolo.c` (~130 righe)

`rendi_oracolo <cartella> <stato> <riferimento.ppm> [uscita.ppm]`: legge i sei
`.bin` (controlla le dimensioni), chiama `rendi_frame`, legge il PPM P6
256x224 maxval 255 di riferimento, confronta. Uscita su stdout, una riga:
- tutti uguali: `<stato>: 57344/57344 pixel uguali`, exit 0;
- altrimenti: `<stato>: N pixel diversi, primo (x,y) atteso RRGGBB reso RRGGBB`
  (primo in ordine di righe, x poi y in coordinate del framebuffer,
  esadecimale minuscolo), exit 1;
- errore di file: messaggio `ERRORE: ...` su stderr, exit 2.
Con `uscita.ppm` scrive il frame reso come P6 (`P6\n256 224\n255\n` + 172032 byte).

Il PNG non si decodifica in C: niente zlib o libpng (nessuna dipendenza
nuova; H3 userà SDL2 che non legge PNG). La conversione la fa Python.

### 5. `decomp/png_ppm.py` (~20 righe)

`python3 -I png_ppm.py <in.png> <out.ppm>`: PIL, `convert('RGB')`, scrive P6
con la stessa intestazione.

### 6. `decomp/harness/video_immagine.c` (~45 righe)

`video_immagine <cartella>`: ricompone dai dodici array gli otto file ROM
(`12.3t` = car_a+car_b, `11.3r` = car_c+car_d, `10.3p` = fon_a+fon_b,
`9.3m` = fon_c+fon_d, `2.5lm`… = spr_a…d) e li scrive nella cartella (fuori dal
repository), stile `audio_immagine.c`.

### 7. `decomp/rendi_video.sh` (~35 righe)

Da `decomp/`: `sh rendi_video.sh [stato...]` (senza argomenti: `titolo gioco
stanza10 bonus finale finale2 prove`). Cartella temporanea `mktemp -d`
(rimossa alla fine); per ogni stato `png_ppm.py` poi
`harness/rendi_oracolo out/video <S> <tmp>/<S>.ppm`; stampa le righe e in
fondo `RENDI <k>/<n> identici`; exit 0 solo se k = n.

### 8. Modifiche

- `video.sh`: stato `prove)  FR=1000; ING=nessuno; POKE=$POKE_P ;;` con
  `POKE_P` definita accanto a `POKE_F` (riga sopra). Lista predefinita
  invariata (i sei stati): `prove` si genera solo nominandolo.
- `Makefile`: `SORGENTI_GRAFICA = $(wildcard video_dati_*.c)`, bersagli
  `rendi_oracolo` (rendi_oracolo.c video_rendi.c grafica, dipende da video.h)
  e `video_immagine` (video_immagine.c grafica); aggiungerli ad `all` e `clean`.
- `.gitignore`: `decomp/harness/rendi_oracolo`, `decomp/harness/video_immagine`.
- `NOTICE.md` punto 1: le tabelle `decomp/harness/video_dati_*.c` sono i byte
  delle ROM grafiche Tecmo, estratti con `decomp/estrai_grafica.py`, non di
  proprietà dell'autore.
- `decomp/VIDEO.md`: sostituire «Da decidere in H2» con il formato provato
  (sezione sopra), aggiungere `prove` alla tabella degli stati.

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master/decomp` salvo indicazione.

1. Generatore: `python3 -I estrai_grafica.py | tail -1` →
   `grafica: 12 file, 196608 byte, andata e ritorno ok`. Seconda corsa
   identica: `sha1sum harness/video_dati_*.c > /tmp/g1; python3 -I estrai_grafica.py >/dev/null; sha1sum harness/video_dati_*.c | diff /tmp/g1 - && echo STABILE` → `STABILE`.
2. Build: `cd harness && make 2>&1 | grep -c -i -E 'warning|error'` → `0`;
   `ls rendi_oracolo video_immagine` elenca i due binari.
3. Andata e ritorno: `T=$(mktemp -d); harness/video_immagine $T && for f in 12.3t 11.3r 10.3p 9.3m 2.5lm 3.6lm 4.7lm 5.8lm; do cmp $T/$f ../roms/ext/solomon/$f || echo DIVERSO $f; done; echo fine` → solo `fine`.
4. Stato sintetico: `sh video.sh prove` → `VIDEO prove OK`;
   `python3 -I verifica_video.py out/video | grep '^prove'` → (misurato):
   ```
   prove 256x224 406d433dc9989ad81d62891eae35746c7cf63a06
   prove_bg_colorram.bin 1024 c80054e4f32939fe537402b086fd7ccdb1c6980d
   prove_bg_videoram.bin 1024 b5dfde746e26901aa37f21b6a956020d9172d3ae
   prove_fg_colorram.bin 1024 26337da7f612874cf34643191dfe33fdb256fc51
   prove_fg_videoram.bin 1024 321e51a3e162f4f1760f1cd7aec4fc580ceeeb7a
   prove_palette.bin 512 831dbb2ad5b064fc84f446433ce780bd71e6de04
   prove_spriteram.bin 128 f10d191e84c55a96ef2390434e309934df34ca08
   ```
5. **Verifica principale**: `sh rendi_video.sh; echo "exit $?"` → sette righe
   `<stato>: 57344/57344 pixel uguali` (titolo, gioco, stanza10, bonus,
   finale, finale2, prove), poi `RENDI 7/7 identici`, `exit 0`
   (attesa dal prototipo Python, che dà 0 pixel diversi su tutti e sette).
6. Il confronto vede gli errori: `python3 -I png_ppm.py out/video/gioco.png /tmp/g.ppm; harness/rendi_oracolo out/video titolo /tmp/g.ppm; echo "exit $?"` →
   `titolo: 24502 pixel diversi, primo (1,0) atteso 0099ff reso 000000`,
   `exit 1` (misurato col prototipo).
7. Frame scritto = pixel del PNG: per ogni stato
   `harness/rendi_oracolo out/video <S> /tmp/<S>.ppm /tmp/r.ppm >/dev/null; tail -c 172032 /tmp/r.ppm | sha1sum` →
   lo sha1 della riga `<S> 256x224 …` di `verifica_video.py`: titolo
   `b88e0bb3c3b4…`, gioco `539f7014f227…`, stanza10 `8c75598ddd53…`, bonus
   `f8e1ded4a5b3…`, finale `50357fb27c65…`, finale2 `6193568923ca…`, prove
   `406d433dc998…` (misurati).
8. Oracolo H1 intatto: `git status --short out/video` → solo righe
   `?? out/video/prove…` (sette file).
9. Banco: `cd harness && ./harness --tutte > /tmp/h42.txt; echo "exit $?"; grep -c FALLITO /tmp/h42.txt; grep -c '^RIEPILOGO' /tmp/h42.txt; grep -c '^cattura' /tmp/h42.txt` →
   `exit 0`, `0`, `304`, `1270`.
10. `harness/sched_proto out/sched_log.txt | tail -1` →
    `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`.
11. `harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r42.txt 2>&1; echo "exit $?"; tail -4 /tmp/r42.txt` → `exit 0` e
    ```
    spedizioni eseguite: 64686, fino al frame 5499
    NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
    prima divergenza a metà attività (attesa): frame 2415
    nessuna divergenza nello spedidore
    ```
12. Righe: `wc -l harness/video*.c harness/video.h harness/rendi_oracolo.c estrai_grafica.py png_ppm.py rendi_video.sh video.sh | awk '$1>800 && $2!="totale"'` → vuoto.
13. Ambito (dalla radice): `git status --short` → solo `M` di `.gitignore`,
    `NOTICE.md`, `STATO.md`, `decomp/VIDEO.md`, `decomp/video.sh`,
    `decomp/harness/Makefile`, `compiti/42-…`, e `??` dei file nuovi sopra
    (12 `video_dati_*`, `video.h`, `video_rendi.c`, `rendi_oracolo.c`,
    `video_immagine.c`, `estrai_grafica.py`, `png_ppm.py`, `rendi_video.sh`,
    sette `out/video/prove*`). Nessun binario: `git status --short | grep -c -E 'rendi_oracolo$|video_immagine$|\.o$|\.ppm$'` → `0`.
14. `pgrep -c mame` → `0`.

## Consegna

Commit unico «Lotto H2: renderizzatore software (fondale, testo, sprite,
palette)» con riga finale `Co-Authored-By: <modello vero> <noreply@anthropic.com>`.
Non fare push. In fondo a questo brief: esito e uscita delle verifiche.
Rapporto finale: massimo trenta righe con l'uscita delle 14 verifiche e le
deviazioni. Se la verifica 5 fallisce, riportare la riga di ogni stato e non
toccare il formato senza un numero misurato.

## Domande aperte

1. **Scelta di progetto.** I file `video_dati_*.c` sono i byte delle ROM
   grafiche (192 KB su 288 KB del set) messi nel repository, come già
   `audio_dati*.c` per la ROM audio; `PIANO-HARDWARE.md` H7 dice invece che
   «il gioco le carica da `roms/`». Proposta: committarli (piano H2 e
   precedente audio), con la nota in NOTICE.md. Alternativa: generarli in
   build da `roms/ext/` e metterli in `.gitignore` (cambia solo `.gitignore`
   e il Makefile).
2. **Preferenza.** Ribaltamento dello schermo (modalità cocktail) non
   modellato: il latch non è in RAM e nessun oracolo lo prova. Proposta: fuori
   da H2; si aggiunge un parametro a `rendi_frame` in H4/H5 se serve.
3. **Preferenza.** Stato sintetico `prove` aggiunto a `video.sh` e
   all'oracolo: è l'unico che esercita flip verticale BG e sprite, bit FG da
   ignorare e ritaglio in x. Proposta: tenerlo, fuori dalla lista predefinita.
4. **Preferenza.** Confronto PNG tramite `png_ppm.py` + confronto in C,
   invece di un lettore PNG in C. Proposta: così, nessuna dipendenza nuova.

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

1. (Scelta di progetto, risolta dalla chat principale col precedente
   ratificato dall'utente per l'audio — regione dati come sorgente C
   generato, ROM mai nel repo, dati derivati pubblicati con la nota di
   NOTICE.md:) sì, i byte delle ROM grafiche entrano nel repository come
   C generato da `estrai_grafica.py`, con prova di andata e ritorno byte
   per byte. La voce H7 del piano si intende di conseguenza: il gioco
   non caricherà le ROM grafiche da `roms/`, perché sono già in C; da
   `roms/` si carica solo l'eventuale resto. Da segnalare all'utente.
2. Sì (preferenza): il ribaltamento dello schermo (cocktail) resta fuori
   da H2; il latch non è in RAM e nessun oracolo lo prova.
3. Sì (preferenza): lo stato `prove` resta in `video.sh` fuori dalla
   lista predefinita, come unico esercizio di flip verticale, bit FG
   ignorati e taglio in x.
4. Sì (preferenza): PNG→PPM con Python (`png_ppm.py`), confronto in C;
   nessun lettore PNG in C né dipendenze nuove.

## Esito (2026-11-03, sviluppatore)

Tutte le 14 verifiche passate al primo colpo: generatore 12 file/196608 byte e
STABILE; build 0 warning; andata e ritorno ROM senza differenze; `prove` con
hash attesi; `RENDI 7/7 identici` (exit 0); errore visto
(`titolo: 24502 pixel diversi, primo (1,0) atteso 0099ff reso 000000`, exit 1);
sha1 dei frame scritti uguali ai PNG; oracolo H1 intatto; banco 0 FALLITO/304/1270;
sched_proto 106124 differenze 0; replay come atteso; nessun file oltre 800 righe;
ambito pulito; nessun mame attivo. Nessuna deviazione dal brief.
