# Stato del progetto

Aggiornato al 2026-10-21, sera. Fotografia operativa per riprendere il
lavoro; la descrizione del metodo è nel [README](README.md), la storia di
ogni ciclo nei brief in `compiti/`.

## Numeri

| Cosa | Valore |
|---|---|
| Destinazioni statiche nel Z80 principale | 299 |
| Coperte dalle sessioni (fase 1) | 195 |
| Routine osservate nel callgraph | ~120 |
| Righe in `decomp/ROUTINE.md` | 116 |
| …di cui `verificata` (doppia cattura MAME) | 104 |
| …di cui `tradotta` con nota (cattura singola) | 12 |
| Banco di prova (`harness --tutte`) | 112 confronti, 0 falliti |
| Catture `regs` in `decomp/out/` | 217 |
| Destinazioni mai osservate | 104 (blocco grosso in `8xxx-Axxx`: con ogni probabilità bonus e finale) |

## Fasi

- **Fase 1 (brief 01-04): chiusa.** Tracce, callgraph, formato livelli,
  scheduler a funzioni, copertura 195/299.
- **Fase 2 (brief 05-15): chiusa.** Perimetro osservato tutto tradotto in
  lotti verificati; 9 candidate escluse con motivazione (brief 15). Dentro:
  NMI (`0066`), scheduler `0493`, stato di Dana, motore oggetti, gestori
  d'urto, porte come funzioni di piattaforma (`porte.c`, proposta A del
  brief 12; proposta C — log degli accessi — rinviata a NMI/audio).

## Prossimi passi, in ordine

1. **Lotto 12 — copertura di bonus e finale** (decisione sciolta il
   2026-10-21: si fanno). Partita tracciata o sessioni guidate con
   forzature di RAM; stima 25-35 routine nuove. Il vincolo «bonus e finale
   non vanno tentati» del brief 04 cade con questo lotto, che deve
   aggiornare anche i cancelli di copertura.
2. **Altre zone mai eseguite**: forzature mirate di RAM e ricerca di
   copertura con input automatici (MAME a `-nothrottle` fa ore di gioco in
   minuti); una parte sarà codice morto da documentare e basta.
3. **Strato hardware** (video, sprite, palette, input) per rendere
   giocabile il C, poi pulizia con il replay come regressione.
4. **Z80 audio** (ROM `1.3jk`, 16 KB): da valutare; richiede la proposta C
   per l'oracolo delle scritture ai chip sonori. In alternativa resta
   emulato o campionato.

## Come si lavora (ricetta collaudata)

- Ciclo per lotto: brief dell'analista con fatti verificati e verifiche a
  uscita attesa → sviluppatore → verificatore → consegna. In cloud
  (sessioni Claude Code col repo GitHub collegato) la consegna è branch
  `lotto-NN` + PR; in locale si riverifica sempre: rigenerazione delle
  catture (devono tornare identiche bit a bit; i `.log` variano per la
  velocità media e si ripristinano prima del confronto; le catture
  `SNAP_COND` possono variare tra build di MAME — si valida col banco su
  entrambe le coppie) e verificatore sul brief, poi merge su main.
- Le istruzioni di ogni lotto vivono in `compiti/ISTRUZIONI-*.md` su main;
  alle sessioni cloud si allegano le due ROM (gitignorate) al primo
  messaggio.
- MAME senza finestra: `-video none -sound none` più `SDL_VIDEODRIVER=dummy`
  (già negli script).
- Attenzione ai costi cloud: un lotto con sotto-agenti ad alto ragionamento
  costa 30-60 USD. Al 2026-10-21 sera i crediti cloud sono esauriti
  (~7 USD residui): i prossimi lotti girano in locale o attendono ricarica.
