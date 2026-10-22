# Compito 19: Z80 audio, ossatura e scrittura dei registri in C

Data: 2026-10-22. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Primo lotto di traduzione del Z80 audio (ROM `1.3jk`): tradurre in C
l'ossatura del driver (avvio, NMI, IRQ, smistatore dei comandi, `rst`,
scrittura dei registri ai tre YM2149), costruire un banco che confronta il
C con l'oracolo `out/audio/*_ay.txt`, censire i gestori del `jp (hl)` a 0216
per il lotto dell'interprete, coprire l'avvio con una traccia dal frame 0.
L'interprete delle sequenze (`0147` e i gestori di 0216) **non** si traduce
qui: resta uno stub vuoto.

Vincoli assoluti:
- Modifiche solo in `decomp/` più una riga in `.gitignore`. Non toccare
  `roms/`, `compiti/`, `README.md`, `STATO.md`, i sorgenti C esistenti di
  `harness/` (eccetto `Makefile`), `trace.lua`, `sessione.lua`, `snap.lua`,
  `sessioni.sh`, `cattura.sh`, `coverage.py`, `copertura_call.py`,
  `audio.lua`. Nessuna operazione git (il commit lo fa la chat principale).
- Non rifare la corsa completa di `audio.sh` (10 minuti): solo il blocco
  nuovo `avvio`. Gli oracoli esistenti in `out/audio/` non si rigenerano.
- MAME `/usr/games/mame` 0.264 solo tramite `audio.sh` (già senza finestra:
  `-video none -sound none`, `SDL_VIDEODRIVER=dummy`); mai `-window`,
  `-video soft`, `-debugger imgui`.
- Niente ROM né byte della ROM nel repo (nemmeno in commenti o documenti:
  indirizzi e conteggi sì). Il C legge `1.3jk` a tempo di esecuzione.
  Tracce cancellate dopo l'elaborazione; nessun file sopra 50 MB.
- `gcc` 13.3 con le `CFLAGS` del `Makefile` (`-std=c11 -Wall -Wextra -O0`).
- Commenti, messaggi e documenti in italiano; nessun file di codice sopra
  le 800 righe.

## Cosa esiste già (fatti verificati)

### File coinvolti

| File | Righe | Ruolo |
|---|---|---|
| `decomp/out/audio.dasm` | ~990 di codice | `z80dasm -a -l -g 0` di 1.3jk (ignorato da git); fonte della traduzione |
| `decomp/harness/Makefile` | 17 | `SORGENTI = harness.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c $(wildcard z80_*.c)`; `all: harness sched_proto`; `clean` |
| `decomp/harness/ram.h` | 54 | modello del principale: `M(a)`, `struct z80`, `leggi()`; **non** si riusa (spazio c000-ffff) |
| `decomp/harness/harness.c` | 214 | `carica_rom()` riga 18 (cerca `out/main64.bin` poi `../out/main64.bin`, `exit(2)` se manca): modello per il caricamento di 1.3jk |
| `decomp/audio.lua` | 187 | oracolo (vedi sotto); non si modifica |
| `decomp/audio.sh` | 94 | blocchi `codici ignorati lunghi gioco`, funzione `corsa`, `case` finale, `copertura_call.py` in coda |
| `decomp/audio_numeri.py` | 171 | numeri di `AUDIO.md`; `CARTELLA = 'out/audio'`, `ROM = '../roms/ext/solomon/1.3jk'`, `principale()` riga 63 |
| `decomp/AUDIO.md` | 117 | rapporto del lotto 1 audio |
| `.gitignore` | — | righe 50-54: `decomp/out/*`, `decomp/harness/harness`, `decomp/harness/sched_proto`; `audio_banco` **non** è ignorato (da `git check-ignore`) |

Convenzioni del C esistente (da `ram.h`, `routine.c`, `nmi.c`): una
funzione per routine col nome dell'indirizzo, commento di testa
`/* 0x05B3: ... */` con il significato, commenti brevi a destra col
mnemonico originale dove serve (`/* ld (c040),ix */`), costanti `#define`
per gli indirizzi RAM con nome, nessun byte di ROM nel sorgente.

### Driver audio dal dasm (righe di `out/audio.dasm`)

- Reset `0000` (riga 5): `di; jp 0072`.
- Avvio `0072-009D` (riga 111): `sp=4800`, `im 1`, poi 16 giri di: azzera
  4000-47FF (`ldir`), `call 00A4`; poi `ei` e ciclo infinito
  `009E: call 029E; jp 009E`. Le scritture a `ffff` (0079, 0088, 008D,
  0093, 009A, 003F) vanno a un nop di MAME. Misurato: `0090 00A4 16`.
- `00A4` (riga 140): 4240-4242 = FF, poi `ld bc,0310h` e 3 giri che
  vogliono inizializzare i registri 7-0A dei tre chip, **con un errore
  dell'originale**: `ld a,0fh; add a,c; ld a,0ffh` (00CE-00D1) scarta la
  somma, quindi `c` avanza solo di 1 per giro. Sequenza di `out` per giro
  con `c` = 10, 11, 12: `(c)←07 (c+1)←FF (c)←08 (c+1)←00 (c)←09 (c+1)←00
  (c)←0A (c+1)←00`. Effetto: il chip 1 riceve 07..0A come dati nel
  registro 0A; le porte 12 e 13 ricevono 8 e 4 scritture per chiamata
  (16 chiamate → `12=128 13=64`, il conteggio misurato in tutte le corse).
- `00D6` (riga 180): flag (byte +00) dei 18 blocchi da 0x20 a partire da
  4000 = 03; ci si arriva con `jr z` da 00E4, il suo `ret` esce da 00E4.
- IRQ `0038` (righe 50-65): salva i registri, `ei`, `call 0147`,
  `call 00E4`, `reti`. Misurato: **2 IRQ per frame** (`0042 0147 1440`
  nella finestra di 720 frame di `cod_00_07`; `1400` su 700 in `gioco`);
  `AUDIO.md` dice «circa 6-7 per frame»: è sbagliato.
- NMI `0066`: `(4243) = (8000) + 1`, `retn`.
- `00E4` (riga 189): c=(4243); 0 → esce; azzera 4243; `dec c`; 0 → `00D6`;
  `cp 41h; ret nc`; `hl=0AF3; rst 8; rst 10h` → `de` = voce v-1 della
  tabella; poi `00FA-0146` (riga 210, avvio del comando): se de=0 esce;
  ciclo su terne `blocco, parola` fino a un byte FF: hl=4000+blocco·32; se
  blocco ≥ 9 imposta il bit 2 del blocco −9 (`set 2,(hl)` a hl−0x120);
  flag = (flag & 4) | 1; +01/+02 = parola−1; +03 = 01; +04, +08, +09, +0A,
  +16, +17 = 00 (`add a,l; ld l,a` senza riporto: i blocchi sono allineati).
- `rst 8` (0008): `hl += (uint8_t)(2a)` (somma a 8 bit di `add a,a`),
  d=0, e=2a. Tutti i siti sono seguiti da `rst 10h`.
- `rst 10h` (0010): `de = parola(hl)`, `hl += 1` (il sito 0587 usa hl dopo).
- `rst 18h` (0018): puntatore (ix+1/+2) += 1, `a` = byte puntato.
- `029E` (riga 506, `ret` a 02DC): una passata su 18 blocchi; blocco k →
  chip `k/3 % 3 + 1` (porte 10/20/30), voce b = `3 − k%3` (registro volume
  `8+b−1`, tono `2(b−1)` e `2(b−1)+1`); blocchi 9-17 sono il secondo
  strato con la stessa associazione. Per blocco attivo (flag bit 0):
  bit 1 → `02DD`: flag &= 4, poi `02E5`; bit 2 → salta il blocco; bit 3 →
  `02E5` (mixer: voce spenta, volume 0); bit 4-5 → `0381` (inviluppo
  hardware: volume 10, registri 0D/0C/0B da +0F/+11/+10, +0F azzerato dopo
  l'uso); altrimenti `030B`: volume = +08 − +09 (0 se negativo), mixer da
  +0A (rumore sì/no), registro 6 = +0A − 1 se +0A ≠ 0, tono da +0B/+0C.
  In fondo (`02CA`) registro 7 dei tre chip da 4240-4242 (ombre del
  mixer). Usa il banco alternativo (`exx`): in C variabili locali.
- `036B` (riga 656): de = 4240 + chip − 1, `h = 9 << (b−1)`,
  `l = l << (b−1)`.
- Fuori dal lotto: `0147` (riga 280, interprete, chiamato dall'IRQ) e i
  gestori 03EE-05C5. Con flag 00 o 03 in tutti i blocchi `0147` non fa
  nulla (`rrca; jp nc,01ACh` / `rrca; jr c,01ACh`): per questo lo stub
  vuoto è esatto nei tratti confrontati qui sotto.
- Indirizzo selezionato ≥ 0x10: nessun `out` a porta pari del driver
  scrive un valore ≥ 0x10 (valori 06-0D o calcolati da `b`); negli oracoli
  la colonna `reg` vale solo 00-0D (misurato su tutti gli `_ay.txt`). La
  mancata maschera di `audio.lua` non ha effetto: si documenta, non si
  corregge.

### Oracolo `audio.lua` (semantica da riprodurre nel C)

Porta p: chip = `p >> 4`; se chip ∉ 1..3 o `(p & 0x0f) > 1` → porta
ignota (contata); porta pari → registro selezionato del chip = byte intero;
dispari → se nessun registro selezionato, ignota; altrimenti riga
`"%d\t%X\t%02X\t%02X\n"` (frame, chip, reg, val) solo se il valore cambia
rispetto all'ombra (un registro mai scritto conta come cambio).
Intestazione `# frame\tchip\treg\tval`; `# codice <frame> <cc>` prima della
scrittura di ogni codice forzato. `_latch.txt`: `# frame\tvalore\torigine`
poi `frame\tVV\tgioco|forzato`. A fine corsa MAME stampa
`PORTE IGNOTE 12=128 13=64` (misurato in tutte le corse).

### Misure sugli oracoli (oggi, da `out/audio/`)

- Avvio: in tutti i 43 `_ay.txt` (10 `cod_*`, 32 `lungo_*`, `gioco`)
  l'ultimo cambio prima del primo codice è al frame 13, 85 righe di cambi
  nei frame 0-13, stato a fine frame 13 identico:
  `1/07=FF 1/08=00 1/09=00 1/0A=0A 2/07=FF 3/07=FF`; 0 cambi fra il frame
  14 e il primo codice.
- Primo codice: frame 120 in tutti i `cod_*` e `lungo_*`, 1188 in `gioco`.
- `cod_00_07`: codice 00 al 120, codice 01 al 210; 7 cambi nei frame
  120-209, stato a fine 209: registri 07=FF e 08/09/0A=00 sui tre chip.
  Ultimo latch 750, ultimo cambio 759.
- `cod_41_ff`: latch 41..FF dal 120 al 500 a passo 2, **0 cambi dopo il
  frame 13** (codici ignorati da 00E4).
- Gestori del `jp (hl)` a 0216 (tabella a 03CE, 16 voci, indice = byte
  di sequenza F0-FF & 0F; esecuzioni = somma dei `_istruzioni.txt`):
  `F0=03EE:1165 F1=0470:4767 F2=0477:121 F3=047E:0 F4=0485:466 F5=0499:302
  F6=04A6:1394 F7=04BB:326 F8=04D5:286 F9=04E4:4264 FA=04EC:52 FB=04F3:126
  FC=0578:367 FD=0591:0 FE=0598:0 FF=05A6:365`, somma 14001 = esecuzioni
  di 0216. Altre classi di byte di sequenza (a 01F0-01F7): 00-7F nota
  (`0217`, 15038 volte), 80-EF voce della tabella 0790 (`01FA`, 2106).
- Corsa di prova dell'avvio (fatta oggi fuori dal repo, `AUDIO_TRACE_DA=0
  AUDIO_TRACE_A=30 AUDIO_FINE=30`, 0,4 s): traccia 2,1 MB, 124 PC
  distinti, **54 PC nuovi** rispetto all'unione (0000 0001 0072-009D
  00A4-00D5), chiamate `0042 0147 33`, `0045 00E4 33`, `0090 00A4 16`,
  `009E 029E 390`; `PORTE IGNOTE 12=128 13=64`; il suo `_ay.txt` coincide
  con i frame 0-29 di `cod_00_07_ay.txt`. Con i suoi derivati:
  `copertura_call.py` → `unione: 28 su 97 (28.9%)` (oggi 17);
  `audio_numeri.py` → `PC distinti eseguiti: 721 (max 05C3, PC >= 4000: 0)`,
  `byte di ROM eseguiti: 1268 su 16384 (7.7%), mai eseguiti: 15116`,
  `routine: 0008 0010 0018 0038 0066 00A4 00E4 0147 029E 036B`.
- Banco del principale: `./harness/harness --tutte` esce con 0, 112 righe
  `RIEPILOGO`.

## Struttura da realizzare

Tutto in `/home/rufio72/OpenSolomonsKey-master/decomp/`. Nomi delle
funzioni audio con prefisso `aud_` (i nomi `sub_0008`, `sub_0066` ecc.
esistono già nel principale). Firme C idiomatiche (niente `struct z80`):
il banco confronta solo le uscite AY.

1. **`harness/audio.h`** (sotto 100 righe):
   ```c
   extern uint8_t aud_ram[0x800];                 /* 4000-47ff */
   extern uint8_t aud_rom[0x4000];                /* 1.3jk */
   #define AM(a) aud_ram[(uint16_t)(a) - 0x4000]
   uint8_t aud_leggi(uint16_t a);                 /* ROM sotto 4000, RAM 4000-47ff */
   void aud_out(uint8_t porta, uint8_t v);        /* out (c),r: semantica di audio.lua */
   void aud_avvio(void);                          /* 0072-009D senza il ciclo infinito */
   void aud_00a4(void);
   void aud_0038(void);                           /* IRQ: aud_0147(); aud_00e4(); */
   void aud_0066(uint8_t latch);                  /* NMI */
   void aud_00e4(void);
   uint16_t aud_0008(uint16_t hl, uint8_t a);
   uint16_t aud_0010(uint16_t *hl);
   uint8_t aud_0018(uint16_t ix);
   void aud_029e(void);                           /* una passata */
   void aud_0147(void);                           /* interprete: lotto successivo */
   ```
   più `#define` con nome per 4240 (ombre mixer), 4243 (comando), 0AF3,
   passo 0x20, 18 blocchi, campi del blocco usati qui. `aud_036b` può
   restare `static` in `audio_registri.c`.
2. **`harness/audio_ossatura.c`**: `aud_avvio`, `aud_00a4` (con l'errore
   dell'originale riprodotto e commentato), `aud_0038`, `aud_0066`,
   `aud_00e4` con `00D6` e `00FA` come funzioni `static`, i tre `rst`.
   Le scritture a `ffff` si omettono con un commento.
3. **`harness/audio_registri.c`**: `aud_029e` e `aud_036b`, tutti i rami
   (02DD, 02E5, 030B, 0381) anche se qui ne viene esercitata solo una parte.
4. **`harness/audio_interprete.c`**: `void aud_0147(void) {}` con commento
   «interprete delle sequenze, lotto successivo».
5. **`harness/audio_piattaforma.c`**: `aud_ram`, `aud_rom`, caricamento di
   `../roms/ext/solomon/1.3jk` (poi `../../roms/...`; 16384 byte o
   `exit(2)`), `aud_leggi`, `aud_out` con registro selezionato e ombra per
   chip come `audio.lua`, scrittura delle righe di cambio sul file aperto
   dal banco, conteggio delle porte ignote, frame corrente come variabile.
6. **`harness/audio_banco.c`**: `main`, uso `audio_banco <nome> [frame_max]`
   da `decomp/`. Legge `out/audio/<nome>_latch.txt` e `_ay.txt`; scrive
   `out/audio/banco/<nome>_ay.txt` nello stesso formato (crea la cartella;
   `# codice` per le righe latch `forzato`). Modello del tempo: al frame 0
   `aud_avvio()`; per ogni frame F da 0 a frame_max: ogni latch di F, in
   ordine, → `aud_0066(v)` (latch prima del frame 14 ignorati con avviso:
   l'avvio di MAME azzera la RAM); poi due volte `aud_0038(); aud_029e();`.
   frame_max predefinito = massimo fra ultimo frame di latch e ultimo
   frame di cambio dell'oracolo. Confronti:
   - sequenza dei cambi (chip, reg, val) dei frame 0-13, senza colonna
     frame, fra oracolo e C;
   - stato dei registri a fine frame (mai scritto ≠ qualsiasi valore) per
     F da 13 a frame_max.
   Stampa esattamente, in quest'ordine:
   ```
   <nome>: avvio <n> cambi, sequenza uguale|diversa
   <nome>: frame 13-<frame_max> confrontati <n>, primo frame diverso: nessuno|<F>
   PORTE IGNOTE 12=<n> 13=<n>
   ```
   Uscita 0 se sequenza uguale e nessun frame diverso, altrimenti 1; 2 per
   errori di file.
   **Il file di flusso è un'interfaccia stabile**: alla fine dei lotti
   audio il C dovrà generare i `.wav` dei 65 codici passando questo flusso
   a un emulatore YM2149 (test finale della catena; il renderizzatore non
   fa parte di questo lotto). Quindi `out/audio/banco/<nome>_ay.txt` va
   scritto per intero (tutti i frame fino a frame_max, non solo il tratto
   confrontato) e byte per byte nel formato di `audio.lua` (intestazione,
   `"%d\t%X\t%02X\t%02X\n"`, righe `# codice`), così che oracolo e C
   siano intercambiabili per il renderizzatore. Scarto noto e da scrivere
   in `AUDIO.md`: i cambi dell'avvio nel C portano tutti il frame 0
   (in MAME 0-13). La scrittura del flusso sta in `audio_piattaforma.c`
   dietro una funzione sola (`aud_flusso_apri(const char *percorso)` /
   `aud_flusso_chiudi(void)`), separata dal confronto in `audio_banco.c`.
7. **`harness/Makefile`**: `SORGENTI_AUDIO = audio_banco.c
   audio_piattaforma.c audio_ossatura.c audio_registri.c
   audio_interprete.c`, bersaglio `audio_banco` (dipende anche da
   `audio.h`), aggiunto ad `all` e a `clean`. **`.gitignore`**: riga
   `decomp/harness/audio_banco` dopo `decomp/harness/sched_proto`.
8. **`audio.sh`**: blocco `avvio()` con `rm -f out/audio/avvio_*` e
   `AUDIO_CODICI= AUDIO_TRACE_DA=0 AUDIO_TRACE_A=30 AUDIO_FINE=30 corsa avvio`;
   voce `avvio` nel `case`, primo in `tutto`, citato nel commento d'uso e
   nel messaggio d'errore.
9. **`audio_numeri.py`**: riga in più dopo `jp (hl) eseguiti`:
   `gestori 0216: F0=03EE:<n> ... FF=05A6:<n> somma <n>` (indirizzi letti
   dalla tabella a 03CE della ROM, esecuzioni sommate dai `_istruzioni.txt`).
10. **`AUDIO-SEQUENZE.md`** (nuovo, 60-120 righe, input del lotto
    interprete): classi dei byte di sequenza (00-7F, 80-EF, F0-FF) con
    indirizzi e conteggi; tabella dei 16 gestori (codice, indirizzo,
    esecuzioni, campi del blocco letti/scritti, lettura del dasm marcata
    «da confermare nel lotto interprete»; F3, FD, FE mai eseguiti); campi
    del blocco da 32 byte noti da 00FA, 029E, 0147; tabelle dati usate
    (0AF3, 03CE, 0700, 0760, 0790, 0850).
11. **`AUDIO.md`**: numeri aggiornati con l'avvio (721, 1268, 28 su 97,
    10 routine, intervalli), «2 IRQ per frame» al posto di «6-7»,
    spiegazione delle porte 12/13 (errore di 00A4), nota sul registro
    selezionato non mascherato (senza effetto, misurato), sezione
    «Ossatura tradotta» (file, banco, cosa confronta, tratti che
    coincidono, limite: l'ordine dei cambi dentro un frame dipende da dove
    l'IRQ interrompe 029E e non è modellato), rimando a `AUDIO-SEQUENZE.md`; il flusso del banco è
    l'ingresso del futuro renderizzatore `.wav`), rimando a `AUDIO-SEQUENZE.md`.

## Verifiche obbligatorie (da `decomp/`)

1. `make -C harness clean all 2>&1 | grep -E 'audio_[a-z]*\.[ch].*(warning|error)'`
   → vuoto; `ls harness/audio_banco` esiste.
2. `./harness/harness --tutte > /dev/null; echo $?` → `0` e
   `./harness/harness --tutte | grep -c '^RIEPILOGO'` → `112` (misurato).
3. `sh audio.sh avvio > out/audio/avvio.log 2>&1`; nel log: nessun
   `ERRORE:`, `PORTE IGNOTE 12=128 13=64`, ultima riga di copertura
   `unione: 28 su 97 (28.9%)` (misurato); `wc -l < out/audio/avvio_eseguiti.txt`
   → `124`; `grep -P '^0090\t00A4\t16$' out/audio/avvio_chiamate.txt` → una
   riga (misurati).
4. `python3 audio_numeri.py` → contiene
   `PC distinti eseguiti: 721 (max 05C3, PC >= 4000: 0)`,
   `byte di ROM eseguiti: 1268 su 16384 (7.7%), mai eseguiti: 15116`,
   `routine: 0008 0010 0018 0038 0066 00A4 00E4 0147 029E 036B`,
   `gestori 0216: F0=03EE:1165 F1=0470:4767 F2=0477:121 F3=047E:0 F4=0485:466 F5=0499:302 F6=04A6:1394 F7=04BB:326 F8=04D5:286 F9=04E4:4264 FA=04EC:52 FB=04F3:126 FC=0578:367 FD=0591:0 FE=0598:0 FF=05A6:365 somma 14001`
   (misurati; la riga dei gestori nel formato prescritto).
5. `./harness/audio_banco cod_41_ff; echo $?` →
   ```
   cod_41_ff: avvio 85 cambi, sequenza uguale
   cod_41_ff: frame 13-500 confrontati 488, primo frame diverso: nessuno
   PORTE IGNOTE 12=128 13=64
   0
   ```
   (conteggi misurati sull'oracolo; l'uguaglianza del C è attesa).
6. `./harness/audio_banco gioco 1187` → seconda riga
   `gioco: frame 13-1187 confrontati 1175, primo frame diverso: nessuno`, uscita 0.
7. `./harness/audio_banco cod_00_07 209` → seconda riga
   `cod_00_07: frame 13-209 confrontati 197, primo frame diverso: nessuno`,
   uscita 0 (il codice 00 è tutta ossatura).
8. Prefisso di tutti gli oracoli fino al primo codice:
   `for f in out/audio/*_latch.txt; do n=$(basename $f _latch.txt); [ $n = avvio ] && continue; L=$(awk '!/^#/{print $1; exit}' $f); ./harness/audio_banco $n $((L-1)) | grep -c 'diverso: nessuno'; done | sort | uniq -c`
   → `     43 1`.
9. `head -1 out/audio/banco/cod_41_ff_ay.txt | od -c | head -1` uguale a
   `head -1 out/audio/cod_41_ff_ay.txt | od -c | head -1`, e
   `diff <(grep -v '^#' out/audio/cod_41_ff_ay.txt | cut -f2-) <(grep -v '^#' out/audio/banco/cod_41_ff_ay.txt | cut -f2-)`
   → vuoto, `diff <(grep '^# codice' out/audio/cod_41_ff_ay.txt) <(grep '^# codice' out/audio/banco/cod_41_ff_ay.txt)`
   → vuoto (formato del flusso identico all'oracolo; attesa, non misurata).
10. `./harness/audio_banco cod_00_07; echo $?` → primo frame diverso ≥ 210,
   uscita 1 (attesa, non misurata: dal codice 01 serve l'interprete).
11. `awk '!/^#/ && $3 !~ /^0[0-9A-D]$/' out/audio/*_ay.txt | wc -l` → `0`
    (misurato: registri solo 00-0D).
12. `grep -c '6-7 per frame' AUDIO.md` → `0`; `grep -c '2 IRQ per frame' AUDIO.md` → ≥ 1.
13. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale" && $2!="total"'` → vuoto.
14. `ls out/audio/*trace* 2>/dev/null` → vuoto; `find .. -size +50M -not -path '../roms/*'` → vuoto; `pgrep -c mame` → `0`.
15. `grep -n -e '-window' -e 'video soft' -e imgui audio.sh` → vuoto.
16. `git -C .. status --short` → solo `.gitignore`, `decomp/AUDIO.md`,
    `decomp/AUDIO-SEQUENZE.md`, `decomp/audio.sh`, `decomp/audio_numeri.py`,
    `decomp/harness/Makefile`, `decomp/harness/audio*.{c,h}` (nuovi) e questo
    brief; `git -C .. check-ignore decomp/harness/audio_banco` → la stampa.

## Consegna

- I file della «Struttura»; in `out/audio/` solo derivati (`avvio_*`,
  `banco/`), nessuna traccia. Nessun commit; se richiesto, messaggio in
  italiano con riga `Co-Authored-By: <modello> <noreply@anthropic.com>`.
- Rapporto finale, massimo trenta righe: uscita delle verifiche 1-16,
  frame del primo scarto della verifica 10, righe dei file nuovi,
  deviazioni dal brief con motivo.

## Domande aperte

1. «scelta di progetto» Il banco confronta lo stato dei registri a fine
   frame (più la sequenza esatta dell'avvio), non il file dei cambi riga
   per riga: l'ordine dei cambi dentro un frame dipende da dove l'IRQ
   interrompe la passata di 029E, che il C non modella a cicli (nel codice
   00 dell'oracolo il chip 1 compare dopo i chip 2 e 3 per questo). È il
   criterio che erediterà il lotto interprete. Proposta: stato a fine frame.
2. «scelta di progetto» Con la sola ossatura nessun codice 01-40 è
   verificabile: l'avvio del comando (00FA) prepara i blocchi ma le note le
   legge 0147. Verificabili qui: avvio, inattività, codice 00, codici
   41-FF. Alternativa: cattura della RAM audio prima/dopo 00E4 per ogni
   codice (script nuovo sul modello di `snap.lua`). Proposta: niente
   cattura, 00FA si verifica nel lotto interprete.
3. «preferenza» Prefisso `aud_` e firme C idiomatiche invece di
   `struct z80`. Proposta: così.
4. «preferenza» Modello del tempo: latch a inizio frame, 2 × (IRQ +
   una passata di 029E) per frame. Proposta: così; la fase degli IRQ nel
   frame si misura nel lotto interprete se serve.
5. «preferenza» `audio.lua` non si corregge (selezione non mascherata
   senza effetto, misurato); solo nota in `AUDIO.md`. Proposta: così.
6. «preferenza» Censimento in `decomp/AUDIO-SEQUENZE.md` invece che in
   `AUDIO.md`. Proposta: file a parte.
7. Decisione già presa (2026-10-22), non è una domanda: il flusso dei
   cambi del banco è nel formato degli oracoli perché un lotto futuro lo
   userà per generare i `.wav` dei 65 codici con un emulatore YM2149;
   il renderizzatore non si fa in questo lotto.

## Risoluzioni (2026-10-22)

Scelte di progetto, risolte con l'utente:
1. **Banco a stato di fine frame.** Si confronta lo stato dei 14 registri
   di ogni chip alla fine di ogni frame, non riga per riga: l'ordine dei
   cambi dentro il frame dipende da dove l'IRQ interrompe 029E e il C non
   lo modella. Il criterio vale anche per il lotto dell'interprete; il
   tempismo fine lo collauda il test finale dei .wav generati dal C.
2. **Verifica di 00FA rinviata al lotto interprete.** In questo lotto
   niente catture di RAM attorno a 00E4; l'ossatura si verifica sui
   registri e sui casi che coincidono già (avvio, attesa, codice 00,
   codici 41-FF).

Preferenze accettate con la proposta dell'analista:
3. Prefisso `aud_` e firme C semplici, senza struct z80.
4. Modello del tempo: comando a inizio frame, poi 2 volte IRQ seguito da
   una passata di 029E.
5. `audio.lua` non si tocca; la mancata maschera dei 4 bit bassi è
   documentata in AUDIO.md (ininfluente: i registri negli oracoli valgono
   solo 00-0D).
6. Censimento dei gestori in `AUDIO-SEQUENZE.md`, file a parte.

(Il punto 7 delle domande aperte riporta la decisione già presa
dall'utente sul flusso per i .wav: il banco scrive il flusso dei cambi
nello stesso formato di out/audio/*_ay.txt, renderizzatore in un lotto
futuro.)

## Esito (2026-10-22): PASSATO

Ciclo in locale: analista Opus ~134k token, sviluppatore Opus ~113k,
verificatore Sonnet ~19k. Un giro di correzione: la verifica 9 era
fallita al verificatore perché `audio_banco` con `frame_max` troncava il
file di flusso al frame limite (la 8 lo riscriveva fino al 119, prima del
primo `# codice` al 120); corretto facendo scrivere sempre il flusso per
intero, col limite valido solo per i confronti. Dopo la correzione tutte
le 16 verifiche passano, rifatte nell'ordine del brief.

Risultato: l'ossatura C coincide con l'originale su avvio (85 cambi,
bug delle porte 12/13 di 00A4 riprodotto), attesa, codice 00 (1175 frame
di gioco) e codici ignorati 41-FF; con l'interprete a stub `cod_00_07`
diverge al frame 211 come atteso. Il flusso dei cambi esce nello stesso
formato degli oracoli, righe `# codice` comprese: è l'interfaccia del
futuro renderizzatore wav.

Scoperte da ricordare per il lotto interprete:
- i bit per l'inviluppo hardware sono 5-6 del flag (il brief diceva 4-5);
  il bit 4 è il glissando usato da 0147 e impostato dal comando FB;
- il ramo 0381 senza bit 5 passa da 02E5 e azzera il volume;
- il comando 40 punta a 7A09, fuori ROM: funziona perché l'hardware legge
  00 dalla memoria non mappata (il C lo riproduce con aud_leggi/aud_scrivi
  e il latch);
- gestori di 0216: 16, tabella a 03CE; F3, FD, FE mai eseguiti.
