# Istruzioni per il ciclo del lotto 9 (porte, proposta A)

Ciclo completo: analista, sviluppatore, verificatore, consegna su branch e
PR. Il disegno è già deciso: `compiti/12-filone-porte.md` contiene il
censimento, le proposte e le **Decisioni** (proposta A adottata; C rinviata
a NMI/audio; B scartata; e603 rinviata alle zone mai eseguite). Attenersi a
quelle decisioni senza riaprirle.

## Passi

0. **Preparazione.** ROM allegate: copiale in `roms/` (`solomon.zip`,
   `solomonj.zip`). MAME via apt (`PATH=$PATH:/usr/games`). Rigenera i dati
   derivati come nei lotti precedenti (chiusure dei brief 07-12).
1. **Analista.** Scrivi `compiti/13-lotto-09-porte.md`:
   - il livello di piattaforma secondo la proposta A e la domanda 1 del
     brief 12: `porte.c` con `leggi_porta`/`scrivi_porta`, indirizzi
     hardware espliciti nel C tradotto (mai `M()` sulle porte), commento
     con il significato MAME a ogni sito;
   - le routine da tradurre: i grappoli sbloccati censiti nel brief 12
     (~1,5 KB pronti più le candidate che risultano eleggibili), con la
     tabella usuale (estensione reale, conteggi, catture per stato e
     condizioni), stesse regole di qualità dei lotti precedenti;
   - verifiche con uscita attesa; domande aperte: preferenze risolte con la
     tua proposta, eventuali scelte di progetto nuove risolte in modo
     conservativo e annotate (le decisioni del brief 12 non si riaprono).
2. **Sviluppatore.** Esegui il brief: `porte.c`, traduzioni, catture con
   `cattura.sh`.
3. **Verificatore.** Riesegui le verifiche del brief e riporta passato o
   fallito per ciascuna.
4. **Consegna.** Branch `lotto-09`, committa tutto (brief, sorgenti,
   ROUTINE.md, catture `decomp/out/snap_*`; mai `roms/` né `.gitignore`),
   push e PR verso main con un riassunto. Se il push o la PR falliscono,
   fermati e dillo nel rapporto.

## Vincoli

- Nessun file sopra le 800 righe.
- Nessun push su main.
- Rapporto finale di massimo 20 righe.
