# Compito 63: driver audio in `tecmo/`, dati Tecmo tutti in `tecmo/`, `gioca` senza ROM

Data: 2026-11-06. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Secondo di tre lotti verso «il binario non legge più nessuna ROM». Oggi
`gioca` apre a runtime un solo file, `roms/ext/solomon/1.3jk` (audio). A fine
lotto: il codice del driver audio (0000-05C5 di `1.3jk`, 1478 byte) è un blob
C generato in `tecmo/`; grafica e musica già estratte stanno in `tecmo/`
invece che in `decomp/harness/`; `make gioco` non chiede più ROM; un clone
del repository **senza `roms/` e senza `decomp/out/`** compila `gioca` e gioca
con gli stessi hash di oggi.

Vincoli assoluti:
- Modifiche solo a: `tecmo/` (file nuovi e spostati, `README.md`),
  `decomp/harness/{Makefile,audio_piattaforma.c,audio.h,audio_immagine.c}`,
  i file spostati con `git mv` (elenco al punto 2), `decomp/{estrai_rom.py,
  estrai_dati.py,estrai_grafica.py,censimento_rom.py,CENSIMENTO-ROM.md,
  AUDIO.md,AUDIO-SEQUENZE.md,VIDEO.md,SPARTITI.md}`, `STATO.md`, questo brief.
- Non toccare: `README.md` e `NOTICE.md` principali (lotto 64, anche se
  citano ancora `decomp/harness/*_dati*`), `roms/`, `decomp/ricette/`,
  `decomp/out/`, `decomp/build_rom.sh`, `scheda_audio.c`, `audio_banco.c`,
  `audio_suona.c`, `audio_interprete.c`, `audio_ossatura.c`.
- I file spostati sono **rinomine pure**: contenuto identico byte per byte
  (niente ritocchi alle intestazioni dei file generati).
- Commenti, messaggi e documenti in italiano; **solo `tecmo/README.md` in
  inglese** (testo sotto).
- Nessun file di codice sopra le 800 righe (i `video_dati_*.c` generati ne
  hanno 689, gli spartiti al massimo 567).
- Niente push. Niente git distruttivo (`stash`, `reset --hard`,
  `filter-branch`). Build: `make -C decomp/harness` (gcc e SDL2 installati);
  comandi di gioco con `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy`, da
  `decomp/`. Non usare `pkill`: solo PID propri.

## Cosa esiste già (fatti verificati)

| file | righe | ruolo |
|---|---|---|
| `decomp/harness/audio_piattaforma.c` | 176 | `aud_rom[0x4000]` (r. 15), `rom_vicino_exe()` (32-43), `aud_carica_rom()` (45-67, apre `1.3jk`, exit 2 se manca), `aud_usa_dati_estratti(int)` (74-84), `aud_leggi()` (104) |
| `decomp/harness/audio.h` | 96 | commento r. 1-6 («la ROM si legge a tempo di esecuzione»), `extern uint8_t aud_rom[0x4000]; /* 1.3jk */` (r. 13), prototipi r. 65-66 |
| `decomp/harness/audio_dati.h` | 75 | **scritto a mano**: `AUD_DATI_INIZIO 0x05C6`, `AUD_DATI_FINE 0x4000`, `struct aud_pezzo`, macro dei flussi, include `"audio_dati_indirizzi.h"` |
| `decomp/harness/audio_dati.c` | 46 | **scritto a mano**: `aud_dati_serializza(img)` — fa `memset(img, 0xAA, 0x4000)` e poi riempie 05C6-3FFF; include `"audio_dati_gruppi.h"` |
| `decomp/harness/audio_immagine.c` | 28 | scrive 05C6-3FFF (14906 byte) ricomposti in un file |
| `decomp/estrai_rom.py` | 167 | genera `tecmo/rom_dati_*.c`, `rom_dati.h`, `rom_voci.c`; `leggi_rom(nome, lung)` (44), `scrivi(nome, t)` in `TECMO` (121), riga finale `rom: 91 voci, 20448 byte, andata e ritorno ok` |
| `decomp/estrai_dati.py` | 641 | genera in `HARNESS` (r. 25) `audio_dati_{tabelle,spartiti_a..g}.c`, `audio_dati_{indirizzi,gruppi}.h` (r. 633) e `SPARTITI.md`; docstring r. 5-6 e testo di `documento()` r. 501-504 citano `harness/` |
| `decomp/estrai_grafica.py` | 88 | genera in `HARNESS` (r. 19) i 12 `video_dati_*.c`, che fanno `#include "video.h"` (header di `harness/`, non dati) |
| `decomp/censimento_rom.py` | — | `AUD_SITI` (r. 246): chiave `'audio_piattaforma.c:104': ('D', ())` = riga della definizione di `aud_leggi`; `--verifica` la cerca per numero di riga e scandisce solo `harness/*.c` |

Da grep/grafo:
- Chiamanti di `aud_carica_rom()`/`aud_usa_dati_estratti()`: `scheda_audio.c:74-75`
  (`aud_carica_rom(); aud_usa_dati_estratti(1);`, percorso di `gioca`),
  `audio_banco.c:150-156` e `audio_suona.c:37-43` (modo 1 con
  `AUDIO_DATI=estratti`, modo 2 con `AUDIO_DATI=vuoto`, altrimenti ROM grezza;
  usati da `audio_wav.sh`). `aud_rom` è usato solo in `audio_piattaforma.c`.
- Letture del blob 0000-05C5 a runtime: solo 03CE-03ED, dentro
  `controlla_tabella()` (`audio_interprete.c:340`), che confronta una volta i
  16 gestori con la copia a mano `indirizzi[16]` e fa exit 2 se diversi. Il
  resto è codice già tradotto in C. Decisione presa: si estrae l'intero blob
  0000-05C5 (non i soli 32 byte).
- Misurato: i primi 1478 byte di `1.3jk` hanno md5
  `73ab3570bbdd23f450c3572173a3ec0d`; `1.3jk` intero
  `43b065c0ca0bae238fec504ccfee55c0`; `head -c 1478 1.3jk` seguito
  dall'uscita di `audio_immagine` è identico a `1.3jk` (`cmp` muto).
- Nessun file `audio_dati*`/`video_dati*` contiene `leggi(` (grep): spostarli
  non cambia il conteggio di `censimento_rom.py --verifica` (201).
- Generati da `estrai_dati.py` (vanno in `tecmo/`): `audio_dati_tabelle.c`,
  `audio_dati_spartiti_{a,b,c,d,e,f,g}.c`, `audio_dati_indirizzi.h`,
  `audio_dati_gruppi.h`. Generati da `estrai_grafica.py`: `video_dati_{car,fon,spr}_{a,b,c,d}.c`.
- `harness/Makefile` (121 righe): r. 2 `CFLAGS`, r. 4 `TECMO = ../../tecmo`,
  r. 15-17 `SORGENTI_DATI = $(wildcard audio_dati*.c)`,
  `INTESTAZIONI_DATI = $(wildcard audio_dati*.h)`, `SORGENTI_DRIVER`; r. 51-52
  `audio_immagine`; r. 63 `SORGENTI_GRAFICA = $(wildcard video_dati_*.c)`;
  r. 97-116: commento, `gioco: rom gioca` (r. 105), bersaglio `rom` (estrae
  `roms/solomon.zip`, «ERRORE: mancano le ROM: metti solomon.zip in roms/»,
  chiama `build_rom.sh`). `decomp/Makefile`: `gioco: $(MAKE) -C harness gioco`.
- `.gitignore` radice: ignora `roms/ext/`, `roms/*.zip`, `decomp/out/*`
  (tranne `snap_*`, `video/`); `roms/decoded/` è tracciato (143 file).
- Messaggi sulle ROM nel percorso di gioco (grep su `gioca*.c`,
  `macchina*.c`, `comune.c`, `scheda_audio.c`): solo quelli di
  `aud_carica_rom()` e del bersaglio `rom`. `carica_rom()` di `comune.c` non
  apre più file dal lotto 62.
- Misurato oggi in un clone senza `roms/` e `decomp/out/`:
  `make -C decomp gioco` → `ERRORE: mancano le ROM: metti solomon.zip in roms/`;
  `gioca -n 600 --muto` → `1.3jk non trovato …` / `Esegui: make -C decomp/harness gioco`, uscita 2.

## Struttura da realizzare

### 1. Blob del driver: `estrai_rom.py` → `tecmo/audio_driver.{c,h}`

In `estrai_rom.py` (resta sotto ~220 righe), dopo la parte del principale:
leggere `leggi_rom('1.3jk', 0x4000)`, prendere `[0:0x05C6]`; controllo di
coerenza (uscita 2 con messaggio) che le voci `'A'` di `VOCI` siano
`aud_gestori_03ce` dentro 0000-05C5 e `aud_dati_05c6` con inizio 0x05C6.
Scrivere con `scrivi()`:
- `audio_driver.h`: commento `AVVISO`, guardia `AUDIO_DRIVER_H`,
  `#include <stdint.h>`, `extern const uint8_t aud_driver[1478];  /* 0000-05C5 di 1.3jk */`.
- `audio_driver.c`: commento `AVVISO` più una riga «codice del driver del
  Z80 audio, 0000-05C5 di 1.3jk», `#include "audio_driver.h"`,
  `const uint8_t aud_driver[1478] = { … };` con 24 byte per riga `0x%02X,`
  (~68 righe).
Round-trip come per le voci: rilettura degli `0x..,` dal testo e confronto con
i byte, uscita 2. Seconda riga stampata, esatta:
`audio: driver 0000-05C5, 1478 byte, andata e ritorno ok`.
Aggiornare la docstring (ora legge anche `1.3jk` e scrive `audio_driver.*`).

### 2. Spostamento dei dati generati in `tecmo/`

`git mv` di `decomp/harness/audio_dati_tabelle.c`, `audio_dati_spartiti_{a..g}.c`,
`audio_dati_indirizzi.h`, `audio_dati_gruppi.h` e dei 12 `video_dati_*.c`
in `tecmo/`. **Restano in `harness/`** `audio_dati.h` e `audio_dati.c`
(codice dell'autore, non dati) e `video.h`.

Gli include non cambiano: in `Makefile`, r. 2 diventa
`CFLAGS = -std=c11 -Wall -Wextra -O0 -I. -I$(TECMO)` (con `-I.` un file di
`tecmo/` trova `video.h`/`audio_dati.h` di `harness/`; con `-I$(TECMO)`
`audio_dati.h` trova `audio_dati_indirizzi.h`). Poi:
- `SORGENTI_DATI = audio_dati.c $(wildcard $(TECMO)/audio_dati_*.c)`
- `INTESTAZIONI_DATI = audio_dati.h $(wildcard $(TECMO)/audio_dati_*.h)`
- `SORGENTI_DRIVER = … $(SORGENTI_DATI) $(TECMO)/audio_driver.c` e
  `$(TECMO)/audio_driver.h` fra le dipendenze di `audio_banco`, `audio_suona`,
  `gioca`, `audio_immagine`
- `audio_immagine: audio_immagine.c $(SORGENTI_DATI) $(TECMO)/audio_driver.c …`
- `SORGENTI_GRAFICA = $(wildcard $(TECMO)/video_dati_*.c)`

Generatori: `estrai_grafica.py` e `estrai_dati.py` scrivono i `.c`/`.h` in
`os.path.join(QUI, '..', 'tecmo')` (costante `TECMO`, come `estrai_rom.py`);
`SPARTITI.md` resta in `decomp/`. Aggiornare docstring e il testo di
`documento()` (`harness/audio_dati_*.c` → `tecmo/audio_dati_*.c`; `audio_dati.h`
resta in `harness/`): `SPARTITI.md` rigenerato cambia solo lì. Rieseguiti,
non devono cambiare nessun file di `tecmo/`.

### 3. `aud_carica_rom()` senza file (`audio_piattaforma.c`, `audio.h`)

Nuovo corpo (stesso nome e firma, chiamanti invariati):

```c
_Static_assert(sizeof aud_driver == AUD_DATI_INIZIO, "driver 0000-05C5");

/* Ricompone l'immagine di 1.3jk senza leggerla: regione dati 05C6-3FFF
 * dalle tabelle (audio_dati_*.c, che prima riempiono tutto di 0xAA) e
 * codice del driver 0000-05C5 dal blob di tecmo/audio_driver.c. */
void aud_carica_rom(void)
{
    aud_dati_serializza(aud_rom);
    memcpy(aud_rom, aud_driver, sizeof aud_driver);
}
```

L'ordine conta: `aud_dati_serializza` azzera prima a 0xAA tutta l'immagine.
Togliere `rom_vicino_exe()`, `#include <unistd.h>` e il commento r. 29-31;
aggiungere `#include "audio_driver.h"`. `aud_usa_dati_estratti()` resta (modo 1
ora ricopia gli stessi byte, modo 2 resta il controllo negativo AA):
aggiornarne il commento («altrimenti la regione resta quella delle tabelle»)
e la r. 1 del file. In `audio.h`: commento r. 1-6 («nessun file ROM a
runtime: l'immagine si ricompone da tecmo/»), r. 13 `/* immagine di 1.3jk
ricomposta (aud_carica_rom) */`, r. 65 `/* driver e dati da tecmo/ */`.

`censimento_rom.py`: aggiornare la chiave `'audio_piattaforma.c:104'` di
`AUD_SITI` alla nuova riga della definizione di `aud_leggi`
(`grep -n '^uint8_t aud_leggi' harness/audio_piattaforma.c`).

### 4. `audio_immagine.c` scrive l'immagine intera

`aud_dati_serializza(img); memcpy(img, aud_driver, sizeof aud_driver);` e
scrittura di tutti i 0x4000 byte; commento in testa: «da confrontare con
`cmp` contro 1.3jk». Aggiornare `AUDIO.md:243` (prova: 16384 byte, `cmp`
contro `1.3jk`).

### 5. `make gioco` senza ROM (`harness/Makefile`)

- r. 105: `gioco: gioca` (niente `rom`); il messaggio resta
  `Pronto: lancia decomp/harness/gioca (gioca --aiuto per le opzioni)`.
- Commento r. 97-99: «make gioco compila gioca e basta: nessuna ROM a
  runtime né in compilazione (Z80 principale, driver, musica e grafica in
  tecmo/)».
- Bersaglio `rom` invariato nel corpo, fuori dal percorso del giocatore, con
  commento sopra: «strumento di sviluppo per chi ha le ROM: estrae
  roms/solomon.zip e costruisce out/main64.bin (servono a rom_immagine e agli
  estrattori di decomp/)».

### 6. Documenti

- `tecmo/README.md`, testo da copiare intero:

```markdown
# Tecmo data tables

The C files in this directory are byte-for-byte data extracted from the
arcade ROM set of *Solomon's Key* (Tecmo, 1986). They are generated by the
scripts in `decomp/` (which need the original ROMs) and must not be edited
by hand. The game binary reads no ROM file: everything it needs from the
original boards is compiled in from here.

| files | content | source ROM | generator |
|---|---|---|---|
| `rom_dati_*.c`, `rom_voci.c`, `rom_dati.h` | main CPU data tables (rooms, objects, text, backgrounds, palettes) | `6.3f`, `7.3h`, `8.3jk` | `decomp/estrai_rom.py` |
| `audio_driver.c`, `audio_driver.h` | sound CPU driver code, 0000-05C5 | `1.3jk` | `decomp/estrai_rom.py` |
| `audio_dati_*.c`, `audio_dati_*.h` | music and sound effect data, 05C6-3FFF | `1.3jk` | `decomp/estrai_dati.py` |
| `video_dati_*.c` | character, background and sprite graphics | `9.3m` `10.3p` `11.3r` `12.3t` `2.5lm`-`5.8lm` | `decomp/estrai_grafica.py` |

This content is **not** the property of the author of this repository:
graphics, palettes, levels, text, music, sound driver code and game data
are copyright Tecmo (today Koei Tecmo). It is included for study and
preservation purposes only. The source code of this project remains the
work of its author (see `NOTICE.md` in the repository root).
```

- `CENSIMENTO-ROM.md`: sezione in coda «Esito del lotto 63» (blob del driver
  in `tecmo/audio_driver.c`, dati audio e grafica spostati, `1.3jk` non più
  aperto, il bersaglio `rom` solo per sviluppo).
- `AUDIO.md` (r. 122, 231-236, 243), `AUDIO-SEQUENZE.md` (r. 136),
  `VIDEO.md` (r. 43): percorsi `harness/{audio,video}_dati_*` → `tecmo/`.
- `STATO.md` r. 19: «Tabelle Tecmo in `tecmo/` (brief 62-63) | 91 tabelle del
  principale, driver audio, musica e grafica; `gioca` non apre nessuna ROM».

## Verifiche obbligatorie

Da `decomp/`, con `export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy`.
Le uscite 4-8 sono **misurate oggi** sul binario attuale e devono restare
identiche.

1. `python3 -I estrai_rom.py` → due righe, la seconda nuova (attesa, non
   misurata): `rom: 91 voci, 20448 byte, andata e ritorno ok` e
   `audio: driver 0000-05C5, 1478 byte, andata e ritorno ok`.
2. Generatori rieseguiti senza cambiare nulla: dopo il commit di prova o con
   `git status --short tecmo/ SPARTITI.md` prima e dopo, `python3 -I
   estrai_grafica.py` → `grafica: 12 file, 196608 byte, andata e ritorno ok`
   (misurato); `python3 -I estrai_dati.py` → `pezzi: 478, file di spartiti: 7,
   etichette: 478` (misurato); `python3 -I estrai_rom.py` → come al punto 1;
   `git status --short` identico prima e dopo.
3. `make -C harness` senza errori né warning nuovi; `harness/rom_immagine` →
   `… coincide con out/main64.bin`; `D=$(mktemp -d); harness/audio_immagine
   $D/a.bin && cmp $D/a.bin ../roms/ext/solomon/1.3jk && md5sum < $D/a.bin` →
   `cmp` muto, `43b065c0ca0bae238fec504ccfee55c0` (misurato su `1.3jk`).
4. `sh giro_completo.sh` → uscita 0 e, misurato:
   `SALDATURA 2410: byte diversi in c000-e5ff (escluse c040-c043, cf00-cfff): 0, coincide`
   `NMI confrontate: 5434, divergenti: 30`
   `dopo il boot: nello spedidore 2617 (divergenti 0), a metà attività 2280 (divergenti 30)`
   `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`
5. Ricette, misurate (`harness/gioca --riproduci ricette/<R>.reg -n <N> --muto`,
   uscita 0, nessun `AVVISO`):
   | ricetta | -n | HASH ultimo | AUDIO hash |
   |---|---|---|---|
   | bottiglia_rossa | 11167 | 69a54ac9aa6bea9e | b70dc75d81b355b9 |
   | partita_1a | 3919 | 40254b8858b57c6e | 09e3a1a90b2275f8 |
   | riferimento_nuova | 14525 | b9ab89317932188c | 902b2830066dba24 |
   | stanze_bonus | 12387 | bbebf0f3c64e446a | cb78ab354330a616 |
6. Oracolo, misurato: `harness/gioca -n 5434 --oracolo out/giroboot_oracolo.txt
   out/giroboot_disp.txt --riproduci out/ingressi_replay.txt --muto` →
   `grep -c '^FRAME_VIDEO.*coincide'` = 13, `FRAME rese 5434`,
   `HASH ultimo a239d0a940bec0b9`, `FRAME ultimo 5499`,
   `AUDIO campioni 4042500`, `AUDIO hash c4b40fea06623a96`.
7. Banco, misurato: `harness/harness --tutte 2>&1 | awk '/^RIEPILOGO/ {n+=$3;
   ok+=$5; r++} END{print n" catture, "ok" OK, "r" riepiloghi"}'` →
   `1422 catture, 1422 OK, 335 riepiloghi`.
8. Gioco libero, misurato: `harness/gioca -n 600 --muto` → `FRAME rese 600`,
   `HASH ultimo ecd5acf694285489`, `AUDIO campioni 489510`,
   `AUDIO hash 8aedf41c0424df54`.
9. Dieci semi: per S=1..10 `python3 -I ricette/genera_ingressi.py S
   /tmp/ingr_S.txt 20000` poi `harness/gioca --riproduci /tmp/ingr_S.txt -n
   20000 --muto` → uscita 0, nessun FERMO.
10. Nessuna ROM aperta: `strace -f -e trace=openat harness/gioca -n 100 --muto
    2>&1 | grep -cE 'main64|1\.3jk|roms/'` → `0` (oggi 1: `../roms/ext/solomon/1.3jk`).
11. Clone senza ROM, dalla radice del repository:
    ```sh
    D=$(mktemp -d) && git ls-files -z -co --exclude-standard | tar --null -T - -cf - | tar -xf - -C $D \
      && rm -rf $D/roms $D/decomp/out && make -C $D/decomp gioco 2>&1 | grep -v '^make' \
      && cd $D/decomp && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy harness/gioca -n 600 --muto
    ```
    → nessun `ERRORE`/`estraggo`/`costruisco`, riga `Pronto: lancia
    decomp/harness/gioca (gioca --aiuto per le opzioni)`, poi le quattro righe
    della verifica 8 identiche, uscita 0 (attesa, non misurata; oggi:
    `ERRORE: mancano le ROM…` e `1.3jk non trovato…`, uscita 2). Nella stessa
    copia la verifica 10 dà `0`.
12. `python3 -I censimento_rom.py --verifica` → ultima riga, misurata:
    `CENSIMENTO OK: 201 occorrenze, 0 non classificate`.
13. Nessun file di codice sopra 800 righe:
    `wc -l decomp/harness/*.c decomp/harness/*.h tecmo/*.c tecmo/*.h decomp/estrai_*.py | awk '$1>800 && $2!="totale"'` → vuota.
14. Rinomine pure: `git diff --cached -M --name-status | grep '^R'` → 22
    righe, tutte `R100` (8 `.c` audio, 2 `.h` audio, 12 `video_dati`);
    `ls decomp/harness/*_dati_*` → vuoto. `git status --short` dalla radice:
    solo i file del perimetro; niente in `roms/`, `decomp/out/`, `decomp/ricette/`.

## Consegna

- Un commit su `main`, messaggio in italiano, per esempio «Driver audio e
  dati Tecmo in tecmo/, gioca senza ROM, brief 63», chiuso da
  `Co-Authored-By: <modello che esegue> <noreply@anthropic.com>`.
- Rapporto finale: massimo trenta righe con l'uscita delle verifiche 1-14 e
  ogni deviazione dal brief.

## Domande aperte

1. **`audio_dati.h`/`audio_dati.c` restano in `harness/`** (codice dell'autore:
   macro, struttura, serializzatore) e in `tecmo/` vanno solo i file generati;
   gli include si risolvono con `-I. -I$(TECMO)` senza toccare il testo
   generato. Proposta: così. «Preferenza».
2. **Estrattore del blob in `estrai_rom.py`** (non in `estrai_dati.py`, che
   descrive i dati musicali e scrive `SPARTITI.md`). Proposta: così.
   «Preferenza».
3. **`audio_immagine` scrive 16384 byte** (driver più dati) invece di 14906,
   così il round-trip in C è un `cmp` diretto contro `1.3jk`. Proposta: così,
   con `AUDIO.md` aggiornato. «Preferenza».
4. **`aud_usa_dati_estratti()` resta** (modo 1 ridondante, modo 2 controllo
   negativo) per non toccare `scheda_audio.c`, `audio_banco.c`,
   `audio_suona.c`; senza `AUDIO_DATI` gli strumenti audio usano ora le
   tabelle invece della ROM grezza (stessi byte, verifica 3). «Preferenza».
5. **`README.md`/`NOTICE.md` restano disallineati** (citano
   `decomp/harness/*_dati*`, «four ROM files», `make` che estrae lo zip) fino
   al lotto 64. Proposta: lasciarli, elenco delle righe in «Esito del lotto
   63» di `CENSIMENTO-ROM.md` come promemoria. «Preferenza».

## Decisioni (chat principale, 2026-11-06)

Tutte e cinque le domande sono preferenze, accettate con le proposte:
1. `audio_dati.h`/`.c` restano in `harness/`, include con `-I. -I$(TECMO)`.
2. Il blob del driver lo estrae `estrai_rom.py`.
3. `audio_immagine` scrive 16384 byte, controllo con `cmp` diretto su `1.3jk`.
4. `aud_usa_dati_estratti()` resta; senza `AUDIO_DATI` gli strumenti audio
   usano le tabelle.
5. `README.md` e `NOTICE.md` restano disallineati fino al lotto 64.
