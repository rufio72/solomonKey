# Istruzioni per il ciclo del lotto 7 (sessione cloud)

Contesto: main è aggiornato ai lotti 1-6 (66 righe in ROUTINE.md, 63
verificate). Le routine «foglia» sono esaurite: il lotto 7 apre i blocchi con
chiamate a routine non tradotte (vedi la chiusura di
`compiti/10-lotto-06-routine.md`).

## Passi

0. **Preparazione.** Le due ROM sono allegate al messaggio: copiale in
   `roms/` con i nomi `solomon.zip` e `solomonj.zip`. Installa MAME via apt
   (il binario finisce in `/usr/games`: usa `PATH=$PATH:/usr/games` senza
   modificare gli script). Rigenera i dati derivati gitignorati come nei
   lotti precedenti (vedi le chiusure dei brief 07-10).
1. **Analista.** Mappa i grappoli rimasti tra le destinazioni coperte: per
   ogni routine non tradotta chiamata dalle candidate, che cosa
   sbloccherebbe la sua traduzione. Scegli il primo grappolo (quello che
   sblocca di più col rischio minore) e scrivi
   `compiti/11-lotto-07-routine.md` con la mappa dei grappoli, i fatti
   verificati dal disassembly, le verifiche con uscita attesa e le domande
   aperte: le preferenze risolvile con la tua proposta, le scelte di
   progetto risolvile in modo conservativo e annotale nel brief.
2. **Sviluppatore.** Esegui il brief: traduzioni in C e catture con
   `cattura.sh`, stesse regole di qualità dei lotti precedenti.
3. **Verificatore.** Riesegui le verifiche del brief e riporta passato o
   fallito per ciascuna.
4. **Consegna.** Crea il branch `lotto-07`, committa tutto (brief, sorgenti,
   ROUTINE.md, catture `decomp/out/snap_*`; mai `roms/` né `.gitignore`),
   fai il push del branch e apri una pull request verso main con un
   riassunto del lotto. Se il push o la PR falliscono, fermati e dillo nel
   rapporto senza tentare altre vie.

## Vincoli

- Nessun file sopra le 800 righe.
- Nessun push su main.
- Rapporto finale di massimo 20 righe.
