# Stato del progetto

Aggiornato al 2026-10-22. Fotografia operativa per riprendere il
lavoro; la descrizione del metodo è nel [README](README.md), la storia di
ogni ciclo nei brief in `compiti/`.

## Numeri

| Cosa | Valore |
|---|---|
| Destinazioni statiche nel Z80 principale | 299 |
| Coperte dalle sessioni (lotto 12 compreso) | 235 (78,6%) |
| Routine osservate nel callgraph | ~120 |
| Righe in `decomp/ROUTINE.md` | 116 |
| …di cui `verificata` (doppia cattura MAME) | 104 |
| …di cui `tradotta` con nota (cattura singola) | 12 |
| Banco di prova (`harness --tutte`) | 112 confronti, 0 falliti |
| Catture `regs` in `decomp/out/` | 217 |
| Destinazioni mai osservate | 64, tutte con sito di chiamata mai eseguito (probabile dato o codice raggiungibile solo da zone mai eseguite; 37 a `8000` o sopra) |

## Fasi

- **Fase 1 (brief 01-04): chiusa.** Tracce, callgraph, formato livelli,
  scheduler a funzioni, copertura 195/299.
- **Lotto 12 (brief 16): chiuso il 2026-10-22.** Copertura di bonus e
  finale con sole forzature di RAM: unione a 235/299, 40/104 del lotto
  censite in `decomp/CENSIMENTO-12.md`; il vincolo «bonus e finale non
  vanno tentati» del brief 04 è caduto. Ciclo fatto in locale
  (sviluppatore Opus ~95k token alla ripresa, verificatore Sonnet ~16k).
- **Fase 2 (brief 05-15): chiusa.** Perimetro osservato tutto tradotto in
  lotti verificati; 9 candidate escluse con motivazione (brief 15). Dentro:
  NMI (`0066`), scheduler `0493`, stato di Dana, motore oggetti, gestori
  d'urto, porte come funzioni di piattaforma (`porte.c`, proposta A del
  brief 12; proposta C — log degli accessi — rinviata a NMI/audio).

## Prossimi passi, in ordine

1. **Traduzione delle routine censite nel lotto 12**: 40 destinazioni in
   25 blocchi contigui in `decomp/CENSIMENTO-12.md`, due catture pilota
   già fatte (`0x3FCE porta`, `0x51FA finale`). Per i lotti futuri il
   censimento va rigenerato a mano o lo script va aggiunto a `decomp/`.
2. **Altre zone mai eseguite**: restano 64 destinazioni, tutte con sito
   di chiamata mai eseguito (tabella SI/NO in coda al censimento);
   forzature mirate di RAM, una parte sarà codice morto da documentare e
   basta. Attenzione: `att_27` è artificiale (esecuzione in RAM).
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
