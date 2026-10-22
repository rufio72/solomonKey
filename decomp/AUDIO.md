# Z80 audio: osservazione e pianificazione della traduzione

Decisione del 2026-10-22: **il Z80 audio sarà tradotto** per intero (niente
emulazione né campioni). Questo rapporto non è un via/stop: serve a
pianificare i lotti di traduzione. Prodotto dal brief 18; nessuna riga
tradotta finora. Indirizzi e conteggi, nessun byte della ROM.

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
al gioco normale; sono scritture del solo avvio (stessi conteggi con 90 o 900
frame), non del ciclo per interruzione. La traduzione può ometterle o
lasciarle a una funzione di piattaforma vuota.

## Flusso del comando

- Scrittura a e800 dal principale → NMI sull'audio. NMI `0066` legge 8000,
  incrementa, salva in **4243**, `retn`.
- Avvio `0072`: `sp = 4800h`, `im 1`; IRQ su `0038` (circa 6-7 per frame).
- Spedidore `00E4`: legge 4243 (0 = niente), lo azzera, decrementa: codice
  00 → `00d6` (arresto), 01-40 → `rst 8`/`rst 10h` sulla tabella a **0AF3**
  (voce del codice v a 0AF3+2v); 41-FE ignorati (`cp 041h; ret nc`), FF
  diventa 0. Confermato: i codici 41-FF producono **0 cambi AY** e 0 PC
  nuovi (verifica 5 del brief).
- Codici inviati dal gioco nella corsa `gioco` (frame 0-2200): 29 scritture
  e800, tutte `gioco` in `out/audio/gioco_latch.txt`.

## Numeri (rifarli: `sh audio.sh > out/audio/audio.log 2>&1; python3 audio_numeri.py`)

Unione di 43 corse (9 gruppi `cod_*` 00-40, `cod_41_ff`, 32 `lungo_*`,
`gioco`); main sospeso con `suspend 0` tranne in `gioco`.

- PC distinti eseguiti: **667** (max 05C3, nessuno ≥ 4000).
- Byte di ROM eseguiti: **1170 su 16384 (7,1%)**, mai eseguiti 15214;
  ultimo byte eseguito 05C5; byte eseguiti oltre 0AF3: 0.
- Intervalli codice 13, dati 14: `0008-000d 0010-0013 0018-0026 0038-004f
  0066-0071 009e-00a3 00d6-01ce 01e8-028a 029e-03c6 03ee-043d 0470-047d
  0485-0590 05a6-05c5`.
- Riempimento uniforme in fondo alla ROM: 385F-3FFF (1953 byte).
- Destinazioni di call/rst eseguite: 7; **routine (con 0038 e 0066): 9**:
  `0008 0010 0018 0038 0066 00E4 0147 029E 036B`.
- `rst` usati: `08=26930 10=68863 18=40599` esecuzioni; `jp (hl)` eseguiti
  14001, da un solo sito (`0216`): è lo smistamento dei comandi delle
  sequenze, i cui gestori non risultano come routine.
- Copertura delle call statiche: **unione 17 su 97 (17,5%)**. Delle 80 mai
  osservate, 79 hanno il sito di chiamata a 05C6 o sopra (dati letti come
  codice da z80dasm); l'unica con sito nel codice è `00A4` da `0090`, nel
  test di avvio, che parte prima della finestra di traccia (frame 120).
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
eseguito. Fuori dalle tracce resta solo l'avvio (0000-0007, 0072-009D, 00A4):
test della RAM e inizializzazione, facile da coprire con una traccia dal
frame 0.

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
