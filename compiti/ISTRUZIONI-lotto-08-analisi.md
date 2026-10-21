# Istruzioni per la fase di analisi del filone porte (lotto 8)

Solo fase analista: nessuna modifica al codice, nessuna traduzione. Il
risultato è un brief con il censimento delle porte e le proposte di disegno,
con le scelte di progetto lasciate aperte: le decide la chat principale con
l'utente prima dello sviluppo.

## Contesto

Main è aggiornato al lotto 7 (74 righe in ROUTINE.md, 71 verificate). La
mappa dei grappoli in `compiti/11-lotto-07-routine.md` mostra che i blocchi
rimasti sono quasi tutti bloccati da accessi a porte hardware. Serve un
livello di piattaforma nel C dell'harness per sbloccarli.

## Passi

0. **Preparazione.** ROM allegate: copiale in `roms/` (`solomon.zip`,
   `solomonj.zip`). Installa MAME via apt (`PATH=$PATH:/usr/games`).
   Rigenera i dati derivati come nei lotti precedenti (chiusure dei brief
   07-11).
1. **Censimento.** Trova tutti gli accessi a porte del Z80 principale nel
   disassembly (`in`/`out` e varianti), e per ciascuno: indirizzo
   dell'istruzione, porta, direzione, routine che lo contiene (e suo stato in
   ROUTINE.md), semantica dedotta dal contesto, frequenza osservata nelle
   tracce (titolo/stanza_00/gameover). Verifica nel manuale del driver MAME
   (`solomon.cpp` è pubblico, ma se la rete non lo consente deducilo dal
   codice) a cosa corrisponde ogni porta.
2. **Proposte di disegno.** Due o tre alternative per rappresentare le porte
   nell'harness C (per esempio: funzioni di piattaforma con valori
   registrati da snap.lua nelle catture; stub con valori fissi e nota;
   estensione del formato di cattura). Per ciascuna: fedeltà dell'oracolo,
   modifiche richieste a `cattura.sh`/`snap.lua`/harness, lavoro stimato,
   rischi.
3. **Brief.** Scrivi `compiti/12-filone-porte.md` con: censimento completo
   (tabella), i grappoli che si sbloccherebbero, le proposte con pro e
   contro, le verifiche future con uscita attesa, e le domande aperte
   marcate «preferenza» (risolte con la tua proposta) o «scelta di progetto»
   (lasciate aperte con la tua raccomandazione motivata).
4. **Consegna.** Branch `lotto-08-analisi`, committa solo il brief ed
   eventuali script di censimento in `decomp/` (mai `roms/` né
   `.gitignore`), push e PR verso main. Rapporto finale di massimo 20
   righe: numeri del censimento, grappoli sbloccabili, le scelte di
   progetto da sottoporre all'utente.

## Vincoli

- Nessuna modifica a codice C, script esistenti o ROUTINE.md.
- Nessun push su main.
