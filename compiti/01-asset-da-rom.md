# Compito 01: ricostruire `res/` dal set ROM arcade USA

Data: 2026-10-19. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Creare la cartella `res/` (20 fogli PNG, 27 sfondi `res/backgrounds/N.png`,
10 wav in `res/audio/`) a partire dai chip già estratti in `roms/ext/solomon/`
e dai dump RAM di MAME, con script rieseguibili in `tools/`, così che
`build/solomons_key` lanciato dalla radice parta senza assert e mostri il
livello 0 con la grafica originale arcade.

Vincoli assoluti:
- Nessuna modifica fuori da `/home/rufio72/OpenSolomonsKey-master`; dentro,
  si scrive solo in `res/`, `tools/`, `roms/decoded/mame/rooms/` (dump) e
  `compiti/` (screenshot). Nessuna modifica a `src/`: il binario esistente
  `build/solomons_key` (compilato oggi, con la correzione a
  `src/x11_OpenSolomonsKey.cpp:441` già fatta) va usato così com'è. Se una
  modifica al C++ sembrasse indispensabile, fermarsi e scriverla nel rapporto.
- Non installare pacchetti. Disponibili e misurati: `python3` con PIL 10.2.0 e
  numpy 1.26.4 (eseguire sempre `python3 -I`), `mame` 0.264 in
  `/usr/games/mame`, `xdotool`, `xwd`, `gnome-screenshot`; `PIL.ImageGrab.grab()`
  funziona su `DISPLAY=:1` (misurato: 1920x1080). Non ci sono ImageMagick,
  ffmpeg, scrot.
- Set ROM: solo `roms/solomon.zip` (USA); ignorare `solomonj.zip`.
- Audio: i 10 wav sono placeholder silenziosi di formato valido (scelta del
  brief: l'estrazione dal chip sonoro YM è fuori perimetro).
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe (`src/sprites.cpp`, 1831 righe, è già oltre e non si tocca).

## Cosa esiste già (fatti verificati)

### Il port

| File | Righe | Cosa conta |
|---|---|---|
| `src/resources.h` | 191 | macro `ALL_TILEMAPS` (20 fogli, righe 13-32), `ALL_BACKGROUNDS` (27, righe 38-64), `ALL_CHARACTERS` con `DEF_ANIM(char, nome, durata, {col,riga}, frame, loop)` (righe 71-176), `ALL_SOUNDS` (11 voci, 10 file: `splat.wav` è usato due volte, righe 179-189) |
| `src/resources.cpp` | 167 | `load_tilemap_textures()` r.112: `assert(data)` su ogni PNG; `gl_load_background_texture(long)` r.104: `assert(data)`; `load_image_as_rgba_pixels` r.97 = `stbi_load(name,&w,&h,&n,4)` (sempre RGBA) |
| `src/gl_graphics.cpp` | 272 | `gl_load_rgba_tilemap(data,w,h,rows,cols)` r.101: cella = `w/cols` x `h/rows`, texture array `GL_NEAREST`; `gl_background_draw()` r.193: lo sfondo è UNA texture stirata su 960x768 unità logiche a partire da (32,64) |
| `src/audio.cpp` | 314 | `Wave_load_from_file` r.81: RIFF/WAVE, accetta SOLO chunk `fmt ` e `data` (altro → `assert(0)`), `format_tag==1`, 16 bit, `nchannels==2`, `samples_per_sec==48000` (costanti `AUDIO_CHANNELS`, `AUDIO_SAMPLERATE` in `src/OpenSolomonsKey.h:113-114`) |
| `src/OpenSolomonsKey.cpp` | 293 | `cb_init` r.144 carica tutto all'avvio e `gl_load_background_texture(17)`; `draw_extra_stuff` r.92 (cornice da `misc.png`); `draw_ribbon` r.179; `draw_ui` r.216 |
| `src/text.cpp` | 79 | mappa carattere → cella di `font.png` (vedi sotto) |
| `src/menu.cpp` | 117 | logo: `TM_logo` cella 0 disegnata 644x352 logiche (r.61) |
| `src/levels.cpp` | 684 | `scene_draw_tilemap` r.67: blocco fragile = cella 0, mezzo = 4, solido = 5 di `essentials.png`; r.461 `add_tilemap_background(num)` legge `BG n` dal `.osk` |
| `src/map/entity.h` | 130 | `enum PickupType` r.31 (0..33, `PT_Count`=34) = indice di cella in `pickups.png` (`objects.h:258,491`) |

Unità: 1 tile del port = 64 unità logiche = 16 px arcade; il livello è 15x12
tile = 960x768 logiche = 240x192 px arcade. Lo sfondo copre esattamente
l'area di gioco. Le celle dei fogli vengono disegnate con `GL_NEAREST`, quindi
la risoluzione nativa (16x16 per sprite, 8x8 per i caratteri) va bene.

Fogli (`ALL_TILEMAPS`, nome → righe x colonne → dimensione cella → PNG):

| PNG | rows x cols | cella | PNG px | contenuto |
|---|---|---|---|---|
| logo3.png | 1x1 | 168x88 | 168x88 | logo del titolo (vedi sotto) |
| essentials.png | 5x5 | 16 | 80x80 | blocchi, porta, chiave, specchio |
| dana_all.png | 5x6 | 16 | 96x80 | Dana |
| dana_fireball.png | 4x1 | 16 | 16x64 | palla di fuoco di Dana |
| fairies_all.png | 2x2 | 16 | 32x32 | fate |
| demon_head.png | 2x8 | 16 | 128x32 | testa di demone |
| goblin_all.png | 4x7 | 16 | 112x64 | goblin |
| blue_flame.png | 2x7 | 16 | 112x32 | fiamma blu |
| ghost_all.png | 2x3 | 16 | 48x32 | fantasma |
| panel_monster_all.png | 3x5 | 16 | 80x48 | mostro del pannello + fiamma |
| wyvern_all.png | 2x3 | 16 | 48x32 | viverna |
| dragon_all.png | 4x5 | 16 | 80x64 | drago + fuoco |
| spark_ball_all.png | 1x8 | 16 | 128x16 | palla di scintille |
| gargoyle_all.png | 3x6 | 16 | 96x48 | gargoyle |
| effects.png | 5x4 | 16 | 64x80 | fumo, stella, flash, colpo |
| font.png | 6x16 | 8 | 128x48 | font ASCII |
| misc.png | 4x4 | 16 | 64x64 | cornice |
| pickups.png | 5x7 | 16 | 112x80 | 34 oggetti, indice = `PickupType` |
| ribbons.png | 1x3 | 16 | 48x16 | barra degli slot in basso |
| ribbons_fire.png | 1x6 | 8 | 48x8 | icona fuoco negli slot (celle 4 e 5) |

Celle usate dal codice (da `DEF_ANIM` e grep): `essentials` (0,0) blocco
fragile, (4,0) blocco fragile danneggiato, (0,1) blocco solido, (3,1) specchio
(KMirror), (1,1) e (2,1) riquadri di debug (vanno bene un contorno di 1 px),
(0..4,2) porta che si chiude (5 frame), (0..4,3) porta che si apre (5 frame),
(0,4) chiave. `font` (`src/text.cpp`): cella 14 = '.', celle 16..31 = '0'..'?',
celle 33..58 = 'A'..'Z', cella 64 = '%', celle 65..90 = 'a'..'z'. `misc`
(`draw_extra_stuff`): celle 8,9 = barra alta (disegnate specchiate in x e y a
y=24 logiche), 10,11 = barra bassa (a y=800 logiche), 4,5 = barra verticale
destra (x=992), 6,7 = barra verticale sinistra (x=-32). `ribbons`: cella 0 =
capo sinistro, 1 = slot vuoto, 2 = capo destro; `ribbons_fire` celle 4 (alto) e
5 (basso), tinte dal port. `dana_all` cella 2 = qualsiasi (serve solo come
barra nera tinta), cella 5 = icona vita. `pickups` cella n = `PickupType` n
(Bag10000=0, Bag20000, Bag100, Bag200, Bag500, Bag1000, Bag2000, Bag5000,
Bell=8, Jewel20000, Jewel1000, Jewel2000, Jewel10000, PotionDestruction,
Hourglass, Jewel5000, Jewel50000, Jewel100, Jewel200, Jewel500, JewelChange,
JewelRange, JewelRange2, PotionFire, PotionSuperFire, PotionFireGrowth,
PotionLife, PotionTime1, PotionTime2, ScrollExtension, SolomonSeal, Sphinx,
PaperCrane, SolomonsKey=33); cella 34 vuota.

### I chip grafici (verificati pixel per pixel)

`roms/decoded/decode.py` ha i layout giusti ma **l'ordine dei piani
invertito**: in MAME il piano 0 è il bit più significativo del pen. Il pen
corretto è `pen = Σ bit_p << (3-p)` (cioè il nibble di `decode.py` con i 4
bit rovesciati: 1↔8, 2↔4, 3↔12, 5↔10, 7↔14, 11↔13). Vale per tutti e tre i
set (caratteri, tile, sprite). Con questa correzione, la ricostruzione dello
schermo da `ram_level1.bin` e da `ram_title.bin` coincide con
`snap_level1.png` e `snap_title.png` in **0 pixel su 57344** (misurato).

- Caratteri FG 8x8: `12.3t` + `11.3r` concatenati, 32 byte/tile, 2048 tile;
  bit del pixel (x,y) piano p a offset `p + 4*x + 32*y` dal tile.
- Tile BG 8x8: `10.3p` + `9.3m`, stesso layout.
- Sprite 16x16: `2.5lm`,`3.6lm`,`4.7lm`,`5.8lm` = piani 0..3 (16 KB ciascuno),
  32 byte per piano per sprite, 512 sprite; offset `x` per x<8, `64+(x-8)`
  per x≥8; `8*y` per y<8, `128+8*(y-8)` per y≥8.
- Palette: 256 voci a 0xe400-0xe5ff, little endian, `xxxxBBBBGGGGRRRR`,
  canale a 8 bit = nibble*17. Voci 0-127 (banchi 0-7) per FG e sprite,
  128-255 (banchi 8-15) per BG.
- FG: colorram 0xd000, videoram 0xd400; BG: colorram 0xd800, videoram 0xdc00;
  1 KB ciascuna, indice = riga*32 + colonna (32x32). Attributo (stesso
  formato FG e BG): bit 0-2 = bit alti dell'indice (`code = v + 256*(attr&7)`),
  bit 4-6 = banco colore (`base = (attr>>4&7)*16`, +128 per il BG), bit 7 =
  flip X (verificato sulle statue destre), bit 3 = flip Y (non osservato nei
  dump, trattarlo come MAME). FG trasparente dove pen 0; BG opaco.
- Spriteram 0xe000-0xe07f, 32 voci da 4 byte `b0 b1 b2 b3`:
  `code = b0 + 16*(b1 & 0x10)` (0..511), `banco = (b1 & 0x0e) >> 1`,
  flip X = `b1 & 0x40`, flip Y = `b1 & 0x80`, x = `b3`, y = `241 - b2`;
  pen 0 trasparente. Schermo visibile: righe 2..29 (y schermo = y - 16).
- Area di gioco: BG righe 4..27, colonne 1..30 → 240x192 px a (8,16); tile
  del port (c,r) ↔ caratteri (1+2c, 4+2r). Colonna 0 e 31 e righe 2-3, 28-29
  sono cornice e HUD (`misc`, font). Il BG sotto i blocchi è completo: i
  blocchi, la porta, la chiave e gli oggetti stanno nel FG come gruppi 2x2
  (`n n+1 / n+2 n+3`). Misurati al livello 1: blocco fragile = caratteri
  0x10-0x13 banco 2; blocco solido = 0x14-0x17 banco 5; porta chiusa =
  0xc4-0xc7 banco 7; chiave = 0xe0-0xe3 banco 7; campanella (`PT_Bell`) =
  0xd8-0xdb banco 7; barra bassa FG = 0xdc 0xdd 0xde 0xdf banco 5 (riga 28);
  font = ASCII a 0x20-0x7f (maiuscole 0x41-0x5a, minuscole 0x61-0x7a, '%'
  0x25, '.' 0x2e). Tile BG misurati: cornice colonna 0 = 0x08,0x0a,0x0c,0x0e
  banco 5 (si ripete ogni 4 righe), colonna 31 = 0x09,0x0b,0x0d,0x0f banco 5,
  barra alta riga 3 = 0x04 0x05 0x06 0x07 banco 5; slot in basso (righe 28-29,
  banco 7): capo sinistro 0x92/0x93, slot vuoto 0x82/0x83, slot con fuoco
  0x84/0x85, capo destro 0x90/0x91 (alto/basso).
- Logo: nel dump della schermata titolo (frame 700 della demo, nessuna
  moneta) il logo è nel FG, righe 5-15 colonne 6-26 → ritaglio 168x88 px a
  (48,24), banchi 2 e 3 della palette di quel dump.

### MAME

Stanza del gioco: variabile `0xc243` (indice 0-based; `0xc246` è solo il
contatore mostrato da "ROUND"). Scriverla da Lua non basta (lo start la
azzera e carica la stanza nello stesso frame): serve un **read tap**
`mem:install_read_tap(0xc243, 0xc243, "room", function(o,d,m) return N end)`
attivo dal frame 1500 al 1950. Misurato per N=0..51: ogni stanza carica il
proprio layout; la stanza 50 (N=49) non produce il dump (il gioco non arriva
al frame 2000) e va saltata; 51 dump su 52. Fra le 51 stanze ci sono 43
coppie distinte (layout BG righe 4-27, palette banchi 8-15); stanze con lo
stesso sfondo: {6,16}, {13,22,32}, {36,43,44}, {37,42}, {45,46}, {48,49}.
Con `-video none -nothrottle -sound none -skip_gameinfo` una corsa di 2010
frame dura 1,5 s e `manager.machine.video:snapshot()` funziona lo stesso.
Script Lua misurato (una stanza):

```lua
local frames = 0
local mem = manager.machine.devices[":maincpu"].spaces["program"]
local coin, start, tap
for pname, port in pairs(manager.machine.ioport.ports) do
  for fname, field in pairs(port.fields) do
    if fname == "Coin 1" then coin = field end
    if fname == "1 Player Start" then start = field end
  end
end
emu.register_frame_done(function()
  frames = frames + 1
  if frames == 700 then --[[ dump titolo: solo nel modo "title" ]] end
  if frames == 1500 then tap = mem:install_read_tap(0xc243, 0xc243, "room", function(offset, data, mask) return ROOM0 end) end
  if frames == 1510 then coin:set_value(1) end
  if frames == 1530 then coin:set_value(0) end
  if frames == 1560 then start:set_value(1) end
  if frames == 1580 then start:set_value(0) end
  if frames == 1950 then tap:remove() end
  if frames == 2000 then
    local f = io.open("ram_room_NN.bin", "wb")
    for a = 0xc000, 0xffff do f:write(string.char(mem:read_u8(a))) end
    f:close(); manager.machine.video:snapshot()
  end
  if frames == 2010 then manager.machine:exit() end
end)
```

Riga di comando misurata (dalla cartella in cui si vogliono i file):
`mame solomon -rompath /home/rufio72/OpenSolomonsKey-master/roms -autoboot_script X.lua -nothrottle -video none -sound none -skip_gameinfo -snapshot_directory .`
(lo snapshot finisce in `./solomon/0000.png`). `os.getenv` in Lua non è
verificato: generare un file Lua per stanza con `sed` sul modello.

Coppie (indice sprite, banco) osservate nella spriteram durante 12000 frame
di demo (197 coppie, un solo indice con due banchi: 0 → {1,5}):
- banco 0: 136 137 140 141 143 144 145 146 152-155 160 161 256 342 343 352-359 466 467 474 (136/137 = torce)
- banco 1: 0 116-119 121-123 128-131 147-151 216-222 224-229 232-239 258 259 272-281
- banco 2: 1 17 30 32 34 35 42-44 46 47 52-58 174 175
- banco 3: 384-387 390 394 400-407 416-419 424-426 431-435 (400-407 goblin, 424-426 fata)
- banco 5: 0 192-207 209-215 230 240 241 243
- banco 6: 2-14 20-29 292-295 (Dana; nel livello 1 Dana è l'indice 23 con flip X quando guarda a sinistra)
- banco 7: 33 59 61-63 156-159 162-167 189-191 (effetti)

## Struttura da realizzare

Tutto in `tools/`, Python con PIL (niente numpy obbligatorio), ogni script
sotto le 400 righe, eseguibile dalla radice del progetto con `python3 -I`.

1. `tools/oskrom.py` (libreria): `leggi_chip()`, `decodifica_caratteri()`,
   `decodifica_tile()`, `decodifica_sprite()` con l'ordine dei piani corretto;
   `palette_da_dump(path) -> [256 RGB]`; `disegna_tile(img, tile, x, y, pal,
   base, flipx, flipy, trasparente)`; `cella_2x2(code, banco, pal) -> Image
   16x16 RGBA`; `sprite(idx, banco, pal, flipx, flipy) -> Image RGBA`;
   `layer_bg(dump) -> Image 256x256`, `layer_fg(dump) -> Image RGBA`.
2. `tools/dump_mame.sh` + `tools/mame_room.lua.in`: per N in 1..52 (salta 50)
   genera il Lua con `sed` e scrive `roms/decoded/mame/rooms/ram_room_N.bin`
   e `roms/decoded/mame/rooms/snap_room_N.png`; modo "title": dump al frame
   700 in `roms/decoded/mame/rooms/ram_title700.bin`. Idempotente (salta i
   file già presenti se si passa `--keep`).
3. `tools/make_backgrounds.py`: `res/backgrounds/N.png` = BG righe 4-27,
   colonne 1-30 (240x192 RGB) della stanza `STANZE[N]`, con
   `STANZE = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,17,18,19,20,21,23,24,25,26,27,28,29]`
   (le prime 27 stanze con sfondo distinto, in ordine).
4. `tools/sheet_map.py`: tabella scritta a mano, una voce per foglio:
   `{"file": "res/dana_all.png", "rows": 5, "cols": 6, "cella": 16, "celle": {(col,riga): sorgente}}`
   dove `sorgente` è `("sprite", idx, banco, flipx, flipy)`, `("char2x2",
   code, banco)`, `("char", code, banco)`, `("tile2x2"/"tile", code, banco)`
   per i tile BG (banco +8), `("ritaglio", dump, x, y, w, h)` per il logo,
   `("contorno",)` per i riquadri di debug, assente = trasparente. Palette:
   `ram_room_1.bin` (sprite/FG) e il dump della stanza per i BG.
5. `tools/make_sheets.py`: legge `sheet_map.py`, compone i 20 PNG RGBA (pen 0
   → alpha 0), e scrive `tools/out/contatti/*.png`: fogli di contatto con
   indice stampato di tutti i 512 sprite in ciascun banco 0-7 e dei 512 gruppi
   2x2 di caratteri nei banchi 0-7, per compilare la tabella a vista e per il
   controllo. Il font: celle 0..95 = carattere 0x20+n, tranne la cella 64 =
   0x25; pen diverso da 0 → bianco opaco (il port tinge il testo).
   `misc`: celle 4,5 = colonna 31 (tile 0x09/0x0b e 0x0d/0x0f) nella metà
   sinistra della cella; celle 6,7 = colonna 0 (0x08/0x0a, 0x0c/0x0e) nella
   metà destra; celle 8,9 = `0x04 0x05` e `0x06 0x07` alle righe 6..13 della
   cella, poi specchiate in x e y nel PNG (il port le rispecchia di nuovo);
   celle 10,11 = `0xdc 0xdd` e `0xde 0xdf` (FG banco 5) alle righe 8..15.
   `ribbons`: colonna 8x16 centrata nella cella (x 4..11). `logo3.png` =
   `layer_fg(ram_title700.bin)` ritagliato a (48,24,216,112).
6. `tools/make_audio.py`: 10 wav PCM 16 bit stereo 48000 Hz, 0,5 s di
   silenzio (96000 byte di dati), solo chunk `fmt ` e `data`.
7. `tools/check_res.py`: conta i file (20/27/10), apre ogni PNG e verifica
   `w % cols == 0`, `h % rows == 0`, cella = quella attesa della tabella sopra,
   sfondi 240x192; apre ogni wav e verifica intestazione, 2 canali, 48000 Hz,
   16 bit, nessun chunk oltre `fmt ` e `data`. Stampa una riga per problema e
   infine `OK: 20 fogli, 27 sfondi, 10 wav`; uscita 1 se ci sono problemi.
8. `tools/verify_decode.py`: ricostruisce lo schermo da `ram_level1.bin` e
   `ram_title.bin` con le regole sopra e stampa `level1 pixel diversi: 0` e
   `title pixel diversi: 0` rispetto agli snapshot.
9. `tools/screenshot_port.py OUT.png`: lancia `./build/solomons_key` dalla
   radice, attende 3 s, `PIL.ImageGrab.grab()`, salva, termina il processo.

Associazione indice sprite → cella: si compila a mano in `sheet_map.py`
guardando i fogli di contatto, partendo dai banchi misurati sopra e dalla
tabella `DEF_ANIM` (numero di frame per riga). Orientamento: il port
specchia da sé, quindi mettere nel foglio i frame così come stanno nel chip
(flip falsi) salvo che la direzione "a riposo" di Dana risulti opposta a quella
del livello 0 a schermo. Per i 34 oggetti usare i gruppi 2x2 dei caratteri
(sacchetti, gioielli, pozioni, clessidra, pergamene stanno nei gruppi
64..127 del foglio di contatto, cioè caratteri 0x100-0x1ff) scegliendo il
banco per colore; dove non si trova un oggetto, cella vuota e riga nel
rapporto. Dove un'animazione del port ha più frame di quelli trovati,
ripetere l'ultimo frame.

## Verifiche obbligatorie

Tutte dalla radice `/home/rufio72/OpenSolomonsKey-master`.

1. `ls res res/backgrounds res/audio | wc -l` → `64` (misurato su un albero
   finto con gli stessi nomi); `find res -type f | wc -l` → `57`.
2. `python3 -I tools/check_res.py; echo $?` → ultima riga
   `OK: 20 fogli, 27 sfondi, 10 wav` e `0`.
3. `python3 -I tools/verify_decode.py` → `level1 pixel diversi: 0` e
   `title pixel diversi: 0` (misurato con la mia implementazione).
4. `ls roms/decoded/mame/rooms/ram_room_*.bin | wc -l` → `51`;
   `ls roms/decoded/mame/rooms/ram_title700.bin` esiste.
5. `timeout 5 ./build/solomons_key; echo $?` → nessuna riga contenente
   `Assertion`, codice `124`. Stato attuale misurato senza `res/`:
   `solomons_key: .../src//resources.cpp:107: void gl_load_background_texture(long int): Assertion \`data' failed.`
   seguito da core dump.
6. `python3 -I tools/screenshot_port.py compiti/01-screenshot.png && python3 -I -c "from PIL import Image; print(Image.open('compiti/01-screenshot.png').size)"`
   → una dimensione non nulla; guardare l'immagine: finestra del port con muri
   di pietra arcade sotto il livello 0, blocchi e Dana con i colori originali
   (attesa, non misurata). Descrivere in una riga nel rapporto.
7. Codice non toccato: `find src -type f \( -name '*.cpp' -o -name '*.h' \) | sort | xargs md5sum | md5sum`
   → `cc06ea6b62ee5080474e64222e5029da  -` (misurato prima di iniziare);
   `find src -newer compiti/01-asset-da-rom.md -type f` → vuoto.
8. Nessun file di codice sopra le 800 righe:
   `find src tools -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.py' -o -name '*.sh' -o -name '*.lua*' \) | xargs wc -l | awk '$1>800 && $2!="totale"'`
   → solo `1831 src/sprites.cpp`.
9. Rieseguibilità: `rm -rf res && python3 -I tools/make_backgrounds.py && python3 -I tools/make_sheets.py && python3 -I tools/make_audio.py && python3 -I tools/check_res.py`
   → termina con `OK: 20 fogli, 27 sfondi, 10 wav`.
10. Compilazione: non serve (nessuna modifica a `src/`); solo se il binario
    mancasse, `mkdir -p build && bash build.sh` senza righe `error:` (attesa,
    non misurata).

## Consegna

- Cartelle `res/` (57 file), `tools/` (script sopra, più
  `tools/out/contatti/`), `roms/decoded/mame/rooms/`, `compiti/01-screenshot.png`.
- Niente commit: il progetto non è un repository git. In fondo a questo file
  aggiungere una sezione `## Esito` con data, uscita delle verifiche 1-9 e la
  riga `Co-Authored-By: <modello> <noreply@anthropic.com>`.
- Rapporto finale: massimo trenta righe con l'uscita delle verifiche, le
  celle lasciate vuote (foglio, cella, motivo) e ogni deviazione dal brief.

## Domande aperte

1. Quali 27 sfondi («preferenza»): proposta `STANZE` = le prime 27 stanze con
   sfondo distinto in ordine (1-15, 17-21, 23-29). Alternativa: stanze 1-27
   anche con i doppioni 6/16 e 13/22.
2. Font bianco anziché nel banco 7 («preferenza»): proposta bianco, perché il
   port tinge il testo con `NRGBA` e un glifo colorato darebbe tinte sporche.
3. Orientamento di base di Dana e dei nemici («preferenza»): proposta frame
   non specchiati come nel chip; se a schermo Dana guarda dal lato sbagliato
   a riposo, specchiare tutte le celle di `dana_all.png` nel tool.
4. Audio placeholder silenzioso («scelta di progetto»): il brief lo prescrive
   per vincolo; se si vorrà l'audio originale servirà un compito a parte
   (registrazione da MAME con `-wavwrite` e taglio, oppure emulazione YM).
5. Oggetti e frame non identificabili a vista («preferenza»): proposta cella
   trasparente ed elenco nel rapporto; nessun segnaposto inventato.

## Decisioni (chat principale, 2026-10-19)

- Domanda 1 (preferenza): accettata, `STANZE` = prime 27 stanze con sfondo distinto (1-15, 17-21, 23-29).
- Domanda 2 (preferenza): accettata, font bianco.
- Domanda 3 (preferenza): accettata, frame non specchiati; specchiare `dana_all.png` nel tool solo se a schermo Dana guarda dal lato sbagliato a riposo.
- Domanda 4 (scelta di progetto): accettata, audio placeholder silenzioso; l'audio originale sarà un compito a parte.
- Domanda 5 (preferenza): accettata, celle non identificabili trasparenti ed elencate nel rapporto.

## Esito

Data: 2026-10-19 (agente sviluppatore).

1. `ls res res/backgrounds res/audio | wc -l` → `64`; `find res -type f | wc -l` → `57`. Passata.
2. `check_res.py` → `OK: 20 fogli, 27 sfondi, 10 wav`, uscita `0`. Passata.
3. `verify_decode.py` → `level1 pixel diversi: 0`, `title pixel diversi: 0`. Passata.
4. `ram_room_*.bin` → `51`; `ram_title700.bin` presente. Passata.
5. `timeout 5 ./build/solomons_key` → nessuna `Assertion`, codice `124`. Passata.
6. `screenshot_port.py` → `(1000, 900)`: livello 0 con sfondo arcade (mattoni, stanza 19), blocchi, porta, chiave, Dana, nemici, cornice e slot in basso. Passata.
7. md5 di `src` → `cc06ea6b62ee5080474e64222e5029da  -`; nessun file più nuovo del brief. Passata.
8. Solo `1831 src/sprites.cpp` sopra le 800 righe. Passata.
9. Rigenerazione da zero → `OK: 20 fogli, 27 sfondi, 10 wav`. Passata.

Deviazioni: vedi il rapporto dello sviluppatore (codici BG +0x100, chiave/campanella
scambiate nel brief, logo ritagliato a y=40 sul layer, barra alta alle righe 2..9,
font in grigi, screenshot in Xephyr, sfondi 7 e 8 uguali).

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>

## Chiusura ciclo (chat principale, 2026-10-19)

Esito: PASSATO. Verificatore: 10 verifiche su 10 passate, rigenerazione da zero inclusa.
Token: analista Fable circa 194k (più una prima sessione interrotta da errore server 529), sviluppatore Opus 132k, verificatore Sonnet 17k.
Non committato: il progetto non è un repository git.
