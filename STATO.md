# Stato del progetto

Aggiornato al 2026-11-05. Fotografia operativa per riprendere il
lavoro; la descrizione del metodo è nel [README](README.md), la storia di
ogni ciclo nei brief in `compiti/`.

## Numeri

| Cosa | Valore |
|---|---|
| Destinazioni statiche nel Z80 principale | 299 |
| Coperte dalle sessioni (lotto 12 compreso) | 235 (78,6%) |
| Routine osservate nel callgraph | ~120 |
| Voci con stato in `decomp/ROUTINE.md` | 390 (conteggio sulla colonna stato) |
| …di cui `verificata` (doppia cattura MAME non forzata) | 286 |
| …di cui `tradotta` con nota (senza cattura o cattura singola) | 61 |
| …di cui `tradotta (replay)` | 43 |
| Banco di prova (`harness --tutte`) | 335 voci, 1422 catture, 0 fallite |
| Catture `regs` in `decomp/out/` | 1422 (banco) |
| Stub residui del motore nemici | 0 (motore completo, `nemici_stub.c` tolto) |
| Destinazioni mai osservate | 59 documentate in `decomp/ZONE-MAI-ESEGUITE.md`: 53 `DATO`, 0 `MORTO`, 6 `RAGGIUNGIBILE` (5 con ricetta di sveglia provata); le altre 5 delle 64 originali sono ormai tradotte |

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

- **Lotto replay (brief 23): fatto in parte il 2026-10-27, infrastruttura
  verificata.** Oracolo (`replay.lua`/`replay.sh`, ~90 s) riproducibile
  bit a bit e giro in C (`replay.c`) con sanità a F0=2402 coincidente;
  banco storico intatto (220/0). Il giro si ferma al corpo 0A (4DA2),
  che via `rst 38` entra nel **motore dei nemici**: ~3,6 KB e 36 routine
  non tradotte — il brief stimava ~1 KB per tutti i corpi, il totale vero
  è ~5 KB. 7 corpi su 10 tradotti in `attivita.c`, non verificati (la
  loro verifica è il replay che non li raggiunge). Prossimi, in ordine:
  censimento e lotti del motore di 0A, poi corpi 01 e 10 (~870 B), poi
  giro da F0=2410 (`--oltre`). Lista e misure in `decomp/REPLAY.md`.
  La trappola dei provider di input (MAME legge tastiera e joystick
  dell'host anche senza finestra) è sistemata ovunque: dal brief 27 anche
  `cattura.sh` e `sessioni.sh` passano i quattro provider a `none`.
- **Cadenza del gioco libero (decisione utente 2026-11-04: «ci
  ritorniamo»).** Il modo libero chiude il frame con un'euristica (oggi ~22
  corpi per frame, `macchina_libera.c`), non con il costo vero delle
  istruzioni. La correzione del brief 53 è mirata alla sola animazione
  d'inizio quadro (3097); altri difetti di cadenza possono esistere altrove
  e vanno rivisti: già noto il fondale 322A (in MAME ~6 frame con NMI
  interne, nel libero uno solo; vedi `decomp/GIRO.md`, brief 53). Ogni nuovo
  difetto visivo o di ritmo nel gioco libero va sospettato prima come
  cadenza, poi come traduzione. Dal brief 58 il motore dei nemici (4F0F)
  dichiara il proprio costo (`MACCHINA_COSTO`: 3 per oggetto di tipo 8, 1
  per gli altri) e il budget del frame conta anche quello: la cadenza delle
  stanze con nemici è vicina a MAME (stanza 2: 2,39 spedizioni di 0A per
  frame contro 2,43; stanza 0: 4,12 contro 4,88).

## Prossimi passi, in ordine

0. **Motore dei nemici (corpo 0A)**: censito in `decomp/CENSIMENTO-NEMICI.md`
   con 21 tabelle `rst 38` (`harness/rst38_tables.h`), 220 routine e 8833 B.
   **Lotto N1 fatto** (brief 25): corpo 0A, 26 routine e `sub_3b62` in
   `decomp/harness/nemici*.c`, 27 stub in `nemici_stub.c`. Il replay da
   F0=2410 arriva al frame 2589 senza divergenze nello spedidore e si ferma
   al corpo 01. **Corpi 01 e 10 fatti** (brief 26, `attivita_01_10.c`), con
   i gestori d'urto mancanti della tabella 08AE e i punti di sincronia
   della NMI nel replay: il giro arriva in fondo all'oracolo (frame 5499,
   3090 NMI) senza divergenze nello spedidore, exit 0. Prossimo: punto 3
   di `decomp/REPLAY.md` (giro da altri F0 e classificazione delle
   divergenze a metà attività), poi i lotti N3-N12 per le routine che il
   giro 2410-5499 non raggiunge. **Lotto N2 fatto** (brief 27): tipi 8 e
   14 in `nemici_tipi_08_14.c` più 6 dipendenze anticipate da N3-N5,
   verificati con catture MAME nel banco (i moduli `nemici*.c` ora entrano
   anche in `harness`). Strumenti nuovi per i lotti seguenti: stato
   `stanzaNN` di `cattura.sh` (stanza forzata) e `SNAP_FORZA` di
   `snap.lua` (byte a IX/IY+n o registri forzati all'ingresso).
   **Lotti N3 e N4 fatti** (brief 29 e 30, 2026-10-29, schema economico:
   analista e sviluppatore Sonnet, verifica a script senza modello,
   ~0,6-1,1 $ a lotto, entrambi al primo colpo): N3 = tipo 9 in
   `nemici_tipo09.c`/`nemici_tipo09b.c` più 5 dipendenze di N6/N10;
   N4 = comparsa (tipi 0/4/5) in `nemici_comparsa.c`/`nemici_comparsab.c`
   con `cattura_oggetti.sh` e 4FC5 (fata) coperta via forzature dei
   chiamanti. Regola ratificata dall'utente: le routine che leggono il
   registro R si verificano coi wrapper `banco_r_*` di `nemici_r.c`
   (esiste un R compatibile), niente R nelle catture — vale anche per la
   scelta (2) del brief 28. In N3 corretto un bug latente di N1 in
   `sub_4592` (secondo `rla` perso nell'uscita col riporto).
   **Lotti N5, N6 e N7 fatti** (brief 31, 32 e 33, 2026-10-29, stesso
   schema, tutti al primo colpo, ~0,4-0,8 $ l'uno): N5 = tipi 10-13 con
   spezzatura in `nemici_tipi_10_13b.c` e riporto di 6C2C aggiunto in
   `z80_6xxx.c`; N6 = tipo 06b con 5 routine di N10 anticipate (51FA,
   5224, 523C, 5254, 5068), forzatura `BC=1<<j` su 51FA che apre tutte
   le 14 voci della tabella 5207, wrapper `banco_r_51fa`; N7 = rami non
   coperti del corpo 0A in coda a `nemici.c` (4767 ha un secondo
   chiamante 37B4 non tradotto, fuori dal motore, per un lotto futuro).
   **Lotto N8 fatto** (brief 34, 2026-10-30, stesso schema, al primo
   colpo, ~1 $): tipo 07 prima metà (5CFB-5F82) in `nemici_tipo07a.c`
   più 4FEF (di N10) e 5FAC (di N11) anticipate; wrapper `banco_r_5ddf`
   e `banco_r_5e1a` con `nemici_r_somma_r1` nuova; 90 catture in 14
   voci. Stub nuovi 610C, 5FC8 e 627F al posto di quello di 5CFB
   (toccano a N11, che completa il ramo 5D08-5D28 di 5CFB e le 27 righe
   gcov scoperte, tutte spiegate in `ROUTINE.md`). 5DDF e 5E09 del
   brief 28 sono già fatte qui (nota di coerenza in coda al brief 28).
   **Lotti N9 e N10 fatti** (brief 35 e 36, 2026-11-03, stesso schema,
   entrambi al primo colpo, ~1 $ l'uno): N9 = tipi 15-17 (16 routine,
   6B43-809C) in `nemici_tipi_15_17.c`, con 7F68 (N1) nel banco e 6
   forzate in più per coprire la tabella 7F39; N10 = tipo 06 prima metà
   (19 routine, 4FD4 e 5005-51D7) in `nemici_tipo06a.c`, wrapper
   `banco_r_4fd4` e `banco_r_50df` nuovi, 8 routine N10 erano già
   anticipate in N3/N6/N8. **Lotti N11 e N12 fatti insieme** (brief 37,
   2026-11-03, stesso schema, al primo colpo, ~1 $): tipo 07 seconda
   metà (18 routine) in `nemici_tipo07b.c`, 2474 e 4ADA in
   `nemici_comuni.c`, 49AE nel banco (17 catture, unico modo di
   esercitare 4ADA), wrapper `banco_r_5cfb`, convenzione del riporto per
   l'uscita anomala di 627F (come 531C), bug di lettura ROM in 2474
   corretto (`M()`→`leggi()` sulla tabella 9495). `nemici_stub.c` tolto:
   **IL MOTORE DEI NEMICI È COMPLETO**, 220/220 routine, gcov scoperto
   solo sui casi fuori tabella non forzabili.
1. **Routine censite nel lotto 12: FATTE** (brief 38, 2026-11-03, che
   rifà e supera il brief 28; schema economico, decimo ciclo al primo
   colpo, ~1 $): delle 40 destinazioni di `decomp/CENSIMENTO-12.md`,
   23 tradotte in questo lotto (03EE-7B9E, ~500 byte), 9 erano già
   tradotte nei lotti N8-N12, 8 documentate e basta (etichette interne
   e interni di corpi a passi); il destino di tutte e 40 è scritto a
   mano in `CENSIMENTO-12.md`. Wrapper `banco_r_4302` nuovo; stati
   `finale2`/`attNN` in `cattura.sh` e `SNAP_POKE_EXTRA` in
   `cattura_oggetti.sh`. **Lotto della chiave 0D18 FATTO** (brief 39,
   2026-11-03, undicesimo ciclo economico al primo colpo, ~0,7 $):
   dispatcher 0CFD e i 16 gestori della tabella 0D18 (0CFD-123B,
   1279 byte) in `z80_chiave_a.c` e `z80_chiave_b.c`, `sub_0ce6`
   completata (via l'avviso su stderr), 62 forzate per la copertura dei
   rami (gcov pulito salvo 5 righe di sola etichetta), la scrittura in
   ROM di 0D38 (`ld (0),a`) non tradotta per decisione; corretti i due
   errori del censimento (11A5 cade in 1172, non in 113C).
2. **Altre zone mai eseguite: DOCUMENTATE** (brief 40, 2026-11-03,
   dodicesimo ciclo economico al primo colpo, ~0,7 $, solo analisi,
   nessun codice toccato): delle 64 originali, 5 sono ormai tradotte
   (4FC5, 5F82, 6046, 6FC3, 6FDF) e le 59 restanti hanno verdetto in
   `decomp/ZONE-MAI-ESEGUITE.md`: 53 `DATO` (siti in zona dati o byte
   di tabella letti come call, compresa la famiglia AEE2-AFF0), 0
   `MORTO`, 6 `RAGGIUNGIBILE` — 5 esercitate con ricetta provata
   (2533, 6379, 4364, e 7E03/7E14 che richiedono i dip switch Bonus
   Life via `solomon.cfg`), 1 non svegliata (560A, serve un nemico di
   tipo 12 con via libera). Scoperta collaterale: la tabella `rst 20h`
   a 646B ha 10 parole vere contro le 3 di `rst20_tables.h` (tutte già
   eseguite; corretta nel brief 51, anche in `rst20.py`, banco identico). Attenzione: `att_27` è artificiale
   (esecuzione in RAM).
   **Novità 2026-10-27**: una partita dal vivo sorvegliata con punti di
   interruzione (`decomp/copertura_viva.lua`, MAME con finestra) ha
   svegliato `4FC5` — 6 esecuzioni, sito `4F8F` eseguito 1 volta, quindi
   le altre 5 arrivano da un'altra via: non è codice morto. Le altre 63
   restano mute. `4FC5` cade nel motore dei nemici del corpo 0A (vedi
   REPLAY.md). Seconda partita mirata (campanella presa, fata catturata,
   uscita dal quadro, nessun nemico ucciso): 1 sola esecuzione, sito mai
   passato — di quei tre eventi uno solo la accende, con ogni probabilità
   la comparsa della fata; cattura e uscita non passano di lì. Nella
   partita libera gli altri scatti corrispondono a morti dei nemici e
   palla di fuoco. Ipotesi: gestore della comparsa di un'entità da evento
   (spawn), non della sua rimozione. Ricette di riproduzione annotate qui;
   log in `decomp/out/partita_viva_20261027.log` e `...27b.log`.
3. **Strato hardware** (video, sprite, palette, input) per rendere
   giocabile il C, poi pulizia con il replay come regressione.
   Lotto H1 (brief 41) chiuso: oracolo video in `decomp/out/video/`,
   formato in `decomp/VIDEO.md`.
   Lotto H2 (brief 42) chiuso: renderizzatore software `rendi_frame`
   (`harness/video_rendi.c`), 7 stati su 7 identici (`sh rendi_video.sh`).
   Lotto H3 (brief 43) chiuso: finestra SDL2 (`video_sdl.c`), programmi
   `mostra_oracolo` e `mostra_replay` (3090 frame resi, gancio `replay_dopo_nmi`).
   Lotto H4a (brief 44) chiuso: `harness/gioco_libero` fa girare il C dal
   vettore 0000 (boot 2833-2959, `sub_29cf`/`sub_2a03` tradotte), NMI una
   per frame con gli ingressi registrati; 537 NMI (frame 66-602)
   coincidono con MAME e il dump 602 coincide byte per byte; si ferma al
   corpo 1F (43C8).
   Lotto H4b1 (brief 45) chiuso: NMI nello spedidore e a metà attività nella
   macchina, corpi 1F e 06 tradotti (`attivita_attract.c`), strumento
   `registra_r.sh` e sentinella del registro R (0 letture nei frame
   0-5499); il giro arriva al corpo 07 (frame 1188) con 1123 NMI e nessuna
   divergenza nello spedidore.
   Lotto H4b2 (brief 46) chiuso: corpo 07 (2CCD) tradotto fino alla fase 12
   (`attivita_attract07*.c`) e routine 3ADF nel banco (305 voci, 1272
   catture); il giro arriva al frame 1398 (`ROUTINE 2DCE mancante`), 1333 NMI
   confrontate, 0 divergenze nello spedidore.
   Lotto H4b3 (brief 47) chiuso: fasi 13-25 del corpo 07 (`attivita_attract07c.c`,
   `07d.c`), corpi 08, 0C e 0B (`attivita_attract08.c`), routine 37B4 e 2D8A nel
   banco (307 voci, 1278 catture); il giro dal boot arriva al frame 2410 e
   si salda all'oracolo storico: `SALDATURA 2410 ... 0, coincide`, 41396
   spedizioni, 2345 NMI (30 divergenti a metà attività, 0 nello spedidore).
   Per la saldatura sono serviti ganci `PUNTO_GIRO` (attivi solo nel giro) in
   routine del motore dei nemici già verificate; il replay storico non cambia.
   H4c (brief 48): il giro arriva al frame 5499 senza arresti, con oracolo
   esteso, hash di 13 framebuffer resi dalla RAM del C (tutti coincidenti con
   MAME) e una riga di regressione (`sh giro_completo.sh`): `GIRO COMPLETO
   0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività
   (attese), 13 frame resi con hash coincidenti`. H4 chiuso. Vedi `decomp/GIRO.md`.
   H6 (brief 50): `gioca` suona. Le scritture a e800 diventano NMI del driver
   audio tradotto, due IRQ per frame (ordine IRQ A, latch, IRQ B), emu2149 a
   44100 Hz mono verso SDL audio (o hash con `-n`). Latch identici a MAME
   (165 su 165); cambi AY: 604 frame con insieme diverso, 312 con stato
   diverso (fissati come atteso in `sh partita_audio.sh confronta`); hash audio
   c4b40fea06623a96. Residuo da modellare in un eventuale H6b. Vedi `decomp/GIRO.md`.
   H7 (brief 51, 2026-11-04): pacchetto. `make -C decomp/harness gioco`
   (estrae lo zip, costruisce `main64.bin`, compila), `gioca` da qualunque
   cartella (percorsi da `/proc/self/exe`), a runtime solo 4 ROM; joystick SDL
   grezzo con diagnostica `--joystick`, hotplug e `--joy-mappa` (predefinita
   0,1,7,6), `--volume`, `--pieno`, `--aiuto`; `rst20_646B` a 10 voci. Banco,
   replay, giro e audio identici byte per byte a prima. Piano hardware chiuso;
   resta la prova manuale sul DAIJA. Vedi `decomp/GIRO.md`.
   [x] Lotto 52 (brief 52, 2026-11-04): banco dei corpi (`SNAP_CORPO`,
   `solo_ram`, `cattura_corpi.sh`) e 13 corpi del gioco libero (04 05 09 0F
   12 13 17 18 19 1A 1B 1D 23, 15 voci e 32 catture nuove); la partita
   registrata `decomp/ricette/partita_1a.reg` (magia sulla chiave) non si
   ferma più al frame 3320. Il corpo 12 si ferma sul registro R, non
   modellato (lotto dedicato). Nota: la continuazione 12AB di 1288
   (raccolta di certi oggetti) non è tradotta e nel gioco libero non ha
   effetto. Vedi `decomp/GIRO.md`.
   [x] Brief 53 (2026-11-04): stelline d'inizio quadro nel gioco libero,
   un passo del cerchio per frame come in MAME (3097 dichiara il frame pieno,
   gancio `MACCHINA_FRAME_PIENO`); oracolo, banco, replay e giro identici.
   Il fondale (322A) ha un difetto di cadenza diverso, misurato e rimandato.
   Vedi `decomp/GIRO.md`.
   [x] Brief 58 (2026-11-05): cadenza del motore dei nemici nel gioco
   libero; 4F0F dichiara il costo per oggetto (`MACCHINA_COSTO`, 3 tipo 8,
   1 altri), gli orbitanti della stanza 3 non vanno più 2,26 volte troppo
   veloci; oracolo, banco, replay e giro identici; partite registrate del
   libero con hash nuovi (vanno rifatte). Vedi `decomp/GIRO.md`.
   [x] Lotto 59 (brief 59, 2026-11-05): famiglia 3901/3903 della fase 0
   del corpo 07 (giro sulla tabella 82D3 = 03 0C 15 1F 28 32 FF, blocco
   3903-392F «BONUS»/«FINAL»/«PRISON!», c045 bit 4 e c09a = 1E), 3960 (cifra
   delle decine a d610) e 2D1F-2D48 della fase 5 (rimescola otto celle di
   c0bd con R); `banco_r_2ccd` cerca R anche nei corpi (`solo_ram`); 10
   catture nuove (f0q02..f0q32, f5r1, f5r2), banco 335 voci / 1422 catture.
   Giro, oracolo e partita 1A identici. La riproduzione di riferimento
   (`ricette/riferimento_nuova.reg`, prima FERMO 3903 al 13926) arriva a
   16000 frame senza FERMO: `HASH ultimo a040200b69679371`, `AUDIO hash
   caa004b13ad0a661` (parte A). CODA RESIDUA: la 3901 è chiusa; il libero
   senza ingressi e i semi 4/67 si fermano ora, negli stessi frame (3385,
   25937, 25198) e con gli stessi hash, su «routine 39B9 mancante» (c243 =
   33, coda della fase 0, stanza dopo la finale nella demo post-game-over),
   censita e non tradotta. Parte B: lettura e603 a 4CED modellata come
   `C & 08` (vedi `decomp/DATI.md`).
   [x] Lotto 54 (brief 54, 2026-11-04): corpi 1C, 25, 26, 22 (con 12F0, il
   suo `rst 38` e 11 gestori di raccolta nuovi), raccolta 12AB di 1288 e
   rami 352E/353B/351E della fase 1 del 07; 5 voci e 61 catture nuove
   (modo `stato` di `cattura_corpi.sh`). Semi 4 e 67 arrivano a 6000 frame;
   partita 1A: nuovo `AUDIO hash c097308a8678392e`. Vedi `decomp/GIRO.md`.
   [x] Lotto 55 (brief 55, 2026-11-04): corpi 15, 16 (classifica 6842 e
   7A56), 24 (finale), 27 e le voci vuote 1E 21: tutti i 40 codici della
   tabella 3B11 sono tradotti; 8 voci e 41 catture nuove. Il fine partita
   arrivava poi a rami mancanti del corpo 06 dell'attract: tradotte le fasi
   5 (7A0B), 8 (7B3B), 10 e 11 (7604; la tabella 75EC ha 12 voci). I semi
   10, 4, 67 passano fine partita e attract e si fermano solo sul registro R.
   Il registro R (corpo 12, 4302) è poi modellato nel lotto 56; nessun
   altro ramo «routine mancante» incontrato dal fine partita. Vedi
   `decomp/GIRO.md`.
   [x] Lotto 56 (brief 56, 2026-11-04): registro R surrogato (LCG a 32 bit)
   nel solo gioco libero, banco/replay/oracolo identici. I semi 10, 4, 67
   superano il corpo 12 e si fermano su «routine 386A mancante» (fase 0 del
   corpo 07 dell'attract con c243 >= 24) ai frame 27938, 25954, 25218:
   tradotta nel lotto 57.
   [x] Lotto 57A (brief 57, 2026-11-04): 386A (`inc c`, c09a = 1B con
   c243 >= 24) in `fase_07_3861`. I semi 10, 4, 67 passano 386A e, nello
   stesso frame (27938, 25954, 25218) e nella stessa fase, si fermano su
   «routine 3901 mancante»: il giro 38FB-3901 sulla tabella 82D3 con c243
   oltre la prima voce (`jr l38fbh`), NON tradotto (perimetro chiuso, da
   decidere dopo il test dell'utente). FRAME rese 27872/25888/25152, HASH
   ultimo 16ea230eea591d30, AUDIO hash invariati rispetto al lotto 56.
   [x] Lotto 57B (brief 57, 2026-11-04): opzione `--scanlines` di `gioca`
   (predefinita spenta): texture 1x448 semitrasparente sopra il frame, la
   metà inferiore di ogni riga sorgente scurita al 69% circa (alpha 80).
   Solo presentazione: framebuffer, hash video, replay e oracolo invariati.
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
