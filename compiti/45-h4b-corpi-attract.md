# Compito 45: H4b1 — NMI nello spedidore e a metà attività, corpi 1F e 06, registro R (sentinella)

Data: 2026-11-03. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Primo taglio del lotto H4b (`compiti/PIANO-HARDWARE.md`, voce H4; contesto in
`compiti/44-h4-giro-libero.md` e `decomp/GIRO.md`). Il giro dal boot
(`harness/gioco_libero`) oggi si ferma a «CORPO 1F (43C8) non tradotto al
frame 602». Qui si fa avanzare fino al corpo 07, e cioè:
1. la macchina gestisce le NMI nella scansione dello spedidore, prima del
   corpo e a metà attività con la stessa semantica di `replay.c` (si copia il
   meccanismo, non si reinventa);
2. si traducono i corpi **1F** (43C8, 13 istruzioni) e **06** (75E7-797E, 91);
3. strumento di registrazione del registro R e sentinella nel C (vedi
   «Il registro R»);
4. l'oracolo del boot passa a FINE=2411 con dump al frame 2410 (serve a H4b3).

Arresto atteso e misurabile: `CORPO 07 (2CCD) non tradotto al frame 1188`,
dopo 16407 spedizioni (1F×1, 06×8203, 14×8203) e 1123 NMI (frame 66-1188),
nessuna divergente nello spedidore. Il perimetro intero di H4b (corpi 07,
08, 0B, 0C, 1F, 06) è ~930 istruzioni eseguite, quasi due lotti col metro del
brief 26 (503): per questo si taglia (misure e proposta in «Domande aperte»).

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/`, `compiti/`, `STATO.md`.
- Non si toccano: `harness/replay.c`, `sched.c`, `sched_proto.c`,
  `video_rendi.c`, `video_sdl.c`, `mostra_replay.c`, `rendi_oracolo.c`.
  Banco (304 voci, 1270 catture, 0 fallite), replay storico e `sched_proto`
  restano verdi bit a bit. Nei sorgenti tradotti e verificati si possono solo
  **aggiungere ganci a puntatore NULL** (precedenti: `NEMICI_PUNTO`,
  `macchina_punto`, `porte_gancio_scrittura`).
- Commenti, messaggi, documenti in italiano. Nessun file di codice sopra le
  800 righe (soglia di lavoro 650 per i moduli nuovi); `z80_2xxx.c` è a 796 e
  non si accresce. `gcc` dal Makefile, nessuna dipendenza nuova, niente
  binari né `.o` nel commit.
- MAME solo senza finestra, ingressi dell'host spenti (lo fanno già
  `giro_boot.sh`/`replay.sh`); a fine lavoro `pgrep -c mame` → `0`.
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`).

## Cosa esiste già (fatti verificati)

| file | righe | ruolo |
|---|---:|---|
| `harness/macchina.c` | 369 | boot 2833-2959; `punto()` (NMI per ritorno+impronta, solo boot); `esegui_nmi(const struct riga_f*)`; `sanita()` a 602; `involucro(slot)`; `macchina_esegui(oracolo, disp, ingr, fine)`. Dopo `boot_2833()` gira `for(;;) spedisci_una(...)` con `involucro` che confronta con le righe D (`sp[]`); non carica le righe N |
| `harness/gioco_libero.c` | 25 | `main`: `macchina_esegui(argv[1..3], fine)` |
| `harness/replay.c` | 462 | NON toccare; da copiare: `nel_rst20`, `pendente`, `nello_spedidore`, `esegui_nmi` (classifica pulita/meta), `prova_punto`, `punto_nemici`, `gancio_scansione`, `involucro` (con `nmi_corpo`, `indice_prima`), il ciclo su `ev[]` di `main` (D/N, `nmi_attesa`, `nmi_scan`) e `carica_registro` (righe D e N in `struct evento`) |
| `harness/attivita.c` | 351 | `corpo_attivita_replay[SCHED_CODICI]` (r. 340-351): tradotti 01 02 03 0A 0D 0E 10 11 14 20 |
| `harness/attivita_01_10.c` | 75 | modello di corpo: `void attivita_01_48d1(uint8_t slot)`, `tabella[slot]=n; return` per `pop hl; ld (hl),n; ret`; prototipi dei corpi in `nemici.h` r. 68-69 |
| `harness/z80_7xxx.c` | 269 | già tradotte: `sub_75aa`, `sub_797e`, `sub_7c84`, `sub_7dea`, `sub_7eca`, `sub_7b95`, `sub_7b9e`, `sub_7e24`, `sub_7379` |
| `harness/nemici_r.c` | 176 | `nemici_r_somma()` / `nemici_r_somma_r1(&r1)`: leggono `r_valori[]` (0 fuori dal banco); chiamati da `z80_4xxx.c` (430A), `nemici_tipo06b.c` (54C5), `nemici_tipo09b.c` (59E5), `nemici_tipo07a.c` (5DDF) |
| `harness/Makefile` | 75 | `SORGENTI_NEMICI = $(wildcard nemici*.c) attivita.c attivita_01_10.c` (entra in banco, replay e gioco_libero); bersaglio `gioco_libero` esistente |
| `giro_boot.lua` / `giro_boot.sh` | — | oracolo del boot: predefiniti FINE=2410, F0=602, dump solo a 602; l'impronta esclude c040-c043, cf00-cfff, dbc0-dbff |
| `out/giroboot_oracolo.txt`, `out/giroboot_disp.txt`, `out/ingressi_replay.txt` | 2344 F, 41396 D, 5434 | già presenti, gitignorati |

Formato dell'oracolo (da `giro_boot.lua`): `F <f> <e600> <e601> <e602> <ritorno> <hash>`;
registro `D <f> <slot> <codice>`, `N <f> <ritorno> <letture>`, `W`, `I`.
Convenzioni di `replay.c` che servono (da `replay.c`): una NMI è «pulita» se il
ritorno sta in 3AF3-3B10 (`nello_spedidore`) o se è eseguita prima del corpo;
«a metà attività» altrimenti; le divergenze pulite fermano il giro, quelle a
metà attività si contano e basta; `pendente(n)` = ritorno in 3AFB-3B10 con
letture 0; `nel_rst20` = 0008-000D, 0010-0014, 0018-001A, 0020-0026.

### Finestra 602-1188 (misurata su `out/giroboot_disp.txt`)

- Spedizioni: `D 602 00 1F`, `D 602 01 06`, poi solo 06 (slot 03, 8203 volte) e
  14 (slot 02, 8203 volte, già tradotto). L'ultima 06 è `D 1187 03 06`: il corpo
  scrive 07 nello slot a 7617 (`W 1187 7619 03 07`). Prima spedizione di 07:
  `D 1188 03 07` (riga 18261 del file), preceduta da `N 1188 75D3 0`. Il corpo
  1F viene spedito una sola volta in questa finestra (la seconda al frame 1562).
- Righe N dal 603 al 1187: 585 (350 con letture > 0); 378 hanno il ritorno in
  3AF3-3B10 (3AFA×84, 3B00×96, 3AFD×70, 3AFB×59, 3AFE×43, altri), 207 altrove.
  Ritorni a metà attività: rst 20 e helper 0008-0026 (circa 110 eventi), corpo 14
  tradotto (44B5-4517), corpo 06 (75B4, 75E7×2, 75EA, 75EB×3, 763E, 7641, 7643,
  7659×2, 765A), routine tradotte chiamate da 06 (3291, 329A, 32A8, 32FC, 337A,
  3396), 7F03, 7F10.
- Totale NMI dei frame 66-1188: 1123 (righe F) = 537 del boot + 586.

### Corpo 1F (43C8)

`out/main64.dasm` allinea male 43C8-43D4 (mezza istruzione prima): le 13
istruzioni eseguite sono in `out/titolo_istruzioni.txt` (`awk '$1>="43C8" &&
$1<="43FF"' out/titolo_istruzioni.txt`): `xor a; ld (c09b),a; ld (c0b5),a; ld
bc,0018; ld de,c09d; ld hl,43e5; ldir; ld c,06; ld de,c0b7; ldir; pop hl; ld
(hl),14; ret`. I dati sono ROM a 43E5-43FC (24 byte) e 43FD-4402 (6 byte). Il
secondo `ldir` parte da HL = 43FD. Slot: `tabella[slot] = 0x14`.

### Corpo 06 (75E7)

Disassemblato: `awk '/;75e7[ \t]*$/,/;797e[ \t]*$/' out/main64.dasm`. Istruzioni
eseguite nella finestra (da `out/titolo_istruzioni.txt` e `out/stanza_00_istruzioni.txt`):
75E7-75EB, 760B-765A, 78E0-7931, 7961-797D (91). Struttura: 75E7 `hl=c1ae;
a=(hl); rst 20` con tabella a 75EC (10 voci: 78E0 763E 760B 760B 760B 7A0B 763E
760B 7B3B 763E; sono eseguite solo 78E0, 763E, 760B; 7A0B e 7B3B no, e il
`rst 20` non ritorna: il corpo termina nel gestore). 760B: `rst 10` a 7634 (`sub_0010`),
slot ← 07 a 7617, chiamate a 7C84, 75AA, 33EB, scrittura di c25d, c243, c1b7.
763E: attesa c467 ≥ 40, poi 2551. 78E0: (hl)++ , 75AA, 673F, `posta_attivita(14)`
(0493), 322A, 33BA, 3F88, 7ECA, 6771×5, 797E, `posta_attivita(06)`. Tutte le
routine chiamate sono già tradotte (`routine.h`). La tabella è in
`rst20_tables.h` solo per l'elenco 3B10; per 75EC si legge la ROM
(`od -A n -t x2 -j $((0x75ec)) -N 20 out/main64.bin`). Le voci 7A0B e 7B3B non
si traducono: `switch` con `default` che stampa un avviso e termina il giro (codice 1).

### Il registro R (misurato)

Registrazione MAME con breakpoint sul successivo di ogni `ld a,r` (2D1F, 2D22,
430A, 430D, 54C5, 54C8, 59E5, 59E8, 5DDF, 5DE2): **0 letture nei frame 0-2410 e
0 nei frame 0-5499** con gli ingressi registrati. Controllo positivo: lo stesso
script sul sito 3AF9 produce `R 602 3AF9 1F` come prima riga. L'oracolo F è
identico (byte per byte) a `out/giroboot_oracolo.txt`. Quindi per la partita
registrata nessun valore di R va fornito al giro; serve una sentinella che
fermi il giro se un percorso diverso (altri ingressi, H5) legge R.

## Struttura da realizzare

1. **`decomp/registra_r.lua` + `decomp/registra_r.sh`** (nuovi). `registra_r.lua` =
   copia di `giro_boot.lua` con tre modifiche (provate fuori dal repo):
   - subito dopo `local frames = 0`: `reg_r = io.open("out/" .. nome .. "_r.txt", "w")` (globale);
   - subito dopo `dbg:command("bpset 66")`:
   ```lua
   local siti_r = {}
   for x in string.gmatch(os.getenv("REGR_SITI") or "2D1F 2D22 430A 430D 54C5 54C8 59E5 59E8 5DDF 5DE2", "%x+") do
     local pc = tonumber(x, 16)
     siti_r[pc + 2] = pc          -- `ld a,r` è ED 5F: A vale R all'istruzione dopo
     dbg:command(string.format("bpset %X", pc + 2))
   end
   ```
   - nel `register_periodic`, al posto della riga `if cpu.state["PC"].value ~= 0x0066 then dbg:command("go") return end`:
   ```lua
   local pc = cpu.state["PC"].value
   if siti_r[pc] then
     reg_r:write(string.format("R %d %04X %02X\n", frames, siti_r[pc], cpu.state["A"].value))
     dbg:command("go") return
   end
   if pc ~= 0x0066 then dbg:command("go") return end
   ```
   - in `chiudi()` dopo `f_oracolo:close()`: `reg_r:close()`.
   `registra_r.sh` = copia di `giro_boot.sh` con `REPLAY_NOME=${REPLAY_NOME:-regr}`,
   `registra_r.lua` come script, FINE predefinito 2411, e come messaggio finale
   `R: <n> letture in out/regr_r.txt` (`wc -l < out/regr_r.txt`). Formato riga:
   `R <frame> <PC del ld a,r> <A>`. Intestazione dei due file in italiano, con la
   spiegazione sopra.
2. **`giro_boot.sh`**: predefiniti `REPLAY_FINE=2411`, `REPLAY_DUMPS=2410`
   (F0 resta 602); aggiornare i commenti di `giro_boot.sh`/`giro_boot.lua`.
3. **`harness/nemici_r.c` + `nemici.h`**: gancio `void (*nemici_r_gancio)(void) = NULL;`
   (dichiarato `extern` in `nemici.h` accanto agli altri `banco_r_*`), chiamato
   come prima istruzione di `nemici_r_somma_r1` se non NULL. Banco e replay
   invariati (gancio spento).
4. **`harness/attivita_attract.c`** (nuovo, ~120 righe): `void attivita_1f_43c8(uint8_t slot)`
   e `void attivita_06_75e7(uint8_t slot)`, prototipi in `nemici.h` (r. 68-69 come
   modello). Stile di `attivita_01_10.c` (commento di testa con le regole
   `pop hl`, un solo `struct z80 z = {0}` per corpo, indirizzi originali a
   margine). In 06 si inseriscono **punti di sincronia** `NEMICI_PUNTO(da, a)`
   (`nemici.h` r. 57-62) dove cadono i ritorni a metà attività elencati sopra:
   regola di `replay.c`/`nemici.h`: l'intervallo [da, a] contiene gli indirizzi
   delle istruzioni che, se la NMI entra prima di eseguirle, vedono la stessa RAM
   (dalla prima istruzione dopo l'ultima scrittura fino all'istruzione che scrive
   inclusa). Minimo: 75E7-75EB (ingresso, prima del `rst 20`), 0020-0026 (gestore
   `rst 20`), 0010-0014 (`rst 10` di 7634), intervalli di 760B-7639 e 763E-765A
   separati dalle scritture di c045, c467, c25d, c243, c1b7. Se una NMI pulita
   diverge, guardare il suo ritorno nella diagnostica e aggiungere il punto
   mancante; i ritorni dentro routine già tradotte e senza punti (32xx-33xx,
   75B4/75D3 in `sub_75aa`, 7F03/7F10) restano a metà attività e possono
   divergere senza fermare il giro (come nel replay).
5. **`harness/attivita.c`**: nella tabella aggiungere `[0x06] = attivita_06_75e7,
   [0x1f] = attivita_1f_43c8,` e aggiornare il commento di testa. **`Makefile`**:
   `SORGENTI_NEMICI = ... attivita.c attivita_01_10.c attivita_attract.c`.
6. **`harness/macchina.c`**: dopo `boot_2833()` il ciclo diventa quello di
   `replay.c`.
   - `carica_registro` legge anche le righe N (`struct evento` come in `replay.c`;
     `sp[]` e il vecchio `involucro` a righe D spariscono).
   - Fine del boot: `macchina_punto = NULL;` (i punti del boot restano nelle
     routine condivise ma non devono più iniettare NMI), `nemici_punto = punto_nemici;`,
     `sched_gancio_scansione = gancio_scansione;`, `corpo_attivita[i] = involucro`
     (versione di `replay.c`, con `corpo_mancante` e il messaggio `CORPO %02X (%04X)
     non tradotto al frame %ld` stampato con l'indirizzo di `rst20_3B10`).
   - Punto di partenza: `i0` = indice dell'evento N con frame uguale a quello
     dell'ultima NMI eseguita dal boot (`ff[k_ff-1].frame`, 602) più uno; ci
     deve stare `D 602 00 1F`, altrimenti messaggio di errore e `ferma(2)`.
     Fine: dopo l'ultima N con frame ≤ `frame_fine` (predefinito: ultimo F).
   - `esegui_nmi` mantiene: controllo `maschera_nmi`, deposito di e600-e602 dal file
     ingressi, `sanita()` a 602, `in_nmi`; aggiunge la classificazione
     pulita/metà e i contatori di `replay.c`; una divergenza pulita stampa il
     messaggio `DIVERGENZA frame ...` come `replay.c` e ferma con codice 1.
   - `nemici_r_gancio = r_letto;` con `r_letto()` che stampa `R letto prima della
     NMI del frame %ld: oracolo del registro R assente (registra_r.sh)` e `ferma(1)`.
   - Riepilogo (stampato anche all'arresto per corpo mancante), prima riga come
     in H4a, poi due nuove:
     ```
     spedizioni eseguite: <n>
     NMI confrontate: <N>, divergenti: <M>
     dopo il boot: nello spedidore <P> (divergenti <X>), a metà attività <Q> (divergenti <Y>)
     ```
   - Se `macchina.c` supera 650 righe, spostare il ciclo a eventi in
     `harness/macchina_sped.c` esportando le poche variabili condivise (a scelta
     dello sviluppatore; Makefile: aggiungere il file a `SORGENTI_LIBERO`).
7. **Documenti**: `decomp/GIRO.md` (sezione H4b1: ciclo a eventi, punti in 06,
   sentinella R, uso di `registra_r.sh`, limiti); `STATO.md` (una voce H4b1);
   `compiti/PIANO-HARDWARE.md` (H4b spezzato in H4b1 fatto / H4b2 / H4b3, vedi
   «Domande aperte»); `ROUTINE.md` non cambia (nessuna routine nuova nel banco).
   **Catture di banco: nessuna** in questo lotto, i corpi 1F e 06 non sono
   chiamabili con registri e le routine che usano sono già nel banco; la
   verifica è l'avanzamento del giro.

## Verifiche obbligatorie

Da `decomp/` salvo diversa indicazione. (m) = misurato prima di scrivere il
brief; (a) = attesa, non misurata, da riportare esatta.

1. `cd harness && make -B all gioco_libero 2>&1 | grep -c -i -E 'warning|error'` → `0`.
2. `sh giro_boot.sh` (circa 45 s) → `REPLAY giroboot: 2345 righe F, 41421 spedizioni` (m, stessa MAME con FINE=2411 e dump 2410).
3. `cmp out/giroboot_2410.bin out/replay_2410.bin && echo identici` → `identici` (m: il dump al 2410 coincide byte per byte col dump storico).
4. `diff <(awk '{print $1,$2,$3,$4,$5,$6}' out/replay_oracolo.txt | head -2345) <(awk '{print $1,$2,$3,$4,$5,$6}' out/giroboot_oracolo.txt)` → vuoto (m).
5. `sh estrai_ingressi.sh && sha1sum out/ingressi_replay.txt` → `ESTRAI_INGRESSI: 5434 righe in out/ingressi_replay.txt` e `43934a8eeed8d44a071d7ab958d32ea74fbd437f` (m, invariato).
6. R: `sh registra_r.sh` → `R: 0 letture in out/regr_r.txt` (m con FINE=2411; anche con `REPLAY_FINE=5500`, circa 90 s, → `R: 0 letture`). Controllo positivo: `REGR_SITI=3AF9 REPLAY_FINE=700 sh registra_r.sh >/dev/null; head -1 out/regr_r.txt` → `R 602 3AF9 1F` (m).
7. Giro: `harness/gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt out/ingressi_replay.txt; echo "exit $?"` →
   - `SANITA 602: ... coincide` (come H4a);
   - `CORPO 07 (2CCD) non tradotto al frame 1188` (m: prima D di 07 nel registro);
   - `spedizioni eseguite: 16407` (m: D prima della riga 18261);
   - `NMI confrontate: 1123, divergenti: <M>` (1123 m; M (a): somma delle divergenze a metà attività, accettabile);
   - `dopo il boot: nello spedidore <P> (divergenti 0), ...` con P+Q = 586 (m) e **divergenti nello spedidore = 0** (a, condizione dura);
   - `exit 1`. Se una NMI pulita diverge, NON si allenta l'impronta: si aggiunge il punto mancante (struttura 4) e si riporta nel rapporto la diagnostica.
8. Sentinella: `grep -c nemici_r_gancio harness/nemici_r.c harness/macchina.c` → ogni file ≥ 1; in gioco_libero nessuna riga `R letto` (verifica 7) perché la finestra non legge R (m).
9. `pgrep -c mame` → `0`.
10. Banco: `cd harness && ./harness --tutte > /tmp/h45.txt; echo "exit $?"; grep -c FALLITO /tmp/h45.txt; grep -c '^RIEPILOGO' /tmp/h45.txt; grep -c '^cattura' /tmp/h45.txt` → `exit 0`, `0`, `304`, `1270` (m, brief 44).
11. Replay storico: `harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r45.txt 2>&1; echo "exit $?"; tail -4 /tmp/r45.txt` → `exit 0` e
    `spedizioni eseguite: 64686, fino al frame 5499` /
    `NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885` /
    `prima divergenza a metà attività (attesa): frame 2415` /
    `nessuna divergenza nello spedidore` (m, brief 44).
12. `harness/sched_proto out/sched_log.txt | tail -1` → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (m, brief 44).
13. `sh rendi_video.sh; echo "exit $?"` → `RENDI 7/7 identici`, `exit 0` (m, brief 44).
14. `git diff --stat -- decomp/harness/replay.c decomp/harness/sched.c decomp/harness/sched_proto.c decomp/harness/video_rendi.c decomp/harness/video_sdl.c decomp/harness/mostra_replay.c decomp/harness/rendi_oracolo.c` → vuoto.
15. `wc -l harness/*.c | awk '$1>800 && $2!="totale"'` → vuoto; `wc -l harness/macchina.c harness/attivita_attract.c` → entrambi sotto 650 (o `macchina_sped.c` introdotto).
16. Ambito: `git status --short` dalla radice → solo: `decomp/registra_r.lua`, `registra_r.sh`, `giro_boot.sh` (e `giro_boot.lua` se serve per i commenti), `harness/attivita_attract.c`, `attivita.c`, `nemici.h`, `nemici_r.c`, `macchina.c` (e `macchina_sped.c`), `Makefile`, `GIRO.md`, `STATO.md`, `compiti/PIANO-HARDWARE.md`, `compiti/45-…`. `git status --short | grep -c -E 'gioco_libero$|\.o$|\.bin$'` → `0`.

## Consegna

Commit unico «Lotto H4b1: NMI nello spedidore e a metà attività, corpi 1F e 06,
sentinella R» con riga finale `Co-Authored-By: <modello vero> <noreply@anthropic.com>`.
Niente push. In fondo a questo brief: esito e uscita delle verifiche. Rapporto
finale: massimo trenta righe con l'uscita delle 16 verifiche, i punti
`NEMICI_PUNTO` inseriti in 06 con i ritorni che coprono, il numero di NMI pulite
e a metà attività e le deviazioni.

## Domande aperte

1. **Scelta di progetto — spezzatura di H4b.** Perimetro misurato (istruzioni
   eseguite distinte, unione di `out/titolo_istruzioni.txt` e
   `out/stanza_00_istruzioni.txt` con visita statica dal disassemblato, tetto
   superiore): 1F 13; 06 90; 0B (2C27) 44; 08 (7C98) 46; 0C (7BF6) 44; **07
   (2CCD) ~690**, macchina a fasi con tre `rst 20` (2CD2: 26 voci, 2DB8: 6, 2DD9:
   7 = 39 destinazioni) i cui gestori stanno in 2C27-2F56 e 3009-3AF2, più
   routine non tradotte (2D8A, 3110, 37B4, 38C3). Totale ~930. Proposta: **H4b1 =
   questo brief** (arresto a 07, frame 1188); **H4b2 = corpo 07** (~690, arresto
   a 08 o al primo corpo mancante); **H4b3 = 08, 0C, 0B** (~134) più il confronto
   byte per byte al frame 2410 contro `out/giroboot_2410.bin` (= `replay_2410.bin`,
   verifica 3) con arresto con esito 0 alla NMI 2410. Se l'utente preferisce due
   lotti: H4b1 come qui, H4b2 = 07+08+0C+0B (~830, oltre il metro di 503).
   Per H4b2/H4b3 le catture di banco sono da provare (non provate): 2D8A è
   chiamata da 0B a 2C34, `sh cattura.sh 0x2D8A att0B` (stato `att0B` = posta di
   0B al 2300).
2. **Scelta di progetto — oracolo R.** L'incarico prevedeva un file di valori di
   R dato al giro; la registrazione (script provato, controllo positivo ok) dà 0
   letture fino al frame 5499, quindi il file sarebbe vuoto. Proposta: in questo
   lotto solo lo strumento di registrazione e la sentinella; il meccanismo con
   file (`nemici_r_somma` che consuma `R frame pc valore`) si costruisce quando
   una registrazione mostra letture (partita libera con H5, o 2D1F nel corpo 07
   se un altro percorso lo raggiunge).
3. **Scelta di progetto — regione 6253-673F fuori da H4b.** Nessuna istruzione di
   quella regione viene eseguita nei frame 0-2410 (unione delle sessioni
   titolo + stanza_00: solo 63A4, corpo 0E, già tradotto). Appartiene al giro
   oltre il 2410 (H4c). Proposta: toglierla dal perimetro di H4b.
4. **Preferenza — ciclo a eventi dentro `macchina.c`** con eventuale `macchina_sped.c`
   oltre 650 righe; nomi `attivita_attract.c`, `registra_r.lua/.sh`,
   `nemici_r_gancio`, `out/regr_r.txt`. Proposta: così.
5. **Preferenza — oracolo del boot a FINE=2411 con dump al 2410** come predefiniti
   di `giro_boot.sh` (necessario a H4b3: la riga F del 2410 e il dump). Proposta: così.
6. **Preferenza — il confronto al 2410 esclude solo c040-c043 e cf00-cfff**
   (non dbc0-dbff, a differenza di `sanita()` a 602), così nel lavoro di H4b3
   la zona dbc0-dbff, esclusa dall'impronta, viene verificata una volta. Se lì
   differiscono solo quei byte, si riporta come deviazione. Proposta: così.

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Domande 1-3 sono scelte di progetto, risolte dalla chat principale sotto il
mandato della spezzatura H4a/b/c già ratificata (brief 44, risoluzione 1),
tutte fondate su misure dell'analista; da segnalare all'utente:
1. Sì, taglio in tre: H4b1 = questo brief (NMI complete nella macchina,
   corpi 1F e 06, strumento e sentinella per R); H4b2 = corpo 07 (~690
   istruzioni); H4b3 = corpi 08, 0C e 0B (~134) più il confronto byte per
   byte al frame 2410. L'alternativa a due lotti sforerebbe il metro
   (830 > 503).
2. Sì, oracolo R ridotto a strumento di registrazione più sentinella: la
   misura (con controllo positivo su 3AF9) mostra 0 letture di R nella
   partita registrata fino al frame 5499; il meccanismo a file si
   costruisce solo se una registrazione futura mostra letture.
3. Sì, la regione 6253-673F passa a H4c: mai eseguita prima del 2410.
Domande 4-6, preferenze accolte con la proposta:
4. Sì: ciclo a eventi in `macchina.c`, spezzatura in `macchina_sped.c`
   oltre le 650 righe, nomi come proposti.
5. Sì: `giro_boot.sh` con FINE=2411 e dump al 2410 predefiniti (misurato
   identico a `out/replay_2410.bin`).
6. Sì: confronto al 2410 con esclusione delle sole c040-c043 e cf00-cfff.

## Esito (sviluppatore, 2026-11-03)

Tutte le 16 verifiche passate. Giro: `CORPO 07 (2CCD) non tradotto al frame 1188`,
16407 spedizioni, 1123 NMI (466 nello spedidore, 120 a metà attività), divergenti
0 nello spedidore e 9 a metà attività (primo corpo 06, frame 603-611). Registro R:
0 letture, controllo positivo `R 602 3AF9 1F`. Banco 304/1270, replay e
sched_proto invariati, `RENDI 7/7`. Deviazioni: vedi rapporto (coda di N dopo la D,
`PUNTO_LARGO`, copia di F000-FFFF dalla ROM, `sub_0493` al posto di
`posta_attivita` per il banco).
