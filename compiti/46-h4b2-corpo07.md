# Compito 46: H4b2 — corpo 07 (2CCD), fasi 0-12 e routine 3ADF

Data: 2026-11-03. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Secondo taglio del lotto H4b (`compiti/PIANO-HARDWARE.md`, voce H4; quadro in
`decomp/GIRO.md`, `compiti/44-h4-giro-libero.md`, `compiti/45-h4b-corpi-attract.md`).
Il giro dal boot (`harness/gioco_libero`) oggi si ferma a «CORPO 07 (2CCD) non
tradotto al frame 1188». Qui si traduce il corpo 07 (macchina a fasi sul
contatore c19d) **fino alla fase 12 compresa**: il giro avanza dal frame 1188
al frame 1398 e si ferma sulla prima fase non tradotta (fase 13, gestore 2DCE),
con 0 divergenze nello spedidore. Si traduce anche la routine 3ADF (catturabile,
entra nel banco). Le fasi 13-25 e i corpi 08/0C/0B restano a H4b3 (vedi
«Perimetro e taglio»).

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/`, `compiti/`, `STATO.md`.
- Non si toccano: `harness/replay.c`, `sched.c`, `sched_proto.c`,
  `video_rendi.c`, `video_sdl.c`, `mostra_replay.c`, `rendi_oracolo.c`.
  Banco storico (304 voci, 1270 catture, 0 fallite), replay e `sched_proto`
  restano verdi bit a bit. Nei sorgenti già verificati si possono solo
  aggiungere ganci a puntatore NULL (precedenti: `NEMICI_PUNTO`,
  `attract_punto_largo`, `nemici_r_gancio`).
- `harness/z80_2xxx.c` è a 796 righe e **non si tocca**; `z80_3xxx.c` (564)
  non si accresce oltre le 650. Nessun file di codice sopra le 800 righe;
  soglia di lavoro 650 per i moduli nuovi.
- Commenti, messaggi, documenti in italiano. `gcc` dal Makefile, nessuna
  dipendenza nuova, niente binari né `.o` nel commit.
- MAME solo senza finestra, ingressi dell'host spenti (lo fanno già gli script);
  a fine lavoro `pgrep -c mame` → `0`. Niente git distruttivo.
- **Non si rigenera l'oracolo** (`giro_boot.sh`): `out/giroboot_oracolo.txt`
  (2345 righe F), `out/giroboot_disp.txt`, `out/ingressi_replay.txt` esistono
  già (gitignorati) e sono quelli del lotto H4b1.

## Perimetro e taglio (misurato)

Misura fatta in una copia fuori dal repository: tre tracce MAME con gli
ingressi del replay (moneta 1510, start 1560, casuale da 1900, seme 1),
finestre 1187-1190, 1188-1531, 1531-2411, riunite con
`coverage.py --prefisso`; poi visita delle istruzioni eseguite a partire da
2CCD, con le tre tabelle `rst 20` (2CD2, 2DB8, 2DD9) e **senza entrare nelle
routine già tradotte** (le chiamate a routine del banco o a funzioni `sub_XXXX`
già in C si fermano alla chiamata). Il disassemblato `out/main64.dasm` non
basta: decodifica come codice i byte delle tabelle inline (2CD3-2D06,
2DB9-2DC4, 2DDA-2DED) e perde le istruzioni vere dopo (per esempio 2DC5). Le
istruzioni vere sono nei file `out/titolo_istruzioni.txt` e
`out/stanza_00_istruzioni.txt` (colonne: pc, esecuzioni, testo con operandi
assoluti).

- Il corpo 07 intero esegue **847 istruzioni da tradurre** (non ~690: quella
  stima del brief 45 contava sul disassemblato sfalsato). Il metro di un lotto
  è 503.
- Il contatore c19d vale 0 alla prima spedizione (frame 1188) e avanza di 1
  a ogni spedizione; il corpo gira 3281 volte tra 1188 e 1509 (slot 03; alla
  spedizione 3281 la fase 25 azzera lo slot, a 1509), poi passa all'attività
  08 (scritta nello slot 07 dalla NMI del frame 1531: `W 1531 04AB 07 08`).
  Le fasi si susseguono in ordine; le fasi 0-25 sono tutte nella finestra
  1188-1509; la voce 16 (2D9D) non è mai eseguita (frame 0-2410).
- Il taglio per fasi è pulito: il giro si ferma da solo alla prima fase non
  tradotta. **H4b2 (questo lotto) = fasi 0-12 = 501 istruzioni** (4 di
  ingresso + 497), arresto alla fase 13 (frame 1398). **H4b3 = fasi 13-25
  (346) + corpi 08, 0C, 0B (134) + confronto al 2410 = 480.**

Tabella delle fasi del lotto (T1 = `rst 20` a 2CD2, T2 = 2DB8, T3 = 2DD9;
istruzioni «nuove» = non già contate dalle fasi precedenti, in ordine di tempo):

| fase | gestore | primo frame | istr. nuove | note |
|---|---|---:|---:|---|
| ingresso | 2CCD-2CD2 | 1188 | 4 | `hl=c19d; a=(hl); inc (hl); rst 20` |
| T1[0] | 3861 | 1188 | 125 | blocchi 3861-38B7, 38C3-38FF, 3931-39A9 (38C3 non è una routine: è un blocco della fase) |
| T1[1] | 34FA | 1188 | 30 | 34FA-351C, 352E-3539, 3547-354A |
| T1[2] | 2DEE | 1189 | 7 | 2DEE-2E0E in parte |
| T1[3] | 37D7 | 1189 | 24 | 37D7-3809 (la coda 37B4-37D6 è la routine 37B4, chiamata solo dalla fase 18: H4b3) |
| T1[4] | 358F | 1189 | 142 | 358F-365D, 3667-366B; la `call z,366C` (3657) non è mai eseguita |
| T1[5] | 2D07 | 1189 | 7 | 2D07-2D0D, 2D19-2D1E; vedi «Registro R» |
| T1[6] | 3225 | 1189 | 3 | chiama 322A (routine del banco) |
| T1[7] | 3A7D | 1195 | 63 | 3A7D-3AF2: 51 + routine 3ADF (12 istruzioni, chiamata a 3A82 e 3AC8) |
| T1[8] | 2E89 | 1195 | 4 | attesa: 1342 spedizioni (frame 1195-1291) |
| T1[9] | 2F2D | 1291 | 17 | |
| T1[10] | 2E5D | 1291 | 2 | attesa: 169 spedizioni |
| T1[11] | 2DFA | 1304 | 8 | |
| T1[12] + T2 | 2DAD | 1304 | 65 | `rst 20` a 2DB8 (6 voci: 2FC1, 2E79, 2FA9, 2E71, 2F8F, 2E7D), c19e fa da sotto-contatore; 2E79/2E71/2E7D sono attese (812, 197, 147 spedizioni) |

Somma 501. Le voci 13-25 di T1 (e la 16, mai eseguita) **non** si traducono
qui: nel `switch` hanno `nemici_manca(<gestore>)` (elenco in «Fasi per H4b3»).

Fatti sulle altre routine (misurati):
- 2D8A e 3110 (citate nel brief 45) **non** sono raggiunte dal corpo 07: 2D8A è
  chiamata da 0B (2C34) e da 6455, 6A15; 3110 è un salto di 7AF2 (corpo 06,
  fase 5). Fuori perimetro.
- 37B4 (secondo chiamante di 4767, N7) è chiamata a 374B dalla fase 18 (3709):
  H4b3. 38C3: vedi tabella.

## Cosa esiste già (fatti verificati)

| file | righe | ruolo |
|---|---:|---|
| `harness/attivita_attract.c` | 223 | corpi 1F e 06; da copiare: `posta()` (0493 con A = codice), `PUNTO_LARGO` (macro locale su `attract_punto_largo`, dichiarato in `nemici.h` r. 74), `fase_760b` (stile dei `NEMICI_PUNTO` con gli indirizzi a margine), `attivita_06_75e7` (`switch` sulla fase, `nemici_manca` per le fasi non tradotte) |
| `harness/attivita.c` | 354 | `corpo_attivita_replay[SCHED_CODICI]` (r. 339-354): aggiungere `[0x07] = attivita_07_2ccd` |
| `harness/attivita_01_10.c` | 75 | regole `pop hl` dei corpi: `pop hl; ld (hl),n; ret` = `tabella[slot] = n; return`; `pop hl; ret` = `return` (HL è il puntatore allo slot) |
| `harness/nemici.h` | 358 | `NEMICI_PUNTO(da, a)` r. 57-62; prototipi dei corpi r. 66-72; `nemici_manca(uint16_t)`, `nemici_fuori_tabella(sito, idx)` r. 22-25; `nemici_r_somma()` r. 338 |
| `harness/z80_38xx.c` | 282 | qui va `sub_3adf` (include `routine.h`; stile di `sub_3a1c`: commento con ingresso/uscita) |
| `harness/routine.h`, `routine_tab.c` (314) | | prototipi `void sub_xxxx(struct z80 *z);` e voci `{ "0x3A2E", sub_3a2e, 0, 0 },` in ordine di indirizzo |
| `harness/macchina.c` | 590 | NON si tocca: la tabella dei corpi viene da `corpo_attivita_replay` via `involucro` |
| `harness/Makefile` | 81 | r. 7: `SORGENTI_NEMICI = $(wildcard nemici*.c) attivita.c attivita_01_10.c attivita_attract.c`: aggiungere i moduli nuovi (non rientrano nel wildcard) |
| `decomp/ROUTINE.md` | 828 | tabella delle routine: una riga per ingresso (formato: ingresso, funzione, file, byte, descrizione, stato, catture OK, stati, note) |

Esito di diagnostica del giro (da `macchina.c`, r. 538-549): `nemici_manca(x)`
→ dopo la spedizione stampa `ROUTINE %04X mancante (motore dei nemici) al
frame %ld` e termina con codice 1 (la spedizione conta già in «spedizioni
eseguite»); `nemici_fuori_tabella` → `INDICE %02X fuori dalla tabella rst del
sito %04X al frame %ld`.

Routine già tradotte che il lotto chiama (misurate dalla visita; tutte
`void sub_xxxx(struct z80 *z)`; file): 240B, 2457, 2E11, 2E90, 2F57, 2FF3
(z80_2xxx.c); 31CE, 31E9, 322A, 349F, 354B, 3A54 (z80_3xxx.c); 380A, 3812,
3A1C, 3F10, 3F88 (z80_38xx.c); 4AF6, 4B0E, 4D87 (z80_4xxx.c); 6771
(z80_6xxx.c); `rst 08`, `rst 10`, `rst 28` = `sub_0008`, `sub_0010`,
`sub_0028`. Verificare ogni chiamata con `grep -n 'sub_xxxx' harness/routine.h`.

Routine da tradurre nel banco: **3ADF** (12 istruzioni, `ld bc,2004; ld (hl),c;
res 2,h; ld (hl),e; set 2,h; inc hl; inc c; bit 3,c; jr z; ld c,4; djnz; ret`),
catture misurate (copia fuori dal repository, `cattura.sh` invariato):
- `sh cattura.sh 0x3ADF titolo` → `CATTURA 0x3ADF titolo frame_da=1000 OK`
  (occorrenza 1 al frame 1196; prima PC=3ADF AF=0728 BC=0700 DE=3A51
  HL=D460; dopo PC=3A85 AF=0718 BC=0004 DE=3A51 HL=D480);
- `sh cattura.sh 0x3ADF titolo 2 n2` → `CATTURA 0x3ADF titolo_n2 frame_da=1000
  OK` (occorrenza 2 al frame 1197).
Provati e **non catturabili** (tutte le occorrenze scartate per NMI): i
gestori 2D07, 2DEE, 3709, 3A7D con stato `titolo`; `stanza00` per 37B4 e
3ADF fallisce. I gestori di fase si verificano col giro.

### Finestra 1188-1398 (misurata su `out/giroboot_disp.txt`)

- Spedizioni: la prima spedizione di 07 è a riga D numero 16408; dopo 1188 solo
  14 (slot 02) e 07 (slot 03). La prima fase 13 (2DCE) è la 2682-esima
  spedizione di 07, frame 1398, con 21769 righe D prima di lei.
- NMI frame 1188-1398: 211 (righe N), di cui 131 con ritorno in
  3AF3-3B10 (spedidore), 31 in rst/helper 0008-0026, 16 in 2C27-3AF2 e 33
  altrove (corpo 14 tradotto 44B8-4517, una in 75D3); 117 con letture > 0.
  Ritorni in 2C27-3AF2: 2CCD×2 2CD0×1 2E8E×2 2E8F×1 2E90×4 (ingresso
  del corpo e routine 2E90 del banco) e 32AC 337F 3391 339C 33A2 33B0 (dentro 322A,
  già coperti dal `PUNTO_LARGO(0x3225, 0x33b9)` di `attivita_attract.c`
  r. 139). Le NMI con ritorno 44xx sono già gestite (corpo 14).
- L'oracolo è quello del boot: ritorno e impronta per ogni frame (righe F),
  nessuna lettura del registro R.

### Registro R

Il gestore 2D07 (fase 5) fa: `pop hl; ld a,(c243); cp 1D; jr nz,2D19` e a
2D19 `ld a,(c045); and 10; ret z`. Nella partita registrata c243 ≠ 1D e il bit
4 di c045 è spento (2D0F-2D18 e 2D1F-2D49 non vengono eseguite). 2D1F è uno
dei siti `ld a,r` della sentinella (brief 45: 0 letture di R nei frame
0-5499). Nel C: dove il bit 4 di c045 sarebbe acceso, chiamare
`nemici_r_somma()` (la sentinella `nemici_r_gancio` ferma il giro con
`R letto prima della NMI del frame N`) e poi `nemici_manca(0x2d1f)`.

## Struttura da realizzare

Tre moduli nuovi (nomi a scelta libera solo se sopra 650 righe; stile di
`attivita_attract.c`: commento di testa con le regole `pop hl`, gli indirizzi
originali a margine, un solo `struct z80 z = {0}` per fase):

1. **`harness/attivita_attract07.h`** (nuovo, ~25 righe): `PUNTO_LARGO` (copia
   della macro di `attivita_attract.c` r. 31-32) e i prototipi
   `void fase_07_XXXX(uint8_t slot);` delle fasi dei moduli `a` e `b`.
2. **`harness/attivita_attract07.c`** (~300 righe): `void
   attivita_07_2ccd(uint8_t slot)` (prototipo in `nemici.h` accanto ai corpi 1F
   e 06): `NEMICI_PUNTO` per l'ingresso (2CCD-2CD2, prima e dopo `inc (c19d)`),
   `fase = M(0xc19d); M(0xc19d) = fase + 1;` e `switch`: fasi 2, 5, 6, 8, 9, 10,
   11, 12 (con le sotto-fasi T2 sul contatore c19e) tradotte qui; fasi 0, 1, 3 →
   chiamate al modulo `a`; fasi 4, 7 → modulo `b`; fasi 13-25 e voce 16 (2D9D) →
   `nemici_manca(<gestore>)` (gestori in tabella sopra e in «Fasi per H4b3»);
   `default` → `nemici_fuori_tabella(0x2cd2, fase)`. Le attese (2E89, 2E5D, 2E79,
   2E71, 2E7D) hanno i loro `NEMICI_PUNTO` perché sono ~2900 spedizioni su 3281.
3. **`harness/attivita_attract07a.c`** (~300 righe): fasi 0 (3861), 1 (34FA),
   3 (37D7).
4. **`harness/attivita_attract07b.c`** (~300 righe): fasi 4 (358F, con il suo
   ciclo 3642-3668) e 7 (3A7D, chiama `sub_3adf`).
5. **`harness/z80_38xx.c`**: `void sub_3adf(struct z80 *z)` (z80_38xx.c resta
   sotto le 320 righe); prototipo in `routine.h` (vicino a `sub_3a2e`, r. 42);
   voce `{ "0x3ADF", sub_3adf, 0, 0 },` in `routine_tab.c` in ordine di
   indirizzo; riga in `ROUTINE.md` (verificata, 2 catture, stati `titolo`,
   nota: occorrenze 1 e 2 del titolo, frame 1196 e 1197, il banco la
   scrive via HL; scrive).
6. **`harness/attivita.c`**: in `corpo_attivita_replay` aggiungere `[0x07] =
   attivita_07_2ccd,`; commento di testa. **`Makefile`**: aggiungere
   `attivita_attract07.c attivita_attract07a.c attivita_attract07b.c` a
   `SORGENTI_NEMICI`.
7. **Regole di traduzione**: si traduce solo ciò che è stato eseguito nei
   frame 0-2410 (istruzioni dei file `*_istruzioni.txt`); i rami mai eseguiti
   (per esempio 2D0F-2D18, la `call z,366C` a 3657) → `nemici_manca(<indirizzo>)`
   o commento «mai eseguito» con `nemici_manca`, come le fasi 5 e 8 di 06.
   Per ogni scrittura in RAM e ogni chiamata a routine tradotta un
   `NEMICI_PUNTO(da, a)` secondo la regola di `nemici.h` e del brief 45
   («[da, a] = dalla prima istruzione dopo l'ultima scrittura fino
   all'istruzione che scrive inclusa»); nei cicli (3642-3668, `ldir`, `djnz`)
   il punto va dentro il ciclo. Se una NMI pulita diverge, leggere la
   diagnostica (ritorno, hash C e hash MAME), aggiungere il punto mancante
   o un `PUNTO_LARGO` per un ritorno dentro una routine tradotta senza punti;
   NON si allenta l'impronta. Le NMI a metà attività divergenti non fermano il
   giro (si contano), come nel replay.
8. **Documenti**: `decomp/GIRO.md` (sezione H4b2: fasi tradotte, punti,
   arresto), `STATO.md` (una voce H4b2; i numeri del banco 304/1270 →
   305/1272 dove compaiono: `grep -n -E '\b304\b|\b1270\b' STATO.md README.md
   decomp/ROUTINE.md`, solo le righe che dicono «banco»), `compiti/PIANO-HARDWARE.md`
   (H4b2 fatto; H4b3 = fasi 13-25 di 07, 08, 0C, 0B e confronto al 2410),
   `decomp/ROUTINE.md` (riga 3ADF e totale).

### Fasi per H4b3 (solo `nemici_manca` qui)

Gestori da mettere nel `switch` come `nemici_manca`: T1[13] 2DCE (con T3 a
2DD9: 3009, 305F, 3131, 2E65, 3161, 2E81, 31B7), 14 31C5, 15 2E0A, 17 e 21 2E9C,
18 3709 (chiama 37B4 a 374B), 19 3829, 20 321E, 22 2EF5, 23 2E6D, 24 2F1A,
25 2E1E (azzera lo slot a 2E58); la voce 16 (2D9D) non è mai eseguita. Il
`nemici_manca` della **fase 13** (2DCE) è quello che ferma il giro all'arresto.

## Verifiche obbligatorie

Da `decomp/` salvo diversa indicazione. (m) = misurato prima di scrivere il
brief; (d) = dedotto dal registro (`out/giroboot_disp.txt`), non misurabile
finché il C non esiste; (a) = attesa, non misurata. Riportare esatto ciò che
esce.

1. `cd harness && make -B all gioco_libero 2>&1 | grep -c -i -E 'warning|error'`
   → `0`.
2. `sh cattura.sh 0x3ADF titolo; sh cattura.sh 0x3ADF titolo 2 n2` →
   `CATTURA 0x3ADF titolo frame_da=1000 OK` e `CATTURA 0x3ADF titolo_n2
   frame_da=1000 OK` (m).
3. `cd harness && ./harness 0x3ADF | tail -1` → `RIEPILOGO 0x3ADF: 2 catture, 2
   OK` (a).
4. Giro (da `decomp/`; richiede `out/giroboot_oracolo.txt`):
   `harness/gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt
   out/ingressi_replay.txt; echo "exit $?"` →
   - `SANITA 602: ... coincide` (m, come H4a);
   - `ROUTINE 2DCE mancante (motore dei nemici) al frame 1398` (d: 2DCE è la
     prima fase 13, 2682-esima spedizione di 07, frame 1398);
   - `spedizioni eseguite: 21770` (d: 21769 righe D prima di lei più la
     spedizione che ferma il giro);
   - `NMI confrontate: 1333, divergenti: <M>` (d: frame 66-1398, una NMI per
     frame; il numero può superare 1333 di poche unità per la coda di N dopo
     la D; M a metà attività è accettabile);
   - `dopo il boot: nello spedidore <P> (divergenti 0), a metà attività <Q>
     (divergenti <Y>)` con P + Q = 796 (d: 1333 meno le 537 del boot) e
     **divergenti nello spedidore = 0** (condizione dura);
   - nessuna riga `R letto` né `DIVERGENZA` pulita; `exit 1`.
   Se il giro si ferma prima (altro `ROUTINE`/`INDICE`/`DIVERGENZA`), si
   corregge e si riporta la sequenza degli arresti intermedi nel rapporto.
5. Ingressi e oracolo invariati: `grep -c '^F ' out/giroboot_oracolo.txt` →
   `2345` (m); `sha1sum out/ingressi_replay.txt` →
   `43934a8eeed8d44a071d7ab958d32ea74fbd437f` (m).
6. Sentinella: `grep -c 'nemici_r_somma' harness/attivita_attract07.c` → `1` o
   più (a).
7. `pgrep -c mame` → `0`.
8. Banco: `cd harness && ./harness --tutte > /tmp/h46.txt; echo "exit $?"; grep
   -c FALLITO /tmp/h46.txt; grep -c '^RIEPILOGO' /tmp/h46.txt; grep -c
   '^cattura' /tmp/h46.txt` → `exit 0`, `0`, `305`, `1272` (a: 304 + 1 voce,
   1270 + 2 catture).
9. Replay storico: `harness/replay out/replay_oracolo.txt out/replay_disp.txt
   out/replay_2410.bin 2410 > /tmp/r46.txt 2>&1; echo "exit $?"; tail -4
   /tmp/r46.txt` → `exit 0` e le quattro righe del brief 45 (m):
   `spedizioni eseguite: 64686, fino al frame 5499` / `NMI confrontate: 3090
   (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0,
   a metà attività 885` / `prima divergenza a metà attività (attesa): frame
   2415` / `nessuna divergenza nello spedidore`.
10. `harness/sched_proto out/sched_log.txt | tail -1` → `spedizioni confrontate:
    106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (m).
11. `sh rendi_video.sh; echo "exit $?"` → `RENDI 7/7 identici`, `exit 0` (m).
12. `git diff --stat -- decomp/harness/replay.c decomp/harness/sched.c
    decomp/harness/sched_proto.c decomp/harness/video_rendi.c
    decomp/harness/video_sdl.c decomp/harness/mostra_replay.c
    decomp/harness/rendi_oracolo.c decomp/harness/z80_2xxx.c
    decomp/harness/macchina.c` (dalla radice) → vuoto.
13. `wc -l harness/*.c | awk '$1>800 && $2!="totale"'` → vuoto;
    `wc -l harness/attivita_attract07*.c harness/z80_38xx.c` → ciascuno sotto
    650.
14. Ambito, `git status --short` dalla radice → solo: `decomp/harness/attivita_attract07.h`,
    `attivita_attract07.c`, `attivita_attract07a.c`, `attivita_attract07b.c`,
    `attivita.c`, `nemici.h`, `z80_38xx.c`, `routine.h`, `routine_tab.c`,
    `Makefile`, `decomp/GIRO.md`, `decomp/ROUTINE.md`, `STATO.md`,
    `compiti/PIANO-HARDWARE.md`, `compiti/46-h4b2-corpo07.md`; `git status
    --short | grep -c -E 'gioco_libero$|\.o$|\.bin$'` → `0`.

## Consegna

Commit unico «Lotto H4b2: corpo 07 (fasi 0-12) e routine 3ADF» con riga
finale `Co-Authored-By: <modello vero> <noreply@anthropic.com>`. Niente push.
In fondo a questo brief: esito e uscita delle verifiche. Rapporto finale:
massimo trenta righe con l'uscita delle 14 verifiche, i punti `NEMICI_PUNTO` e
`PUNTO_LARGO` aggiunti con i ritorni che coprono, il numero di NMI pulite e a
metà attività, le istruzioni davvero tradotte e le deviazioni.

## Domande aperte

1. **Scelta di progetto — taglio di H4b2/H4b3.** Il corpo 07 conta 847
   istruzioni (846 fino all'arresto al corpo 08, frame 1531): oltre il metro
   di 503 (+68%). Proposta (questo brief): H4b2 = fasi 0-12 (501, arresto
   `ROUTINE 2DCE mancante ... frame 1398`); H4b3 = fasi 13-25 (346) + 08, 0C, 0B
   (134) + confronto al 2410 = 480. Alternativa: lotto unico di 846 con arresto
   `CORPO 08 (7C98) non tradotto al frame 1531`, spedizioni 23359 e NMI 1466
   (d); in tal caso il brief resta valido e lo sviluppatore prosegue con le fasi
   13-25 (tabella «Fasi per H4b3», moduli `attivita_attract07c.c`/`d.c`, 37B4
   e 4767 in z80_38xx.c).
2. **Preferenza — moduli e nomi.** `attivita_attract07.c/.h`, `07a`, `07b`,
   `sub_3adf` in `z80_38xx.c`, `PUNTO_LARGO` copiato nell'header nuovo.
   Proposta: così (z80_3xxx.c a 564 e z80_2xxx.c a 796 non ci stanno).
3. **Preferenza — rami mai eseguiti.** Proposta: `nemici_manca(<indirizzo>)`
   come per le fasi 5 e 8 del corpo 06; il registro R, se letto, passa dalla
   sentinella `nemici_r_somma()`.
4. **Preferenza — catture di 3ADF.** Le due occorrenze (frame 1196 e 1197)
   sono adiacenti; non sono state trovate altre catture in `stanza00`.
   Proposta: accettarle come due catture (due esecuzioni diverse, registri
   diversi); se lo sviluppatore ne trova una terza con altro stato (`gioco`,
   `porta`), aggiungerla.
5. **Preferenza — arresto e conteggi dedotti.** Le uscite 21770 / 1333 / 796 /
   frame 1398 sono dedotte dal registro dello spedidore e dalle tracce, non da
   una corsa del C. Proposta: se il giro si ferma altrove, vale la sequenza
   degli arresti riportata nel rapporto, non la riga della verifica 4.

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

1. (Scelta di progetto, risolta dalla chat principale col criterio delle
   spezzature già ratificate — il metro di 503 istruzioni per lotto; da
   segnalare all'utente:) sì, H4b2 = fasi 0-12 del corpo 07 (501
   istruzioni più 3ADF), arresto atteso «ROUTINE 2DCE mancante al frame
   1398»; H4b3 = fasi 13-25 (346) più i corpi 08, 0C e 0B (134) più il
   confronto byte per byte al frame 2410 (480 in tutto).
Preferenze 2-5, accolte con la proposta:
2. Sì: moduli `attivita_attract07.c/.h` (eventuali `07a`/`07b` oltre le
   soglie), `sub_3adf` in `z80_38xx.c`.
3. Sì: rami mai eseguiti con `nemici_manca`, come per le fasi 5 e 8 del
   corpo 06.
4. Sì: le due catture adiacenti di 3ADF (frame 1196 e 1197) valgono come
   due catture.
5. Sì: le uscite dedotte (21770 spedizioni, 1333 NMI, frame 1398) si
   accettano come attese non misurate, con la sequenza degli arresti nel
   rapporto dello sviluppatore.

## Esito (sviluppatore, 2026-11-03)

Tutte le 14 verifiche passate. Giro: `ROUTINE 2DCE mancante (motore dei nemici)
al frame 1398`, spedizioni 21770, NMI 1333 (divergenti 15), dopo il boot
nello spedidore 615 (divergenti 0), a metà attività 181 (divergenti 15);
catture 3ADF 2/2 OK; banco exit 0, 0 fallite, 305 voci, 1272 catture; replay,
sched_proto e rendi invariati; pgrep mame 0. Deviazione: la fase 6 usa
`fondale_322a()` (copia di sub_322a con PUNTO_LARGO prima di sub_33ba), perché
con sub_322a intera la palette partiva sei frame in anticipo (divergenza al
frame 1196). README.md (riga banco 304/1270) non toccato: fuori dall'ambito.

