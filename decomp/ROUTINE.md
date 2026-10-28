# Routine del Z80 principale tradotte in C

Aggiornato da ogni lotto (schema in `compiti/05-lotto-01-routine.md`).
Stato `tradotta` = C scritto, meno di 2 catture OK; `verificata` = almeno 2
catture tutte OK e regole di qualità rispettate. Verifica da `decomp/harness`:
`./harness 0xXXXX` o `./harness --tutte`. Catture: `sh cattura.sh <ENTRY> <stato> [N] [SUFFISSO]`
da `decomp/` (`SNAP_COND` dall'ambiente = condizione del breakpoint d'ingresso);
i frame sono quelli della cattura riuscita.

| ingresso | funzione | file | byte | descrizione | stato | catture OK | stati | note |
|---|---|---|---:|---|---|---|---|---|
| 0x0008 | sub_0008 | z80_0xxx.c | 6 | `rst 08`: DE = A, HL += 2·A (indice in tabella di parole) | verificata | 2 | titolo, gioco | titolo f1002, gioco f2402; solo registri |
| 0x0010 | sub_0010 | z80_0xxx.c | 5 | `rst 10`: DE = parola a HL (anche ROM), HL += 2 | verificata | 2 | titolo, gioco | titolo f1002, gioco f2402; solo registri |
| 0x0018 | sub_0018 | z80_0xxx.c | 3 | `rst 18`: come `rst 10` con DE e HL scambiati | verificata | 2 | titolo, gioco | titolo f1002, gioco f2402; solo registri |
| 0x0028 | sub_0028 | z80_0xxx.c | 10 | gestore `rst 28`: accoda il suono C via sub_0043 conservando AF, BC, HL | verificata | 2 | titolo, gioco | titolo f1189, gioco f2406 (scrivono via sub_004f) |
| 0x0043 | sub_0043 | z80_0xxx.c | 12 | filtro suoni in attract (dal gestore rst 28): se c045&6 = 0 e il bit 0 di e604 (DSW1, Demo Sounds) è acceso esce; altrimenti accoda C alla coda suoni via sub_004f | verificata | 2 | titolo, gioco | titolo f1189, gioco f2406 (scrivono via sub_004f) |
| 0x004F | sub_004f | z80_0xxx.c | 14 | scrive C nel primo slot libero fra c054-c056 (c057 se pieni) | verificata | 2 | gioco, tardo | gioco f2406, tardo f4037 (scrivono) |
| 0x0066 | sub_0066 | nmi.c | 351 | la NMI (una per frame): palette e sprite ai registri video, ingressi e600-e602 in c050-c052, monete (sub_01d5), stato di Dana (03FB/042F/027C/04AC/0CE6/1288), motore oggetti (sub_0602 sui 24 slot), tabella sprite (sub_05b3), contatori, copione (sub_01c5), tempo (sub_026c), protezione e603, watchdog e606, coda suoni su e800, riarmo maschera su e600; registri del chiamante intatti | verificata | 2 | titolo, gioco | titolo f1002, gioco f2402 (scrivono) |
| 0x01C5 | sub_01c5 | z80_0xxx.c | 16 | confronta L col byte corrente del copione c1b7, estrae il codice seguente in c053 | verificata | 2 | gioco, gioco_raro | gioco f1511 (`ret nz`); ramo principale con `SNAP_COND='l==b@(w@c1b7)'` f1515; tardo senza occorrenze entro 6000 frame |
| 0x01D5 | sub_01d5 | nmi.c | 68 | anti-rimbalzo monete: c046/c047 += 2 a frame, il fronte di pressione dei bit 2-3 di c052 segna il bit 0, il fronte di rilascio chiama sub_0219 | verificata | 2 | titolo, gioco | titolo f1002, gioco f2402 (scrivono) |
| 0x0219 | sub_0219 | nmi.c | 83 | moneta accettata: suono 18, contatore c048/c049, tariffa c04a/c04b sui crediti c04c, posta 08 in attract | tradotta | 1 | gioco | gioco f1532 (scrive): unica moneta della sequenza convenzionale (frame 1510-1530); N=2 e gli altri stati senza occorrenze |
| 0x026C | sub_026c | z80_0xxx.c | 16 | dalla NMI: se c08d bit 7, parola c090-c091 += c08f | verificata | 2 | titolo, gioco | titolo f1002 (scrive), gioco f2402 (`ret z`) |
| 0x027C | sub_027c | nmi_oggetti.c | 370 | collisioni di Dana coi 17 slot c4fe (margini dalla seconda metà del record 9495): il ramo eseguito non scrive (solo registri); le finestre 0283-0308 (marcatura col bit 7 di c4ce), 0316-031B e la morte 03AA-03E0 mai eseguite: dal dasm statico | verificata | 2 | gioco, tardo | gioco f2402, tardo f4002 |
| 0x03FB | sub_03fb | nmi_oggetti.c | 52 | spegnimento di Dana: posa e direzione salvate in +29/+2c, sub_0469, 41 in c46e, posta 01 | verificata | 2 | gioco, gioco_pieno | gioco f2406 (scrive); occorrenza 2 con `SNAP_COND='b@c82e>=c0'` f2590 |
| 0x042F | sub_042f | nmi_oggetti.c | 58 | ingresso nel cancello: c299 = c0, azzera c29a-c29c, sub_0469 (posa 7), 43 in c46e, posta 0E | verificata | 2 | gioco, gioco_n2 | occorrenze 1 (f2422) e 2 (f2478); scrivono |
| 0x0469 | sub_0469 | nmi_oggetti.c | 42 | spegne l'oggetto IX: velocità salvate in +2a/+2b, posa e direzione da C e B, X ± 3 dal bit 0 di B, c463 = 0 | verificata | 2 | gioco, gioco_n2 | occorrenze 1 (f2406) e 2 (f2422); scrivono |
| 0x0493 | sub_0493 | z80_0xxx.c | 25 | posta dello scheduler: primo slot libero di c000-c03f da c04e (indice & 3f), vi scrive A (modello: posta_attivita di sched.c, che resta) | verificata | 2 | titolo, gioco | titolo f1510 (la moneta), gioco f2406 (scrivono) |
| 0x04AC | sub_04ac | nmi_oggetti.c | 263 | animazione e stati di Dana: suono dei passi dal segno di c83a, contapassi c830, transizioni posa/direzione dal comando c053 (salto col bit 2), posta 10 sul passo nuovo | verificata | 2 | gioco, tardo | gioco f2402, tardo f4002 (scrivono) |
| 0x05B3 | sub_05b3 | routine.c | 79 | tabella sprite c8ee dagli oggetti a c46e | verificata | 2 | base, gioco | gioco f2402 (scrive 2 byte) |
| 0x05B5 | sub_05b5 | z80_0xxx.c | 77 | secondo ingresso della tabella sprite sub_05b3 (il `ld c,0` di 05B3 è morto, riassegnato prima d'ogni uso): delega a sub_05b3 | verificata | 2 | titolo, gioco | titolo f1002, gioco f2402 (scrivono); il PC è attraversato anche dalla NMI via 05B3 |
| 0x0602 | sub_0602 | z80_0xxx.c | 476 | motore per-oggetto della NMI (0126, 24 slot c46e): moto sui 2 assi o ricarico campi dal record a746, margini da 9495, celle d'urto c0bd in +1A, poi sub_08a2 e sub_07de | verificata | 2 | titolo, gioco | titolo f1293, gioco f2402, con `SNAP_COND='b@hl>=c0'` (slot attivo); scrivono entrambe |
| 0x07DE | sub_07de | z80_0xxx.c | 196 | avanzamento dell'animazione di un record da 14 byte (tabelle ROM 9555) | verificata | 2 | titolo, gioco | titolo f1293, gioco f2402 (scrivono) |
| 0x08A2 | sub_08a2 | z80_08xx.c | 12 | smistamento d'urto: se bit 5 di (IX+0), E = C&0F sceglie il gestore nella tabella inline 08AE via 08CE (pop hl/push de/ret, statica nel C) | verificata | 2 | titolo, gioco_attiva | titolo f1293 (ret z), gioco f2402 con `SNAP_COND='(b@ix & 20)==20'` (gestore 08D8) |
| 0x08D8 | sub_08d8 | z80_08xx.c | 21 | gestore d'urto (indici 0/F): se 3 ≤ stato < 7, ≠ 6 e C&30 = 0 → stato 6 | verificata | 3 | gioco, gioco_scrive, tardo | gioco f2402, tardo f4002 (rami ret); ramo che scrive con `SNAP_COND='b@(ix+1d)>=3 && b@(ix+1d)<=5 && (c&30)==0'` f2448 |
| 0x094D | sub_094d | z80_08xx.c | 102 | gestore d'urto (indice 2): spinge l'oggetto fuori dalla cella sul lato di minor penetrazione (X o Y), azzera i sottopixel, stato e direzione | verificata | 2 | gioco, gioco_n2 | gioco f2403, occorrenza 2 f2767 (scrivono) |
| 0x0A98 | sub_0a98 | z80_08xx.c | 43 | gestore d'urto (indice 3): appoggio sul pavimento, Y alla base della riga meno i margini; se stato ≠ 2: stato 1 | verificata | 2 | gioco, gioco_n2 | gioco f3632, occorrenza 2 f3671 (scrivono) |
| 0x0AC3 | sub_0ac3 | z80_08xx.c | 54 | gestore d'urto (indice C): urto col soffitto, Y alla riga sotto, sottopixel f0; stato 1 e bit 1 direzione, o azzera (IX+0B)/(IX+0C) | verificata | 2 | gioco, tardo | gioco f2441, tardo f4005 (scrivono) |
| 0x0AF9 | sub_0af9 | z80_08xx.c | 94 | gestore d'urto (indice 9): urto sul lato sinistro, X alla colonna accanto, velocità X azzerata; stato 2 con direzione da bit 7 di (IX+0C), o (IX+7) = ff | verificata | 2 | gioco, tardo | gioco f2850, tardo f4460 (scrivono) |
| 0x0B57 | sub_0b57 | z80_08xx.c | 100 | gestore d'urto (indice 6): come 0AF9 sul lato destro, direzioni speculari | verificata | 2 | gioco, gioco_n2 | gioco f2588, occorrenza 2 f2639 (scrivono) |
| 0x08ED | sub_08ed | z80_08xx.c | 96 | gestore d'urto (indice 1): come 094D con la cella a destra, X corretta in avanti (stato 2, direzione 1) | tradotta (replay) | 0 | — | brief 26: eseguita dal replay, nessuna cattura |
| 0x09B3 | sub_09b3 | z80_08xx.c | 113 | gestore d'urto (indice 8): cella in alto a destra, X in avanti (direzione 1/3) o Y sotto la riga (stato 1) | tradotta (replay) | 0 | — | brief 26: eseguita dal replay |
| 0x0A24 | sub_0a24 | z80_08xx.c | 116 | gestore d'urto (indice 4): come 09B3 con la cella a sinistra (direzione 0/2) | tradotta (replay) | 0 | — | brief 26: eseguita dal replay |
| 0x0BBB | sub_0bbb | z80_08xx.c | 59 | gestore d'urto (indice 7, e da 0CDA): angolo in basso a destra, stato 2, direzione 0 | tradotta (replay) | 0 | — | brief 26: tradotta per completare la tabella 08AE, non in onda nel giro 2410-5499 |
| 0x0BF6 | sub_0bf6 | z80_08xx.c | 55 | gestore d'urto (indice B, e da 0CCE): angolo in basso a sinistra, stato 2, direzione 1 | tradotta (replay) | 0 | — | brief 26: eseguita dal replay via 0CCE |
| 0x0C2D | sub_0c2d | z80_08xx.c | 79 | gestore d'urto (indice D, e da 0CDA): angolo in alto a sinistra, Y sotto la riga; stato 1 direzione 3 o velocità azzerate | tradotta (replay) | 0 | — | brief 26: eseguita dal replay |
| 0x0C7C | sub_0c7c | z80_08xx.c | 82 | gestore d'urto (indice E, e da 0CCE): angolo in alto a destra; stato 1 direzione 2 o velocità azzerate | tradotta (replay) | 0 | — | brief 26: eseguita dal replay dal frame 2894 (prima era NULL e Dana non urtava) |
| 0x0CCE | sub_0cce | z80_08xx.c | 12 | gestore d'urto (indice A): 0C7C col bit 7 di (IX+0C), altrimenti 0BF6 | tradotta (replay) | 0 | — | brief 26: eseguita dal replay |
| 0x0CDA | sub_0cda | z80_08xx.c | 12 | gestore d'urto (indice 5): 0C2D col bit 7 di (IX+0C), altrimenti 0BBB | tradotta (replay) | 0 | — | brief 26: come 0BBB, non in onda |
| 0x0CE6 | sub_0ce6 | z80_08xx.c | 13 | stati della chiave: IX = c4ce, IY = c299, esce col bit 7 di c4ce spento (sempre nelle tracce); continuazione 0CF3 (`jp (hl)` sulla tabella 0D18) non tradotta: avviso su stderr | verificata | 2 | gioco, tardo | gioco f2402, tardo f4002; solo registri |
| 0x1288 | sub_1288 | z80_1xxx.c | 35 | classe della cella al centro di Dana (sub_36f0), esce se ≥ 37 (sempre nelle tracce); continuazione 12AB (raccolta, zona mai eseguita) non tradotta: avviso su stderr | verificata | 2 | gioco, tardo | gioco f2402, tardo f4002; solo registri |
| 0x234A | sub_234a | z80_2xxx.c | 9 | (HL) = C, (HL+1) = A; A = bit 6-7 di B (Lives, con B = DSW1 a nibble scambiati; da 2980) | verificata | 2 | titolo, titolo_n2 | occorrenze 1 (f152) e 2 (f378) al boot (scrivono) |
| 0x2368 | sub_2368 | z80_2xxx.c | 14 | HL = 12·byte(HL+A), DE = 2·byte | verificata | 2 | titolo, gioco | titolo f3698 (occorrenza 3, A=33), gioco f1855; solo registri |
| 0x237E | sub_237e | z80_2xxx.c | 35 | (HL) = ((HL)+1) & 7f; parola big-endian (HL+1) -= 0150 | verificata | 2 | titolo, gioco | titolo f1399, gioco f1776 (occorrenza 2) |
| 0x23A1 | sub_23a1 | z80_2xxx.c | 11 | parola (HL) += parola (DE), HL e DE += 2 | verificata | 2 | titolo, gioco | titolo f1399, gioco f1775 (occorrenza 2) |
| 0x23AC | sub_23ac | z80_2xxx.c | 15 | azzera (DE) e D:(E+20), DE++ | verificata | 2 | titolo, gioco | titolo f1189, gioco f1855 (scrive) |
| 0x23F8 | sub_23f8 | z80_2xxx.c | 10 | record di 3 byte a HL+3·A: DE = parola, C = byte, B = A = 0 | verificata | 2 | titolo, gioco | titolo f1189, gioco f1667; solo registri |
| 0x2402 | sub_2402 | routine.c | 9 | parola DE a HL, HL += 2, DE += BC, A-- | verificata | 2 | base, titolo | titolo f156 (occorrenza 5, scrive E408) |
| 0x240B | sub_240b | z80_2xxx.c | 13 | HL = 48·A, BC = A | verificata | 2 | titolo, gioco | titolo f3407 (occorrenza 2, A=33), gioco f1564; solo registri; il brief diceva 24·A, il codice fa 48·A |
| 0x2418 | sub_2418 | z80_2xxx.c | 13 | HL = 32·D + E (cella di griglia), D = 0 | verificata | 2 | titolo, gioco | titolo f1292, gioco f2419; solo registri |
| 0x2457 | sub_2457 | z80_2xxx.c | 29 | D = (E>>4)·2+2, E = A = (E&0f)·2-1 | verificata | 2 | titolo, gioco | titolo f1292, gioco f2419; solo registri |
| 0x249E | sub_249e | z80_2xxx.c | 19 | (HL) += C, parola (HL+3) += D:C, (HL+5) += C, HL += 5 + DE | verificata | 2 | gioco, tardo | gioco f2402, tardo f4002 (scrive) |
| 0x24B1 | sub_24b1 | z80_2xxx.c | 13 | (HL+1..3) = E, A ruotato col riporto, C; E = B, B = C = C+1 | verificata | 2 | gioco, tardo | gioco f2402 (scrive), tardo f4002; usa il riporto in ingresso |
| 0x2551 | sub_2551 | z80_2xxx.c | 10 | incrementa il contatore a DE+1 e lo confronta col byte del record di 3 byte all'indice BC della tabella HL (anche ROM, il chiamante 7650 passa 765B); riporto d'uscita = byte ≥ contatore, emulato su z->f | verificata | 2 | titolo, titolo_n2 | occorrenze 1 (f1060) e 2 (f1125); scrivono |
| 0x2980 | sub_2980 | z80_2xxx.c | 71 | azzera c046-c04b e estrae i campi di e604 (DSW1): crediti Coin A/B in c04a-c04b (tabella 29C7 via sub_234a), 0x80 in c048-c049 se Coin A = Coin B, vite in c04d (tabella 29CB) | verificata | 2 | titolo, titolo_n2 | occ 1 f152, occ 2 f378 (scrivono); blocco esplicito c046-c04d |
| 0x2AE2 | sub_2ae2 | z80_2xxx.c | 19 | disegna gli 8 bit di e604 (DSW1) in d765 e di e605 (DSW2) in d785 (riga di servizio) via sub_2af5; il riporto fra le due chiamate è il bit 7 di DSW1 | verificata | 2 | titolo, titolo_n2 | occ 1 f153, occ 2 f154 |
| 0x2AF5 | sub_2af5 | z80_2xxx.c | 19 | 8 righe di testo via sub_2b08 (stringa 0c/0d dal bit di A ruotato col riporto in ingresso) | verificata | 2 | titolo, titolo_n2 | solo titolo/test RAM (s = g = 0): occorrenze 1 (f153) e 2 (f154) |
| 0x2B08 | sub_2b08 | z80_2xxx.c | 33 | stringa C (tab. 2B61) nella destinazione B (tab. 2B29, o HL col bit 7 di B), attributo 40 | verificata | 2 | titolo, titolo_n2 | solo titolo/test RAM (s = g = 0): occorrenze 1 (f152) e 2 (f153) |
| 0x2BC3 | sub_2bc3 | z80_2xxx.c | 27 | lettura ingressi di avvio/attract (coda esclusiva 2353): A = ((e600 P1 \| e601 P2) & 3f) \| (e602 SYSTEM & 3) | tradotta | 1 | titolo | titolo f153; solo registri (non scrive c000-e5ff), gira solo in attract (t/s/g 71135/0/0): le occorrenze N=1..5 danno `prima` e regs identici (attract deterministico, porte a riposo) |
| 0x2BC9 | sub_2bc9 | z80_2xxx.c | 46 | test RAM d'avvio a nibble: pattern rotanti 1..80 (riempimento sub_2c1a), rilettura a coppie byte/nibble; riporto d'uscita = esito, emulato su F | tradotta | — | — | non registrata nel banco: la routine testa tutta la RAM, dura molti frame e la NMI la attraversa sempre, snap.lua scarta ogni occorrenza (come lo smistatore 08CE del lotto 7, non catturabile). Logica identica a sub_2bf3 (verificata) salvo il confronto a nibble |
| 0x2BF3 | sub_2bf3 | z80_2xxx.c | 39 | test RAM d'avvio a byte intero (come 2BC9), lettura a vuoto di e602 per byte (watchdog) | verificata | 2 | titolo, titolo_n2 | occ 1 f34, occ 2 f67 (siti 2843/284F, prima dell'abilitazione NMI; scrivono) |
| 0x2C1A | sub_2c1a | z80_2xxx.c | 13 | riempie BC byte da HL col pattern rotante D, lettura a vuoto di e602 per byte (watchdog); l'ingresso è anche il ciclo | verificata | 2 | titolo, titolo_n2 | occ 1 f3, occ 2 f4 (scrivono) |
| 0x2E11 | sub_2e11 | z80_2xxx.c | 13 | c19e = 0 e confronto della stanza corrente c248 con c249 (Z d'uscita emulato su F per il chiamante) | tradotta | 2 | titolo, gioco | titolo f1305, gioco f1855; scrittura sempre banale (c19e già 0 anche con N=2; tardo e N=3 senza occorrenze): ramo non banale non catturato |
| 0x2E90 | sub_2e90 | z80_2xxx.c | 12 | cancello a conto alla rovescia: dec (HL); se c467 ≥ C azzera c467 e rincrementa (HL) | verificata | 2 | titolo, gioco | titolo f1196, gioco f1702 (scrivono) |
| 0x2F57 | sub_2f57 | z80_2xxx.c | 29 | carico del fondale di gioco: sub_3f88 (HL = 80D3), azzera d5c0-d61f, sub_4398 sui gestori 1 e 0, c467 = 0 | verificata | 2 | titolo, gioco | titolo f1292, gioco f1667 (scrivono) |
| 0x2F74 | sub_2f74 | z80_2xxx.c | 27 | aggancia una cella a un oggetto: H/L in (IX+0E)/(IX+15), (IX+3)=0a, sub_4af6 (A=c0, E=0a), poi L+=10 e IX+=30 | verificata | 2 | titolo, gioco | titolo f1464, gioco f1838 (scrivono) |
| 0x2FF3 | sub_2ff3 | z80_2xxx.c | 13 | HL e DE = differenze di coordinate ×4 col segno (prestito a sub_3000) | verificata | 2 | titolo, gioco | titolo f1305, gioco f1774; solo registri |
| 0x3000 | sub_3000 | z80_3xxx.c | 9 | HL = A·4 con segno dal riporto in ingresso | verificata | 2 | titolo, gioco | titolo f1305, gioco f1774; solo registri; usa il riporto in ingresso |
| 0x3045 | sub_3045 | z80_3xxx.c | 26 | inizializza i 16 oggetti c46e con sub_4af6 (A=45, E=40, B=4, C=0) | verificata | 2 | titolo, gioco | titolo f1399, gioco f1774 (scrivono) |
| 0x3097 | sub_3097 | z80_3xxx.c | 88 | 16 oggetti c46e su un cerchio (sub_30ef + sub_3101, centro c2c2/c2c4, raggio c2c6) | verificata | 2 | titolo, gioco | titolo f1399, gioco f1774 (scrivono) |
| 0x30EF | sub_30ef | z80_3xxx.c | 18 | A = tabella ROM 3111 a specchio su 64 passi | verificata | 2 | titolo, gioco | titolo f1399 (A=00), gioco f1775 (occorrenza 2, A=20); solo registri |
| 0x3101 | sub_3101 | z80_3xxx.c | 16 | byte alto di H·L (moltiplicazione seriale), bit bassi in L | verificata | 2 | titolo, gioco | titolo f1399, gioco f1774; solo registri |
| 0x31CE | sub_31ce | z80_3xxx.c | 19 | scritta fissa 31E1 a dfa7 via sub_6771, poi cifra c238 in ASCII a dfac | verificata | 2 | titolo, gioco | titolo f1189, gioco f1564 (scrivono) |
| 0x31E9 | sub_31e9 | z80_3xxx.c | 53 | barra di min(A,6) colonne (tile 80/81, attr 60) a df80, resto pulito con sub_23ac | verificata | 2 | titolo, gioco | titolo f1189, gioco f1855 (scrivono) |
| 0x322A | sub_322a | z80_3xxx.c | 52 | fondale completo della stanza c243: sub_33aa, 0 su e802 (`scrivi_porta`, non mappato in MAME), flusso del record F068+4·c243 a sub_326c (BC=1000), sovrapposizioni (byte alto e basso della parola 2) a sub_325e, palette con sub_33ba (C=1) | tradotta | — | — | non registrata nel banco: carico di quadro più lungo di un frame, la NMI attraversa ogni occorrenza e snap.lua le scarta tutte (come 2BC9 e 75AA) |
| 0x325E | sub_325e | z80_3xxx.c | 14 | sovrapposizione A del fondale: record F138 (flusso, C, B) a sub_326c | verificata | 2 | titolo, gioco | titolo f1195, gioco f1541 (occorrenza 2) |
| 0x326C | sub_326c | fondale.c | 238 | decodifica del fondale (callee 23BB, 23CB, 23EC, 336C) | verificata | 2 | base, gioco | gioco f1543 |
| 0x33AA | sub_33aa | z80_3xxx.c | 16 | azzera la palette di stanza c9ca-ca49, bit 6 di c044 | verificata | 2 | titolo, gioco | titolo f3409 (occorrenza 2), gioco f1534 |
| 0x33BA | sub_33ba | z80_3xxx.c | 54 | record palette F000[c243] a c9ca (ldir); passata rld col bit 0 del C d'ingresso | verificata | 2 | titolo, gioco | titolo f1196, gioco f1856 |
| 0x33EB | sub_33eb | z80_3xxx.c | 25 | azzera c237-c267, c244 = ff, copia 6 byte ROM 3427 in c258; prosegue senza ret in sub_3404 | verificata | 2 | titolo, gioco | titolo f1189, gioco f1563 (scrivono) |
| 0x3404 | sub_3404 | z80_3xxx.c | 35 | coda di 33EB: c237 = c04d, tabellone (sub_349f), puntatore Bonus Life (sub_342d); se c045&6 azzera dc55-dc57; c24e = 4 | verificata | 2 | titolo, gioco | titolo f1189, gioco f1563 (scrivono); mai chiamata: cattura dal PC d'ingresso, rientro da (SP) |
| 0x342D | sub_342d | z80_3xxx.c | 18 | e605 (DSW2, bit 5-7 = Bonus Life): copia la parola della tabella 343F in c23d-c23e | verificata | 2 | titolo, gioco | titolo f1189, gioco f1563 (scrivono) |
| 0x349F | sub_349f | z80_3xxx.c | 53 | tabellone: pulizia (sub_34d4) e caratteri 82+2·(coppie di bit di c25c) con sub_34e1, chiusi da 90/92 | verificata | 2 | titolo, gioco | titolo f1189, gioco f1563 (scrivono) |
| 0x34D4 | sub_34d4 | z80_3xxx.c | 13 | azzera 10 byte da HL in due piani (bit 2 di H) | verificata | 2 | titolo, gioco | titolo f1190 (occorrenza 2), gioco f1563 (scrive) |
| 0x34E1 | sub_34e1 | z80_3xxx.c | 25 | caratteri A e A+1 in colonna (righe da 0x20) a df00+L, attributo 70 | verificata | 2 | titolo, gioco | titolo f1189, gioco f1563 (scrivono) |
| 0x354B | sub_354b | z80_3xxx.c | 18 | copia il record di 4 byte della stanza-bonus c24a (tabella 355D) via sub_3569 con A = 0 | verificata | 2 | titolo, gioco | titolo f1189, gioco f1564 (scrivono) |
| 0x3569 | sub_3569 | z80_3xxx.c | 11 | copia generica: destinazione e lunghezza dal record A della tabella 3574 (sub_23f8), sorgente HL (anche ROM) | verificata | 2 | titolo, gioco | titolo f1189, gioco f1667 (scrivono) |
| 0x3678 | sub_3678 | z80_3xxx.c | 59 | palette e400 con 4 passate di sub_36bd e 10 byte ROM 36B3 a e496 | tradotta | 1 | titolo | solo titolo f152, occorrenza unica entro 6000 frame (N=2 e gioco falliti) |
| 0x36BD | sub_36bd | z80_3xxx.c | 6 | ripete sub_2402 finché A non arriva a 0 | verificata | 2 | titolo, titolo_n2 | solo titolo (s = g = 0): occorrenze 1 (f152) e 2 (f153) |
| 0x36C3 | sub_36c3 | z80_3xxx.c | 45 | 4 ldir da ROM: 80D3→e400 (0x200), 8199→c96e, 80D3→c994, 82A7→c9b4 | tradotta | 1 | titolo | solo titolo f603, occorrenza unica entro 6000 frame (N=2 e gioco falliti) |
| 0x36F0 | sub_36f0 | z80_3xxx.c | 25 | pixel (H, L) → cella della griglia FG: HL = c0bd+offset, DE = offset | verificata | 2 | gioco, tardo | gioco f2402, tardo f4002; solo registri |
| 0x380A | sub_380a | z80_3xxx.c | 8 | 16 byte 0xff da HL | verificata | 2 | titolo, gioco | titolo f1191 (occorrenza 2), gioco f1564; scrivono entrambe |
| 0x3812 | sub_3812 | z80_38xx.c | 23 | espande una mappa di bit di 24 righe per 8 colonne da IX nella griglia c0cd: scrive C dove il bit è 1 | verificata | 2 | titolo, gioco | titolo f1190, gioco f1564 (scrivono) |
| 0x3A1C | sub_3a1c | z80_3xxx.c | 18 | due cifre ASCII di A in D, E; A = D | verificata | 3 | titolo, gioco, titolo_decine | titolo f1189, gioco f1546 (occorrenza 5), A = 01; ramo decine catturato con `SNAP_COND='a>=#10'` in titolo f3406 (A = 0C) |
| 0x3A2E | sub_3a2e | z80_3xxx.c | 38 | coppia B, C dalle tabelle ROM 8578/858E secondo A | verificata | 2 | titolo, gioco | titolo f1483 (occorrenza 3, A=B7), gioco f1856 (A=01); solo registri |
| 0x3A54 | sub_3a54 | z80_3xxx.c | 41 | riquadro di 2×2 tile alla cella (D=riga, E=colonna): tile B..B+3 sul piano d400 e attributo C nelle 4 posizioni del piano d000; uscita B += 3, A −= 3, HL e DE conservati | verificata | 2 | titolo, gioco | titolo f1292, gioco f2419 (scrivono) |
| 0x3F10 | sub_3f10 | z80_3xxx.c | 41 | tempo BCD c093-c095 in 6 cifre a c061, terminatore ff | verificata | 2 | titolo, gioco | titolo f1196, gioco f2403 |
| 0x3F88 | sub_3f88 | z80_38xx.c | 11 | copia il record 6 della tabella 3574 da HL (sub_3569) e accende il bit 6 di c044 | verificata | 2 | titolo, gioco | titolo f1189, gioco f1667 (scrivono) |
| 0x4398 | sub_4398 | z80_4xxx.c | 33 | record 43B9/43C5 (bit 4 di A): OR del bit in c09b/c0b5, (c464) alla parola, 0 alla parola+2 | verificata | 2 | titolo, gioco | titolo f1292, gioco f1667 (scrivono) |
| 0x452B | sub_452b | z80_4xxx.c | 67 | canale di coppie (registro, valore): record da 3 byte, lista con riavvolgimento su ff | verificata | 2 | titolo, gioco | titolo f1296, gioco f2403 (scrivono) |
| 0x4722 | sub_4722 | z80_4xxx.c | 12 | C = A = 7 + soglie della tabella ROM 9484 minori o uguali ad A | verificata | 2 | titolo, gioco | titolo f1481 (occorrenza 2), gioco f1855; solo registri |
| 0x472E | sub_472e | z80_4xxx.c | 57 | inizializza il record C (tab. 488D) e l'oggetto IX (tab. 48AF); parola 8B2D scelta da c2ae | verificata | 2 | titolo, gioco | titolo f1480, gioco f1855 |
| 0x4AF6 | sub_4af6 | z80_4xxx.c | 24 | inizializza IX+00,04,05,1B,1C,1D,1E di un oggetto | verificata | 2 | titolo, gioco | titolo f1292, gioco f2406 |
| 0x4B0E | sub_4b0e | z80_4xxx.c | 18 | cella (riga:colonna in A) in pixel: H = y, L = x | verificata | 2 | titolo, gioco | titolo f1190, gioco f2406; solo registri |
| 0x4D05 | sub_4d05 | z80_4xxx.c | 36 | cifre c239/c23b a c061 (sub_4d37) e a video via sub_4d87 se c045 & 6 | verificata | 2 | titolo, gioco | gioco f1563, titolo f2585 (demo); tardo senza occorrenze entro 6000 frame |
| 0x4D37 | sub_4d37 | z80_4xxx.c | 80 | DE:HL a 32 bit in cifre decimali a c061, terminatore ff | verificata | 2 | titolo, gioco | gioco f1563, titolo f2585 (demo); tardo e gioco N=2 senza occorrenze entro 6000 frame |
| 0x4D87 | sub_4d87 | z80_4xxx.c | 27 | 8 byte da (DE) all'indietro in (HL) due piani (+0x30, attributo C), zeri dopo ff | verificata | 2 | gioco, tardo | gioco f2403, tardo f4004 (scrivono) |
| 0x4DEE | sub_4dee | z80_4xxx.c | 67 | cambio di sequenza di animazione negli slot c2c1 (tabella 83FA) | verificata | 2 | gioco, tardo | gioco f2461 con `SNAP_COND='b@(hl+1)!=((a-1)&ff)'` (ramo principale, scrive), tardo f4002 (`ret z`) |
| 0x4E31 | sub_4e31 | z80_4xxx.c | 86 | x, y degli oggetti c4fe negli slot c2c1, direzioni in c2b5; chiama sub_24b1 | verificata | 2 | gioco, tardo | gioco f2402, tardo f4002 (scrivono entrambe); riporto di `sub d` passato a sub_24b1 |
| 0x4E87 | sub_4e87 | z80_4xxx.c | 95 | passo c2ab da c2a9, bit delle righe a c2ac, accende bit 6-7 di c459 | verificata | 2 | gioco, tardo | gioco f2461 con `SNAP_COND='(((w@c2a9)*4/100)&ff)!=(b@c2ab)'` (scrive c2ab), tardo f4002 (`ret z`) |
| 0x5507 | sub_5507 | z80_5xxx.c | 19 | prossimità per i corpi di fase dei nemici: se il doppio del valore assoluto di (ix+4) è ≥ C esce col riporto 0; altrimenti riporto dal bit 7 di ((iy+1e) ruotato XOR (ix+5)), emulato su z->f | verificata | 2 | gioco, tardo | gioco f2462, tardo f4002; solo registri |
| 0x5C32 | sub_5c32 | z80_5xxx.c | 70 | classi (sub_5c78) delle 4 celle attorno all'oggetto IY in c07e-c081 (cella da sub_36f0) | verificata | 2 | gioco, tardo | gioco f2462, tardo f4002 (scrivono) |
| 0x5C78 | sub_5c78 | z80_5xxx.c | 22 | classe del byte a DE in C (0..3) | verificata | 2 | gioco, tardo | gioco f2462, tardo f4002 |
| 0x673F | sub_673f | z80_6xxx.c | 11 | azzera la tabella attività c000-c03f | verificata | 2 | titolo, gioco | titolo f3403, gioco f1534 |
| 0x6771 | sub_6771 | z80_6xxx.c | 20 | stringa fino a ff da HL+1 in due piani da DE, attributo (HL) | verificata | 2 | titolo, gioco | titolo f1189, gioco f1534 (scrivono); estensione reale 6771-6784 |
| 0x6C2C | sub_6c2c | z80_6xxx.c | 51 | primo passo d'inizializzazione di un oggetto: `set 5,(ix+1)`, posa A in (iy+1d), (ix+0d) = 0, direzione dal bit 2 di (iy+4) in (iy+1e); i passi successivi (6C48-6C5E, `set 0` o suono 30 via `rst 28`) mai eseguiti nelle tracce: dal dasm statico | verificata | 2 | gioco, gioco_n2 | occorrenze 1 e 2 (f1891, slot diversi); scrivono |
| 0x7379 | sub_7379 | z80_7xxx.c | 30 | primo slot libero (bit 7 di +0 spento) dei 17 slot c2c1 dall'ultimo (c441, passo −0x18): azzera il byte +1, A = indice, HL = slot, riporto 1 su z->f; esauriti (738A, mai eseguito nelle tracce): riporto 0; coda privata 2540 | tradotta | 2 | titolo, gioco | titolo f1480, gioco f2428; scrittura sempre banale (il byte +1 dello slot libero è già 0; provato N=1..3 gioco, 1..2 titolo, 1 tardo): ramo non banale non catturabile |
| 0x75AA | sub_75aa | z80_7xxx.c | 61 | azzeramenti di cambio quadro: piani d480/d080 (0x300 byte), c299-c2a8, bit 0 di c258, slot oggetti c46e-c8ed, slot c2c1-c458 | tradotta | — | — | non registrata nel banco: azzera circa 0x1000 byte, dura molti frame e la NMI la attraversa sempre, snap.lua scarta ogni occorrenza (come 08CE del lotto 7, non catturabile) |
| 0x797E | sub_797e | z80_7xxx.c | 38 | due cifre BCD di (HL) in ASCII a (DE) e (DE+8); il ramo oltre '1' (0x73 a DE+6, passo 2) non è mai eseguito: dal dasm statico | tradotta | 1 | titolo | titolo f612, occorrenza unica nelle tracce (t/s/g 1/0/0) |
| 0x7C84 | sub_7c84 | z80_7xxx.c | 20 | azzera le due righe del pannello ddaa-ddb7 e ddca-ddd7 | verificata | 2 | titolo, gioco | titolo f1188, gioco f1534 (scrivono) |
| 0x7DEA | sub_7dea | z80_7xxx.c | 25 | le tre stringhe dell'esito di fine partita via sub_7e1d (A=3/4/5) | tradotta | 1 | gioco | gioco f1542, occorrenza unica entro 6000 frame (N=2, titolo e tardo falliti) |
| 0x7E1D | sub_7e1d | z80_7xxx.c | 7 | stringa d'esito: destinazione dalla tabella 7E3A (sub_7e32) e sub_6771 | verificata | 2 | gioco, gioco_n2 | occorrenze 1 (f1542) e 2 (f1543); tardo senza occorrenze entro 6000 frame |
| 0x7E24 | sub_7e24 | z80_7xxx.c | 14 | come 7E1D più un carattere saltato e la stringa fissa 7EA3 | verificata | 2 | gioco, gioco_n2 | occorrenze 1 (f1542) e 2 (f1543); tardo senza occorrenze entro 6000 frame |
| 0x7E32 | sub_7e32 | z80_7xxx.c | 8 | DE = parola della tabella 7E3A all'indice A, HL conservato | verificata | 2 | gioco, gioco_n2 | occorrenze 1 (f1542) e 2 (f1543); tardo e titolo senza occorrenze; solo registri |
| 0x7ECA | sub_7eca | z80_7xxx.c | 52 | cornice del pannello di gioco: caratteri 04-11 a ddaa e 12-1F a ddca (sub_7f2d), bordi a celle con sub_7efe (attributo 2 su 11 righe × 16 colonne, attributo 3 su 11 × 5) | tradotta | — | — | non registrata nel banco: carico di quadro più lungo di un frame, la NMI attraversa ogni occorrenza e snap.lua le scarta tutte (come 322A) |
| 0x7EFE | sub_7efe | z80_7xxx.c | 47 | cella in d486/d496 da D, E, C: A e C in due piani | verificata | 2 | titolo, gioco | titolo f610, gioco f1544 (occorrenza 5); scrivono entrambe |
| 0x7F2D | sub_7f2d | z80_7xxx.c | 12 | B caratteri consecutivi a partire da C da HL in avanti (attributo 60 sull'altro piano); uscita C += B, B = 0, HL avanzato | verificata | 2 | titolo, gioco | titolo f610, gioco f1540 (scrivono) |

## Corpi di attività (lotto 23)

Ingressi della tabella 3B11, raggiunti con `rst 20` dallo spedidore e non con
`call`: non fanno parte delle 125 routine sopra. Stanno in
`harness/attivita.c` (`corpo_attivita_replay[]`) e si verificano col replay
(`REPLAY.md`), non con le catture. Stato «tradotta (replay)» = C scritto e
confrontato col giro. Dopo i corpi 01 e 10 (brief 26) il giro da F0 =
2410 arriva in fondo all'oracolo (frame 5499) con tutte le NMI pulite
coincidenti. 01 e 10 stanno in `harness/attivita_01_10.c`.

| codice | ingresso | funzione | descrizione | stato |
|---|---|---|---|---|
| 02 | 0x4B84 | attivita_02_4b84 | attende c463 ≥ 0e, chiude l'oggetto c46e (4B98, statica), diventa 03 | tradotta (replay) |
| 03 | 0x4BC6 | attivita_03_4bc6 | attende c463 ≥ 0e, riaccende Dana dalle copie +29..+2c, si spegne | tradotta (replay) |
| 0D | 0x5C8E | attivita_0d_5c8e | cancello c4ce: contatore c29a/c29b (250E, statica) contro c25a, posa 2 o chiusura col tabellone | tradotta (replay) |
| 0E | 0x63A4 | attivita_0e_63a4 | ingresso nel cancello: diventa 03, oppure apre c4ce sopra Dana e diventa 0F | tradotta (replay) |
| 11 | 0x456E | attivita_11_456e | attende c466 ≥ 6, azzera c49e, si spegne | tradotta (replay) |
| 14 | 0x44B5 | attivita_14_44b5 | canali di registri c09d (4) e c0b7 (1) via sub_452b | tradotta (replay) |
| 20 | 0x3E90 | attivita_20_3e90 | il tempo: sottrazione BCD da c093-c095, cifre e colore a d472, fine tempo | tradotta (replay) |
| 01 | 0x48D1 | attivita_01_48d1 | cella davanti a Dana (c82e) secondo (IY+2C) e (IY+29): 49AE se < 80 (slot = esito, 25 con A = 4 e B = 0), altrimenti 4938 e slot 02 | tradotta (replay) |
| 0A | 0x4DA2 | attivita_0a_4da2 | `rst 38` su c2b8 verso le fasi 4DAD / 4632 / 4EE6 del motore dei nemici (`nemici.c`, lotto N1) | tradotta (replay) |
| 10 | 0x457B | attivita_10_457b | 4592 con IY = c82e e IX = c49e: riporto acceso → slot 0, spento → c466 = 0 e slot 11 | tradotta (replay) |

## Motore dei nemici (lotto N1)

Corpo 0A e routine che il replay esegue (brief 25, `CENSIMENTO-NEMICI.md`).
Stanno in `harness/nemici*.c` e si verificano col giro. Dal brief 27 i
moduli `nemici*.c` (con `attivita.c` e `attivita_01_10.c`) entrano anche
nel banco `harness`: le routine N1 restano «tradotta (replay)» finché non
hanno catture proprie, quelle dei lotti N2-N12 si verificano con le
catture (sezione «lotto N2» sotto). Prototipi in `harness/nemici.h`;
il riporto, dove il chiamante lo legge, è il bit 0 di `z->f`. Le 27
routine non tradotte che N1 chiama o smista erano stub (25 dopo il lotto
N2, che ha tradotto 6CE8 e 6ECA) in `nemici_stub.c`
(`STUB_NEMICI`): il replay si ferma con `ROUTINE xxxx mancante`. Stato
«tradotta (replay)»: il giro coincide con MAME a ogni NMI pulita; «non in
onda» = nessuna esecuzione nel giro (gcov). Dopo il brief 26 il giro va da
2410 a 5499 e 2482, 248D, 24BE, 4592, 49AE, 4B20, 4C11 e 55C9 vanno in
onda (in parte: 49AE al 27% delle righe, 4592 e 4B20 a circa metà).
`nemici.h` dichiara anche `NEMICI_PUNTO(da, a)`, il punto di sincronia
per la NMI del replay (4EE6 fra due slot, 58DF, 5928; `REPLAY.md`).

| ingresso | funzione | modulo | descrizione | stato |
|---|---|---|---|---|
| 0x4DAD | sub_4dad (statica) | nemici.c | fase 0: c2b7 sui 17 slot (249E), conta in c2af, 4E87, 4E31, 4DEE | tradotta (replay) |
| 0x4632 | sub_4632 (statica) | nemici.c | fase 1: con c459 bit 5 e bit 6-7, `rst 20` a 4644 (46A3, 4649) | tradotta (replay); `rst 20` non in onda |
| 0x4EE6 | sub_4ee6 (statica) | nemici.c | fase 2: 4F0F per ogni oggetto c4fe con (IY+0) ≥ c0, fase a 0 | tradotta (replay) |
| 0x4F0F | sub_4f0f | nemici.c | gestore per tipo: `rst 38` a 4F1B su (IY+3) − 6 se (IY+4) ≥ 6 | tradotta (replay) |
| 0x3B62 | sub_3b62 | nemici.c | `ret` solo, voce nulla delle tabelle | tradotta |
| 0x2482 | sub_2482 | nemici_comuni.c | HL = 32·D + E | tradotta (replay) |
| 0x248D | sub_248d | nemici_comuni.c | C nel blocco 2×2 a HL | tradotta (replay) |
| 0x24BE | sub_24be | nemici_comuni.c | da coordinate di schermo a cella | tradotta (replay) |
| 0x4592 | sub_4592 | nemici_comuni.c | presa del blocco sotto l'oggetto IY, riporto = fallita | tradotta (replay) |
| 0x4938 | sub_4938 | nemici_comuni.c | posa dell'oggetto IX sulla cella E di valore A | tradotta (replay) |
| 0x49AE | sub_49ae | nemici_comuni.c | posa con contatti (c4ce, Dana e oggetti), A = esito | tradotta (replay) |
| 0x4B20 | sub_4b20 | nemici_comuni.c | contatto fra l'oggetto IX e la posizione H, L (riporto) | tradotta (replay) |
| 0x4C11 | sub_4c11 | nemici_comuni.c | carattere B e attributo C nel blocco 2×2 della cella D, E | tradotta (replay) |
| 0x55B2 | sub_55b2 | nemici_tipi_10_13.c | tipo 12: `rst 38` a 55B5 su (IY+1D) | tradotta (replay) |
| 0x55C9 | sub_55c9 | nemici_tipi_10_13.c | tipo 12, fase 4: cammino dopo l'attesa | tradotta (replay) |
| 0x561B | sub_561b | nemici_tipi_10_13.c | tipo 12, fase 2: inversione, fase 5 | tradotta (replay) |
| 0x5640 | sub_5640 | nemici_tipi_10_13.c | tipo 12, fase 5: cammino, vicinanza (5507) | tradotta (replay) |
| 0x5693 | sub_5693 | nemici_tipi_10_13.c | tipo 12, fase 0: presa o chiusura del blocco davanti | tradotta (replay) |
| 0x714B | sub_714b | nemici_tipi_10_13.c | tipo 10: `rst 38` a 714E su (IY+1D) | tradotta (replay) |
| 0x7155 | sub_7155 | nemici_tipi_10_13.c | tipo 10, fase 0: velocità dalla tabella 7196 | tradotta (replay) |
| 0x7235 | sub_7235 | nemici_tipi_10_13.c | tipo 10, fase 2: cammino con la cella davanti (734D) | tradotta (replay) |
| 0x58CF | sub_58cf | nemici_tipo09.c | l'oggetto della tabella 48AF si posa sul blocco davanti (4938 / 49AE) | tradotta (replay) |
| 0x5917 | sub_5917 | nemici_tipo09.c | chiusura dell'oggetto della tabella 48AF (4B98) | tradotta (replay) |
| 0x5928 | sub_5928 | nemici_tipo09.c | cella davanti all'oggetto IY secondo (IY+1E) | tradotta (replay) |
| 0x734D | sub_734d | nemici_comparsa.c | cella vicina in una delle quattro direzioni (scostamenti B, C) | tradotta (replay) |
| 0x7F51 | sub_7f51 | nemici_tipi_15_17.c | tipo 17, fase 4 di 7F39, poi 809C | tradotta (replay) |
| 0x7F68 | sub_7f68 | nemici_tipi_15_17.c | tipo 17: `rst 38` a 7F39 ((IY+4) = a1) o a 7F72 | tradotta (replay) |

## Motore dei nemici (lotto N2)

Tipi 8 (6ECA) e 14 (6CE8) del gestore 4F1B (brief 27), in
`harness/nemici_tipi_08_14.c`, più 6 dipendenze anticipate dai lotti N3-N5
nei moduli del censimento. Si verificano nel banco: i moduli `nemici*.c`
entrano in `harness` dal brief 27. Catture con lo stato `stanzaNN` di
`cattura.sh` (stanza NN forzata, primo frame 1951); i suffissi `faseN`,
`fermo`, `attesa`, `chiude`, `libera`, `dir1`, `vert`, `vertok` sono catture
con `SNAP_COND` sui campi di IX/IY o sui registri (es. `b@(iy+1d)==3`),
`gN`/`nN` occorrenze successive. Le catture `forza*` partono da uno
**stato forzato** con `SNAP_FORZA` (MAME esegue il codice vero da lì) e
non contano per lo stato `verificata`, che qui si regge sempre su almeno
due catture non forzate:
- `0x571D stanza10_forza`: `ix+d:0x40`, apre il salto a 80A5;
- `0x6CE8 stanza10_forza`: `ix+c:ff`, 57AA senza riporto → 57B3 (mai
  vista nelle stanze 10, 14, 24);
- `0x6D1B stanza10_forza`: `iy+4:78`, ramo col bit 3 di (IY+4);
- `0x6E55 stanza10_forza`: `ix+5:0,ix+4:0`, apre 6E83 (mai visto);
- `0x6EA1 stanza10_forza`: `ix+1:08`, ramo col bit 3 di (IX+1);
- `0x6F91 stanza08_forza6fc3`: `SNAP_COND='(c&2)==0'`, `e:0x89`, apre 6FC3;
- `0x6F91 stanza08_forza6fdf`: `SNAP_COND='(c&2)==2'`, `d:0x81`, apre 6FDF;
- `0x6FED stanza08_forza`: `c:00`, ramo col bit 1 di C spento.

Tutte le forzature hanno aperto il ramo voluto (gcov). 6E83 e 6E8B sono
etichette dentro `sub_6e55`, 6FC3 e 6FDF statiche (raggiunte solo da
6F91), la coda 7056-70A7 statica, chiamata da `sub_6eca` dopo la
destinazione di 6F1C. Righe di `nemici_tipi_08_14.c` mai eseguite dalle
catture: le uscite fuori tabella e i `default` degli smistamenti, 6D7C
e 6D88 (nessuno slot libero), 6E3D (cella davanti senza blocco in 6DCE),
6E6D (cella libera in 6E55) e cinque voci dello `switch` di 6F1C dentro
`sub_6eca` (le destinazioni hanno catture proprie).

| ingresso | funzione | file | byte | descrizione | stato | catture OK | stati | note |
|---|---|---|---:|---|---|---|---|---|
| 0x571D | sub_571d | nemici_tipi_10_13.c | 9 | tipo 14, fase 0 (6D03[0]): a (IX+0D) ≥ 40 chiude slot e oggetto (80A5); dipendenza anticipata da N5 | verificata | 3 | stanza10 | stanza10 f3025; stanza10_forza f3025; stanza10_n2 f3027 — stanza10_forza: stato forzato |
| 0x57AA | sub_57aa | nemici_tipi_10_13.c | 9 | HL = (IX+0B..0C) − BC − riporto, riporto del prestito; anticipata da N5 | verificata | 2 | stanza10, stanza14 | stanza10 f2002; stanza14 f2001 |
| 0x57B3 | sub_57b3 | nemici_tipi_10_13.c | 26 | senza il bit 3 di (IX+1): bit 0 di A → bit 0 di (IY+0), fase 0, azzera (IX+0B..0D); anticipata da N5 | verificata | 2 | stanza10, stanza11 | stanza10 f3025; stanza11 f2049 |
| 0x6CE8 | sub_6ce8 | nemici_tipi_08_14.c | 28 | tipo 14: 57AA con BC = c1a6/c1a8 (bit 3 di (IY+4)), 57B3 se NC, `rst 38` a 6D03 su (IY+1D) | verificata | 9 | stanza10, stanza14, stanza24 | stanza10 f2002; stanza10_fase0 f3026; stanza10_fase1 f2022; stanza10_fase4 f2575; stanza10_forza f2002; stanza14 f2001; stanza14_fase5 f2022; stanza24_fase2 f2296; stanza24_fase3 f2031 — stanza10_forza: stato forzato |
| 0x6D12 | sub_6d12 | nemici_tipi_08_14.c | 9 | tipo 14, fase 3: (IY+12) = 5, fase 5 | verificata | 2 | stanza10, stanza14 | stanza10 f2022; stanza14 f2021 |
| 0x6D1B | sub_6d1b | nemici_tipi_08_14.c | 28 | tipo 14, fase 1: (IX+0B..0C) = 0 col bit 3 di (IY+4), direzione dal segno di (IX+5), fase 3 | verificata | 3 | stanza10, stanza14 | stanza10 f2022; stanza10_forza f2022; stanza14 f2021 — stanza10_forza: stato forzato |
| 0x6D37 | sub_6d37 | nemici_tipi_08_14.c | 151 | tipo 14/15, fase 2: prende due oggetti (7379) e fa comparire il secondo (4AF6, A = 45); poi attese su (IX+0D) 24 e 30 (suono 2C, fase 4) | verificata | 5 | stanza42 | stanza42 f2074; stanza42_attesan1 f2110; stanza42_attesan3 f2114; stanza42_chiude f2123; stanza42_n2 f2076 |
| 0x6DCE | sub_6dce | nemici_tipi_08_14.c | 135 | tipo 14/15, fase 4: posa sul blocco davanti (4938); attese 15 e 29, libera gli oggetti (6EC1, 6EB7), fase 3 | verificata | 6 | stanza42 | stanza42 f2124; stanza42_attesan1 f2144; stanza42_attesan2 f2146; stanza42_dir1 f2888; stanza42_libera f2163; stanza42_n2 f2126 |
| 0x6E55 | sub_6e55 | nemici_tipi_08_14.c | 59 | tipo 14, fase 5: Dana vicina (6E90, HL = 0816) → direzione dal segno di (IX+5), fase 2 (6E83, 6E8B interni); altrimenti cella davanti (5C32) | verificata | 5 | stanza10, stanza14 | stanza10 f2023; stanza10_fermon1 f2526; stanza10_fermon2 f2886; stanza10_forza f2023; stanza14 f2022 — stanza10_forza: stato forzato |
| 0x6E90 | sub_6e90 | nemici_tipi_08_14.c | 17 | riporto se |2·(IX+5)| < L e (IX+4) < H | verificata | 2 | stanza10, stanza14 | stanza10 f2023; stanza14 f2022 |
| 0x6EA1 | sub_6ea1 | nemici_tipi_08_14.c | 22 | tipo 14, fase 6: col bit 3 di (IX+1) libera (IX+0E) e (IX+0F) (6EB7) | verificata | 3 | stanza10, stanza23 | stanza10 f2002; stanza10_forza f2002; stanza23 f2003 — stanza10_forza: stato forzato |
| 0x6EB7 | sub_6eb7 | nemici_tipo09.c | 10 | azzera i due byte della tabella 488D all'indice A, cade in 6EC1; anticipata da N3 | verificata | 2 | stanza10, stanza34 | stanza10 f2615; stanza34 f2303 |
| 0x6EC1 | sub_6ec1 | nemici_tipo09.c | 9 | azzera il byte della tabella 48AF all'indice A; anticipata da N3 | verificata | 2 | stanza10, stanza34 | stanza10 f2595; stanza34 f2281 |
| 0x6ECA | sub_6eca | nemici_tipi_08_14.c | 165 | tipo 8: velocità 6F3D in c1a0, nuova posizione, celle attorno (70FE), direzioni 70EE, `rst 20` a 6F1C (16 voci), coda 7056 (statica) | verificata | 8 | stanza08, stanza11, stanza22, stanza27 | stanza08 f1953; stanza11 f1953; stanza11_gn46 f2005; stanza22_gn2 f1955; stanza22_gn34 f1995; stanza27_gn19 f1974; stanza27_gn31 f1988; stanza27_gn58 f2020 |
| 0x6F49 | sub_6f49 | nemici_tipi_08_14.c | 2 | 6F1C[0], [15]: `or a; ret` | verificata | 2 | stanza11, stanza32 | stanza11 f2343; stanza32 f1953 |
| 0x6F4B | sub_6f4b | nemici_tipi_08_14.c | 19 | 6F1C[1]: 6F91 o gira di 2 (70A8) | verificata | 2 | stanza08, stanza11 | stanza08 f1965; stanza11 f1953 |
| 0x6F5E | sub_6f5e | nemici_tipi_08_14.c | 16 | 6F1C[2]: 6F91 o gira di 1 | verificata | 2 | stanza08, stanza11 | stanza08 f2346; stanza11 f1979 |
| 0x6F6E | sub_6f6e | nemici_tipi_08_14.c | 19 | 6F1C[4]: 6F91 o gira di 2 | verificata | 2 | stanza08, stanza11 | stanza08 f2356; stanza11 f2022 |
| 0x6F81 | sub_6f81 | nemici_tipi_08_14.c | 16 | 6F1C[8]: 6F91 o gira di −1 | verificata | 2 | stanza08, stanza11 | stanza08 f1961; stanza11 f1956 |
| 0x6F91 | sub_6f91 | nemici_tipi_08_14.c | 34 | 6FB3/6FD1 secondo il bit 1 di C; col bit 7 allinea l'altro asse (6FDF/6FC3, statiche) e C = 0/2 | verificata | 4 | stanza08, stanza11 | stanza08 f1996; stanza08_forza6fc3 f1996; stanza08_forza6fdf f2189; stanza11 f1954 — stanza08_forza6fc3, stanza08_forza6fdf: stato forzato |
| 0x6FB3 | sub_6fb3 | nemici_tipi_08_14.c | 16 | bit 7 di C se D & 0F ≠ B >> 4 | verificata | 2 | stanza08, stanza11 | stanza08 f2189; stanza11 f1954 |
| 0x6FD1 | sub_6fd1 | nemici_tipi_08_14.c | 14 | bit 7 di C se (E + 8) & 0F ≠ B & 0F | verificata | 2 | stanza08, stanza11 | stanza08 f1996; stanza11 f1972 |
| 0x6FED | sub_6fed | nemici_tipi_08_14.c | 10 | 6F1C[3], [12]: asse x, 70B0 → C = 2 | verificata | 3 | stanza08, stanza11 | stanza08 f1953; stanza08_forza f1953; stanza11 f1953 — stanza08_forza: stato forzato |
| 0x6FF7 | sub_6ff7 | nemici_tipi_08_14.c | 10 | 6F1C[6], [9]: asse y, 70CE → C = 0 | verificata | 4 | stanza08, stanza11 | stanza08 f2025; stanza11 f1953; stanza11_vert f2435; stanza11_vertok f2437 |
| 0x7001 | sub_7001 | nemici_tipi_08_14.c | 10 | 6F1C[5]: B = 44/CC sul bit 3 di D, poi 7015/7036 | verificata | 2 | stanza11, stanza22 | stanza11 f1971; stanza22 f3093 |
| 0x700B | sub_700b | nemici_tipi_08_14.c | 10 | 6F1C[10]: B = 4C/C4 sul bit 3 di D, poi 7025/7045 | verificata | 2 | stanza11, stanza22 | stanza11 f2214; stanza22 f1995 |
| 0x7015 | sub_7015 | nemici_tipi_08_14.c | 16 | 6F1C[7]: col bit 0 di C spento gira di 3 | verificata | 2 | stanza08, stanza11 | stanza08 f2023; stanza11 f1971 |
| 0x7025 | sub_7025 | nemici_tipi_08_14.c | 17 | 6F1C[11]: con (C + 1) & 2 nullo gira di 2 | verificata | 2 | stanza08, stanza11 | stanza08 f2517; stanza11 f2044 |
| 0x7036 | sub_7036 | nemici_tipi_08_14.c | 15 | 6F1C[13]: col bit 0 di C acceso gira di 1 | verificata | 2 | stanza08, stanza11 | stanza08 f2520; stanza11 f1955 |
| 0x7045 | sub_7045 | nemici_tipi_08_14.c | 17 | 6F1C[14]: con (C + 1) & 2 acceso gira di 2 | verificata | 2 | stanza08, stanza11 | stanza08 f2018; stanza11 f1956 |
| 0x70A8 | sub_70a8 | nemici_tipi_08_14.c | 8 | 70B0 e, con riporto, 70CE | verificata | 2 | stanza08, stanza11 | stanza08 f1961; stanza11 f1953 |
| 0x70B0 | sub_70b0 | nemici_tipi_08_14.c | 30 | asse y: allinea D a B >> 4 (riporto) o bit 7 di C | verificata | 2 | stanza08, stanza11 | stanza08 f1961; stanza11 f1953 |
| 0x70CE | sub_70ce | nemici_tipi_08_14.c | 32 | asse x: allinea E a (B + 8) & 0F (riporto) o bit 7 di C | verificata | 2 | stanza08, stanza11 | stanza08 f1961; stanza11 f1953 |
| 0x70FE | sub_70fe | nemici_tipi_08_14.c | 77 | le quattro celle attorno a (D, E) (36F0): A = B & 0F, bit 7 delle celle piene | verificata | 2 | stanza08, stanza11 | stanza08 f1953; stanza11 f1953 |
| 0x80A5 | sub_80a5 | nemici_comparsa.c | 14 | azzera (IX+0), (IX+1), (IY+0), (IY+1); anticipata da N4 | verificata | 2 | stanza11, stanza17 | stanza11 f2128; stanza17 f2143 |
