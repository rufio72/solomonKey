# Compito 20: Z80 audio, interprete delle sequenze

Data: 2026-10-22. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Tradurre in C l'interprete delle sequenze del Z80 audio (ROM `1.3jk`): la
routine `0147` (chiamata dall'IRQ), la lettura dei byte di sequenza da `01E8`,
la nota `0217-029D`, i 16 gestori della tabella `03CE` (`03EE-05C5`), il
glissando e l'inviluppo. Alla fine `audio_banco` deve dare **0 frame diversi
su tutti gli oracoli**, per l'intera durata di ognuno, e la RAM dei blocchi
deve coincidere con catture fatte in MAME attorno all'avvio di ogni comando
(verifica di `00FA`).

Vincoli assoluti:
- Modifiche solo in `decomp/`. Non toccare `roms/`, `compiti/`, `README.md`,
  `STATO.md`, `.gitignore`, i sorgenti C del principale in `harness/`,
  `trace.lua`, `sessione.lua`, `snap.lua`, `sessioni.sh`, `cattura.sh`,
  `coverage.py`, `copertura_call.py`, `audio_numeri.py`. Nessuna operazione
  git (il commit lo fa la chat principale).
- Gli oracoli esistenti in `out/audio/` (`cod_*`, `lungo_*`, `gioco`,
  `avvio`) **non si rigenerano**: niente `sh audio.sh` senza argomento né coi
  blocchi `codici`, `ignorati`, `lunghi`, `gioco`. Ammessi solo i blocchi
  `avvio` (0,4 s) e il nuovo `foto`.
- MAME `/usr/games/mame` 0.264 solo tramite `audio.sh` (già senza finestra:
  `-video none -sound none`, `SDL_VIDEODRIVER=dummy`); mai `-window`,
  `-video soft`, `-debugger imgui`. Una corsa per volta.
- Niente ROM né byte della ROM nel repo (nemmeno in commenti o documenti:
  indirizzi, mnemonici e conteggi sì). Il C legge `1.3jk` a tempo di
  esecuzione. Nessuna traccia lasciata; nessun file sopra 50 MB.
- `gcc` 13.3 con le `CFLAGS` del `Makefile` (`-std=c11 -Wall -Wextra -O0`).
- Commenti, messaggi e documenti in italiano; nessun file di codice sopra
  le 800 righe.

## Cosa esiste già (fatti verificati)

### File coinvolti (righe da `wc -l`, firme da `grafo symbol`/`outline`)

| File | Righe | Ruolo |
|---|---|---|
| `decomp/out/audio.dasm` | 13896 | `z80dasm` di 1.3jk (ignorato da git). `0147` a riga 280, `0217-029D` righe 437-505, gestori `03EE-05C5` righe 759-989, `00E4/00FA` righe 190-279, IRQ `0038` riga 61 |
| `harness/audio.h` | 78 | `AM(a)`, `#define` dei blocchi (`AUD_BLOCCHI 0x4000`, `AUD_N_BLOCCHI 18`, `AUD_PASSO_BLOCCO 0x20`, `AUD_COMANDO 0x4243`), campi `BL_*` (solo quelli dell'ossatura), prototipi |
| `harness/audio_interprete.c` | 11 | `void aud_0147(void) {}`: stub da sostituire |
| `harness/audio_ossatura.c` | 152 | `aud_avvio` 10, `aud_00a4` 24, `static aud_00d6` 52, `static aud_00fa(uint16_t de)` 65, `aud_00e4` 96, `aud_0038` 114 (`aud_0147(); aud_00e4();`), `aud_0066` 121, `aud_0008` 128, `aud_0010` 135, `aud_0018` 145 |
| `harness/audio_registri.c` | 125 | `aud_029e` 96 (rami 02DD, 02E5, 030B, `aud_0381` 70); non si modifica |
| `harness/audio_piattaforma.c` | 138 | `aud_leggi` 66 (ROM < 4000, RAM 4000-47FF, `aud_latch` a 8000, **00 altrove**), `aud_scrivi` 76 (solo RAM) |
| `harness/audio_banco.c` | 196 | `main` 98: per frame latch → `aud_0066`, poi `for (irq<2) { aud_0038(); aud_029e(); }`; `FINE_AVVIO 13`, `PRIMO_LATCH 14` |
| `audio.lua` | 187 | oracolo; callback `register_frame_done` in fondo: `frames = frames + 1`, poi suspend/ram/trace, poi il codice di `piano[frames]` scritto a e800 |
| `audio.sh` | 102 | funzione `corsa`, blocchi `avvio codici ignorati lunghi gioco`, `case` finale, `copertura_call.py` in coda |
| `AUDIO-SEQUENZE.md` | 98 | censimento: classi dei byte, 16 gestori con conteggi, campi del blocco, tabelle; letture marcate «da confermare» |
| `AUDIO.md` | 172 | rapporto audio; sezione «Ossatura tradotta» righe ~150-172 |

Primitive già pronte (da `grafo symbol`): `aud_0008(hl,a)` = `hl + (uint8_t)(a+a)`
(l'originale pone anche `de = (uint8_t)(a+a)`, d=0); `aud_0010(&hl)` = parola
a hl, hl avanzato di 1 (resta sul byte alto); `aud_0018(ix)` = puntatore
+01/+02 incrementato e riscritto, ritorna il byte puntato (l'originale lascia
`hl` = puntatore nuovo, `de` intatto). Convenzioni: una funzione per routine
col nome dell'indirizzo (`aud_xxxx`, `static` se interna), commento di testa
`/* 0x0217: ... */`, mnemonico originale a destra dove serve.

### Lettura del dasm (fatta oggi, righe 280-505 e 759-989)

**0147, per ognuno dei 18 blocchi** (`hl` da 4000 a passo 20 fino a 4240):
1. flag = +00; bit 0 a zero o bit 1 a uno → blocco saltato. Il flag ruotato
   due volte resta in `af'` ed è quello letto ai punti 3-4 (flag della
   **partenza** del blocco).
2. Durata a 16 bit +03/+04: `+03 -= 1`; se non c'è prestito → punto 3. Se
   c'è prestito: se +04 = 0 → **lettura della sequenza** (`01E8`, `ix` =
   blocco) e fine del blocco per questo IRQ (niente glissando né inviluppo);
   altrimenti `+04 -= 1` → punto 3. Quindi una durata d dura d+1 IRQ
   (per questo `01FA` scrive parola − 1 e `00FA` scrive +03 = 1).
3. Glissando (flag bit 4, `01B2`): +0B/+0C += +0D/+0E (16 bit, si avvolge;
   **nessuna condizione di fine**, si ferma solo con una nota nuova che fa
   `flag &= E7`); se anche il bit 6: +10/+11 = periodo >> 3.
4. Inviluppo software solo se i bit 5 e 6 sono a zero (il bit 3 non conta):
   `+05 -= 1`; se arriva a 0: p = +06/+07; se il byte a p+1 non è FF:
   +08 = (p+1), puntatore = p+2, +05 = (p+2); se è FF: riparte da
   s = +14/+15: +08 = (s), puntatore = s+1, +05 = (s+1).

**Lettura `01EF`** (`rst 18h`): F0-FF → gestore (sotto); 80-EF →
+12/+13 = parola di `0790 + 2·((b & 7F) + (+16))` (indice a 8 bit, `rst 8`)
meno 1, poi byte seguente; 00-7F → nota `0217`: nibble basso ≥ 0C → pausa
(`set 3` del flag) e `0250`; altrimenti `flag &= E7`, indice = 12·ottava +
nota + (+17) (a 8 bit); < 30 → parola da `0700 + 2·indice`, altrimenti byte
da `0760 + indice − 30` (byte alto 0); → +0B/+0C; `0250`: +03/+04 =
+12/+13; se flag & 60 = 0: +08 = (s), +05 = (s+1), +06/+07 = s+1 con
s = +14/+15; se flag & 60 ≠ 0 e bit 6: +10/+11 = **`de`** >> 3. Fine del
blocco.

**Il registro `de` va portato come variabile.** In `0286` (pausa con bit 6)
`de` non è il periodo ma l'ultimo valore lasciato dal percorso. Valori:
all'ingresso del blocco k ≥ 1 di 0147 `de = 0x4240 − (0x4000 + 0x20·k)`
(da `019F-01A7`); al blocco 0 è quello del codice interrotto dall'IRQ (non
modellabile); dopo `rst 8` `de = (uint8_t)(2a)`; dopo `rst 10h` `de` =
parola; all'ingresso di ogni gestore `de = 0x03CF + 2·(b & 0F)` (`ex de,hl`
a 0215); 80-EF lascia `de` = durata − 1; nota < 30 lascia la parola, ≥ 30 il
byte (d = 0). Gestori che cambiano `de`: F0 (inizio inviluppo o periodo),
F4 e F7 (parola − 1), F6 se ripete (+18/+19), F8 (+1B/+1C), FB (periodo
iniziale, `pop de`), FC (periodo); gli altri lo lasciano.

**Gestori** (indice b & 0F, tabella 03CE; «→ 01EF» = byte seguente):

| cod | ind. | effetto (verificato sul dasm) |
|---|---|---|
| F0 | 03EE | n = byte a ptr+1. n = FF → forma = byte, periodo = parola seguente → +0F, +10/+11; ptr sull'ultimo byte; flag = (flag & 9F) \| 20. n = FE → +0F = byte seguente; se il byte dopo è FF: ptr su quell'FF (consumato), flag = (flag & 9F) \| 40; altrimenti ptr sul byte della forma, flag = (flag & 9F) \| 60. Altrimenti ptr = indirizzo di n, +14/+15 = parola di `0850 + (uint8_t)(2n)`, flag &= 9F. → 01EF |
| F1 | 0470 | +09 = byte → 01EF |
| F2 | 0477 | +17 = byte → 01EF |
| F3 | 047E | +16 = byte → 01EF. **Non osservato** |
| F4 | 0485 | ptr = parola a ptr+1 − 1 → 01EF |
| F5 | 0499 | +1A = byte; +18/+19 = ptr dopo `rst 18h` (indirizzo del contatore) → 01EF |
| F6 | 04A6 | `+1A -= 1`; se ≠ 0 ptr = +18/+19 (contatore 0 = 256 giri) → 01EF |
| F7 | 04BB | ptr = parola − 1; +1B/+1C = indirizzo del byte alto della parola (un solo livello) → 01EF |
| F8 | 04D5 | ptr = +1B/+1C → 01EF |
| F9 | 04E4 | +0A = byte + 1 → 01EF |
| FA | 04EC | +0A = 0 → 01EF |
| FB | 04F3 | `flag &= E7`; p0 = parola 1 → +0B/+0C; p1 = parola 2; diff = p1 − p0 (16 bit), se negativo `set 4` e diff = −diff; de = diff << 4 (16 bit); dur = parola 3 → +12/+13, ptr sul byte alto di dur; divisione a **12 giri** (`0547-0559`: `sla e; rl d; rl l; rl h; sbc hl,bc; inc de`, ripristino se prestito) → quoziente = (diff & 0FFF) / dur; se bit 4: de = −de; +0D/+0E = de; `set 4`; `de` = p0; `0250`. Fine del blocco. Tradurre il ciclo alla lettera in `uint16_t` (la lettura «·16» di `AUDIO-SEQUENZE.md` è sbagliata) |
| FC | 0578 | `flag &= E7`; periodo = parola a ptr+1 → +0B/+0C; ptr sul byte alto; `024A` → `0250`. Fine del blocco |
| FD | 0591 | (4243) = byte → 01EF (il comando parte da 00E4 nello stesso IRQ). **Non osservato** |
| FE | 0598 | a = +09 + 1; se < 0F → +09 = a, → 01EF; altrimenti prosegue in FF (+09 non aggiornato). **Non osservato** |
| FF | 05A6 | flag = (flag & 04) \| 03; se il blocco è oltre 4100 (blocchi 9-17) `res 2` del flag del blocco − 9 (hl − 120). Fine del blocco |

### Blocchi, voci e RAM (misurati sui `_ram.txt` e dal codice di 029E)

- Byte di RAM scritti in tutte le corse: 458. Blocchi 0-17 tutti usati;
  campi scritti +00-+1C (+0D/+0E in 7 blocchi, +0F-+11 in 2, +18-+1A in 13,
  +1B/+1C in 9), **+1D-+1F mai**. Globali 4240-4243. Pila 34 byte da 47DE
  (sp iniziale 4800). Unico altro byte: 46E0, scritto dal comando 40.
- L'ipotesi «un blocco = una voce» è **falsa**: da `aud_029e` il blocco k va
  al chip `k/3 % 3 + 1`, voce `3 − k%3`; i blocchi 9-17 sono un secondo
  strato sulle stesse 9 voci dei blocchi 0-8 (00FA pone il bit 2 «coperto»
  sul blocco k − 9, FF lo toglie). 18 blocchi = 9 voci × 2 strati.
- Trappole già note: il flag bit 4 è il glissando, bit 5-6 l'inviluppo
  hardware; `aud_0381` con il solo bit 6 (flag & 60 = 40, da F0 FE + FF)
  passa da 02E5 e azzera il volume (già tradotto, non toccare).

### Il comando 40 (misurato su `cod_40_ram.txt`, `cod_40_ay.txt`)

La voce 40 della tabella 0AF3 punta a 7A09: `00FA` scorre la memoria non
mappata (letta 00 → blocco 0) fino a FFFF, legge il latch a 8000 (40 →
blocco 40: `set 2` a 46E0, frame 123, flag a 4800 perso) e riparte dalla
ROM a 0000. **Dura ~86 frame dentro il primo IRQ**: 4000 scritto 11436
volte fra il frame 120 e il 209; l'IRQ fa `ei` a 003E, quindi gli IRQ
successivi si annidano (0147 + 00E4) mentre `029E` non gira: nessun cambio
AY fra il 120 e il 206 (cambi solo a 206 e 207), +03 scritto 11615 volte
(179 passate di 0147 in più), +05 179 volte. Costo di un giro di 00FA
(contato dal dasm, attesa, non misurata): ~356 cicli con blocco < 9, ~413
con blocco ≥ 9; IRQ ogni 3072000 / 120 = **25600 cicli** (`mame -listxml`:
audiocpu 3072000 Hz, schermo 60 Hz; 2 IRQ per frame misurati). Con lo stub,
`cod_40` e `lungo_40` divergono già al frame 120 per questo.

### Misure di oggi sugli oracoli (stub attuale)

`for f in out/audio/*_latch.txt; do n=$(basename $f _latch.txt); ./harness/audio_banco $n 2>/dev/null | sed -n 2p; done`:
44 righe (43 oracoli più `avvio`); intervalli: `cod_00_07` 13-759 (747),
`gioco` 13-2199 (2187), `cod_41_ff` 13-500 (488), `cod_40`/`lungo_40`
13-207 (195), 17 `lungo_*` 13-1019 (1007), `avvio` 13-13 (1). Primo frame
diverso: `nessuno` solo per `avvio` e `cod_41_ff`; 211 per `cod_00_07`, 1190
per `gioco`, 120 per `cod_40`/`lungo_40`, 121 per tutti gli altri.
L'elenco degli intervalli senza la parte «primo frame» ha md5
`8ed1dd030384a5c283dee5a0c648e8cf`. Regressioni: `harness --tutte` → 112
righe `RIEPILOGO`, uscita 0; principale `unione: 235 su 299 (78.6%)`.

## Struttura da realizzare

Tutto in `/home/rufio72/OpenSolomonsKey-master/decomp/`.

1. **`harness/audio_interprete.c`** (stima 350-500 righe): `aud_0147` e, come
   `static`, `aud_01b2` (glissando), `aud_0172` (inviluppo software),
   `aud_01e8` (ciclo di lettura 01EF), `aud_0217` (nota), `aud_024a`,
   `aud_0250`, i 16 gestori `aud_03ee` … `aud_05a6`. Gestore: `static int
   aud_xxxx(uint16_t ix)` che ritorna 1 = torna a 01EF, 0 = fine del blocco.
   Smistamento con una tabella C di 16 puntatori indicizzata da `b & 0F`;
   accanto un array degli indirizzi originali (03EE … 05A6) confrontato una
   volta con le parole lette dalla ROM a 03CE (`exit(2)` con messaggio se
   diverse). `static uint16_t de` per il registro DE, aggiornato dove lo
   aggiorna l'originale (elenco sopra); al blocco 0 vale 0 (non modellabile,
   commentarlo). Tutte le letture con `aud_leggi`, le scritture alla RAM con
   `AM()` o `aud_scrivi`; FD scrive `AM(AUD_COMANDO)`. F3, FD, FE con
   commento `non osservato` nel commento di testa. Se il file supera 800
   righe: i 16 gestori vanno in `harness/audio_gestori.c`, con `aud_0250`,
   `aud_024a` e la variabile `de` esportati da una sezione «Interprete,
   interno» di `audio.h` (nome `aud_de`), e `audio_gestori.c` aggiunto a
   `SORGENTI_AUDIO`.
2. **`harness/audio.h`**: `#define` dei campi mancanti (`BL_INV_CONT 0x05`,
   `BL_INV_PUNT 0x06`, `BL_GLISSA 0x0D`, `BL_DUR_NOTA 0x12`, `BL_INV_INIZIO
   0x14`, `BL_RIP_PUNT 0x18`, `BL_RIP_CONT 0x1A`, `BL_RIT 0x1B`; nomi liberi
   purché in italiano), tabelle `AUD_TAB_GESTORI 0x03CE`, `AUD_TAB_TONI
   0x0700`, `AUD_TAB_TONI_B 0x0760`, `AUD_TAB_DURATE 0x0790`,
   `AUD_TAB_INVILUPPI 0x0850`; commento di `BL_FLAG` con tutti i bit (0 attivo,
   1 da spegnere, 2 coperto, 3 muto, 4 glissando, 5-6 inviluppo hardware);
   commento di `aud_0147` aggiornato. Costanti del modello del comando 40
   (punto 3) e `int aud_00fa_in_corso(void);`.
3. **`harness/audio_ossatura.c`, comando lungo** (vedi la domanda aperta 1;
   piano proposto): `aud_00fa` diventa riprendibile, con `de` di 00FA e
   «in corso» in variabili `static`, e un bilancio di cicli: costo per giro
   contato dal dasm (`AUD_CICLI_GIRO`, `AUD_CICLI_GIRO_ALTO` per blocco ≥ 9),
   `AUD_CICLI_IRQ 25600`, costo dell'IRQ annidato e cicli disponibili nello
   slot d'avvio come due costanti da tarare sugli oracoli. `aud_0038`: se
   00FA è in corso → IRQ annidato (`aud_0147(); aud_00e4();`) e poi 00FA
   riprende per il bilancio dello slot; altrimenti come oggi, e se 00E4 avvia
   00FA questo gira col bilancio dello slot d'avvio. Per i comandi normali
   (pochi terne) 00FA deve finire nello stesso slot: comportamento identico
   a oggi. In `audio_banco.c` il ciclo diventa
   `aud_0038(); if (!aud_00fa_in_corso()) aud_029e();`.
4. **`audio.lua`**: variabile `AUDIO_FOTO=1` (documentata in testa): nella
   callback, **subito dopo** `frames = frames + 1` e prima di ogni altra
   azione, se `piano[frames - 1]` o `piano[frames]` esiste, scrive in
   `out/audio/<nome>_foto.txt` la riga `frame\t4000\t<esadecimale>` con
   frame = `frames - 1` (frame appena finito) e i 580 byte 4000-4243 letti
   con `audio.spaces["program"]:read_u8` (1160 cifre maiuscole);
   intestazione `# frame\tinizio\tbyte`. Senza `AUDIO_FOTO` il comportamento
   e le uscite restano identici.
5. **`audio.sh`**: blocco `foto()`: `rm -f out/audio/foto_01_40_*` e
   `AUDIO_FOTO=1 AUDIO_CODICI=01-40 AUDIO_PASSO=$PASSO corsa foto_01_40`
   (nessuna traccia); voce `foto` nel `case`, nel commento d'uso e nel
   messaggio d'errore; **non** aggiunto a `tutto`.
6. **`harness/audio_banco.c`**: se esiste `out/audio/<nome>_foto.txt`, a fine
   di ognuno dei suoi frame confronta `aud_ram[0x000..0x243]` con la foto e
   stampa dopo la seconda riga
   `<nome>: foto RAM <n> frame, primo frame diverso: nessuno|<F> (<AAAA>)`
   (AAAA = primo indirizzo diverso); uscita 1 se diverso. Senza il file
   l'uscita resta esattamente quella di oggi (tre righe). Sotto 300 righe.
7. **`AUDIO-SEQUENZE.md`**: letture sostituite da quelle verificate qui (FB
   a 12 giri, F0 coi valori 20/40/60, `de` in 0286, durata d+1 IRQ, nessuna
   fine del glissando), «da confermare» tolto ovunque, F3/FD/FE «tradotti dal
   dasm, non osservati»; sezione «Blocchi e voci» (9 voci × 2 strati,
   +1D-+1F inutilizzati, pila 47DE-47FF, 46E0); sezione «Comando 40».
8. **`AUDIO.md`**: sezione «Interprete tradotto» (file, risultato del banco
   su tutti gli oracoli, foto RAM, modello del comando 40 con le costanti
   tarate, limite di `de` al blocco 0).

## Verifiche obbligatorie (da `decomp/`)

Prima di modificare `audio.lua`: `cp out/audio/avvio_ay.txt out/audio/avvio_ay.prima`.

1. `make -C harness clean all 2>&1 | grep -E 'audio_[a-z]*\.[ch].*(warning|error)'`
   → vuoto; `ls harness/audio_banco` esiste.
2. `./harness/harness --tutte > /dev/null; echo $?` → `0`;
   `./harness/harness --tutte | grep -c '^RIEPILOGO'` → `112` (misurato).
3. `python3 copertura_call.py $(ls out/*_eseguiti.txt | grep -v unione) | tail -1`
   → `unione: 235 su 299 (78.6%)` (misurato).
4. Banco su tutti gli oracoli:
   `for f in $(ls out/audio/*_latch.txt | grep -v foto_); do n=$(basename $f _latch.txt); ./harness/audio_banco $n 2>/dev/null | sed -n 2p; done > /tmp/b.txt; grep -c 'diverso: nessuno' /tmp/b.txt; sed 's/, primo.*//' /tmp/b.txt | md5sum`
   → `44` e `8ed1dd030384a5c283dee5a0c648e8cf` (md5 misurato oggi; il 44
   è l'obiettivo, attesa, non misurata). Anche le uscite: tutte `0`.
5. `./harness/audio_banco cod_00_07 | sed -n 2p` →
   `cod_00_07: frame 13-759 confrontati 747, primo frame diverso: nessuno`;
   `./harness/audio_banco gioco | sed -n 2p` →
   `gioco: frame 13-2199 confrontati 2187, primo frame diverso: nessuno`
   (intervalli misurati, esito atteso).
6. `./harness/audio_banco cod_40 | sed -n 2p` e `lungo_40` →
   `… frame 13-207 confrontati 195, primo frame diverso: nessuno` (attesa,
   non misurata; dipende dal punto 3 della struttura).
7. `./harness/audio_banco cod_41_ff; echo $?` → esattamente
   `cod_41_ff: avvio 85 cambi, sequenza uguale`,
   `cod_41_ff: frame 13-500 confrontati 488, primo frame diverso: nessuno`,
   `PORTE IGNOTE 12=128 13=64`, `0` (misurato nel lotto precedente).
8. `sh audio.sh avvio > out/audio/avvio.log 2>&1; cmp out/audio/avvio_ay.txt out/audio/avvio_ay.prima && echo uguale; rm out/audio/avvio_ay.prima`
   → `uguale`; nel log `PORTE IGNOTE 12=128 13=64` e nessun `ERRORE:`.
9. `sh audio.sh foto > out/audio/foto.log 2>&1`; `grep -c '^CODICE' out/audio/foto.log`
   → `64`; `grep -c ERRORE: out/audio/foto.log` → `0`;
   `grep -vc '^#' out/audio/foto_01_40_foto.txt` → `128`;
   `awk '!/^#/{print length($3)}' out/audio/foto_01_40_foto.txt | sort -u`
   → `1160` (attese, non misurate; durata attesa sotto un minuto).
10. `./harness/audio_banco foto_01_40; echo $?` → seconda riga con
    `primo frame diverso: nessuno`, terza
    `foto_01_40: foto RAM 128 frame, primo frame diverso: nessuno`, uscita
    `0` (attesa, non misurata: è la verifica di 00FA rinviata dal lotto 2).
11. `grep -c 'non osservato' harness/audio_interprete.c harness/audio_gestori.c 2>/dev/null`
    → almeno 3 in totale; `grep -c 'da confermare' AUDIO-SEQUENZE.md` → `0`;
    `grep -c '2 strati\|secondo strato' AUDIO-SEQUENZE.md` → ≥ 1.
12. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale" && $2!="total"'` → vuoto.
13. `ls out/audio/*trace* 2>/dev/null` → vuoto; `find .. -size +50M -not -path '../roms/*'`
    → vuoto; `pgrep -c mame` → `0`.
14. `grep -n -e '-window' -e 'video soft' -e imgui audio.sh audio.lua` → vuoto.
15. `git -C .. status --short` → solo `decomp/AUDIO.md`,
    `decomp/AUDIO-SEQUENZE.md`, `decomp/audio.lua`, `decomp/audio.sh`,
    `decomp/harness/audio*.{c,h}`, eventualmente `decomp/harness/Makefile`
    (solo con `audio_gestori.c`) e questo brief.

Se la 4 non arriva a 44, non si allenta il criterio: si riportano gli
oracoli e il primo frame diverso di ciascuno, con la causa trovata.

## Consegna

- I file della «Struttura»; in `out/audio/` solo derivati (`banco/`,
  `foto_01_40_*`, `avvio_*`). Nessun commit; se richiesto, messaggio in
  italiano con riga `Co-Authored-By: <modello> <noreply@anthropic.com>`.
- Rapporto finale, massimo trenta righe: uscita delle verifiche 1-15,
  righe dei file C, valori tarati delle costanti del comando 40 e come
  sono stati trovati, oracoli diversi se ce ne sono, deviazioni dal brief
  con motivo.

## Domande aperte

1. «scelta di progetto» Il comando 40 tiene il primo IRQ occupato ~86 frame
   con IRQ annidati e `029E` fermo: il modello attuale (00FA atomico, 2 ×
   IRQ + 029E per frame) non può dare 0 frame diversi su `cod_40` e
   `lungo_40`. Proposta A (scritta nel punto 3): 00FA riprendibile con
   bilancio di cicli e IRQ annidati, due costanti tarate sugli oracoli.
   Alternativa B: 00FA resta atomico e il banco esclude dal confronto
   `cod_40` e `lungo_40` (o i loro frame 120-209), con nota in `AUDIO.md`.
   Il gioco non manda mai il 40. Proposta: A.
2. «preferenza» `de` all'ingresso del blocco 0 vale 0 (in MAME è quello del
   codice interrotto; conta solo con una pausa e il bit 6 prima di ogni
   gestore che cambi `de`). Proposta: 0, documentato.
3. «preferenza» Un solo file `audio_interprete.c`, gestori in
   `audio_gestori.c` solo oltre 800 righe. Proposta: così.
4. «preferenza» Foto della RAM 4000-4243 a fine frame (frame del comando e
   precedente) invece che con un punto d'arresto subito dopo 00E4: stesso
   criterio a fine frame del banco, niente debugger. Corsa `foto_01_40`
   fuori da `tutto`. Proposta: così.
5. «preferenza» Smistamento dei gestori con tabella C controllata contro le
   parole della ROM a 03CE all'avvio. Proposta: così.

## Risoluzioni (2026-10-22)

Scelta di progetto, risolta con l'utente:
1. **Comando 40: opzione B, escluso dal confronto.** `cod_40` e
   `lungo_40` sono esclusi dal banco con motivazione documentata: la voce
   in tabella punta fuori dalla ROM (7A09), il gioco non invia mai il
   codice 40, e il suo comportamento osservato (IRQ occupato ~86 frame
   con IRQ annidati) è un incidente di tempismo a livello di cicli che il
   modello a stato di fine frame deliberatamente non simula. Gli altri
   64 codici restano a confronto pieno.

Preferenze accettate con la proposta dell'analista:
2. `de` vale 0 all'ingresso del blocco 0, documentato.
3. Un solo `audio_interprete.c`; `audio_gestori.c` solo oltre le 800
   righe.
4. RAM fotografata a fine frame (frame del comando e precedente), niente
   punto d'arresto dopo 00E4; la corsa `foto_01_40` resta fuori da
   `tutto`.
5. Gestori smistati con tabella C controllata contro la ROM a 03CE.

## Risoluzione aggiuntiva (2026-10-22, con l'utente)

L'obiettivo «0 frame diversi su tutti i 44 oracoli» del brief si è rivelato
irraggiungibile senza simulazione a cicli (attesa sbagliata del brief, non
della traduzione). Deciso con l'utente:
- il banco ignora il registro 6 (periodo del rumore) quando è inudibile,
  cioè quando il rumore è spento nel mixer per tutti i canali del chip:
  con questo criterio, documentato in AUDIO.md, 9 oracoli diventano
  confronti pieni (40 su 44 identici per intero);
- restano 4 eccezioni di tempismo fine, documentate una per una con il
  primo frame diverso atteso: cod_08_0f (301), cod_20_27 (571),
  cod_38_3f (751) — confine del frame a metà passata di 029E — e gioco
  (1189), latch in punti diversi del frame; più la seconda riga della
  corsa foto (931), stessa causa;
- niente modello a cicli in questo lotto; l'arbitro delle 4 eccezioni è
  il test finale dei .wav generati dal C (lotto renderizzatore). Se
  all'ascolto o al grafico qualcosa stona, si riapre il punto con un
  modello a cicli mirato.

### Correzione alla risoluzione aggiuntiva (stesso giorno)

La maschera del registro 6 ha rivelato che uno dei 9 casi «fantasma»
(cod_28_2f) nasconde un'eccezione vera: lì il rumore è acceso e due voci
scrivono il registro 6 a turno (05 e 17); il frame di MAME finisce tra le
due scritture. Stessa causa delle altre (passata di 029E tagliata dal
fine frame). Consuntivo finale: 39 oracoli su 44 identici per intero;
5 eccezioni di tempismo documentate in tabella in AUDIO.md — cod_08_0f
(301), cod_20_27 (571), cod_28_2f (752), cod_38_3f (751), gioco (1189) —
più la seconda riga della foto (931).

## Esito (2026-10-22): PASSATO (con gli attesi corretti dalle risoluzioni)

Ciclo in locale: analista Opus ~128k token, sviluppatore Opus ~158k
(due riprese), verificatore Sonnet ~19k. L'interprete delle sequenze è
tradotto per intero in `audio_interprete.c` (438 righe): aud_0147,
glissando, inviluppo software, lettura sequenze, 16 gestori smistati da
tabella C controllata contro la ROM a 03CE; F3, FD, FE tradotti dal
disassemblato e marcati «non osservato». Stato del sequencer a 9 voci ×
2 strati.

Risultato del banco: 39 oracoli su 44 identici per intero (0 frame
diversi per tutta la durata, musiche da 900 frame comprese); comando 40
escluso (risoluzione 1); registro 6 ignorato quando il rumore è spento
su tutto il chip (criterio in AUDIO.md); 5 eccezioni di tempismo fine
attese e documentate con causa e primo frame diverso in tabella in
AUDIO.md (301, 571, 752, 751, 1189; foto 931). Foto RAM dell'avvio dei
comandi: 127/127 uguali. Regressioni invariate: harness 112/0,
principale 235/299.

La verifica 8 del brief è difettosa: `audio.sh avvio` fa
`rm -f out/audio/avvio_*` e cancella il riferimento prima del `cmp`.
Lo sviluppatore ha validato l'uguaglianza con una copia fuori repo;
l'equivalenza dell'avvio resta coperta dalla verifica 7 (sequenza di 85
cambi identica nel banco). Da sistemare se si riusa il comando.

PUNTO APERTO (richiesto dall'utente): le 5 eccezioni di tempismo si
giudicano col test dei .wav generati dal C (lotto renderizzatore); se
qualcosa stona, modello a cicli mirato su 029E/0147/00E4.
