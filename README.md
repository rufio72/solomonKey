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

## Status (October 2026)

- **Feasibility survey** (`decomp/RAPPORTO.md`): clean code, no self-modifying
  tricks on the main CPU, ~250-300 routines estimated.
- **Phase 1 — mapping, done:** full instruction traces, call graph, level
  format decoded (`decomp/DATI.md`), scheduler reproduced as C functions.
- **Phase 2 — batch translation, done:** the entire observed perimeter is
  translated — **116 routines in `ROUTINE.md`, 104 verified** with
  before/after MAME captures, harness at **112 comparisons, 0 failures**.
  This includes the NMI, the activity scheduler, Dana's state machine, the
  per-object engine and the hardware ports as platform functions.
- **Bonus rooms & ending coverage, done:** guided sessions with targeted RAM
  forcing pushed coverage to **235 of 299 static call destinations (78.6%)**;
  the 64 never-executed remainder all have call sites that never run
  (data misread as code, or code reachable only from dead zones) and are
  censused in `decomp/CENSIMENTO-12.md`.
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
- **After that:** the hardware layer (video, sprites, palettes, input) to make
  the C port playable, then cleanup with the replay as regression.

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

## Requirements

MAME 0.264, `z80dasm`, Python 3 with Pillow, gcc and make. Put the `solomon`
ROM set in `roms/` (never commit it). Every measurement in the reports can be
reproduced with the commands listed at the end of `decomp/RAPPORTO.md`.

## Contributing

Issues, questions and pull requests are welcome — curiosity about Z80 reverse
engineering is the only prerequisite. Good starting points: read
`decomp/AUDIO.md` for the sound driver, or pick a routine in
`decomp/ROUTINE.md` and follow its verification trail.
