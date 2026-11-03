# Compito 48: H4c — giro completo 0-5499 e hash dei framebuffer

Data: 2026-11-03. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Ultimo lotto del giro dal boot (`compiti/PIANO-HARDWARE.md`, voce H4; quadro in
`decomp/GIRO.md`, brief 44 e 47). Il giro dal boot (`harness/gioco_libero`) arriva
già al frame 2410 e si salda a `out/replay_2410.bin`. Qui lo si fa proseguire fino
al frame 5499 (fine dell'oracolo storico), si aggiungono gli hash dei framebuffer
(`rendi_frame`) su 13 frame campione e si chiude tutto in **una sola riga di
regressione**:

`GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 33 a metà attività (attese), 13 frame resi con hash coincidenti`

Il tratto 2411-5499 è già misurato: **non ha arresti** (nessuna routine o corpo
mancante). Il lavoro è quindi: oracolo esteso, hook video, script di regressione,
documenti. Nessuna traduzione di codice Z80 è prevista; se ne emergesse una
(arresto non previsto) vale la politica del brief 47: piccola (~30 istruzioni) si
traduce, grande ci si ferma e si riporta.

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/`, `compiti/`, `STATO.md`. `README.md` non si tocca.
- Non si toccano: `harness/replay.c`, `sched.c`, `sched_proto.c`, `video_rendi.c`,
  `video_sdl.c`, `mostra_replay.c`, `rendi_oracolo.c`, `z80_2xxx.c` (796 righe).
  Banco storico (307 voci, 1278 catture, 0 fallite), replay e `sched_proto`
  restano verdi bit a bit. Nei sorgenti verificati si possono solo aggiungere ganci
  a puntatore NULL.
- `macchina.c` (614 righe) può solo ricevere le chiamate al modulo video e la riga
  finale (vedi «Struttura»); resta sotto le 650 righe. Nessun file di codice sopra
  le 800 righe; ogni file nuovo sotto le 650.
- Commenti, messaggi, documenti in italiano. `gcc` dal Makefile, nessuna
  dipendenza nuova (SDL non entra nel giro: `vsdl_hash` non si usa, si copia la
  funzione), niente binari né `.o` nel commit.
- MAME solo senza finestra, ingressi dell'host spenti (lo fa già `giro_boot.sh`);
  a fine lavoro `pgrep -c mame` → `0`. Niente git distruttivo.
- Non si allenta l'impronta e non si aggiungono esclusioni alla saldatura oltre a
  c040-c043 e cf00-cfff.

## Cosa esiste già (fatti verificati)

Prototipo misurato in una copia fuori dal repository (stesso codice, `giro_boot.sh`
con `REPLAY_FINE=5500`, `macchina.c` con un controllo temporaneo dei byte video):

| fatto | valore (m = misurato) |
|---|---|
| tempo di `giro_boot.sh` fino al 5500 | 13 s (m) |
| righe di `giroboot_oracolo.txt` / `giroboot_disp.txt` estesi | 5434 F; 112856 righe, 106104 D (m) |
| prefisso dell'oracolo esteso rispetto a quello attuale | identico: `head -2345` md5 `3cbf06dfea3f92539d8d83bc8d90c6bf`; `head -44764` del registro md5 `4086b7fff87773591115e6d4bec581e9` (m) |
| registro esteso contro `out/replay_disp.txt` | `cmp` identici (MAME deterministico) (m) |
| md5 file estesi | oracolo `6357869d623b3927e1f85b1a9c2ea3f1`, registro `8c46a6d7ee0debee1f9f5dc0957da215` (m) |
| `gioco_libero` sui file estesi (1,1 s, exit 0) | vedi verifica 4 (m) |
| 6253-673F nel tratto 2410-5499 | non eseguita: la traccia `out/replay_gioco_eseguiti.txt` ha 15 indirizzi nella regione (inizio 63A4: corpo 0E, già tradotto; `673F` è nel banco); il giro arriva al 5499 senza arresti (m) |

Perché i numeri a metà attività sono 33 e non 885: il replay storico (`replay.c`)
non ha i ganci `PUNTO_GIRO` di H4b3 (attivi solo con `giro_dal_boot`), il giro dal
boot sì. Il tratto 2411-5499 conta 3089 NMI: 1485 nello spedidore (0 divergenti) e
1604 a metà attività (3 divergenti; 30 sono quelle di 66-2410 del brief 47). Il
replay storico resta invariato a 885 ed è una regressione separata.

Dump di MAME e hash dell'impronta: l'impronta di `macchina.c` (FNV-1a 32 bit con
dbc0-dbff esclusa) differisce da quella di `replay.c`: gli hash di
`replay_oracolo.txt` dopo il 2410 **non** valgono per il giro dal boot (es. frame
2410: `A350CF53` contro `39630C53`). Per questo l'oracolo del giro si rigenera
esteso, invece di concatenare i due.

Video (da `decomp/VIDEO.md`, `harness/video.h`, misurato):
- `void rendi_frame(const vid_ram *ram, uint8_t rgb[VID_ALTO][VID_LARGO][3]);`
  con `vid_ram = { fg_colorram (d000), fg_videoram (d400), bg_colorram (d800),
  bg_videoram (dc00), spriteram (e000, 128 byte), palette (e400, 512 byte) }`, tutti
  `const uint8_t *`; nel giro puntano dentro `ram[]` (indice `a - 0xc000`).
  `VID_ALTO` 224, `VID_LARGO` 256.
- Hash: FNV-1a 64 bit sui 172032 byte RGB, come `vsdl_hash` (`video_sdl.c` r. 156):
  `h = 0xcbf29ce484222325; h = (h ^ byte) * 0x100000001b3`.
- Bersagli: `rendi_oracolo: rendi_oracolo.c video_rendi.c $(SORGENTI_GRAFICA) video.h`
  (Makefile r. 61); `SORGENTI_GRAFICA = $(wildcard video_dati_*.c)` (r. 59);
  `gioco_libero: $(SORGENTI_LIBERO) ...` con `SORGENTI_LIBERO = $(filter-out
  harness.c,$(SORGENTI)) sched.c macchina.c gioco_libero.c` (r. 37-41).
- Hash di riferimento già noti, da `harness/mostra_oracolo -n 1 <stato>` con
  `SDL_VIDEODRIVER=dummy` (m): titolo `ecd5acf694285489` (stato validato contro il
  PNG di MAME, frame 1000), gioco `00d246e511baf342` (frame 2400, vedi nota sotto).
- Punto di confronto: la RAM del C prima della NMI del frame (dentro `esegui_nmi`,
  `macchina.c` r. 235-262, dove oggi `if (r->frame == FRAME_SALDATURA) saldatura();`,
  prima della scrittura delle porte e di `sub_0066`) coincide con l'istante in cui
  `giro_boot.lua` scarica `out/giroboot_<F>.bin` (c000-ffff, 16384 byte).
- Prototipo (m): a ognuno di 13 frame campione i byte d000-e5ff del C coincidono
  con quelli del dump di MAME (0 diversi); in c000-cfff i diversi sono 25-31 e
  stanno solo in c040-c043 e cf00-cfff (già escluse). I frame: 700, 1000, 1300,
  1600, 1900, 2100, 2400, 3000, 3500, 4000, 4500, 5000, 5499 (ritorni NMI 3B0B,
  3AFA, 3AFB, 3AFB, 4E55, 4DD4, 3AFB, 3AFB, 4EF0, 3AFE, 3B00, 3B00, 3AFA: sia
  pulite sia a metà attività, tutte con video coincidente).
- Nota sui PNG: dei sei stati di `out/video` solo `titolo` (frame 1000, ingressi
  nessuno) è confrontabile con il giro: 0 byte diversi su tutte le RAM video.
  `gioco` (2400) ha 2 byte di sprite diversi dal dump del giro (le altre 5 aree a
  0), quindi il suo hash `00d246...` non vale per il giro; `stanza10`, `bonus`,
  `finale`, `finale2` sono stati con stanza forzata o poke e non sono raggiunti dal
  giro. Per questo il riferimento del giro è il dump `giroboot_<F>.bin` reso dallo
  stesso `rendi_frame`; il PNG vale come conferma indipendente solo per il 1000.

## Struttura da realizzare

1. **Oracolo esteso** (`decomp/giro_boot.sh`): default `REPLAY_FINE=5500` (era 2411)
   e `REPLAY_DUMPS=700,1000,1300,1600,1900,2100,2400,2410,3000,3500,4000,4500,5000,5499`
   (era 2410). Aggiornare l'intestazione del file. Nessun'altra modifica;
   `giro_boot.lua` non si tocca. I file `out/giroboot_*` (gitignorati) vengono
   rigenerati: i vecchi sono un prefisso di quelli nuovi (tabella sopra).
2. **Modulo video del giro** `harness/giro_video.c` (nuovo, sotto 150 righe)
   prototipi in `harness/giro_video.h` (nuovo).
   Interfaccia:
   `void giro_video_campione(long frame, const uint8_t *ram_c000, const char *prefisso);`
   `int giro_video_riepilogo(long *campioni, long *coincidenti);`
   - Tabella `static const struct { long frame; uint64_t hash; } campione[]` con i
     13 frame sopra e l'hash di riferimento (FNV-1a 64 del framebuffer reso dalla
     RAM del C). I valori li misura lo sviluppatore alla prima corsa (nessuno è noto
     in anticipo, salvo il confronto col dump) e li incolla nella tabella.
   - `giro_video_campione`: se `frame` è in tabella, rende `ram_c000` con
     `rendi_frame`, calcola l'hash C; legge `<prefisso>_<frame>.bin` (stessa
     costruzione del nome di `diff_dump`, `macchina.c` r. 183), rende le sue aree
     video (offset `a - 0xc000`) e calcola l'hash MAME. Stampa
     `FRAME_VIDEO <frame>: hash C <16 hex> MAME <16 hex> tabella <16 hex>, coincide`
     oppure `..., DIVERSO` (basta una delle due discrepanze: C diverso da MAME o
     C diverso dalla tabella). Dump assente: stampa `dump <nome> assente`, conta
     come DIVERSO.
   - `giro_video_riepilogo`: ritorna 0 se tutti i campioni in tabella sono stati
     visti e coincidono; riporta quanti.
3. **`macchina.c`** (solo queste modifiche, file sotto 650 righe): in `esegui_nmi`
   dopo `saldatura()` la chiamata
   `giro_video_campione(r->frame, ram, prefisso);` (prototipo da `giro_video.h`);
   in `riepilogo()` o a fine `macchina_esegui` (dove sono già note le statistiche,
   r. 151-157) la riga finale (punto 4). `ram` è l'array globale di `ram.h`.
4. **Riga finale**, stampata da `macchina_esegui` solo se il giro arriva in fondo
   senza `ferma()` e senza arresti, con il testo esatto
   `GIRO COMPLETO 0-<F>: saldatura <N> byte, spedidore <D> divergenze, <M> a metà attività (attese), <K> frame resi con hash coincidenti`
   (F = ultimo frame dell'oracolo, N = byte diversi alla saldatura, D = divergenti
   nello spedidore, M = divergenti a metà attività, K = campioni coincidenti). Se
   N, D o i campioni non tornano, stampa `GIRO COMPLETO 0-<F>: FALLITO (<motivo>)`
   e il codice di uscita è 1. Le divergenze a metà attività non fanno fallire il
   giro (attese). Il codice di uscita resta 0 se tutto torna.
5. **Makefile**: `SORGENTI_LIBERO` aggiunge `giro_video.c video_rendi.c
   $(SORGENTI_GRAFICA)`; `gioco_libero` dipende anche da `video.h giro_video.h`.
   Gli altri bersagli non cambiano (`replay`, `harness`, `sched_proto` non vedono il
   modulo video).
6. **Script di regressione** `decomp/giro_completo.sh` (nuovo, POSIX sh, sotto 40
   righe): da `decomp/`, `sh giro_completo.sh` esegue
   `harness/gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt
   out/ingressi_replay.txt`, stampa solo le righe `SALDATURA`, `NMI confrontate`,
   `dopo il boot` e la riga `GIRO COMPLETO`, verifica che M valga esattamente il
   numero atteso (33, vedi domanda 3; se differisce stampa `M diverso dall'atteso`
   ed esce 1) e restituisce il codice di uscita del giro. Con `REGEN=1` rilancia
   prima `sh giro_boot.sh`. Se i file `out/giroboot_*` mancano o hanno meno di 5434
   righe F, stampa il comando per rigenerarli ed esce 2.
7. **Oltre il 2410 non c'è nulla da tradurre** (m). Se la corsa fermasse (arresto
   non previsto: `ROUTINE`, `CORPO`, `INDICE`, `DIVERGENZA`, `R letto`), riportare
   frame e causa e applicare la politica in testa.
8. **Divergenze oltre il 2410**: le 3 NMI divergenti a metà attività dopo il 2410
   non sono identificate. Aggiungere a `macchina.c` (stesso budget di righe) una
   stampa `MEZZA frame <F> ritorno <R>` per ogni NMI a metà attività divergente
   (solo in `giro_completo.sh` con `RIGHE=1`, oppure sempre ma filtrata dallo
   script: scelta libera) e riportare nel rapporto i 33 frame e i ritorni; se la
   causa è un punto mancante evidente, aggiungere `PUNTO_GIRO`/`PUNTO_LARGO` come
   in H4b3 (M scende; allora la costante dello script e la riga finale cambiano,
   vedi domanda 3).
9. **Regione 6253-673F**: non si traduce. Annotarla in `GIRO.md`: nel giro 0-5499
   non si esegue oltre 63A4-63C2 (corpo 0E) e `673F` (banco); resta fuori
   dall'harness, da affrontare solo se un giro più lungo (H5, partita libera) la
   raggiunge.
10. **Documenti**: `decomp/GIRO.md` (intestazione: giro 0-5499; nuova sezione H4c con
    oracolo esteso, tabella dei 13 frame con hash, riga finale, differenza 33/885,
    regione 6253-673F), `STATO.md` (voce H4c; aggiornare dove dice che resta H4c),
    `compiti/PIANO-HARDWARE.md` (H4 fatto, restano H5 e oltre; spuntare la voce),
    `decomp/REPLAY.md` solo se cita il giro dal boot (altrimenti no).

## Verifiche obbligatorie

Da `decomp/` salvo diversa indicazione. (m) = misurato in copia fuori dal
repository; (a) = attesa, non misurata. Riportare esatto ciò che esce.

1. `cd harness && make -B all gioco_libero 2>&1 | grep -c -i -E 'warning|error'` → `0`.
2. `sh giro_boot.sh` → `REPLAY giroboot: 5434 righe F, 106104 spedizioni` (m);
   `ls out/giroboot_*.bin | wc -l` → `15` (602 e i 14 di `REPLAY_DUMPS`) (m, su 13+602+2410).
3. Oracolo esteso coerente con il precedente: `head -2345 out/giroboot_oracolo.txt |
   md5sum` → `3cbf06dfea3f92539d8d83bc8d90c6bf  -`; `head -44764 out/giroboot_disp.txt |
   md5sum` → `4086b7fff87773591115e6d4bec581e9  -`; `cmp out/giroboot_disp.txt
   out/replay_disp.txt && echo identici` → `identici`; `md5sum out/giroboot_oracolo.txt
   out/giroboot_disp.txt` → `6357869d623b3927e1f85b1a9c2ea3f1` e
   `8c46a6d7ee0debee1f9f5dc0957da215` (tutti m).
4. Giro senza la parte video (m, da prototipo; il testo di queste righe non cambia):
   `harness/gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt
   out/ingressi_replay.txt; echo "exit $?"` contiene:
   ```
   SANITA 602: byte diversi in c000-e5ff (escluse c040-c043, cf00-cfff, dbc0-dbff): 0, coincide
   SALDATURA 2410: byte diversi in c000-e5ff (escluse c040-c043, cf00-cfff): 0, coincide
   spedizioni eseguite: 106082
   NMI confrontate: 5434, divergenti: 33
   dopo il boot: nello spedidore 2617 (divergenti 0), a metà attività 2280 (divergenti 33)
   ```
   (106104 D nel registro meno 22 dopo l'ultima N: non si eseguono) e nessuna riga
   `R letto`, `DIVERGENZA`, `ROUTINE`, `CORPO`, `INDICE`; `exit 0`.
5. Hook video: stesse corse con il modulo nuovo: 13 righe `FRAME_VIDEO <f>: ... coincide`
   (frame 700, 1000, 1300, 1600, 1900, 2100, 2400, 3000, 3500, 4000, 4500, 5000,
   5499) e `grep -c DIVERSO` → `0` (a: il confronto C-contro-MAME è misurato a
   livello di byte, l'hash è da misurare). Il hash del frame 1000 vale
   `ecd5acf694285489` (m, dallo stato `titolo` validato col PNG).
6. Riga finale e script: `sh giro_completo.sh; echo "exit $?"` → termina con
   `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 33 a metà attività (attese), 13 frame resi con hash coincidenti`
   (a: M = 33 salvo diversa misura dopo il punto 8) e `exit 0`; `REGEN=1 sh
   giro_completo.sh` dà lo stesso esito (a).
7. Prova negativa (a): alterare per un istante un valore della tabella `campione[]`
   o rinominare `out/giroboot_1000.bin` fa uscire il giro con `FALLITO` e `exit 1`;
   riportarlo e ripristinare. Non si committa l'alterazione.
8. Ingressi e oracolo storico invariati: `sha1sum out/ingressi_replay.txt` →
   `43934a8eeed8d44a071d7ab958d32ea74fbd437f`; `cmp out/giroboot_2410.bin
   out/replay_2410.bin && echo identici` → `identici` (m); `grep -c '^F '
   out/giroboot_oracolo.txt` → `5434` (m).
9. Regione 6253-673F: `grep -c -i -E '^(625[3-9a-f]|62[6-9a-f].|6[3-6]..|67[0-2].|673.)$'
   out/replay_gioco_eseguiti.txt` → `15` (m); documentato in `GIRO.md`.
10. Banco: `cd harness && ./harness --tutte > /tmp/h48.txt; echo "exit $?"; grep -c
    FALLITO /tmp/h48.txt; grep -c '^RIEPILOGO' /tmp/h48.txt; grep -c '^cattura'
    /tmp/h48.txt` → `exit 0`, `0`, `307`, `1278`.
11. Replay storico: `harness/replay out/replay_oracolo.txt out/replay_disp.txt
    out/replay_2410.bin 2410 > /tmp/r48.txt 2>&1; echo "exit $?"; tail -4 /tmp/r48.txt`
    → `exit 0` e (m, brief 47): `spedizioni eseguite: 64686, fino al frame 5499` /
    `NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti:
    nello spedidore 0, a metà attività 885` / `prima divergenza a metà attività
    (attesa): frame 2415` / `nessuna divergenza nello spedidore`.
12. `harness/sched_proto out/sched_log.txt | tail -1` → `spedizioni confrontate:
    106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (m, brief 47);
    `sh rendi_video.sh; echo "exit $?"` → `RENDI 7/7 identici`, `exit 0` (m, brief 47).
13. `git diff --stat -- decomp/harness/replay.c decomp/harness/sched.c
    decomp/harness/sched_proto.c decomp/harness/video_rendi.c
    decomp/harness/video_sdl.c decomp/harness/mostra_replay.c
    decomp/harness/rendi_oracolo.c decomp/harness/z80_2xxx.c decomp/giro_boot.lua
    README.md` (dalla radice) → vuoto.
14. `wc -l harness/*.c | awk '$1>800 && $2!="totale"'` → vuoto; `wc -l harness/macchina.c
    harness/giro_video.c` → ciascuno sotto 650.
15. `pgrep -c mame` → `0`. `git status --short` dalla radice → solo: `compiti/48-h4c-giro-completo.md`,
    `compiti/PIANO-HARDWARE.md`, `STATO.md`, `decomp/GIRO.md`, `decomp/giro_boot.sh`,
    `decomp/giro_completo.sh`, `decomp/harness/{macchina.c,giro_video.c,giro_video.h,Makefile}`
    (e `REPLAY.md` se toccato); `git status --short | grep -c -E 'gioco_libero$|\.o$|\.bin$'` → `0`.

## Consegna

Commit unico «Lotto H4c: giro completo 0-5499, oracolo esteso e hash dei framebuffer»
con riga finale `Co-Authored-By: <modello vero> <noreply@anthropic.com>`. Niente
push. In fondo a questo brief: esito e uscita delle verifiche. Rapporto finale:
massimo trenta righe con l'uscita delle 15 verifiche, la riga `GIRO COMPLETO`, la
tabella frame/hash, i 33 frame divergenti a metà attività (almeno i 3 oltre il
2410, con ritorno e causa), eventuali arresti e deviazioni. Nota per la chat
principale: `README.md` ha la riga del banco/giro da aggiornare (la fa lei).

## Domande aperte

1. **Preferenza — oracolo esteso sostituisce quello attuale.** `giro_boot.sh`
   rigenera `out/giroboot_*` fino al 5500 (13 s; i file vecchi sono un prefisso dei
   nuovi, verifica 3), invece di concatenare con `replay_oracolo.txt` (impossibile:
   impronta diversa). Proposta: così.
2. **Preferenza — frame campione.** I 13 frame sopra (due stati naturali del giro,
   titolo e gioco, più 11 lungo la partita), non i sei stati dell'oracolo video:
   quattro di quelli (`stanza10`, `bonus`, `finale`, `finale2`) usano stanza forzata
   o poke e il giro non li raggiunge; `gioco` ha 2 byte di sprite diversi dal giro.
   Il riferimento è il dump `giroboot_<F>.bin` reso con `rendi_frame`; il PNG vale
   per il solo frame 1000. Proposta: così.
3. **Preferenza — numero delle divergenze a metà attività.** Lo script di
   regressione lo vuole esatto (33 oggi) per accorgersi di una regressione; se lo
   sviluppatore le riduce (punto 8) la costante scende e si annota. Proposta: esatto,
   non soglia.
4. **Preferenza — nomi.** `harness/giro_video.c`/`.h`, `decomp/giro_completo.sh`, riga
   `FRAME_VIDEO`, riga `GIRO COMPLETO`. Proposta: così.
5. **Scelta di progetto — riduzione delle 33 divergenze.** Il punto 8 chiede di
   identificarle e, se la causa è un punto mancante evidente, di sanarla. Perimetro
   alternativo: solo identificare e documentare, senza toccare i moduli verificati.
   Cambia quanto lavoro entra nel lotto e se la costante M si muove. Proposta:
   identificare tutte, sanare solo le 3 oltre il 2410 se bastano uno o due
   `PUNTO_GIRO`/`PUNTO_LARGO`; le 30 note restano.
6. **Preferenza — regione 6253-673F.** Mai eseguita nel giro 0-5499 (oltre 63A4 e
   673F già tradotte): solo annotata in `GIRO.md`, non tradotta. Proposta: così.

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Domanda 5 (scelta di progetto, risolta dalla chat principale con la
proposta dell'analista perché è la via minima e reversibile; da segnalare
all'utente): sì, si identificano tutte e 33 le divergenze a metà attività
e si sanano solo le 3 nuove oltre il 2410, se bastano uno o due punti di
sincronia; altrimenti si documentano e basta.
Preferenze 1-4 e 6, accolte con la proposta:
1. Sì: l'oracolo esteso a 5500 sostituisce quello attuale del boot (ne è
   un prefisso identico, verificato).
2. Sì: i 13 frame campione proposti, riferimento `giroboot_<F>.bin` reso
   con `rendi_frame`; i sei stati dell'oracolo video non si usano.
3. Sì: lo script di regressione pretende il numero esatto di divergenze
   (33), non una soglia.
4. Sì: nomi come proposti (`giro_video.c`, `giro_completo.sh`,
   righe `FRAME_VIDEO` e `GIRO COMPLETO`).
6. Sì: la regione 6253-673F resta annotata in GIRO.md, non tradotta.

## Esito (sviluppatore, 2026-11-03)

Fatto. Le 15 verifiche passate con l'uscita attesa. Deviazione: M = 30 e non 33,
perché le 3 NMI oltre il 2410 (frame 4029, 4508 ritorno 4632; 5459 ritorno 4635)
sono state sanate con `PUNTO_GIRO(0x4632, 0x4635)` in `sub_4632` (nemici.c); la
costante dello script e la riga finale sono 30. Hash in `decomp/GIRO.md`.
