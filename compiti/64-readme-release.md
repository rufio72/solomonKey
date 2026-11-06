# Compito 64: README, NOTICE e pacchetto della release v1.01

Data: 2026-11-06. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Dopo i lotti 62-63 il gioco non legge più nessuna ROM (tutto il materiale
Tecmo sta in `tecmo/`). Questo lotto sistema ciò che è rivolto al pubblico:
riscrive `README.md` e `NOTICE.md` (in inglese), prepara in una cartella non
versionata il pacchetto binario `tar.gz` e le note della release v1.01.
La release su GitHub la crea la chat principale: qui non si usa `gh`.

## Vincoli assoluti

- Cartella di lavoro: `/home/rufio72/OpenSolomonsKey-master`. Nessuna
  modifica fuori da essa.
- **Nessun file di codice del gioco si tocca** (`decomp/harness/*.c|h`,
  `tecmo/*`, Makefile, script `.py/.sh`): solo documenti e impacchettamento.
  Se serve toccare codice, ci si ferma e lo si scrive nel rapporto.
- Tutto il materiale pubblico (README.md, NOTICE.md, note e README del
  pacchetto) è IN INGLESE. Brief, STATO.md e documenti interni in italiano.
- Niente `git push`, niente `gh`, niente tag. Niente installazioni.
- Il pacchetto e le note vanno in `decomp/out/release/` (ignorata da git:
  `.gitignore` riga 50 `decomp/out/*`, verificato con `git check-ignore -v`).
  Non vanno committati.
- Nessun file di codice sopra le 800 righe (nessuno si tocca; verifica 9).
- Nessun riferimento a `solomon.zip` o `roms/` nel percorso del giocatore
  (README principale, README del pacchetto, note).

## Cosa esiste già (fatti verificati)

| Fatto | Fonte |
|---|---|
| `README.md` 246 righe, inglese; titolo «solomonKey — reverse engineering…» r.1; copertina `![…](docs/cover.png)` r.3; «Ownership note» r.17-21; sezione «Status (November 2026)»; «Repository layout»; «How to play» r.168-232; «Requirements» r.236-239 (dice di mettere le ROM in `roms/`); «Contributing» | lettura |
| Righe da correggere in README: r.146-147 («prepares everything (ROM extraction, `out/main64.bin`, build)»); r.173-179 (solomon.zip, «four ROM files»); r.182 (`# extracts the zip, builds main64.bin and gioca`); r.236-239; voce «Next up» r.~150 («manual joystick test…» superata) | grep |
| `NOTICE.md` 23 righe, italiano; r.9-11 citano `decomp/harness/audio_dati*.c` e `video_dati_*.c` (ora in `tecmo/`); sezione 3 emu2149 (MIT, Mitsutaka Okazaki, commit `441f3dc295a9b82db16921461f0678b483db8304`, percorso `decomp/harness/emu2149/`) | lettura |
| Promemoria del lotto 63: «restano disallineati `README.md` r. 173-179 e `NOTICE.md` r. 9-11» | `decomp/CENSIMENTO-ROM.md` in fondo |
| `tecmo/` (1,4 MB, versionata): 91 tabelle del Z80 principale, driver audio (`audio_driver.c`), musica (`audio_dati_*.c`), grafica (12 `video_dati_*.c`) | STATO.md r.19, brief 62-63 |
| `roms/decoded/` è versionata (grafica decodificata, dump RAM, screenshot MAME): derivata dalle ROM. `roms/*.zip` e `roms/ext/` sono ignorate (`.gitignore` r.47-48), nessuno zip versionato | `git ls-files`, `.gitignore` |
| Build: `make -C decomp/harness gioco` compila solo `gioca` (nessuna ROM, nessun `unzip`); senza SDL2 dà «ERRORE: SDL2 non trovato. Installa libsdl2-dev…», uscita 2. Il bersaglio `rom` resta strumento di sviluppo | `decomp/harness/Makefile` r.101-122 |
| Strumenti: gcc 13.3.0, SDL2 2.30.0 (`sdl2-config --version`), x86_64 | misurato |
| `decomp/harness/gioca` è ELF dinamico: dipende da `libSDL2-2.0.so.0`, libc, libm (più le dipendenze di SDL: asound, pulse, X11…). Pacchetto Ubuntu runtime: `libsdl2-2.0-0` | `ldd` |
| `decomp/harness/gioca` è ignorato da git (`.gitignore` r.82) | `git check-ignore` |
| Tag esistente: `v1.0-rc1` (2026-11-04, commit `3b851a5`); 15 commit dopo il tag | `git log` |
| Immagini: `docs/cover.png` (1039x954), `docs/special/stanza_00.png`…`stanza_51.png` (≈720x656; 52 stanze + `README.md` = 53 voci in `docs/special/`), `decomp/out/video/{titolo,gioco,bonus,finale,finale2,stanza10,prove}.png` (versionati) | `ls`, `file`, `git ls-files` |
| `docs/special/README.md` descrive le immagini degli oggetti nascosti (inglese) | lettura |
| Repo remoto: `git@github.com:rufio72/solomonKey.git` | `git remote -v` |

### Opzioni e tasti di `gioca` (da `gioca --aiuto` e `gioca --mappa`)

- Schermo: `-s/--scala N` (1-8, default 3), `--pieno`, `--scanlines [S]`
  (S 1-3, default 2; da scala 2; solo presentazione, hash invariati),
  `-n/--headless N` (N frame senza finestra, poi esce), `-m/--max N`.
- Audio: `--muto`, `--volume N` (0-100).
- Ingressi: `--joystick` (diagnostica), `--joy-mappa B1,B2,START,COIN`
  (default `0,1,7,6`), `--senza-joystick`, `--mappa`.
- Partita: `--vite`, `--cabinet`, `--demo`, `--bonus`, `--difficolta`,
  `--timer`, `--extra`, `--dsw1`, `--dsw2`, `--stampa-dsw`.
- Registrazione: `--registra F`, `--riproduci F`, `--hash-frame F,..`,
  `--oracolo O D`.
- Tasti: frecce o WASD; Z, Spazio, Ctrl sinistro = BUTTON1; X, Alt sinistro =
  BUTTON2; 5 o C gettone; 1 o Invio START1 (2 = START2); giocatore 2 IJKL,
  N, M; in gioco P pausa, F2 ricomincia, ESC esce. (Quale tra BUTTON1 e
  BUTTON2 sia salto e quale magia: non verificato, vedi domanda 2.)
- Il testo di `--aiuto` è in italiano e resta tale (codice, non si tocca).

### Uscite misurate di `gioca` (cartella `decomp/`, `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy`)

- `harness/gioca -n 60 --muto` →
  ```
  FRAME rese 60
  HASH ultimo 815bb645bc46a325
  AUDIO campioni 92610
  AUDIO hash 9ce6e788ff2bc0d4
  ```
- `harness/gioca -n 600 --muto` → `FRAME rese 600`,
  `HASH ultimo ecd5acf694285489`, `AUDIO campioni 489510`,
  `AUDIO hash 8aedf41c0424df54` (uguale al brief 63, punto 8).

### Fatti per le note della release (da `compiti/58-63`, `STATO.md`)

Novità rispetto a v1.0-rc1 (tutti già nel repo):
- **Nessuna ROM richiesta** (brief 61-63): il Z80 principale (91 tabelle),
  il driver audio, la musica e la grafica stanno in `tecmo/`; `gioca` non
  apre alcun file ROM; un clone pulito compila e gioca.
- **Cadenza** (brief 58): il motore dei nemici dichiara il proprio costo;
  nelle stanze con nemici la velocità è vicina a MAME (stanza 2: 2,39
  spedizioni del corpo 0A per frame contro 2,43 di MAME; stanza 0: 4,12
  contro 4,88). Prima nella stanza 3 due nemici andavano 2,26 volte più
  veloci. (Tra rc1 e il brief 58 c'è anche il fix stelline, brief 53, già
  in rc1: non citarlo come novità.)
- **Stanze bonus e finale** (brief 59): famiglia di routine 3901/3903
  tradotta, il gioco libero non si ferma più su «routine 3901/3903
  mancante» al cambio stanza; lettura e603 a 4CED modellata come `C & 08`.
  Aggiunta una partita registrata di regressione che supera la stanza 3 ed
  entra nella 4 (commit 0d8b5cb).
- **Mappe degli oggetti nascosti** (brief 60): `docs/special/stanza_00-51.png`,
  128 oggetti nascosti in 40 stanze, chiave della cella E (orange/green
  outline), generate da `decomp/mappe_special.py`.
- **Scanlines** `--scanlines [S]`: già in rc1 (brief 57C); citare come
  opzione, non come novità.
- Banco di prova: 335 voci, 1422 catture, 0 fallite (STATO.md r.18).
- Difetti noti (da riportare in «Known issues»): il gioco libero può
  fermarsi con una riga `FERMO ...` (uscita 3) su codice Z80 non ancora
  tradotto; la cadenza del gioco libero è un'euristica (il fondale della
  stanza ha un ritmo diverso da MAME: ~5 frame); la «bottiglia rossa di
  fuoco» del livello 2 (raccolta, non uccide i nemici) era un bug aperto a
  rc1 e STATO.md non ne annota la chiusura: vedi domanda 3.

## Struttura da realizzare

### A. `README.md` (riscrittura, inglese)

Ordine delle sezioni (l'implementatore sceglie il testo, ma i vincoli sono):

1. Titolo esatto: `# Solomon's Key (Tecmo, 1986)`; sottotitolo (riga di
   testo o in grassetto): `A playable C decompilation, reverse-engineered from the original arcade ROMs`;
   subito sotto la riga di copyright: `Original game © 1986 Tecmo (now Koei Tecmo). This project is an unofficial, non-commercial study and preservation effort; see [NOTICE.md](NOTICE.md).`
2. **Niente immagine a tutta larghezza.** Togliere `![…](docs/cover.png)`.
   Al suo posto una sola fila di 4 miniature cliccabili, nella forma
   `<a href="docs/cover.png"><img src="docs/cover.png" width="160" alt="…"></a>`:
   1. `docs/cover.png`;
   2-4. tre immagini scelte da `docs/special/stanza_NN.png` o
   `decomp/out/video/{titolo,gioco,bonus,finale}.png` (proposta: `cover.png`,
   `decomp/out/video/titolo.png`, `decomp/out/video/gioco.png`,
   `decomp/out/video/finale.png`; lo sviluppatore le guarda con Read e
   sostituisce una se è illeggibile). Tutti i `href` e `src` devono puntare
   a file versionati esistenti. Sotto la fila, una riga con link a
   `docs/special/` («hidden-object maps for all 52 rooms») e, opzionale,
   una gallery completa in `<details>` (sconsigliata se allunga oltre
   ~15 righe: basta il link).
3. Introduzione breve (riusare il paragrafo esistente «A from-scratch
   decompilation…», togliendo «from nothing but the original ROMs» se
   contraddice: il repo ora contiene i dati in `tecmo/`).
4. **How to play** (spostata in alto, prima del metodo):
   - Opzione 1: scaricare `solomons-key-v1.01-linux-x86_64.tar.gz` dalla
     pagina Releases, estrarre, eseguire `./gioca`; requisito runtime:
     SDL2 (`sudo apt install libsdl2-2.0-0` su Debian/Ubuntu), Linux x86_64.
   - Opzione 2: `git clone git@github.com:rufio72/solomonKey.git` (usare
     l'URL https `https://github.com/rufio72/solomonKey.git`), poi
     `make -C decomp/harness gioco` (servono `gcc`, `make`, `libsdl2-dev`),
     poi `decomp/harness/gioca`.
   - **Nessuna ROM serve, né per giocare né per compilare.**
   - Blocco comandi con le opzioni principali (riprendere dal blocco
     esistente r.180-193: `-s 4`, `--pieno`, `--scanlines [S]`, `--vite`,
     `--volume`, `--muto`, `--aiuto`), sezioni tasti, joystick, DIP e
     registrazioni dal testo esistente (r.195-232) ripulite da ROM/unzip.
     Nella riga dei tasti NON affermare quale pulsante salta/spara magia
     (vedi domanda 2). Annotare che `--aiuto` è in italiano.
   - «Known limit» aggiornato con i difetti noti dei fatti sopra.
5. Metodo («Method: decompilation with an oracle»): invariato salvo la
   frase «reconstructed … from nothing but the original ROMs».
6. Status: aggiornare la voce «Packaged — H7» (r.144-149: toglie «ROM
   extraction») e la voce «Next up» (r.150-153); aggiungere una voce
   «Self-contained (v1.01)» con: nessuna ROM a runtime né in compilazione,
   dati Tecmo in `tecmo/` (91 tabelle, driver audio, musica, grafica),
   banco a 1422 catture/335 voci/0 fallite, mappe degli special in
   `docs/special/`. Le altre voci storiche restano; non riscrivere i numeri
   che non si sono mossi.
7. **Ownership note** riscritta (sostituisce r.17-21): il repo contiene
   dati derivati dal gioco arcade originale in `tecmo/` e `roms/decoded/`
   (grafica, tabelle, musica, driver audio), copyright Tecmo/Koei Tecmo,
   inclusi per studio e conservazione; non sono di proprietà dell'autore;
   le ROM originali (`solomon.zip` ecc.) NON sono incluse e non vanno
   committate (restano ignorate da git); rimando a NOTICE.md.
8. Repository layout: aggiungere `tecmo/` e `docs/` (r.158-169), togliere
   ogni suggerimento che le ROM servano per giocare.
9. «Requirements» (r.236-239) rinominata «Requirements for development»:
   MAME 0.264, `z80dasm`, Python 3 con Pillow, gcc, make, e **solo per
   rigenerare gli oracoli e gli estrattori** il set ROM `solomon` in
   `roms/` (mai committato); `make -C decomp/harness rom` lo estrae.
   Contributing invariato.

### B. `NOTICE.md` (riscrittura, inglese, ≈25-40 righe)

Tre sezioni numerate come oggi:
1. **Data derived from the original arcade game**: i dati in `tecmo/`
   (tabelle del Z80 principale, `audio_driver.c` e `audio_dati_*.c` per
   driver e musica del Z80 audio, 12 `video_dati_*.c` con i byte delle ROM
   grafiche, `rom_dati.*`) e in `roms/decoded/` sono derivati dal gioco
   arcade *Solomon's Key* (Tecmo, 1986), copyright Koei Tecmo; inclusi per
   studio e conservazione; non sono di proprietà dell'autore del repo; gli
   estrattori sono `decomp/estrai_rom.py`, `decomp/estrai_dati.py`,
   `decomp/estrai_grafica.py` (controllare i nomi con `ls decomp/*.py`
   prima di citarli); le ROM originali (`roms/*.zip`, `roms/ext/`) non sono
   incluse.
2. **Analysis and decompilation** (`decomp/`, `compiti/`): script, rapporti
   e traduzioni in C scritti nel progetto; il codice originale dell'autore
   resta suo; le traduzioni in C sono opere derivate dal programma Tecmo e
   si offrono per uso personale e di studio; nessun diritto sul gioco
   originale è rivendicato.
3. **emu2149**: voce invariata nel contenuto (MIT, «Copyright (c) 2014
   Mitsutaka Okazaki», URL, commit sopra, percorso
   `decomp/harness/emu2149/`, incluso non modificato, usato da
   `decomp/harness/audio_wav.c` e dal pacchetto `gioca`) tradotta in inglese.
Aggiungere una frase: non affiliato a Tecmo/Koei Tecmo; segnalazioni di
rimozione benvenute (issue).

### C. Pacchetto e note in `decomp/out/release/`

1. Compilare da zero in una copia pulita (non nell'albero di lavoro):
   `D=$(mktemp -d) && git clone -q . $D/src && make -C $D/src/decomp/harness gioco`.
2. Cartella di staging `solomons-key-v1.01-linux-x86_64/` contenente:
   - `gioca` (copiato dalla build pulita, `strip` consentito);
   - `README.txt` in inglese (≈25-40 righe): cos'è il gioco, requisiti
     runtime (Linux x86_64, SDL2: `libsdl2-2.0-0`), avvio `./gioca`, tasti
     principali e opzioni principali (stesso contenuto della sezione How to
     play, senza dire quale pulsante salta), nota che nessuna ROM serve,
     copyright/NOTICE (testo breve + rimando a
     https://github.com/rufio72/solomonKey), difetti noti;
   - `NOTICE.md` (copia del file riscritto in B).
3. `tar -czf decomp/out/release/solomons-key-v1.01-linux-x86_64.tar.gz -C <staging-padre> solomons-key-v1.01-linux-x86_64`;
   scrivere anche `decomp/out/release/SHA256SUMS` (`sha256sum` del tar.gz).
4. `decomp/out/release/RELEASE-NOTES-v1.01.md` in inglese: titolo
   `v1.01`, tre blocchi «What's new since v1.0-rc1» (elenco dai fatti
   sopra), «Known issues», «How to play» (3-6 righe, rimanda a README),
   e una riga «Requires SDL2 (libsdl2-2.0-0), Linux x86_64». Segnaposto
   per la copertina: prima riga `![](docs/cover.png)` NON va messa (la
   chat decide l'asset); lasciare un commento HTML `<!-- cover -->` in
   testa.
5. Il pacchetto non va estratto né aggiunto a git.

### D. `STATO.md`

Cambiare la riga 3 «Aggiornato al…» in 2026-11-06 e aggiungere in
«Fasi» (dopo la voce del brief 60, r.39-44) una voce in italiano:
«Lotto 64 (brief 64, 2026-11-06): README e NOTICE riscritti in inglese,
pacchetto e note della v1.01 in `decomp/out/release/` (non versionato)».
Non toccare i numeri della tabella iniziale.

## Verifiche obbligatorie

1. `grep -n -iE 'solomon\.zip|unzip|main64|four ROM|roms/' README.md` →
   solo righe nella sezione «Requirements for development», nella
   «Ownership note»/Repository layout (non nel percorso del giocatore);
   nessuna riga dentro «How to play». Controllare a mano ogni riga trovata
   ed elencarla nel rapporto.
2. `grep -c 'docs/cover.png' README.md` ≥ 1 e `grep -n '!\[' README.md`
   → nessuna immagine markdown a tutta larghezza;
   `grep -c '<img src=' README.md` → 4 (o più se c'è una gallery, ma la
   prima fila ha 4 con `width="160"`).
3. Le miniature esistono: `grep -oE '(href|src)="[^"]+"' README.md | cut -d'"' -f2 | sort -u | while read f; do [ -f "$f" ] || echo MANCA $f; done`
   → uscita vuota (dalla radice del repo; ignorare URL http).
4. `head -1 README.md` → `# Solomon's Key (Tecmo, 1986)`;
   `grep -c 'A playable C decompilation, reverse-engineered from the original arcade ROMs' README.md` → 1.
5. NOTICE.md in inglese: `grep -c -iE 'Koei Tecmo' NOTICE.md` ≥ 1;
   `grep -c 'Mitsutaka Okazaki' NOTICE.md` ≥ 1;
   `grep -c '441f3dc295a9b82db16921461f0678b483db8304' NOTICE.md` → 1;
   `grep -n 'tecmo/' NOTICE.md` ≥ 1 riga; `grep -c -iE 'derivati|proprietà|ricavati' NOTICE.md` → 0;
   `grep -n 'decomp/harness/audio_dati\|decomp/harness/video_dati' NOTICE.md README.md` → vuoto.
6. Pacchetto: `D=$(mktemp -d); tar -xzf decomp/out/release/solomons-key-v1.01-linux-x86_64.tar.gz -C $D && cd $D/solomons-key-v1.01-linux-x86_64 && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./gioca -n 60 --muto`
   → esattamente `FRAME rese 60`, `HASH ultimo 815bb645bc46a325`,
   `AUDIO campioni 92610`, `AUDIO hash 9ce6e788ff2bc0d4` (misurata sul
   binario di lavoro; il binario della build pulita deve dare lo stesso);
   `ls` della cartella → `NOTICE.md  README.txt  gioca`.
7. `ls decomp/out/release/` → `RELEASE-NOTES-v1.01.md  SHA256SUMS  solomons-key-v1.01-linux-x86_64.tar.gz`
   (più eventualmente la cartella di staging);
   `sha256sum -c decomp/out/release/SHA256SUMS` (da dentro la cartella) → OK;
   `git status --short` non mostra nulla sotto `decomp/out/release/`.
8. Clone pulito verde: `D=$(mktemp -d) && git clone -q . $D/src && cd $D/src && git log -1 --format=%h && make -C decomp/harness gioco 2>&1 | tail -2 && cd decomp && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy harness/gioca -n 600 --muto`
   (il clone parte dal commit committato: eseguire DOPO il commit dei
   documenti, oppure copiare i documenti nuovi nel clone) →
   `Pronto: lancia decomp/harness/gioca …`, poi `FRAME rese 600`,
   `HASH ultimo ecd5acf694285489`, `AUDIO campioni 489510`,
   `AUDIO hash 8aedf41c0424df54`. Il clone non contiene `roms/ext/`
   (`ls $D/src/roms/ext` → non esiste).
9. Fuori ambito: `git show --stat HEAD | grep -vE '^ (README|NOTICE|STATO)\.md|compiti/64|^ *[0-9]+ files? changed|^$|commit|Author|Date|Co-Authored|^    '` → vuoto
   (il commit tocca solo `README.md`, `NOTICE.md`, `STATO.md`, eventualmente
   il brief);
   `find decomp tecmo -name '*.[ch]' | xargs wc -l | awk '$1>800 && $2!="total"'` → uscita pari a quella di prima del lotto (nessun file toccato; confrontare con l'uscita ottenuta all'inizio).
10. `grep -n '2026-11-06' STATO.md | head -3` → riga «Aggiornato al 2026-11-06»
    e la voce del lotto 64.

## Consegna

- Un commit con `README.md`, `NOTICE.md`, `STATO.md` (e il brief con
  l'esito in fondo): messaggio «README e NOTICE in inglese, pacchetto
  v1.01 preparato, brief 64», riga `Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>`
  (o il modello effettivo). Niente push.
- Fuori dal commit, in `decomp/out/release/`: il tar.gz, `SHA256SUMS`,
  `RELEASE-NOTES-v1.01.md`.
- Rapporto finale (massimo 30 righe): esito delle 10 verifiche, hash e
  dimensione del tar.gz, elenco delle righe di README trovate dalla
  verifica 1, le 4 miniature scelte, deviazioni.

## Domande aperte

1. **Nome del tag e del pacchetto** (preferenza): `v1.01` e
   `solomons-key-v1.01-linux-x86_64.tar.gz` come da richiesta; il tag
   resta `v1.01`, non `v1.0.1`. Proposta: accettare.
2. **Mappa dei pulsanti** (preferenza): README e pacchetto non dicono
   quale tra BUTTON1 (Z/Spazio/Ctrl) e BUTTON2 (X/Alt) salta o lancia la
   magia (non verificato in questo brief); proposta: elencarli come
   «BUTTON1»/«BUTTON2» finché una prova non lo stabilisce; la riga
   «Z jump, X magic» non va scritta.
3. **Bottiglia rossa di fuoco nel livello 2** (scelta di progetto): bug
   aperto a rc1 (raccolta senza effetto sui nemici); STATO.md non ne
   riporta l'esito. Proposta: elencarlo in «Known issues» di README e note
   come difetto noto; se la chat sa che è stato risolto, toglierlo prima
   di pubblicare. Va deciso da chi sa lo stato reale.
4. **Compilatore del pacchetto** (preferenza): binario dinamico su SDL2
   di sistema, `strip` consentito, build sull'host (gcc 13.3, Ubuntu
   24.04): compatibilità garantita solo con glibc ≥ quella dell'host.
   Proposta: dichiararlo nel README del pacchetto («built on Ubuntu 24.04»).
5. **Gallery completa** (preferenza): solo link a `docs/special/`, niente
   `<details>` con 52 immagini. Proposta: accettare.
6. **URL di clone** (preferenza): usare `https://github.com/rufio72/solomonKey.git`
   nel README pubblico (il remoto locale è SSH). Proposta: accettare.

## Decisioni (chat principale, 2026-11-06)

1. Tag `v1.01`, pacchetto `solomons-key-v1.01-linux-x86_64.tar.gz` —
   preferenza accettata (il tag era già deciso dall'utente).
2. Mappa pulsanti: restano «BUTTON1»/«BUTTON2» — preferenza accettata; la
   riconciliazione fine della mappa DAIJA resta nella coda minore.
3. Bottiglia rossa: NON va elencata come difetto noto — scelta già risolta
   dall'utente il 2026-11-05 (riprovata di persona, funziona, chiusa come
   falso allarme salvo ricomparsa).
4. Pacchetto dinamico su SDL2, costruito su Ubuntu 24.04, dichiarato nel
   README del pacchetto — preferenza accettata.
5. Gallery: 4 miniature cliccabili + link a docs/special/, niente details
   con 52 immagini — preferenza accettata.
6. URL di clone https nel README pubblico — preferenza accettata.

## Esito (sviluppatore Opus, 2026-11-06)

- README riscritto (inglese, 297 righe): titolo, sottotitolo e copyright
  come da brief; 4 miniature `width="160"` (`docs/cover.png`,
  `decomp/out/video/titolo.png`, `gioco.png`, `finale.png`) + link a
  `docs/special/`; How to play in alto senza ROM; voce «Self-contained
  (v1.01)»; Ownership note, layout con `tecmo/` e `docs/`, «Requirements for
  development». Nella voce storica H5 tolto «Z jump, X magic» (decisione 2).
- NOTICE riscritto in inglese (3 sezioni + non affiliazione e richieste di
  rimozione). «derivative works» reso «works derived from» perché la
  verifica 5 cerca `derivati` senza distinzione di maiuscole e lo trovava
  come prefisso.
- Pacchetto in `decomp/out/release/` (non versionato): tar.gz 229138 byte,
  sha256 `2d7da8cedf8fe4d8d1c59603d727876220d2e4b81c3dfcc48f8bed4d62d40e3a`,
  `gioca` da clone pulito con `strip`; note `RELEASE-NOTES-v1.01.md`.
- Verifiche 1-10 passate (la 8 sul clone con i documenti copiati prima del
  commit, la 9 dopo il commit).
