# Compito 22: Z80 audio, lotto 5: renderizzatore wav

Data: 2026-10-22. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Trasformare i flussi dei cambi di registro (formato `_ay.txt`) in file
`.wav` con un emulatore YM2149 preso così com'è (emu2149, MIT). Poi
generare i wav dei comandi 01-3F dal C del driver, provare che C e
oracolo MAME danno wav identici byte per byte sui 39 oracoli già
equivalenti, e misurare la differenza sulle 5 eccezioni di tempismo.

Vincoli assoluti:
- Modifiche solo in `decomp/`, più `NOTICE.md` (provenienza di emu2149) e
  `.gitignore` (nuovi eseguibili). Non toccare `roms/`, `compiti/`,
  `STATO.md`, `README.md`, `audio.lua`, `audio.sh`, `audio_banco.c`,
  `audio_ossatura.c`, `audio_registri.c`, `audio_interprete.c`,
  `audio_piattaforma.c`, `audio_dati*.c/.h`. Nessuna operazione git (il
  commit lo fa la chat principale).
- Niente corse MAME, niente `sh audio.sh` in nessuna forma (il blocco
  `avvio` cancella `out/audio/avvio_*`): gli oracoli in `out/audio/` si
  leggono soltanto.
- Nel repo entrano solo testo C/Python/shell/Markdown. Nessun `.wav`, nessun
  byte della ROM. I wav vanno in `decomp/out/wav/` (già ignorata da
  `decomp/out/*` in `.gitignore`).
- `gcc` 13.3 con le `CFLAGS` del `Makefile` (`-std=c11 -Wall -Wextra -O0`):
  nessun warning nei file nuovi. Python 3 standard, senza pacchetti (niente
  numpy). Unico scaricamento ammesso: i tre file di emu2149 al commit
  indicato sotto.
- Commenti, messaggi e documenti in italiano (i file di emu2149 restano
  intatti, in inglese). Nessun file di codice sopra le 800 righe.

## Cosa esiste già (fatti verificati)

### Codice (da `grafo outline`/`symbol` e grep)

| File | Righe | Ruolo |
|---|---|---|
| `harness/audio.h` | 96 | prototipi: `aud_carica_rom` (65), `aud_usa_dati_estratti(int modo)` (66: 1 estratti, 2 tutta AA), `aud_azzera` (67), `aud_flusso_apri(const char*)` (74, 0 se fallisce), `aud_flusso_codice(int frame, uint8_t cc)` (75), `aud_flusso_chiudi` (76), `extern int aud_frame` (64), `aud_avvio` (81), `aud_0038` (83), `aud_0066(uint8_t latch)` (84), `aud_029e` (91) |
| `harness/audio_piattaforma.c` | 156 | `aud_out` (102): riga `%d\t%X\t%02X\t%02X` nel flusso solo se il valore del registro cambia; intestazione `# frame\tchip\treg\tval`; ROM da `../roms/ext/solomon/1.3jk` |
| `harness/audio_banco.c` | 281 | `main` (143): `audio_banco <nome> [frame_max]` da `decomp/`; gestione di `AUDIO_DATI` (righe ~150-157, da copiare); ciclo del tempo: avvio al frame 0, poi per frame `f`: `aud_frame = f`, latch del frame (prima `aud_flusso_codice` se forzato, latch prima del 14 ignorati, poi `aud_0066`), poi 2 volte `aud_0038(); aud_029e();`. Scrive `out/audio/banco/<nome>_ay.txt` fino a `frame_fine` = max(ultimo latch, ultimo cambio dell'oracolo) |
| `harness/Makefile` | 27 | tracciato (malgrado `Makefile` in `.gitignore`); `SORGENTI_AUDIO`, `SORGENTI_DATI`, `INTESTAZIONI_DATI`; `all: harness sched_proto audio_banco audio_immagine`; `clean` |
| `estrai_dati.py` | 619 | genera `SPARTITI.md`; la tabella riassuntiva è scritta a righe 486-506 (riga 505: `'\| %02X \| \| %04X ...'`, colonna «nome» vuota); la legenda (righe ~486-498) dice «i nomi si assegnano nel lotto dei .wav» |
| `AUDIO.md` | 252 | quadro audio; si aggiunge in fondo «Renderizzatore (brief 22)» |
| `NOTICE.md` | 14 | punti 1 e 2; si aggiunge il punto 3 |

`SPARTITI.md` è generato da `estrai_dati.py` («non modificare a mano»):
qualunque cambiamento passa dal generatore.

### Hardware (da `mame -listxml solomon` e `src/mame/tecmo/solomon.cpp` di mame0264)

- Tre `YM2149 SSG` `ay1-3` a **1 500 000 Hz**; altoparlante **mono**;
  schermo a **60,000000 Hz**.
- Righe 383-385 del driver:
  `YM2149(config, "ayN", 1'500'000).add_route(ALL_OUTPUTS, "mono", 0.12);`
  quindi tutti i 9 canali sommati con lo stesso peso.

### Oracoli e flussi (misurati)

- Formato: riga `frame chip reg val` (decimale, 1-3, esa, esa) solo al
  cambio; `# codice <frame> <cc>` prima di un codice forzato; prima riga
  `# frame\tchip\treg\tval`. Registri usati 00-0D.
- Oracoli: 44 file `out/audio/*_ay.txt` più `foto_01_40_ay.txt` (fuori
  dal confronto). I flussi del C in `out/audio/banco/` **possono essere
  vecchi** (trovati quelli del controllo negativo `AUDIO_DATI=vuoto` su
  `cod_00_07`): vanno sempre rigenerati con
  `AUDIO_DATI=estratti ./harness/audio_banco <nome>` prima di usarli.
- Le righe non coincidono (ordine dentro il frame, avvio spalmato sui
  frame 0-13 in MAME e tutto al frame 0 nel C), ma lo **stato a fine
  frame** sì. Misurato con uno script di prova, applicando la regola del
  registro 6 descritta sotto, frame 14 → ultimo frame dell'oracolo:

| oracoli | frame con stato applicato diverso |
|---|---|
| i 39 (tutti i `lungo_*` 0C-3E, `cod_00_07`, `cod_10_17`, `cod_18_1f`, `cod_30_37`, `avvio`, `cod_41_ff`; `cod_40` e `lungo_40` fino al 119) | **0** |
| `cod_08_0f` | 2 (primo 301, ultimo 302) |
| `cod_20_27` | 2 (571, 661) |
| `cod_28_2f` | 50 (752-838) |
| `cod_38_3f` | 1 (751) |
| `gioco` | 478 (1189-2199) |

- Senza la regola del registro 6 differirebbero anche `cod_10_17` (44
  frame), `cod_18_1f` (12), `lungo_14` 42, `lungo_16` 64, `lungo_19` 60,
  `lungo_1C` 50, `lungo_26` 5, `lungo_28` 70: periodo del rumore scritto
  mentre il rumore è spento su tutto il chip (criterio già in `AUDIO.md`).
- Ultimo frame degli oracoli: `cod_00_07` 759, `cod_08_0f` 839,
  `cod_10_17` 838, `cod_18_1f` 839, `cod_20_27` 795, `cod_28_2f` 839,
  `cod_30_37` 815, `cod_38_3f` 752, `gioco` 2199, `avvio` e `cod_41_ff` 13,
  `lungo_*` fra 121 e 1019. Le corse `lungo_*` hanno un solo codice
  forzato al frame 120. `gioco` non ha righe `# codice` (latch d'origine
  `gioco`; al 1188 il gioco invia 3F).
- Registro 0D (forma dell'inviluppo hardware) scritto solo in `cod_28_2f`
  e `lungo_2F` (2 righe ciascuno; comando 2F).
- Durate: `SPARTITI.md`; 15 comandi in ciclo `13 14 16 19 1A 1B 1C 1E 1F
  20 21 23 24 28 2A`; il 3D dura 1800 frame senza ciclo; 00 = arresto,
  41-FF ignorati (0 cambi), 40 escluso (fuori modello, mai inviato dal
  gioco).

### emu2149 (scaricato e controllato)

- `https://github.com/digital-sound-antiques/emu2149`, licenza **MIT**
  («Copyright (c) 2014 Mitsutaka Okazaki»; intestazione del `.c`: v1.42,
  2001-2022), commit `441f3dc295a9b82db16921461f0678b483db8304`.
  `emu2149.c` 420 righe, `emu2149.h` 94, `LICENSE` 21: sotto le 800,
  nessuna eccezione. md5: `f6c88fadf87d15578d58fd1268c861d2 emu2149.c`,
  `c3fcde853d5d55bf18f09af029ce16b8 emu2149.h`,
  `f02cc1b74a109c83b11a710042ea3dc7 LICENSE`. Con le CFLAGS del progetto
  `gcc -fsyntax-only` non dà avvisi.
- API: `PSG *PSG_new(uint32_t clk, uint32_t rate)`, `PSG_reset`,
  `PSG_delete`, `PSG_setVolumeMode(psg, 1)` = tabella YM2149 a 32 passi,
  `PSG_setQuality(psg, 1)` = convertitore di frequenza,
  `PSG_writeReg(psg, reg, val)`, `int16_t PSG_calc(psg)` = somma dei 3
  canali, al massimo 3 × (0xFF << 4) = 12240. Interi, deterministico.
- Il generatore di rumore avanza sempre (riga 267), anche a rumore
  spento: per questo la regola del registro 6 è necessaria.
- Scartato ayumi (MIT, 338 righe): virgola mobile e filtri; emu2149 è
  intero e ha la tabella YM2149 esplicita.

## Struttura da realizzare

### 1. `harness/emu2149/` (vendorizzato)

Scaricare i tre file al commit indicato
(`https://raw.githubusercontent.com/digital-sound-antiques/emu2149/441f3dc295a9b82db16921461f0678b483db8304/<file>`)
in `decomp/harness/emu2149/`, senza modificarli; controllare gli md5.
`NOTICE.md`: punto 3 con nome, autore, licenza MIT, URL, commit, percorso,
«non modificato».

### 2. `harness/audio_wav.c`: il renderizzatore

`./harness/audio_wav <flusso_ay.txt> <uscita.wav> [da [a]]` (da `decomp/`).
- Legge il flusso: ignora le righe che iniziano con `#` tranne
  `# codice F CC`. `da` predefinito = frame del primo `# codice`, se non
  c'è 14; `a` predefinito = frame dell'ultima riga di dati. `a < da`:
  errore, uscita 2. File illeggibile: uscita 2.
- Tre `PSG_new(1500000, 44100)`, `PSG_setVolumeMode(p, 1)`,
  `PSG_setQuality(p, 1)`, `PSG_reset(p)`.
- Stato del flusso `st[chip][reg]` (−1 = mai scritto) e stato scritto nel
  chip `sc[chip][reg]` (−1). Per ogni frame `f` da 0 ad `a`: applica a `st`
  tutte le righe del frame `f`; se `f < da` passa oltre (niente campioni,
  niente scritture). Altrimenti per ogni chip e registro 0-13 tranne 6:
  se `st ≥ 0` e `st ≠ sc`, `PSG_writeReg` e aggiorna `sc`. **Registro 6**:
  lo scrive (se `st ≥ 0` e diverso da `sc`) solo se il rumore è acceso su
  almeno un canale del chip, cioè `(r7 & 0x38) != 0x38` con `r7` = `sc`
  del registro 7 (mai scritto = 0xFF). Poi 735 campioni (44100/60):
  `s = (PSG_calc(p1) + PSG_calc(p2) + PSG_calc(p3)) * 7 / 8` in `int32_t`
  (picco 32130, nessun taglio; se si preferisce si limiti comunque a
  ±32767).
- Uscita: RIFF/WAVE PCM canonico, intestazione di 44 byte scritta a mano
  in little endian, 44100 Hz, 16 bit, 1 canale; campioni `(a−da+1)×735`.
- Limiti noti, da scrivere in testa al file e in `AUDIO.md`: i cambi si
  applicano a confine di frame (il tempo dentro il frame si perde, uguale
  per C e MAME); una riscrittura del registro 0D con lo stesso valore (che
  sul chip riavvia l'inviluppo) non compare nei flussi.
- Obiettivo < 250 righe.

### 3. `harness/audio_suona.c`: un comando dal C

`./harness/audio_suona <cc> <frame_fine> <uscita_ay.txt>` (cc esadecimale).
Stessa sequenza di `audio_banco.c` senza oracolo: `aud_carica_rom()`,
gestione di `AUDIO_DATI` copiata dal banco (stessa riga `dati: estratti`
su stderr), `aud_azzera()`, `aud_flusso_apri`, `aud_frame = 0;
aud_avvio();`, poi per `f` da 0 a `frame_fine`: `aud_frame = f`; se
`f == 120`: `aud_flusso_codice(120, cc); aud_0066(cc);`; poi 2 volte
`aud_0038(); aud_029e();`. Infine `aud_flusso_chiudi()`. Il frame 120 è
lo stesso degli oracoli `lungo_*` (costante `FRAME_CODICE`). Obiettivo
< 90 righe. Il suo flusso deve essere identico byte per byte a
`out/audio/banco/lungo_<cc>_ay.txt` a parità di `frame_fine`.

### 4. `Makefile` e `.gitignore`

`audio_wav: audio_wav.c emu2149/emu2149.c emu2149/emu2149.h` e
`audio_suona: audio_suona.c` + i sorgenti audio del banco senza
`audio_banco.c` (definire `SORGENTI_DRIVER` e riusarlo in
`SORGENTI_AUDIO`); aggiungerli ad `all` e a `clean`. In `.gitignore`:
`decomp/harness/audio_wav` e `decomp/harness/audio_suona`.

### 5. `audio_wav_confronto.py`

`python3 audio_wav_confronto.py <a.wav> <b.wav> [da]` (da = frame del
primo campione, predefinito 14). Stampa una riga:
`<a> <b>: campioni <na>/<nb>, diversi <k>, primo <i> (frame <F>), ultimo <j> (frame <G>), scarto massimo <m>, frame diversi <n>, rms differenza <x> dBFS`
(frame = `da + campione // 735`; se `k = 0`: `<a> <b>: identici, campioni <n>`).
Uscita 0 se identici, 1 se no. Solo `wave` e `array`.

### 6. `audio_wav.sh` (da `decomp/`)

- `sh audio_wav.sh codici`: per cc 01-3F, `AUDIO_DATI=estratti
  ./harness/audio_suona cc 2000 out/wav/flussi/<cc>_ay.txt`, poi
  `./harness/audio_wav` da 120 ad `a = min(ultimo frame di dati + 30,
  2000)` in `out/wav/codici/<cc>.wav`. Esclusi 00, 40, 41-FF (dirlo in
  `AUDIO.md`). Stampa una riga per codice: `cc frame_da frame_a secondi`.
- `sh audio_wav.sh oracoli`: per ognuno dei 44 oracoli (non `foto_01_40`)
  rigenera il flusso del C con `AUDIO_DATI=estratti ./harness/audio_banco
  <nome> >/dev/null`, poi rende oracolo e C con `da = 14` e `a` = ultimo
  frame di dati dell'oracolo (119 per `cod_40`, `lungo_40`; 120 se
  l'ultimo frame è < 14) in `out/wav/oracoli/<nome>_mame.wav` e
  `<nome>_c.wav`, e li confronta con `cmp -s`. Stampa
  `<nome> uguali|diversi` e in fondo `uguali N diversi M`. Per i diversi
  stampa anche la riga di `audio_wav_confronto.py` e la copia in
  `out/wav/eccezioni.txt`.

### 7. Nomi dei suoni

`estrai_dati.py` legge, se esiste, `decomp/audio_nomi.tsv` (righe
`cc<TAB>nome`, `#` = commento, nome vuoto ammesso, `|` sostituito da `/`)
e mette il nome nella colonna «nome» della tabella riassuntiva. Creare
`audio_nomi.tsv` con un commento d'intestazione (ascoltare
`out/wav/codici/<cc>.wav`, scrivere il nome, rieseguire `python3
estrai_dati.py`) e le 63 righe `01`-`3F` con nome vuoto. Cambiare la frase
della legenda «si assegnano nel lotto dei .wav, ascoltandoli» in «si
scrivono in `audio_nomi.tsv` ascoltando `out/wav/codici/<cc>.wav`». Con
nomi vuoti la tabella non cambia.

### 8. Documenti

`AUDIO.md`, sezione «Renderizzatore (brief 22)»: emulatore e commit,
clock, mix, 735 campioni per frame, regola del registro 6, limiti, comandi
d'uso, esito di `oracoli` (N uguali) e la tabella delle 5 eccezioni con la
riga del confronto (campioni diversi, primo/ultimo frame, scarto massimo,
rms). Correggere la riga 173 («futuro renderizzatore»). L'ascolto finale
è per le coppie `out/wav/oracoli/<nome>_mame.wav` / `_c.wav`.

## Verifiche obbligatorie (da `decomp/`)

1. `make -C harness all 2>&1 | grep -iE 'warning|error'` → vuoto.
2. `cd harness/emu2149 && md5sum emu2149.c emu2149.h LICENSE` → i tre
   md5 sopra.
3. `for c in 0C 0D 0F 13 14 15 16 17 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 26 28 29 2A 2B 2C 2D 2E 2F 3D 3E; do AUDIO_DATI=estratti ./harness/audio_banco lungo_$c >/dev/null 2>&1; f=$(tail -n1 out/audio/banco/lungo_${c}_ay.txt | cut -f1); AUDIO_DATI=estratti ./harness/audio_suona $c $f /tmp/s_$c.txt 2>/dev/null; cmp -s /tmp/s_$c.txt out/audio/banco/lungo_${c}_ay.txt || echo diverso $c; rm -f /tmp/s_$c.txt; done`
   → vuoto (attesa, non misurata: stesso ciclo del banco). Se l'ultimo
   cambio del C precede l'ultimo latch, `f` è comunque ≥ 120 per tutti
   (ultimi frame 121-1019).
4. `sh audio_wav.sh oracoli | tail -n1` → `uguali 39 diversi 5`; i
   diversi sono `cod_08_0f cod_20_27 cod_28_2f cod_38_3f gioco`
   (attesa: precondizione misurata, stato applicato uguale in 39/39; wav
   non misurati).
5. `cat out/wav/eccezioni.txt` → 5 righe; per ciascuna il primo campione
   diverso è ≥ (F − 14) × 735 con F = 301, 571, 752, 751, 1189, cioè
   ≥ 210945, 409395, 542430, 541695, 863625 (attesa, non misurata; può
   anche risultare `identici` se il frame diverso cade su volumi a zero:
   in quel caso il riepilogo della verifica 4 lo conta fra gli uguali e va
   riportato come tale, non è un errore).
6. `python3 audio_wav_confronto.py out/wav/oracoli/lungo_13_c.wav out/wav/oracoli/lungo_13_mame.wav; echo $?`
   → `... identici, campioni 739410` e `0` ((1019 − 14 + 1) × 735).
7. `sh audio_wav.sh codici | wc -l; ls out/wav/codici/*.wav | wc -l` →
   `63` e `63`. Per i 15 comandi in ciclo la riga dice `120 2000`
   (attesa, non misurata) e il file è di 44 + 2 × 1881 × 735 = 2765114
   byte.
8. `python3 -c "import wave;w=wave.open('out/wav/codici/13.wav');print(w.getframerate(),w.getsampwidth(),w.getnchannels())"`
   → `44100 2 1`.
9. Due corse uguali: `cp out/wav/codici/2F.wav /tmp/a.wav && sh audio_wav.sh codici >/dev/null && cmp out/wav/codici/2F.wav /tmp/a.wav && echo stabile; rm -f /tmp/a.wav`
   → `stabile`.
10. `python3 estrai_dati.py && git status --short` → solo i file attesi
    dal compito (nessun `harness/audio_dati*` modificato); `git diff
    SPARTITI.md` → solo la frase della legenda.
11. `find . -path ./out -prune -o \( -name '*.c' -o -name '*.h' -o -name '*.py' -o -name '*.sh' \) -print | xargs wc -l | awk '$1>800 && $2!="totale"'`
    → vuoto.
12. `git status --short` (dalla radice) → solo: `.gitignore`, `NOTICE.md`,
    `decomp/AUDIO.md`, `decomp/SPARTITI.md`, `decomp/estrai_dati.py`,
    `decomp/harness/Makefile`, e i nuovi `decomp/harness/audio_wav.c`,
    `decomp/harness/audio_suona.c`, `decomp/harness/emu2149/`,
    `decomp/audio_wav_confronto.py`, `decomp/audio_wav.sh`,
    `decomp/audio_nomi.tsv`. Nessun `.wav`, nessun eseguibile.
13. Regressione del banco: `for n in cod_00_07 cod_28_2f lungo_13; do AUDIO_DATI=estratti ./harness/audio_banco $n 2>/dev/null | sed -n 2p; done`
    → `cod_00_07: frame 13-759 confrontati 747, primo frame diverso: nessuno`,
    `cod_28_2f: frame 13-839 confrontati 827, primo frame diverso: 752`,
    `lungo_13: ... primo frame diverso: nessuno` (le prime due misurate).

## Consegna

File sopra, nessun commit (lo fa la chat principale con
`Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`). Rapporto di
al massimo trenta righe: uscita delle verifiche, le 5 righe di
`eccezioni.txt`, le durate dei wav più lunghi e più corti, deviazioni dal
brief.

## Domande aperte

1. **Emulatore**: emu2149 (intero, tabella YM2149) invece di ayumi.
   Proposta: emu2149. «preferenza»
2. **Formato e mix**: 44100 Hz, 16 bit, mono, somma dei 3 chip × 7/8,
   come la scheda (mono, stesso peso 0,12). Proposta: così. «preferenza»
3. **Regola del registro 6** nel renderizzatore (scrittura rimandata
   finché il rumore del chip è spento): rende uguali per costruzione gli 8
   oracoli col periodo del rumore scritto a rumore spento; si scosta dal
   chip solo nella fase del generatore di rumore, inudibile. Proposta:
   regola sempre attiva, senza opzione. «preferenza»
4. **Durata dei wav dei comandi**: fino all'ultimo cambio + 30 frame,
   massimo 2000 (31,35 s dal codice) per i comandi in ciclo e il 3D;
   la lunghezza di un giro non è nota. Proposta: così. «preferenza»
5. **Codici**: solo 01-3F (63 wav); niente 00 (silenzio), 40 (fuori
   modello), 41-FF (nessun cambio). Proposta: così. «preferenza»
6. **Nomi dei suoni** in `audio_nomi.tsv` letto da `estrai_dati.py`,
   perché `SPARTITI.md` è generato e non si modifica a mano. Proposta:
   così. «preferenza»
7. **Campioni wav nel repo**: nessuno; si rigenerano in pochi secondi.
   Proposta: nessun wav committato. «preferenza»
8. **Giudizio sulle 5 eccezioni**: questo compito le misura e le prepara
   per l'ascolto; un eventuale modello a cicli di `029E/0147/00E4` resta un
   compito a parte, deciso dopo l'ascolto. «scelta di progetto»

## Risoluzioni (2026-10-22)

8. **Scelta di progetto, già decisa dall'utente** (risoluzione aggiuntiva
   del brief 20 e memoria di progetto): l'eventuale modello a cicli, se
   le 5 eccezioni stonano all'ascolto o al grafico, è un compito a parte,
   fuori da questo lotto.

Preferenze accettate con la proposta dell'analista:
1. emu2149 (commit 441f3dc, MIT) invece di ayumi; provenienza in
   NOTICE.md.
2. Wav a 44100 Hz, 16 bit, mono, tre chip sommati × 7/8.
3. Registro 6 scritto all'emulatore solo quando il rumore del chip è
   acceso (coerente col criterio del banco).
4. Durata dei wav: ultimo cambio + 30 frame, tetto al frame 2000.
5. Solo i codici 01-3F: 63 wav.
6. Nomi dei suoni in audio_nomi.tsv, letto da estrai_dati.py (SPARTITI.md
   è generato).
7. Nessun wav nel repo.

## Esito (2026-10-22): PASSATO

Ciclo in locale: analista Opus ~103k token, sviluppatore Opus ~59k,
verificatore Sonnet ~18k. Tutte le 13 verifiche passate due volte, al
primo colpo dello sviluppatore.

Risultato — il test finale della catena audio:
- i wav renderizzati dal flusso del C e dal flusso MAME sono BYTE PER
  BYTE IDENTICI in 40 confronti su 44; la quinta eccezione del banco
  (cod_38_3f) sparisce ai campioni: il suo unico frame diverso non tocca
  l'audio reso;
- le 4 differenze restanti sono misurate in eccezioni.txt: cod_08_0f
  (197 frame, picco −33,1 dBFS), cod_20_27 (4 frame, −49,3), cod_28_2f
  (86 frame, −40,4), gioco (592 frame sparsi, −37,0). Coppie
  <nome>_mame.wav / <nome>_c.wav in out/wav/oracoli/ per il giudizio
  all'ascolto (punto aperto dell'utente);
- 63 wav dei comandi 01-3F da `sh audio_wav.sh codici` in
  out/wav/codici/, stabili tra corse, non committati;
- emu2149 vendorizzato (commit 441f3dc, MIT, md5 verificati, LICENSE
  conservata con eccezione in .gitignore), clock 1,5 MHz e missaggio
  mono a pesi uguali come da driver MAME;
- nomi dei suoni da battezzare in decomp/audio_nomi.tsv (SPARTITI.md è
  generato e li recepisce via estrai_dati.py).
