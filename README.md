# solomonKey

Ricostruzione in C, da zero e a partire dalle sole ROM arcade, di **Solomon's Key**
(Tecmo, 1986): il puzzle-platform in cui il mago Dana crea e distrugge blocchi per
raggiungere la chiave e la porta di ogni stanza.

> **Nota di proprietà.** Le immagini, i dump e i dati in `roms/decoded/` e le tabelle
> in `decomp/harness/` derivano dalle ROM arcade Tecmo e **non sono di proprietà
> dell'autore di questo repository**. Vedi [NOTICE.md](NOTICE.md). Le ROM non sono
> incluse e non vanno aggiunte.

## Cosa stiamo facendo

Il gioco originale gira su una scheda con due Z80: uno per il gioco (48 KB di
programma più 4 KB di dati) e uno per l'audio (16 KB). Non esiste un sorgente
pubblico. L'obiettivo è ottenere un sorgente C leggibile e modificabile che
riproduca esattamente la logica dell'arcade: regole, comportamento dei nemici,
tempi, bug compresi.

Il metodo è una decompilazione guidata dall'esecuzione, con MAME come oracolo:

1. **Traccia.** MAME esegue l'originale e registra ogni istruzione eseguita.
   Ogni byte eseguito almeno una volta è codice, il resto è dati.
2. **Mappa.** Dal disassembly e dalla traccia si ricavano il grafo delle chiamate,
   le aree di RAM e di hardware toccate da ogni routine, le tabelle di dati.
3. **Traduzione verificata.** Ogni routine viene tradotta in C su una copia della
   RAM originale. Un banco di prova carica lo stato della RAM catturato in MAME
   un istante prima della routine, esegue la versione C e confronta byte per byte
   con lo stato catturato un istante dopo. Se combacia, la traduzione è giusta.
4. **Replay.** Quando tutte le routine sono tradotte, una partita registrata in
   MAME viene rigiocata dal programma C con la RAM identica frame per frame.
5. **Pulizia.** Solo a quel punto il C viene reso leggibile: nomi, strutture,
   costanti, con il replay come test di regressione.

## Stato

- **Sondaggio di fattibilità** (`decomp/RAPPORTO.md`): fattibile con riserve.
  Codice pulito, nessun codice in RAM né salti indiretti; 250-300 routine stimate.
- **Fase 1, in corso:**
  - tabelle inline dell'idioma `rst $20` estratte e tradotte in dati C;
  - protezione hardware `e603` tabellata (un solo punto di lettura, valore fisso);
  - formato dei livelli nella ROM decodificato (`decomp/DATI.md`): sfondo e
    palette identici all'originale su tutte le 52 stanze, oggetti su 38;
  - tre routine tradotte e verificate, fra cui il decodificatore degli sfondi;
  - scheduler del ciclo principale: un giro su 64 slot di attività, ognuna una
    funzione a passi con stato in RAM; viene riprodotto uno a uno con funzioni C;
  - copertura del codice con sessioni guidate su tutte le stanze.
- **Dopo:** traduzione in lotti delle routine, routine di interrupt, strato
  hardware (video, sprite, palette, input) per rendere giocabile il risultato,
  pulizia finale. Il Z80 audio resta emulato o sostituito da campioni.

## Contenuto del repository

- `decomp/`: script di traccia, copertura e grafo (`trace.lua`, `coverage.py`,
  `callgraph.py`), cattura prima/dopo (`snap.lua`), estrattori (`rst20.py`,
  `e603.lua`, `stanze.py`), banco di prova e routine tradotte (`harness/`),
  rapporti (`RAPPORTO.md`, `DATI.md`).
- `roms/decoded/`: chip grafici decodificati, palette, dump di RAM e screenshot
  presi da MAME, usati come oracolo.
- `compiti/`: un brief per ogni ciclo di lavoro, con fatti verificati, verifiche
  obbligatorie, decisioni prese ed esito.

## Strumenti richiesti

MAME 0.264, `z80dasm`, Python 3 con Pillow, gcc e make. Il set ROM `solomon` va
messo in `roms/` e non va committato. Le misure si rifanno con i comandi in fondo a
`decomp/RAPPORTO.md`.
