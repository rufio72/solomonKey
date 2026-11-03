# Compito 39: lotto della chiave, tabella 0D18 (gestori 0CFD-123B)

Data: 2026-11-03. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere. Non servono i brief 28 e 38 (il 28 è superato, il 38 è chiuso).

## Obiettivo

Tradurre in C lo smistamento della chiave: `sub_0cfd` (dispatcher) e i 16
gestori della tabella ROM 0D18, e completare `sub_0ce6` (oggi si ferma a
0CF3 con un avviso su stderr). Blocco 0CFD-123B = 1279 byte di codice
(+ 140 byte di tabelle dati, vedi sotto), ~770 righe C in due moduli nuovi.
17 voci nuove nel banco (`harness --tutte`), `0x0CE6` con 2 catture nuove.
Banco storico, `sched_proto` e replay non devono cambiare.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `snap.lua`, `sessioni.sh`, `replay.*`, `sched*.c`, `rst20*`,
  `rst38_tables.h`, `censimento_nemici.py`, `STATO.md`, `README.md`,
  `compiti/` (tranne questo file, niente), né le catture già in
  `decomp/out/` (`git ls-files decomp/out | grep snap_`), comprese le due di
  `0x0CE6` (`gioco`, `tardo`). `cattura.sh` e `cattura_oggetti.sh` non
  servono modifiche: non toccarli.
- Niente installazioni. MAME (`/usr/games/mame`) solo senza finestra, come
  `cattura.sh` (`-video none -sound none`). Nessuna partita a mano. Una
  cattura dura 40-100 s; fino a 6 in parallelo con `xargs -P6`.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe; i due moduli nuovi sotto le 650 (stima 340 e 430).
- Niente registro R (nessuna `ld a,r` nel blocco).
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`); si
  ripristina un file solo con `git checkout -- <file>`.
- Un solo agente alla volta su `decomp/harness`.

## Cosa esiste già (fatti verificati)

Baseline misurata il 2026-11-03 sui binari già compilati (`decomp/harness`):
`./harness --tutte` exit 0, 0 `FALLITO`, 287 `^RIEPILOGO`, 1173 `^cattura`;
`RIEPILOGO 0x0CE6: 2 catture, 2 OK`; `sched_proto` e replay: vedi verifiche 8-9.
Il replay non attiva mai la chiave (bit 7 di c4ce sempre spento), quindi
`sub_0ce6` esteso non lo cambia.

| file | righe | ruolo |
|---|---:|---|
| `decomp/harness/z80_08xx.c` | 460 | contiene `sub_0ce6` (r. 452-460): IX = c4ce, IY = c299, `ret z` se bit 7 di (c4ce) spento, poi `fprintf` di avviso. Include `<stdio.h>` (usato anche altrove, non toglierlo senza controllare) |
| `decomp/harness/routine.h` | 188 | prototipi; `sub_0ce6` a r. 148; ultima sezione «Lotto 38 (compito 38)» |
| `decomp/harness/routine_tab.c` | 297 | `{ "0xXXXX", sub_xxxx, 0, 0 },` in ordine di indirizzo; `0x0CE6` a r. 37 |
| `decomp/harness/Makefile` | 46 | `$(wildcard z80_*.c)`: i moduli nuovi `z80_chiave_*.c` entrano da soli, nessuna modifica |
| `decomp/harness/nmi.c` | | r. 203: `sub_0ce6(&n);` dalla NMI (non cambia) |
| `decomp/ROUTINE.md` | | tabella a colonne `\| ingresso \| funzione \| file \| byte \| descrizione \| stato \| catture OK \| stati \| note \|`; riga di `0x0CE6` a r. 49 |
| `decomp/CENSIMENTO-12.md` | | r. 193-195: 0CFD, 0E0B, 11A5 «documentata ... lotto futuro (1278 B)» |

Convenzioni (da `ROUTINE.md` e dai moduli): `z->a b c d e h l`, `z->ix`, `z->iy`,
`z->f`; `M(a)` solo RAM (c000-ffff); **ROM con `leggi(a)`** (ram.h: una
tabella ROM letta con `M()` ha già dato un bug); `BC(z) DE(z) HL(z)`,
`SET_BC SET_DE SET_HL`. Il banco confronta RAM e A, BC, DE, HL, IX, IY
all'uscita, non F: anche i registri «sporchi» devono coincidere (le catture
mostrano HL = indirizzo del gestore, DE e BC sporchi).
Prototipo di tutte: `void sub_xxxx(struct z80 *z)`.

### Disassemblato

`decomp/out/main64.dasm` (sorgente, mawk). Estrazione del blocco, da
`decomp/`; il file è tracciato in git come `bonus_n38.dasm` (`git add -f`):
```
awk '/^[A-Za-z_0-9]+:/ {lab=$0; next} /;[0-9a-f][0-9a-f][0-9a-f][0-9a-f][ \t]*$/ {a=substr($0,length($0)-3); if(a>="0ce6" && a<="1287"){ if(lab!="") print lab; print} lab=""; next} {lab=""}' out/main64.dasm > out/chiave_n39.dasm
```
Uscita attesa (misurata): `wc -l < decomp/out/chiave_n39.dasm` → `845`;
`grep -c 'jp (hl)' decomp/out/chiave_n39.dasm` → `1` (solo 0D17). Le tabelle
dati compaiono nel dasm come istruzioni spurie: non sono da tradurre.

### Mappa del blocco (misurata dal dasm e dalle catture)

Oggetto: IX = c4ce (slot a 0x30 byte), IY = c299 (parola di stato della
chiave). Campi: (IX+00) bit 7 = attiva; (IX+0E) Y pixel e (IX+0D) sottopixel;
(IX+15) X pixel; (IX+1A) tipo, nibble basso = indice del gestore; (IX+1D)
stato; (IX+1E) bit di direzione/fase; (IY+04), (IY+05) uscite verso la
chiave. Confronti sul nibble basso di D = (IX+0E), E = (IX+15).

**0CE6 (esistente) → 0CF3 → 0CFD**:
- 0CF3: `A = (ix+1D)`; `cp 2`; `ret nc`; `call 0CFD`; `ret`. Dentro `sub_0ce6`,
  al posto del `fprintf`: `z->a = M(0xc4ce + 0x1d); if (z->a >= 2) return;
  sub_0cfd(z);` (A = (ix+1D) deve restare nel registro d'uscita).
- 0CFD (27 B, nuova `sub_0cfd`): `A = (ix+1A) & 0F`; `HL = 0D18 + 2·A`
  (indice A, poi `E = 2A`, `D = 0`, `add hl,de`); HL = parola ROM a quell'indirizzo
  (indirizzo del gestore: **HL esce = indirizzo del gestore**, vedi catture:
  HL=0F75 dopo 0F75); `E = (ix+15)`, `D = (ix+0E)`, `B = 0`; salta al gestore.
  A d'uscita = indice del gestore se questo non lo cambia (0D38 esce con
  A=0F). Smistamento in C: tabella `static void (*const gestori_0d18[16])(struct z80 *)`
  nell'ordine della tabella ROM, indice `A`; la tabella ROM 0D18-0D37 (16
  parole) è: 0D3C 0D8E 0E1F 0EAC 0EDF 0F6B 0F75 0FA9 100E 109C 10CF 10D9
  113C 1172 11D8 0D38.

Gestori (ingressi veri, `jp (hl)` dal dispatcher o `jp` da altro gestore).
Byte = dal dasm; righe C stimate 0,6 per byte:

| ingresso | byte | modulo | note dal dasm |
|---|---:|---|---|
| 0D38 | 4 | `z80_chiave_b.c` | `ld (0000),a; ret`: scrive in ROM, **in C non scrivere** (`M(0)` sarebbe fuori array); solo `return` |
| 0D3C | 50+32 dati | `z80_chiave_a.c` | se (IX+1D) != 1 `ret`; altrimenti usa la tabella dati 0D6E (16 coppie, indice 4·(…) vedi dasm 0D41-0D6D): scrive (IX+1D)=0, (IX+1E), (IY+05); B esce 0 salvo `ld b,6/0A`; BC sporco |
| 0D8E | 145 | a | ramo basso (A=(E−2)&0F < 6, `jr c 0DD6`) e ramo alto; tabella ROM 123C (10 B, indice A−6); etichette interne 0DAE, 0DB2, 0DBC, 0DC2, 0DCE, 0DD6, 0DF6, 0DFA, 0E04, **0E0B**, 0E17 (tutte nello stesso gestore, usare `goto`) |
| 0E1F | 141 | a | come 0D8E su E, tabelle 124F e 1258 (9 B ciascuna), (IY+04) = 1, (IX+1E) = 2/3 |
| 0EAC | 51 | a | (IX+1D)−1 → 0EC1 (se zero); altrimenti `D&0F < 4` → `ret`; scrive (IX+0E) = (D&F0)+3 e (IY+05)=3 |
| 0EDF | 140 | a | su E come 0D8E, tabelle 1261 (9 B) e 126A (10 B), (IY+04) = 2 |
| 0F6B | 10 | b | `bit 3,(ix+0E)`: spento → `jp 0FA9` (chiama `sub_0fa9(z)`), acceso → `jp 1172` (`sub_1172(z)`) |
| 0F75 | 52 | b | (IX+1D)=1 → 0F8A; altrimenti `E&0F < 0C` → `ret`; scrive (IX+15) = (E&F0)+0B e (IY+05)=1 |
| 0FA9 | 101 | b | usa B = (IX+1D)−1 e i bit 0-2 di B come flag; scrive (IX+0E)/(IX+15) = (D o E & F0) + 3/0B |
| 100E | 142 | b | su E come 0D8E, tabelle 1274 (10 B) e 127E (10 B), (IY+04) = 3 |
| 109C | 51 | b | come 0EAC su E (`cp 5`, `ret nc`); (IX+15) = (E&F0)+5, (IY+05) = 0 |
| 10CF | 10 | b | `bit 3,(ix+0E)`: spento → `sub_10d9(z)`, acceso → `sub_11d8(z)` |
| 10D9 | 99 | b | come 0FA9 sui valori 5/3 |
| 113C | 54 | b | (IX+1D)=1 → 1155; altrimenti `D&0F >= 0D` → `ret`; (IX+0E) = (D&F0)+0D, (IY+05)=2 |
| 1172 | 102 | b | come 0FA9, contiene la label **11A5** (interna, non è un ingresso), (IY+05)=2 |
| 11D8 | 100 | b | come 10D9, uscita su (IY+05) = 1 o 2 |

Il disassemblato è la sola fonte per i dettagli: ogni gruppo di istruzioni
si traduce letteralmente e si commenta con l'indirizzo del dasm, come nei
moduli esistenti (`z80_08xx.c`). Registri sporchi: dopo 0D3C B=FF e C=0007,
dopo 0EDF HL=126C ecc.; seguire il dasm riga per riga, non semplificare.

Tabelle dati ROM (leggere con `leggi()`, non copiare): 0D18 (16 parole,
dispatcher), 0D6E-0D8D (32 B, solo 0D3C), 123C (10 B), 1246 (9 B), 124F (9 B),
1258 (9 B), 1261 (9 B), 126A (10 B), 1274 (10 B), 127E (10 B) fino a 1287.
Il censimento diceva «11A5 in 113C»: è inesatto, 11A5 cade in 1172
(113C-1171 chiude con `ret` a 1171). 0E0B è dentro 0D8E.

## Struttura da realizzare

1. `decomp/harness/z80_chiave_a.c` (nuovo, ~340 righe): `sub_0cfd`,
   `sub_0d3c`, `sub_0d8e`, `sub_0e1f`, `sub_0eac`, `sub_0edf`. Include
   `<stddef.h>` e `routine.h`; per i salti ai gestori di `z80_chiave_b.c`
   le funzioni sono dichiarate in `routine.h`. Intestazione col commento del
   campo-oggetto (offsetti `#define` locali: nomi liberi, come
   `OGG_*` di `z80_08xx.c`, che non si condividono).
2. `decomp/harness/z80_chiave_b.c` (nuovo, ~430 righe): `sub_0d38` (solo
   `return`), `sub_0f6b`, `sub_0f75`, `sub_0fa9`, `sub_100e`, `sub_109c`,
   `sub_10cf`, `sub_10d9`, `sub_113c`, `sub_1172`, `sub_11d8`. 0F6B e 10CF
   fanno tail call (`sub_0fa9(z); return;`), quindi `routine.h` basta.
3. `z80_08xx.c`: `sub_0ce6` completata (sopra); aggiornare il commento di
   testa della funzione (non più «non traducibile»); `fprintf` e avviso tolti.
4. `routine.h`: sezione nuova «Lotto 39 (compito 39): la chiave, tabella
   0D18» con i 17 prototipi.
5. `routine_tab.c`: 17 voci `{ "0x....", sub_xxxx, 0, 0 }` in ordine di
   indirizzo (0CFD dopo 0CE6; 0D38 ... 11D8 prima di 0x1288).
6. Catture (sotto), documenti (sotto).

La suddivisione in `a` (dispatcher + 5 gestori da 0D3C a 0EDF) e `b` (resto,
0F6B-11D8 più 0D38) lascia entrambi i moduli sotto le 450 righe; il nome
`z80_chiave_*.c` è libero ma deve restare `z80_*.c` (wildcard del Makefile).

## Catture (da `decomp/`)

Provate in una copia di lavoro fuori dal repository (`out/` vuota, `roms`
collegata): tutte le catture elencate sotto riescono. Si usa lo stato
`gioco` di `cattura_oggetti.sh` (già nel repository, 14 terne di oggetti
08-1B): la chiave si attiva con tutti i 16 tipi. Una cattura riuscita
stampa `CATTURA <ENTRY> <tag> frame_da=2400 OK`. Per `jp (hl)` l'ingresso è
l'indirizzo del gestore (SP in cima = 0CFC, ritorno a 0CFC con SP+2).

**A. Non forzate (34)**, file `/tmp/n39_a.txt` (`ENTRY N [suffisso]`),
`xargs -P6 -L1 sh cattura_oggetti.sh < /tmp/n39_a.txt`:
tutte le coppie `<ENTRY> 1` e `<ENTRY> 2 n2` per ENTRY in
`0x0CE6 0x0CFD 0x0D3C 0x0D8E 0x0E1F 0x0EAC 0x0EDF 0x0F6B 0x0F75 0x0FA9
0x100E 0x109C 0x10CF 0x10D9 0x113C 0x1172 0x11D8` (17 ENTRY × 2 = 34;
`0x0CE6` con tag `gioco_oggetti`, diverso dalle sue due esistenti).
Frame di cattura (misurati, N1 / N2): 0CE6 2402 / 2404 · 0CFD 2430 / 2432 ·
0D3C 3310 / 3312 · 0D8E 2597 / 2599 · 0E1F 3175 / 3177 · 0EAC 2608 / 2610 ·
0EDF 2438 / 2440 · 0F6B 3735 / 3737 · 0F75 2430 / 2432 · 0FA9 3237 / 3239 ·
100E 2449 / 2451 · 109C 2579 / 2581 · 10CF 3731 / 3733 · 10D9 3216 / 3218 ·
113C 2518 / 2520 · 1172 2513 / 2515 · 11D8 2534 / 2536.
Durata misurata del gruppo A: ~15 minuti con `-P6`.

Occorrenze naturali per ingresso (da `oggetti_istruzioni.txt`, esecuzioni in
tutta la sessione): 0D3C 43 · 0D8E 13 · 0E1F 42 · 0EAC 45 · 0EDF 46 ·
0F6B 11 · 0F75 3 · 0FA9 11 · 100E 49 · 109C 116 · 10D9 4 · 113C 62 ·
1172 8 · 11D8 10 (10CF non compare lì ma la cattura N1 e N2 riescono).
Altre occorrenze si prendono con `sh cattura_oggetti.sh 0x.... N nN`
(provato: `0x0E1F` N=3,4,5,6 riescono).

**B. Forzata di 0D38 (1)**: 0D38 non è mai raggiunto in modo naturale
(FALLITA N1 e N2, misurato). Si ottiene scrivendo 0F in (IX+1A) = c4e8 un
frame prima del dispatch 0CFD (frame 2430):
```
SNAP_POKE_EXTRA=2429:0xc4e8:0x0f sh cattura_oggetti.sh 0x0D38 1 forza01
```
(misurato: OK; `prima AF=0F0C ... SP=CFEE`, `dopo PC=0CFC AF=0F0C ... SP=CFF0`.)
Una sola cattura: 0D38 resta `tradotta`.

**C. Forzate per i rami non coperti (da decidere con gcov)**. Le catture
naturali non esercitano tutti i rami (nel dasm: 0D41-0D6D, 0EB8, 0F8A-0F99,
0FB0-0FC5, 1101-1108, 1137, 118x ecc. non sono mai eseguiti nelle sessioni
`oggetti` e `finale2`). `SNAP_FORZA` accetta `ix+HH:VV`, `iy+HH:VV` e
`a|b|c|d|e|h|l:VV` all'ingresso. Esempi provati (OK, 1 occorrenza ciascuno,
tag `forza01`):
```
SNAP_FORZA=ix+1d:0x00,d:0x0a sh cattura_oggetti.sh 0x0EAC 1 forza01
SNAP_FORZA=ix+1d:0x01 sh cattura_oggetti.sh 0x0F75 1 forza01
SNAP_FORZA=ix+1d:0x01 sh cattura_oggetti.sh 0x0D3C 1 forza01
SNAP_FORZA=ix+1d:0x01 sh cattura_oggetti.sh 0x10CF 1 forza01
SNAP_FORZA=ix+1d:0x01 sh cattura_oggetti.sh 0x113C 1 forza01
```
Procedura: scritto il C, eseguire la verifica 12 (gcov) e per ogni riga
`#####` raggiungibile cambiando un registro, un byte di (IX+n)/(IY+n) o di
RAM un frame prima, aggiungere una forzata con il tag `forzaNN` successivo
(per `jp (hl)` l'ingresso è il gestore: cambiare D, E, B e (IX+1D),
(IX+1E), (IY+04), (IY+05)). Righe non raggiungibili: spiegarle nel rapporto.
Prevedere 15-30 forzate. Una cattura da forzata non vale per `verificata`.

`finale2` (4500) non serve: `cattura.sh 0x0F75 finale2` e `0x0FA9 finale2`
danno FALLITA (misurato).

## Documenti

- `decomp/ROUTINE.md`: in fondo, sezione `## La chiave: tabella 0D18 (lotto 39)`
  con un paragrafo (moduli, dispatcher, 0CE6 completata, 0D38 forzata, tabelle
  letti con `leggi`) e la tabella standard, una riga per routine (17 più
  l'aggiornamento della riga `0x0CE6` a r. 49: descrizione senza «non
  tradotta: avviso su stderr», catture 4, stati `gioco, tardo, oggetti`).
  Stato `verificata` per chi ha 2 o più catture non forzate (tutte tranne
  0D38, `tradotta`); note «N non forzate e M forzate»; in coda l'elenco delle
  forzate e le righe gcov non coperte con il motivo.
- `decomp/CENSIMENTO-12.md`: r. 193-195 e la riga r. 152: 0CFD, 0E0B e 11A5
  diventano «tradotta nel lotto 39 (`z80_chiave_a.c`/`b.c`, 0E0B dentro 0D8E,
  11A5 dentro 1172)»; correggere «11A5 dentro 113C».
- `STATO.md` e `README.md` non si toccano.

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master`. Prima di modificare qualunque
file: `cd decomp/harness && ./harness --tutte > /tmp/h39_prima.txt`.

1. Disassemblato: `wc -l < decomp/out/chiave_n39.dasm; grep -c 'jp (hl)' decomp/out/chiave_n39.dasm`
   → `845` e `1` (misurate).
2. Catture: tutte le uscite in `/tmp/n39_esito.txt`; `grep -c ' OK$' /tmp/n39_esito.txt`
   ≥ 35 (34 + 1; più le forzate di C) e `grep -c FALLITA /tmp/n39_esito.txt` → `0`;
   `git status --short decomp/out | grep -E 'snap_0x0CE6_(gioco|tardo)_'` vuoto.
3. Compilazione: `cd decomp/harness && make harness replay sched_proto > /tmp/m39.txt 2>&1; echo "make $?"; grep -c -E 'warning|error|undefined' /tmp/m39.txt`
   → `make 0` e `0`.
4. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h39.txt; echo "exit $?"; grep -c FALLITO /tmp/h39.txt; grep -c '^RIEPILOGO' /tmp/h39.txt; grep -c '^cattura' /tmp/h39.txt`
   → `exit 0`, `0`, `304` (287+17, dedotta), e il numero di `^cattura` = 1173 +
   catture nuove (35 + forzate di C; dedotta, riportare il numero vero).
5. `grep '^RIEPILOGO 0x0CE6:' /tmp/h39.txt` → `RIEPILOGO 0x0CE6: 4 catture, 4 OK`;
   `grep -c '0ce6:' /tmp/h39.txt` → `0` (avviso tolto; oggi `0`).
6. Catture per voce: `for a in 0CFD 0D38 0D3C 0D8E 0E1F 0EAC 0EDF 0F6B 0F75 0FA9 100E 109C 10CF 10D9 113C 1172 11D8; do grep "^RIEPILOGO 0x$a:" /tmp/h39.txt; done`
   → ognuna `N catture, N OK` con N ≥ 2 (0D38: 1).
7. Voci precedenti invariate: `diff <(grep '^RIEPILOGO' /tmp/h39_prima.txt) <(grep '^RIEPILOGO' /tmp/h39.txt)`
   → solo `>` (17 nuove) e la sola riga `<`/`>` di `0x0CE6` (da 2 a 4).
8. `cd decomp && harness/sched_proto out/sched_log.txt | tail -1` →
   `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
9. Replay: `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r39.txt 2>&1; echo "exit $?"; tail -4 /tmp/r39.txt` →
   `exit 0` e (misurato):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
10. Una funzione per ingresso: `for a in 0cfd 0d38 0d3c 0d8e 0e1f 0eac 0edf 0f6b 0f75 0fa9 100e 109c 10cf 10d9 113c 1172 11d8; do grep -c -E "^void sub_$a\(struct z80 \*z\)$" decomp/harness/*.c | awk -F: -v a=$a '{s+=$2} END{printf "%s:%d ", a, s}'; done; echo`
    → ogni voce `:1` (attesa, dedotta).
11. Dimensioni: `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'` → vuoto;
    `wc -l decomp/harness/z80_chiave_*.c | awk '$1>650 && $2!="totale" && $2!="total"'` → vuoto.
12. Copertura (gcov, fuori dal repository):
    ```
    D=/tmp/cov39 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
     && ln -s "$PWD/decomp/out" $D/out && (cd $D && gcc -std=c11 -O0 --coverage -o h \
     harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c z80_*.c \
     nemici*.c attivita.c attivita_01_10.c && ./h --tutte > /dev/null; \
     for f in z80_chiave_a z80_chiave_b z80_08xx; do gcov h-$f.gcda > /dev/null 2>&1; \
     echo "== $f"; awk -F: '$1 ~ /#####/' $f.c.gcov; done)
    ```
    (attesa, non misurata) in `z80_chiave_*.c` nessuna riga `#####` tranne i
    casi spiegati nel rapporto (es. il ramo di 0D38 non c'è: solo `return`); in
    `z80_08xx.c` righe scoperte preesistenti da ignorare.
13. Vietati intatti: `git diff --name-only HEAD | grep -E 'snap\.lua|sessioni|replay\.|sched|rst20|rst38_tables|censimento_nemici\.py|STATO\.md|README\.md|compiti/|cattura'` → vuoto.
14. Ambito: `git status --short | grep -v -E '^( M|\?\?|A ) (compiti/39-|decomp/(ROUTINE\.md|CENSIMENTO-12\.md|harness/(z80_(08xx|chiave_a|chiave_b)\.c|routine\.h|routine_tab\.c)|out/(snap_0x(0CE6|0CFD|0D38|0D3C|0D8E|0E1F|0EAC|0EDF|0F6B|0F75|0FA9|100E|109C|10CF|10D9|113C|1172|11D8)_|chiave_n39\.dasm)))'` → vuoto;
    `git status --short | grep -c -E '\.(gcda|gcno)$'` → `0`.
15. Documenti: `grep -c '^## La chiave: tabella 0D18 (lotto 39)$' decomp/ROUTINE.md` → `1`;
    `grep -c 'lotto futuro (1278 B)' decomp/CENSIMENTO-12.md` → `0` (oggi `3`).

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto 39: la chiave (tabella 0D18), 17 voci nel banco, 0CE6 completata`
  e riga finale `Co-Authored-By: <modello> <noreply@anthropic.com>`.
  Comprende `decomp/out/chiave_n39.dasm` e le catture nuove (`git add -f`
  di `decomp/out/snap_<ENTRY>_<tag>_{prima.bin,dopo.bin,regs.txt}` e `.log`).
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-15; voci con
  numero di catture e stato; righe gcov non coperte con il motivo; elenco
  delle forzate aggiunte; deviazioni dal brief.

## Domande aperte

1. **Un solo lotto.** 17 routine, 1279 B, ~770 righe C, ma gestori dello
   stesso schema (stato + posizione sul nibble di D o E, tabelle da 9-10
   byte): nessuna spezzatura. Se lo sviluppatore ritiene troppo lungo il giro
   di catture, il taglio naturale è A (dispatcher + 0D3C-0EDF, 6 routine,
   340 righe) e B (0F6B-11D8 più 0D38, 11 routine, 430 righe). Proposta: un
   lotto solo. — «preferenza»
2. **Due moduli `z80_chiave_a.c` / `z80_chiave_b.c`** (e non nuove righe in
   `z80_08xx.c`, che salirebbe a ~1200). Proposta: come scritto. — «preferenza»
3. **Tabelle dati lette con `leggi()`** dalla ROM e non copiate in C.
   Proposta: come scritto. — «preferenza»
4. **0D38 scrive in ROM (`ld (0),a`)**: in C non si scrive (nessun effetto in
   MAME). Proposta: `return` con commento. — «preferenza»
5. **Stato di 0D38 e delle forzate**: 0D38 `tradotta` (una sola cattura,
   forzata); le forzate di C non contano per `verificata`. Proposta: accettarlo.
   — «preferenza»
6. **Dispatcher con tabella di puntatori a funzione** (non `switch`): una
   voce per indice, stesso ordine della tabella ROM 0D18. Proposta: come
   scritto. — «preferenza»

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Tutte le domande sono preferenze, accolte con la proposta dell'analista:
1. Sì: lotto unico (il taglio naturale indicato resta come ripiego se lo sviluppatore si incaglia).
2. Sì: due moduli nuovi `z80_chiave_a.c` e `z80_chiave_b.c`, senza ingrandire `z80_08xx.c`.
3. Sì: tabelle dati lette con `leggi()` dalla ROM, non copiate in C.
4. Sì: la scrittura in ROM di 0D38 (`ld (0),a`) non si traduce; la funzione fa solo `return` di quel ramo, con commento.
5. Sì: 0D38 resta `tradotta`; le forzate non valgono per `verificata`.
6. Sì: dispatcher con tabella di puntatori a funzione.

## Esito (chat principale, 2026-11-03)

Passato al primo colpo, undicesimo ciclo con lo schema economico
(analista Sonnet ~109k token, sviluppatore Sonnet ~113k, verifica a
script senza modello). Dispatcher 0CFD e 16 gestori della tabella 0D18
in `z80_chiave_a.c` (359 righe) e `z80_chiave_b.c` (418 righe);
`sub_0ce6` completata. Banco a 304 voci e 1270 catture con 0 fallite,
replay a fine oracolo senza divergenze nello spedidore, `sched_proto`
invariato. Deviazione principale: 62 forzate invece delle 15-30
stimate, per coprire riga per riga i rami alti e bassi dei cinque
gestori a tabella; gcov pulito salvo 5 righe di sola etichetta.
16 voci `verificata` (2 catture non forzate l'una), 0D38 `tradotta`
(mai raggiunta in modo naturale, forzata unica; la sua scrittura in
ROM `ld (0),a` non è tradotta per decisione). Censimento corretto:
11A5 cade in 1172, non in 113C.
