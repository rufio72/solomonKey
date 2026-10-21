# Compito 14: lotto 10 — la catena della NMI (0066) con 0493 e rst 28

Data: 2026-10-21. Scritto dall'agente analista per un agente sviluppatore;
questo file è tutto il contesto che serve. Schema, convenzioni, vincoli e
strumenti: brief 05 («Schema dei lotti»), 06-13. Le decisioni del brief 12
restano definitive e non si riaprono: proposta A adottata (porte sul dump,
`leggi_porta`/`scrivi_porta` in `porte.c`), C rinviata a quando servirà
l'oracolo delle scritture, B scartata, e603 al PC 4CF0 rinviata alle zone
mai eseguite.

Vincoli assoluti (come i lotti 1-9): MAME `/usr/games/mame` 0.264 con
`PATH=$PATH:/usr/games`, solo `-video none -sound none`, corse in
sottofondo, max 4 in parallelo; nessuna operazione git; non toccare `roms/`
né `.gitignore`; nessun file sopra le 800 righe; commenti e documenti in
italiano; nomi `sub_xxxx`. Non toccare `sessioni.sh`, `trace.lua`,
`sessione.lua`, `sched*.c/h`, `fondale.c`, `routine.c`, `build_rom.sh`,
`disasm.sh`, `porte.c`, né le traduzioni dei lotti 1-9. Ai siti porta del C
tradotto: indirizzo hardware esplicito (`leggi_porta(0xe600)`), mai `M()`
sulle porte, commento con il significato MAME; e603 solo via
`leggi_e603(pc dell'istruzione di lettura)`.

## Cosa esiste già (misurato oggi, 2026-10-21, dopo il lotto 9)

VM nuova: dati derivati rigenerati oggi con gli script del repo (ROM
estratte in `roms/ext/solomon/`, md5 di `main48.bin`/`main64.bin`
`daaf50c3…`/`0bca296a…`; z80dasm 1.1.6; `main48.dasm` 35732 righe; traccia
di controllo 1900 frame; sessioni `titolo`/`stanza_00`/`gameover`;
`sched_log.txt` 5500 frame). Banco: 93 routine registrate, `--tutte` → 93
RIEPILOGO, 0 FALLITO; `sched_proto` → `spedizioni confrontate: 106095,
differenze: 0; posta_attivita: 131, slot coincidenti: 131`; ROUTINE.md 95
righe (87 `verificata`, 6 `tradotta`, 2 non registrate); 180 catture `regs`
con tag (183 col vecchio formato senza tag). Censimento porte conforme al
brief 12: 31 accessi diretti, 45 righe; `out/<sessione>_e603.txt` = solo
`0161 00 <n>`; `leggi_porta(0xe6` nel C: 10 siti; `scrivi_porta`: 0 siti.

## Mappa dei grappoli (ricalcolata oggi sul callgraph)

| filone | contenuto | byte | giudizio |
|---|---|---:|---|
| (b) NMI strutturale + (a) 0493 + rst 28 | 0066 e tutte le sue callee non tradotte: 01D5, 0219, 027C, 03FB, 042F, 0469, 04AC, 0CE6, 1288, più i prerequisiti 0493 (posta dello scheduler, chiamata da 0219/03FB/042F/04AC) e 0028 (gestore `rst 28`, chiamato da 04AC e da 12 siti esterni) | ~1370 | **SCELTO**: completa l'intera catena di interrupt (le altre 11 callee della NMI sono già verificate), nessun bloccante residuo, porte già coperte da `porte.c`; la cattura della NMI è possibile (sotto); rischio concentrato nelle finestre mai eseguite, gestite come nei lotti 7 e 9 |
| (a) 0493 da solo | 25 byte; i chiamanti esterni (2951/2956 avvio, 2E35, 7979, 7C53, 7CA6, 7CE0, 64FE…) stanno in corpi di fase o jp-chain non registrabili | 25 | assorbito nel filone NMI: da solo sblocca quasi nulla |
| (c) code sbloccate dai lotti 7-9 | eleggibili oggi fuori dalla NMI: 05B5 (75 B, 2 inv), 5507 (73 B), 6C2C (100 B), 7379 (20 B), 2551 (ingresso a metà codice, da ricensire); bloccate: 29CF e 03EE da 3A54←3ADF (idioma `rst 20`, filone a sé), 37B4 da 4767 (smistatore con `rst 38`, esclusione del brief 11 invariata), 7ECA/7F2D/4F0F da 0038 (manipola l'indirizzo di ritorno), 2A03/322A (`ex (sp),hl`) | ~550 sparsi | rinviato: routine slegate fra loro, resa minore della catena NMI; rientrano nel prossimo ciclo |

## La cattura della NMI è possibile (verificato su snap.lua, oggi)

`snap.lua` arma il breakpoint d'ingresso su `SNAP_ENTRY`, legge il rientro
da `(SP)` e arma `bpset <ritorno>,sp==<SP+2>` più `bpset 66` per scartare
le occorrenze interrotte. Per `SNAP_ENTRY=0x0066`: il breakpoint scatta
all'ingresso dell'interrupt con il PC interrotto già in pila, quindi
`ritorno` = PC interrotto e la condizione su SP distingue il `retn` da ogni
passaggio del programma interrotto sugli stessi indirizzi; la NMI non è
annidabile (si riarma a 01BB-01C1, una per frame), perciò il controllo
`pc == 0x0066` in fase «dopo» non scatta mai prima del rientro. Nessuna
modifica a `snap.lua` o `cattura.sh`. La profondità di pila della catena
(≤ ~30 byte) sta nella finestra esclusa dal confronto (SP−0x40).

## Fatti verificati dal disassembly (oggi; il CSV sovrastima le estensioni)

- **0066-01C4** (351 byte, una per frame a NMI abilitata): salva AF/exx,
  IX/IY in c040/c042 (celle confrontate dal banco); se bit 2 di c044 è
  spento salta tutto il video; col bit 6 copia le palette piene c994→e400
  (0x20) e c9ca→e500 (0x80), altrimenti le code c96e→e4c6 (0x26) e
  c9b4→e5d4 (0x16); sprite c8ee→e020 (0x60); ingressi: `ldir` di 3 byte
  **e600 P1, e601 P2, e602 SYSTEM → c050-c052** (in C tre `leggi_porta`);
  `call 01D5` (monete); con bit 5 di c044 copia c050 (o c051 secondo bit 5
  di c045, byte a 00D9 mai eseguito) in c053; con bit 0 di c044 smista lo
  stato di Dana: c46e & C1 = 0 → `call z 03FB`, altrimenti c46e & 10 = 0 →
  `call z 042F`, poi (c078 ≠ 0) `call nz 03EE` (mai eseguito, chiama
  sub_3a54 non tradotta → stub con avviso) e 027C, 04AC, 0CE6, 1288; motore
  oggetti: 24 slot da 0x30 da c46e, per ciascuno `call 0602` con HL = IX =
  slot, BC da 0x1800 (push/pop attorno); `call 05B3`; `inc` su c463-c468 e
  sulla parola c2a9; contatori c2b7, c29a; se c045 & 6 = 0 e bit 3 di c045
  `call 01C5` con L = byte basso di c2a9; `call 026C`; **e603** a 015E
  (`leggi_e603(0x015E)` → 00): col bit 0 la finestra 0164-0182 (mai
  eseguita in MAME: posta 27 dopo sub_673f, statica dal dasm); con bit 1 di
  c045 il bonus-tempo: parola c234 += 1 via `rst 10` (0193 mai eseguito);
  lettura a vuoto di **e606** (watchdog); coda suoni: se c054 ≠ 0 lo
  scrive su **e800** (latch audio, `scrivi_porta`) e fa scorrere c055-c057;
  ripristina IX/IY da c040/c042, exx/pop af; riarmo NMI: **e600** = 0 poi 1
  (`scrivi_porta` ×2); `retn`. Registri d'uscita = d'ingresso (lavorare su
  locali).
- **0028-0031** (10 byte, gestore `rst 28`): push AF/BC/HL, `call 0043`
  (accoda il suono C, verificata), pop, ret. Chiamato con `rst $28` da
  04AC (04CB, 04ED, 050E, 0541) e da 12 siti esterni già censiti.
- **01D5-0218** (68 byte): monete: c046 e c047 += 2 (saturati dal fronte);
  dai bit 2-3 di c052 (COIN1/COIN2, copia di SYSTEM) gestisce
  l'anti-rimbalzo su c046/c047 (bit 0 = moneta in corso) e sul fronte nuovo
  `call 0219`; 01FE mai eseguito (statico).
- **0219-026B** (83 byte): moneta accettata: res 0,(hl); accoda il suono
  0x18 (sub_004f); contatore BCD c048/c049 (+1 col bit 7 della cella
  tariffa); tariffa dal nibble di c04a/c04b (celle scritte da sub_2980):
  somma monete in c04c, al traboccare crediti++ in c04c alto; se c045 & 6
  = 0 posta l'attività 08 (sub_0493). Una sola esecuzione nelle tracce
  (la moneta del frame ~1510).
- **027C-03ED** (370 byte): collisioni di Dana. Se bit 7 di c4ce acceso:
  primo giro 0283-0308 sui 17 slot da 0x30 di c4fe (margini dalla tabella
  ROM 9495, riquadro attorno a c4dc/c4e3): su sovrapposizione marca
  l'oggetto (res 6/set 1), 82 in c4ce, e in coda posta 12 — **mai
  eseguito** (bit 7 mai acceso nelle tracce), statico dal dasm. Altrimenti
  da c82e (Dana): se spento (< C0) o in stato 2/4 esce; 0316-031B (bit 1:
  res 0 su c46e, mai eseguito, statico); secondo giro 031C-03ED sui 17
  slot c4fe: riquadro di Dana da c83c/c843 con correzioni da c84b/c84c
  (stato 7 = scala), confronto sui margini 9495; nel ramo eseguito non
  scrive mai (solo registri); sul contatto la finestra di morte 03AA-03E0
  (pop bc, sub_05b3, suono 0A via `rst 28`, c82e = A2, azzera c467, c08d,
  c19d, res 6 sui 18 slot c4ce, posta 13) — **mai eseguita**, statica.
- **03FB-042E** (52 byte): spegnimento di Dana (chiamata quando c46e & C1
  = 0): se c82e < C0 esce; copia stato (ix+1d)→(ix+29) e direzione
  (ix+1e)→(ix+2c) (con res 1 se stato ≠ 4, 041C mai eseguito: statico),
  `call 0469`, 41 in c46e, posta 01. IX = c82e.
- **042F-0468** (58 byte): come 03FB per il cancello (chiamata quando
  c46e & 10 = 0): esce se c82e < C0 o bit 1 acceso; copia stato in +29;
  se bit 7 di c299 acceso esce, altrimenti c299 = C0 e azzera c29a-c29c;
  `call 0469`, 43 in c46e, posta 0E.
- **0469-0492** (42 byte): spegne l'oggetto IX: res 5,(ix+0); salva le
  velocità (ix+0b)/(ix+0c) in +2a/+2b; stato = C, direzione = B; X
  (ix+15) += 3 o −3 secondo il bit 0 di B; c463 = 0.
- **0493-04AB** (25 byte): la primitiva di posta dello scheduler (brief
  04): cerca da c04e il primo slot libero di c000-c03f (indice & 3f,
  c04e++) e vi scrive A; già modellata come `posta_attivita` in `sched.c`
  (che non si tocca): la traduzione `sub_0493` entra nel banco per i
  chiamanti.
- **04AC-05B2** (263 byte, nessuna finestra mai eseguita): macchina degli
  stati dell'animazione di Dana (IX = c82e, esce se < C0): contapassi
  c830++; dal segno/valore di c83a il suono 0C via `rst $28` (con il bit 2
  di c82f come anti-ripetizione); smistamento su (ix+1d): stato 0 con
  c830 ≥ 5 suona 03; stato 1 secondo bit 1 di (ix+1e) e c830: suoni
  06/07, bit 0 di c82f, posta 10 con suono 07 (0513); stati ≥ 3: nuova
  coppia stato/direzione da c053 (comando) con i casi 02/07 e la scelta
  fra 00/04/05/06; scrive sempre (ix+1d)/(ix+1e) in coda (05AC).
- **0CE6-0CF2** (13 byte eseguiti): IX = c4ce, IY = c299; se bit 7 di
  (c4ce) spento esce (sempre, nelle tracce). La continuazione 0CF3+
  (stati della chiave, salto indiretto `jp (hl)` sulla tabella 0D18) non è
  traducibile staticamente: **stub con avviso** su stderr, stato invariato.
- **1288-12AA** (35 byte eseguiti): se Dana < C0 o stato = 7 esce; cella
  al centro di Dana ((ix+0e)+8, (ix+15)+8) via sub_36f0 (verificata); se
  classe ≥ 0x37 esce (sempre, nelle tracce). La continuazione 12AB+
  (raccolta, zona mai eseguita fino a 2349): **stub con avviso**.
- Nessun salto eseguito entra nei corpi dall'esterno (verificato sul dasm:
  i soli riferimenti incrociati vengono da zone dati ≥ 8000 o mai
  eseguite 3A00/70F2/70FB): nessuna coda condivisa.

## Routine del lotto 10 (12 registrate)

t/s/g dalle `out/<sessione>_chiamate.txt` (0066 = conteggio della NMI =
chiamate a 01D5). Catture con le regole del brief 05 §1; stati scelti dai
conteggi. Porte toccate nel lotto: 7 siti (e600 R+W×2, e601, e602, e603,
e606, e800 W).

| ingresso | estensione | byte | t / s / g | come si arriva | effetto (da dasm) | catture |
|---|---|---:|---|---|---|---|
| 0028 | 0028-0031 | 10 | 5 / 25 / 3 | `rst $28` (04AC e 12 siti esterni) | salva AF/BC/HL, accoda il suono C via sub_0043, ripristina | titolo, gioco |
| 0066 | 0066-01C4 | 351 | 1435 / 700 / 700 | NMI, una per frame | v. sopra: video/sprite, ingressi e600-e602, monete, stato di Dana, motore oggetti ×24, contatori, e603, coda suoni → e800, riarmo e600 | titolo, gioco |
| 01D5 | 01D5-0218 | 68 | 1435 / 700 / 700 | 00C4 (NMI) | anti-rimbalzo monete c046/c047, sul fronte dei bit 2-3 di c052 chiama sub_0219 | titolo, gioco |
| 0219 | 0219-026B | 83 | 0 / 1 / 0 | 020C, 0215 (da 01D5) | moneta accettata: suono 18, contatori BCD c048-c049, tariffa da c04a/c04b su c04c, posta 08 | gioco sola (la sequenza di trace.lua ha una sola moneta, frame ~1510): `tradotta` con nota (N=2 e tardo attesi fallire) |
| 027C | 027C-03ED | 370 | 0 / 370 / 337 | 010E (NMI, bit 0 di c044) | collisioni di Dana sui 17 slot c4fe (margini 9495); ramo eseguito solo-registri; finestre 0283-0308, 0316-031B, 03AA-03E0 (morte) mai eseguite: statiche con nota | gioco, tardo |
| 03FB | 03FB-042E | 52 | 0 / 10 / 0 | 00F0 (NMI, c46e & C1 = 0) | se c82e attivo: salva stato/direzione in +29/+2c, sub_0469, 41 in c46e, posta 01 | gioco; gioco con `SNAP_COND='b@c82e>=c0'` suffisso `pieno` (il ramo corto `ret c` è banale) |
| 042F | 042F-0468 | 58 | 0 / 6 / 0 | 00FE (NMI, c46e & 10 = 0) | come 03FB per il cancello: c299 = C0, azzera c29a-c29c, 43 in c46e, posta 0E | gioco, gioco N=2 |
| 0469 | 0469-0492 | 42 | 0 / 10 / 0 | 0421 (03FB), 045B (042F) | spegne l'oggetto IX: salva le velocità in +2a/+2b, stato/direzione da C/B, X ± 3, c463 = 0 | gioco, gioco N=2 |
| 0493 | 0493-04AB | 25 | 4 / 24 / 4 | 0267, 042B, 0465, 0513 (lotto); 2951, 2956, 2C52, 2C6E, 2E35, 78EC, 7979, 7C53, 7CA6, 7CE0, 64FE… (filoni futuri) | posta dello scheduler: primo slot libero di c000-c03f da c04e (& 3f), vi scrive A | titolo, gioco |
| 04AC | 04AC-05B2 | 263 | 0 / 370 / 337 | 0111 (NMI, bit 0 di c044) | animazione/stati di Dana: contapassi c830, suoni via `rst 28`, transizioni (ix+1d)/(ix+1e) da c053, posta 10 | gioco, tardo |
| 0CE6 | 0CE6-0CF2 | 13 | 0 / 370 / 337 | 0114 (NMI, bit 0 di c044) | IX = c4ce, IY = c299, esce col bit 7 spento; continuazione 0CF3+ (`jp (hl)` su tabella 0D18) mai eseguita: stub con avviso | gioco, tardo (solo registri: le righe `prima` dei regs devono differire) |
| 1288 | 1288-12AA | 35 | 0 / 370 / 337 | 0117 (NMI, bit 0 di c044) | classe della cella al centro di Dana (sub_36f0), esce se ≥ 37; continuazione 12AB+ mai eseguita: stub con avviso | gioco, tardo (solo registri) |

Totale ~1370 byte di estensione (eseguiti ~1120): il lotto più grande
finora (il lotto 7 era 912), ma un grappolo unico con un solo nuovo
meccanismo (la cattura dell'interrupt). Il disassembly si legge come nei
brief precedenti, da `decomp/`:
`sed -n "$(grep -n ';04ac$' out/main48.dasm | cut -d: -f1),+180p" out/main48.dasm`
e le istruzioni eseguite con
`awk -F'\t' '$1>="04AC" && $1<="05B2"' out/stanza_00_istruzioni.txt`.
Attenzione: la traccia di controllo non esegue 0405-0468; lì fanno fede
`stanza_00_istruzioni.txt` e il dasm.

### Convenzioni di traduzione specifiche del lotto

- `sub_0066`: commento di testa che documenta `push af`/`exx`/`retn`: la
  funzione lavora su variabili locali (il banco alternativo) e lascia in
  `z` i registri d'ingresso; scrive però z→ix/z→iy in c040/c042 con `M()`
  come l'originale. La chiamata a sub_01c5 passa L = byte basso del nuovo
  c2a9; le chiamate a sub_0602 impostano z come il ciclo 0124-0131.
- Finestre mai eseguite dentro corpi eseguiti (0283-0308, 0316-031B,
  03AA-03E0, 0164-0182, 00D9, 0193, 01FE, 041C): tradotte dal dasm
  statico con commento «mai eseguito nelle tracce» (precedente: 797E).
  Continuazioni non traducibili (0CF3+ con `jp (hl)`; 12AB+; 03EE che
  chiama sub_3a54 non tradotta): avviso su stderr e ritorno, mai
  silenzioso (precedente: gestori NULL del lotto 7).
- Il riporto/Z d'uscita non serve ad alcun chiamante del lotto (la NMI
  consuma le proprie bandiere internamente): nessuna emulazione su z→f.
- `sched.c`/`sched_proto` non si toccano: `sub_0493` è la traduzione
  registrata nel banco, `posta_attivita` resta il modello dello scheduler.

## Struttura

- `harness/nmi.c` (nuovo, ~320 righe): sub_0066, sub_01d5, sub_0219, più
  lo stub statico `sub_03ee` (avviso). `harness/nmi_oggetti.c` (nuovo,
  ~500 righe): sub_027c, sub_03fb, sub_042f, sub_0469, sub_04ac. File con
  nome come `fondale.c` (v. domanda 1).
- `z80_0xxx.c` (373 righe): sub_0028 e sub_0493 in fondo (~420 righe).
- `z80_08xx.c` (221 righe): sub_0ce6 in fondo. `z80_1xxx.c` (nuovo):
  sub_1288.
- `Makefile`: `nmi.c nmi_oggetti.c` aggiunti a `SORGENTI` (i `z80_*.c`
  nuovi sono coperti dal wildcard).
- `routine.h`: blocco «Lotto 10 (compito 14)» con i 12 prototipi;
  `routine_tab.c`: 12 voci in ordine di indirizzo (0028, 0066, 01D5,
  0219, 027C, 03FB, 042F, 0469, 0493, 04AC, 0CE6, 1288), tutte
  `blocco_ini/fine` = (0, 0).
- `ROUTINE.md`: 12 righe nuove (107 totali).
- Catture: 23 attese (11 routine × 2 + 0219 singola).

## Verifiche obbligatorie (da `decomp/`)

1. `make -C harness 2>&1 | grep -ciE 'warning|error'` → `0` (attesa).
2. Per ognuna delle 11 con 2 catture: `(cd harness && ./harness 0xXXXX |
   tail -1)` → `RIEPILOGO 0xXXXX: 2 catture, 2 OK`, uscita 0;
   `./harness 0x0219` → `1 catture, 1 OK` (attesa; 1 cattura con nota dove
   la regola del brief 06 domanda 2 lo ammette).
3. `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `105`;
   `grep -c FALLITO` → `0` (attesa).
4. `(cd harness && ./sched_proto ../out/sched_log.txt | tail -1)` →
   `spedizioni confrontate: 106095, differenze: 0; posta_attivita: 131,
   slot coincidenti: 131` (misurato oggi).
5. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale" && $2!="total"'` → vuoto.
6. `grep -c '^| 0x' ROUTINE.md` → `107`; `grep -c '| verificata |'
   ROUTINE.md` → `98` attesa (87 + 11), meno le eventuali rimaste
   `tradotta` con nota (0219 già conteggiata fuori).
7. `ls out/snap_0x*_*_regs.txt | wc -l` → `203` attesa (180 + 23), meno le
   eventuali catture annotate come assenti.
8. `prima.bin` (o righe `prima` dei `regs` per le solo-registri) diverse
   per ognuna delle 11 con 2 catture (ciclo `cmp`) → vuoto (attesa).
9. `python3 censimento_porte.py | head -1` → `# Accessi diretti (operando
   e6xx/e8xx): 31`; `| grep -c '^[0-9A-F]'` → `45` (il lotto non tocca la ROM).
10. `grep -n 'M(0xe6\|M(0xe8' harness/*.c` → solo la riga storica di
    `sub_33ba` (`M(0xe802)`, z80_3xxx.c riga 144);
    `grep -c 'leggi_porta(0xe6' harness/*.c` (totale) → `14` (10 storici
    + e600, e601, e602, e606 in nmi.c); `grep -c 'scrivi_porta(0xe'
    harness/*.c` → `3` (e800 a 01A4, e600 a 01BB e 01BF);
    `grep -c 'leggi_e603(0x015E' harness/nmi.c` → `1`.
11. `grep -n -e '-window' -e 'video soft' cattura.sh snap.lua` → vuoto.
12. `find .. -size +50M` → vuoto; `pgrep -c mame` → `0`.
13. `git status --short` → solo i file del lotto (Makefile, routine.h,
    routine_tab.c, nmi.c e nmi_oggetti.c nuovi, z80_0xxx.c, z80_08xx.c,
    z80_1xxx.c nuovo, ROUTINE.md, catture nuove in out/, questo brief).

## Domande aperte e risoluzioni dell'analista

1. File nuovi col nome (`nmi.c`, `nmi_oggetti.c`) come `fondale.c`,
   invece di spezzare `z80_0xxx.c` in quarti di blocco (il blocco 0x0000
   è già diviso a metà con `z80_08xx.c` e le nuove ~820 righe non ci
   stanno): il contenuto è un sottosistema unico e il nome lo dice.
   Risolta con la proposta dell'analista. «preferenza»
2. Finestre mai eseguite dentro corpi eseguiti tradotte dal dasm statico
   con nota; continuazioni con salti indiretti o callee non tradotte
   (0CF3+, 12AB+, 03EE) ridotte a stub con avviso su stderr: visibile,
   mai silenzioso, nessun lavoro senza oracolo. Risolta in modo
   conservativo e annotata. «scelta di progetto»
3. 0219 resta a una cattura (`tradotta` con nota): la sequenza d'ingresso
   convenzionale di trace.lua/snap.lua ha una sola moneta e non si
   modifica (le catture di tutti i lotti condividono quella linea
   temporale). Risolta in modo conservativo. «scelta di progetto»
4. Lotto da 12 routine e ~1370 byte (sopra la media dei lotti): è un
   grappolo unico — la catena della NMI — e spezzarlo lascerebbe la NMI
   non traducibile a metà; il solo meccanismo nuovo è la cattura
   dell'interrupt, verificata possibile su snap.lua. Risolta in modo
   conservativo (si traduce solo la catena censita). «scelta di progetto»
5. `sub_0493` nel banco accanto a `posta_attivita` di `sched.c` (che
   resta il modello dello spedidore): nessuna unificazione in questo
   lotto, un futuro lotto di pulizia potrà farla. Risolta con la proposta
   dell'analista. «preferenza»
