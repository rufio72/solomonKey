# Compito 07: lotto 3 di traduzione in C delle routine del Z80 principale

Data: 2026-10-21. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Tradurre in C 15 nuove routine del Z80 principale di Solomon's Key (set MAME
`solomon`) e verificarle con il banco di prova `decomp/harness/` su due catture
MAME ciascuna. Schema, convenzioni e strumenti sono quelli dei lotti 1 e 2:
`compiti/05-lotto-01-routine.md` (sezione «Schema dei lotti») e
`compiti/06-lotto-02-routine.md`, già realizzati in `decomp/`. Si lavora solo
in `decomp/`.

Vincoli assoluti:
- MAME solo con `-video none -sound none`, mai `-window` né `-video soft`;
  corse MAME in sottofondo, al massimo 4 in parallelo, file diversi per ENTRY
  e stato. `cattura.sh` già lo rispetta. MAME è `/usr/games/mame` 0.264:
  **`/usr/games` non è nel PATH di questa VM**, eseguire con
  `PATH=$PATH:/usr/games`, senza modificare `cattura.sh`.
- Nessuna operazione git (add, commit, checkout, stash, reset): commit della
  chat principale. Non toccare `roms/` né `.gitignore`.
- Niente modifiche fuori da `decomp/`. Non toccare `sessioni.sh`, `trace.lua`,
  `sessione.lua`, `sched*.c/h`, `fondale.c`, `routine.c`, `porte.c`,
  `build_rom.sh`, `disasm.sh` né le traduzioni dei lotti 1 e 2 (si aggiunge
  solo in fondo a `z80_0xxx.c`, `z80_2xxx.c`, `z80_3xxx.c`, `z80_4xxx.c`).
- Non rigenerare tracce complete né `callgraph.csv`: i dati derivati sono già
  stati rigenerati oggi (vedi sotto). File sopra 50 MB creati durante il
  lavoro vanno cancellati.
- Nessun file sopra le 800 righe; commenti, messaggi e documenti in italiano.
  Compilatore `gcc` nel PATH. Nomi C `sub_xxxx` (esadecimale minuscolo).

## Cosa esiste già (fatti verificati oggi, 2026-10-21)

Questa VM è un clone fresco: `decomp/out/` conteneva solo le catture
`snap_*` tracciate in git. I dati derivati sono stati **rigenerati oggi**
dall'analista con gli script del repo (ROM ricostruite in `out/` dalla copia
estratta in una cartella temporanea, senza toccare `roms/`; z80dasm 1.1.6
installato da apt):
- `out/main48.bin`, `out/main64.bin` (md5 `daaf50c3…`, `0bca296a…`; 65536 byte);
- `out/main48.dasm` (35732 righe, z80dasm; etichette `sub_xxxxh:`/`lxxxxh:`,
  indirizzo nel commento finale `;3101`);
- traccia di controllo a 1900 frame → `out/istruzioni.txt`, `out/chiamate.txt`,
  `out/callgraph.csv` (traccia cancellata);
- sessioni `titolo` (frame 0-1500), `stanza_00` (1500-2200), `gameover`
  (7000-7700) → `out/<sessione>_chiamate.txt` (tracce cancellate);
- `out/sched_log.txt` rigenerato (5500 frame): **106095 spedizioni**, non le
  106124 storiche; `./sched_proto ../out/sched_log.txt | tail -1` →
  `spedizioni confrontate: 106095, differenze: 0; posta_attivita: 131, slot
  coincidenti: 131` (misurato oggi). Il valore storico non è riproducibile
  da questo log: le verifiche di questo lotto usano 106095.

Stato del banco, misurato oggi dopo la rigenerazione:
```
make -C harness                              → senza warning né errori
./harness --tutte | grep -c '^RIEPILOGO'     → 33
./harness --tutte | grep -c FALLITO          → 0
grep -c '| verificata |' ROUTINE.md          → 33
ls out/snap_0x*_*_regs.txt | wc -l (da decomp) → 64  (più 3 catture `base`)
```
Strumenti: `sh cattura.sh <ENTRY> <stato> [N] [SUFFISSO]` (stati titolo /
gioco / tardo, `SNAP_COND` dall'ambiente = condizione del breakpoint);
firma `void sub_xxxx(struct z80 *z)`; `M(a)` RAM c000-ffff, `leggi(a)` tutto
lo spazio; riporto in ingresso `z->f & 1`; bandiere in uscita e pila sotto SP
non confrontate; una routine chiama le tradotte in C (`sub_0010(z);`).

## Routine del lotto 3

Selezione secondo lo schema (brief 05, sezione 6): `brutta` vuota in
`callgraph.csv`, nessuna porta, tutte le destinazioni dei siti dentro
l'estensione reale già `verificata` in `ROUTINE.md`; priorità a invocazioni e
siti. L'estensione reale è letta in `main48.dasm` (il CSV sovrastima: la sua
«estensione» arriva al prossimo ingresso e ingloba tabelle ROM e code di
altre routine). Conteggi t / s / g = somme per destinazione in
`out/titolo_chiamate.txt`, `out/stanza_00_chiamate.txt`,
`out/gameover_chiamate.txt`.

| ingresso | estensione | byte | t / s / g | siti | effetto (da dasm) | catture |
|---|---|---:|---|---|---|---|
| 01C5 | 01C5-01D4 | 16 | 0 / 52 / 0 | 0150 (NMI) | A=L; HL=(c1b7); se (HL)==A: legge (HL+1) in c053 e aggiorna c1b7; quasi sempre `ret nz` | gioco, tardo + condizionata (sotto) |
| 07DE | 07DE-08A1 | 196 | 6 / 4 / 0 | 2F07 | aggiorna un record puntato da HL (confronti e somme A·4+byte); una `push hl` a 0844 con `pop hl` per ramo (084C, 0881, 089F); 3 `ret`; nessuna call | titolo, gioco |
| 2B08 | 2B08-2B28 | 33 | 24428 / 0 / 0 | 2A06, 2A14, 2A2C, 2A3E, 2A49, 2AFE | B bit 7 o indice B in tabella 2B29 (`rst 8`+`rst 18`), poi indice C in 2B61 (`rst 8`+`rst 10`); copia da (DE) fino a FF in (HL) e piano basso = 40 | titolo, titolo N=2 (solo test RAM/titolo) |
| 3000 | 3000-3008 | 9 | 2 / 2 / 0 | 2FF6, 2FFC | HL = A·4 con segno dal **riporto in ingresso** (`jr nc` a 3003: CY=1 → H=FF); solo registri | titolo, gioco |
| 3101 | 3101-3110 | 16 | 2176 / 2144 / 0 | 30B1, 30CD | moltiplicazione seriale 8 bit: H = alto(H·L)·2 (+riporti), B conservato in D; solo registri; la tabella a 3111 NON fa parte della routine | titolo, gioco |
| 31CE | 31CE-31E0 | 19 | 1 / 1 / 1 | 3218 | stringa fissa 31E1 («P.0…») a dfa7 via `sub_6771`, poi cifra c238+30 a dfac | titolo, gioco |
| 325E | 325E-326B | 14 | 4 / 4 / 2 | 324D, 3255 | record F138[(A-1)·2] (`rst 8`+`rst 10`): C,B = parametri, poi `call sub_326c` (fondale) | titolo, gioco |
| 33AA | 33AA-33B9 | 16 | 2 / 2 / 1 | 3228 | azzera c9ca-ca49 (0x80 byte), poi coda comune 33B4: `set 6,(c044)`; la coda è condivisa con 33BA | titolo, gioco |
| 33BA | 33BA-33E9 +coda 33B4-33B9 | 54 | 4 / 4 / 1 | 3231, 3DC4, 42E9, 7D30 | azzera e802; record palette F000[c243·2] (`rst 8`+`rst 18`): `ldir` di 2·(h&7f) byte a c9ca; se bit7 di h·2: passata `rld`/`srl` (il riporto di `rrc c` entra nella `rra`: tradurre esplicito); coda 33B4 | titolo, gioco |
| 34E1 | 34E1-34F9 | 25 | 10 / 15 / 5 | 34B3, 34C7, 34D0 | scrive A e 70 (due piani, `res 2,h`) in B=2 colonne dalla riga H=df, avanza L di 20 per riga; A, L in uscita | titolo, gioco |
| 36BD | 36BD-36C2 | 6 | 4 / 0 / 0 | 3690 | `call sub_2402` finché A≠0 (lo Z viene dal `dec a` dentro 2402: in C `do {…} while (z->a)`) | titolo, titolo N=2 (solo titolo) |
| 452B | 452B-456D | 67 | 104 / 292 / 406 | 44D3 | (IX+0)=D; record di 3 byte a tabella[2A+A] (`rst 8`, `add hl,de`, `rst 10`); contatore (IX+2) ciclico su (IX+3); coppie dalla lista (IX+4/5) con riavvolgimento su FF; push/pop bilanciati | titolo, gioco |
| 472E | 472E-4766 | 57 | 4 / 4 / 0 | 37CA | tabelle 488D, 8B2D (indice da c2ae bit 5-7), 48AF (`rst 8`+`rst 10`); scrive 7F e 00 nei campi +4,+5,+C del record, parola nei campi +6/+7, H/L in (IX+0E)/(IX+15) | titolo, gioco |
| 4D37 | 4D37-4D86 | 80 | 0 / 1 / 0 | 4D25 | converte HL:DE (32 bit, D·65536+…) in cifre decimali a c061+ (divisioni per 10 seriali, bit di L come resto), terminatore FF; con C≥9 azzera c239-c23c e c061=FF | gioco, tardo |
| 4D87 | 4D87-4DA1 | 27 | 1 / 64 / 62 | 3ADA, 3EE9, 4D25 | B=8 byte da (DE) all'indietro in (HL) due piani (+30, attributo C); da FF in poi azzera; il codice a 4DA2 NON fa parte della routine | gioco, tardo |

Totale circa 635 byte. Note di traduzione:
- 3000 usa il riporto in ingresso (come 24B1): `z->f & 1`.
- 33BA e 33AA condividono la coda 33B4-33B9: in C si ripete il codice (2
  righe) in entrambe le funzioni, con un commento.
- 2B08: `ret z` dentro il ciclo di copia; uscita con HL, DE avanzati.
- 01C5 quasi sempre esce su `ret nz`: la cattura del ramo principale si fa
  con la condizione di breakpoint `SNAP_COND='l==b@(w@c1b7)'` e suffisso
  `raro`: `SNAP_COND='l==b@(w@c1b7)' sh cattura.sh 0x01C5 gioco 1 raro`
  (poi tardo). Se MAME rifiuta l'espressione o non c'è occorrenza entro
  6000 frame: nota «ramo principale non catturato» in ROUTINE.md, stato
  `tradotta` (non bloccante), come da regole di qualità del brief 05.
- 2B08 e 36BD girano solo nel titolo/test RAM (s = g = 0): la seconda
  cattura è `sh cattura.sh 0xXXXX titolo 2 n2` (stessa regola di qualità:
  le due `prima` devono differire; con N=2 uguale a N=1 provare N=3).
- 4D87 ha t=1 (troppo raro nel titolo): stati gioco e tardo.
- Se in una routine compare un'istruzione non prevista (porta, `ex (sp),hl`,
  `rst 20`/`rst 38`, call a routine non verificata), fermarsi su quella
  routine, stato `scartata` in ROUTINE.md, non indovinare.

Escluse, con ragione: 0602 e 08A2 (chiamano 08CE, che manipola l'indirizzo
di ritorno), 04AC (`rst 28` → 0043, porte), 4F0F / 5507 / 7F2D / 6C2C / 3A54
(chiamano 0038 `ex (sp),hl` o 3ADF/4D87 non verificate), 01D5 / 0219
(0219 chiama 0493/004F), 36F0 / 5C32 / 1288 (catena con 7379 e 37B4 non
verificate), 4767 (nei rami non eseguiti chiama 3569, porte, e `rst 38`),
2AF5 e 3097 (chiamano 2B08 / 3101 di questo stesso lotto: al lotto 4),
027C, 0CE6, 7379, 234A, 05B5 (ragioni dei brief precedenti), 004F / 36C3
(riserve: 1 invocazione, cattura fragile).

## Struttura da realizzare

1. Funzioni in fondo ai file esistenti (stile dei lotti: commento di testa
   con ingresso/uscita/significato, `#define` per i campi RAM, niente
   bandiere salvo i riporti indicati):
   - `z80_0xxx.c`: `sub_01c5`, `sub_07de`
   - `z80_2xxx.c`: `sub_2b08`
   - `z80_3xxx.c`: `sub_3000`, `sub_3101`, `sub_31ce`, `sub_325e`,
     `sub_33aa`, `sub_33ba`, `sub_34e1`, `sub_36bd`
   - `z80_4xxx.c`: `sub_452b`, `sub_472e`, `sub_4d37`, `sub_4d87`
   Se un file supera 600 righe si spezza come da brief 05.
2. `routine.h`: blocco «Lotto 3 (compito 07)» con i 15 prototipi.
3. `routine_tab.c`: 15 voci in ordine di indirizzo; `blocco_ini/blocco_fine`
   = 0, 0 per tutte tranne 33AA (c9ca-ca49) e 4D87 (0, 0: l'indirizzo
   dipende da HL).
4. Catture da `decomp/` con `PATH=$PATH:/usr/games`, tutte via `sh
   cattura.sh`: due per routine (colonna «catture»), più la condizionata di
   01C5. Regole di qualità del brief 05 §1 (prima.bin diverse; righe `prima`
   diverse per le «solo registri» 3000 e 3101; almeno una cattura non banale
   per chi scrive RAM, altrimenti `SNAP_N` 2..5 e nota).
5. `ROUTINE.md`: 15 righe nuove in ordine di indirizzo, formato attuale.

## Verifiche obbligatorie

Da `decomp/` salvo dove indicato. «Attesa, non misurata» = dipende dal
codice che si scrive; «misurato» = misurato oggi dall'analista.

1. `make -C harness 2>&1 | grep -ci -E 'warning|error'` → `0`; esistono
   `harness/harness` e `harness/sched_proto` (attesa, non misurata).
2. Per ognuna delle 15 routine: `(cd harness && ./harness 0xXXXX | tail -1)`
   → `RIEPILOGO 0xXXXX: 2 catture, 2 OK` (3/3 per 01C5 se la condizionata
   riesce), uscita 0 (attesa, non misurata).
3. 01C5, se la condizionata riesce: `grep prima out/snap_0x01C5_*raro_regs.txt`
   → una riga con `PC=01C5` (il test percorre davvero il ramo; attesa).
4. `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `48`;
   `... | grep -c FALLITO` → `0`; uscita 0 (attesa, non misurata).
5. `(cd harness && ./sched_proto ../out/sched_log.txt | tail -1)` →
   `spedizioni confrontate: 106095, differenze: 0; posta_attivita: 131, slot coincidenti: 131`
   (misurato oggi sul log rigenerato).
6. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale"'`
   → vuoto (misurato oggi: vuoto).
7. `grep -c '| verificata |' ROUTINE.md` → `48`; `grep -c '^| 0x' ROUTINE.md`
   → `48` (attesa; meno di 48 `verificata` solo per routine `tradotta` o
   `scartata`, da spiegare nel rapporto).
8. `ls out/snap_0x*_*_regs.txt | wc -l` → `94` (64 + 30; `95` con la
   condizionata di 01C5) (attesa, non misurata).
9. I due `prima.bin` di ogni routine differiscono:
   `for e in 01C5 07DE 2B08 3000 3101 31CE 325E 33AA 33BA 34E1 36BD 452B 472E 4D37 4D87; do set -- out/snap_0x${e}_*_prima.bin; cmp -s "$1" "$2" && echo "UGUALI $e"; done`
   → vuoto (attesa, non misurata).
10. `grep -n -e '-window' -e 'video soft' cattura.sh snap.lua` → vuoto
    (misurato oggi: vuoto).
11. `find .. -size +50M` → vuoto (misurato oggi: vuoto).
12. `git status --short` (sola lettura) → solo `M decomp/ROUTINE.md`,
    `M decomp/harness/routine.h`, `M decomp/harness/routine_tab.c`,
    `M decomp/harness/z80_0xxx.c`, `z80_2xxx.c`, `z80_3xxx.c`, `z80_4xxx.c`,
    il nuovo `compiti/07-lotto-03-routine.md` e, non tracciati, i nuovi
    `decomp/out/snap_*`; inoltre `M decomp/out/...` non compare perché
    `out/` è ignorata salvo le catture. Nessun processo MAME residuo:
    `pgrep -c mame` → `0`.

## Consegna

File sopra elencati, nessun commit né push (recupero dalla chat principale).
Rapporto finale di massimo trenta righe: uscita delle verifiche 1-12, per
ogni routine gli stati e i frame di cattura usati, routine rimaste
`tradotta` o `scartata` e perché, esito della condizionata di 01C5,
deviazioni dal brief.

## Domande aperte

1. 2B08 e 36BD esistono solo nel titolo/test RAM: due catture nello stesso
   stato con N diverso e suffisso `n2`, invece di due stati. Proposta: sì
   (le regole di qualità su `prima.bin` restano). «preferenza»
2. 01C5: ramo principale con `SNAP_COND='l==b@(w@c1b7)'` e suffisso `raro`,
   terza cattura non bloccante. Proposta: come descritto. «preferenza»
3. 4D87 con t=1: stati gioco e tardo al posto di titolo e gioco. Proposta:
   sì. «preferenza»
4. I dati derivati (`callgraph.csv`, `*_chiamate.txt`, `istruzioni.txt`,
   `sched_log.txt`, dasm, bin) non sono in git e sono stati rigenerati oggi;
   `sched_log.txt` rigenerato dà 106095 spedizioni (vs 106124 storiche),
   differenze 0. Le verifiche usano i valori misurati oggi. «scelta di progetto»
5. 2AF5 e 3097 chiamano routine di questo stesso lotto (2B08, 3101): il
   criterio del brief 05 §6 chiede callee già `verificata` alla selezione,
   quindi restano fuori e passano al lotto 4. «scelta di progetto»

## Decisioni (chat principale, 2026-10-21)

- Domande 1, 2, 3 (preferenza): accettate come proposte dall'analista.
- Domanda 4 (scelta di progetto, conservativa): i dati derivati rigenerati
  restano fuori da git come prima (`out/` ignorata salvo `snap_*`); le
  verifiche misurate si riferiscono ai file rigenerati oggi su questa VM,
  valore sched_proto 106095. Annotato qui e nel rapporto.
- Domanda 5 (scelta di progetto, conservativa): criterio §6 applicato alla
  lettera; 2AF5 e 3097 rinviate al lotto 4.

## Chiusura ciclo (chat principale, 2026-10-21)

Esito: PASSATO. Ciclo intero (analista, sviluppatore, verificatore) eseguito
in una sessione cloud (`session_01PQGqq9gWFBpFcL3k9EC54S`, VM Ubuntu 24.04,
MAME 0.264 da apt, ROM allegate alla sessione perché gitignorate): 12
verifiche su 12 passate in cloud. Recupero in locale: diff dei file di testo
(il teleport non porta i file; gli allegati base64 sono bloccati
dall'estensione del browser, il file è stato scaricato da claude.ai con
permesso dell'utente), catture binarie rigenerate in locale con MAME 0.264:
30 catture, 0 fallite, regs identici bit a bit a quelli del cloud.
Verificatore locale: 11/12 passate; la 5 dà il conteggio storico 106124 (il
log locale non è quello rigenerato in cloud, che dava 106095), con
differenze 0 e posta_attivita 131/131 — sostanza passata. 01C5 resta con 3
catture (gioco, tardo fallita, condizionata gioco_raro con PC=01C5).
Totale routine verificate: 48.
Token: ciclo intero in cloud su Fable 5 (crediti per sessioni cloud, non
piano); verificatore locale Sonnet 17k; più il test di fattibilità MAME
nella stessa sessione.
