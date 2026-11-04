# Compito 57: ramo 386A e effetto scanlines

Data: 2026-11-04. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Due parti indipendenti nello stesso lotto, due commit separati:
- **A**: tradurre l'unica istruzione `inc c` a 386A in `fase_07_3861`, così i
  semi 10/4/67 del gioco libero non si fermano più su «routine 386A mancante».
- **B**: opzione `--scanlines` di `gioca` (opt-in): righe orizzontali scure da
  CRT arcade, puro effetto di presentazione, senza alcun effetto su hash e
  modo oracolo.

## Vincoli assoluti

- Nessuna modifica fuori da `/home/rufio72/OpenSolomonsKey-master`.
- Commenti, messaggi, `--aiuto`, STATO e brief in italiano; solo la sezione
  «How to play» del `README.md` in inglese.
- Nessun file di codice sopra le 800 righe (oggi i file toccati sono tutti
  sotto: video_sdl.c 176, gioca.c 280, gioca_opzioni.c 61, attivita_attract07a.c 259).
- MAME non va eseguito (e se mai servisse: `-video none`). Non toccare
  oracoli, ricette, `out/`. Non fare push. Un agente per volta sui file.
- Il default di `gioca` resta pixel-perfetto: senza `--scanlines` il
  comportamento è identico a oggi.
- Strumenti: `make`, gcc, SDL2 già installati; `z80dasm` non serve
  (`decomp/out/main64.dasm` esiste).

## Cosa esiste già (fatti verificati)

### Parte A

- `decomp/harness/attivita_attract07a.c:21-37` (da `grafo`/lettura): `fase_07_3861`
  legge `uint8_t a = M(0xc243)`; se `a >= 0x24` fa `nemici_manca(0x386a); return;`
  (r. 28-31). Poi il giro sulle sette coppie di `3A0E` (r. 32-36, `nemici_manca(0x3874)`
  se `a` coincide), poi `NEMICI_PUNTO(0x3861, 0x3879); M(0xc09a) = 0x1a; /* 3879 */` (r. 37-38).
- Disassemblato (`decomp/out/main64.dasm` r. 10766-10781, da lettura):
  ```
  3861 ld a,(c243)   3864 ld c,1A   3866 cp 24   3868 jr c,386B
  386A inc c         386B ld b,07   386D ld hl,3A0E
  3870 cp (hl)  3871 inc hl  3872 jr nz,3875  3874 ld c,(hl)  3875 inc hl  3876 djnz 3870
  3878 ld a,c        3879 ld (c09a),a
  ```
  Quindi `c = 0x1a`, e `0x1b` se `a >= 0x24`; il ciclo dopo non cambia `a`
  (il confronto con la tabella resta quello già tradotto, vale anche per
  `a >= 0x24`); `c09a` riceve `c`. Più avanti `c` è riassegnato (`ld c,3F` a 3888),
  quindi nessun altro stato dipende da `c`.
- Fermate da rimuovere (dall'esito del brief 56, non rimisurate): «FERMO routine
  386A mancante» seme 10 frame 27938 (`FRAME rese 27872`, `HASH ultimo
  16ea230eea591d30`, `AUDIO hash 46b2da66631444cb`), seme 4 frame 25954
  (`25888`, stesso hash video, `e7d412a36e88f8cc`), seme 67 frame 25218 (`25152`,
  stesso hash video, `f447b05e997e0553`).
- Comando di misura (da `decomp/`, dal brief 56): `python3 ricette/genera_ingressi.py S /tmp/sS.txt 40000 8`
  poi `harness/gioca -n 40000 --muto --riproduci /tmp/sS.txt 2>&1 | grep -a 'FERMO\|FRAME rese\|HASH ultimo\|AUDIO hash'`
  (circa 1 minuto per seme).
- Il sito 2D1F (`attivita_attract07.c:65`, `fase_2d07`, corpo 07 dell'attract, solo
  con bit 4 di c045 acceso) è ancora `nemici_manca(0x2d1f)`: fuori perimetro.

### Parte B

Da lettura di `decomp/harness/video_sdl.c` (176 righe) e `video_sdl.h` (29):
- Statiche: `finestra`, `rend` (`SDL_Renderer`), `tex` (texture `SDL_PIXELFORMAT_RGB24`,
  `STREAMING`, `VID_LARGO`x`VID_ALTO` = 256x224), `con_vsync`, `a_schermo_pieno`.
- `vsdl_apri(titolo, scala)`: finestra `VID_LARGO*scala` x `VID_ALTO*scala`; hint
  `SDL_HINT_RENDER_SCALE_QUALITY "0"` (nearest); renderer accelerato+vsync, ripiego
  software. Con `a_schermo_pieno`: `FULLSCREEN_DESKTOP`, `SDL_RenderSetLogicalSize(rend, 256, 224)`
  e `SDL_RenderSetIntegerScale` (scala intera, bordi neri).
- `vsdl_presenta(fb)`: `SDL_UpdateTexture(tex, fb)` → `SDL_RenderClear` →
  `SDL_RenderCopy(rend, tex, NULL, NULL)` → temporizzazione → `SDL_RenderPresent` →
  eventi. Il framebuffer `fb` non viene mai scritto da SDL.
- `vsdl_hash(fb)` (FNV-1a sul `fb`, senza SDL) è chiamato in `gioca.c` `passo()`
  (r. 124) subito dopo `rendi_frame(&vr, reso)` e PRIMA di `vsdl_presenta(reso)` (r. 132);
  la pausa (r. ~100) ripresenta `reso`. Con `-n N` (headless) SDL non è nemmeno inizializzata.
- `video_sdl.c` è usato anche da `mostra_oracolo` e `mostra_replay` (Makefile r. 71):
  non devono cambiare comportamento né rompersi al link.
- Opzioni: `gioca_opzione_extra` (`gioca_opzioni.c:18`) gestisce `--aiuto/-h`, `--pieno`
  (`gx.pieno = 1; return 1;`), `--joystick`, `--senza-joystick`, `--volume`, `--joy-mappa`, `-t`.
  Struct `gioca_extra gx = { 0, 100, 0, 0, 0, 0 }` in `gioca_opzioni.h`
  (campi: `pieno, volume, joystick, senza_joy, tempo, tempo_dato`).
  `gioca.c:258-259`: `vsdl_pieno(gx.pieno); vsdl_apri("Solomon's Key - gioca", opz.scala)`.
- Testo aiuto: `gioca_aiuto.c` (47 righe), sezione «Schermo» con `-s`, `--pieno`, `-n`, `-m`.
- README: sezione «## How to play» r. 169-220; blocco di comandi r. 178-187
  (`--pieno` r. 183); il paragrafo «Known limit» r. 219-220.
- Misura: `partita_1a` con `-m 4000` e `SDL_VIDEODRIVER=dummy` (finestra, non headless)
  dà già oggi `FRAME rese 4000`, `HASH ultimo 3f25ba053eeb56b9`, `AUDIO campioni 2988510`,
  `AUDIO hash c097308a8678392e` (67 s). Questo è il banco di prova dell'invarianza dell'hash.

## Struttura da realizzare

### Parte A

In `fase_07_3861` sostituire il ramo `a >= 0x24` (r. 28-31):
- variabile `uint8_t c = (a >= 0x24) ? 0x1b : 0x1a;` (386A inc c, tutte e due le vie);
- il ciclo 386D-3876 resta com'è (la coppia che coincide già ferma con 0x3874);
- r. 38: `M(0xc09a) = c;  /* 3879 (ld a,c) */`;
- aggiornare il commento di testa della funzione (r. 14-16: «c09a = 1A» → «1A, 1B con c243 >= 24»);
- punti di sincronia `NEMICI_PUNTO` invariati (nessuna scrittura RAM nuova prima di c09a).
Se i semi si fermano su qualcos'altro (altro `routine XXXX mancante`, 2D1F, altro):
NON tradurre, censire messaggio/seme/frame/hash nell'esito e in STATO.

### Parte B (scelte; vedi domande aperte)

1. `gioca_opzioni.h/.c`: aggiungere `int scanlines;` in coda a `gioca_extra`
   (inizializzatore `{ 0, 100, 0, 0, 0, 0, 0 }`) e `if (!strcmp(a, "--scanlines")) { gx.scanlines = 1; return 1; }`.
2. `video_sdl.h/.c`: `void vsdl_scanlines(int on);` da chiamare prima di `vsdl_apri`
   (come `vsdl_pieno`; predefinito spento). In `vsdl_apri`, se acceso, creare una texture
   overlay `SDL_PIXELFORMAT_RGBA32`, `SDL_TEXTUREACCESS_STATIC`, `SDL_BLENDMODE_BLEND`,
   larghezza 1 e altezza `VID_ALTO*2` (448): riga pari = (0,0,0,0), riga dispari = (0,0,0,A)
   con `A = 80` (circa 31%). In `vsdl_presenta`, dopo `SDL_RenderCopy(rend, tex, ...)` e prima
   della temporizzazione/`RenderPresent`, `SDL_RenderCopy(rend, overlay, NULL, NULL)`.
   Ogni riga sorgente diventa così due metà: la superiore intatta, l'inferiore scurita,
   a qualsiasi scala intera e anche con `--pieno` (la dimensione logica 256x224 fa scalare
   anche l'overlay, bordi neri esclusi dal rendering). `vsdl_chiudi` distrugge l'overlay.
   Il framebuffer `fb` e `vsdl_hash` non si toccano. `SDL_BLENDMODE_BLEND` con alpha 80 e nero
   dà `colore * (1 - 80/255)`. Se l'overlay non si crea: messaggio di avviso su stderr e
   si continua senza effetto (non errore fatale).
3. `gioca.c`: prima di `vsdl_apri` (r. 258) aggiungere `vsdl_scanlines(gx.scanlines);`.
   Con `-n` (headless) l'opzione è accettata e ignorata. Nel modo `--oracolo` l'opzione
   è accettata ma non cambia nulla del calcolo (solo la presentazione).
4. `gioca_aiuto.c`, sezione «Schermo»: riga
   `"  --scanlines          righe scure alla CRT (solo presentazione, hash invariati)\n"`.
5. `README.md` «How to play»: nel blocco comandi dopo `--pieno`:
   `decomp/harness/gioca --scanlines  # CRT-style dark horizontal lines (display only, default off)`;
   una frase in inglese sotto: works with `-s N` and `--pieno`, looks best at scale 2 or more,
   never changes the video hash, replays or the oracle mode.
6. Aggiornare `decomp/STATO.md` con una voce per il lotto 57 (cerca l'ultima voce
   `[x] Lotto 56`) e, se citato, aggiornare la riga che dice che il gioco libero si ferma a 386A.
   Aggiungere i campi nuovi al commento di `gioca_opzioni.h` se serve.

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master` salvo dove scritto. Uscita esatta salvo «attesa, non misurata».

1. Compilazione: `cd decomp/harness && make harness replay gioco_libero gioca sched_proto mostra_oracolo mostra_replay > /tmp/m57.txt 2>&1;
   echo "make $?"; grep -c -E 'warning|error' /tmp/m57.txt` → `make 0` e `0` (attesa, non misurata per i due `mostra_*`).
2. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h57.txt; echo "exit $?"; grep -c FALLITO /tmp/h57.txt;
   grep -c '^RIEPILOGO' /tmp/h57.txt; grep -c '^cattura' /tmp/h57.txt; grep -c '^OK' /tmp/h57.txt` →
   `exit 0`, `0`, `335`, `1412`, `1412`.
3. Giro: da `decomp/`, `sh giro_completo.sh | tail -1` → `GIRO COMPLETO 0-5499: saldatura 0 byte,
   spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`.
4. Oracolo: da `decomp/`, `harness/gioca -n 5434 --muto --oracolo out/giroboot_oracolo.txt out/giroboot_disp.txt
   --riproduci out/ingressi_replay.txt 2>/dev/null | tail -2` → `AUDIO campioni 4042500` / `AUDIO hash c4b40fea06623a96`.
5. Partita 1A headless: da `decomp/harness`, `./gioca --riproduci ../ricette/partita_1a.reg -n 4000 --muto 2>/dev/null | tail -4`
   → `FRAME rese 4000`, `HASH ultimo 3f25ba053eeb56b9`, `AUDIO campioni 2988510`, `AUDIO hash c097308a8678392e`.
6. Semi (parte A): per `S in 10 4 67`, ingressi `python3 ricette/genera_ingressi.py $S /tmp/s$S.txt 40000 8` (da `decomp/`), poi
   `harness/gioca -n 40000 --muto --riproduci /tmp/s$S.txt 2>&1 | grep -a 'FERMO\|FRAME rese\|HASH ultimo\|AUDIO hash'`.
   Atteso `FRAME rese 40000` senza `FERMO` (attesa, non misurata). Se compare un `FERMO` nuovo, riportare messaggio,
   seme, frame, `FRAME rese`, `HASH ultimo`, `AUDIO hash` senza tradurre (2D1F compreso) e continuare il lotto: la
   verifica allora è «nessun `386A`». Un seme di sicurezza basta ai semi 4 e 67: eseguirli tutti e tre.
7. Determinismo: ripetere il seme 10 e confrontare con `diff` le righe filtrate della verifica 6 → diff vuoto.
8. Scanlines, hash invariato in finestra: da `decomp/harness`, due volte (senza e con l'opzione)
   `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./gioca [--scanlines] --riproduci ../ricette/partita_1a.reg -m 4000 --muto 2>/dev/null | tail -4`
   → in entrambe `FRAME rese 4000`, `HASH ultimo 3f25ba053eeb56b9`, `AUDIO hash c097308a8678392e` (senza opzione: misurato).
   Ripetere con `--scanlines --pieno -s 2` (con driver dummy) → stessi tre valori (attesa, non misurata; se `--pieno` non
   regge col driver dummy riportarlo e provare solo `-s 2`).
9. Scanlines, opzione riconosciuta: `./gioca --scanlines --stampa-dsw` → esce 0 stampando `DSW1 .. DSW2 ..` (l'opzione non
   è «sconosciuta»); `./gioca --aiuto | grep -c scanlines` → `1`; `grep -c scanlines ../../README.md` → almeno `1`.
10. Scanlines, effetto presente: provare con un programma di prova usa-e-getta nello scratchpad (non nel repository), o con
    una `SDL_RenderReadPixels` temporanea rimossa prima del commit, con driver dummy/software a scala 2 e 3: la riga
    di schermo `2k` ha pixel uguali al framebuffer, la riga `2k+1` ha ciascun canale ≈ `v * 175/255` (±2); senza
    `--scanlines` tutte uguali al framebuffer. Riportare i valori letti. (Attesa, non misurata.) Se il driver dummy non
    permette la lettura, riportarlo e basarsi sulla verifica 8 + revisione del codice.
11. Dimensioni: `find decomp/harness -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 && $2!="totale"'` → vuota,
    oppure i soli file già oltre 800 righe prima del lotto (confrontare con `git stash`-free: `git show HEAD:<file> | wc -l`).
12. Ambito: `git status --short` elenca solo `attivita_attract07a.c`, `video_sdl.c/.h`, `gioca.c`, `gioca_opzioni.c/.h`,
    `gioca_aiuto.c`, `README.md`, `decomp/STATO.md`, questo brief; nessun binario né `.o` aggiunto (verificare
    `git status --short | grep -v '^ M'` → solo eventuali file ignorati); `pgrep -c mame` → `0`.

## Consegna

Due commit, ciascuno dopo le sue verifiche:
- «Lotto 57A: ramo 386A, c09a = 1B con c243 >= 24»;
- «Lotto 57B: opzione --scanlines (presentazione, hash invariati)».
Riga finale di entrambi: `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
(o il modello effettivo). In fondo a questo file aggiungere la sezione «Esito» (uscite delle
verifiche, eventuali fermate nuove dei semi con messaggio/seme/frame, deviazioni). Rapporto
finale: massimo trenta righe con uscita delle verifiche e deviazioni. Niente push.

## Domande aperte

1. **Intensità e geometria dell'effetto**: proposta overlay 1x448 con riga dispari alpha 80 (circa 31% di oscuramento,
   una metà-riga ogni riga sorgente); alternativa riga intera ogni y dispari della sorgente (più forte, scintillio a scala 1).
   A scala 1 la metà-riga non è risolvibile (aspetto indefinito, in pratica una riga su due, 50% di luminosità media):
   documentato come «meglio da scala 2». Rischio basso, tutto reversibile. Marca: preferenza.
2. **Dove applicare l'effetto**: proposta overlay texture sul renderer (funziona con ridimensionamento della finestra, `-s N`
   e `--pieno` senza ricalcoli, nessun accesso ai pixel). Alternativa: scurire le righe in un buffer scalato lato CPU
   (più lento, legato alla scala). Se l'utente vuole il «bloom»/maschera RGB o la curvatura CRT, è un'estensione da
   un lotto a parte. Marca: preferenza.
3. **Fermate nuove dei semi dopo 386A** (per esempio 2D1F o altri rami dell'attract): proposta di non tradurle in questo
   lotto, censirle e lasciare al lotto successivo; la verifica 6 passa comunque se il solo difetto è la fermata nuova
   e non c'è più «386A». Se invece il gioco libero con `FRAME rese 40000` fosse requisito del test utente, serve decidere
   se estendere il perimetro. Marca: scelta di progetto (perimetro).
4. **`--scanlines` con modo `--oracolo` e `--riproduci`**: proposta accettare l'opzione e ignorarla per il calcolo (solo
   presentazione); alternativa vietarla con errore. Marca: preferenza.

## Risoluzioni delle domande aperte (chat principale sotto mandato utente, 2026-11-04)

1. Intensità e geometria: preferenza ACCETTATA con la proposta (overlay
   1x448, alpha 80 ≈ 31% sulle righe dispari). A scala 1 l'effetto può
   risultare poco visibile: va bene così, è un limite fisico, annotarlo
   nell'aiuto non serve.
2. Overlay texture sul renderer: preferenza ACCETTATA (niente buffer CPU).
3. Fermate nuove dei semi dopo 386A (p.es. 2D1F): CENSIRE E SEGNALARE
   SENZA TRADURRE. Il perimetro del gioco libero si chiude con questo
   lotto; eventuali code si decidono dopo il test dell'utente.
4. --scanlines con --oracolo o --riproduci: ACCETTATA E IGNORATA per il
   calcolo (è solo presentazione), preferenza accettata.

## Esito (sviluppatore Opus 5.5, 2026-11-04)

Due commit: «Lotto 57A» (386A) e «Lotto 57B» (--scanlines). Verifiche:
1. `make 0`, `0` warning/errori (anche i due `mostra_*`), dopo A e dopo B.
2. Banco: `exit 0`, `0`, `335`, `1412`, `1412`.
3. Giro: riga attesa esatta.
4. Oracolo: `AUDIO campioni 4042500` / `AUDIO hash c4b40fea06623a96`.
5. Partita 1A headless: 4000 / 3f25ba053eeb56b9 / 2988510 / c097308a8678392e.
6. Semi: nessun `386A`; FERMATA NUOVA, censita e non tradotta, nella stessa
   fase e nello stesso frame: «FERMO routine 3901 mancante» (il giro
   38FB-3901 sulla tabella 82D3, `jr l38fbh`, quando c243 supera la voce
   confrontata; `attivita_attract07a.c`, ramo `q > leggi(0x82d3)`).
   Seme 10: frame 27938, FRAME rese 27872, HASH 16ea230eea591d30, AUDIO 46b2da66631444cb.
   Seme 4: frame 25954, FRAME rese 25888, stesso HASH, AUDIO e7d412a36e88f8cc.
   Seme 67: frame 25218, FRAME rese 25152, stesso HASH, AUDIO f447b05e997e0553.
   I numeri coincidono con quelli del lotto 56: la fermata si è solo spostata
   più avanti nella stessa chiamata della fase 0 (nessuna scrittura RAM nuova resa).
7. Determinismo seme 10: diff vuoto.
8. Finestra dummy: senza opzione, `--scanlines`, `--scanlines --pieno -s 2`:
   tutte e tre 4000 / 3f25ba053eeb56b9 / 2988510 / c097308a8678392e, nessun avviso.
9. `--scanlines --stampa-dsw` → `DSW1 02 DSW2 00`, uscita 0; aiuto `1`; README `2`.
10. Programma di prova nello scratchpad (include `video_sdl.c`, renderer software,
    `SDL_RenderReadPixels`), pixel (200,100,50): scala 2 righe 20/21 = 200,100,50 /
    137,68,34; scala 3 righe 30,31 intatte, 32 = 137,68,34 (overlay 448 su 672 righe:
    una sotto-riga su tre scura); `--pieno` uguale a scala 3; senza opzione tutte
    intatte; 0 canali fuori tolleranza ±2. Scala 1: tutte le righe scurite
    (limite accettato nella risoluzione 1).
11. Nessun file sopra 800 righe (video_sdl.c 209, attivita_attract07a.c 256).
12. Ambito: solo i file previsti; `pgrep -c mame` → `0`.

Deviazioni: STATO.md è nella radice del repository, non in `decomp/STATO.md`
come scritto nel brief: aggiornato quello. A scala 3 l'effetto scurisce una
sotto-riga su tre, non «la metà inferiore» (geometria 448 righe, atteso).

## Esito del ciclo (chat principale, 2026-11-04)

Verificatore (Sonnet, ~22k token): verifiche 1-9, 11, 12 TUTTE PASSATE;
la 10 (pixel) non rieseguibile (programma di prova nello scratchpad
sparito) ma coperta dalla 8 (hash invariati con --scanlines). Fermata
nuova attesa: «routine 3901 mancante» (ciclo 38FB-3901 sulla tabella
82D3, stessa fase dell'attract post-game-over), censita e NON tradotta
per decisione — si valuta dopo il test dell'utente. Analista Sonnet ~49k,
sviluppatore ~49k. Lotto 57 CHIUSO con d8b8c33 (386A) e c7c6838
(scanlines). LA CODA DEL MANDATO È COMPLETA: si passa al test utente.

## Addendum 57C: scanlines rifatte (feedback utente dal test, 2026-11-04)

Giudizio dell'utente sulla 57B: le linee sono TROPPE/troppo presenti e
sono agganciate alla griglia sbagliata. Specifica nuova:

1. Le linee vivono TRA le file di pixel DEL GIOCO (griglia nativa
   256x224), non sulle righe fisiche dello schermo. A scala N ogni fila
   di pixel del gioco è alta N pixel fisici: la linea scura va al
   CONFINE tra una fila e la successiva.
2. Lo spessore è PROPORZIONALE alla scala: max(1, round(N/4)) pixel
   fisici (a scala 4 → 1 px scuro ogni 4, a scala 8 → 2 ogni 8). Se il
   gioco si rimpicciolisce, le linee si stringono con lui.
3. Intensità più leggera della 57B: alpha ~64 (≈25%) invece di 80.
4. A scala 1 non c'è spazio «tra i pixel»: effetto DISATTIVATO (con
   --scanlines attivo ma scala 1, nessun overlay), senza avvisi.
5. Con --pieno la scala intera effettiva decide spessore e posizione
   come sopra.
6. Invarianti come 57B: hash video e audio INVARIATI con e senza
   --scanlines (verifica 8 del brief, stessa uscita); nessun effetto su
   oracolo/riproduzione; banco e giro completo intatti.

Verifica dei pixel (script nello scratchpad, salvarne il sorgente in
decomp/harness/prova_scanlines.c o simile COMMITTATO stavolta, così la
verifica resta rieseguibile): a scala 4, per ogni blocco di 4 righe
fisiche dentro l'area di gioco, 3 righe intatte e 1 scurita di ~25%;
a scala 2, 1 ogni 2 scurita; a scala 1 tutte intatte; senza opzione
tutte intatte.

## Esito 57C (sviluppatore Opus 5.5, 2026-11-04)

Prima consegna (82a34ba): overlay sulla griglia del gioco, spessore
max(1, round(N/4)), alpha 64. Su richiesta dell'utente dopo il test visivo,
secondo commit: spessore parametrico. `--scanlines [S]`, S in quarti di fila
di pixel del gioco da 1 a 3, predefinito 2 (il doppio della prima versione);
spessore fisico max(1, round(N*S/4)) limitato a N-1 (deviazione: senza il
limite a scala 2 con S=3 la fila sarebbe tutta scura, nessuna linea; lì S=3
vale come S=2). Sintassi: argomento facoltativo nella stessa opzione; gioca
non ha argomenti posizionali, quindi la parola che segue `--scanlines` e non
comincia con `-` è sempre lo spessore: nessuna ambiguità, niente opzione
separata. 0, 4, testo → «ERRORE: --scanlines vuole uno spessore da 1 a 3
(quarti di fila di pixel del gioco)», uscita 2.

Overlay: texture 1 x 224*N per la scala intera effettiva (in finestra
altezza d'uscita/224, con `--pieno` la scala di `SDL_RenderSetIntegerScale`),
rifatta se la scala cambia; a scala 1 nessun overlay. Prova dei pixel
committata, attese calcolate in modo indipendente da `video_sdl.c`:
`make prova_scanlines && SDL_VIDEODRIVER=dummy ./prova_scanlines 1 2 3 4 5 6 8`
→ righe scure (S=1/2/3): scala 2 224/224/224 su 448; scala 3 224/448/448 su
672; scala 4 224/448/672 su 896; scala 5 224/672/896; scala 6 448/672/1120;
scala 8 448/896/1344 su 1792; `--pieno` (dummy 1024x768, scala 3) come scala
3; scala 1 e senza opzione 0; pixel 200,100,50 → 149,74,37; 0 fuori
tolleranza ±2; «PROVA SCANLINES PASSATA», uscita 0.
Verifica 9: `--scanlines [3] --stampa-dsw` → `DSW1 02 DSW2 00` uscita 0;
aiuto 1; README 3. `make`: 0 warning.
Banco e giro non rieseguiti: non compilano `video_sdl.c`.
Hash partita 1A (verifica 8) senza opzione, `--scanlines`, `--scanlines 1`,
`--scanlines 2 -s 4`, `--scanlines 3 -s 4`, `--scanlines 3 --pieno -s 2`:
tutte 4000 / 3f25ba053eeb56b9 / 2988510 / c097308a8678392e.
