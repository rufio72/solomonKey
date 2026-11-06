# Notice on content and ownership

This is an unofficial, non-commercial project. It is **not affiliated with,
endorsed by or connected to Tecmo or Koei Tecmo**. If you are a rights holder
and want something removed, please open an issue on
<https://github.com/rufio72/solomonKey/issues>: takedown requests are welcome
and will be handled promptly.

1. **Data derived from the original arcade game** (`tecmo/`,
   `roms/decoded/`): extracted from the arcade ROM set of *Solomon's Key*
   (Tecmo, 1986) with the scripts in this repository. In `tecmo/`: the
   main-CPU data tables (`rom_dati_*.c`, `rom_dati.h`, `rom_voci.c`), the
   sound-CPU driver (`audio_driver.c`) and music data (`audio_dati_*.c`,
   `audio_dati_*.h`), and the bytes of the graphics ROMs (12
   `video_dati_*.c`). In `roms/decoded/`: decoded graphics, palettes, RAM
   dumps and screenshots. Graphics, palettes, levels, text, music, sound
   driver and original program are copyright Tecmo (now Koei Tecmo); they
   are **not the property of this repository's author** and are included
   for study and preservation purposes only. The extractors are
   `decomp/estrai_rom.py`, `decomp/estrai_dati.py` and
   `decomp/estrai_grafica.py`. The original ROMs (`roms/*.zip`,
   `roms/ext/`) are **not** included.

2. **Analysis and decompilation** (`decomp/`, `compiti/`): scripts, reports
   and C translations written in this project. The author's original code
   remains the author's work. The C translations of the game routines are
   works derived from the Tecmo program and are offered for personal and
   study use only. No right to the original game is claimed.

3. **emu2149** (`decomp/harness/emu2149/`: `emu2149.c`, `emu2149.h`,
   `LICENSE`): YM2149/AY-3-8910 PSG emulator by Mitsutaka Okazaki, MIT
   license («Copyright (c) 2014 Mitsutaka Okazaki»), from
   <https://github.com/digital-sound-antiques/emu2149>, commit
   `441f3dc295a9b82db16921461f0678b483db8304`. Included **unmodified**; it
   is used by the WAV renderer `decomp/harness/audio_wav.c` and by the
   `gioca` game binary.
