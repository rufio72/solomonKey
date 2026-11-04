# solomonKey — reverse engineering Solomon's Key (Tecmo, 1986)

A from-scratch **decompilation** of the arcade game **Solomon's Key** — the
classic Tecmo puzzle-platformer where the wizard Dana creates and destroys
blocks to reach the key in every room — reconstructed in **readable C** from
nothing but the original ROMs, with **MAME as the oracle**.

No original source code exists. This project recovers it the hard way:
execution-driven **reverse engineering** of the two **Zilog Z80** CPUs on the
arcade board (a 48 KB game program + 4 KB of data, and a 16 KB sound driver
for three **YM2149/AY-3-8910** PSG chips), routine by routine, with every
translation **verified bit-for-bit** against the real machine state captured
in the emulator. Rules, enemy AI, timers, glitches — original bugs included.

> **Ownership note.** The images, dumps and data tables under `roms/decoded/`
> and `decomp/harness/` are derived from the Tecmo arcade ROMs and are **not
> the property of this repository's author**. See [NOTICE.md](NOTICE.md).
> The ROMs are not included and must never be committed.

**Keywords:** reverse engineering · decompilation · disassembly · Z80 ·
arcade · Tecmo · Solomon's Key · 1986 · MAME · emulation · bit-accurate ·
cycle-exact verification · game preservation · C port · retrogaming ·
YM2149 · AY-3-8910 · static and dynamic analysis.

## Method: decompilation with an oracle

Every line of C in this repository is backed by a measurement, not a guess:

1. **Trace.** MAME runs the original and logs every executed instruction.
   Any byte executed at least once is code; the rest is data.
2. **Map.** From the disassembly and the traces we build the call graph and
   chart the RAM, hardware ports and data tables each routine touches.
3. **Verified translation.** Each routine is translated to C operating on a
   copy of the original RAM. A test harness loads the RAM state captured in
   MAME one instant *before* the routine, runs the C version, and compares
   byte-for-byte with the state captured one instant *after*. If it matches,
   the translation is correct.
4. **Replay.** Once everything is translated, a game session recorded in MAME
   is replayed by the C program with identical RAM, frame by frame.
5. **Cleanup.** Only then is the C made human-friendly — names, structs,
   constants — with the replay as the regression test.

The sound driver got the same treatment with its own oracle: a per-frame log
of every PSG register change, captured for each of the 65 sound/music command
codes. Its end-to-end test — WAV files rendered from the C driver through a
YM2149 emulator and compared against the original's — has been passed.

## Status (November 2026)

- **Feasibility survey** (`decomp/RAPPORTO.md`): clean code, no self-modifying
  tricks on the main CPU, ~250-300 routines estimated.
- **Phase 1 — mapping, done:** full instruction traces, call graph, level
  format decoded (`decomp/DATI.md`), scheduler reproduced as C functions.
- **Phase 2 — batch translation, done:** the entire observed perimeter is
  translated — **116 routines in `ROUTINE.md`, 104 verified** with
  before/after MAME captures, harness at **112 comparisons, 0 failures**.
  This includes the NMI, the activity scheduler, Dana's state machine, the
  per-object engine and the hardware ports as platform functions.
- **Bonus rooms & ending — done, translated:** guided sessions with targeted
  RAM forcing censused the region, and every reachable routine is now
  translated and in the harness, including the key-pickup dispatcher and
  its 16 handlers (table `0D18`). The never-executed remainder — 59
  destinations — has a written verdict per entry in
  `decomp/ZONE-MAI-ESEGUITE.md`: 53 are data misread as code, none are
  dead code, 6 are reachable under special conditions (5 with a proven
  wake-up recipe, two of them needing specific DIP-switch settings).
  **With this, the main Z80's reachable code is fully translated:**
  the harness stands at **1,278 comparisons, 307 entries, 0 failures**.
- **Audio Z80 — done: fully decompiled and verified end to end.** The whole
  sound driver is now C (`decomp/harness/audio_*.c`): command dispatch, the
  sequence interpreter (9 voices × 2 layers, glissando, envelopes) and the
  register writer for the three PSGs — down to reproducing a harmless
  original-ROM bug that writes to two unmapped ports. The music data region
  was extracted into generated, human-readable C source (`SPARTITI.md`
  catalogs all 63 sound/music commands) and re-serializes byte-for-byte back
  into the ROM. Final test, passed: WAV files rendered from the C driver's
  register stream through an off-the-shelf YM2149 emulator (emu2149) are
  **byte-for-byte identical** to the same renderer fed by the MAME-captured
  streams in 40 of 44 cases; the 4 remaining differences are sub-frame timing
  artifacts peaking at −33 to −49 dBFS, measured, documented and confirmed
  inaudible by listening tests.
- **Full-game replay — done, end to end.** A recorded 90-second MAME run is
  replayed by the C translation frame by frame, comparing RAM fingerprints
  against the original at every frame: **64,686 dispatches and 3,090 NMIs
  over frames 2410–5499, zero divergences**, exit 0 (`decomp/replay.sh` +
  `decomp/harness/replay.c`, see `decomp/REPLAY.md`). Getting there exposed
  and fixed a latent bug — nine collision handlers of table 08AE had been
  silently missing since batch 7 — and refined the NMI interleaving model.
  The replay is now the regression test for everything.
- **Enemy engine — fully translated.** Behind activity
  body 0A lives the per-enemy engine: **8,833 bytes and 220 routines**,
  mapped with the real `rst 38` dispatch tables
  (`decomp/CENSIMENTO-NEMICI.md`) and split into batches N1–N12.
  **All batches N1 through N12 are done and the engine is complete** —
  the replay-exercised core, every enemy type, the spawn handlers
  (including the fairy routine `4FC5`, first observed only in live play)
  and the shared helpers — all **220 routines translated**, every entry
  backed by before/after MAME captures with targeted RAM forcing, and
  the stub file is gone. The harness stands at
  **1,110 comparisons, 264 entries, 0 failures**. Along
  the way the bench caught and fixed a latent carry-flag bug in an
  N1-verified routine, and grew two reusable techniques: wrappers that
  verify routines reading the Z80 refresh register R (whose value no
  capture can record) by searching for a consistent value, and a
  dispatch-table forcing trick (`BC = 1<<j`) that exercises every entry
  of a `rst 38` table from a single call site.
- **Hardware layer — underway (SDL2).** The plan lives in
  `compiti/PIANO-HARDWARE.md` (batches H1-H7). Done so far:
  **H1, the video oracle** — per-frame dumps of both tilemaps, sprite RAM
  and palette RAM taken in the same frame as a MAME screenshot, for six
  sample states (`decomp/out/video/`, format in `decomp/VIDEO.md`); and
  **H2, the pure software renderer** — `rendi_frame` turns the video RAM
  into an RGB framebuffer that is **pixel-for-pixel identical to MAME on
  all seven sample states**, with the graphics ROMs' pixel data shipped
  as generated C source (round-trip verified, see `NOTICE.md`).
- **Free-running game — done: boots and plays the whole recorded match.**
  H3 added the SDL2 window (`mostra_oracolo`, `mostra_replay`). H4a-H4c
  then cut the game loose from the oracle: the C game **cold-boots from
  the reset vector**, runs its RAM tests, plays the whole attract mode
  and title (bodies 1F, 06, 07, 08, 0C, 0B — ~1,450 instructions
  translated across four batches), consumes the recorded match's inputs
  at the right frames, and plays it through to the end. The project's
  single regression line now reads:
  `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze,
  30 a metà attività (attese), 13 frame resi con hash coincidenti`
  (`decomp/giro_completo.sh`).
- **Playable — the game runs on the keyboard.** H5 added `gioca`
  (`decomp/harness/gioca`): cold boot, SDL2 window, keys mapped on the
  real port bits from the MAME driver (arrows/WASD, Z jump, X magic,
  5 coin, 1 start; player 2 on IJKL+N/M), readable DIP-switch options,
  and record/replay of input files — any played match can become a
  future oracle. In `--oracolo` mode it replays the recorded match and
  the final frame's framebuffer hash is **identical to MAME's**; free
  mode is deterministic and plays the same match through without a
  single stop (timing differs slightly from MAME by design: no Z80
  cycle model, same criterion the audio chain ratified).
- **Sound — `gioca` plays the translated audio driver.** H6 wires the
  game's sound-latch writes to the translated Z80 audio driver, runs it
  two IRQs per frame, and renders the three YM2149 with emu2149 to SDL audio
  (`--muto` to disable). The latch sequence is identical to MAME's; the
  register-change stream still differs in 312 of 5486 frames (timing of the
  sound routine inside a frame), to be judged by ear.
- **Packaged — H7 closes the hardware plan.** `make -C decomp/harness gioco`
  prepares everything (ROM extraction, `out/main64.bin`, build), `gioca` runs
  from any directory, and it gained joystick support with a live diagnostic
  (`--joystick`, hotplug, button remapping), `--volume`, `--pieno`
  (fullscreen) and `--aiuto`. See [How to play](#how-to-play).
- **Next up:** the manual joystick test on a real arcade stick, then
  classifying the remaining
  mid-activity NMI divergences, and finally the readability rewrite,
  module by module, with the harness and the full run as the safety
  net.

## Repository layout

- `decomp/` — tracing, coverage and call-graph tooling (`trace.lua`,
  `coverage.py`, `callgraph.py`), before/after capture (`snap.lua`), data
  extractors, the audio toolchain (`audio.lua`, `audio.sh`), the test harness
  with the translated routines (`harness/`), and the reports (`RAPPORTO.md`,
  `DATI.md`, `AUDIO.md`, `ROUTINE.md`).
- `roms/decoded/` — decoded graphics chips (tiles, sprites, palettes), RAM
  dumps and MAME screenshots used as oracles, plus `colora.py` to render the
  sheets with any room's real palette.
- `compiti/` — one brief per work cycle, with verified facts, mandatory
  checks with expected outputs, decisions taken and outcome. The project's
  working documents are in Italian; the code and this README are the
  interface for everyone else.

## How to play

**Requirements:** `libsdl2-dev` (SDL2 ≥ 2.0, tested with 2.30.0), gcc, make
and `unzip`. Put `solomon.zip` in `roms/` (never commit it). At runtime the
game uses only four ROM files: `6.3f`, `7.3h`, `8.3jk` (main CPU, assembled
into `decomp/out/main64.bin`) and `1.3jk` (sound CPU). The other eight ROMs
(graphics, and `solomonj.zip`) are no longer needed: graphics and sound data
ship as generated C in this repository. MAME is needed only to rebuild the
oracles, not to play.

```
make -C decomp/harness gioco      # extracts the zip, builds main64.bin and gioca
decomp/harness/gioca              # play (works from any directory)
decomp/harness/gioca -s 4         # window at 4x scale
decomp/harness/gioca --pieno      # fullscreen, integer scaling, 8:7 kept
decomp/harness/gioca --scanlines  # CRT-style dark horizontal lines (display only, default off)
decomp/harness/gioca --scanlines 1  # thinner lines (1, 2 or 3 quarters of a game pixel row)
decomp/harness/gioca --vite 5     # DIP switch: 5 lives
decomp/harness/gioca --volume 50  # half volume
decomp/harness/gioca --muto       # no sound
decomp/harness/gioca --aiuto      # every option (in Italian)
```

`--scanlines [S]` draws a dark line (about 25% darker) at the border between
consecutive rows of game pixels. S is the thickness in quarters of a game
pixel row, from 1 to 3 (default 2): at scale N the line is
max(1, round(N*S/4)) screen pixels thick, at most N-1, so the lines follow
the game grid with `-s N` and `--pieno`. At
scale 1 there is no room between the pixels and the option has no effect.
It is display only and never changes the video hash, replays or the oracle
mode.

**Keys** (from `gioca --mappa`): arrows or WASD move; Z, Space or Left Ctrl
is BUTTON1; X or Left Alt is BUTTON2 (which of the two is jump and which is
magic will be noted after the manual test); 5 or C inserts a coin; 1 or
Enter is START1 (2 = START2). Player 2: IJKL, N (BUTTON1), M (BUTTON2).
In game: P pauses, F2 resets, ESC quits.

**Joystick:** any SDL joystick works and adds to the keyboard (it does not
replace it); sticks plugged in while the game runs are picked up (hotplug).
Directions come from axes 0/1 and from hat 0 — arcade encoders often report
the lever as a hat. To find your button numbers run the diagnostic, which
lists the devices and prints every event with the game action it maps to:

```
$ decomp/harness/gioca --joystick
JOYSTICK 0 "Bigben Interactive DAIJA Arcade Stick" assi=4 pulsanti=13 cappelli=1 guid=...
CAPPELLO 0 0 destra (destra)
PULSANTE 0 0 giu (BUTTON1)
PULSANTE 0 0 su (BUTTON1)
```

(ESC in its small window or Ctrl-C to quit.) Then remap with
`--joy-mappa B1,B2,START,COIN`, e.g. `--joy-mappa 2,3,9,8` (button indices,
`-1` = none; default `0,1,7,6`). `--senza-joystick` disables joysticks.

**DIP switches and recordings:** `--vite`, `--cabinet`, `--demo`, `--bonus`,
`--difficolta`, `--timer`, `--extra` (or raw `--dsw1`/`--dsw2`;
`--stampa-dsw` prints the result). `--registra FILE` records your inputs,
`--riproduci FILE` replays them — any match can become a future oracle.

**Known limit:** in free mode the game stops with a `FERMO ...` line (exit
code 3) if it reaches Z80 code that is not translated yet.

## Requirements

MAME 0.264, `z80dasm`, Python 3 with Pillow, gcc and make. Put the `solomon`
ROM set in `roms/` (never commit it). Every measurement in the reports can be
reproduced with the commands listed at the end of `decomp/RAPPORTO.md`.

## Contributing

Issues, questions and pull requests are welcome — curiosity about Z80 reverse
engineering is the only prerequisite. Good starting points: read
`decomp/AUDIO.md` for the sound driver, or pick a routine in
`decomp/ROUTINE.md` and follow its verification trail.
