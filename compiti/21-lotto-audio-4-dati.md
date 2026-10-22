# Compito 21: Z80 audio, estrazione dei dati musicali

Data: 2026-10-22. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Trasformare la regione dati della ROM audio `1.3jk` (`05C6-3FFF`) in
sorgente C strutturato e leggibile (tabelle, inviluppi, tabella dei comandi,
sequenze con macro parlanti), generato da uno script Python che legge la
ROM; descrivere i comandi 01-3F in `decomp/SPARTITI.md`; provare che le
tabelle riserializzate ricostruiscono la regione byte per byte; far suonare
il banco `audio_banco` dalle tabelle estratte con risultato identico.

Vincoli assoluti:
- Modifiche solo in `decomp/` più, in `NOTICE.md` (radice), una frase che
  cita le nuove tabelle (vedi Domande aperte 5). Non toccare `roms/`,
  `compiti/`, `README.md`, `STATO.md`, `.gitignore`, i sorgenti C del
  principale, `audio.lua`, `audio.sh`, `audio_numeri.py`, `audio_registri.c`,
  `audio_interprete.c`, `audio_ossatura.c`. Nessuna operazione git (il
  commit lo fa la chat principale).
- Gli oracoli in `out/audio/` non si rigenerano: niente `sh audio.sh`, in
  nessuna forma (il blocco `avvio` fa `rm -f out/audio/avvio_*` e cancella
  il riferimento: non usarlo). Nessuna corsa MAME in questo compito.
- Nel repo entrano **solo testo C/Python/Markdown**: niente file `.bin`,
  dump o immagini della ROM. Le tabelle C derivate sì (il progetto le
  ammette, `NOTICE.md` punto 1). Il confronto con la ROM si fa su file
  temporanei in una cartella `mktemp -d` fuori dal repo, cancellata alla
  fine. `SPARTITI.md` descrive a parole e con numeri: niente blocchi di
  byte esadecimali.
- Il driver (codice `0000-05C5`, tabella dei gestori `03CE`) continua a
  essere letto dalla ROM caricata a tempo di esecuzione; si estrae solo
  `05C6-3FFF`.
- `gcc` 13.3 con le `CFLAGS` del `Makefile` (`-std=c11 -Wall -Wextra -O0`);
  nessun warning nei file audio. Python 3 standard, senza pacchetti nuovi.
- Commenti, messaggi e documenti in italiano; **nessun file di codice
  sopra le 800 righe** (obiettivo pratico: sotto le 600).
- Tutto in locale; nessuna traccia lasciata (file temporanei cancellati).

## Cosa esiste già (fatti verificati)

### Codice (da `grafo` e dai file)

| File | Righe | Ruolo |
|---|---|---|
| `harness/audio.h` | 95 | `aud_rom[0x4000]`, `aud_ram`, `AM(a)`, `AUD_TAB_*`, prototipi (`aud_carica_rom` 65, `aud_leggi` 68) |
| `harness/audio_piattaforma.c` | 138 | `aud_carica_rom` (28), `aud_azzera` (~46), `aud_leggi(uint16_t a)`: ROM sotto 4000, RAM 4000-47FF, latch a 8000, altrove 00; `aud_scrivi` |
| `harness/audio_interprete.c` | 438 | tutte le letture di sequenze e tabelle passano da `aud_leggi` (nessun accesso diretto a `aud_rom` fuori da `audio_piattaforma.c`; da grep) |
| `harness/audio_banco.c` | 270 | `main`: argomenti `<nome> [frame_max]`, chiama `aud_carica_rom(); aud_azzera();` (riga ~147); stdout letto con `sed -n 2p` nelle verifiche |
| `harness/Makefile` | 18 | `SORGENTI_AUDIO = audio_banco.c audio_piattaforma.c audio_ossatura.c audio_registri.c audio_interprete.c`; bersaglio `audio_banco: $(SORGENTI_AUDIO) audio.h` |
| `AUDIO.md` (223), `AUDIO-SEQUENZE.md` (131) | | semantica di blocchi, gestori F0-FF, tabelle; da aggiornare in fondo con una sezione «Dati estratti (brief 21)» |

ROM: `../roms/ext/solomon/1.3jk` (16384 byte, da `decomp/`; il banco la cerca
anche come `../../roms/...` da `decomp/harness/`). `0x5C6` = 1478.

### Mappa della regione dati (misurata sulla ROM con un parser di prova)

| Indirizzi | Contenuto |
|---|---|
| `05C6-06FF` | 314 byte, tutti FF (riempimento) |
| `0700-075F` | 48 parole: periodi del tono, indice < 30h (`AUD_TAB_TONI`) |
| `0760-078F` | 48 byte: periodi, indice >= 30h (`AUD_TAB_TONI_B`) |
| `0790-084F` | 96 parole: durate in IRQ (`AUD_TAB_DURATE`); indice = (byte & 7F) + (+16) |
| `0850-086F` | 16 parole: inizi degli inviluppi software (`AUD_TAB_INVILUPPI`), puntano a `0870, 0898, 08C0, 08C3, 08EB, 0913, 0933, 094B, 0973, 099B, 09BB, 09DB, 0A03, 0A2B, 0A6B, 0A93` |
| `0870-0AF2` | i 16 inviluppi (coppie di byte, terminazione FF come da `0172`/`0250` in `AUDIO-SEQUENZE.md`); lunghezza di ognuno = distanza dal puntatore successivo, l'ultimo finisce a `0AF2` |
| `0AF3-0B72` | tabella dei comandi: 64 parole, indice = valore scritto nel latch (01-3F; la parola 0 vale 0000; latch 00 ferma il driver via `00D6`) |
| `0B73-3810` | per ogni comando 01-3F: elenco di record da 3 byte `(blocco, parola indirizzo)` chiuso da `FF`, poi i flussi di sequenza. Il blocco è 0-17 (0-8 primo strato, 9-17 secondo strato; da `00FA`: >= 9 mette il bit 2 sul blocco − 9). Chip e voce: blocco k → chip `k/3 % 3 + 1`, voce `3 − k%3` |
| `3811-385E` | 78 byte **non raggiunti dalla tabella**: un elenco di 9 record (blocchi 0-8) a `3811-382C` e i flussi puntati a partire da `382D`, residuo di un comando mai collegato (nessuna voce della tabella lo raggiunge). Estrarre come pezzo «residuo_3811» |
| `385F-3FFF` | 1953 byte tutti FF (riempimento) |

Il comando 40 legge la parola di tabella a `0B73` (indice 40h), che si
sovrappone ai primi byte dell'elenco del comando 01 (voce `7A09`: fuori dalla
ROM, vedi `AUDIO-SEQUENZE.md`, sezione Comando 40). La tabella dei comandi
copre quindi 64 parole `0AF3-0B72`; la sovrapposizione si documenta, non si
rappresenta.

Lunghezze dei gestori nel flusso (da `AUDIO-SEQUENZE.md`, verificate con un
parser di prova che copre tutta la regione tranne FF di separazione e il
residuo): nota 00-7F e durata 80-EF 1 byte; F1 F2 F3 F9 FD 2; F4 F7 FC 3;
F5 2; F6 F8 FA FE FF 1; FB 7 (tre parole dopo il codice); F0: `F0 n` 2 byte,
`F0 FE forma` 3, `F0 FE forma FF` 4, `F0 FF forma periodo(2)` 5. F4 e F7
portano l'indirizzo del byte successivo da leggere (non il valore − 1).
Dopo F4, F8 e FF il flusso si interrompe; i byte `FF` isolati fra due
flussi (77, misurati) sono separatori.

Misure sulla ROM: 63 comandi (01-3F); record per comando: 1 (16 comandi), 2
(11), 3 (6), 4 (11), 5 (1), 9 (13), 14 (1), 18 (4); 382 indirizzi distinti
di ingresso (puntatori dei record e bersagli di F4/F7); 11345 dei 11500 byte
di `0B73-385E` sono flussi raggiungibili, 155 no (77 FF di separazione + i 78
byte del residuo). Nei flussi raggiungibili compaiono F0, F1, F2, F4, F5,
F6, F7, F8, F9, FA, FB, FC, FF; **mai** F3, FD, FE. Comandi con un ciclo
(F4 all'indietro): `13 14 16 19 1A 1B 1C 1E 1F 20 21 23 24 28 2A` (15, in
esadecimale; sono quasi tutti i «lunghi» di `AUDIO.md`: `3D` resta attivo 900
frame senza ciclo F4 trovato, probabilmente una sequenza lunga).

### Tempo (da `AUDIO-SEQUENZE.md`)

2 IRQ per frame. Una nota con parola di durata d (tabella `0790`) dura d
IRQ, cioè d/2 frame; FB scrive la durata senza − 1 (dura d + 1 IRQ). Il
cambio di `+16` (F3) non compare nei dati.

## Struttura da realizzare

Scelte fissate (il resto è libero, sotto «libero»):

1. **`decomp/estrai_dati.py`** (nuovo, generatore): legge la ROM (stessi
   percorsi di `audio_piattaforma.c`), decodifica la regione `05C6-3FFF` e
   scrive i file C sotto in `decomp/harness/`. Rieseguirlo non cambia nulla
   (idempotente). Sceglie lui i nomi: tabelle `tab_periodi_toni`,
   `tab_durate`, `inviluppo_NN`, comandi `cmd_13_...` con blocco e strato,
   punti d'ingresso interni `..._p2` ecc. Un ingresso = un'etichetta =
   un pezzo.
2. **Modello**: ogni dato è un *pezzo* a indirizzo originale fisso:
   ```c
   struct aud_pezzo { uint16_t indirizzo, lung; const uint8_t *dati;
                      const char *nome; };   /* dati NULL = riempimento FF */
   ```
   in `harness/audio_dati.h` (nuovo), con le macro dei flussi (sotto) e
   `void aud_dati_serializza(uint8_t img[0x4000]);`: parte da img piena di
   `0xAA` (nessuna copertura = byte avvelenato), copia ogni pezzo al suo
   indirizzo, `exit(2)` con messaggio se due pezzi si sovrappongono o se un
   byte di `05C6-3FFF` resta `0xAA`. Nessun pezzo sotto `05C6`.
3. **File C** (nomi indicativi): `audio_dati.c` (serializzatore + elenco dei
   gruppi), `audio_dati_tabelle.c` (riempimenti, toni, durate, inviluppi,
   tabella comandi, residuo), `audio_dati_spartiti_a.c`, `_b.c`, … (i
   flussi, divisi per intervallo di comandi, ognuno < 600 righe; ogni file
   esporta il suo array di pezzi e il numero), `audio_dati_indirizzi.h`
   (generato: un `#define ADR_<nome> 0x....` per ogni etichetta, usato dalle
   macro di puntatore). Righe dei flussi: massimo ~100 caratteri, un'unità
   logica per riga o poche affiancate.
4. **Macro dei flussi** (il formato che l'interprete conosce, byte esatti):
   nota `NOTA(ottava, DO|DOd|RE|…)` = `(ottava<<4)|nota` con pausa `PAUSA`
   (nibble basso C; la decodifica usa il byte, le altre pause vanno in
   `B(0x..)`), `DUR(i)` = `0x80|i`, un nome per ogni gestore
   (`ATTEN(x)` F1, `TRASP(x)` F2, `SALTA(ADR_..)` F4, `RIP_INIZIO(n)` F5,
   `RIP_FINE` F6, `CHIAMA(ADR_..)` F7, `RITORNA` F8, `RUMORE(x)` F9,
   `RUMORE_OFF` FA, `GLISSA(p0,p1,dur)` FB, `PERIODO(p)` FC, `FINE` FF,
   `STRUM(n)` e le due forme hardware di F0), `W(x)` = byte basso, byte
   alto di una parola. Il generatore decodifica solo ciò che è certo; ogni
   byte non decodificabile esce come `B(0x..)`. Il generatore stesso
   controlla, prima di scrivere, che le macro espanse diano gli stessi byte
   (verifica interna, in più della 2).
5. **Programma di prova `harness/audio_immagine.c`** (nuovo, < 40 righe):
   `audio_immagine <file>` chiama `aud_dati_serializza` e scrive i byte
   `05C6-3FFF` (14906 byte) nel file; bersaglio `audio_immagine` nel
   `Makefile` (e in `all`/`clean`), con `audio_dati*.c`.
6. **Modalità del banco**: in `audio_piattaforma.c` una funzione
   `void aud_usa_dati_estratti(int modo)` (prototipo in `audio.h`); modo 1
   = sovrascrive in `aud_rom` i byte `05C6-3FFF` con l'immagine serializzata
   (le letture della regione passano così dalle tabelle); modo 2 =
   controllo negativo: stessa regione tutta `0xAA`. `audio_banco.c` legge
   la variabile d'ambiente `AUDIO_DATI` (`estratti` → modo 1, `vuoto` →
   modo 2, altro/assente → ROM com'è oggi) subito dopo `aud_carica_rom()`
   e scrive su **stderr** una riga `dati: estratti` (o `dati: vuoto`):
   stdout resta identico a oggi. `SORGENTI_AUDIO` nel `Makefile` aggiunge
   `audio_dati*.c` (con `$(wildcard audio_dati_*.c)` o elenco) e
   `audio_dati.h` fra le dipendenze.
7. **`decomp/SPARTITI.md`** (nuovo, generato dal Python o scritto a mano;
   preferibile generato per le parti numeriche): sezioni `### Comando NN`
   per 01-3F (NN esadecimale a due cifre): indirizzo dell'elenco; per ogni
   record blocco, strato (1 o 2), chip e voce; n. di byte e di note;
   intervallo di ottave; strumenti usati (rumore, glissando, inviluppo
   software n, inviluppo hardware); durata nominale in frame calcolata dalle
   durate dei dati (somma lungo il percorso principale della voce più
   lunga, F6 con i suoi giri, F7/F8 seguiti) oppure «ciclo» se il flusso
   rientra con un F4 all'indietro; a metà documento una tabella riassuntiva
   dei 63 comandi e la riga «Comandi in ciclo: 13 14 16 19 1A 1B 1C 1E 1F 20
   21 23 24 28 2A». Il nome musicale dei comandi non si ricava dai dati
   (Domande aperte 3): la descrizione è funzionale («3 voci, 1 strato, 18 s,
   glissando, rumore»). In `AUDIO.md` il punto 3 dei lotti («Dati») va
   annotato come fatto, con rimando a `SPARTITI.md`; in `AUDIO-SEQUENZE.md`
   una riga di rimando.

Libero: nomi interni, suddivisione in file, come il generatore codifica le
due forme di F0 (purché i byte tornino), ordine delle sezioni del documento.

## Verifiche obbligatorie (da `decomp/`)

Misure già fatte sulla base di partenza (ROM, nessuna modifica) con le
stesse righe di comando; i valori della modalità `estratti` devono essere
**uguali** a questi.

1. Costruzione e avvertenze:
   `make -C harness clean all 2>&1 | grep -E '(warning|error)'` → vuoto;
   `ls harness/audio_banco harness/audio_immagine harness/harness harness/sched_proto`
   → esistono tutti e quattro.
2. Andata e ritorno (dump fuori repo):
   ```
   D=$(mktemp -d); ./harness/audio_immagine $D/img.bin && \
   dd if=../roms/ext/solomon/1.3jk of=$D/rom.bin bs=1 skip=1478 2>/dev/null && \
   cmp $D/img.bin $D/rom.bin && echo UGUALE; wc -c < $D/img.bin; md5sum < $D/img.bin; rm -rf $D
   ```
   → `UGUALE`, `14906`, `281b711dde6e81ec5e87f0b1af69f431  -` (md5 della
   regione `05C6-3FFF` della ROM, misurato; il `cmp` ne è la prova, l'md5 è
   solo il riscontro).
3. Il generatore è idempotente e non altera la ROM:
   `python3 estrai_dati.py && git status --short | sort > /tmp/s1` (usare
   `$(mktemp)`), rieseguire e confrontare: stesso insieme; nessun file
   sotto `../roms` modificato (`git status --short ../roms` vuoto).
4. Banco dalla ROM (riferimento, modalità di oggi), tutti gli oracoli:
   ```
   for f in $(ls out/audio/*_latch.txt | grep -v foto_); do n=$(basename $f _latch.txt); ./harness/audio_banco $n 2>/dev/null | sed -n 2p; done > $R/rom.txt
   grep -c 'diverso: nessuno' $R/rom.txt; sed 's/, primo.*//' $R/rom.txt | md5sum
   grep -v nessuno $R/rom.txt
   ./harness/audio_banco foto_01_40 > /dev/null 2>&1; cat out/audio/banco/*_ay.txt | md5sum
   ```
   (`R=$(mktemp -d)`) → `39`; `2dad3fe23af76b8a611b3895d03c039b  -`;
   le 5 righe:
   `cod_08_0f: frame 13-839 confrontati 827, primo frame diverso: 301`,
   `cod_20_27: frame 13-795 confrontati 783, primo frame diverso: 571`,
   `cod_28_2f: frame 13-839 confrontati 827, primo frame diverso: 752`,
   `cod_38_3f: frame 13-752 confrontati 740, primo frame diverso: 751`,
   `gioco: frame 13-2199 confrontati 2187, primo frame diverso: 1189`;
   ultimo `md5sum`: `aac0326e3bb023b2f7feeecb7d920c7b  -` (45 file:
   44 oracoli più `foto_01_40`; misurato).
5. Banco dalle tabelle estratte: stesso ciclo con `AUDIO_DATI=estratti`
   davanti a ogni `./harness/audio_banco` (anche quello di `foto_01_40`),
   uscita in `$R/est.txt`: `cmp $R/rom.txt $R/est.txt && echo IDENTICO` →
   `IDENTICO`; di nuovo `39`, `2dad3fe23af76b8a611b3895d03c039b`, le stesse
   5 righe, e `cat out/audio/banco/*_ay.txt | md5sum` →
   `aac0326e3bb023b2f7feeecb7d920c7b  -` (flussi dei cambi identici byte per
   byte). Un solo avvio mostra la riga:
   `AUDIO_DATI=estratti ./harness/audio_banco cod_00_07 2>&1 >/dev/null | grep -c '^dati: estratti'` → `1`.
6. Foto della RAM e casi particolari in modalità estratti:
   `AUDIO_DATI=estratti ./harness/audio_banco foto_01_40 2>/dev/null | sed -n 2,4p` →
   `foto_01_40: frame 13-5789 confrontati 5777, primo frame diverso: 931`,
   `foto_01_40: comando 40 al frame 5790, confronto fermato (fuori modello)`,
   `foto_01_40: foto RAM 127 frame, primo frame diverso: nessuno`;
   `AUDIO_DATI=estratti ./harness/audio_banco cod_41_ff 2>/dev/null; echo $?`
   → `cod_41_ff: avvio 85 cambi, sequenza uguale`,
   `cod_41_ff: frame 13-500 confrontati 488, primo frame diverso: nessuno`,
   `PORTE IGNOTE 12=128 13=64`, `0` (output e uscita già osservati nel brief
   20; non rimisurati qui: attesa).
7. Controllo negativo (le letture passano davvero dalla regione estratta):
   `AUDIO_DATI=vuoto ./harness/audio_banco cod_00_07 2>/dev/null | sed -n 2p`
   → una riga con `primo frame diverso:` seguito da un numero (non
   `nessuno`) (attesa, non misurata).
8. Regressioni invariate:
   `./harness/harness --tutte > /dev/null; echo $?` → `0`;
   `./harness/harness --tutte | grep -c '^RIEPILOGO'` → `112`;
   `python3 copertura_call.py $(ls out/*_eseguiti.txt | grep -v unione) | tail -1`
   → `unione: 235 su 299 (78.6%)`.
9. Limite delle 800 righe:
   `find . -maxdepth 2 \( -name '*.c' -o -name '*.h' -o -name '*.py' \) | xargs wc -l | awk '$1>800 && $2!="totale"'`
   → vuoto.
10. Documento: `grep -c '^### Comando ' SPARTITI.md` → `63`;
    `grep -c 'Comandi in ciclo: 13 14 16 19 1A 1B 1C 1E 1F 20 21 23 24 28 2A' SPARTITI.md`
    → `1`; `grep -nE '(^|[^0-9A-Za-z])([0-9A-F]{2} ){8,}' SPARTITI.md | wc -l`
    → `0` (nessuna sequenza di byte esadecimali).
11. Ambito e pulizia:
    `git status --short` → solo: `?? compiti/21-...` se non già
    committato, e i file attesi: `M NOTICE.md`, `M decomp/AUDIO.md`,
    `M decomp/AUDIO-SEQUENZE.md`, `M decomp/harness/Makefile`,
    `M decomp/harness/audio.h`, `M decomp/harness/audio_banco.c`,
    `M decomp/harness/audio_piattaforma.c`, `?? decomp/estrai_dati.py`,
    `?? decomp/SPARTITI.md`, `?? decomp/harness/audio_dati*.c`,
    `?? decomp/harness/audio_dati*.h`, `?? decomp/harness/audio_immagine.c`;
    `git status --short | grep -E '\.(bin|rom|img)$'` → vuoto;
    `ls /tmp` e la cartella `mktemp` non devono contenere i tuoi dump
    (cancellati).

## Consegna

Nessun commit (lo fa la chat principale). Rapporto finale, massimo trenta
righe: elenco dei file creati con numero di righe, uscita delle 11
verifiche (valori misurati contro attesi), pezzi e righe del serializzatore
(numero di pezzi, numero di file spartiti), deviazioni dal brief, e le righe
di `SPARTITI.md` per i 3 comandi più lunghi come campione.

## Domande aperte

1. **Indirizzi fissi o rilocabili** — proposta: i pezzi stanno agli
   indirizzi originali (`ADR_*` costanti), perché i puntatori F4/F7 e la
   tabella dei comandi sono indirizzi della ROM e l'interprete li legge
   così; un formato rilocabile (etichette risolte da un linker di dati)
   darebbe sorgenti più «puliti» ma non servirebbe ora e allontana la
   prova byte per byte. Marca: **preferenza**.
2. **Selezione della modalità del banco** — proposta: variabile d'ambiente
   `AUDIO_DATI` (`estratti`, `vuoto`), riga di conferma su stderr, invece di
   un secondo eseguibile o di un argomento (non cambia `argv` né stdout).
   Marca: **preferenza**.
3. **Nomi musicali dei comandi** — «che suono/musica è» ogni comando non è
   ricavabile dai dati: il documento descrive voci, strati, durata, ciclo,
   strumenti (funzionale). Dare nomi («moneta», «tema di stanza», «morte»)
   richiede di ascoltare il risultato (lotto renderizzatore) o di incrociare
   le chiamate del principale a `e800`. Proposta: rimandare i nomi al lotto
   renderizzatore, lasciando nel documento una colonna «nome» vuota da
   riempire. Marca: **scelta di progetto** (cambia il perimetro del
   documento).
4. **Residuo `3811-385E`** (comando mai collegato) — proposta: estratto come
   pezzo `residuo_3811` con le stesse macro e annotato in `SPARTITI.md` in
   una sezione a parte, non fra i 63 comandi; serve per l'andata e ritorno
   completa. Marca: **preferenza**.
5. **`NOTICE.md`** — proposta: aggiungere al punto 1 una frase («le tabelle e
   i flussi in `decomp/harness/audio_dati*.c` sono dati musicali derivati dal
   programma Tecmo, estratti con `decomp/estrai_dati.py`»). Marca:
   **preferenza**.

## Risoluzioni (2026-10-22)

Scelta di progetto, risolta con l'utente:
3. **Nomi dei comandi nel lotto renderizzatore.** SPARTITI.md in questo
   lotto resta funzionale (voci, strati, durata, ciclo) con la colonna
   «nome» vuota; i nomi si assegnano nel lotto dei .wav, ascoltandoli
   (li battezza l'utente).

Preferenze accettate con la proposta dell'analista:
1. Pezzi a indirizzi ROM fissi (`ADR_*`), niente formato rilocabile.
2. Modalità del banco con la variabile d'ambiente `AUDIO_DATI`
   (`estratti`, `vuoto`), riga di conferma su stderr.
4. I byte 3811-385E, non raggiunti da alcun comando, estratti come pezzo
   `residuo_3811` fuori dai 63 comandi.
5. Frase in NOTICE.md sulle nuove tabelle derivate.

### Nota sulla verifica 10c (2026-10-22, chat principale)

La 10c com'è scritta è in conflitto con la 10b: la riga obbligatoria
«Comandi in ciclo: 13 14 16 …» contiene per forza coppie esadecimali
separate da spazi, quindi il grep della 10c non può dare 0. Attesa
corretta: il controllo della 10c si applica escludendo la riga «Comandi
in ciclo» (e simili elenchi di codici richiesti dalla 10b); su quel
perimetro l'atteso resta 0.

## Esito (2026-10-22): PASSATO

Ciclo in locale: analista Sonnet ~89k token, sviluppatore Opus ~132k,
verificatore Sonnet ~21k. Tutte le 11 verifiche passate (la 10c con la
correzione annotata sopra: era in conflitto con la 10b; l'eseguibile
audio_immagine è stato aggiunto a .gitignore dalla chat principale in
chiusura).

Risultato: la regione dati 05C6-3FFF è sorgente C generato da
estrai_dati.py (478 pezzi etichettati, 7 file di spartiti, tabelle e
indirizzi), con andata e ritorno byte per byte sulla ROM (14906 byte,
md5 281b711d…) e idempotenza. Il banco con AUDIO_DATI=estratti dà
risultati IDENTICI alla ROM: 39/44, stessi md5, stesse 5 eccezioni;
controllo negativo con dati vuoti diverge al frame 210 come deve.
SPARTITI.md descrive i 63 comandi (voci, strati, durata, ciclo), colonna
«nome» da riempire nel lotto renderizzatore ascoltando i wav.

Scoperte: il comando 0E ha un blocco in due record; il flusso
cmd_1A_b2_s1 è condiviso dal comando 13 (riuso di spartito
nell'originale); il residuo 3811-385E ha salti interni ed etichette
proprie; il 3D dura 1800 frame esatti (30 s) senza ciclo.
