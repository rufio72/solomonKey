# Hidden objects ("special") in every room

One image per room, `stanza_NN.png`, where NN is the 0-based room index
(c243, 00-51, hidden/bonus rooms and the final room included). Each image
shows the room as in the game (background and blocks, first visit, no
sprites or HUD) with the hidden objects drawn in their cell using the
game's own graphics, the same tiles the game shows when a block created on
that cell is broken.

- green outline: hidden object (cell value 0x40-0x76, class = value & 0x3F);
- orange outline: cell E of the room record, the key (class 05/06/07,
  depending on c24a);
- blue outline (only with `--nei-blocchi`, in `nei_blocchi/`): object already
  inside a block.

The legend lists class (hex), name, row/column and points; the last line
gives the four time-step bytes of the record. Regenerate from `decomp/`
with `python3 mappe_special.py` (needs the MAME dumps
`out/dump_stanza_N_2300.bin` for the palette). Class table: `decomp/DATI.md`.
