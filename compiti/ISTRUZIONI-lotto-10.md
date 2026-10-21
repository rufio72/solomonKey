# Istruzioni per il ciclo del lotto 10

Ciclo completo: analista, sviluppatore, verificatore, consegna su branch e
PR. Main è aggiornato al lotto 9: 95 righe in ROUTINE.md (87 verificate),
livello porte attivo (`porte.c`, proposta A del brief 12). Restano ~25
routine osservate: lo scheduler `0493` nel banco, il filone NMI strutturale
(prerequisito caduto con le porte) e le code sbloccate dai lotti 7-9.

## Passi

0. **Preparazione.** ROM allegate: copiale in `roms/` (`solomon.zip`,
   `solomonj.zip`). MAME via apt (`PATH=$PATH:/usr/games`). Rigenera i dati
   derivati come nei lotti precedenti (chiusure dei brief 07-13).
1. **Analista.** Ricalcola i grappoli eleggibili sul callgraph aggiornato e
   scegli il lotto a resa migliore col rischio minore tra: scheduler
   `0493`, NMI strutturale, code sbloccate. Scrivi
   `compiti/14-lotto-10-routine.md` nello stile dei brief 11 e 13: mappa,
   fatti verificati dal disassembly, tabella delle routine con catture e
   condizioni, verifiche con uscita attesa, domande aperte (preferenze
   risolte con la tua proposta; scelte di progetto risolte in modo
   conservativo e annotate; le decisioni dei brief 12 non si riaprono).
2. **Sviluppatore.** Esegui il brief: traduzioni e catture con
   `cattura.sh`, stesse regole di qualità dei lotti precedenti; gli accessi
   alle porte passano da `leggi_porta`/`scrivi_porta`.
3. **Verificatore.** Riesegui le verifiche del brief e riporta passato o
   fallito per ciascuna.
4. **Consegna.** Branch `lotto-10`, committa tutto (brief, sorgenti,
   ROUTINE.md, catture `decomp/out/snap_*`; mai `roms/` né `.gitignore`),
   push e PR verso main con un riassunto. Se il push o la PR falliscono,
   fermati e dillo nel rapporto.

## Vincoli

- Nessun file sopra le 800 righe (spezzare prima di aggiungere, come fatto
  con z80_38xx.c).
- Nessun push su main.
- Rapporto finale di massimo 20 righe.
