# Z80 audio: osservazione e pianificazione della traduzione

Decisione del 2026-10-22: **il Z80 audio sarà tradotto** per intero (niente
emulazione né campioni). Questo rapporto non è un via/stop: serve a
pianificare i lotti di traduzione. Prodotto dal brief 18, aggiornato dal
brief 19 (ossatura tradotta, sezione in fondo). Indirizzi e conteggi,
nessun byte della ROM.

## Hardware (MAME 0.264, `space.map.entries`)

| CPU / spazio | intervallo | cosa |
|---|---|---|
| audiocpu program | 0000-3fff | ROM `1.3jk` (16 KB) |
| audiocpu program | 4000-47ff | RAM (2 KB) |
| audiocpu program | 8000 | lettura del latch (`:soundlatch`) |
| audiocpu program | ffff | scrittura nop (l'avvio vi scrive) |
| audiocpu io | 10-11, 20-21, 30-31 | tre YM2149 `:ay1` `:ay2` `:ay3` a 1,5 MHz (pari = indirizzo, dispari = dato) |
| maincpu program | e800 | scrittura del comando audio |

Z80 audio a 3,07 MHz. Il driver scrive anche alle porte **12 e 13, non
mappate**: misurato `PORTE IGNOTE 12=128 13=64` in tutte le 43 corse, uguale
al gioco normale e nella corsa `avvio`; sono scritture del solo avvio
(stessi conteggi con 90 o 900 frame), non del ciclo per interruzione.
Causa: un errore dell'originale in `00A4`. Il ciclo dei tre chip fa
`ld a,0fh; add a,c; ld a,0ffh` (00CE-00D1) e scarta la somma, quindi `c`
avanza di 1 per giro (10, 11, 12) invece che a 20 e 30: il chip 1 riceve
07..0A come dati nel registro 0A e le porte 12 e 13 ricevono 8 e 4
scritture per chiamata; 16 chiamate all'avvio danno `12=128 13=64`. Il C
riproduce l'errore e conta le stesse porte ignote.

Registro selezionato: `audio.lua` (e il C) usano il byte intero scritto
alla porta pari, senza la maschera dei 4 bit bassi del YM2149. Non ha
effetto: il driver seleziona solo 06-0D o valori calcolati da `b`, e negli
oracoli la colonna `reg` vale solo 00-0D (misurato su tutti gli `_ay.txt`).
Fuori da ROM, RAM e latch la memoria del Z80 audio si legge 00 (lo mostra
il comando 40, vedi `AUDIO-SEQUENZE.md`).

## Flusso del comando

- Scrittura a e800 dal principale → NMI sull'audio. NMI `0066` legge 8000,
  incrementa, salva in **4243**, `retn`.
- Avvio `0072`: `sp = 4800h`, `im 1`, 16 giri di azzeramento della RAM e
  `00A4`, poi `ei` e ciclo infinito su `029E`; IRQ su `0038`,
  **2 IRQ per frame** (misurato: `0042 0147 1440` in 720 frame di `cod_00_07`, 33 in
  30 frame di `avvio`).
- Spedidore `00E4`: legge 4243 (0 = niente), lo azzera, decrementa: codice
  00 → `00d6` (arresto), 01-40 → `rst 8`/`rst 10h` sulla tabella a **0AF3**
  (voce del codice v a 0AF3+2v); 41-FE ignorati (`cp 041h; ret nc`), FF
  diventa 0. Confermato: i codici 41-FF producono **0 cambi AY** e 0 PC
  nuovi (verifica 5 del brief).
- Codici inviati dal gioco nella corsa `gioco` (frame 0-2200): 29 scritture
  e800, tutte `gioco` in `out/audio/gioco_latch.txt`.

## Numeri (rifarli: `sh audio.sh > out/audio/audio.log 2>&1; python3 audio_numeri.py`)

Unione di 44 corse (`avvio` con traccia dal frame 0, 9 gruppi `cod_*`
00-40, `cod_41_ff`, 32 `lungo_*`, `gioco`); main sospeso con `suspend 0`
tranne in `gioco`. La corsa `avvio` (brief 19, `sh audio.sh avvio`, 30
frame) aggiunge 54 PC: 0000-0001, 0072-009D, 00A4-00D5.

- PC distinti eseguiti: **721** (max 05C3, nessuno ≥ 4000).
- Byte di ROM eseguiti: **1268 su 16384 (7,7%)**, mai eseguiti 15116;
  ultimo byte eseguito 05C5; byte eseguiti oltre 0AF3: 0.
- Intervalli codice 12, dati 12: `0000-0003 0008-000d 0010-0013 0018-0026
  0038-004f 0066-01ce 01e8-028a 029e-03c6 03ee-043d 0470-047d 0485-0590
  05a6-05c5`.
- Riempimento uniforme in fondo alla ROM: 385F-3FFF (1953 byte).
- Destinazioni di call/rst eseguite: 8; **routine (con 0038 e 0066): 10**:
  `0008 0010 0018 0038 0066 00A4 00E4 0147 029E 036B`.
- `rst` usati: `08=26930 10=68863 18=40599` esecuzioni; `jp (hl)` eseguiti
  14001, da un solo sito (`0216`): è lo smistamento dei comandi delle
  sequenze, i cui gestori non risultano come routine. Censimento dei 16
  gestori (riga `gestori 0216` di `audio_numeri.py`) in `AUDIO-SEQUENZE.md`.
- Copertura delle call statiche: **unione 28 su 97 (28,9%)** con la corsa
  `avvio` (17 prima). Le mai osservate hanno il sito di chiamata a 05C6 o
  sopra (dati letti come codice da z80dasm); `00A4` da `0090` è ora
  osservata (`0090 00A4 16` in `avvio_chiamate.txt`).
- RAM audio scritta dal frame 110: **458 byte distinti in 41 zone**.
  Più scritti: `47FC=1078715 47FD=1078715 47FE=631824 47FF=631824
  4242=392234 4240=336636 4241=274334 47EE=159408 47EF=159408 47F0=144741`.
- Codici 00-40: 65 osservati, **65 con cambi AY**. Cambi per codice nella
  finestra di 90 frame in `out/audio/cod_*_ay.txt` (stampati da
  `audio_numeri.py`): da 2 (04, 05, 3B, 3C) a 1540 (2F).
- **Codici lunghi** (cambi negli ultimi 10 dei 90 frame): 32: `0C 0D 0F 13
  14 15 16 17 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 26 28 29 2A 2B 2C 2D 2E
  2F 3D 3E 40`. Nelle corse `lungo_*` da 900 frame restano attivi fino in
  fondo (musiche in ciclo, presumibilmente): `13 14 16 19 1A 1B 1C 1E 1F 20
  21 23 24 28 2A 3D`. Alcuni «lunghi» erano falsi positivi del criterio
  (cambi del codice precedente ancora in corso nella finestra): da soli
  `17`, `2E`, `3E` finiscono in 1 frame, `29` in 9, `2C` in 21.
- Corsa completa di `audio.sh`: 10 min 25 s (le 32 corse lunghe pesano
  ~8 min a 15 s l'una), traccia massima ~100 MB, cancellata.

## Codice contro dati

Il codice sta in **0000-05C5 (circa 1,5 KB)**; da 05C6 a 385E (circa 12,6 KB)
ci sono tabelle e sequenze: la tabella dei puntatori dei comandi a 0AF3, le
sequenze dalla 0B73; poi il riempimento. Nessun byte oltre 05C5 viene mai
eseguito. L'avvio (0000-0001, 0072-009D, 00A4-00D5) è coperto dalla corsa
`avvio` con traccia dal frame 0.

## Stato del sequencer in RAM

- **4000-423F: 18 blocchi da 32 byte** (zone scritte a passo 0x20: in ogni
  blocco i byte 00-0C/0E e 12-1A/1C). Attesa, non misurata: un blocco per
  voce/canale del sequencer (18 = 2 × 9 canali dei tre chip?).
- **4240-4243**: stato globale; 4243 è la casella del comando (NMI).
- **47DE-47FF**: pila (`sp = 4800h`); 47FC-47FF sono i byte più scritti.
- 46E0: un byte isolato.

## Stima dei lotti di traduzione

Nel principale un lotto ha tradotto in media ~10 routine (116 in 11 brief).
L'audio ha 9 routine da call/rst più i gestori dei comandi di sequenza
raggiunti da `jp (hl)` a 0216 (da censire: servono i PC dopo 0216 nella
traccia). Proposta: **3 lotti**.
1. Ossatura: avvio, IRQ `0038`, NMI `0066`, spedidore `00E4`, `rst 8/10/18`,
   scrittura dei registri AY per interruzione; banco di prova sull'oracolo.
2. Interprete delle sequenze: `0147`, `029E`, `036B` e i gestori di `0216`.
3. Dati: tabella 0AF3 e sequenze 0B73-385E estratte come dati (da ROM a
   tempo di costruzione, non nel repo), più i codici lunghi in ciclo.
   **Fatto (brief 21)**: la regione 05C6-3FFF è in `harness/audio_dati_*.c`,
   generata da `estrai_dati.py`; i comandi sono descritti in `SPARTITI.md`
   (sezione «Dati estratti» in fondo).

Difficoltà: poco codice ma molto stato (18 blocchi voce) e un interprete
guidato dai dati con smistamento indiretto; l'errore si vede solo nella
sequenza temporale dei registri AY, quindi il confronto va fatto frame per
frame sull'oracolo, non su catture di registri a una chiamata.

## Uso dell'oracolo `_ay.txt`

`out/audio/<nome>_ay.txt` ha una riga `frame chip reg val` (esadecimali,
chip 1-3) solo quando un registro cambia, e `# codice <frame> <cc>` a ogni
comando forzato. Una traduzione futura, pilotata con gli stessi codici agli
stessi frame (`AUDIO_CODICI`, `AUDIO_DA`, `AUDIO_PASSO` di `audio.lua`) e
con le stesse interruzioni per frame, deve produrre lo stesso file: il
confronto è un `diff`. Le porte 12/13 vanno tenute fuori dal confronto.

## Ossatura tradotta (brief 19)

File in `harness/` (bersaglio `make -C harness audio_banco`, prefisso
`aud_` per non urtare i nomi del principale):

- `audio.h`: modello di memoria (`aud_ram`, `aud_rom`, `AM()`), indirizzi e
  campi del blocco con nome, firme.
- `audio_piattaforma.c`: caricamento di `1.3jk` a tempo di esecuzione,
  `aud_leggi`/`aud_scrivi`, `aud_out` con la semantica di `audio.lua`
  (selezione, ombra, porte ignote), scrittura del flusso dei cambi.
- `audio_ossatura.c`: avvio `0072` (senza il ciclo infinito), `00A4` con
  l'errore dell'originale, IRQ `0038`, NMI `0066`, `00E4` con `00D6` e
  `00FA`, `rst 8/10h/18h`.
- `audio_registri.c`: `029E` e `036B` con tutti i rami (`02DD`, `02E5`,
  `030B`, `0381`). L'inviluppo hardware è selezionato dai bit 5-6 del flag.
- `audio_interprete.c`: `aud_0147` vuoto nel brief 19, tradotto nel brief 20 (sotto).
- `audio_banco.c`: `./harness/audio_banco <nome> [frame_max]` da `decomp/`.

Modello del tempo del banco: al frame 0 l'avvio; per ogni frame i latch
del frame (NMI; quelli prima del frame 14 ignorati con avviso), poi 2 volte
IRQ e una passata di `029E`. Confronta la sequenza (chip, reg, val) dei
cambi dei frame 0-13 e lo stato dei registri a fine frame dal 13 in poi.
Limite: l'ordine dei cambi dentro un frame dipende da dove l'IRQ
interrompe `029E` e non è modellato (nel codice 00 dell'oracolo il chip 1
compare dopo i chip 2 e 3), quindi non si confronta riga per riga.

Tratti che coincidono: avvio (85 cambi, sequenza uguale), attesa fino al
primo codice in tutti i 43 oracoli, codice 00 (`cod_00_07` fino al 209),
codici 41-FF (`cod_41_ff` intero), `gioco` fino al 1187. Dal codice 01 in
poi serve l'interprete: `cod_00_07` intero diverge al frame 211.

Il flusso `out/audio/banco/<nome>_ay.txt` è scritto per intero nel formato
di `audio.lua` (intestazione, righe `frame chip reg val`, `# codice`) ed è
l'ingresso del renderizzatore `.wav` `harness/audio_wav` (brief 22). Scarto
noto: i cambi dell'avvio nel C portano tutti il frame 0 (in MAME 0-13).
Rimando: `AUDIO-SEQUENZE.md` per l'interprete.

## Interprete tradotto (brief 20)

- `harness/audio_interprete.c` (438 righe): `aud_0147` e, interne, il
  glissando `01B2`, l'inviluppo software `0172`, la lettura `01E8/01EF`,
  la nota `0217`, `024A`, `0250` e i 16 gestori `03EE-05A6`, smistati da
  una tabella C controllata contro le parole della ROM a 03CE (`exit(2)`
  se diverse). Il registro DE si porta in una variabile: conta solo in
  `0286` (pausa col bit 6). Al blocco 0 vale 0: in MAME è quello del
  codice interrotto dall'IRQ, non modellabile. Letture e campi in
  `AUDIO-SEQUENZE.md`.
- `audio.h`: campi `BL_INV_CONT`, `BL_INV_PUNT`, `BL_GLISSA`,
  `BL_DUR_NOTA`, `BL_INV_INIZIO`, `BL_RIP_PUNT`, `BL_RIP_CONT`, `BL_RIT`,
  tabelle `AUD_TAB_*`, bit del flag.
- Foto della RAM: `sh audio.sh foto` (fuori da `tutto`, ~3 s) forza i
  codici 01-40 con `AUDIO_FOTO=1`; `audio.lua` scrive la RAM 4000-4243 a
  fine del frame prima di ogni codice e di quello del codice in
  `out/audio/foto_01_40_foto.txt`. `audio_banco` la confronta se il file
  esiste: **127 foto su 127 uguali** (la 128ª è del comando 40, escluso),
  quindi `00FA` e il primo passo di 0147 coincidono con MAME byte per byte.
- Comando 40: escluso (scelta B del brief 20). Il banco ferma il confronto
  al frame prima del primo latch 40 e lo dice in una riga; `cod_40` e
  `lungo_40` coincidono fino al 119.

Registro 6 inudibile: il banco non confronta il registro 6 (periodo del
rumore) di un chip quando il rumore è spento su tutti i suoi canali (bit
3-5 del registro 7 a 1) sia nell'oracolo sia nel C. Motivo: in MAME l'IRQ
che legge `F1 0F F9 00 0C` (rumore 0 e pausa) può interrompere `029E` dopo
la lettura del flag della voce e prima di quella di +0A; il resto della
passata scrive il registro 6 = 00 su una voce che poi è muta e non lo
riscrive più. Il valore resta nel chip ma non si sente. Casi: `cod_10_17`
523, `cod_18_1f` 217, `lungo_14` 219, `lungo_16` 410, `lungo_19` 289,
`lungo_1C` 173, `lungo_26` 126, `lungo_28` 123 (ora confronti pieni).

Banco su tutti gli oracoli: **39 su 44** con 0 frame diversi per intera
durata (`cod_40` e `lungo_40` fino al 119). Le 5 eccezioni sono fallimenti
attesi del modello del tempo (IRQ e passata di `029E` atomici, latch a
inizio frame), non della traduzione; il primo frame diverso atteso è:

| oracolo | primo frame diverso | causa |
|---|---|---|
| `cod_08_0f` | 301 | passata di `029E` tagliata dalla fine del frame: toni e mixer del chip 3 un frame dopo |
| `cod_20_27` | 571 | come sopra, mixer e volumi del chip 1 |
| `cod_28_2f` | 752 | come sopra: due voci del chip 1 col rumore scrivono il registro 6 a turno (05 e 17); il frame finisce fra le due scritture |
| `cod_38_3f` | 751 | come sopra, mixer dei chip 1-2 (comando 3F) |
| `gioco` | 1189 | i latch del gioco cadono prima, fra o dopo i due IRQ del frame; `_latch.txt` registra solo il frame |
| `foto_01_40` (seconda riga) | 931 | come `cod_08_0f` (comando 0A); le foto RAM invece coincidono tutte (127) |

Per arrivare a 0 servirebbe contare i cicli di `029E`, `0147` e `00E4` e
sapere dove cade il latch del gioco nel frame: scelta di chiudere senza
modello a cicli (risoluzione aggiuntiva del brief 20).

## Dati estratti (brief 21)

- `estrai_dati.py` legge `1.3jk` e scrive la regione dati 05C6-3FFF come
  sorgente C in `harness/`: `audio_dati_tabelle.c` (riempimenti, periodi dei
  toni 0700/0760, durate 0790, i 16 inviluppi software 0850-0AF2, tabella dei
  comandi 0AF3, residuo 3811), `audio_dati_spartiti_a.c`-`_g.c` (elenchi e
  flussi dei comandi 01-3F), `audio_dati_indirizzi.h` (un `ADR_<nome>` per
  etichetta) e `audio_dati_gruppi.h`. Macro dei flussi e struttura dei pezzi
  in `audio_dati.h`, serializzatore in `audio_dati.c`. Il generatore controlla
  da sé che le macro espanse ridiano la ROM; rieseguirlo non cambia nulla.
- 478 pezzi a indirizzi ROM fissi, uno per etichetta (inizio di tabella,
  elenco, flusso di un record, bersaglio di F4/F7). Nomi dei flussi
  `cmd_NN_bK_sS` (comando, blocco, strato), ingressi interni `_p2`, `_p3`...;
  un blocco ripetuto nello stesso elenco prende `_r2` (comando 0E). Il flusso
  `cmd_1A_b2_s1` è usato anche dal comando 13.
- Prova: `harness/audio_immagine <file>` ricompone 05C6-3FFF (14906 byte)
  uguale alla ROM; `AUDIO_DATI=estratti ./harness/audio_banco <nome>` legge
  la regione dalle tabelle (riga `dati: estratti` su stderr) e dà gli stessi
  risultati e gli stessi flussi `_ay.txt` della ROM su tutti gli oracoli;
  `AUDIO_DATI=vuoto` (regione tutta AA) li fa divergere. Il codice del driver
  0000-05C5 resta letto dalla ROM.
- `SPARTITI.md`: per ogni comando voci, strati, durata nominale o ciclo,
  strumenti; tabella riassuntiva con la colonna «nome» (da `audio_nomi.tsv`,
  brief 22). Comandi in ciclo: gli stessi 15 dei lunghi (3D dura 1800
  frame nominali senza ciclo).

## Renderizzatore (brief 22)

- `harness/audio_wav <flusso_ay.txt> <uscita.wav> [da [a]]`: tre YM2149
  emulati con emu2149 (Mitsutaka Okazaki, MIT, commit `441f3dc`, in
  `harness/emu2149/` non modificato; vedi `NOTICE.md`), clock 1 500 000 Hz,
  tabella dei volumi YM2149, convertitore di frequenza attivo. Uscita
  44100 Hz, 16 bit, mono: 735 campioni per frame a 60 Hz, somma dei tre chip
  × 7/8 (sulla scheda i 9 canali vanno all'altoparlante mono con lo stesso
  peso 0,12). `da` predefinito = primo `# codice` (se non c'è 14), `a` =
  ultima riga di dati.
- A ogni frame lo stato del flusso a fine frame si scrive nel chip solo dove
  cambia. Registro 6 (periodo del rumore): si scrive solo con il rumore
  acceso su almeno un canale del chip (`(r7 & 0x38) != 0x38`), lo stesso
  criterio del banco: rende uguali C e MAME quando il periodo cambia a
  rumore spento e sposta solo la fase del generatore di rumore, inudibile.
- Limiti: i cambi si applicano a confine di frame (il tempo dentro il frame
  si perde, allo stesso modo per C e MAME); una riscrittura del registro 0D
  con lo stesso valore, che sul chip riavvia l'inviluppo, non compare nei
  flussi e non si sente.
- `harness/audio_suona <cc> <frame_fine> <uscita_ay.txt>`: un comando dal C,
  stesso ciclo del banco senza oracolo, codice al frame 120; il flusso è
  identico byte per byte a `out/audio/banco/lungo_<cc>_ay.txt`.
- `sh audio_wav.sh codici`: wav dei comandi 01-3F in `out/wav/codici/`, da
  120 all'ultimo cambio + 30 frame, al massimo il 2000 (i 15 comandi in
  ciclo arrivano al tetto, 31,35 s; il 3D al 1951). Esclusi 00 (arresto),
  40 (fuori modello, mai inviato dal gioco), 41-FF (ignorati, nessun
  cambio). I nomi dei suoni si scrivono in `audio_nomi.tsv` ascoltandoli e
  finiscono in `SPARTITI.md` con `python3 estrai_dati.py`.
- `sh audio_wav.sh oracoli`: per i 44 oracoli (non `foto_01_40`) rigenera il
  flusso del C, rende oracolo e C dal frame 14 in
  `out/wav/oracoli/<nome>_mame.wav` / `<nome>_c.wav` e li confronta con
  `cmp`. Esito: **40 uguali, 4 diversi**. `cod_38_3f` risulta uguale: il
  suo frame diverso (751) non cambia i campioni fino al 752, ultimo reso.
  L'ascolto finale si fa sulle coppie `_mame.wav` / `_c.wav`.

Le 4 eccezioni (`out/wav/eccezioni.txt`, `audio_wav_confronto.py c mame`;
lo scarto dopo il primo frame diverso resta per la fase di toni e
inviluppi):

| oracolo | campioni diversi | primo frame | ultimo frame | frame diversi | scarto massimo | rms differenza |
|---|---|---|---|---|---|---|
| `cod_08_0f` | 106937 / 607110 | 301 | 839 | 197 | 7140 | −33,1 dBFS |
| `cod_20_27` | 1113 / 574770 | 571 | 662 | 4 | 3570 | −49,3 dBFS |
| `cod_28_2f` | 42622 / 607110 | 752 | 839 | 86 | 4830 | −40,4 dBFS |
| `gioco` | 341807 / 1606710 | 1305 | 2199 | 592 | 8174 | −37,0 dBFS |

Il giudizio sulle eccezioni (ed eventualmente un modello a cicli di
`029E/0147/00E4`) è un compito a parte, dopo l'ascolto.
