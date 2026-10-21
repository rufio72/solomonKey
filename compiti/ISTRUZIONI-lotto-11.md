# Istruzioni per il ciclo del lotto 11 (chiusura del perimetro osservato)

Ciclo completo: analista, sviluppatore, verificatore, consegna su branch e
PR. Main è aggiornato al lotto 10: 107 righe in ROUTINE.md (98 verificate),
NMI, scheduler e porte tradotti. Restano ~13 routine osservate nel
callgraph: questo lotto chiude il perimetro osservato del Z80 principale.

## Passi

0. **Preparazione.** ROM allegate: copiale in `roms/` (`solomon.zip`,
   `solomonj.zip`). MAME via apt (`PATH=$PATH:/usr/games`). Rigenera i dati
   derivati come nei lotti precedenti (chiusure dei brief 07-14).
1. **Analista.** Ricalcola le routine osservate rimanenti sul callgraph
   aggiornato e includile tutte nel lotto, salvo esclusioni motivate.
   Scrivi `compiti/15-lotto-11-routine.md` nello stile dei brief 13 e 14:
   fatti verificati dal disassembly, tabella con catture e condizioni,
   verifiche con uscita attesa, domande aperte (preferenze risolte con la
   tua proposta; scelte di progetto risolte in modo conservativo e
   annotate; le decisioni dei brief 12-14 non si riaprono). Se una routine
   risulta non traducibile ora (dipendenze fuori perimetro), escludila con
   motivazione e rinviala al filone giusto.
2. **Sviluppatore.** Esegui il brief: traduzioni e catture con
   `cattura.sh`; porte via `leggi_porta`/`scrivi_porta`.
3. **Verificatore.** Riesegui le verifiche del brief e riporta passato o
   fallito per ciascuna.
4. **Consegna.** Branch `lotto-11`, committa tutto (brief, sorgenti,
   ROUTINE.md, catture `decomp/out/snap_*`; mai `roms/` né `.gitignore`),
   push e PR verso main. Nel rapporto finale includi anche il quadro di
   chiusura: quante routine osservate restano fuori e perché, e cosa resta
   nel progetto (zone mai eseguite, audio).

## Vincoli

- Nessun file sopra le 800 righe (spezzare prima di aggiungere).
- Nessun push su main.
- Rapporto finale di massimo 25 righe.
