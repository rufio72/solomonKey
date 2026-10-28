# Compito 28: routine dei quadri bonus e del finale (censimento del lotto 12)

Data: 2026-10-28. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere.

## Obiettivo

Chiudere le 40 destinazioni di `decomp/CENSIMENTO-12.md`: tradurre in C le
28 che sono routine vere e catturabili, ciascuna con catture MAME nel banco
(`harness --tutte`), e dare un destino scritto alle altre 12 (interni di
routine già tradotte, lotto a parte, lotti N8-N11). Prima si ricostruisce lo
script del censimento (`decomp/censimento_12.py`) e si estende `cattura.sh`
con gli stati `oggetti`, `finale2` e `attNN`. Banco, `sched_proto` e replay
fanno da regressione e non devono cambiare.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `replay.lua`, `replay.sh`, `replay.c`, `sched*.c`, `rst20.py`,
  `rst20_tables.h`, `rst38_tables.h`, `censimento_nemici.py`, `sessioni.sh`
  né le catture già nel repository (`git ls-files decomp/out | grep snap_`).
- Niente installazioni. MAME (`/usr/games/mame`) solo senza finestra, come
  in `cattura.sh` (`SDL_VIDEODRIVER=dummy -video none -sound none`, i
  quattro provider a `none`). Nessuna partita a mano.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra
  le 800 righe; i moduli `nemici*.c` sotto le ~700.
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`). Si
  ripristina solo il singolo file con `git checkout -- <file>`.

## Cosa esiste già (fatti verificati)

### Strumenti e banco

| file | righe | ruolo |
|---|---:|---|
| `decomp/cattura.sh` | 79 | stati `titolo gioco tardo porta bonus finale stanzaNN`; `SNAP_POKE`, `SNAP_POSTA`, `SNAP_COND`, `SNAP_FORZA` passano dall'ambiente; prova i frame in `FRAMES` e scrive `out/snap_<ENTRY>_<tag>_{prima,dopo}.bin`, `_regs.txt` |
| `decomp/snap.lua` | 221 | breakpoint all'ingresso; `SNAP_LIMITE` (predefinito 6000, riga 33); `local REG = { "PC", "AF", "BC", "DE", "HL", "IX", "IY", "SP" }` (riga 70); `registri()` (riga 136) scrive ` %s=%04X` per voce |
| `decomp/sessioni.sh` | 133 | funzione `lotto12()`: inneschi delle sessioni `porta bonus_04 bonus_42 finale finale2 oggetti attract2 att_XX` (da copiare, il file non si tocca) |
| `decomp/harness/harness.c` | 193 | legge i registri con `sscanf(riga, "%s PC=%x AF=%x BC=%x DE=%x HL=%x IX=%x IY=%x SP=%x")` e `== 8` (righe 46-50); cerca `snap_<ENTRY>_regs.txt` e `snap_<ENTRY>_*_regs.txt` |
| `decomp/harness/ram.h` | 56 | `struct z80 { uint8_t a, f, b, c, d, e, h, l; uint16_t ix, iy, sp; };` (righe 19-22): nessun registro R |
| `decomp/harness/routine_tab.c` | 157 | `tabella_routine[]`, voci `{ "0xXXXX", sub_xxxx, blocco_ini, blocco_fine }` in ordine di indirizzo |
| `decomp/harness/Makefile` | — | `$(wildcard z80_*.c)` e `$(wildcard nemici*.c)` entrano in `harness` e `replay`: moduli nuovi con quei prefissi non richiedono modifiche |

Il registro R è leggibile da Lua: `manager.machine.devices[":maincpu"].state["R"].value`
(misurato: stampa `R=27` al frame 100). Le catture in `decomp/out/` sono
tracciate in git malgrado `.gitignore` (`decomp/out/*`): le nuove si
aggiungono con `git add -f`.

### Moduli di destinazione (righe misurate con `wc -l`)

`z80_1xxx.c` 32, `z80_2xxx.c` 502, `z80_38xx.c` 132, `z80_4xxx.c` 439,
`z80_6xxx.c` 77, `z80_7xxx.c` 229, `nmi.c` 264, `attivita.c` 354,
`nemici_comparsa.c` 50, `nemici_comuni.c` 433, `routine.h` 163,
`nemici.h` 177. `z80_3xxx.c` (551) non si tocca: le routine 3Cxx-3Fxx
vanno in `z80_38xx.c`, che contiene già `sub_3f88`.

### Stato attuale delle 40 (grep su `harness/*.c|h`, cammino statico su `out/main64.bin`)

Byte = PC raggiungibili dall'ingresso senza attraversare altri ingressi né
routine tradotte (stima per le righe: 0,6 righe C a byte).

**A. Da tradurre in questo lotto (23 del lotto 12, 502 B):**

| ingresso | byte | chiama | stato oggi | modulo |
|---|---:|---|---|---|
| 03EE | 13 | 3A54 | stub `static` in `nmi.c` righe 23-30 (stampa avviso), chiamato dalla NMI a 010B se c078 ≠ 0 | `nmi.c` |
| 1471 | 3 | — (cade in 1474) | assente: `ld hl,c093` | `z80_1xxx.c` |
| 1474 | 11 | — | assente: raddoppio BCD di 3 byte a HL (`adc a,a; daa`) | `z80_1xxx.c` |
| 147F | 18 | rst 10 | assente | `z80_1xxx.c` |
| 1533 | 22 | — | assente; esce con `scf` o `ret z` | `z80_1xxx.c` |
| 2425 | 11 | — | assente | `z80_2xxx.c` |
| 2430 | 17 | — | assente | `z80_2xxx.c` |
| 2441 | 9 | — | assente: HL = 16·A | `z80_2xxx.c` |
| 250E | 15 | rst 10 | `static uint16_t sub_250e(uint16_t hl, int *riporto)` in `attivita.c` riga 88, usata da `attivita_0d_5c8e` (5C9E) | `attivita.c` |
| 3CC1 | 1 | — (cade in 3CC2) | assente: `ld (hl),a` | `z80_38xx.c` |
| 3CC2 | 8 | — | assente | `z80_38xx.c` |
| 3D20 | 15 | — (ciclo `djnz`, cade in 3D2F) | assente | `z80_38xx.c` |
| 3D2F | 20 | — | assente | `z80_38xx.c` |
| 3D43 | 9 | 4D87 | assente | `z80_38xx.c` |
| 3DA6 | 13 | — | assente | `z80_38xx.c` |
| 3DEA | 36 | 4AF6 | assente | `z80_38xx.c` |
| 3FB2 | 28 | 3FCE | assente (`rld` su c095-c093) | `z80_38xx.c` |
| 3FCE | 26 | — | assente (`jr z` d'ingresso: usa Z di F) | `z80_38xx.c` |
| 4302 | 23 | 2441 | assente; **usa `ld a,r` due volte** | `z80_4xxx.c` |
| 4319 | 127 | 24BE, 4B98, 4C11, 6EB7 | assente; smista su A−0F: <4 → 432B, 5-6 → 4360, 8 → 437C | `z80_4xxx.c` |
| 6976 | 49 | rst 10 | assente | `z80_6xxx.c` |
| 7B95 | 9 | — | assente | `z80_7xxx.c` |
| 7B9E | 19 | 3A54 | assente (ciclo fino a ff) | `z80_7xxx.c` |

**B. Dipendenze anticipate dei lotti nemici (5, 113 B)**, nel modulo che
`CENSIMENTO-NEMICI.md` prescrive (come le 6 anticipate dal brief 27):

| ingresso | byte | lotto | stato oggi | modulo |
|---|---:|---|---|---|
| 134B | 16 | N4 | assente | `nemici_comparsa.c` |
| 4ADA | 28 | N12 | `STUB_NEMICI(4ada)` in `nemici_stub.c`, dichiarata in `nemici.h` riga 154; scrive il record c078-c07b che 03EE consuma | `nemici_comuni.c` |
| 5DDF | 42 | N8 | assente; **usa `ld a,r` due volte**; chiama 5E09 | `nemici_tipo07a.c` (nuovo) |
| 5E09 | 17 | N8 | assente | `nemici_tipo07a.c` (nuovo) |
| 6248 | 10 | N11 | assente | `nemici_tipo07b.c` (nuovo) |

Firme già dichiarate da usare: `sub_3a54`, `sub_4d87`, `sub_4af6`,
`sub_2457`, `sub_3a2e` in `routine.h`; `void sub_4b98(uint16_t ix)`,
`sub_24be`, `sub_4c11`, `sub_6eb7` in `nemici.h` (righe 72, 81, 86, 147).

**C. Fuori da questo lotto (12), da annotare in `CENSIMENTO-12.md`:**
- 02A2, 02A4, 02A5: etichette interne di `sub_027c` (027C-03ED, 370 B,
  `nmi_oggetti.c`, già verificata); la «prima call statica» (9A7D, 9CB1...)
  è un dato disassemblato come codice.
- 40FF: interno del ciclo `40FE ld (hl),d / add hl,de / djnz 40FE` nel
  corpo dell'attività 1C (spedidore a passi, `rst 20` a 4170), non
  tradotto: va col corpo 1C.
- 7B64: interno del passo 7B3B-7B94 (`djnz 7B64`, chiude con `pop hl; ret`):
  va col corpo che lo contiene.
- 0CFD, 0E0B, 11A5: la continuazione 0CF3 di `sub_0ce6` smista con
  `jp (hl)` sulla tabella 0D18 (16 gestori: 0D3C 0D8E 0E1F 0EAC 0EDF 0F6B
  0F75 0FA9 100E 109C 10CF 10D9 113C 1172 11D8 0D38); 0E0B sta nel
  gestore 0D8E, 11A5 in 113C. Insieme 1278 B (≈ 770 righe C, 1065 B
  eseguiti nelle sessioni): lotto a parte con modulo proprio.
- 5115, 5124, 51FA: N10. 5124 e 51FA finiscono in `rst 38` sulle tabelle
  5128 (8 voci) e 5208 (14 voci) e 51FA chiama 51D7→54C5; 5115 non ha
  occorrenze nello stato `finale` con gli ingressi «traccia» (misurato).
- 5FDC: N11 (chiama 4FEF e 5E8F, ricade in 5FAC).

## Struttura da realizzare

### 1. `decomp/censimento_12.py` (nuovo, < 150 righe)

Rigenera le colonne 1-5 della tabella di `CENSIMENTO-12.md` dai file di
`decomp/out/`. Algoritmo (verificato: riproduce le 40 righe identiche):
- sessioni, in quest'ordine: `porta bonus_04 bonus_42 finale finale2
  oggetti attract2 att_16 att_1D att_25 att_26 att_27`;
- destinazioni = prima colonna di `out/call_mai_prima_lotto12.txt` (righe
  non `#`), tenute se il PC compare nell'unione di `out/<s>_eseguiti.txt`;
- estensione = intorno contiguo dell'ingresso nell'unione ordinata, con
  passo ≤ 4 fra PC consecutivi;
- invocazioni e chiamanti da `out/<s>_chiamate.txt` (`sito destinazione
  numero`), sommati per sito, siti ordinati, `sito×numero` separati da
  spazio, `-` se nessuno;
- sessioni = quelle il cui `_eseguiti` contiene l'ingresso, nell'ordine sopra;
- stampa `| %s | %04X-%04X | %d | %s | %s |` per riga, in ordine di
  destinazione. Uso: `python3 censimento_12.py` da `decomp/`.

### 2. `decomp/cattura.sh`: tre stati nuovi

Inneschi copiati da `sessioni.sh` (`lotto12()`); `SNAP_POKE`/`SNAP_POSTA`
dall'ambiente restano prioritari; aggiornare commento e messaggio d'errore.
- `oggetti`: `FRAMES="2300"`; poke e poste costruiti col ciclo di
  `sessioni.sh` righe 82-93 (terne `08 09 0a` … `1a 1b 00` in c05b, posta
  22 ogni 100 frame dal 2300).
- `finale2`: inneschi di `finale`, `FRAMES="4500"`, ed
  `export SNAP_LIMITE=${SNAP_LIMITE:-9500}`.
- `att[0-9A-F][0-9A-F]` (es. `att25`): poke `2250:0xc19d:0,2250:0xc19e:0`,
  posta `2300:0x<NN>`, `FRAMES="2300"`.

### 3. Registro R (per 4302 e 5DDF)

- `snap.lua`: aggiungere `"R"` in coda a `REG` (la riga diventa `... SP=xxxx R=00xx`).
- `ram.h`: `uint8_t r;` in coda a `struct z80`.
- `harness.c`: dopo il `sscanf` a 8 campi, un secondo `sscanf` che legge
  ` R=%x` se presente (catture vecchie senza R: `z->r = 0`).
- Nel C: valore letto da `ld a,r` = `(r0 & 0x80) | ((r0 + k) & 0x7f)`, con
  r0 = R all'ingresso e k = numero di cicli M1 fino a quell'istruzione
  compresa (1 per istruzione, 2 per prefissi CB/DD/ED/FD; `call` e `ret`
  di 2441 contano). Il valore esatto di k va confermato dalla cattura
  (attesa, non misurata).

### 4. Traduzioni

- Ogni routine: `void sub_xxxx(struct z80 *z)`, commento con indirizzo e
  effetto, registri d'uscita come l'originale; Z/riporto letti dal
  chiamante sul bit 0/6 di `z->f` come da convenzione (`ROUTINE.md`).
- 1471 e 3CC1 cadono nella successiva: `sub_1471` = `HL = c093` poi
  `sub_1474(z)`; `sub_3cc1` = `M(HL) = A` poi `sub_3cc2(z)`. 3D20 cicla
  `djnz` e poi cade in 3D2F: chiama `sub_3d2f(z)` in coda.
- 03EE: togliere `static` e l'avviso, tradurre (b = a, (hl) = 0, C, D, E da
  c079-c07b, `sub_3a54`); dichiararla in `routine.h` (togliere la frase
  «Lo stub 03EE è statico in nmi.c», riga 137).
- 250E: nuova `void sub_250e(struct z80 *z)` esportata (uscita: A = 0,
  B = 0, C = vecchio (HL), DE = HL+1, HL = parola nuova, riporto di
  `add hl,bc` nel bit 0 di `z->f`); la statica diventa un involucro
  (rinominata, es. `conta_250e`) che chiama `sub_250e` su una `struct z80`
  locale, così `attivita_0d_5c8e` non cambia comportamento.
- 4ADA: togliere `STUB_NEMICI(4ada)` da `nemici_stub.c`, definirla in
  `nemici_comuni.c`.
- Nuove dichiarazioni: le z80 in `routine.h`, le nemici in `nemici.h`.
- `routine_tab.c`: 28 voci nuove in ordine di indirizzo, blocco `0, 0`.

### 5. Catture (da `decomp/`)

`sh cattura.sh <ENTRY> <stato> [N] [SUFFISSO]`. Frame misurati con
`snap.lua` lanciato a mano con gli stessi inneschi, ingressi «traccia» e
`SNAP_DIR` temporanea: `cattura.sh` con gli stati nuovi deve dare gli
stessi frame.

| ingresso | catture (stato N suffisso → frame) |
|---|---|
| 03EE | `stanza17` f1975; `stanza17 2 n2` f2047 |
| 1471 | `oggetti` f2502; `oggetti 2 n2` f2602 |
| 1474 | `oggetti` f2502; `oggetti 2 n2` f2503 |
| 147F | `oggetti` f2602; `oggetti 2 n2` f2603 |
| 1533 | `oggetti` f2302; `oggetti 2 n2` f2402 |
| 134B | `oggetti` f2402; `oggetti 2 n2` f2702 |
| 2425 | `porta` f2303; `porta 2 n2` f2304; `bonus` f2303 |
| 2430 | `porta` f2302; `finale` f2302 (N=2 assente in entrambi) |
| 2441 | `oggetti` f2456 (unica: resta `tradotta`) |
| 250E | `oggetti` f2422; `oggetti 2 n2` f2423 (in `finale2` nessuna) |
| 3CC1, 3CC2 | `att25` f2501; `att25 2 n2` f2502 |
| 3D20, 3D2F, 3D43 | `att25` f2502; `att25 2 n2` f2503 |
| 3DA6 | `att25` f2368; `att25 2 n2` f2369; `att26` f2302 |
| 3DEA | `att25` f2302; `att26` f2302 |
| 3FB2 | `porta` f2386; `finale` f2386; `att25` f2502 |
| 3FCE | `porta` (già nel repository, non rifarla); `porta 2 n2` f2387 |
| 4302 | `oggetti` f2456 (unica: resta `tradotta`) |
| 4319 | `oggetti` f2456 (A = 12, ramo 432B); `SNAP_FORZA=a:0x15` `oggetti 1 forza4360` f2456; `SNAP_FORZA=a:0x17` `oggetti 1 forza437c` f2456 |
| 6976 | `att16` f2430; `finale2` f5988 |
| 7B95, 7B9E | `finale2` f8663; `finale2 2 n2` f8664 |
| 4ADA | `stanza17` f1974; `stanza17 2 n2` f2046; `stanza06` f2230 |
| 5DDF, 5E09 | `finale` f5553; `finale 2 n2` f5700 |
| 6248 | `finale` f3419; `finale 2 n2` f3751; `finale2` f5215 |

Totale: 60 catture nuove (3FCE porta esclusa). Le `forza*` contano come
OK ma non per `verificata` (servono 2 catture non forzate).

### 6. Documenti

- `ROUTINE.md`: sezione nuova «Routine dei quadri bonus e del finale
  (brief 28)» con la tabella a colonne standard (ingresso, funzione, file,
  byte, descrizione, stato, catture OK, stati, note) e le note sulle
  forzature di 4319 e sul registro R.
- `CENSIMENTO-12.md`: colonna o sezione «destino» per le 40 (gruppi A, B, C
  sopra) e riga sullo script `censimento_12.py`.
- `CENSIMENTO-NEMICI.md`: 134B, 4ADA, 5DDF, 5E09, 6248 anticipate dal brief 28.
- `STATO.md`: punto 1 di «Lavoro in sospeso» aggiornato (fatto, resta il
  lotto della chiave 0D18).
- Se il replay cambia, correggere fino a riallinearlo e annotarlo in
  `REPLAY.md`.

## Verifiche obbligatorie

1. `cd decomp && diff <(grep '^| [0-9A-F]\{4\} | [0-9A-F]\{4\}-' CENSIMENTO-12.md | cut -d'|' -f1-6 | sed 's/$/|/') <(python3 censimento_12.py)`
   → uscita vuota, exit 0 (misurato con lo stesso algoritmo; vale anche
   dopo le modifiche al documento: le colonne 1-5 non cambiano).
2. Catture del punto 5: ogni comando stampa `CATTURA <ENTRY> <tag> frame_da=<F> OK`
   (attesa; frame di cattura misurati come in tabella), e
   `git status --short decomp/out/snap_0x3FCE_porta_*` esce vuoto.
3. `cd decomp/harness && make harness replay sched_proto`: nessun errore,
   nessun warning nei file toccati.
4. `cd decomp/harness && ./harness --tutte > /tmp/banco.txt; echo "exit $?"`
   → `exit 0`; `grep -c FALLITO /tmp/banco.txt` → 0;
   `grep -c '^cattura' /tmp/banco.txt` → 381 (oggi 321, misurato);
   `grep -c RIEPILOGO /tmp/banco.txt` → 175 (oggi 147, misurato).
5. `cd decomp/harness && for e in 0x03EE 0x250E 0x4302 0x4319 0x4ADA 0x5DDF; do ./harness $e > /dev/null || echo "$e fallita"; done`
   → nessuna riga.
6. `cd decomp && harness/sched_proto out/sched_log.txt | tail -1` →
   `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
7. `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/replay.txt 2>&1; echo "exit $?"; tail -4 /tmp/replay.txt`
   → `exit 0` e (misurato oggi):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
8. `grep -c STUB_NEMICI decomp/harness/nemici_stub.c` → 25 (oggi 26), e
   `grep -c 4ada decomp/harness/nemici_stub.c` → 0.
9. `grep -n "03ee: c078 non zero" decomp/harness/nmi.c` → nessuna riga.
10. `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/z80_38xx.c decomp/harness/z80_4xxx.c` < 700 ciascuno.
11. `git status --short` mostra solo: `censimento_12.py`, `cattura.sh`,
    `snap.lua`, `harness/*.c|h`, i `.md` del punto 6, le 60 catture nuove
    (`git add -f`) e questo brief. Nessun binario né `.gcda`/`.gcno`.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Brief 28: routine dei quadri bonus e del finale, 28 voci nel banco`
  e riga finale `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Rapporto di massimo 30 righe: uscita delle verifiche 1, 4, 6, 7, 8, 10;
  le voci con catture e stato (`verificata`/`tradotta`); il k del registro
  R trovato per 4302 e 5DDF; le catture che hanno dato frame diversi dalla
  tabella; le deviazioni dal brief.

## Domande aperte

1. **Perimetro: 28 tradotte su 40.** Proposta: gruppi A e B in questo
   lotto; i 12 del gruppo C con destino scritto (3 interni di `sub_027c`,
   2 interni di corpi a passi, la chiave 0D18 come lotto a parte di ~770
   righe, 5115/5124/51FA/5FDC ai lotti N10/N11). — «scelta di progetto»
2. **Anticipare 134B, 4ADA, 5DDF, 5E09, 6248 dai lotti nemici**, creando
   `nemici_tipo07a.c` e `nemici_tipo07b.c` con una o due routine. Proposta:
   sì, nei moduli prescritti dal censimento; 4ADA toglie uno stub. — «preferenza»
3. **Registro R nelle catture** (`snap.lua`, `struct z80`, lettura in
   `harness.c`), necessario per 4302 e 5DDF. Proposta: sì, campo in coda
   e compatibile con le catture vecchie. In alternativa 4302 e 5DDF restano
   fuori dal banco. — «scelta di progetto»
4. **`sub_250e` esportata con involucro** al posto della statica in
   `attivita.c`. Proposta: sì; il replay deve restare identico (verifica 7). — «preferenza»
5. **Stati `oggetti`, `finale2`, `attNN` in `cattura.sh`** invece di
   passare ogni volta `SNAP_POKE`/`SNAP_POSTA`. Proposta: sì, coi nomi
   delle sessioni senza trattino basso. — «preferenza»
6. **03EE catturata nella stanza 17** (dove 4ADA scrive c078 e la NMI
   successiva chiama 03EE), non in `bonus`, dove con gli ingressi «traccia»
   non compare. Proposta: accettarlo. — «preferenza»
