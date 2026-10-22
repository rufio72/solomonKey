# Compito 18: Z80 audio, infrastruttura di osservazione

Data: 2026-10-22. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Primo lotto sul Z80 audio (`:audiocpu`, ROM `1.3jk`, 16 KB, mai toccato):
costruire l'osservazione, **non tradurre nulla**. Quattro prodotti: tracce e
copertura delle destinazioni statiche di `call` sull'audio; oracolo delle
scritture ai tre YM2149; giro di copertura forzando il latch con tutti i
codici; rapporto `decomp/AUDIO.md`.

Decisione già presa (2026-10-22): **il Z80 audio sarà tradotto**; l'ipotesi
«resta emulato o campionato» è scartata. `AUDIO.md` non è un via/stop ma un
input di pianificazione: stima quanti lotti di traduzione serviranno e dove
sta la difficoltà (codice contro dati, stato del sequencer in RAM).

Vincoli assoluti:
- Modifiche solo in `decomp/`. Non toccare `roms/`, `compiti/`,
  `.gitignore`, `README.md`, `STATO.md`, `harness/`, `trace.lua`,
  `sessione.lua`, `snap.lua`, `sessioni.sh`, `cattura.sh`, `callgraph.py`.
  Nessuna operazione git.
- MAME `/usr/games/mame` 0.264 (`PATH=$PATH:/usr/games`), sempre
  `-video none -sound none -skip_gameinfo -nothrottle` e `SDL_VIDEODRIVER=dummy`;
  mai `-window`, `-video soft`, `-debugger imgui`; massimo 4 corse in
  parallelo (le corse di questo lotto vanno in serie); tutto in locale.
- Niente ROM né estratti di ROM nel repo (nemmeno dump esadecimali in
  `AUDIO.md`: indirizzi e conteggi sì, byte della ROM no). Tracce grezze
  cancellate subito dopo l'elaborazione; a fine lavoro nessun file sopra
  50 MB nel repo.
- I prodotti dell'audio vanno in `decomp/out/audio/` (già ignorata da
  `.gitignore` con `decomp/out/*`), **mai** in `decomp/out/*_eseguiti.txt`:
  il cancello del Z80 principale usa il glob `out/*_eseguiti.txt`.
- Commenti, messaggi e documenti in italiano; nessun file di codice sopra
  le 800 righe.

## Cosa esiste già (fatti verificati)

### Hardware audio (misurato oggi con Lua in MAME 0.264, `space.map.entries`)

| CPU / spazio | intervallo | cosa |
|---|---|---|
| audiocpu program | 0000-3fff | ROM 1.3jk |
| audiocpu program | 4000-47ff | RAM (2 KB) |
| audiocpu program | 8000 | lettura del latch (`:soundlatch`, generic_latch_8) |
| audiocpu program | ffff | scrittura nop (l'avvio scrive 0x10 a ffff, 0079) |
| audiocpu io (mask ff) | 10-11, 20-21, 30-31 | scrittura: tre YM2149 `:ay1` `:ay2` `:ay3` a 1,5 MHz |
| maincpu program | e800 | scrittura del comando audio |

- `mame -listdevices solomon`: audiocpu Z80 3,07 MHz; ay1/ay2/ay3 «YM2149
  SSG @ 1.50 MHz»; soundlatch generic_latch_8.
- Porta pari = registro indirizzo, dispari = dato (convenzione
  `address_data_w`; l'associazione 10→ay1, 20→ay2, 30→ay3 segue l'ordine
  della mappa: attesa, non misurata, non conta per il lotto).
- Misurato (tap di scrittura su io, frame 0-2200 di gioco normale):
  `10=73554 11=73618 12=128 13=64 20=67152 21=67152 30=74495 31=74495`:
  il driver scrive anche alle porte **12 e 13, non mappate** (MAME le
  ignora). L'oracolo le conta a parte.
- Circa 150 scritture io per frame in ogni stato (il driver riscrive i
  registri a ogni interruzione): l'oracolo deve registrare solo i cambi.

### Flusso del comando (da dasm di 1.3jk e tracce misurate oggi)

- Una scrittura a e800 dal lato principale provoca una NMI sull'audio:
  misurato 16 scritture e800 = 16 letture di 8000; in una corsa a 8 codici
  forzati, 8 ingressi a `0066` nella traccia.
- NMI `0066`: `push af; ld a,(8000h); inc a; ld (4243h),a; pop af; ei; retn`.
- Avvio `0072`: `ld sp,4800h`, `im 1`. IRQ 0 su `0038` (modo 1): 400
  interruzioni in una finestra tracciata di 60 frame.
- Spedidore `00E4`: legge 4243, se 0 esce; azzera 4243; `dec c`: codice
  latch 00 → `l00d6h` (presumibilmente arresto); codici 01-40 → `rst 8`
  e `rst 10h` sulla tabella di puntatori a **0AF3** (voce del codice v a
  0AF3+2v, prima voce utile 0B73); `cp 041h; ret nc` → **codici 41-FE
  ignorati**; FF diventa 0 (nessun comando). Scrittori di 4243: 006B,
  00EB, 0592.
- Codici inviati dal gioco (tap su e800, frame 0-1900 con moneta 1510 e
  start 1560): `3f 38 36 34 35 06 18 3e 3f 38 36 34 35 1a 06`.
- `mem:write_u8(0xe800, c)` da Lua su `manager.machine.devices[":maincpu"].spaces["program"]`
  passa dal gestore vero (latch + NMI): misurato.
- Il comando del debugger `suspend 0` ferma il Z80 principale: misurato
  0 scritture e800 dal gioco dopo il frame 100. Così i codici forzati non
  si mescolano a quelli del gioco.
- Traccia del solo audio: `trace <file>,:audiocpu,noloop` (formato righe
  `PPPP: istruzione` come il principale, più righe
  `   (interrupted at XXXX, IRQ 0)` che la regex di `coverage.py` scarta).

### Misure di copertura (oggi, script di prova fuori dal repo)

- `z80dasm -a -l -g 0 ../roms/ext/solomon/1.3jk`: 218 righe `call`, 106
  destinazioni distinte con la regex `CALL` di `copertura_call.py`, **97
  sotto 4000** (le altre 9 sono dati letti come codice).
- Corsa unica, main sospeso al 100, codici 00-40 uno ogni 90 frame dal 120
  (fine al 5970), traccia audio: 645 MB, 84 s; **667 PC distinti, max
  05C3, nessuno ≥ 4000; 17 destinazioni su 97 osservate**.
- Corsa codici 41-FF ogni 2 frame: 42 MB, 85 PC, tutti già nella corsa
  00-40 (0 PC nuovi).
- Gruppo di 8 codici (01-08) × 90 frame: traccia 78 MB, 11 s.
- RAM audio: 458 byte distinti scritti fra il frame 110 e il 5970 con i
  codici 00-40 (il test di avvio scrive tutti i 2048).

### Script esistenti da estendere (da `grafo outline` e lettura)

- `coverage.py` (232 righe): `principale()` riga 79; costanti globali
  `NMI = 0x0066`, `ROM = [(0x0000, 0xBFFF), (0xF000, 0xFFFF)]`; opzioni
  `--traccia --rom (out/main64.bin) --cancella --prefisso`;
  `assert len(rom) == 0x10000`; `in_rom(a)` riga 66 usa `ROM`;
  `lunghezza(rom, pc)` riga 48 indicizza `rom[pc]` (serve un'immagine da
  64 KB); conta `PC >= c000`; `uscita(nome)` = `os.path.join('out', pre + nome)`
  e crea solo `out/`. Uscite: `_eseguiti _istruzioni _chiamate _copertura _numeri`.
- `copertura_call.py` (91 righe): `destinazioni(dasm)` scarta `d >= 0xC000`;
  `UNIONE = 'out/eseguiti_unione.txt'` e `out/call_mai_osservate.txt`
  fissi; opzioni `eseguiti... --dasm (out/main48.dasm) --classifica`;
  stampa `unione: N su T (P%)` se legge più di un file.
- `disasm.sh` (6 righe): due righe `z80dasm -a -l -g 0` in `out/`.
- `trace.lua` (38 righe): modello di traccia (`dbg:command("trace ...")`,
  `register_frame_done`, `manager.machine:exit()`); resta intatto.
- `sessioni.sh` (132 righe): modello di driver shell (`MOPT`, funzione
  `sessione`, `coverage.py ... --cancella`, `ERRORE:` se la traccia manca).
- Stato del principale (esito del brief 16): `unione: 235 su 299 (78.6%)`.

## Struttura da realizzare

Tutto in `/home/rufio72/OpenSolomonsKey-master/decomp/`.

1. **`disasm.sh`**: una riga in più
   `z80dasm -a -l -g 0 ../roms/ext/solomon/1.3jk > out/audio.dasm`.
2. **`coverage.py`**: opzione `--cpu {main,audio}` (predefinito `main`, uscite
   del principale identiche a oggi). Con `audio`: `--rom` predefinito
   `../roms/ext/solomon/1.3jk`, assert 0x4000 byte, immagine riempita a
   0x10000 con zeri in memoria; `ROM = [(0x0000, 0x3FFF)]` (renderla
   variabile del modulo impostata da `principale`); il contatore RAM conta i
   PC in 4000-47ff con etichetta `PC in RAM 4000-47ff`. `uscita()` crea la
   cartella del file (`os.makedirs(os.path.dirname(...), exist_ok=True)`),
   così `--prefisso audio/cod_00_07` scrive in `out/audio/`.
3. **`copertura_call.py`**: opzioni `--limite` (esadecimale, predefinito
   `0xC000`; audio `0x4000`) e `--uscite` (cartella, predefinito `out`) per
   `eseguiti_unione.txt` e `call_mai_osservate.txt`; il salto del file
   unione in ingresso usa il percorso nuovo.
4. **`audio.lua`** (nuovo, sotto 200 righe), variabili d'ambiente documentate
   in testa:
   - `AUDIO_NOME` (prefisso delle uscite in `out/audio/`);
   - `AUDIO_CODICI` elenco esadecimale con intervalli (`00-07`, `41-ff`,
     `05,1a`); `AUDIO_DA` (frame del primo codice, predefinito 120);
     `AUDIO_PASSO` (frame per codice, predefinito 90);
   - `AUDIO_GIOCO=1`: niente `suspend`, moneta/start ai frame 1510-1580 e
     input casuali dal 1900 come `trace.lua`; altrimenti `suspend 0` al
     frame 100;
   - `AUDIO_TRACE_DA`/`AUDIO_TRACE_A` (vuoti = nessuna traccia):
     `trace out/audio/<nome>_trace.txt,:audiocpu,noloop`;
   - `AUDIO_FINE` frame di uscita (predefinito: ultimo codice + passo).
   Comandi: `main:write_u8(0xe800, c)`, stampa `CODICE <frame> <cc>`.
   Oracolo: tap di scrittura su io 00-ff dell'audiocpu; porta pari =
   registro selezionato del chip `(porta >> 4)`, dispari = dato; copia
   ombra per chip e registro; scrive `out/audio/<nome>_ay.txt` con
   intestazione `# frame\tchip\treg\tval` e una riga solo quando il valore
   cambia (esadecimali, chip 1-3), più righe `# codice <frame> <cc>` ai
   cambi di codice; le scritture a porte fuori da 10/11/20/21/30/31 vanno
   contate e stampate a fine corsa (`PORTE IGNOTE 12=n 13=n`). Tap di
   scrittura e800 sul principale: `out/audio/<nome>_latch.txt`
   (`frame valore origine`, origine `gioco|forzato`). Tap di scrittura su
   4000-47ff dell'audio dal frame 110: `out/audio/<nome>_ram.txt` (indirizzo,
   scritture, primo e ultimo frame). I tap in variabili globali (come
   `sessione.lua`).
5. **`audio.sh`** (nuovo, sotto 150 righe, modello `sessioni.sh`): funzione
   `corsa <nome>`; dopo MAME, se c'è la traccia,
   `coverage.py --cpu audio --traccia out/audio/<nome>_trace.txt --prefisso audio/<nome> --cancella`,
   altrimenti `ERRORE:` se era attesa. Blocchi:
   - `codici`: nove corse `cod_00_07` … `cod_38_3f`, `cod_40`, ognuna con
     traccia sull'intera finestra (8 × 90 frame ≈ 78 MB, cancellata);
   - `ignorati`: `cod_41_ff`, passo 2, traccia;
   - `lunghi`: per ogni codice il cui `_ay.txt` ha cambi negli ultimi 10
     frame della sua finestra, corsa singola `lungo_<cc>` con passo 900 e
     traccia (attenzione: ~100 MB per corsa, cancellata subito);
   - `gioco`: `AUDIO_GIOCO=1`, traccia 1500-2200, fine 2200;
   - in coda: `copertura_call.py --dasm out/audio.dasm --limite 0x4000 --uscite out/audio --classifica out/audio/*_eseguiti.txt`
     (esclusa l'unione).
   Senza argomento tutti i blocchi in ordine; con un argomento solo quello.
6. **`audio_numeri.py`** (nuovo, sotto 200 righe): dai file di `out/audio/`
   stampa i numeri di `AUDIO.md`: byte di ROM eseguiti e non (unione dei
   `_istruzioni.txt` con `lunghezza` di `coverage.py`), intervalli codice/
   dati, destinazioni di call/rst eseguite (routine) da `_chiamate.txt` più
   0038 e 0066, PC distinti, `rst` usati, RAM scritta (byte distinti
   dall'unione dei `_ram.txt`, i più scritti), codici 00-40 che producono
   scritture AY e quanti cambi ciascuno, codici «lunghi».
7. **`AUDIO.md`** (nuovo, 60-120 righe): hardware e flusso del comando
   (fatti sopra, verificati); numeri di `audio_numeri.py` con il comando
   per rifarli; quanto della ROM è codice e quanto dati (atteso: codice in
   0000-05C3 circa, il resto tabelle e sequenze dalla 0AF3); quante
   routine; quanto stato tiene il sequencer in RAM e dove; le porte 12/13;
   **stima dei lotti di traduzione** (routine per lotto come nei lotti del
   principale) e dove sta la difficoltà; come si usa l'oracolo `_ay.txt`
   per confrontare una traduzione futura. Annotare in testa la decisione:
   l'audio sarà tradotto, il rapporto serve alla pianificazione.

## Verifiche obbligatorie (da `decomp/`)

1. `sh disasm.sh && grep -cP '^\tcall ' out/audio.dasm` → `218` (misurato).
2. `sh audio.sh > out/audio/audio.log 2>&1` (creare prima `out/audio`)
   termina senza righe `ERRORE:`; durata attesa 3-6 minuti (misurato 84 s
   per tutti i codici in una corsa; attesa, non misurata a gruppi).
3. `tail -1` dell'uscita di `copertura_call.py` nel log →
   `unione: N su 97 (P%)` con N ≥ 17 (misurato 17 su una corsa unica
   00-40; a gruppi e con `gioco`/`lunghi` può salire).
4. `cat out/audio/*_eseguiti.txt | grep -v unione | sort -u | wc -l` → ≥ 667
   (misurato 667 sulla corsa unica); nessun PC ≥ 4000 (misurato).
5. PC di `cod_41_ff_eseguiti.txt` assenti dall'unione dei `cod_0*`/`cod_[1-3]*`/
   `cod_40` → `0` (misurato 0).
6. `grep -c '^CODICE' out/audio/audio.log` → `256` (65 + 191), più una per
   ogni corsa `lungo_` (conteggio dei codici del piano, non misurato).
7. `grep -h 'PORTE IGNOTE' out/audio/audio.log | head -1` → contiene `12=`
   e `13=` (attesa, non misurata con i codici forzati; misurata in gioco).
8. Regressione del principale:
   `python3 copertura_call.py $(ls out/*_eseguiti.txt | grep -v unione) | tail -1`
   → `unione: 235 su 299 (78.6%)` (esito del brief 16) e
   `ls out/*_eseguiti.txt | grep -c audio` → `0`.
9. `python3 audio_numeri.py` termina senza errori e i suoi numeri compaiono
   identici in `AUDIO.md`; RAM scritta distinta ≥ 400 byte (misurato 458
   sulla corsa unica, attesa simile).
10. `wc -c out/audio/*_ay.txt | tail -1` → sotto 50 MB (attesa, non misurata:
    solo cambi).
11. `ls out/audio/*trace*.txt 2>/dev/null` → vuoto; `find .. -size +50M` →
    vuoto; `pgrep -c mame` → `0`.
12. `wc -l *.lua *.py *.sh | awk '$1>800 && $2!="totale" && $2!="total"'` →
    vuoto.
13. `grep -n -e '-window' -e 'video soft' -e imgui audio.sh audio.lua` → vuoto.
14. `git -C .. status --short` → solo `decomp/` (`disasm.sh`, `coverage.py`,
    `copertura_call.py`, `audio.lua`, `audio.sh`, `audio_numeri.py`,
    `AUDIO.md`) e questo brief; `git -C .. status --short --ignored | grep -i 3jk`
    → vuoto fuori da `roms/`.

## Consegna

- I file del punto «Struttura»; in `out/audio/` i derivati (nessuna traccia
  grezza). Nessun commit (lo fa la chat principale); se richiesto,
  messaggio in italiano con riga `Co-Authored-By: <modello> <noreply@anthropic.com>`.
- Rapporto finale: massimo trenta righe con l'uscita delle verifiche 1-14,
  copertura audio (N su 97), byte codice/dati, routine, RAM del sequencer,
  codici lunghi, stima dei lotti, deviazioni, tempo MAME speso.

## Domande aperte

1. «preferenza» Script nuovo `audio.lua` invece di adattare `trace.lua`
   (che resta lo strumento storico del principale). Proposta: nuovo.
2. «preferenza» Main sospeso (`suspend 0`) nelle corse forzate, così
   l'oracolo vede solo il codice forzato; il comportamento in gioco è
   coperto dalla corsa `gioco`. Proposta: sì.
3. «preferenza» Gruppi di 8 codici × 90 frame (tracce ~78 MB temporanee)
   più le corse `lunghi` a 900 frame. Proposta: così.
4. «preferenza» Oracolo solo sui cambi di registro, non su ogni scrittura
   (~150 scritture per frame ripetitive). Proposta: solo cambi.
5. «preferenza» Uscite in `out/audio/` e opzioni `--cpu`/`--limite`/`--uscite`
   sugli script esistenti invece di copie `audio_coverage.py`. Proposta: opzioni.

## Risoluzioni (2026-10-22)

Tutte le domande sono preferenze, accettate con la proposta dell'analista:
1. Script nuovo `audio.lua`; `trace.lua` resta intatto.
2. Z80 principale fermo con `suspend 0` del debugger durante le corse
   forzate.
3. Gruppi di 8 codici da 90 frame ciascuno, più corse lunghe da 900 frame
   per i suoni che non finiscono in tempo.
4. L'oracolo registra solo i cambi di registro, non ogni scrittura.
5. Opzioni sugli script esistenti (`--cpu audio`, `--limite`, `--uscite`)
   invece di copiarli.

Contesto deciso dall'utente (già in testa al brief): l'audio va tradotto
per intero, niente emulazione né campioni; `AUDIO.md` è pianificazione
dei lotti di traduzione.

## Esito (2026-10-22): PASSATO

Ciclo in locale: analista Opus ~72k token, sviluppatore Opus ~75k,
verificatore Sonnet ~21k. Tutte le 14 verifiche passate due volte; il
verificatore ha rifatto anche la corsa completa di `audio.sh` (10 min 16 s,
0 errori). Riserva solo di formato sulla verifica 9: i cambi AY per codice
e le corse lunghe non compaiono in `AUDIO.md` nel formato stampato da
`audio_numeri.py`.

Risultati chiave per i lotti di traduzione (dettagli in `decomp/AUDIO.md`):
- codice in 0000-05C5 (~1,5 KB, 1170 byte eseguiti su 16384, 13
  intervalli); da 05C6 a 385E dati, poi riempimento;
- 9 routine vere da call/rst più i gestori del salto indiretto `jp (hl)`
  a 0216 (smistatore dei comandi di sequenza, da censire);
- 79 delle 80 call mai osservate sono dati letti come codice; l'unica
  vera è 00A4 (test di avvio, prima dell'inizio delle tracce);
- RAM del sequencer: 18 blocchi da 32 byte in 4000-423F (ipotesi voce
  per blocco, non misurata), stato globale 4240-4243, pila 47DE-47FF;
- 16 codici restano attivi dopo 900 frame (musiche in ciclo); il
  criterio dei «lunghi» ha falsi positivi (17, 2E, 3E da soli finiscono
  in un frame).

Stima dello sviluppatore: 3 lotti di traduzione (ossatura e scrittura dei
registri; interprete delle sequenze; estrazione dei dati).

Da sapere: la corsa completa dura ~10,5 minuti, non 3-6 (32 corse lunghe
da ~15 s); l'avvio dell'audio (0072-009D, con la call 00A4) resta fuori
dalle tracce — serve una corsa con traccia dal frame 0 in un lotto futuro;
`audio.lua` registra il registro selezionato col byte intero, senza
mascherare i 4 bit bassi.
