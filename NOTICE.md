# Nota su contenuti e proprietà

Questo repository contiene tre cose distinte.

1. **Il port OpenSolomonsKey** (`src/`, `editor/`, `level_*.osk`): codice C++
   scritto da zero dall'autore del progetto originale OpenSolomonsKey e
   rilasciato nel pubblico dominio (vedi `LICENSE`). Qui è usato come base
   eseguibile, con una correzione di compilazione.

2. **Immagini, suoni e dati derivati dal gioco arcade** (`res/`,
   `roms/decoded/`, `tools/out/`, le tabelle in `decomp/harness/`):
   sono ricavati dal set ROM arcade di *Solomon's Key* (Tecmo, 1986) tramite
   gli script in `tools/` e `decomp/`. **Non sono di proprietà dell'autore di
   questo repository**: grafica, palette, livelli e codice originale sono
   copyright Tecmo (oggi Koei Tecmo). Sono inclusi a scopo di studio e
   conservazione. Le ROM vere (`roms/*.zip`, `roms/ext/`) non sono incluse e
   non vanno aggiunte.

3. **Il lavoro di analisi e decompilazione** (`decomp/`, `tools/`, `compiti/`):
   script, rapporti e traduzioni in C di singole routine, prodotti in questo
   repository. Le traduzioni in C sono opere derivate dal programma Tecmo e
   valgono solo per uso personale e di studio.
