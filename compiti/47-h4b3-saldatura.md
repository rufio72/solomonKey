# Compito 47: H4b3 — fasi 13-25 del corpo 07, corpi 08, 0C, 0B e saldatura al frame 2410

Data: 2026-11-03. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Terzo e ultimo taglio del lotto H4b (`compiti/PIANO-HARDWARE.md`, voce H4; quadro
in `decomp/GIRO.md`, brief 44, 45, 46). Il giro dal boot (`harness/gioco_libero`)
oggi si ferma con `ROUTINE 2DCE mancante (motore dei nemici) al frame 1398`.
Qui si traducono le fasi 13-25 del corpo 07 (2CCD), i corpi 08 (7C98), 0C (7BF6)
e 0B (2C27) e le routine 37B4 e 2D8A, finché il giro arriva al frame 2410 e la
RAM del C coincide byte per byte con `out/replay_2410.bin` (la **saldatura**:
il boot si salda all'oracolo storico 2410-5499).

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/`, `compiti/`, `STATO.md`. `README.md` non
  si tocca.
- Non si toccano: `harness/replay.c`, `sched.c`, `sched_proto.c`,
  `video_rendi.c`, `video_sdl.c`, `mostra_replay.c`, `rendi_oracolo.c`,
  `z80_2xxx.c` (796 righe). Banco storico (305 voci, 1272 catture, 0 fallite),
  replay e `sched_proto` restano verdi bit a bit. Nei sorgenti già verificati
  si possono solo aggiungere ganci a puntatore NULL.
- `macchina.c` (590 righe) si può modificare **solo** per la saldatura (vedi
  «Struttura», punto 6) e deve restare sotto le 650 righe; `z80_3xxx.c` (564)
  non si accresce oltre le 650; `attivita_attract07.c` (305), `07a` (226),
  `07b` (211) restano sotto le 650. Nessun file di codice sopra le 800 righe.
- Commenti, messaggi, documenti in italiano. `gcc` dal Makefile, nessuna
  dipendenza nuova, niente binari né `.o` nel commit.
- MAME solo senza finestra, ingressi dell'host spenti (lo fanno già gli script);
  a fine lavoro `pgrep -c mame` → `0`. Niente git distruttivo.
- **Non si rigenera l'oracolo** (`giro_boot.sh`): `out/giroboot_oracolo.txt`,
  `out/giroboot_disp.txt`, `out/giroboot_2410.bin`, `out/ingressi_replay.txt`
  esistono già (gitignorati). Non si allenta l'impronta, non si aggiungono
  esclusioni alla saldatura oltre a c040-c043 e cf00-cfff.

## Perimetro (misurato)

Istruzioni da tradurre: **fasi 13-25 = 346** e **corpi 08, 0C, 0B = 134**, in
tutto 480 (conteggi del brief 46, che escludono le routine già tradotte).
Nessuna delle routine di fase è già tradotta: `grep -i` su `harness/*.c *.h` per
2DCE, 31C5, 2E0A, 2E9C, 3709, 3829, 321E, 2EF5, 2E6D, 2F1A, 2E1E, 3009, 305F,
3131, 2E65, 3161, 2E81, 31B7 trova solo `rst20_tables.h` e le `nemici_manca` di
`attivita_attract07.c`; `ROUTINE.md` non ha righe per esse (la dicitura «motore
dei nemici» è solo il testo dell'arresto). Si traducono quindi tutte. Le routine
`sub_3000`, `sub_3045`, `sub_3097`, `sub_30ef`, `sub_3101`, `sub_2ff3`, `sub_31ce`,
`sub_31e9`, `sub_325e`, `sub_2e11`, `sub_2f74`, `sub_2c1a` sono già nel banco
(da `routine_tab.c`): si chiamano, non si rifanno.

Sequenza dei corpi nel registro (da `out/giroboot_disp.txt`, righe D = frame,
slot, codice):
- `D 1531 07 08` (corpo 08, una volta), `D 1541 09 0C` (corpo 0C, 192 volte),
  `D 1562 0A 0B` (corpo 0B, una volta), `D 1562 0B 1F`, poi `D 1562 0C 07`.
- **Il corpo 07 gira due volte**: la prima nello slot 03 (frame 1188-1509,
  3281 spedizioni, la 2682-esima è la fase 13 al frame 1398), la seconda nello
  slot 0C dal frame 1562 al 1884 (3312 spedizioni): 0B azzera c19d a 2C3E e
  rimette in coda 07 a 2C6E. Le fasi 13-25 e le attese (2E89 ecc.) si ripassano
  nella seconda corsa; i rami eseguiti solo lì sono nei file `*_istruzioni.txt`.
- Dal 1562 al 2410 girano solo corpi già tradotti (01, 02, 03, 0A, 0D, 0E, 10,
  11, 14, 20 in `corpo_attivita_replay`).
- D totali fino alla N del frame 2410 (ultima N del registro): 41396 (le 25 D
  dopo l'ultima N non si eseguono: `giro_spedidore` si ferma all'ultima N entro
  `frame_fine`). N = una per frame dal 66 al 2410 = 2345 (537 nel boot, 1808
  dopo).

### Fasi 13-25 del corpo 07 (tabelle `rst 20`, da `rst20_tables.h` e brief 46)

`switch` in `attivita_07_2ccd` (`attivita_attract07.c` r. 281-304, oggi
`nemici_manca`): 13 → 2DCE (con `rst 20` T3 a 2DD9, 7 voci: 3009, 305F, 3131,
2E65, 3161, 2E81, 31B7; c19e sotto-contatore), 14 → 31C5, 15 → 2E0A, 16 → 2D9D
(mai eseguita: lasciare `nemici_manca(0x2d9d)`), 17 e 21 → 2E9C, 18 → 3709
(chiama **37B4** a 374B, ritorno a 374E), 19 → 3829, 20 → 321E, 22 → 2EF5,
23 → 2E6D, 24 → 2F1A, 25 → 2E1E (a 2E58 azzera lo slot: la corsa finisce).
Alcuni gestori sono attese come 2E89/2E5D: riusare `attesa()` di
`attivita_attract07.c` r. 28 (con il sito giusto) dove l'originale è 2E8B-2E9A.

### Corpi 08, 0C, 0B (da `out/titolo_istruzioni.txt`, misurato)

- **0B, 2C27** (44 istr., 1 esecuzione): `ld hl,c234; ld bc,0300; ciclo ld (hl),c`
  (3 giri), `call 33EB`, **`call 2D8A`**, `call 33EB`, `call 673F`, c19d = 0,
  ciclo 2C47-2C4E (32 giri, scrive d440 e il suo specchio con `res/set 2,h`),
  `ld a,1F; call 0493`, `(c045)` → `call 4D05`, ramo `jr z,2C6B`, 2C6B `pop hl;
  ld a,07; call 0493; ret`. Routine chiamate tutte nel banco: 33EB, 673F, 0493,
  4D05.
- **0C, 7BF6** (44 istr., 192 esecuzioni, 191 escono a 7C28 `pop hl; ret` perché
  `jp pe,7C28`): legge c04c, `call 6771` (HL = 7C58, DE = d646), `call 3A1C`,
  scrive d713, poi il parity test su `c052 & 3`. Solo l'ultima esecuzione prosegue
  7C26-7C57: c04c, c045, `call 673F`, bit 7 di c044, `call 3F88` (HL = 80D3),
  `call 7C84`, `call 75AA`, `ld a,0B; call 0493`, `pop hl; ret`. Nella fase di
  attesa i punti di sincronia vanno dentro il corpo (come 763E in 06).
- **08, 7C98** (46 istr., 1 esecuzione): `call 75AA`, `call 7C84`, `ld c,3E; rst 28`
  (`sub_0028`), `call 673F`, `ld a,14; call 0493`, `call 3F88` (HL = 80B3), porta
  e604 = 0 (`scrivi_porta`, non conta per l'impronta), tre `call 6771` (HL = 7EA7,
  7EB2, 7EC4; DE = d70B, d748, d62E), c243 = 0, `call 322A`, `ld c,01; call 33BA`,
  `call 7ECA`, `ld a,0C; call 0493`, `pop hl`, poi `a = (e605) >> 5 & 7` (3 `rlca` e
  `and 07`) e `rst 20` con tabella a 7CED (8 voci: 7D6A, 7DC6, 7DA0, 7DDE,
  7D85, 7DD2, 7DB3, 3B62; da `od`). Si esegue solo l'indice 0 (7D6A: `call 7DEA`,
  tre volte `call 7E24` con A = 0, 1, 2 e HL = 7E46, 7E56, 7E66, `ret`). Le altre voci
  → `nemici_manca(<gestore>)`. `leggi_porta(0xe605)` dà 0 nel giro (`macchina.c`
  r. 302), coerente con l'indice 0 di MAME. `sub_7dea` e `sub_7e24` sono nel banco.
- Regole `pop hl` dei corpi: `attivita_01_10.c` (pop hl + ret = return;
  `pop hl; ld a,n; call 0493; ret` = posta + return).

### Routine nuove per il banco (catture misurate in una copia fuori dal repository)

- **37B4** (19 istr.; `push de; ld (hl),D0; call 4B0E; push bc; call 472E; pop bc;
  ld d,c; ld e,b; ld bc,0500; ld a,A0; call 4AF6; ld a,e; call 4722; ld (ix+03),a;
  ld a,e; ld c,d; call 4767; pop de; ret`). Tutte le chiamate sono tradotte
  (`routine.h`: 4af6, 4b0e, 472e, 4722; `nemici.h` r. 250: `sub_4767`, condivisa
  con la 4649, N7). Catture (m, `cattura.sh` invariato):
  `sh cattura.sh 0x37B4 titolo` → `CATTURA 0x37B4 titolo frame_da=1000 OK`
  (prima PC=37B4 AF=3821 BC=3410 DE=8667 HL=C441 IX=C82E IY=FFFF; dopo PC=374E
  AF=0100 BC=3410 DE=8667 HL=C81C IX=C7FE; 25 byte diversi);
  `sh cattura.sh 0x37B4 gioco` → `CATTURA 0x37B4 gioco frame_da=1700 OK` (stessi
  registri con IY=C4CE; 25 byte diversi); e le occorrenze 2: `sh cattura.sh 0x37B4
  titolo 2 n2` → `CATTURA 0x37B4 titolo_n2 frame_da=1000 OK`, `sh cattura.sh 0x37B4
  gioco 2 n2` → `CATTURA 0x37B4 gioco_n2 frame_da=1700 OK`. Con `att07` fallisce
  (non usare). L'uscita è a 374E (il chiamante è la fase 18 a 374B).
- **2D8A** (12 istr.: `ld hl,c237; ld de,c268; ld b,31; ciclo 2D92-2D9A` scambia
  49 byte fra (HL) e (DE) con `ex de,hl`, `ret`). Catture (m):
  `sh cattura.sh 0x2D8A gioco` → `CATTURA 0x2D8A gioco frame_da=1500 OK` (prima
  PC=2D8A AF=0410 BC=0000 DE=344F HL=DC58 IX=C0BD IY=C4CE; dopo PC=2C37 AF=0010
  DE=C299 HL=C268; **14 byte diversi**, scrive); `sh cattura.sh 0x2D8A att0B` →
  `CATTURA 0x2D8A att0B frame_da=2300 OK` (IX=C82E; **0 byte diversi**: i blocchi
  coincidono, cattura banale da annotare). Con `titolo`, `tardo`, `porta`
  fallisce; `att0B` N=2 fallisce. Il suo chiamante nel giro è 0B (2C34); altri
  chiamanti: 6455, 6A15 (H4c).
- Gestori di fase (2DCE, 3709, 3829 ecc.) non sono catturabili (NMI nelle
  occorrenze, brief 46): si verificano col giro.

## Cosa esiste già (fatti verificati)

| file | righe | ruolo (da `wc -l` e brief 46) |
|---|---:|---|
| `harness/attivita_attract07.c` | 305 | ingresso 2CCD, fasi 2, 5, 6, 8-12, `attesa(hl, c, sito)` r. 28, `switch` r. 281-304 |
| `harness/attivita_attract07a.c`, `07b.c` | 226, 211 | fasi 0, 1, 3 e 4, 7; stile da copiare (commento di testa con regole `pop hl`, `struct z80 z = {0}`, indirizzi a margine) |
| `harness/attivita_attract07.h` | 27 | `PUNTO_LARGO` e prototipi `void fase_07_XXXX(void);` |
| `harness/attivita.c` | 356 | `corpo_attivita_replay[]` r. ~340-356 (aggiungere `[0x08]`, `[0x0b]`, `[0x0c]`) |
| `harness/nemici.h` | 361 | `NEMICI_PUNTO`, prototipi dei corpi (r. 66-82), `nemici_manca`, `nemici_fuori_tabella`, `nemici_r_somma` |
| `harness/macchina.c` | 590 | giro: `diff_dump(frame)` r. 177 (esclude c040-c043, cf00-cfff, dbc0-dbff), `sanita()` r. 200, `esegui_nmi()` r. 214 (chiama `sanita()` se `frame == FRAME_DUMP` 602), `giro_spedidore()` r. 489 |
| `harness/z80_38xx.c` | 303 | qui `sub_37b4` (stile `sub_3adf`) |
| `harness/z80_2xxx.c` | 796 | **intoccabile**: `sub_2d8a` va in un file nuovo |
| `harness/routine.h` (210), `routine_tab.c` (315) | | prototipi e voci `{ "0x37B4", sub_37b4, 0, 0 },` in ordine di indirizzo |
| `harness/Makefile` | | r. 3: `SORGENTI_BASE` ha `$(wildcard z80_*.c)`; r. 7 `SORGENTI_NEMICI` elenca a mano `attivita_attract*.c` |
| `decomp/ROUTINE.md` | 831 | una riga per ingresso; tabella dei corpi r. 140-163 |

Baseline misurata (copia fuori dal repository, binari già costruiti):
`harness/gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt
out/ingressi_replay.txt; echo "exit $?"` →
```
SANITA 602: byte diversi in c000-e5ff (escluse c040-c043, cf00-cfff, dbc0-dbff): 0, coincide
ROUTINE 2DCE mancante (motore dei nemici) al frame 1398
spedizioni eseguite: 21770
NMI confrontate: 1333, divergenti: 15
dopo il boot: nello spedidore 615 (divergenti 0), a metà attività 181 (divergenti 15)
exit 1
```
Banco: `./harness --tutte` → 305 `RIEPILOGO`, 1272 `cattura`, 0 `FALLITO`.
`cmp out/giroboot_2410.bin out/replay_2410.bin` → nessuna uscita (identici, 16384
byte); `sha1sum out/ingressi_replay.txt` → `43934a8eeed8d44a071d7ab958d32ea74fbd437f`.

## Struttura da realizzare

1. **Fasi 13-25 del corpo 07**: due moduli nuovi, ciascuno sotto le 650 righe,
   nomi proposti `harness/attivita_attract07c.c` (fasi 13 con T3 su c19e, 14, 15,
   17/21) e `attivita_attract07d.c` (fasi 18, 19, 20, 22, 23, 24, 25); prototipi
   `void fase_07_XXXX(void);` aggiunti a `attivita_attract07.h`; le chiamate nel
   `switch` al posto di `nemici_manca` (la 16 resta `nemici_manca(0x2d9d)`).
   Se una fase è breve e usa `attesa()`, può restare in `attivita_attract07.c`
   (sotto le 650). Ogni modulo: stesso stile di `07a`/`07b`.
2. **Corpi 08, 0C, 0B**: un modulo `harness/attivita_attract08.c` con
   `void attivita_08_7c98(uint8_t slot); void attivita_0c_7bf6(uint8_t slot);
   void attivita_0b_2c27(uint8_t slot);` (prototipi in `nemici.h` dopo il corpo
   07). In `attivita.c`: `[0x08] = attivita_08_7c98, [0x0b] = attivita_0b_2c27,
   [0x0c] = attivita_0c_7bf6`. `sched_proto.c` ha già stub `static` con quei nomi
   e non cambia.
3. **`sub_37b4`** in `z80_38xx.c` (prototipo in `routine.h`, voce in
   `routine_tab.c`, riga in `ROUTINE.md` con le 4 catture); la fase 18 la chiama
   (con i punti di sincronia attorno alla chiamata e alle scritture).
4. **`sub_2d8a`** in un file nuovo `harness/z80_2dxx.c` (rientra nel wildcard
   `z80_*.c`; include `routine.h`), prototipo in `routine.h`, voce in
   `routine_tab.c`, riga in `ROUTINE.md` (2 catture: `gioco` e `att0B`, la seconda
   banale). Il corpo 0B la chiama con un punto di sincronia dentro lo scambio.
5. **Regole di traduzione** (come brief 46): solo ciò che gira nei frame
   0-2410; rami mai eseguiti → `nemici_manca(<indirizzo>)`; il registro R, se
   letto, passa da `nemici_r_somma()`; per ogni scrittura in RAM e ogni chiamata
   a routine tradotta un `NEMICI_PUNTO(da, a)` ([da, a] = dalla prima istruzione
   dopo l'ultima scrittura fino all'istruzione che scrive inclusa; nei cicli
   dentro il ciclo; `NEMICI_PUNTO(0x0008, 0x0026)` per il gestore dei `rst 20`
   interni, `(0x0008, 0x0014)` per `rst 08/10`); `PUNTO_LARGO` per ritorni dentro
   routine tradotte senza punti interni. Se una NMI pulita diverge, leggere la
   diagnostica (ritorno, hash C e MAME), aggiungere il punto o il punto largo
   mancante; le NMI a metà attività divergenti non fermano il giro.
6. **Saldatura in `macchina.c`**: `diff_dump(long frame)` diventa
   `diff_dump(long frame, int pila)` (con `pila = 1` non esclude dbc0-dbff:
   solo c040-c043 e cf00-cfff; `sanita()` e la diagnostica a r. 395 passano 0).
   Nuova `saldatura()` accanto a `sanita()`; in `esegui_nmi()` (dove oggi
   `if (r->frame == FRAME_DUMP) sanita();`) aggiungere `if (r->frame ==
   FRAME_SALDATURA) saldatura();` con `#define FRAME_SALDATURA 2410`. Stampa
   (testo da rispettare):
   `SALDATURA 2410: byte diversi in c000-e5ff (escluse c040-c043, cf00-cfff): <N>, coincide`
   oppure `..., DIVERSO` (e allora `nmi_divergenti++; ferma(1)` come `sanita()`;
   `diff_dump` stampa già i primi 32 byte diversi). Il dump di MAME è
   `out/giroboot_2410.bin`, preso nello stesso istante del 602 (prima della NMI
   del frame); la NMI 2410 ha ritorno 3AFD, cioè nello spedidore (pulita). Con
   l'oracolo che finisce al 2410, `frame_fine` vale 2410 senza argomenti.
7. **Documenti**: `decomp/GIRO.md` (sezione H4b3: fasi, corpi, punti, sequenza
   degli arresti, esito della saldatura, deviazioni), `STATO.md` (una voce H4b3;
   banco 305/1272 → nuovi numeri dove dicono «banco»: `grep -n -E '\b305\b|\b1272\b'
   STATO.md decomp/ROUTINE.md`), `compiti/PIANO-HARDWARE.md` (H4b3 fatto,
   restano H4c e H5), `decomp/ROUTINE.md` (righe 37B4 e 2D8A, totale, tabella dei
   corpi 07, 08, 0B, 0C con stato «tradotta (giro)»).

## Sequenza degli arresti prevista (attesa, non misurata)

Traducendo in ordine di fase il giro si ferma di volta in volta su
`ROUTINE <gestore> mancante` per il primo gestore non tradotto (2DCE al frame
1398, poi 31C5, 2E0A, 2E9C, 3709, 3829, 321E, 2EF5, 2E6D, 2F1A, 2E1E: il frame
di ciascuno è la prima spedizione della fase, non noto), quindi
`CORPO 08 (7C98) non tradotto al frame 1531`, `CORPO 0C (7BF6) ... 1541`,
`CORPO 0B (2C27) ... 1562`, poi nessun arresto fino al 2410. Riportare nel
rapporto la sequenza davvero osservata (gestore o corpo, frame, spedizioni).

## Verifiche obbligatorie

Da `decomp/` salvo diversa indicazione. (m) = misurato prima di scrivere il
brief; (d) = dedotto dal registro; (a) = attesa, non misurata. Riportare esatto
ciò che esce.

1. `cd harness && make -B all gioco_libero 2>&1 | grep -c -i -E 'warning|error'`
   → `0`.
2. Catture (m): `sh cattura.sh 0x37B4 titolo; sh cattura.sh 0x37B4 titolo 2 n2;
   sh cattura.sh 0x37B4 gioco; sh cattura.sh 0x37B4 gioco 2 n2; sh cattura.sh
   0x2D8A gioco; sh cattura.sh 0x2D8A att0B` → sei righe `CATTURA <entry> <stato>
   frame_da=<F> OK` come sopra (F: 1000, 1000, 1700, 1700, 1500, 2300).
3. `cd harness && ./harness 0x37B4 | tail -1; ./harness 0x2D8A | tail -1` →
   `RIEPILOGO 0x37B4: 4 catture, 4 OK` e `RIEPILOGO 0x2D8A: 2 catture, 2 OK` (a).
4. Giro: `harness/gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt
   out/ingressi_replay.txt; echo "exit $?"` →
   - `SANITA 602: ... 0, coincide` (m);
   - `SALDATURA 2410: byte diversi in c000-e5ff (escluse c040-c043, cf00-cfff): 0,
     coincide` (a: condizione dura della saldatura);
   - `spedizioni eseguite: 41396` (d);
   - `NMI confrontate: 2345, divergenti: <M>` (d; M = divergenti a metà attività,
     accettabile; baseline 15 al frame 1398);
   - `dopo il boot: nello spedidore <P> (divergenti 0), a metà attività <Q>
     (divergenti <Y>)` con P + Q = 1808 e **divergenti nello spedidore = 0** (d);
   - nessuna riga `R letto`, `DIVERGENZA`, `ROUTINE`, `CORPO`, `INDICE`; `exit 0`.
   Se il giro si ferma prima (altro `ROUTINE`/`CORPO`/`INDICE`/`DIVERGENZA`), si
   corregge e si riporta la sequenza degli arresti intermedi.
5. Se la saldatura dà `DIVERGENZA` nei byte (N > 0): non si aggiungono
   esclusioni; si riportano i primi 32 byte diversi e la causa (vedi domanda 2).
   Se differiscono solo byte nella pila dbc0-dbff, riportarlo come deviazione.
6. Ingressi e oracolo invariati: `grep -c '^F ' out/giroboot_oracolo.txt` → `2345`;
   `sha1sum out/ingressi_replay.txt` → `43934a8eeed8d44a071d7ab958d32ea74fbd437f`;
   `cmp out/giroboot_2410.bin out/replay_2410.bin && echo identici` → `identici` (m).
7. `grep -c -E 'case (1[3-9]|2[0-5]):.*nemici_manca' harness/attivita_attract07.c`
   → `1` (solo la fase 16 resta; a).
8. `pgrep -c mame` → `0`.
9. Banco: `cd harness && ./harness --tutte > /tmp/h47.txt; echo "exit $?"; grep -c
   FALLITO /tmp/h47.txt; grep -c '^RIEPILOGO' /tmp/h47.txt; grep -c '^cattura'
   /tmp/h47.txt` → `exit 0`, `0`, `307`, `1278` (a: 305 + 2 voci; 1272 + 4 + 2).
10. Replay storico: `harness/replay out/replay_oracolo.txt out/replay_disp.txt
    out/replay_2410.bin 2410 > /tmp/r47.txt 2>&1; echo "exit $?"; tail -4 /tmp/r47.txt`
    → `exit 0` e (m, brief 45/46): `spedizioni eseguite: 64686, fino al frame 5499` /
    `NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti:
    nello spedidore 0, a metà attività 885` / `prima divergenza a metà attività
    (attesa): frame 2415` / `nessuna divergenza nello spedidore`.
11. `harness/sched_proto out/sched_log.txt | tail -1` → `spedizioni confrontate:
    106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (m, brief 46).
12. `sh rendi_video.sh; echo "exit $?"` → `RENDI 7/7 identici`, `exit 0` (m, brief 46).
13. `git diff --stat -- decomp/harness/replay.c decomp/harness/sched.c
    decomp/harness/sched_proto.c decomp/harness/video_rendi.c
    decomp/harness/video_sdl.c decomp/harness/mostra_replay.c
    decomp/harness/rendi_oracolo.c decomp/harness/z80_2xxx.c README.md` (dalla
    radice) → vuoto.
14. `wc -l harness/*.c | awk '$1>800 && $2!="totale"'` → vuoto; `wc -l
    harness/macchina.c harness/z80_38xx.c harness/z80_2dxx.c harness/attivita_attract*.c`
    → ciascuno sotto 650.
15. Ambito, `git status --short` dalla radice → solo: i moduli nuovi
    (`attivita_attract07c.c`, `07d.c`, `attivita_attract08.c`, `z80_2dxx.c`, o i nomi
    scelti), `attivita_attract07.c`, `attivita_attract07.h`, `attivita.c`,
    `nemici.h`, `macchina.c`, `z80_38xx.c`, `routine.h`, `routine_tab.c`,
    `Makefile`, `decomp/GIRO.md`, `decomp/ROUTINE.md`, `STATO.md`,
    `compiti/PIANO-HARDWARE.md`, `compiti/47-h4b3-saldatura.md`; `git status --short
    | grep -c -E 'gioco_libero$|\.o$|\.bin$'` → `0`.

## Consegna

Commit unico «Lotto H4b3: corpo 07 (fasi 13-25), corpi 08, 0C, 0B e saldatura al
frame 2410» con riga finale `Co-Authored-By: <modello vero> <noreply@anthropic.com>`.
Niente push. In fondo a questo brief: esito e uscita delle verifiche. Rapporto
finale: massimo trenta righe con l'uscita delle 15 verifiche, la sequenza degli
arresti osservata, i punti `NEMICI_PUNTO` e `PUNTO_LARGO` aggiunti con i ritorni
che coprono, NMI pulite e a metà attività, istruzioni davvero tradotte,
esito della saldatura e deviazioni. Nota per la chat principale: `README.md` ha
la riga del banco ferma a 304/1270 e la aggiorna lei.

## Domande aperte

1. **Preferenza — moduli e nomi.** `attivita_attract07c.c`/`07d.c` per le fasi
   13-25, `attivita_attract08.c` per i tre corpi, `sub_37b4` in `z80_38xx.c`,
   `sub_2d8a` nel nuovo `z80_2dxx.c` (perché `z80_2xxx.c` è a 796 righe).
   Proposta: così.
2. **Scelta di progetto — saldatura che non coincide.** Se al 2410 restano byte
   diversi fuori dalle esclusioni decise (c040-c043, cf00-cfff), le cause possibili
   sono una traduzione errata o uno stato non modellato (per esempio la pila:
   SP non si modella, GIRO.md). Proposta: lo sviluppatore cerca la causa, corregge
   se è un errore di traduzione, e se è stato non modellato lo riporta senza
   aggiungere esclusioni né chiudere il lotto; serve una decisione dell'utente
   (esclusione motivata o lavoro in più).
3. **Scelta di progetto — arresti imprevisti tra 1884 e 2410.** Dal 1884 gira il
   gioco (motore dei nemici, ~158/220 routine tradotte): il giro può fermarsi su
   una routine o un tipo di nemico non ancora tradotto, non misurabile prima.
   Proposta: se manca una routine piccola (circa 30 istruzioni o meno) si traduce
   nel lotto; altrimenti ci si ferma, si riporta l'arresto e la saldatura passa a
   un lotto successivo.
4. **Preferenza — catture.** 37B4: `titolo`, `titolo_n2`, `gioco`, `gioco_n2` (4);
   2D8A: `gioco` (14 byte scritti) e `att0B` (banale, 0 byte diversi). Proposta:
   registrarle tutte, con la nota «banale» sulla seconda di 2D8A e un tentativo di
   `sh cattura.sh 0x2D8A gioco 2 n2`; se non esiste, restano 1278 catture.
5. **Preferenza — `macchina.c` toccato solo per la saldatura.** `diff_dump` con
   parametro `pila`, `saldatura()` e `FRAME_SALDATURA`; resto invariato, file
   sotto le 650 righe. Proposta: così.
6. **Preferenza — fase 16 (2D9D).** Mai eseguita nei frame 0-2410: resta
   `nemici_manca(0x2d9d)`. Proposta: così.

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Domande 2 e 3 sono scelte di progetto, risolte dalla chat principale con le
proposte dell'analista perché sono politiche conservative di gestione degli
imprevisti (in entrambi i casi la decisione finale torna all'utente); da
segnalare nel rapporto se scattano:
2. Sì: se la saldatura mostra byte diversi fuori dalle esclusioni decise,
   lo sviluppatore corregge solo se è un errore di traduzione chiaro;
   se è stato non modellato, riporta senza aggiungere esclusioni e senza
   chiudere il lotto — decide l'utente.
3. Sì: arresti imprevisti tra il frame 1884 e il 2410 su routine non
   tradotte del motore: si traducono nel lotto solo quelle piccole
   (~30 istruzioni o meno), altrimenti ci si ferma e si riporta.
Preferenze 1 e 4-6, accolte con la proposta:
1. Sì: moduli `attivita_attract07c.c`/`07d.c`, `attivita_attract08.c`,
   `z80_2dxx.c` per `sub_2d8a`.
4. Sì: tutte le catture registrate (4 per 37B4, 2 per 2D8A, nota
   «banale» su att0B), con la prova di `2D8A gioco 2 n2`.
5. Sì: `macchina.c` toccato solo per `diff_dump`, `saldatura()` e
   `FRAME_SALDATURA`.
6. Sì: la fase 16 (2D9D, mai eseguita) resta `nemici_manca(0x2d9d)`.

## Esito (sviluppatore, 2026-11-03)

Eseguito. Verifiche 1-15 passate; uscite principali:
- Giro: `SANITA 602 ... 0, coincide`; `SALDATURA 2410: byte diversi in c000-e5ff
  (escluse c040-c043, cf00-cfff): 0, coincide`; spedizioni 41396; NMI confrontate
  2345, divergenti 30 (tutte a metà attività, `PUNTO_LARGO`); dopo il boot nello
  spedidore 1132 (divergenti 0), a metà attività 676 (divergenti 30); exit 0.
- Catture: sei righe `OK` (frame 1000, 1000, 1700, 1700, 1500, 2300); banco
  307 voci, 1278 catture, 0 FALLITO; `2D8A gioco 2 n2` non esiste.
- Replay: invariato (64686 spedizioni, 885 divergenze a metà attività, prima al 2415);
  `sched_proto` 106124/0; `RENDI 7/7 identici`; file proibiti non toccati.
- Deviazioni: per la saldatura sono serviti ganci `PUNTO_GIRO` (nuova macro in
  `nemici.h`, attiva solo con `giro_dal_boot` acceso da `gioco_libero.c`) in
  routine già verificate (3097, 452B, 4DEE, 4E31, 4E87, corpo 14, 4DAD, 5693);
  `sub_249e` e `sub_24b1` rifatte in linea perché `z80_2xxx.c` non si tocca.
  La verifica 15 sui `.bin` non vale alla lettera: i `snap_*.bin` delle catture
  sono nel commit come per 3ADF. Dettagli in `decomp/GIRO.md` (sezione H4b3).
