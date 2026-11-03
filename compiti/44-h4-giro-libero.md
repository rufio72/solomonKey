# Compito 44: H4a — giro dal boot (macchina, ingressi registrati, boot verificato)

Data: 2026-11-03. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Primo pezzo del lotto H4 (`compiti/PIANO-HARDWARE.md`): staccare il gioco
dall'oracolo. Si costruisce la **macchina**: boot vero dal vettore 0000 in C,
NMI una per frame, scheduler vero, porte alimentate da un file di ingressi
estratto dalla partita registrata. Il giro parte dal **frame 0** (non dal
2410), verifica il boot (frame 0-601) contro un oracolo MAME con l'impronta a
ogni NMI, e si ferma — atteso e misurabile — alla prima attività non tradotta:
`CORPO 1F (43C8) non tradotto al frame 602`. La traduzione dei corpi
attract/titolo (06, 07, 08, 0B, 0C, 1F, ~950 istruzioni) e il giro completo
0-5499 sono i lotti seguenti H4b/H4c (vedi Domande aperte 1).

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/`, `compiti/`, `STATO.md`.
- `harness/replay.c`, `sched_proto.c`, `sched.c`, `video_rendi.c`,
  `video_sdl.c`, `mostra_replay.c`, `rendi_oracolo.c` **non si toccano**.
  Il banco (304 voci, 1270 catture, 0 fallite), il replay storico e
  `sched_proto` devono restare verdi, bit a bit (verifiche 6-9).
- Nei sorgenti tradotti e verificati si possono solo **aggiungere ganci a
  puntatore di funzione, NULL per default** (precedente: `NEMICI_PUNTO`,
  `sched_gancio_scansione`, `replay_dopo_nmi`): a gancio spento il
  comportamento resta identico.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe. Compilatore `gcc` dal Makefile esistente, nessuna dipendenza
  nuova. Niente binari né `.o` nel commit.
- MAME solo senza finestra, con gli ingressi dell'host spenti (lo fa già
  `replay.sh`); a fine lavoro `pgrep -c mame` → 0.

## Cosa esiste già (fatti verificati)

| file | righe | ruolo |
|---|---:|---|
| `decomp/harness/replay.c` | 462 | giro guidato dall'oracolo dal frame 2410; modello di iniezione NMI (involucro, `prova_punto`, `gancio_scansione`); NON toccare, copiare le parti che servono |
| `decomp/harness/comune.c` | 27 | `ram[0x4000]` (c000-ffff), `rom[]`, `carica_rom()` da `out/main64.bin` |
| `decomp/harness/sched.c` | 46 | `spedisci_una`, `posta_attivita`, ganci `sched_gancio_scansione`/`sched_gancio_stub` |
| `decomp/harness/porte.c` | 38 | `leggi_porta(a)` = `ram[a-0xc000]`, `scrivi_porta` deposita in ram, `leggi_e603(pc)` |
| `decomp/harness/attivita.c` | 351 | `corpo_attivita_replay[]`: tradotti solo 01 02 03 0A 0D 0E 10 11 14 20 |
| `decomp/harness/z80_2xxx.c` | 567 | qui vanno `sub_29cf` e `sub_2a03`; contiene già `sub_2bf3` (r.398), `sub_2bc9` (r.348), `sub_2c1a` (r.324), `sub_2bc3`, `sub_2b08`, `sub_2af5`, `sub_2ae2`, `sub_2980`, `sub_3678`/`sub_36c3` (z80_3xxx.c r.332/349) |
| `decomp/harness/nmi.c` | 270 | `sub_0066` (r.144) tradotta e verificata; con c044 bit 2 = 0 (boot) salta la parte video (r.153-157) |
| `decomp/replay.lua` + `replay.sh` | 166+32 | oracolo: righe `F <f> <e600> <e601> <e602> <rit> <hash>` e registro `D/W/I/N`; impronta FNV-1a 32 su c000-e5ff esclusi c040-c043 e cf00-cfff |
| `decomp/out/main64.dasm` | — | disassemblato completo (z80dasm, installato) |
| `decomp/harness/Makefile` | 75 | `SORGENTI_REPLAY = $(filter-out harness.c,$(SORGENTI)) sched.c replay.c` |

Fatti sul boot (da `out/main64.dasm` e dalle misure sotto):
- **0000**: `xor a; ld (e600),a; jp 2833` (NMI spenta). Da `solomon.cpp`
  (sorgente MAME 0.264, letto in `/home/rufio72/mame-src/solomon.cpp`):
  scrittura e600 = maschera NMI (bit 0, il vblank genera la NMI solo se 1);
  lettura e600/e601/e602 = P1/P2/SYSTEM; e603 protezione (PC noto → 0, come
  `e603.h`); e604/e605 lettura = DSW1/DSW2; **scrittura e604 = flipscreen**
  (non rilegge mai il valore scritto); e606 lettura a vuoto (watchdog);
  scrittura e800 = comando audio. DSW misurati nei dump: e604=02, e605=00.
- **2833-2959** (sequenza di boot, 122 istruzioni eseguite misurate):
  I=0; IX=0; SP=dc00; test RAM a byte `sub_2bf3` su c000-c7ff e c800-cfff
  (ogni esito: `jr c` salta `inc ix`, poi `add ix,ix`); c044=0; **e600=1
  (285D: NMI accesa)**; azzera c000-cfff (2863-286C); SP=d000; test `2bf3`
  su d000-d7ff e d800-dfff; test a nibble `sub_2bc9` su e400-e4ff e
  e500-e5ff; test `2bf3` su e020-e07f; azzera d000+ (28BD, BC=8011) e
  e400-e5ff (28CA); se IX&7F≠0 e I<3 riparte da 2836 (in C i test passano
  sempre: IX resta 0, niente retry); c044=4; `sub_3678` → A=0 scritto su
  e604 (28F1: **flipscreen, non deve sporcare la DSW**); `call 2980`;
  `sub_2a03`; `call 2980`; `sub_29cf`; copia classifica 2908-2920 (10 voci:
  4 byte da 297A, 3 da HL=295C avanzante, 2 da 297E, verso ca4c); azzera
  d000-d007 (2922-292E); `sub_36c3`; riempie c1ba-c236 di 0 (2939, 7D byte);
  c04e=c04f=0; c1ae=0; `sub_673f`; e803=B7; `posta_attivita(0x1F)`;
  `posta_attivita(0x06)`; `jp 3AF3` (spedidore).
- **`sub_29cf`** (25 istruzioni eseguite, NON tradotta): griglia 14×16 di
  riquadri 2×2 via `sub_3a54` (tradotta), poi c467=0 e attesa a 29EB:
  `call sub_2bc3` finché c467 ≥ 0xE0 (224 frame; c467 lo incrementa la NMI);
  i rami 29F8/29FD (tasto premuto) nella registrazione non girano mai
  (primo ingresso al frame 1510).
- **`sub_2a03`** (110 istruzioni eseguite, NON tradotta): schermata esito
  test RAM: stringhe via `sub_2b08` (tradotta), bit di IX stampati (2A23 e
  2A35 ruotano L e H di IX), riempimenti d5a2+/d5c2+ (2A59-2AB5). Legge IX
  dal boot (in C sempre 0).
- Tutte le altre chiamate del boot sono tradotte e verificate (`ROUTINE.md`:
  0x2BF3, 0x2BC9, 0x2C1A, 0x2BC3, 0x2B08, 0x2AF5, 0x2AE2, 0x2980, 0x3678,
  0x36C3, 0x3A54, 0x673F, 0x0493/`posta_attivita`). `sub_2bc3` e il tratto
  2353-2367 leggono solo porte e RAM, **non scrivono**: una NMI al loro
  interno equivale a una NMI al confine dell'iterazione del ciclo chiamante.

Fatti sull'oracolo (misurati da `out/replay_oracolo.txt` e `replay_disp.txt`):
- 5434 righe F (una NMI per frame, 66-5499). Boot = frame 0-601: 536 NMI,
  nessuna nello spedidore; prima spedizione `D 602 00 1F`, poi `D 602 01 06`;
  prima NMI nello spedidore al frame 612 (`F 612 00 00 00 3AFD BAB7B252`).
- Gli **ingressi registrati stanno nei campi e600/e601/e602 delle righe F**
  (valore vivo delle porte alla NMI del frame; gli ingressi di MAME cambiano
  solo al confine di frame, quindi la riga F è completa). 3639 righe con
  porte attive; prima: `F 1511 00 00 04` (moneta, e602 bit 2), start
  `1576 00 00 01`; frame 0-65 senza riga F = porte a zero.
- Ritorni delle 536 NMI del boot: 97 distinti, tutti dentro questi tratti:
  0008-0014 (helper rst), 2353-2367, 2867/2869 (azzera 2863), 28BF/28CA
  (azzeramenti), 2929 (azzera 2922), 29EB-29F5 (attesa 29CF), 2A7A/2AB0/2AB5
  (cicli di 2A03), 2AE8-2B05 (2AE2/2AF5), 2B08-2B27 (2B08), 2BC3/2BC6,
  2BDA-2BE9 (2BC9), 2C00-2C26 (2BF3/2C1A), 3A65 (3A54).
- **Pila nell'impronta ai frame 66-67**: fino a 286E SP=dc00, la pila vive a
  dbxx dentro la zona dell'impronta; i dump misurati (`out/h4boot_66.bin`,
  `_67.bin`) mostrano a dbf6-dbff i residui delle push della NMI stessa, non
  riproducibili dal C (che non modella la pila). Dal frame 68 SP=d000 e la
  pila sta in cf00-cfff, già esclusa. Rimedio sotto: oracolo del boot con
  esclusione in più dbc0-dbff.
- L'oracolo del boot è **riproducibile**: la registrazione
  `REPLAY_NOME=h4boot REPLAY_F0=66 REPLAY_DUMPS=67,68,602 REPLAY_FINE=610
  sh replay.sh` (fatta oggi, ~20 s) produce 544 righe F **identiche** alle
  prime 544 di `out/replay_oracolo.txt`; i dump sono già in
  `out/h4boot_*.bin`. Nel dump 602: e604=02, e605=00.

## Struttura da realizzare

1. **`decomp/estrai_ingressi.sh`** (nuovo, ~10 righe): da `decomp/`,
   `awk '$1=="F"{print $2, $3, $4, $5}' out/replay_oracolo.txt >
   out/ingressi_replay.txt` più controllo d'esistenza. Formato: una riga per
   NMI, `<frame> <e600> <e601> <e602>` esadecimali come nell'oracolo; i frame
   senza riga valgono 00 00 00.
2. **`decomp/giro_boot.lua` + `decomp/giro_boot.sh`** (nuovi): copia di
   `replay.lua`/`replay.sh` con una sola differenza di sostanza: l'impronta
   esclude **anche dbc0-dbff** (pila del boot). Predefiniti: nome `giroboot`,
   F0=602, FINE=2410, dump a 602. I campi 1-6 delle righe F devono restare
   identici a `replay_oracolo.txt` (verifica 3).
3. **`harness/macchina.c`** (nuovo, ~400-500 righe): la macchina del giro
   dal boot.
   - Caricamento di oracolo (righe F), registro (D/N) e file ingressi
     (parser ricopiati da `replay.c`, che resta intatto).
   - `boot_2833()`: trascrizione fedele della sequenza 2833-2959 (sopra),
     coi `call` alle funzioni C già tradotte. SP e registro I non si
     modellano (commento nel codice). IX modellato come variabile locale.
   - **Porte**: prima di eseguire la NMI del frame f si scrivono in
     `ram[e600..e602]` i valori della riga ingressi di f (semantica MAME:
     la NMI legge le porte del frame). e604/e605 inizializzate a 02/00.
     In `porte.c` si aggiunge un gancio NULL per default
     `void (*porte_gancio_scrittura)(uint16_t a, uint8_t v)` consultato in
     testa a `scrivi_porta`: la macchina lo usa per deviare le scritture a
     e600 (maschera NMI, variabile propria, sanità: a ogni NMI del tracciato
     la maschera deve valere 1) e a e604 (latch flipscreen, così la DSW in
     ram non viene sporcata dalla scrittura di 28F1).
   - **Collocazione delle NMI nel boot** (il punto delicato): gancio
     `void (*macchina_punto)(uint16_t da, uint16_t a)` chiamato a ogni
     iterazione dei cicli del percorso di boot con l'intervallo di indirizzi
     Z80 coperto dall'iterazione (compresi i leaf senza scritture chiamati
     dentro: 2353-2367, 2BC3-2C19, 0008-0026). La NMI pendente (prossima
     riga F/N) si esegue al primo punto in cui il ritorno della riga N cade
     nell'intervallo **e** l'impronta corrente (variante boot, con dbc0-dbff
     esclusa) coincide con l'hash della riga F. Se il percorso supera la
     finestra legale senza coincidenza, il giro si ferma con una diagnostica
     (frame, ritorno, hash C e MAME). Ganci da inserire (a puntatore NULL,
     stile `NEMICI_PUNTO`): nei cicli interni di `sub_2bf3`, `sub_2bc9`,
     `sub_2c1a`, `sub_2b08`, `sub_2af5`, fra le due chiamate di `sub_2ae2`,
     nel ciclo di `sub_3a54` usato da 29CF, più i cicli scritti ex novo
     (azzeramenti e copie del boot, cicli di `sub_29cf`/`sub_2a03`).
     Attenzione: il ciclo di attesa 29EB deve passare dal punto a ogni
     iterazione, altrimenti la NMI che incrementa c467 non viene mai
     eseguita e il giro non termina.
   - Dal `jp 3AF3`: stesso schema di `replay.c` — `corpo_attivita[i]` tutti
     a un involucro che confronta slot/codice con la riga D, esegue il corpo
     se presente in `corpo_attivita_replay[]`, altrimenti stampa
     `CORPO %02X (%04X) non tradotto al frame %ld` (indirizzo dalla tabella
     3B11, copiabile da `rst20_tables.h` r.49-52) ed esce con 1.
   - Al frame con dump (602): confronto byte per byte con
     `out/giroboot_602.bin` su c000-e5ff meno le esclusioni, prima di
     eseguire la NMI di quel frame (stessa convenzione della SANITA F0 del
     replay), con messaggio `SANITA 602: ... coincide`.
4. **`harness/gioco_libero.c`** (nuovo, ~80 righe): `main`; uso:
   `harness/gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt
   out/ingressi_replay.txt [fine]`. Riepilogo finale sullo stile del replay:
   `NMI confrontate: N, divergenti: M`.
5. **`sub_29cf` e `sub_2a03`** in `z80_2xxx.c` (+ dichiarazioni in
   `routine.h`): traduzione dal disassemblato
   (`awk '/^sub_29cfh:/,/^sub_2ae2h:/' out/main64.dasm`). Non catturabili
   dal banco (durano centinaia di frame, come `sub_2bc9`): la verifica è il
   giro stesso; nota in `ROUTINE.md` come per 2BC9. z80_2xxx.c resta sotto
   le 800 righe (567 + ~130).
6. **Makefile**: bersaglio `gioco_libero` =
   `$(filter-out harness.c,$(SORGENTI)) sched.c macchina.c gioco_libero.c`;
   aggiunto ad `all` e a `clean`.
7. Documento nuovo `decomp/GIRO.md` (boot, modello dei punti, esclusioni,
   uso) e aggiornamento di `STATO.md`; in `PIANO-HARDWARE.md` la voce H4
   resta aperta con nota «H4a fatto, restano H4b/H4c».

## Verifiche obbligatorie

Dalla radice `decomp/` se non detto altrimenti. Le uscite segnate (m) sono
misurate oggi; le altre sono attese dal progetto e vanno riportate esatte.

1. `cd harness && make -B all gioco_libero 2>&1 | grep -c -i -E 'warning|error'` → `0`.
2. `sh estrai_ingressi.sh && wc -l < out/ingressi_replay.txt && sha1sum out/ingressi_replay.txt` →
   `5434` e `43934a8eeed8d44a071d7ab958d32ea74fbd437f` (m).
3. `REPLAY_FINE=610 sh giro_boot.sh` → `REPLAY giroboot: 544 righe F, 2 spedizioni` (m, da h4boot);
   poi `diff <(head -544 out/replay_oracolo.txt | awk '{print $1,$2,$3,$4,$5,$6}') <(awk '{print $1,$2,$3,$4,$5,$6}' out/giroboot_oracolo.txt)` → vuoto (m: senza
   l'esclusione nuova le righe intere coincidono già).
4. Rigenerare con i predefiniti (`sh giro_boot.sh`, FINE=2410, ~45 s) e
   lanciare `harness/gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt out/ingressi_replay.txt; echo "exit $?"` →
   (attesa, non misurata — il formato è prescritto sopra):
   `SANITA 602` con `coincide`, `NMI confrontate: 537, divergenti: 0`,
   `CORPO 1F (43C8) non tradotto al frame 602`, `exit 1`.
   537 = frame 66-602 compresi. Se una NMI del boot non trova il punto, va
   riportata la diagnostica e NON si allenta l'impronta senza una misura.
5. `pgrep -c mame` → `0`.
6. Banco intatto: `cd harness && ./harness --tutte > /tmp/h44.txt; echo "exit $?"; grep -c FALLITO /tmp/h44.txt; grep -c '^RIEPILOGO' /tmp/h44.txt; grep -c '^cattura' /tmp/h44.txt` →
   `exit 0`, `0`, `304`, `1270` (m, brief 42-43).
7. Replay storico intatto: `harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r44.txt 2>&1; echo "exit $?"; tail -4 /tmp/r44.txt` → `exit 0` e le 4 righe note (m, brief 42-43):
   `spedizioni eseguite: 64686, fino al frame 5499` / `NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885` / `prima divergenza a metà attività (attesa): frame 2415` / `nessuna divergenza nello spedidore`.
8. `harness/sched_proto out/sched_log.txt | tail -1` →
   `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (m).
9. `sh rendi_video.sh; echo "exit $?"` → `RENDI 7/7 identici`, `exit 0` (m, brief 43).
10. `git diff --stat -- decomp/harness/replay.c decomp/harness/sched.c decomp/harness/sched_proto.c decomp/harness/video_rendi.c decomp/harness/video_sdl.c decomp/harness/mostra_replay.c` → vuoto.
11. Righe: `wc -l harness/macchina.c harness/gioco_libero.c harness/z80_2xxx.c harness/porte.c | awk '$1>800 && $2!="totale"'` → vuoto.
12. Ambito: `git status --short` dalla radice → solo i file di questo brief
    (nuovi: `decomp/giro_boot.lua`, `giro_boot.sh`, `estrai_ingressi.sh`,
    `harness/macchina.c`, `harness/gioco_libero.c`, `decomp/GIRO.md`,
    `compiti/44-…`; modificati: `harness/z80_2xxx.c`, `routine.h`,
    `porte.c`, eventuali altri sorgenti con i soli ganci, `Makefile`,
    `.gitignore` se serve, `ROUTINE.md`, `STATO.md`,
    `compiti/PIANO-HARDWARE.md`). Nessun binario:
    `git status --short | grep -c -E 'gioco_libero$|\.o$|\.bin$'` → `0`.

## Consegna

Commit unico «Lotto H4a: giro dal boot (macchina, ingressi registrati)» con
riga finale `Co-Authored-By: <modello vero> <noreply@anthropic.com>`. Non
fare push. In fondo a questo brief: esito e uscita delle verifiche. Rapporto
finale: massimo trenta righe con l'uscita delle 12 verifiche, il numero di
punti-gancio inseriti e le deviazioni.

## Domande aperte

1. **Scelta di progetto — spezzatura di H4.** Il giro libero completo
   richiede i corpi attract/titolo non tradotti: 1F (43C8, 13 istruzioni
   eseguite), 06 (75E7-797E, 90), 07 (2CCD, ~160 in 2C27-2E11 più i passi in
   2E11-2F57, ~150 lorde), 08+0C (7B00-7DEA, 161 lorde), 0B (piccola, via
   673F), più la regione fine-quadro/titolo 6253-673F (~420 lorde): in tutto
   ~950 istruzioni ≈ 2 lotti di traduzione (metro: brief 26 = 503 istruzioni
   in un lotto). Proposta: **H4a = questo brief** (macchina, boot, ingressi,
   arresto atteso al corpo 1F); **H4b** = traduzione dei corpi attract con
   avanzamento del giro (stesso schema dei lotti N: si traduce il corpo dove
   il giro si ferma), più l'**oracolo del registro R** (siti `ld a,r`:
   2D1F/2D22 nel corpo 07, 430A/54C5/59E5/5DDF nel motore nemici della demo;
   registrazione MAME con breakpoint che logga frame e R, da dare al giro
   come file; nel gioco libero vero R diventa un contatore deterministico);
   **H4c** = giro completo 0-5499: confronto byte per byte con
   `replay_2410.bin` al frame 2410 e poi oracolo storico fino al 5499
   (attese le stesse 885 divergenze a metà attività), più hash dei
   framebuffer con `rendi_frame` sui frame campione.
2. **Scelta di progetto — esclusione dbc0-dbff nell'impronta del boot.**
   Ai frame 66-67 la pila (SP=dc00) lascia in dbf6-dbff i residui delle push
   della NMI, non riproducibili senza emulare la pila; dbc0-dbff è però
   colorram di sfondo, quindi l'esclusione vale solo per l'oracolo del boot
   (`giro_boot.lua`), non per quello storico, e il buco è richiuso dal
   confronto byte per byte al frame 602 (il dump misurato lì ha la zona
   azzerata dal clear di 28B6). Proposta: così.
3. **Preferenza — 1F fuori da questo lotto.** Il corpo 1F è minuscolo (13
   istruzioni, «diventa 14») ma la sua verifica naturale è il giro di H4b:
   qui resta stub e l'arresto atteso è su di lui. Proposta: così.
4. **Preferenza — formato del file ingressi** `<frame> <e600> <e601> <e602>`
   esadecimale, una riga per NMI registrata, frame mancanti = 0. Proposta: così.
5. **Preferenza — nomi**: `macchina.c`, `gioco_libero.c`, `giro_boot.lua/sh`,
   `estrai_ingressi.sh`, `out/ingressi_replay.txt`, documento `GIRO.md`,
   gancio `macchina_punto`, gancio `porte_gancio_scrittura`. Proposta: così.
6. **Preferenza — DSW fissi** 02/00 (Upright, demo sound on, come la
   registrazione); le opzioni da riga di comando arrivano con H5. Proposta: così.

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

1. (Scelta di progetto, risolta dalla chat principale sotto il mandato
   «vai avanti col ciclo» — la spezzatura è obbligata dai numeri: ~950
   istruzioni ≈ 2 lotti di traduzione col metro del brief 26:) sì,
   H4a = questo brief (macchina, boot, ingressi, arresto atteso al corpo
   1F); H4b = traduzione dei corpi attract/titolo con avanzamento del
   giro più l'oracolo del registro R; H4c = giro completo 0-5499 col
   confronto al 2410 e poi con l'oracolo storico. Il piano hardware si
   aggiorna di conseguenza. Da segnalare all'utente.
2. (Scelta di progetto, risolta dalla chat principale: tecnicamente
   forzata, con rete di sicurezza:) sì, dbc0-dbff esclusa dall'impronta
   del solo oracolo boot; il confronto byte per byte al frame 602
   richiude il buco. Da segnalare all'utente.
3. Sì (preferenza): il corpo 1F resta stub, l'arresto atteso è su di lui.
4. Sì (preferenza): formato ingressi `<frame> <e600> <e601> <e602>`.
5. Sì (preferenza): nomi come proposti.
6. Sì (preferenza): DSW fissi 02/00; le opzioni arrivano con H5.

## Esito (sviluppatore, 2026-11-03)

Tutte le 12 verifiche passate al primo giro. Uscite: (1) `0`; (2) `5434` e
`43934a8e...b437f`; (3) `REPLAY giroboot: 544 righe F, 2 spedizioni`, diff vuoto;
(4) `SANITA 602: ... 0, coincide`, `CORPO 1F (43C8) non tradotto al frame 602`,
`NMI confrontate: 537, divergenti: 0`, `exit 1`; (5) `0`; (6) `exit 0`, `0`, `304`,
`1270`; (7) `exit 0` e le 4 righe note; (8) `106124, differenze: 0; ... 131/131`;
(9) `RENDI 7/7 identici`, `exit 0`; (10) vuoto; (11) vuoto; (12) solo i file del
brief più `.gitignore`, `0` binari.
Punti inseriti: 29 nelle routine verificate, 47 nel codice nuovo, 2 per byte nei
cicli di azzeramento del boot. Deviazioni: il gancio `punto` rieseguibile
(più NMI nello stesso tratto a RAM costante); `porte_gancio_scrittura` consuma
la scrittura (non deposita in ram[]); il brief diceva «azzera d000-d007» ma
2922 azzera d000-d7ff (BC=0008); la mappa dei fatti sul boot non cambia.
