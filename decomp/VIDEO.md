# Oracolo video (lotto H1)

Per sei stati campione, MAME scrive nello stesso frame i dump binari di video
RAM, sprite RAM e palette RAM più lo screenshot PNG. Servono da oracolo al
renderizzatore software (lotto H2, `sh rendi_video.sh`). Rigenerare, da `decomp/`: `sh video.sh
[stato...]` (senza argomenti tutti e sei; `VIDEO_OUT` cambia la cartella,
predefinita `out/video`). Confronto fra due corse: `python3 -I
verifica_video.py [DIR]` (hash dei pixel dei PNG, sha1 dei `.bin`).

## File (`out/video/<stato>_*`)

| File | Indirizzi | Byte |
|---|---|---|
| `<S>_bg_colorram.bin` | d800-dbff | 1024 |
| `<S>_bg_videoram.bin` | dc00-dfff | 1024 |
| `<S>_fg_colorram.bin` | d000-d3ff | 1024 |
| `<S>_fg_videoram.bin` | d400-d7ff | 1024 |
| `<S>_spriteram.bin` | e000-e07f | 128 |
| `<S>_palette.bin` | e400-e5ff | 512 |
| `<S>.png` (MAME) | schermo 256x224 RGB | - |

## Stati

| Stato | Frame | Ingressi | Stanza forzata | Poke (frame:addr:val) | Posta |
|---|---|---|---|---|---|
| titolo | 1000 | nessuno | - | - | - |
| gioco | 2400 | traccia | - | - | - |
| stanza10 | 2100 | traccia | 10 | - | - |
| bonus | 2400 | traccia | 4 | 2250:c19d:0, 2250:c19e:0, 2250:c24a:0 | 2300:1c |
| finale | 2400 | traccia | - | 2250:c19d:0, c19e:0, c24b:32, c24c:32, c24d:32 | 2300:1c |
| finale2 | 4500 | traccia | - | come finale | 2300:1c |
| prove | 1000 | nessuno | - | `POKE_P` in `video.sh` (flip BG, bit FG, 4 sprite) | - |

`prove` è sintetico: parte da `titolo` e si genera solo nominandolo.
Inneschi e ingressi «traccia» sono quelli di `snap.lua`/`cattura.sh`.
«bonus» e «finale» a frame 2400 mostrano entrambi la schermata «Your Rest
Bonus»; il finale vero (stanza con il pentagramma) è finale2.

## Formato provato (H2)

Il renderizzatore `harness/video_rendi.c` (`rendi_frame`) rende i sette stati
con 0 pixel diversi dal PNG di MAME (`sh rendi_video.sh`). Grafica ROM in C
generato da `estrai_grafica.py` (`harness/video_dati_*.c`).

- Schermo: tilemap 32x32 celle 8x8 = 256x256, cella `i = riga*32 + colonna`,
  nessuno scorrimento. Il PNG è l'area visibile, righe 16-239: riga `y` del
  framebuffer = riga di schermo `y+16`.
- Palette: voce `i` (0-255), `w = pal[2i] | pal[2i+1]<<8`; `R = (w&15)*17`,
  `G = ((w>>4)&15)*17`, `B = ((w>>8)&15)*17`.
- Tile 8x8: grafica FG = `12.3t` + `11.3r` (2048 tile da 32 byte); BG =
  `10.3p` + `9.3m`. Pen di (x,y) del tile c: byte `g[c*32 + y*4 + x/2]`,
  nibble alto se x pari, basso se dispari.
- FG: `a = fg_colorram[i]`, codice `fg_videoram[i] + 256*(a&7)`, colore
  `16*((a>>4)&7) + pen` (voci 0-127), pen 0 trasparente; i bit 0x80 e 0x08
  sono ignorati.
- BG: codice `bg_videoram[i] + 256*(a&7)`, colore `128 + 16*((a>>4)&7) + pen`,
  opaco; `a&0x80` flip orizzontale, `a&0x08` flip verticale.
- Sprite 16x16, 32 voci da 4 byte `b0 b1 b2 b3`: codice `b0 + ((b1&0x10)<<4)`,
  colore `16*((b1>>1)&7) + pen` (voci 0-127), `b1&0x40` flip orizzontale,
  `b1&0x80` flip verticale, `sx = b3`, `sy = 241 - b2` (righe di schermo,
  prima del -16), pen 0 trasparente. Pen = `p(2.5lm)<<3 | p(3.6lm)<<2 |
  p(4.7lm)<<1 | p(5.8lm)`, p(file) = bit `7-(x&7)` del byte
  `s*32 + ((y>=8)?16:0) + ((x>=8)?8:0) + (y&7)`.
- Ordine: BG, FG, sprite dalla voce 31 alla 0 (la 0 resta sopra). Nessun bit
  di priorità sprite/FG.
- Bordi: fuori da x 0-255 o righe 16-239 si scarta, senza avvolgimento.
- Il PNG e i dump RAM dello stesso frame corrispondono (confermato).
- Non modellato: ribaltamento dello schermo (latch di sola scrittura, non in
  RAM; da aggiungere come parametro in H4/H5 se serve). Nessun dump di
  `c000-ffff`.
- `png_ppm.py` converte il PNG in PPM; `rendi_oracolo` confronta in C.
