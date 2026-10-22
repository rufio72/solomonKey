# Nota su contenuti e proprietà

1. **Dati derivati dal gioco arcade** (`roms/decoded/`, le tabelle in
   `decomp/harness/`): ricavati dal set ROM arcade di *Solomon's Key* (Tecmo,
   1986) con gli script di questo repository. **Non sono di proprietà
   dell'autore del repository**: grafica, palette, livelli e codice originale
   sono copyright Tecmo (oggi Koei Tecmo). Sono inclusi a scopo di studio e
   conservazione. Le ROM vere (`roms/*.zip`, `roms/ext/`) non sono incluse.
   Le tabelle e i flussi in `decomp/harness/audio_dati*.c` sono dati musicali
   derivati dal programma Tecmo, estratti con `decomp/estrai_dati.py`.

2. **Analisi e decompilazione** (`decomp/`, `compiti/`): script, rapporti e
   traduzioni in C prodotti qui. Le traduzioni in C sono opere derivate dal
   programma Tecmo e valgono per uso personale e di studio.

3. **emu2149** (`decomp/harness/emu2149/`: `emu2149.c`, `emu2149.h`,
   `LICENSE`): emulatore del PSG YM2149/AY-3-8910 di Mitsutaka Okazaki,
   licenza MIT («Copyright (c) 2014 Mitsutaka Okazaki»), da
   <https://github.com/digital-sound-antiques/emu2149>, commit
   `441f3dc295a9b82db16921461f0678b483db8304`. Incluso **non modificato**;
   lo usa il renderizzatore `decomp/harness/audio_wav.c` (brief 22).
