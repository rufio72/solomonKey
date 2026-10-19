# solomonKey

Decompilazione in C, a partire dalle ROM arcade, di *Solomon's Key* (Tecmo, 1986).
Il lavoro parte da zero: traccia di esecuzione con MAME, disassembly, separazione
codice/dati, traduzione routine per routine verificata byte per byte contro la RAM
dell'originale.

> **Nota di proprietà:** le immagini, i dump e i dati in `roms/decoded/` e le tabelle
> in `decomp/harness/` derivano dalle ROM arcade Tecmo e **non sono di proprietà
> dell'autore di questo repository**. Vedi [NOTICE.md](NOTICE.md). Le ROM non sono
> incluse e non vanno aggiunte.

## Contenuto

- `decomp/`: script di traccia, copertura e grafo delle chiamate, banco di prova in C,
  routine tradotte, `RAPPORTO.md` del sondaggio di fattibilità, `DATI.md` sul formato
  dei livelli nella ROM.
- `roms/decoded/`: chip grafici decodificati, palette, dump di RAM e screenshot da MAME.
- `compiti/`: i brief di ogni ciclo di lavoro, con decisioni ed esito.

## Strumenti richiesti

MAME 0.264, `z80dasm`, Python 3 con Pillow, gcc. Il set ROM `solomon` va messo in
`roms/` a parte.
