# Compito 61: censimento delle letture della ROM a runtime

Data: 2026-11-06. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Compito di SOLA ANALISI: nessuna modifica al codice del gioco. Produrre
`decomp/CENSIMENTO-ROM.md`, il censimento di tutte le letture che il binario
`gioca` fa a runtime nello spazio ROM (sotto 0xC000, più la finestra
F000-FFFF che oggi vive in ROM), classificate «già estratto in C generato» /
«ancora letto dalla ROM», con indirizzi, tabelle e, per ogni voce ancora da
estrarre, la via d'estrazione proposta sul modello di `estrai_grafica.py` e
`estrai_dati.py` (C generato + round-trip byte per byte). Traguardo finale
(non perimetro qui): binario che non legge più nessuna ROM e dati Tecmo
raccolti in una cartella `tecmo/`.

## Vincoli assoluti

- Nessuna modifica a file esistenti. File nuovi ammessi: `decomp/CENSIMENTO-ROM.md`
  e, se serve a riprodurre i numeri, un solo script di sola lettura
  `decomp/censimento_rom.py` (Python 3, nessuna dipendenza esterna; legge
  ROM e file generati, non scrive nulla fuori da `decomp/out/`). Niente
  nuovo codice C nel repository.
- La strumentazione dinamica (vedi sotto) si fa SOLO su una copia di
  `decomp/harness/` in `/tmp/` o nella scratchpad della sessione, mai committata.
- Non toccare `roms/`, `decomp/out/main64.bin`, i file `video_dati_*.c`,
  `audio_dati_*.c`. Non installare nulla. `gh` non serve.
- Documenti e commenti in italiano. Nessun file sopra le 800 righe: se
  `CENSIMENTO-ROM.md` ci arriva, dividilo in `CENSIMENTO-ROM.md` (sintesi e
  tabella) e `CENSIMENTO-ROM-DETTAGLI.md`.
- Un agente per volta sugli stessi file; niente `pkill`; niente git distruttivo.

## Cosa esiste già (fatti verificati)

File letti a runtime dal binario (da `grafo symbol carica_rom` e lettura di
`audio_piattaforma.c`):

| file | chi lo carica | contenuto |
|---|---|---|
| `decomp/out/main64.bin` (65536 byte) | `carica_rom()`, `decomp/harness/comune.c:37`, in `rom[0x10000]` | 0000-3FFF = `6.3f`; 4000-7FFF = seconda metà di `7.3h`; 8000-BFFF = prima metà di `7.3h`; C000-EFFF = zeri (12288 byte); F000-FFFF = primi 4096 byte di `8.3jk` (da `decomp/build_rom.sh`) |
| `roms/ext/solomon/1.3jk` (16384 byte) | `aud_carica_rom()`, `decomp/harness/audio_piattaforma.c:45`, in `aud_rom[0x4000]` | ROM del Z80 audio: codice 0000-05C5, dati 05C6-3FFF |

Come la ROM principale viene letta (da grep, verificato):
- `rom[]` è toccato in 5 righe in tutto l'harness (`comune.c:11` definizione,
  `ram.h:13` e `ram.h:16` dichiarazione e `leggi`, `macchina.c:613` e
  `macchina_libera.c:138`).
- Unico accesso generico: `static inline uint8_t leggi(uint16_t a)` in
  `ram.h:16`: `return a >= 0xc000 ? ram[a - 0xc000] : rom[a];` (da `grafo explain leggi`).
  Ha 201 occorrenze (`grep -hoE '(^|[^_a-z0-9])leggi\(' decomp/harness/*.c | wc -l`)
  in 38 file `.c` (stesso grep con `-l`), concentrate in `z80_0xxx.c`,
  `z80_4xxx.c`, `z80_2xxx.c`, `z80_3xxx.c`, `z80_chiave_a.c`,
  `attivita_attract07b.c`, `nmi_oggetti.c`, `nemici_comuni.c`. Esempio con
  indirizzo costante + indice: `macchina.c:368-370` (`leggi(0x297a + i)`).
  Nessuna chiamata ha un indirizzo costante nudo (`leggi(0x....)`: 0 casi).
- Copia della finestra F000-FFFF in `ram[]` all'avvio:
  `memcpy(&ram[0xf000 - 0xc000], &rom[0xf000], 0x1000)` in `macchina.c:613` e
  `macchina_libera.c:138`: dopo questo, le tabelle F000/F068/F138 (fondali e
  palette, vedi `decomp/DATI.md`) si leggono come RAM, non via `leggi` sotto C000.
- Tabelle già estratte come dati C per un altro motivo: `rst20_tables.h`
  (137 righe), `e603.h` (12 righe). Vanno elencate come «già in C» nel censimento
  ma verificate (coprono tutta la parte di ROM che dicono di coprire?).
- Altre tabelle del Z80 principale con indirizzi, formato e lettore:
  `decomp/DATI.md` (274 righe), sezione «Strutture»: F000, F068, F138, 1557,
  8EA4, 8578, 858E, rst $20, e603. È il punto di partenza, non l'elenco completo.

Cosa è già estratto:
- Grafica: 12 file `decomp/harness/video_dati_{car,fon,spr}_{a,b,c,d}.c` generati da
  `decomp/estrai_grafica.py` (88 righe), che legge le otto ROM grafiche
  (12.3t, 11.3r, 10.3p, 9.3m, 2.5lm, 3.6lm, 4.7lm, 5.8lm), riscrive solo se
  il contenuto cambia e rilegge il testo prodotto confrontandolo con la ROM
  (uscita 2 se differisce). Non servono più a runtime come file.
- Dati musicali: 21 file tra `audio_dati*.c/.h` e `audio_dati_*.h` generati da
  `decomp/estrai_dati.py` (641 righe) per la regione 05C6-3FFF di `1.3jk`;
  `audio_dati.c` (`aud_dati_serializza`) li ricompone in 14906 byte
  (`AUD_DATI_INIZIO 0x05C6`, `AUD_DATI_FINE 0x4000`, `audio_dati.h:19-20`).
  `scheda_audio.c:74-75` fa `aud_carica_rom(); aud_usa_dati_estratti(1);`:
  carica comunque l'intera `1.3jk` e poi sovrascrive 05C6-3FFF con le tabelle.
  Quindi resta letto dalla ROM audio il codice del driver 0000-05C5 (1478 byte),
  che `aud_leggi()` (`audio_piattaforma.c:104`, `a < 0x4000 → aud_rom[a]`) serve
  anche per le letture dati e da cui parte `audio_interprete.c`/`audio_ossatura.c`.
  Va deciso, per ogni lettura sotto 05C6, se è lettura di codice (interpretato
  o già tradotto in C) o di dato.
- Vuoti di `main64.bin`: C000-EFFF sono zeri e non sono mai dati ROM; il censimento
  deve confermarlo (nessuna lettura ROM lì).

Strumenti già presenti (da `ls decomp`): `coverage.py` (copertura da traccia MAME),
`copertura_call.py`, `disasm.sh`, `out/main64.dasm` (disassemblato),
`ricette/*.reg` (partite registrate: `partita_1a.reg`, `bottiglia_rossa.reg`,
`stanze_bonus.reg`, `riferimento_nuova.reg`, `riferimento_nuova2.reg` - quest'ultimo
non versionato), `giro_completo.sh` (regressione 0-5499, uscita attesa
`GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività, 13 frame con hash coincidenti`),
`gioca --riproduci FILE -n N --muto` (senza finestra, vedi `decomp/GIRO.md:409,483`),
`gcc` e `gcov` installati (`/usr/bin/`).

## Struttura da realizzare

Metodo in tre passi; i primi due si riconciliano, il terzo è il documento.

1. Censimento statico. Per ognuna delle 201 occorrenze di `leggi(` e per ogni
   altro accesso (`aud_leggi`, `aud_rom[`, `memcpy` da `rom`), registrare file,
   riga, funzione, espressione dell'indirizzo, e l'intervallo di ROM raggiungibile
   (costante, costante + indice con limite, puntatore letto da altra tabella). Le
   tabelle puntate da puntatori in ROM (es. F000, 8EA4: parole che indicano record
   a lunghezza variabile) si seguono fino ai record.
2. Misura dinamica sulla copia strumentata: nella copia di `ram.h` fuori dal
   repository, far registrare a `leggi()` (e al `memcpy` di F000, e a `aud_leggi`)
   l'insieme degli indirizzi ROM letti (bitmap 64 KB + 16 KB). Eseguire:
   `giro_completo.sh`, ognuna delle ricette `ricette/*.reg` (`gioca --riproduci
   ... -n <frame> --muto`, senza finestra, con `SDL_VIDEODRIVER=dummy` se serve),
   `ricette/genera_ingressi.py SEME` per almeno 10 semi (vedi `GIRO.md:416`).
   Unire le bitmap in `decomp/out/censimento_rom_letti.txt` (intervalli contigui,
   non versionato) e riportarne la sintesi nel documento.
3. `decomp/CENSIMENTO-ROM.md` (in italiano) con:
   - tabella delle voci: indirizzo iniziale e finale, dimensione, nome proposto
     (per esempio `tab_fondali_f000`), chi la legge (funzioni C e routine Z80
     d'origine), formato, stato (`estratto` / `letto dalla ROM`), copertura
     dinamica (byte letti sui byte della voce) e file ROM d'origine;
   - per ogni voce `letto dalla ROM`: via d'estrazione proposta (script
     generatore nuovo o estensione di `estrai_dati.py`, nome del file C
     generato, simboli esportati, controllo di round-trip byte per byte con
     uscita 2 se differisce, regola «riscrivere solo se cambia»);
   - zone di `main64.bin` mai lette (codice tradotto in C, quindi non più
     necessario come byte) e zone lette ma non dichiarate in `DATI.md`;
   - sezione audio: 0000-05C5 di `1.3jk`, letture classificate codice/dato;
   - sezione `8.3jk`: oltre i primi 4096 byte nessun byte è usato (confermare);
   - ordine consigliato dei lotti successivi, con stima di dimensione in byte
     di sorgente C generato per voce (non per stima: usare il rapporto misurato
     di `video_dati_*.c`: 16384 byte di ROM in 689 righe per file) e rischio;
   - elenco dei file che oggi nominano `main64.bin` / `1.3jk` e andrebbero
     toccati dal lotto che rimuove il caricamento (`comune.c`, `audio_piattaforma.c`,
     `Makefile` target `rom`, `build_rom.sh`, README, `NOTICE.md`), senza modificarli.

Scelte libere: nomi delle voci, suddivisione in file C generati, formato interno
del documento (purché abbia la tabella e le sezioni sopra).

## Verifiche obbligatorie

Eseguire da `/home/rufio72/OpenSolomonsKey-master` salvo diverso avviso.

1. Il documento esiste e non supera le 800 righe:
   `wc -l decomp/CENSIMENTO-ROM.md | awk '{print ($1>0 && $1<=800) ? "OK" : "KO"}'` → `OK`.
2. Nessuna modifica a file tracciati: `git status --short` → solo righe `??` per
   i file nuovi ammessi (`decomp/CENSIMENTO-ROM.md`, eventuale
   `decomp/CENSIMENTO-ROM-DETTAGLI.md`, `decomp/censimento_rom.py`) e per
   `decomp/ricette/riferimento_nuova2.reg` (già non versionato); nessuna riga ` M`.
3. Nessun file di codice sopra 800 righe tra i nuovi:
   `wc -l decomp/censimento_rom.py 2>/dev/null | awk '$1>800'` → vuoto.
4. Punto di partenza invariato: `md5sum decomp/out/main64.bin` →
   `0bca296adf15411c009cf0d13718400d  decomp/out/main64.bin` (misurato) e
   `cmp <(head -c 16384 roms/ext/solomon/6.3f) <(head -c 16384 decomp/out/main64.bin) && echo OK`
   → `OK` (misurato).
5. Conteggio delle occorrenze di `leggi(` coerente col documento:
   `grep -hoE '(^|[^_a-z0-9])leggi\(' decomp/harness/*.c | wc -l` → `201` (misurato);
   il documento contiene una riga che dichiara 201 occorrenze e ne spiega la
   ripartizione per file (somma delle voci = 201, controllata a mano o dallo script).
6. Ogni occorrenza è classificata: se esiste lo script,
   `python3 -I decomp/censimento_rom.py --verifica` → ultima riga
   `CENSIMENTO OK: 201 occorrenze, 0 non classificate` (attesa, non misurata:
   il testo esatto lo decide lo sviluppatore, ma deve avere uscita 0 e dire i
   due numeri). Senza script: tabella nel documento con 201 righe o gruppi che
   sommano 201.
7. La bitmap dinamica non contraddice la statica: nel documento, per ogni
   voce `estratto` o `letto dalla ROM`, il numero di byte letti dinamicamente
   ≤ dimensione della voce, e la somma dei byte letti fuori da ogni voce
   dichiarata è riportata (anche se 0). Lo script, se c'è, stampa
   `LETTI FUORI VOCE: <n>`; l'uscita attesa `0` oppure la lista delle zone, tutte
   spiegate nel documento.
8. Zero letture ROM in C000-EFFF: il documento riporta il numero e deve essere 0
   (attesa, non misurata). Se non è 0, spiegarlo.
9. Round-trip degli estratti già esistenti ancora valido (sola lettura):
   `cd decomp && python3 -I estrai_grafica.py && python3 -I estrai_dati.py && git status --short harness/` → vuoto
   (i generatori non cambiano i file). Se uno dei due fallisce o tocca file, riportarlo e non correggerlo.
10. Il documento cita, per ogni voce `letto dalla ROM`, la via d'estrazione e il
    controllo di round-trip: `grep -c 'round-trip' decomp/CENSIMENTO-ROM.md` ≥ numero di voci `letto dalla ROM`.
11. Nessuna ROM nel documento: `grep -nE '^[0-9A-Fa-f]{2}( [0-9A-Fa-f]{2}){15,}' decomp/CENSIMENTO-ROM.md | wc -l` → `0`
    (nessun dump esadecimale lungo; solo indirizzi, dimensioni e nomi).

## Consegna

- File: `decomp/CENSIMENTO-ROM.md` (+ eventuali `-DETTAGLI.md` e `censimento_rom.py`).
- Un commit: «Censimento delle letture della ROM a runtime, brief 61», con riga
  `Co-Authored-By: <modello> <noreply@anthropic.com>`. Niente push.
- Rapporto finale (max 30 righe): esito delle verifiche 1-11, numero di voci
  per stato, byte totali ancora letti dalla ROM (principale e audio), zone
  mai lette, deviazioni dal brief.

## Domande aperte

1. Ammessa la strumentazione dinamica (patch temporanea di `ram.h` in una copia
   fuori dal repository, mai committata) benché il lotto sia «nessun codice»?
   Proposta: sì, è l'unico modo di avere copertura vera sulle letture ad indirizzo
   calcolato; la copia si cancella a fine lavoro. Marca: preferenza.
2. Ammesso lo script `decomp/censimento_rom.py` come strumento di verifica
   riproducibile? Proposta: sì, sola lettura. Marca: preferenza.
3. Copertura dinamica: quali percorsi bastano per dichiarare «mai letto»?
   Proposta: `giro_completo.sh`, tutte le ricette `.reg`, 10 semi di
   `genera_ingressi.py`; una zona mai letta si classifica `non raggiunta nei
   percorsi misurati`, non «inutile», e il lotto di rimozione la estrae comunque.
   Marca: preferenza.
4. Granularità dell'estrazione futura: un'unica immagine C di `main64.bin`
   (0000-BFFF + F000-FFFF, ~49 KB; più semplice, meno leggibile) oppure tabelle
   con nome per voce (più lavoro, in linea con la fase «riscrittura leggibile»)?
   Il censimento propone la seconda, con l'immagine unica come ripiego per le zone
   non classificate. Marca: scelta di progetto.
5. Il codice del driver audio 0000-05C5 (1478 byte di `1.3jk`) è codice Tecmo
   non «dato»: va estratto come blob C (stesso trattamento) o resta l'ultima
   dipendenza da ROM finché il driver non è tradotto interamente? Proposta:
   il censimento dice quanto ne legge ancora `aud_leggi` e raccomanda il blob
   generato. Marca: scelta di progetto.

## Decisioni (chat principale con l'utente, 2026-11-06)

1. Sì, strumentazione dinamica su copia fuori repository, cancellata a fine
   lavoro — preferenza accettata.
2. Sì, `decomp/censimento_rom.py` di sola lettura come strumento di verifica —
   preferenza accettata.
3. Copertura come proposta (giro completo, tutte le ricette, 10 semi); le zone
   mai lette si marcano `non raggiunta nei percorsi misurati` — preferenza
   accettata.
4. TABELLE CON NOME per voce, immagine unica solo come ripiego per le zone non
   classificate — scelta di progetto, decisa dall'utente.
5. BLOB C SUBITO per il codice del driver audio 0000-05C5: la dipendenza da
   `1.3jk` deve sparire con il lotto di estrazione, coerente con il traguardo
   «clicchi e giochi» — scelta di progetto, decisa dall'utente.

## Esito (2026-11-06)

Ciclo passato al primo colpo (32º). Commit `c794308`; verificatore: 11/11
passate — censimento in decomp/CENSIMENTO-ROM.md (523 righe) + script
censimento_rom.py di sola lettura. Numeri: ROM principale 88 voci / ~17739
byte ancora letti dalla ROM (5888 letti nei percorsi misurati), 1 voce già
estratta; audio: 32 byte del driver ancora letti (controllo contro copia già
in C), dati musicali già estratti; 42 tabelle rst20/rst38 coincidenti con la
ROM. Trappola annotata per l'estrazione: ciclo res 6 di 027C (nmi_oggetti.c)
scrive oltre la RAM nella copia F000 — da limitare quando le tabelle
diventeranno const. Token: analista Sonnet ~60k, sviluppatore Opus ~236k,
verificatore Sonnet ~22k.
