# Compito 11: lotto 7 — il grappolo 0602 (motore per-oggetto della NMI)

Data: 2026-10-21. Schema, convenzioni, vincoli e strumenti: brief 05 («Schema
dei lotti»), 06-10. MAME `/usr/games/mame` 0.264 con `PATH=$PATH:/usr/games`;
nessuna operazione git; non toccare `roms/` né `.gitignore`; nessun file
sopra le 800 righe; nomi `sub_xxxx`. Le routine «foglia» sono esaurite
(chiusura del brief 10): questo lotto apre il primo blocco con chiamate a
routine non tradotte.

## Cosa esiste già (misurato oggi, 2026-10-21, dopo il lotto 6)

VM nuova: dati derivati rigenerati oggi con gli script del repo (ROM
estratte in `roms/ext/solomon/`, md5 di `main48.bin`/`main64.bin`
`daaf50c3…`/`0bca296a…` come nel brief 07; z80dasm 1.1.6; `main48.dasm`
35732 righe; traccia di controllo 1900 frame → `istruzioni.txt`,
`chiamate.txt`, `callgraph.csv`; sessioni `titolo`/`stanza_00`/`gameover` →
`out/<sessione>_*`; `sched_log.txt` 5500 frame). Banco: 66 routine,
`--tutte` → 66 RIEPILOGO, 0 FALLITO; `sched_proto` →
`spedizioni confrontate: 106095, differenze: 0; posta_attivita: 131, slot
coincidenti: 131`; ROUTINE.md 66 righe (63 `verificata`, 3 `tradotta`:
3678, 36C3, 7DEA); 127 catture `regs` con tag.

## Mappa dei grappoli (destinazioni non tradotte chiamate dalle candidate)

Per ogni routine non tradotta chiamata da candidate eseguite, che cosa
sbloccherebbe la sua traduzione (dal `callgraph.csv` e dal dasm di oggi):

| callee non tradotta | chiamanti candidate | sbloccherebbe | giudizio |
|---|---|---|---|
| 08CE (smistatore con tabella inline) | 08A2 ← 0602 | 0602 (44040 inv., motore per-oggetto della NMI, sito 0126), 08A2 e i gestori d'urto 08D8-0CE5: ~900 byte eseguiti, nessuna porta/rst in tutto il blocco | **SCELTO**: sblocca di più, rischio minore (blocco autosufficiente) |
| 0469 | corpo statico di 03FB (eseguito solo il `ret c` a 0404, 6 volte) | 03FB | bloccato: a 0465 chiama 0493 (scheduler nel banco) |
| 23BB / 23EC / 336C | 325E, 326C | nulla di nuovo: C già scritto come statiche in `fondale.c` | nessun lavoro |
| 3569 | 2F57, 354B, 3F88 | — | porte (filone «porte come funzioni di piattaforma») |
| 342D ← 33EB; 2C1A/2BC3 ← 2BC9/29CF; 2E90 ← 2E11; 3812 ← 37B4; 2AE2 ← 2A03; 7C84 ← 797E | — | — | porte, come sopra |
| 0493, 0038, 0028 | 01D5/0219, 4F0F/5507/6C2C/7ECA/7F2D, 04AC | — | filone strutturale (scheduler, rst) |
| 7E03, 7E14 | — | — | mai eseguite (filone «zone mai eseguite») |

Candidate singole senza callee non tradotte ma ancora escluse (ragioni già
nei brief 05-10, confermate oggi): 027C (`jp nc,030B` fuori estensione),
0CE6 e 1288 (il corpo statico prosegue in zone mai eseguite), 322A/2A03
(`ex (sp),hl`), 234A (entra a metà routine), 7379 (`jp 2540` fuori
estensione), 4767 (rami non eseguiti con 3569/porte e `rst 38`), 05B5
(brief precedenti), 03FB (sopra).

## Il grappolo scelto (fatti verificati dal disassembly, oggi)

- La NMI, a 011D-0131, scorre 24 slot oggetto da 0x30 byte a partire da
  c46e: per ogni slot `call 0602` con HL = IX = slot (unico sito: 0126).
- **0602-07DD** (476 byte): se (HL) < C0 esce subito (`ret c` a 0605,
  maggioranza dei casi); altrimenti (IX+6) = 4·(IX+1D)+(IX+1E); aggiorna il
  sottorecord a HL+4 (due coppie valore/bersaglio: copia del bersaglio con
  ricerca nella tabella ROM a746, oppure, se già uguali, integrazione a
  16 bit con saturazione verso il bersaglio, ramo 06A7); a 072B copia 4 byte
  dal record ROM 9495+8·A (A = (IX+4) se < 9, altrimenti (IX+3)) in (DE);
  raccoglie in C i bit di 6 celle vicine della griglia c0bd attorno al pixel
  ((IX+0E), (IX+15)) usando i margini del riquadro (IX+16..19) e i bit 0-2
  di C dai cambi di cella; scrive C & 3F in (HL); `call 08A2`; **prosegue
  senza `ret` in 07DE** = `sub_07de` già `verificata` (il C chiama
  `sub_07de(z)` in coda).
- **08A2-08AD** (12 byte): se bit 5 di (IX+0) spento → `ret z`; altrimenti
  E = C & 0F e `call 08CE`. Ai byte **08AE-08CD non c'è codice ma la
  tabella inline di 16 parole** (il dasm lì decodifica spazzatura):
  08D8 08ED 094D 0A98 0A24 0CDA 0B57 0BBB 09B3 0AF9 0CCE 0BF6 0AC3 0C2D
  0C7C 08D8 (indici 0-F).
- **08CE-08D7** (10 byte): smistatore: `pop hl` (l'indirizzo di rientro =
  la tabella dopo la `call`), HL += 2·E, `push` della parola letta, `ret`
  → salta al gestore. Il gestore ritorna direttamente al chiamante di 08A2.
- **Gestori d'urto 08D8-0CE5**: 15 routine contigue che correggono la
  posizione/stato dell'oggetto IX (allineano (IX+0E) o (IX+15) alla
  griglia coi margini (IX+16..19), azzerano i sottopixel (IX+0D)/(IX+12..14),
  impostano stato (IX+1D), direzione (IX+1E), (IX+2)/(IX+7)/(IX+0B)/(IX+0C)).
  Nessuna `call`/`rst`/porta in tutto 08D8-0CE5 (verificato sul dasm);
  0CCE salta a 0BF6/0C7C e 0CDA a 0BBB/0C2D (in C: chiamata all'altro
  gestore). Eseguiti nelle tracce di oggi: 08D8 (E=0/F), 094D (E=2),
  0A98 (E=3), 0AC3 (E=C), 0AF9 (E=9), 0B57 (E=6). Mai eseguiti: 08ED,
  09B3, 0A24, 0BBB, 0BF6, 0C2D, 0C7C, 0CCE, 0CDA.
- L'unico riferimento esterno dentro il blocco è `ld hl,0842` a 1BEB
  (zona mai eseguita 12AB-2349): carica una costante, non salta.

## Routine del lotto 7 (8 registrate + 1 statica)

t/s/g: 0602 e 08A2 dalle `*_chiamate.txt`; gestori dal conteggio del PC
d'ingresso nelle `*_istruzioni.txt` (non sono destinazioni di `call`).

| ingresso | estensione | byte | t / s / g | come si arriva | effetto (da dasm) | catture |
|---|---|---:|---|---|---|---|
| 0602 | 0602-07DD | 476 | 34440 / 16800 / 16800 | 0126 (ciclo oggetti NMI) | v. sopra; coda in `sub_07de` | titolo, gioco, entrambe con `SNAP_COND='b@hl>=c0'` (la prima occorrenza libera è il `ret c` banale) |
| 08A2 | 08A2-08AD (+tabella 08AE-08CD) | 12 | 1271 / 3087 / 3389 | 07D9 (da 0602) | smistamento d'urto: bit 5 di (IX+0), E = C&0F, gestore dalla tabella 08AE | titolo (ramo `ret z`), gioco con `SNAP_COND='(b@ix & 20)==20'` suffisso `attiva` |
| (08CE) | 08CE-08D7 | 10 | — (553/869 in s/g via 08AB) | 08AB | smistatore `pop hl`/tabella inline: **statica in C, non registrabile** (nessun rientro osservabile per snap.lua) | nessuna |
| 08D8 | 08D8-08EC | 21 | 0 / 480 / 758 | tab. 08AE, E=0/F | se 3 ≤ (IX+1D) < 7, ≠ 6 e C&30 = 0 → (IX+1D) = 6 | gioco, tardo |
| 094D | 094D-09B2 | 102 | 0 / 1 / 0 | tab. 08AE, E=2 | spinge l'oggetto fuori dalla cella sul lato più vicino: corregge (IX+15) o (IX+0E), azzera i sottopixel, stato 2 o 1/2 da bit 7 di (IX+0C), aggiorna (IX+1E) | gioco, gioco N=2 |
| 0A98 | 0A98-0AC2 | 43 | 0 / 2 / 0 | tab. 08AE, E=3 | appoggio: (IX+0E) alla base cella − riquadro, (IX+0D)=0; se (IX+1D) ≠ 2: stato 1, (IX+2)=0, `res 1,(IX+1E)` | gioco, gioco N=2 |
| 0AC3 | 0AC3-0AF8 | 54 | 0 / 64 / 110 | tab. 08AE, E=C | urto verso l'alto: (IX+0E) alla cella sotto, (IX+0D)=F0; se stato−4 ∉ {2,3}: stato 1, `set 1,(IX+1E)`, (IX+2)=0; altrimenti azzera (IX+0B)/(IX+0C) | gioco, tardo |
| 0AF9 | 0AF9-0B56 | 94 | 0 / 2 / 1 | tab. 08AE, E=9 | urto laterale: (IX+15) allineato − (IX+17), (IX+14)=0; secondo lo stato: stato 2 con (IX+1E) da bit 7 di (IX+0C) e azzeramenti, oppure (IX+7)=FF e azzera 12/13/0B/0C | gioco, tardo; se tardo fallisce gioco N=2 |
| 0B57 | 0B57-0BBA | 100 | 0 / 4 / 0 | tab. 08AE, E=6 | come 0AF9 sul lato opposto (riquadri (IX+17)+(IX+18)), direzioni speculari | gioco, gioco N=2 |

Totale tradotto ~912 byte. Il disassembly si legge come nei brief
precedenti, per esempio da `decomp/`:
`sed -n "$(grep -n ';0602$' out/main48.dasm | cut -d: -f1),+400p" out/main48.dasm`
e le istruzioni eseguite con
`awk -F'\t' '$1>="0602" && $1<="08CD"' out/istruzioni.txt`.

## Struttura

- `z80_0xxx.c` (oggi 221 righe): `sub_0602` in fondo. Se il file superasse
  600 righe, spezzare secondo la regola del brief 05 (`z80_00xx.c` /
  `z80_08xx.c`) SENZA riordinare le funzioni esistenti; stima: resta sotto.
- `z80_08xx.c` (nuovo): `sub_08a2`, lo smistatore statico e i 6 gestori
  `sub_08d8`, `sub_094d`, `sub_0a98`, `sub_0ac3`, `sub_0af9`, `sub_0b57`.
  Lo smistamento in C: `static void (*const gestori_08ae[16])(struct z80 *)`
  in corrispondenza della tabella ROM 08AE (commento con gli indirizzi);
  gli indici i cui gestori non sono tradotti valgono `NULL` → nessuna
  modifica dello stato (un'eventuale cattura futura fallirebbe il confronto
  in modo visibile, mai in silenzio). In testa a `sub_08a2` un commento
  documenta il meccanismo di 08CE (tabella inline dopo la `call`).
- `sub_0602` termina chiamando `sub_08a2(z)` (sito 07D9) e poi `sub_07de(z)`
  (prosecuzione a 07DE); dentro 0602 le letture ROM con `leggi()`.
- `routine.h`: blocco «Lotto 7 (compito 11)» con gli 8 prototipi;
  `routine_tab.c`: 8 voci, tutte `(0, 0)` (scritture IX-relative).
- Catture come da colonna; regole di qualità del brief 05 §1 (le `prima`
  devono differire; almeno una cattura non banale per chi scrive RAM).
  Per i gestori la cattura parte dal PC d'ingresso (snap.lua legge il
  rientro da (SP): vale anche per il salto via `push`/`ret`).
- `ROUTINE.md`: 8 righe nuove (08CE citato nella descrizione di 08A2,
  senza riga propria, come le callee statiche di `fondale.c`).

## Verifiche obbligatorie (da `decomp/`)

1. `make -C harness 2>&1 | grep -ciE 'warning|error'` → `0` (attesa).
2. Per ognuna delle 8: `(cd harness && ./harness 0xXXXX | tail -1)` →
   `RIEPILOGO 0xXXXX: 2 catture, 2 OK`, uscita 0 (attesa; 1 cattura se la
   routine resta `tradotta` con nota, regola del brief 06 domanda 2).
3. `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `74`;
   `grep -c FALLITO` → `0` (attesa).
4. `(cd harness && ./sched_proto ../out/sched_log.txt | tail -1)` →
   `spedizioni confrontate: 106095, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
5. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale" && $2!="total"'` → vuoto (misurato: vuoto).
6. `grep -c '^| 0x' ROUTINE.md` → `74`; `grep -c '| verificata |' ROUTINE.md`
   → `71` attesa (63 + 8), meno le eventuali rimaste `tradotta` con nota.
7. `ls out/snap_0x*_*_regs.txt | wc -l` → `143` attesa (127 + 16), meno le
   eventuali catture N=2/tardo assenti annotate.
8. `prima.bin` diverse per ognuna delle 8 (ciclo `cmp`) → vuoto (attesa).
9. `grep -n -e '-window' -e 'video soft' cattura.sh snap.lua` → vuoto (misurato).
10. `find .. -size +50M` → vuoto (misurato). `pgrep -c mame` → `0`.
11. `git status --short` → solo i file del lotto.

## Domande aperte e decisioni (chat principale, 2026-10-21)

1. 08CE senza riga propria in ROUTINE.md e senza voce nel banco (non
   catturabile: nessun rientro al punto dopo la `call`): tradotta come
   smistamento statico dentro `z80_08xx.c`. Risolta con la proposta
   dell'analista. «preferenza»
2. I 9 gestori mai eseguiti (08ED, 09B3, 0A24, 0BBB, 0BF6, 0C2D, 0C7C,
   0CCE, 0CDA, ~630 byte) restano non tradotti: appartengono al filone
   «zone mai eseguite con sessioni guidate» (chiusura del brief 10); lo
   smistamento li segnala con `NULL`. Risolta in modo conservativo e
   annotata qui. «scelta di progetto»
3. Lotto da 8 routine registrate (più la statica 08CE): meno di 15 perché
   il grappolo è un blocco unico da ~900 byte, il più grande mai tradotto
   in un lotto. Risolta in modo conservativo (si traduce solo il grappolo
   scelto). «scelta di progetto»
4. Catture di 0602 con `SNAP_COND='b@hl>=c0'` in entrambi gli stati (la
   prima occorrenza per frame è quasi sempre lo slot vuoto, banale);
   nessun suffisso, il tag resta `titolo`/`gioco`. Risolta con la proposta
   dell'analista. «preferenza»

## Chiusura ciclo (chat principale, 2026-10-21)

Esito: PASSATO. Primo lotto «di confini»: ciclo intero in sessione cloud
collegata a GitHub (branch lotto-07, PR #1), con mappa dei grappoli e scelta
del motore per-oggetto della NMI (0602 + smistamento 08A2/08CE + 6 gestori).
Verifica locale: 18 catture rigenerate con MAME 0.264, bin e regs identici
bit a bit al branch; 11 verifiche passate (deviazioni ammesse: sched_log
storico 106124 con differenze 0; 08D8 con 3 catture, la terza condizionata
sul ramo che scrive). I 9 gestori mai eseguiti restano NULL nello
smistamento, rinviati al filone «zone mai eseguite».
Totale: 74 righe in ROUTINE.md, 71 verificate.
Token: ciclo in cloud su Fable 5 (crediti cloud); verificatore locale Sonnet 18k.
