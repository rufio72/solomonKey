# Compito 17: fogli di chars, tiles e sprite a colori (palette della stanza 1)

Data: 2026-10-22. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

`roms/decoded/decode.py` produce fogli in grigi (`us_chars.png`, `us_tiles.png`,
`us_sprites.png`, palette fittizia `g=i*17`). Va aggiunto uno script a fianco che
produce gli stessi fogli a colori veri, con la palette di RAM della stanza 1
(`ram_room_1.bin`), e una verifica che li confronta con lo screenshot
`snap_room_1.png`. L'uscita in grigi esistente non cambia di un byte.

## Vincoli assoluti

- Niente modifiche fuori da `roms/decoded/` (più la riga di stato in `STATO.md`
  e la nota in `NOTICE.md` indicate in Consegna). `decode.py` NON si modifica.
- Non installare nulla: servono solo Python 3 e Pillow 10.2.0 (già presenti,
  da `python3 -c "import PIL"`). Nessun MAME da eseguire.
- Le ROM (`roms/solomon.zip`) restano fuori dal repo e non si copiano; lo script
  le legge da `roms/solomon.zip` con `zipfile`.
- Commenti, messaggi e documenti in italiano.
- Nessun file di codice sopra le 800 righe (i file nuovi ne avranno meno di 200).
- Non usare `dump_stanza_1_2000.bin` (vedi trappola sotto).

## Cosa esiste già (fatti verificati)

| file | righe | note |
|---|---|---|
| `roms/decoded/decode.py` | 62 | `rd`, `bit`, `decode(data,w,h,planes,xoff,yoff,charsize,count)`, `sheet(...)`, `tile8`, `spr16`; il codice a livello di modulo legge `sys.argv` (non importabile). Da `grafo outline` |
| `roms/decoded/us_chars.png`, `us_tiles.png`, `us_sprites.png`, `jp_chars.png` | | fogli grigi da non toccare |
| `roms/decoded/mame/rooms/ram_room_1.bin` | 16384 B | RAM c000-ffff della stanza 1 al frame 2000; `cmp` lo dà identico a `decomp/out/dump_stanza_0_2000.bin` |
| `roms/decoded/mame/rooms/snap_room_1.png` | 256x224 RGB | oracolo dei colori della stanza 1 |

Trappola di nomenclatura (misurata): `snap_room_N.png`, `ram_room_N.bin` sono
1-based; `decomp/out/dump_stanza_K_2000.bin` è 0-based. La stanza 1 è
`dump_stanza_0_2000.bin` = `ram_room_1.bin`. `dump_stanza_1_2000.bin` è la
stanza 2: la sua palette non corrisponde allo screenshot (verificato: il grigio
del muro (68,85,102) non c'è in quel e500). Usare `ram_room_1.bin`.

Layout dei chip (da `compiti/01-asset-da-rom.md` righe 97-135, rimisurato su
`ram_room_1.bin` contro `snap_room_1.png`):

- **Ordine dei piani**: `decode.py` dà il nibble con i piani invertiti. Il pen
  corretto è il nibble con i 4 bit rovesciati: `pen = int(f'{v:04b}'[::-1], 2)`.
  Vale per chars, tiles e sprite.
- **Palette**: 256 voci da 2 byte a offset `0x2400` del dump (c000+0x2400 =
  e400), little endian, parola `xxxxBBBBGGGGRRRR`: `R=(w&15)*17`,
  `G=((w>>4)&15)*17`, `B=((w>>8)&15)*17`. Cioè byte0 = `GGGGRRRR`, byte1 =
  `0000BBBB`. Misurato: voce 9 del banco BG = byte `54 06` = (68,85,102), il
  grigio-blu del muro.
- **Voci 0-127 (e400-e4ff, offset 0x2400)**: chars (FG) e sprite, 8 banchi da
  16 voci, voce = `banco*16 + pen`. **Voci 128-255 (e500-e5ff, offset
  0x2500)**: tiles (BG), voce = `128 + banco*16 + pen`. (Il testo di DATI.md
  riga F000 dice solo "e500 = palette BG, voci 128-255": coerente.)
- **Tiles (ROM `10.3p`+`9.3m`) = piano BG**: colorram d800 (offset 0x1800),
  videoram dc00 (0x1c00), 32x32, indice `riga*32+colonna`; opaco.
- **Chars (ROM `12.3t`+`11.3r`) = piano FG**: colorram d000 (0x1000),
  videoram d400 (0x1400); pen 0 trasparente. Nella stanza 1 contiene blocchi,
  porta, chiave, testi.
- **Attributo** (uguale per i due piani): `code = videoram + 256*(attr&7)`,
  `banco = (attr>>4)&7`, bit 7 = ribaltamento delle righe del tile
  (`t[7-y][x]` invece di `t[y][x]`, usato dal prototipo e coincidente con lo
  screenshot). Il bit 3 non è osservato: ignorarlo.
- **Sprite** (ROM `2.5lm`,`3.6lm`,`4.7lm`,`5.8lm`, layout di `spr16`): spriteram
  a e000 (offset 0x2000), 32 voci da 4 byte `b0 b1 b2 b3`:
  `code = b0 + 16*(b1&0x10)`, `banco = (b1&0x0e)>>1`, flip X `b1&0x40`, flip Y
  `b1&0x80`, `x=b3`, `y=241-b2`; pen 0 trasparente; palette voci 0-127
  (banco*16+pen); y schermo = y - 16.
- **Blocco marrone** (DATI.md riga 1557, "tile 10 colore 20"): in
  `ram_room_1.bin` sono i char 0x10-0x13 con attributo 0x20 (banco 2, quindi
  voci 32-47 di e400), non tile BG. Misurato: la cella riga 10, colonna 14 ha
  `attr=0x20`, `code=0x11`; il suo pen 8 (voce 40 = byte `8c 04`) è (204,136,68),
  il marrone dello screenshot.

Misure del prototipo (script scritto e scartato nello scratchpad, stesso
algoritmo che lo sviluppatore deve realizzare; schermo ricostruito = BG tiles
base 128 opaco, poi FG chars base 0 con pen 0 trasparente, poi 5 sprite dalle
voci 24-28 di spriteram, ritaglio `(0,16,256,240)` confrontato con
`snap_room_1.png`):

- solo BG+FG: 53671 pixel uguali su 57344;
- con i 5 sprite: 54326 su 57344 (i restanti differiscono per HUD, Dana, fiamme
  e simili non ricostruiti: non serve azzerarli);
- cella blocco riga 10, colonna 14: 0 pixel diversi su 64;
- sprite voci 24-28 con palette e400: pixel opachi uguali 81/81, 81/81, 153/153,
  176/176, 164/164; con palette e500 0 su tutti (conferma: gli sprite usano
  e400).
- sha256 (prime 20 cifre) dei fogli grigi, che non devono cambiare:
  `jp_chars.png 262fd9b0de9822c77bbe`, `us_chars.png 9eac7378cad646c863ff`,
  `us_sprites.png 5515fdf1813340b80c7b`, `us_tiles.png a1fdd00d982529d17ef4`.
- Ogni foglio grigio: 2048 tile per chars e tiles, 512 sprite; colonne 64, 64,
  32; scala 2 (misurato dal sorgente `decode.py`, lancio `tile8`/`spr16`).

## Struttura da realizzare

### `roms/decoded/colora.py` (nuovo, circa 120 righe)

Autonomo (non importa `decode.py`, che non è importabile). Copia in cima
`rd`-equivalente, `bit` e `decode` identici a quelli di `decode.py` (dieci
righe; il file va dichiarato duplicato nel commento di testa).

Uso: `python3 colora.py <cartella_rom_zip> <cartella_uscita> [stanza]`, es.
`python3 -I colora.py ../solomon.zip . 1` con `stanza` default 1 (legge
`mame/rooms/ram_room_<stanza>.bin` relativo alla cartella dello script).
Funzioni:

- `leggi_palette(ram)`: lista di 256 tuple (R,G,B) dalla parola a
  `0x2400+2*i`.
- `leggi_piani(ram)`: per FG e BG, liste di 1024 coppie `(code, banco)` con le
  regole sopra, e voci sprite da `0x2000`.
- `banco_riferimento(usi, conteggio)`: per ogni codice il banco più frequente
  nelle celle del dump (parità: il più basso), banco 0 se mai usato.
  Per gli sprite: il banco delle voci di spriteram che usano quel codice, 0 se
  mai usato.
- `foglio_rgba(tiles, w, h, colonne, palette, base, banchi, trasparente0,
  percorso, scala=2)`: salva un PNG RGBA; ogni tile è dipinto con
  `palette[base + 16*banchi[n] + pen]` (pen = nibble rovesciato); se
  `trasparente0` il pen 0 ha alfa 0 (chars e sprite), altrimenti alfa 255
  (tiles). Stessi `colonne`, `scala` e dimensioni di `decode.py`.

File prodotti in `roms/decoded/` (per stanza 1; con `stanza` diversa il
suffisso diventa `_stanzaN`):

- `us_chars_stanza1.png` (base 0, banco di riferimento dal piano d000/d400),
  `us_tiles_stanza1.png` (base 128, banco dal piano d800/dc00),
  `us_sprites_stanza1.png` (base 0, banco dalla spriteram): un foglio per set
  con il banco che quel tile ha nella stanza.
- `us_<set>_stanza1_b0.png` ... `_b7.png` (24 file): lo stesso set tutto con
  banco fisso 0..7, per vedere un tile in ogni sotto-tavolozza.

### `roms/decoded/verifica_colori.py` (nuovo, circa 90 righe)

Riduce i fogli a scala 1 (`[::2]` con `Image.NEAREST`) e ricostruisce lo
schermo della stanza 1: BG da `us_tiles_stanza1_bK.png` col banco di ogni
cella (cella con banco diverso dal banco di riferimento: tile dal foglio `_bK`
corrispondente), FG da `us_chars_stanza1_bK.png`, sprite da
`us_sprites_stanza1_bK.png`, tutto letto dal dump. Confronta ritaglio
`(0,16,256,240)` con `snap_room_1.png`. Stampa le righe:

```
cella blocco r10 c14 pixel diversi: 0
BG+FG pixel uguali: 53671 / 57344
BG+FG+sprite pixel uguali: 54326 / 57344
```

Esce con codice 1 se `cella blocco` > 0 o se i pixel uguali con sprite sono
meno di 54326.

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master`.

1. `cd roms/decoded && python3 -I colora.py ../solomon.zip . 1` → nessun
   errore; una riga per PNG (27). Dimensioni attese, dedotte dal sorgente di
   `decode.py` (non misurate): chars e tiles 1024x512 (2048 tile, 64 colonne,
   scala 2), sprite 1024x512 (512 sprite, 32 colonne di 16 px, scala 2). Controllo:
   `python3 -I -c "from PIL import Image;print(Image.open('us_tiles_stanza1.png').size)"`.
2. `ls roms/decoded/*stanza1*.png | wc -l` → `27`.
3. `sha256sum roms/decoded/jp_chars.png roms/decoded/us_chars.png
   roms/decoded/us_sprites.png roms/decoded/us_tiles.png | cut -c1-20` → le
   quattro cifre iniziali elencate sopra (262fd9b0de9822c77bbe,
   9eac7378cad646c863ff, 5515fdf1813340b80c7b, a1fdd00d982529d17ef4). Oppure
   `git status --short roms/decoded/*.png` non elenca `jp_chars`,
   `us_chars`, `us_sprites`, `us_tiles`.
4. `cd roms/decoded && python3 -I verifica_colori.py` → le tre righe sopra, con
   `cella blocco r10 c14 pixel diversi: 0`, `BG+FG pixel uguali: 53671 / 57344`
   (tolleranza: ±0 se l'algoritmo è identico; se differisce, spiegare nel
   rapporto, minimo accettato 53000), `BG+FG+sprite pixel uguali: ≥ 54326 /
   57344`; codice di uscita 0.
5. Colore del blocco marrone dal foglio:
   `python3 -I -c "from PIL import Image;im=Image.open('roms/decoded/us_chars_stanza1_b2.png').convert('RGBA');print((204,136,68) in {im.getpixel((x,y))[:3] for x in range(272,288) for y in range(0,16) if im.getpixel((x,y))[3]==255})"`
   → `True` (tile 0x11 = colonna 17, riga 0, scala 2: x 272..287). Riferimento
   misurato nello screenshot:
   `python3 -I -c "from PIL import Image;print(Image.open('roms/decoded/mame/rooms/snap_room_1.png').convert('RGB').getpixel((14*8+5,10*8-16+1)))"`
   → `(204, 136, 68)`.
6. Palette: `python3 -I -c "d=open('roms/decoded/mame/rooms/ram_room_1.bin','rb').read();print(d[0x2500+18:0x2500+20].hex(), d[0x2400+80:0x2400+82].hex())"`
   → `5406 8c04` (voce 137 = BG banco 0 pen 9, voce 40 = blocco marrone; byte
   misurati: l'offset 0x2500+18 è la voce 137).
7. `find roms/decoded -name '*.py' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
   → uscita vuota.
8. Assenza di ROM e modifiche fuori ambito: `git status --short` →
   solo file in `roms/decoded/` (27 PNG, `colora.py`, `verifica_colori.py`),
   `compiti/17-...`, `STATO.md`, `NOTICE.md` se toccati; nessun `.zip` o
   `.rom` aggiunto: `git status --short | grep -E '\.(zip|rom|bin)$'` →
   vuoto.

## Consegna

- Commit unico, messaggio "Fogli chars/tiles/sprite a colori con la palette
  della stanza 1" più la riga
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>` (modello
  effettivo di chi esegue).
- `NOTICE.md`: nessuna modifica se la frase del punto 1 ("grafica, palette,
  livelli") copre già le immagini derivate in `roms/decoded/`: la copre
  (verificato). `STATO.md`: aggiungere una riga di una frase con il nome dei
  due script, se esiste una sezione sugli strumenti di `roms/decoded/`; altrimenti
  non toccarlo.
- Rapporto finale (max 30 righe): uscita delle 8 verifiche, deviazioni
  (soprattutto sulle dimensioni dei fogli, attese non misurate) e eventuale
  scarto dai numeri 53671 / 54326.

## Domande aperte

1. Sotto-tavolozza (un tile non conosce il suo banco): proposta, un foglio per
   set col banco più frequente nella stanza 1 più 8 fogli per set a banco
   fisso (27 PNG in tutto). Alternativa scartata: leggere gli attributi dalla
   colorram nel solo foglio, perde i tile non usati nella stanza. Marca:
   «preferenza».
2. Duplicare `rd`/`bit`/`decode` in `colora.py` invece di rendere
   `decode.py` importabile (richiederebbe `if __name__ == '__main__'` e
   toccare il file grigio). Marca: «preferenza».
3. Pen 0 trasparente (alfa 0) per chars e sprite, opaco per tiles, invece di un
   colore pieno; i grigi attuali usano blu scuro per il pen 0. Marca:
   «preferenza».
4. Parametro `stanza` (default 1) per generare i fogli di un'altra stanza dalla
   sua `ram_room_N.bin`: costa poche righe, non è richiesto. Le palette BG
   cambiano per stanza, quelle e400 no (non misurato). Marca: «preferenza».
5. I 27 PNG entrano nel repo (circa 20-50 KB ciascuno, stima non misurata);
   alternativa, tenere nel repo solo i 3 fogli mappati e generare i 24 per
   banco a richiesta. Marca: «preferenza».

## Risoluzioni (2026-10-22)

Tutte le domande sono preferenze, accettate con la proposta dell'analista:
1. Un foglio per set col banco più frequente nella stanza, più 8 fogli per
   set a banco fisso (27 PNG in tutto).
2. `decode` duplicato in `colora.py`; `decode.py` resta intatto.
3. Pen 0 trasparente per chars e sprite, opaco per tiles.
4. Parametro `stanza` con default 1.
5. Tutti i 27 PNG nel repo.

## Esito (2026-10-22): PASSATO

Ciclo in locale: analista Sonnet ~79k token, sviluppatore Opus ~31k,
verificatore Sonnet ~16k. Tutte le 8 verifiche passate due volte
(sviluppatore e verificatore); i PNG rigenerati dal verificatore sono
identici a quelli committati (b282b96) e i quattro fogli grigi sono
rimasti intatti.

Prova dei colori: lo schermo della stanza 1 ricostruito dai fogli e dagli
attributi del dump coincide con lo screenshot MAME per 54326 pixel su
57344; il blocco marrone (r10 c14) coincide pixel per pixel, colore
campione (204, 136, 68).

Da sapere: `dump_stanza_N` è 0-based (la stanza 1 vera è
`dump_stanza_0_2000.bin` = `ram_room_1.bin`); `colora.py` accetta come
primo argomento sia `solomon.zip` sia la cartella che lo contiene e ha il
parametro `stanza` (default 1) per colorare con la palette di altre
stanze.
