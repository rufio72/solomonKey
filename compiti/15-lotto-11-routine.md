# Compito 15: lotto 11 — chiusura del perimetro osservato

Data: 2026-10-21. Scritto dall'agente analista per un agente sviluppatore;
questo file è tutto il contesto che serve. Schema, convenzioni, vincoli e
strumenti: brief 05 («Schema dei lotti»), 06-14. Le decisioni dei brief
12-14 restano definitive e non si riaprono: proposta A per le porte
(`leggi_porta`/`scrivi_porta` in `porte.c`, scritture non mappate comprese,
decisione 2 del brief 12), C rinviata, e603 solo via `leggi_e603(pc)`,
e603 al PC 4CF0 rinviata alle zone mai eseguite.

Vincoli assoluti (come i lotti 1-10): MAME `/usr/games/mame` 0.264 con
`PATH=$PATH:/usr/games`, solo `-video none -sound none`, corse in
sottofondo, max 4 in parallelo; nessuna operazione git; non toccare `roms/`
né `.gitignore`; nessun file sopra le 800 righe; commenti e documenti in
italiano; nomi `sub_xxxx`. Non toccare `sessioni.sh`, `trace.lua`,
`sessione.lua`, `sched*.c/h`, `fondale.c`, `routine.c`, `build_rom.sh`,
`disasm.sh`, `porte.c`, né le traduzioni dei lotti 1-10 (nmi.c e
nmi_oggetti.c compresi: il raccordo di `sub_03ee` con la nuova `sub_3a54`
NON si fa in questo lotto, v. esclusioni). Ai siti porta del C tradotto:
indirizzo hardware esplicito, mai `M()` sulle porte, commento col
significato MAME; letture sotto c000 con `leggi()`.

## Cosa esiste già (misurato oggi, 2026-10-21, dopo il lotto 10)

VM nuova: dati derivati rigenerati oggi con gli script del repo (ROM
estratte in `roms/ext/solomon/`, md5 di `main48.bin`/`main64.bin`
`daaf50c3…`/`0bca296a…`; z80dasm 1.1.6; `main48.dasm` 35732 righe; traccia
di controllo 1900 frame; sessioni `titolo`/`stanza_00`/`gameover`;
`sched_log.txt` 5500 frame). Banco: 105 routine registrate, `--tutte` →
105 RIEPILOGO, 0 FALLITO; `sched_proto` → `spedizioni confrontate: 106095,
differenze: 0; posta_attivita: 131, slot coincidenti: 131`; ROUTINE.md 107
righe (98 `verificata`, 7 `tradotta`, 2 non registrate); 203 catture
`regs`. Censimento porte: 31 accessi diretti, 45 righe;
`leggi_porta(0xe6` nel C: 14 siti; `scrivi_porta(0xe`: 3 siti.

## Ricalcolo del perimetro osservato (oggi, sul callgraph rigenerato)

Ingressi del `callgraph.csv` non presenti in ROUTINE.md: 22. Si dividono
così (verificato sul dasm e sulle `*_chiamate.txt`):

- **9 traducibili e registrabili → questo lotto**: 05B5, 2551, 322A,
  3A54, 5507, 6C2C, 7379, 7ECA, 7F2D (~344 byte di estensione).
- **4 già tradotte dentro `fondale.c`**: 23BB, 23EC, 336C (più 23CB
  raggiunta in coda) sono le callee di sub_326c (verificata), funzioni
  statiche di `fondale.c` che non si tocca: la loro verifica è quella di
  0x326C. Nessun lavoro.
- **9 escluse con motivazione** (sezione «Escluse»): 29CF, 2A03 (filone
  strutturale), 4F0F, 37B4, 4767 (filone smistatori/0038), 2D8A, 3ADF
  (filone `rst 20`), 0020, 0038 (gestori-idioma; 08CE, lo smistatore del
  lotto 7, è già statico dentro sub_08a2).

Con le 9 del lotto il perimetro osservato chiamabile è chiuso: ogni
destinazione di `call`/`rst` osservata nelle tracce è tradotta, dentro una
traduzione verificata, o assegnata con motivazione a un filone.

## Fatti verificati dal disassembly (oggi)

- **05B5-0601** (77 byte): secondo ingresso di sub_05b3 (verificata,
  routine.c): 05B3 è solo `ld c,000h` e prosegue in 05B5; quel C è morto
  (riassegnato a 05C4/05CA prima d'ogni uso), quindi il corpo è identico.
  Tutti i rami del corpo sono eseguiti (05F9 incluso, 207 volte in
  stanza_00). Chiamata diretta: 2F16 (t/s/g 1/1/0); il PC 05B5 è inoltre
  attraversato dalla NMI a ogni frame (via 05B3), quindi le occorrenze per
  la cattura abbondano e il prima/dopo prova esattamente il corpo 05B5.
- **2551-255A** (10 byte): record di 3 byte: HL += 3·BC (tabella, anche
  ROM: il chiamante osservato 7650 passa HL=765B), scambio DE/HL,
  incrementa il contatore M(DE_in+1), A = byte della tabella
  (`leggi()`), `cp (hl)` e `ccf`: riporto d'uscita = A ≥ contatore
  (emularlo su z->f con commento: lo consuma il chiamante). Uscita:
  HL = DE_in+1, DE = HL_in+3·BC.
- **322A-325D** (52 byte): carico del fondale della stanza c243: sub_33aa
  (azzera palette di stanza), 0 su **e802 (non mappato in MAME, no-op:
  `scrivi_porta`, decisione 2 del brief 12)**; record di 4 byte a
  F068+4·c243 (`rst 8`+`rst 10`+`rst 18`): parola 1 = flusso a sub_326c
  (BC=1000), parola 2 = coppia di codici sovrapposizione: se il byte alto
  ≠ 0 sub_325e(A=alto) con HL=F138, se il basso ≠ 0 sub_325e(A=basso);
  palette con sub_33ba (C=1). Il `push de`/`rst 18`/`ex (sp),hl`/`pop hl`
  lavora solo sulla parola appena spinta (nessuna manipolazione
  dell'indirizzo di ritorno): in C è una variabile locale. Callee tutte
  verificate. Entrambi i rami eseguiti nelle tracce.
- **3A54-3A7C** (41 byte): riquadro 2×2 di tile: alla cella (D=riga,
  E=colonna; sub_2418, verificata) del piano tile d400 scrive B, B+1 /
  B+2, B+3 e l'attributo C nelle 4 posizioni del piano d000 (offset
  −0x400); HL e DE conservati (push/pop), uscita B += 3, A −= 3.
  310 invocazioni osservate, 4 chiamanti osservati (29DA, 2F52, 2FA0,
  3851) + 4621, 4BBA in stanza_00; altri siti statici (03F7, 3C35, 4235,
  4993, 68BE, 68C7, 69D6, 6A99) in zone mai eseguite o future.
- **5507-5519** (19 byte): prossimità/direzione per i corpi di fase dei
  nemici (chiamante osservato 5648; IX = slot, IY = oggetto): A = doppio
  del valore assoluto di (ix+4); se A ≥ C esce col riporto 0; altrimenti
  A = 2·(((iy+1e) ror 1) XOR (ix+5)) e riporto = bit 7 del risultato
  prima del raddoppio (emulare su z->f con commento). Solo registri.
  Secondo ingresso 550A mai osservato (unico riferimento da zona dati
  A099): nessuna voce propria.
- **6C2C-6C5E** (51 byte, eseguiti 6C2C-6C47): inizializzazione a due
  passi di un oggetto (chiamante osservato 6C1F; IX = slot c4fe?, IY =
  oggetto; A = posa): primo passo (bit 5 di (ix+1) spento): `set 5`,
  (iy+1d) = A, (ix+0d) = 0, (iy+1e) = bit 2 di (iy+4) portato al bit 0.
  La finestra 6C48-6C5E (passi successivi: `set 0` su (ix+1)/(iy+0),
  oppure suono 30 via `rst 28`) non è mai eseguita nelle tracce:
  tradurla dal dasm statico con nota (precedente: 797E, lotto 9).
- **7379-738E + coda 2540-2547** (30 byte): cerca uno slot libero (bit 7
  di +0 spento) fra i 17 slot c2c1 partendo dall'ultimo (c441) a passo
  −0x18; trovato: azzera il byte +1, A = indice dello slot (B−1),
  riporto 1; esauriti: riporto 0 (`or a`/`ret` a 738A, **mai eseguito
  nelle tracce**: statico con nota). La coda 2540-2547 è privata (unico
  riferimento: il `jp` interno 738C). Riporto su z->f con commento.
- **7ECA-7EFD** (52 byte): cornice del pannello di gioco: due file di 14
  caratteri consecutivi (04-11 a ddaa, 12-1F a ddca) via sub_7f2d; poi le
  celle dei bordi con sub_7efe (verificata): C=2 su 11 righe × 16
  colonne, C=3 su 11 righe × 5 colonne. Chiamanti 790F, 7CDB (avvio
  quadro). Tutto eseguito.
- **7F2D-7F38** (12 byte): scrive B caratteri consecutivi a partire da C
  (attributo 60 sull'altro piano, `res 2,h`/`set 2,h`) da HL in avanti;
  uscita C += B_in, B = 0. Chiamata solo da 7ED0/7ED8 (dentro 7ECA), ma
  registrabile a sé. Tutto eseguito.
- Nessun salto eseguito entra nei corpi dall'esterno (verificato sul
  dasm: l'unico riferimento incrociato, `call z,sub_550ah` ad A099, sta
  in zona dati ≥ 8000): nessuna coda condivisa.

## Routine del lotto 11 (9: 7 registrate + 2 tradotte non registrabili)

t/s/g dalle `out/<sessione>_chiamate.txt`. Catture con le regole del
brief 05 §1; stati scelti dai conteggi. Porte toccate nel lotto: 1 sito
(e802 W in 322A).

**Correzione dopo la prima tornata di catture (misurata oggi):** tutte le
occorrenze di **322A** e **7ECA** sono attraversate dalla NMI (sono i
carichi di quadro, lunghi più di un frame: snap.lua le scarta tutte, in
ogni stato e fino al limite dei 6000 frame). Restano tradotte ma **non
registrabili nel banco**, come 2BC9 e 75AA: stato `tradotta`, nessuna
voce in `routine_tab.c`, nota in ROUTINE.md. Le callee restano coperte
(326C, 325E, 33AA, 33BA, 7F2D, 7EFE verificate). Inoltre la scrittura di
**7379** è sempre banale (azzera il byte +1 di uno slot libero, che è già
0: provato N=1..3 in gioco, N=1..2 in titolo, N=1 in tardo): si tengono
le 2 catture e lo stato resta `tradotta` con nota, come 2E11.

| ingresso | estensione | byte | t / s / g | come si arriva | effetto (da dasm) | catture |
|---|---|---:|---|---|---|---|
| 05B5 | 05B5-0601 | 77 | 1 / 1 / 0 (+1 passaggio/frame via 05B3) | 2F16; corpo comune con 05B3 (NMI) | tabella sprite c8ee dai 24 oggetti c46e, identica a sub_05b3 (il `ld c,0` di 05B3 è morto): in C delega a sub_05b3 | titolo, gioco |
| 2551 | 2551-255A | 10 | 9 / 0 / 0 | 7650 (blocco 760B; altri siti futuri) | contatore M(DE+1)++ e confronto col byte del record HL+3·BC (ROM via `leggi()`); riporto = byte ≥ contatore (su z->f) | titolo, titolo N=2 `n2` |
| 322A | 322A-325D | 52 | 2 / 2 / 1 | 3225 (corpo di fase), 7901, 7CD3 (avvio quadro) | fondale completo della stanza c243: sub_33aa, 0 su **e802** (`scrivi_porta`, no-op MAME), flusso F068[c243] a sub_326c, sovrapposizioni (byte alto/basso della parola 2) a sub_325e, palette sub_33ba | non catturabile (NMI): `tradotta`, non registrata |
| 3A54 | 3A54-3A7C | 41 | 267 / 45 / 1 | 29DA, 2F52, 2FA0, 3851, 4621, 4BBA (+ statici 03F7, 3C35, 4235, 4993, 68BE-6A99) | riquadro 2×2: tile B..B+3 alla cella (D,E) di d400, attributo C alle 4 posizioni di d000; B += 3, A −= 3, HL/DE conservati | titolo, gioco |
| 5507 | 5507-5519 | 19 | 0 / 404 / 351 | 5648 (corpo di fase nemici; 54EC, 553A, 5569 statici) | prossimità: |2·(ix+4)| ≥ C → riporto 0; altrimenti riporto dal XOR direzione (iy+1e)/(ix+5) (su z->f); solo registri | gioco, tardo |
| 6C2C | 6C2C-6C5E | 51 | 0 / 2 / 0 | 6C1F (551C statico) | primo passo: set 5,(ix+1), posa A in (iy+1d), (ix+0d)=0, direzione da bit 2 di (iy+4); finestra 6C48-6C5E (passi 2-3, suono 30 via `rst 28`) mai eseguita: statica con nota | gioco, gioco N=2 `n2` |
| 7379 | 7379-738E + 2540-2547 | 30 | 5 / 6 / 2 | 373F, 56AB (+ 3792, 46A9, 46DA, 5590, 576F, 5858, 5AC4, 5AEF, 5B34, 6BB3, 6D79, 6D83, 720A statici) | primo slot libero dei 17 c2c1 dall'ultimo (c441, passo −0x18): azzera +1, A = indice, riporto 1; esauriti (738A, mai eseguito): riporto 0; riporto su z->f | titolo, gioco |
| 7ECA | 7ECA-7EFD | 52 | 1 / 1 / 0 | 790F, 7CDB (avvio quadro) | cornice del pannello: caratteri 04-11 a ddaa e 12-1F a ddca (sub_7f2d), bordi con sub_7efe (C=2 ×16 colonne, C=3 ×5, righe D=0B..1) | non catturabile (NMI): `tradotta`, non registrata |
| 7F2D | 7F2D-7F38 | 12 | 2 / 2 / 0 | 7ED0, 7ED8 (dentro 7ECA) | B caratteri consecutivi da C (due piani, attributo 60) da HL; C += B, B = 0 | titolo, gioco |

Totale ~344 byte di estensione (eseguiti ~319; nuovi da tradurre ~267,
perché il corpo di 05B5 coincide con sub_05b3 già verificata). Il
disassembly si legge come nei brief precedenti, da `decomp/`:
`sed -n "$(grep -n ';3a54$' out/main48.dasm | cut -d: -f1),+45p" out/main48.dasm`
e le istruzioni eseguite con
`awk -F'\t' '$1>="3A54" && $1<="3A7C"' out/stanza_00_istruzioni.txt`.

### Convenzioni di traduzione specifiche del lotto

- `sub_05b5`: delega (`sub_05b3(z);`) con commento che documenta il
  doppio ingresso e il `ld c,0` morto; `routine.c` non si tocca.
- Riporto d'uscita emulato su `z->f` con commento per 2551, 5507, 7379
  (lo consumano chiamanti dei filoni futuri; il banco non confronta F):
  precedente del brief 13, domanda 5.
- Finestre mai eseguite dentro corpi eseguiti (6C48-6C5E, 738A-738B):
  tradotte dal dasm statico con commento «mai eseguito nelle tracce».
- Porte: in 322A `scrivi_porta(0xe802, 0)` con commento «non mappato in
  MAME: no-op» (decisione 2 del brief 12; `sub_33ba` resta com'è);
  nessun'altra porta nel lotto.
- Tabelle in ROM lette con `leggi()`: 765B (2551), F068/F138 (322A).

## Escluse e perché (chiusura del perimetro osservato)

- **29CF** (29CF-29F7 + finestra 29F8-2A02 mai eseguita) e **2A03**
  (2A03-2AB9 + finestre 2ABA-2AC7, 2AC8-2AE1 mai eseguite): griglia di
  test (29CF disegna 14×16 riquadri via 3A54) e schermo di servizio.
  Entrambe contengono cicli d'attesa multi-frame su c467 (incrementato
  dalla NMI, soglia 0xE0 ≈ 224 frame: misurato 69610 e 1525 giri nelle
  tracce): snap.lua scarta ogni occorrenza attraversata dalla NMI (non
  catturabili, come 2BC9/75AA) e in C la funzione non terminerebbe senza
  una NMI viva. Rinviate al **filone strutturale** (avvio/attract), dove
  il brief 13 già collocava i loro contesti (29CF/29EB, 2AAA); le callee
  (3A54 da questo lotto, 2BC3, 2AE2, 2B08) sono pronte.
- **4F0F** (13 byte eseguiti, poi `rst 38` con tabella inline), **37B4**
  (chiama 4767) e **4767** (smistatore su A con `rst 38`): **filone
  smistatori/0038** (esclusione del brief 11, invariata).
- **2D8A, 3ADF**: idioma `rst 20` con tabelle inline (`brutta = rst20`
  nel callgraph): **filone rst 20**.
- **0020, 0038**: gestori-idioma che manipolano la pila; i loro usi si
  traducono sito per sito nei filoni rispettivi (08CE è già statico
  dentro sub_08a2, lotto 7).
- **23BB, 23CB, 23EC, 336C**: già tradotte come funzioni statiche di
  `fondale.c` (callee di sub_326c, verificata); `fondale.c` non si tocca
  e la registrazione singola non aggiunge oracolo. Nessuna voce.
- Il raccordo di `sub_03ee` (stub del lotto 10) con la nuova `sub_3a54`
  resta al filone zone mai eseguite: 03EE non è mai eseguita (nessun
  oracolo) e nmi.c non si tocca.

## Struttura

- `z80_0xxx.c` (409 righe): `sub_05b5` in fondo (~10 righe).
- `z80_2xxx.c` (484 righe): `sub_2551` in fondo (~20 righe; resta < 600).
- `z80_3xxx.c` (485 righe): `sub_322a` e `sub_3a54` in fondo, in ordine
  di indirizzo (~60 righe; resta < 600).
- `z80_5xxx.c` (60), `z80_6xxx.c` (44): `sub_5507`, `sub_6c2c` in fondo.
- `z80_7xxx.c` (144 righe): `sub_7379`, `sub_7eca`, `sub_7f2d` in fondo,
  in ordine di indirizzo.
- `routine.h`: blocco «Lotto 11 (compito 15)» con i 9 prototipi;
  `routine_tab.c`: 7 voci in ordine di indirizzo (05B5, 2551, 3A54,
  5507, 6C2C, 7379, 7F2D), tutte `blocco_ini/fine` = (0, 0); 322A e
  7ECA non registrate (correzione sopra).
- `ROUTINE.md`: 9 righe nuove (116 totali).
- Catture: 14 attese (7 registrate × 2).

## Verifiche obbligatorie (da `decomp/`)

1. `make -C harness 2>&1 | grep -ciE 'warning|error'` → `0` (attesa).
2. Per ognuna delle 7 registrate: `(cd harness && ./harness 0xXXXX |
   tail -1)` → `RIEPILOGO 0xXXXX: 2 catture, 2 OK`, uscita 0 (attesa).
3. `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `112`;
   `grep -c FALLITO` → `0` (attesa).
4. `(cd harness && ./sched_proto ../out/sched_log.txt | tail -1)` →
   `spedizioni confrontate: 106095, differenze: 0; posta_attivita: 131,
   slot coincidenti: 131` (misurato oggi).
5. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale" && $2!="total"'` → vuoto.
6. `grep -c '^| 0x' ROUTINE.md` → `116`; `grep -c '| verificata |'
   ROUTINE.md` → `104` attesa (98 + 6: 7379 resta `tradotta` per la
   scrittura sempre banale, 322A e 7ECA non registrate).
7. `ls out/snap_0x*_*_regs.txt | wc -l` → `217` attesa (203 + 14).
8. `prima.bin` (o righe `prima` dei `regs` per le solo-registri: 5507)
   diverse per ognuna delle 7 registrate (ciclo `cmp`) → vuoto (attesa).
9. `python3 censimento_porte.py | head -1` → `# Accessi diretti (operando
   e6xx/e8xx): 31`; `| grep -c '^[0-9A-F]'` → `45` (il lotto non tocca la ROM).
10. `grep -n 'M(0xe6\|M(0xe8' harness/*.c` → solo la riga storica di
    `sub_33ba` (`M(0xe802)`); `grep -c 'leggi_porta(0xe6' harness/*.c`
    (totale) → `14` (invariato); `grep -c 'scrivi_porta(0xe' harness/*.c`
    → `4` (3 storici + e802 in `sub_322a`).
11. `grep -n -e '-window' -e 'video soft' cattura.sh snap.lua` → vuoto.
12. `find .. -size +50M` → vuoto; `pgrep -c mame` → `0`.
13. `git status --short` → solo i file del lotto (routine.h,
    routine_tab.c, z80_0xxx.c, z80_2xxx.c, z80_3xxx.c, z80_5xxx.c,
    z80_6xxx.c, z80_7xxx.c, ROUTINE.md, catture nuove in out/, questo
    brief).

## Domande aperte e risoluzioni dell'analista

1. `sub_05b5` come delega a `sub_05b3` invece di duplicarne il corpo: il
   `ld c,0` di 05B3 è morto (C riassegnato prima d'ogni uso), quindi i
   due ingressi sono semanticamente identici e la delega evita un doppione
   da 30 righe. Risolta con la proposta dell'analista. «preferenza»
2. 29CF e 2A03 escluse e rinviate al filone strutturale nonostante le
   callee siano ora tutte pronte: i cicli d'attesa sulla NMI le rendono
   non catturabili e non eseguibili nel banco (nessun oracolo, funzione
   che non termina). Risolta in modo conservativo e annotata. «scelta di
   progetto»
3. 7F2D registrata a sé benché chiamata solo da dentro 7ECA: 12 byte con
   oracolo proprio, e la verifica di 7ECA ne riusa la traduzione. Risolta
   con la proposta dell'analista. «preferenza»
4. Lotto da 9 routine (~344 byte) senza file nuovi: tutte le aggiunte
   stanno sotto le 600 righe per file, nessuno spezzamento. Risolta in
   modo conservativo. «scelta di progetto»
5. Il quadro di chiusura (routine osservate restanti e filoni) va nel
   rapporto finale del ciclo e nella sezione «Chiusura ciclo» di questo
   brief, non in ROUTINE.md. Risolta con la proposta dell'analista.
   «preferenza»

## Chiusura ciclo (chat principale, 2026-10-21)

Esito: PASSATO, 13 verifiche su 13 (coi numeri corretti dopo la prima
tornata di catture: banco 112 RIEPILOGO e 0 FALLITO, ROUTINE.md 116 righe
di cui 104 verificate, 217 catture regs, sched_proto 106095/0 e 131/131,
censimento porte 31/45 invariato). Le 7 registrate passano al primo colpo
con 2 catture ciascuna; 7379 resta `tradotta` (scrittura sempre banale,
provato N=1..3); 322A e 7ECA tradotte ma non registrabili (carichi di
quadro più lunghi di un frame: ogni occorrenza attraversata dalla NMI,
misurato in tutti gli stati).

**Quadro di chiusura del perimetro osservato.** Ogni destinazione di
`call`/`rst` osservata nelle tracce è ora tradotta (107 funzioni nel
banco più 2402/05B3/326C storiche e 5 tradotte non registrabili), dentro
una traduzione verificata (23BB/23CB/23EC/336C in fondale.c; 08CE in
sub_08a2), o assegnata con motivazione a un filone: **strutturale**
(scheduler, corpi di fase, avvio/attract 2833-295B con 29CF e 2A03, cicli
d'attesa sulla NMI), **rst 20** (3ADF, 2D8A e l'idioma 0020 con tabelle
inline), **smistatori/0038** (4F0F, 37B4, 4767), **zone mai eseguite**
(sessioni guidate; lì anche il raccordo dello stub 03EE con sub_3a54 e
l'e603 al PC 4CF0), **audio** (Z80 secondario, fuori perimetro fin dal
sondaggio). Token: ciclo intero in sessione cloud su Fable 5.

## Chiusura ciclo (chat principale, 2026-10-21)

Esito: PASSATO. Ciclo in sessione cloud interrotto per esaurimento crediti
dopo il salvataggio urgente: il branch lotto-11 è arrivato completo (brief,
9 routine tradotte, 14 catture, 9 escluse motivate e rinviate ai filoni
strutturale e zone mai eseguite). In locale: 14 catture rigenerate
identiche bit a bit; verificatore con 13 verifiche passate nella sostanza
(sched_log storico 106124 con differenze 0; prima=dopo dove il brief lo
prevede per routine di sola lettura). IL PERIMETRO OSSERVATO È CHIUSO:
116 righe in ROUTINE.md, 104 verificate, banco a 112 RIEPILOGO e 0 falliti.
Token: ciclo in cloud su Fable 5 (crediti cloud, esauriti a fine lotto);
verificatore locale Sonnet 20k.
