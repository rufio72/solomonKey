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
- **Fase 1, chiusa** (brief 01-04): tabelle `rst $20` estratte, protezione
  `e603` tabellata, formato dei livelli decodificato (`decomp/DATI.md`),
  scheduler riprodotto a funzioni, copertura con sessioni guidate su tutte le
  stanze: 195 destinazioni su 299 (bonus e finale fuori perimetro per
  decisione in sospeso).
- **Fase 2, traduzione in lotti, chiusa** (brief 05-15): al 2026-10-21 il
  perimetro osservato è completo — **116 routine in ROUTINE.md, 104
  verificate** con doppia cattura MAME, banco a 112 confronti e 0 falliti;
  9 candidate escluse con motivazione e rinviate ai filoni futuri. Tra le
  cose fatte: il motore per-oggetto della NMI, la NMI stessa (`0066`) con lo
  scheduler `0493` e lo stato di Dana, e le porte hardware come funzioni di
  piattaforma (`porte.c`, censimento in `compiti/12-filone-porte.md`). I
  lotti dal 3 in poi girano in sessioni cloud di Claude Code che consegnano
  su branch con pull request; ogni lotto viene riverificato in locale
  rigenerando le catture (finora identiche bit a bit, salvo una condizionata
  che varia tra build di MAME ed è stata validata con entrambe le coppie).
- **Dopo, in ordine previsto:**
  - **stanze bonus e sequenza finale** (decisione sciolta il 2026-10-21: si
    fanno): copertura con sessioni guidate o partita tracciata, poi
    traduzione in lotti come il resto;
  - copertura delle altre zone mai eseguite (rami rari, morti particolari),
    con forzature di RAM e ricerca di copertura a input automatici;
  - strato hardware (video, sprite, palette, input) per rendere giocabile il
    risultato, e pulizia finale;
  - Z80 audio: per ora emulato o sostituito da campioni, decompilazione da
    valutare a nucleo chiuso.

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
