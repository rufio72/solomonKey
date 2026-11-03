# Piano dello strato hardware (SDL)

Deciso con l'utente il 2026-11-03: libreria **SDL2** (stabile e pacchettizzata
ovunque; si passa a SDL3 solo se emerge un motivo). Obiettivo: rendere
giocabile il C tradotto, con il replay 2410-5499 come regressione permanente.
Ogni voce diventa un brief `compiti/NN-*.md` col solito ciclo
analista → sviluppatore → verifica a script. Il metodo resta quello di tutto
il progetto: **prima l'oracolo misurato da MAME, poi il codice che lo
eguaglia**, mai il contrario.

Materiale già pronto:
- `roms/decoded/`: tile e sprite decodificati in PNG (`decode.py`), palette
  vere per stanza (`colora.py`), screenshot MAME come oracoli.
- `decomp/harness/porte.c` + `e603.h`: porte di ingresso già modellate.
- Catena audio completa: driver tradotto, dati estratti, emu2149
  vendorizzato, resa wav verificata byte per byte.
- `snap.lua` / `cattura.sh`: infrastruttura per estrarre RAM e stati da MAME.

## Lotti, in ordine

- [x] **H1 — Oracolo video.** FATTO (brief 41, 2026-11-03, ~0,3 $). Script Lua/sh che da MAME estrae, per 4-6
  stati campione (titolo, stanza 1, una stanza avanzata, bonus, finale):
  dump di videoram, colorram, spriteram e palette RAM, più lo screenshot
  PNG dello stesso frame. Definire il formato dei file e dove vivono
  (`decomp/out/video/`). Nessun codice C. Verifica: i dump si rigenerano
  identici. *Analista Sonnet.*

- [x] **H2 — Renderizzatore software puro (senza SDL).** FATTO (brief 42,
  2026-11-03, ~0,4 $; 7/7 stati identici pixel per pixel; i byte delle ROM
  grafiche sono C generato nel repo, quindi in H7 non si caricano da
  `roms/`). Funzione C
  `rendi_frame(ram) → framebuffer RGB` che riproduce il video del gioco:
  fondale (tilemap), sprite con priorità e trasparenze, palette. Tabelle
  grafiche dalle ROM come sorgente C generato (stile `estrai_dati.py`
  dell'audio). Verifica: framebuffer **pixel per pixel identico** agli
  screenshot MAME degli stati H1. È il lotto più delicato: confini nuovi,
  formato hardware degli sprite da ricavare. *Analista Opus; spezzare in
  H2a fondale + palette e H2b sprite se supera le soglie.*

- [ ] **H3 — Finestra SDL.** Modulo `video_sdl.c`: finestra, texture
  streaming del framebuffer di H2, 60 Hz con vsync, scala intera. Primo
  risultato visibile: un visualizzatore che mostra gli stati campione e,
  come dimostrazione, i frame del giro replay. Verifica: confronto a
  script del framebuffer (SDL non cambia i pixel) + sanità manuale.
  *Analista Sonnet.*

- [ ] **H4 — Giro libero.** Staccare il gioco dall'oracolo: ciclo
  principale vero (NMI a 60 Hz, scheduler, RAM inizializzata come al
  boot), il gioco gira da solo senza replay, prima headless. Verifica:
  N frame di giro libero con input nulli coincidono con MAME avviato
  nelle stesse condizioni (stesso seme: attenzione al registro R e al
  contatore di frame); il banco e il replay storici restano intatti.
  È il secondo lotto delicato (confini tra harness e gioco vero).
  *Analista Opus.*

- [ ] **H5 — Input SDL.** Tastiera (e joystick se c'è) → porte di
  ingresso già modellate in `porte.c`/`e603.h`, più i dip switch come
  opzioni da riga di comando (Bonus Life compreso, vedi brief 40).
  Da qui il gioco si gioca. Verifica: sequenza di input registrata che
  riproduce un percorso noto. *Analista Sonnet.*

- [ ] **H6 — Raccordo audio.** Il gioco scrive i codici al driver audio
  tradotto; emu2149 suona nel callback SDL audio, sincronizzato al frame.
  Verifica: il flusso dei cambi di registro durante una sequenza nota
  coincide con l'oracolo `_ay.txt` corrispondente. *Analista Sonnet.*

- [ ] **H7 — Pulizia e pacchetto.** Bersaglio `make gioco`, temporizzazione
  stabile, opzioni (scala, fullscreen, volume), replay come regressione nel
  Makefile, README aggiornato con le istruzioni (le ROM restano fuori dal
  repo: il gioco le carica da `roms/`). *Analista Sonnet.*

Manutenzione da infilare dove capita: correggere `rst20_tables.h`
(la tabella a 646B ha 10 parole vere, non 3 — vedi brief 40).

Dopo H7 restano, fuori da questo piano: la classificazione delle 885
divergenze a metà attività del replay e ogni rifinitura che emerge
giocando.
