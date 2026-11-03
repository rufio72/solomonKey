# Compito 41: H1, oracolo video (dump di video RAM e PNG dello stesso frame)

Data: 2026-11-03. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Primo lotto dello strato hardware SDL2 (`compiti/PIANO-HARDWARE.md`, voce H1).
Costruire l'oracolo video: per 6 stati campione, estrarre da MAME NELLO STESSO
FRAME i dump binari di videoram, colorram (due piani), spriteram e palette RAM,
più lo screenshot PNG di MAME di quel frame. File in `decomp/out/video/`,
formato documentato in `decomp/VIDEO.md`. Nessun codice C. I file servono da
oracolo al lotto H2 (renderizzatore software).

## Vincoli assoluti

- Il lavoro sta in `/home/rufio72/OpenSolomonsKey-master`; si creano solo:
  `decomp/video.lua`, `decomp/video.sh`, `decomp/verifica_video.py`,
  `decomp/VIDEO.md`, `decomp/out/video/*`, e in fondo a questo brief l'esito.
  In più, una riga in `STATO.md` (sezione hardware) se esiste un punto adatto.
- NON modificare `snap.lua`, `cattura.sh`, `replay.lua`, `replay.sh`, nessun
  file di `decomp/harness/`. Banco (304 voci, 1270 catture, 0 fallite),
  replay e sched_proto devono restare intatti (non rieseguirli: basta che
  `git status --short` mostri solo i file sopra).
- MAME sempre senza finestra: `-video none -sound none` e i quattro provider
  di input a `none` (tastiera, mouse, joystick, pistola), `SDL_VIDEODRIVER=dummy`,
  come `cattura.sh`/`replay.sh`. Mai una finestra MAME.
- Commenti, messaggi e documenti in italiano. Niente file di codice sopra le
  800 righe (qui sono tutti molto sotto). Niente installazioni. Niente git
  distruttivo (stash, reset, filter-branch, rebase).
- Python: `python3 -I` con PIL (già installato, 10.2.0; lo usa già `roms/decoded/decode.py`).
- Non scrivere nulla in `snapshot` di MAME fuori da `decomp/out/video/`.

## Cosa esiste già (fatti verificati)

| Fatto | Fonte |
|---|---|
| MAME 0.264 in `/usr/games/mame`, gioco `solomon`, rompath `roms/` (cartella del repo) | `mame -version`, `cattura.sh` |
| Stati e inneschi (frame, poke, posta, stanza forzata, ingressi «traccia») | `decomp/cattura.sh` righe 40-70, `decomp/snap.lua` |
| BG: videoram `dc00-dfff`, colorram `d800-dbff`; fondale da riga 4 colonna 1 | `harness/fondale.c:29`, `DATI.md:58` |
| FG (piano dei caratteri): videoram `d400-d7ff`, attributi `d000-d3ff` («attributi 0x400 sotto») | `harness/nemici_comuni.c:15` |
| Palette: NMI copia `c994`(0x20 byte)→`e400` e `c9ca`(0x80 byte)→`e500` (palette BG, voci 128-255); palette RAM `e400-e5ff` = 512 byte | `harness/nmi.c:158-161`, `DATI.md:12` |
| Spriteram `e000-e07f` (128 byte, 32 voci da 4 byte) copiata dalla NMI da `c8ee`; oltre `e080` fino a `e0ff` è zero | `SCHEDULER.md:112`, misura 2026-11-03 |
| `e604` in lettura restituisce DSW1, NON il ribaltamento schermo (latch in scrittura): non si può dumpare il flip dalla RAM | `harness/ram.h:46-53` |
| Con `-video none` `manager.machine.video:snapshot()` funziona e scrive il PNG (256x224 RGB) | misurato |
| Con `-snapname NOME` e `-snapshot_directory DIR` il file è `DIR/NOME.png` (senza `%g`, niente sottocartella `solomon/`) | misurato |

Frame (conteggio di `emu.register_frame_done`, come in `snap.lua`) e inneschi
già usati dal progetto; gli stessi di `cattura.sh`:

| Stato | Frame | Ingressi | Stanza forzata | Poke (frame:addr:val) | Posta (frame:codice) |
|---|---|---|---|---|---|
| titolo | 1000 | nessuno | - | - | - |
| gioco | 2400 | traccia | - | - | - |
| stanza10 | 2100 | traccia | 10 (tap su c243 frame 1500-1950) | - | - |
| bonus | 2400 | traccia | 4 | `2250:0xc19d:0,2250:0xc19e:0,2250:0xc24a:0` | `2300:0x1c` |
| finale | 2400 | traccia | - | `2250:0xc19d:0,2250:0xc19e:0,2250:0xc24b:0x32,2250:0xc24c:0x32,2250:0xc24d:0x32` | `2300:0x1c` |
| finale2 | 4500 | traccia | - | come finale | `2300:0x1c` |

Ingressi «traccia» (da `snap.lua`): `math.randomseed(1)`; frame 1510 moneta 1,
1530 rilascio; 1560 start 1, 1580 rilascio; dal 1900 ogni 8 frame un solo
tasto casuale tra P1 Left/Right/Up/Down/Button 1/Button 2 (campi
`:P1` `fields[...]`, moneta e start in `:SYSTEM`). Posta: scrive il codice nel
primo byte a zero di `c000-c03f` al frame indicato.

Misura (2026-11-03): in «bonus» e «finale» a 2400 lo schermo mostra la
schermata «Your Rest Bonus» (il finale vero compare in finale2, 4500: stanza
con il pentagramma e molti sprite). Il titolo ha spriteram tutta a zero.

## Struttura da realizzare

### `decomp/video.lua` (MAME)
Prototipo già provato fuori dal repository; riscriverlo con commenti in
italiano. Variabili d'ambiente: `VIDEO_STATO` (nome del file), `VIDEO_DIR`
(cartella delle uscite), `VIDEO_FRAME` (frame del dump), `VIDEO_INGRESSI`
(`nessuno`|`traccia`), `VIDEO_ROOM` (stanza forzata, vuota = nessuna),
`VIDEO_POKE`, `VIDEO_POSTA` (formato `frame:addr:val` e `frame:codice`,
virgole, 0x ammesso). Logica, copiata da `snap.lua`: contatore `frames`
incrementato in `emu.register_frame_done`; a `frames==1500` con stanza
`mem:install_read_tap(0xc243,0xc243,...)` che restituisce la stanza, tolto a
`frames==1950`; `muovi()` ingressi; `inn()` poke/poste; ai frame sopra.
Quando `frames == VIDEO_FRAME`, DENTRO la stessa callback, senza altro codice
in mezzo: scrivere i sei dump con `mem:read_u8` su `maincpu` `program`, poi
`manager.machine.video:snapshot()`, stampare `VIDEO: stato <S> frame <N>`,
`manager.machine:exit()`. I dump vanno prima del `snapshot()` e senza far
girare la CPU tra uno e l'altro (garantisce lo stesso frame).
File prodotti (nomi parlanti, in `VIDEO_DIR`):

| File | Indirizzi | Byte |
|---|---|---|
| `<S>_bg_colorram.bin` | d800-dbff | 1024 |
| `<S>_bg_videoram.bin` | dc00-dfff | 1024 |
| `<S>_fg_colorram.bin` | d000-d3ff | 1024 |
| `<S>_fg_videoram.bin` | d400-d7ff | 1024 |
| `<S>_spriteram.bin` | e000-e07f | 128 |
| `<S>_palette.bin` | e400-e5ff | 512 |
| `<S>.png` (da MAME) | schermo 256x224 RGB | - |

(Nel prototipo i nomi erano `colorram_bg` ecc. e c'era un `flip`: usare i
nomi della tabella, senza flip.)

### `decomp/video.sh`
Uso da `decomp/`: `sh video.sh [stato...]` (senza argomenti: tutti i sei).
Per ogni stato lancia MAME con questi argomenti (esatti, senza `-debug`):
`mame solomon -rompath "$ROMPATH" -video none -sound none -keyboardprovider
none -mouseprovider none -joystickprovider none -lightgunprovider none
-skip_gameinfo -nothrottle -snapshot_directory "$DIR" -snapname <stato>
-autoboot_script video.lua`, con `ROMPATH="$(cd .. && pwd)/roms"`,
`DIR=${VIDEO_OUT:-out/video}`; stati in parallelo (`&` e `wait`, sicuro:
ogni istanza scrive file propri; misurato con 5 in parallelo, ~40 s; finale2
~75 s). Tabella stati come sopra (usare un `case`). Log in
`$DIR/<stato>.log` (non committare i log: aggiungere `decomp/out/video/*.log`
a `.gitignore` solo se il repository lo richiede, altrimenti cancellarli a
fine corsa). Stampa `VIDEO <stato> OK` se esistono i 7 file, altrimenti
`VIDEO <stato> FALLITO` ed esce con 1.

### `decomp/verifica_video.py`
`python3 -I verifica_video.py [DIR]` (predefinito `out/video`): per ogni
`<S>.png` stampa `<S> <larghezza>x<altezza> <sha1 dei pixel RGB>` con
`hashlib.sha1(Image.open(f).convert("RGB").tobytes())`; per ogni `.bin` la
riga `<nome> <byte> <sha1 completo>`. Uscita ordinata per nome. Serve a
confrontare due corse (il PNG intero può differire nei metadati: si confronta
solo l'hash dei pixel).

### `decomp/VIDEO.md` (breve, ~50 righe)
Tabella indirizzi/dimensioni dei file; come si rigenera (`sh video.sh`); tabella
stati (frame, inneschi); ordine dei campi: spriteram 32 voci da 4 byte a
partire da e000 (significato dei byte NON deciso in H1, è lavoro di H2);
palette 256 voci da 2 byte da e400 (ordine byte/nibble NON deciso in H1: H2 lo
ricava confrontando i pixel del PNG con i colori; nel dump del titolo i primi
byte sono `00 00 00 02 4f 07 27 00 c4 05`); il flip schermo non si salva
(latch di sola scrittura, `e604` in lettura è DSW1); «bonus» e «finale»
mostrano la schermata del bonus di fine quadro, finale2 il finale vero;
le coppie colorram+videoram sono due piani distinti (FG sopra il BG? ordine di
priorità da ricavare in H2). Avvertenza esplicita: il PNG è preso nella stessa
callback `frame_done` dei dump, ma che il PNG corrisponda al contenuto RAM
letto subito dopo è un'ipotesi da confermare in H2 (se a H2 il fondale
risulta sfasato di un frame, il lotto H1 va rifatto: segnalarlo).

## Verifiche obbligatorie

Eseguire da `decomp/`. Tutte le uscite sono misurate su due corse
indipendenti del prototipo (2026-11-03) salvo dove scritto.

1. `sh video.sh && ls out/video/*.bin | wc -l && ls out/video/*.png | wc -l`
   Atteso: sei righe `VIDEO <stato> OK`, poi `36` e `6`.
2. Dimensioni: `wc -c out/video/titolo_*.bin` Atteso: 1024 per i quattro
   piani, 128 per `spriteram`, 512 per `palette`.
3. Pixel dei PNG: `python3 -I verifica_video.py out/video | grep '256x224'`
   Atteso: sei righe `<stato> 256x224 <sha1>` con questi hash dei pixel (misurati):
   - titolo `b88e0bb3c3b4b620979820e0b03b7801dd7eee47`
   - gioco `539f7014f22780f0de014ca6284dac955a2f8ca2`
   - stanza10 `8c75598ddd53096bb4427c825ae15293d798f7fa`
   - bonus `f8e1ded4a5b3d20864ea66f90be3dd43026245ac`
   - finale `50357fb27c65ce437446f17a0d94ac465991d4eb`
   - finale2 `6193568923ca8779433bac78e256882ea85066f3` (una sola corsa misurata)
4. Hash prefissi (16 car.) di `titolo_*`: bg_colorram `cb1ada64e6c96336`,
   bg_videoram `b5dfde746e26901a`, fg_colorram `f70cd132e9d011d7`, fg_videoram
   `321e51a3e162f4f1`, spriteram `0ae4f711ef5d6e9d` (128 zeri), palette
   `831dbb2ad5b064fc`. Per `gioco`: palette `e7f116d3cbcd45ef`, spriteram
   `c9dac98c2e06af0c`. Per `stanza10`: spriteram `9555b7935f81d09f`.
5. Riproducibilità: `VIDEO_OUT=/tmp/vid2 sh video.sh` (cartella fuori dal repo,
   da creare con `mkdir -p`), poi
   `python3 -I verifica_video.py out/video > /tmp/h1_a.txt;
   python3 -I verifica_video.py /tmp/vid2 > /tmp/h1_b.txt; cmp /tmp/h1_a.txt /tmp/h1_b.txt && echo IDENTICI`
   Atteso: `IDENTICI`. (Con la scelta di `/tmp` rispettare che `video.sh`
   crei la cartella di uscita.)
6. Gli stati differiscono: `sha1sum out/video/*_bg_videoram.bin | cut -c1-12 | sort -u | wc -l`
   Atteso: `6` (bonus e finale hanno lo stesso fg e la stessa palette, bg diverso: misurato).
7. Nessun file extra: `ls out/video | grep -v -E '\.(bin|png)$' ; ls out/video | wc -l`
   Atteso: nessuna riga dal primo comando (log cancellati a fine corsa), `42` dal secondo.

8. Ambito: `git status --short`
   Atteso: solo `decomp/video.lua`, `decomp/video.sh`, `decomp/verifica_video.py`,
   `decomp/VIDEO.md`, `decomp/out/video/` e al più `STATO.md`, questo brief,
   `.gitignore`; nessun file di `harness/`, `snap.lua`, `cattura.sh`, `replay*`.
9. Nessun MAME con finestra: `grep -c -E 'video (opengl|bgfx|soft|accel)' video.sh`
   Atteso: `0`; e `grep -c -- '-video none' video.sh` atteso `1` o più.
10. File di codice sotto 800 righe: `wc -l video.lua video.sh verifica_video.py`
    Atteso: ognuno sotto 800 (previsto ~80, ~60, ~30).

## Consegna

Commit unico con messaggio «Lotto H1: oracolo video (dump di video RAM e PNG
nello stesso frame)» e riga finale
`Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>` (indicare il modello
vero che ha eseguito). Non fare push. In fondo al brief annotare esito e
verifiche. Rapporto finale: massimo trenta righe con l'uscita delle dieci
verifiche e le deviazioni.

## Domande aperte

1. Stati «bonus» e «finale» a frame 2400 mostrano entrambi la schermata del
   bonus di fine quadro e non un bonus/finale distinti; il finale vero è
   finale2. Proposta: tenerli tutti e sei (il piano ne chiede 4-6, costano
   pochi secondi e i nomi seguono `cattura.sh`). Marca: preferenza.
2. Il PNG è scattato nella callback `frame_done`, e non è provato che mostri
   esattamente la RAM letta in quel callback (potrebbe essere il frame
   precedente se MAME aggiorna lo schermo dopo). Proposta: lasciare la prova a
   H2 (il confronto pixel lo smentisce subito) e in H1 documentarlo in
   `VIDEO.md`. Marca: preferenza.
3. Si dumpa solo il minimo (sei regioni); non c'è un dump dei 16 KB `c000-ffff`.
   Proposta: aggiungere `<S>_ram.bin` (16384 byte) solo se H2 lo chiede. Marca: preferenza.

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Tutte le domande sono preferenze, accolte con la proposta dell'analista:
1. Sì: si tengono tutti e sei gli stati (titolo, gioco, stanza10, bonus, finale, finale2), anche se bonus e finale a frame 2400 mostrano la stessa schermata «Your Rest Bonus»; il finale vero è finale2.
2. Sì: la corrispondenza PNG ↔ RAM nella stessa callback resta un'ipotesi dichiarata in VIDEO.md; la prova vera è il confronto pixel di H2.
3. Sì: niente dump dell'intera RAM c000-ffff; si aggiunge solo se H2 lo chiede.

## Esito (2026-11-03)

Eseguito da Claude Sonnet 5.5. Le dieci verifiche passano con le uscite
attese: 6 righe OK, 36 bin e 6 png; dimensioni 1024/128/512; i sei hash dei
pixel coincidono; hash prefissi di titolo, gioco, stanza10 coincidono;
riproducibilità IDENTICI; 6 bg_videoram distinti; 42 file, nessun log;
nessun `video` con finestra; righe 97/49/18. Deviazione: aggiunta in
`.gitignore` di `!decomp/out/video/` (altrimenti `decomp/out/*` ignorava i file).
