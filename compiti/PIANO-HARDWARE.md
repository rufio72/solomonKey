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

- [x] **H3 — Finestra SDL.** Modulo `video_sdl.c`: finestra, texture
  streaming del framebuffer di H2, 60 Hz con vsync, scala intera. Primo
  risultato visibile: un visualizzatore che mostra gli stati campione e,
  come dimostrazione, i frame del giro replay. Verifica: confronto a
  script del framebuffer (SDL non cambia i pixel) + sanità manuale.
  *Analista Sonnet.*

- [x] **H4 — Giro libero.** *H4c fatto (brief 48, 2026-11-03): H4 chiuso, restano H5 e oltre. H4a fatto (brief 44), H4b1 fatto (brief 45,
  2026-11-03), H4b2 fatto (brief 46, 2026-11-03), H4b3 fatto (brief 47,
  2026-11-03).* Spezzato in tre dal brief 44 (Fable,
  2026-11-03): **H4a** macchina+boot+ingressi registrati con arresto
  atteso al corpo 1F; **H4b** traduzione dei corpi attract/titolo
  (~930 istruzioni), spezzato dal brief 45 in **H4b1** (fatto: NMI complete,
  corpi 1F e 06, strumento e sentinella per R; arresto al corpo 07, frame
  1188), **H4b2** (fatto, brief 46: corpo 07 fasi 0-12 e routine 3ADF, arresto alla
  fase 13 al frame 1398) e **H4b3** (fatto, brief 47: fasi 13-25 del corpo 07,
  corpi 08, 0C, 0B e confronto byte per byte al frame 2410, che coincide); la regione 6253-673F passa
  a H4c; **H4c** giro completo 0-5499 contro
  `replay_2410.bin` e l'oracolo storico. Staccare il gioco dall'oracolo: ciclo
  principale vero (NMI a 60 Hz, scheduler, RAM inizializzata come al
  boot), il gioco gira da solo senza replay, prima headless. Verifica:
  N frame di giro libero con input nulli coincidono con MAME avviato
  nelle stesse condizioni (stesso seme: attenzione al registro R e al
  contatore di frame); il banco e il replay storici restano intatti.
  È il secondo lotto delicato (confini tra harness e gioco vero).
  *Analista Opus.*

- [x] **H5 — Input SDL.** *Fatto (brief 49, 2026-11-03): `decomp/harness/gioca`, modo libero (NMI ai confini dello spedidore, 5434 frame senza FERMO) e `--oracolo` (hash 5499 a239d0a940bec0b9 esatto); vedi GIRO.md.* Tastiera (e joystick se c'è) → porte di
  ingresso già modellate in `porte.c`/`e603.h`, più i dip switch come
  opzioni da riga di comando (Bonus Life compreso, vedi brief 40).
  Da qui il gioco si gioca. Verifica: sequenza di input registrata che
  riproduce un percorso noto. *Analista Sonnet.*

- [x] **H6 — Raccordo audio.** *Fatto (brief 50, 2026-11-03): `gioca` suona; latch identici a MAME, cambi AY con residuo misurato (604/312 frame), eventuale H6b per la durata della passata 029E; vedi GIRO.md.* Il gioco scrive i codici al driver audio
  tradotto; emu2149 suona nel callback SDL audio, sincronizzato al frame.
  Verifica: il flusso dei cambi di registro durante una sequenza nota
  coincide con l'oracolo `_ay.txt` corrispondente. *Analista Sonnet.*

- [x] **H7 — Pulizia e pacchetto.** *Fatto (brief 51, 2026-11-04):
  `make -C decomp/harness gioco`, `gioca` da qualunque cartella, joystick con
  diagnostica `--joystick` e hotplug, `--joy-mappa`, `--volume`, `--pieno`,
  `--aiuto`, sezione «How to play» del README; banco, replay, giro e audio
  identici a prima; vedi GIRO.md. Resta la prova manuale sullo stick DAIJA.*
  Bersaglio `make gioco`, temporizzazione
  stabile, opzioni (scala, fullscreen, volume), replay come regressione nel
  Makefile, README aggiornato con le istruzioni (le ROM restano fuori dal
  repo; a runtime ne servono solo 4 file: `6.3f`, `7.3h`, `8.3jk`, uniti in
  `out/main64.bin` da `make gioco`, e `1.3jk`; grafica e dati audio sono C
  generato). *Analista Sonnet.*

Manutenzione da infilare dove capita: correggere `rst20_tables.h`
(la tabella a 646B ha 10 parole vere, non 3 — vedi brief 40). *Fatto nel
brief 51, anche nel generatore `rst20.py`.*

Dopo H7 restano, fuori da questo piano: la classificazione delle 885
divergenze a metà attività del replay e ogni rifinitura che emerge
giocando.
