# Stato del progetto

Aggiornato al 2026-10-22, sera. Fotografia operativa per riprendere il
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

## Lavoro in sospeso

- **Lotto replay (brief 23, `compiti/23-lotto-replay.md`): brief scritto
  (analista Fable, ~130k token), ciclo NON avviato.** Alla ripresa vanno
  risolte le domande aperte in fondo al brief, poi sviluppatore e
  verificatore. La domanda 1 è una scelta di progetto per l'utente:
  tradurre i 10 corpi di attività mancanti (~1 KB, misurati) nello stesso
  lotto dell'infrastruttura, o spezzare in 23a/23b (proposta
  dell'analista: un lotto solo). Le 2-5 sono preferenze con proposta
  (verifica dei corpi col replay stesso; registrazione in Lua scriptato;
  esclusioni d'impronta c040-c043 e cf00-cfff tarate a F0; finestra
  F0=2402, fine 5500).

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
4. **Z80 audio** (ROM `1.3jk`, 16 KB): decisione del 2026-10-22, **va
   tradotto per intero** (audio originale, niente emulazione né campioni).
   Lotto 1 (infrastruttura, brief 18) e lotto 2 (ossatura, brief 19)
   chiusi lo stesso giorno. C'è l'oracolo dei cambi di registro sui tre
   YM2149 per tutti i 65 codici (`out/audio/`), e l'ossatura C del driver
   (`decomp/harness/audio_*.c`) coincide con l'originale su avvio, attesa,
   codice 00 e codici ignorati; interprete a stub, diverge al frame 211
   come atteso. Banco a stato di fine frame; il flusso emesso è nel
   formato degli oracoli, pronto per il renderizzatore. Censimento dei 16
   gestori di 0216 in `AUDIO-SEQUENZE.md`; copertura call audio 28/97.
   Decisione utente: test finale = i .wav dei 65 codici generati dal C
   via emulatore YM2149 (lotto dedicato dopo la traduzione).
   Lotto 3 (interprete, brief 20) chiuso il 2026-10-22: l'interprete
   delle sequenze è tradotto (aud_0147, glissando, inviluppo, 16 gestori,
   9 voci × 2 strati); 39 oracoli su 44 identici per intera durata,
   comando 40 escluso (mai inviato dal gioco, voce di tabella fuori ROM),
   5 eccezioni di tempismo fine documentate in AUDIO.md. PUNTO APERTO:
   le eccezioni si giudicano col test dei .wav; se stona, modello a cicli
   mirato. Lotto 4 (dati, brief 21) chiuso il 2026-10-22: la regione
   dati 05C6-3FFF è sorgente C generato (`estrai_dati.py`, andata e
   ritorno byte per byte, banco IDENTICO con `AUDIO_DATI=estratti`);
   `SPARTITI.md` cataloga i 63 comandi, nomi da battezzare coi wav.
   Lotto 5 (renderizzatore, brief 22) chiuso il 2026-10-22: emu2149
   vendorizzato, `audio_wav.sh` genera i 63 wav dei comandi dal C
   (`out/wav/codici/`) e le coppie C/MAME dei 44 oracoli
   (`out/wav/oracoli/`). Wav BYTE PER BYTE IDENTICI in 40/44; le 4
   differenze restanti sono deboli (picchi −33…−49 dBFS, eccezioni.txt)
   e il 2026-10-23 l'utente le ha giudicate OK all'ascolto: punto chiuso,
   niente modello a cicli salvo differenze udibili future; nomi dei suoni da
   battezzare in `audio_nomi.tsv`. La catena audio è completa: driver
   tradotto, dati estratti, resa wav dal C.

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
