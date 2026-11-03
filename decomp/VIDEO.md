# Oracolo video (lotto H1)

Per sei stati campione, MAME scrive nello stesso frame i dump binari di video
RAM, sprite RAM e palette RAM più lo screenshot PNG. Servono da oracolo al
renderizzatore software (lotto H2). Rigenerare, da `decomp/`: `sh video.sh
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

Inneschi e ingressi «traccia» sono quelli di `snap.lua`/`cattura.sh`.
«bonus» e «finale» a frame 2400 mostrano entrambi la schermata «Your Rest
Bonus»; il finale vero (stanza con il pentagramma) è finale2.

## Da decidere in H2

- Spriteram: 32 voci da 4 byte a partire da e000; il significato dei byte non
  è deciso in H1. Il titolo ha spriteram tutta a zero.
- Palette: 256 voci da 2 byte da e400; ordine dei byte/nibble non deciso (H2
  lo ricava confrontando i pixel del PNG con i colori). Nel dump del titolo i
  primi byte sono `00 00 00 02 4f 07 27 00 c4 05`.
- Le coppie colorram+videoram sono due piani distinti (FG e BG); l'ordine di
  priorità va ricavato in H2.
- Il ribaltamento dello schermo non si salva: è un latch di sola scrittura e
  `e604` in lettura restituisce DSW1.
- Avvertenza: il PNG è preso nella stessa callback `frame_done` dei dump, ma
  che corrisponda al contenuto RAM letto subito dopo è un'ipotesi da
  confermare in H2. Se il fondale risulta sfasato di un frame, il lotto H1
  va rifatto: segnalarlo.
- Non c'è un dump dell'intera RAM `c000-ffff`: si aggiunge solo se H2 lo chiede.
