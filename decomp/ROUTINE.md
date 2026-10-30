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

## Motore dei nemici (lotto N3)

Tipo 9 (57CD) del gestore 4F1B (brief 29): `harness/nemici_tipo09.c`
(57CD, 57E4, 57FB, 580A, 582B, 58A3, 594F e 5894 statica),
`harness/nemici_tipo09b.c` (5995, 5B12, 5C19, 5C1E, 5C23, 5C29; 5B09 e 5C2D
statiche) e 5 dipendenze anticipate nei moduli del censimento: 52FD, 5345,
54C5 da N6 (`nemici_tipo06b.c`) e 51D7, 24DF da N10 (`nemici_tipo06a.c`).
Catture con lo stato `stanzaNN` di `cattura.sh`; 57E4 e 5C23 hanno anche
`stanza33_n2` (seconda occorrenza), 5C23 anche `stanza50`.

**Il registro R.** 54C5 e 59E5 leggono R con due `ld a,r` ravvicinati
(R1 e R1 + 3 sui 7 bit bassi) e ne usano la somma. Le catture non registrano
R e il «prima» non lo contiene. Le quattro voci che possono arrivare a un
`ld a,r` (57CD, 5995, 51D7, 54C5) stanno nel banco dietro un wrapper
(`banco_r_xxxx` in `nemici_r.c`) che riconosce la cattura dai file, prova i
128 valori di R1 (le 128 coppie dove le letture sono due) e tiene quello per
cui RAM e registri coincidono con MAME. Il resto della routine resta
verificato in pieno; di R si verifica solo che esista un valore compatibile
(lacuna da chiudere se un giorno le catture registreranno R). Il C del
gioco (`nemici_r_somma`) con i valori a zero vale R1 = 0.

**Catture forzate** (`SNAP_FORZA`, suffisso `forzaNNN`): servono a coprire
rami che le stanze 33, 45 e 50 non eseguono. Le forzature (campi di IX e IY e
celle della griglia c0bd-c1bc, queste come `ix+offset` con offset a 16 bit)
sono state trovate con un esploratore in C fuori dal repository (prove a caso
da una cattura di base, tenute solo quelle che aggiungono righe di copertura)
e sono elencate nelle righe `FORZA` dei `.log`. Basi: `stanza33` per 57CD
(27 catture) e 582B (2); 5995 da `stanza33` (20) e `stanza45` (5); le altre
voci non hanno forzate (le copre 57CD). Non contano per lo stato `verificata`.

Una forzatura ha scoperto un'imprecisione di N1: in `sub_4592` (45DB, uscita
col riporto quando nessuna delle due celle sotto i bordi ha il bit 7) A
restava com'era invece del secondo `rla` ((cella) << 1). Corretto in
`nemici_comuni.c`; le catture di 4592 di N1 continuano a passare.

Copertura gcov del banco: `nemici_tipo09.c` 98,98 %, `nemici_tipo09b.c`
97,42 %, `nemici_tipo06a.c` 100 %, `nemici_tipo06b.c` 99,14 %. Righe mai
eseguite: in `nemici_tipo09.c` il `ret nc` di 7379 in 582B (7379 non esaurisce
mai gli slot) e il `default` del `switch` di 57CD (indice fuori tabella); in
`nemici_tipo09b.c` il ramo di 59E5 con somma di R = 2 (`goto 5A2D`, dipende da
R, non forzabile), l'uscita `5A16 → 5B4B`, il `ret z` di 5AC2 (cella uguale a c2bb) e i tre
rami di 7379 senza slot libero (5AC7, 5AF2, 5B37; 7379 non li esaurisce
mai), e dello `switch` di 5BFC le voci 3B62 e 5B09 e il `default`; in `nemici_tipo06b.c` la guardia contro
C = 0 di 54C5 (l'originale non esce). 5B09 è tradotta senza cattura.

| ingresso | funzione | file | byte | descrizione | stato | catture OK | stati | note |
|---|---|---|---:|---|---|---|---|---|
| 0x24DF | sub_24df | nemici_tipo06a.c | 47 | tre indici a nibble (A, (HL), (HL+1)) → E, D e C = E + B; dipendenza anticipata da N10 | verificata | 2 | stanza33, stanza45 | stanza33 f1953; stanza45 f1953 |
| 0x51D7 | sub_51d7 | nemici_tipo06a.c | 35 | posizioni dei bit accesi di BC in c06a, C ne conta il numero, 54C5 ne sceglie uno a caso, A = 0d − tabella; R cercato dal banco; anticipata da N10 | verificata | 2 | stanza33, stanza45 | stanza33 f1953; stanza45 f1953 |
| 0x52FD | sub_52fd | nemici_tipo06b.c | 31 | contatore (IX+8) contro il limite (IX+9/0A): al limite lo azzera e inverte il bit 5 di (IX+0); anticipata da N6 | verificata | 2 | stanza33, stanza45 | stanza33 f1953; stanza45 f1953 |
| 0x5345 | sub_5345 | nemici_tipo06b.c | 384 | maschere B e C delle celle attorno (c0bd + BC) in tre rami (537B, 53F5, 545E); anticipata da N6 | verificata | 2 | stanza33, stanza45 | stanza33 f1953; stanza45 f1953 |
| 0x54C5 | sub_54c5 | nemici_tipo06b.c | 19 | numero a caso < C con due `ld a,r` (R cercato dal banco); anticipata da N6 | verificata | 2 | stanza33, stanza45 | stanza33 f1953; stanza45 f1953 |
| 0x57CD | sub_57cd | nemici_tipo09.c | 7 | tipo 09: 52FD, poi `rst 38` a 57D3 su (IY+1D) (8 voci); R cercato dal banco | verificata | 29 | stanza33, stanza45 | stanza33 f1953; stanza45 f1953 — 27 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x57E4 | sub_57e4 | nemici_tipo09.c | 23 | tipo 09, fase 4: a 0x20 fotogrammi inverte la direzione, fase 5 | verificata | 2 | stanza33 | stanza33_n2 f1955; stanza33 f1953 |
| 0x57FB | sub_57fb | nemici_tipo09.c | 15 | tipo 09, fase 0: a 4 fotogrammi spegne il bit 3 di (IY+1), fase 3 | verificata | 2 | stanza33, stanza45 | stanza33 f2035; stanza45 f1993 |
| 0x580A | sub_580a | nemici_tipo09.c | 33 | tipo 09, fase 2: accende il bit 1 della direzione; a 8 fotogrammi 5894 o fase 6 | verificata | 2 | stanza33, stanza45 | stanza33 f2046; stanza45 f2013 |
| 0x582B | sub_582b | nemici_tipo09.c | 105 | tipo 09, fase 1: presa del blocco (7379, 4592) o, col bit 2 di (IY+1), a 8 fotogrammi fase 5 e 5894 | verificata | 4 | stanza33, stanza45 | stanza33 f1977; stanza45 f1982 — 2 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x5894 | sub_5894 | nemici_tipo09.c | 15 | statica, ingresso interno di 580A e 582B: lascia il blocco, fase 6, 6EB7 | tradotta | - | - | senza voce propria: coperta dalle catture dei chiamanti (580A, 582B) |
| 0x58A3 | sub_58a3 | nemici_tipo09.c | 44 | tipo 09, fase 7: prende (58CF) o posa (5917) il blocco, poi lascia il portato (6EB7) | verificata | 2 | stanza33, stanza45 | stanza33 f2001; stanza45 f1964 |
| 0x594F | sub_594f | nemici_tipo09.c | 70 | tipo 09, fase 6: con (IY+0C) = ff flag di (IX+1) e direzione, fase 7 o c2ba | verificata | 2 | stanza33, stanza45 | stanza33 f1953; stanza45 f1953 |
| 0x5995 | sub_5995 | nemici_tipo09b.c | 597 | tipo 09, fase 5: sceglie la mossa dalle maschere di 5345, dal caso (59E5) e dalle celle davanti; 51D7 e `rst 38` a 5BFC (14 voci); R cercato dal banco | verificata | 27 | stanza33, stanza45 | stanza33 f1953; stanza45 f1953 — 25 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x5B09 | sub_5b09 | nemici_tipo09b.c | 9 | statica: fase 0, bit 1 della direzione; mai eseguita nelle sessioni storiche | tradotta | - | - | senza cattura: nessuna sessione l'ha eseguita e snap.lua non forza le celle che 5C32 scrive in c07e-c081 |
| 0x5B12 | sub_5b12 | nemici_tipo09b.c | 10 | A = ((IY+15) − 6) & 0f, riporto se < 5 | verificata | 2 | stanza33, stanza45 | stanza33 f1986; stanza45 f1961 |
| 0x5C19 | sub_5c19 | nemici_tipo09b.c | 5 | 5BFC[0], [2]: direzione 0 | verificata | 2 | stanza33, stanza50 | stanza33 f1953; stanza50 f2156 |
| 0x5C1E | sub_5c1e | nemici_tipo09b.c | 5 | 5BFC[7], [9]: direzione 1 | verificata | 2 | stanza33, stanza45 | stanza33 f2023; stanza45 f1953 |
| 0x5C23 | sub_5c23 | nemici_tipo09b.c | 6 | 5BFC[1], [3], [4], [5]: direzione 0, cade in 5C2D | verificata | 3 | stanza33, stanza50 | stanza33_n2 f2035; stanza33 f2034; stanza50 f2038 |
| 0x5C29 | sub_5c29 | nemici_tipo09b.c | 4 | 5BFC[8], [10], [11], [12]: direzione 1, cade in 5C2D | verificata | 2 | stanza33, stanza45 | stanza33 f2346; stanza45 f2041 |
| 0x5C2D | sub_5c2d | nemici_tipo09b.c | 5 | statica, coda di 5C23 e 5C29: fase 0 | tradotta | - | - | senza voce propria: coperta dalle catture dei chiamanti (5C23, 5C29) |

## Motore dei nemici (lotto N4)

Comparsa (tipi 0, 4 e 5 del gestore 4F1B, brief 30): `harness/nemici_comparsa.c`
(4F40, 4FC5, 738F, 13xx/14xx, 1549, con 734D e 80A5 dei lotti N1 e N2; 14FC
statica) e `harness/nemici_comparsab.c` (7288 e le fasi del tipo 5, 73A3 e le
fasi del tipo 4, 756C, 758E, 759F, 2548; 73C7 e 7456 statiche), divisi per
stare sotto le 700 righe. Nessun `ld a,r`: niente wrapper `banco_r_*`.

**Stato «oggetti».** Le routine 4F40 e 13xx/14xx si eseguono solo nella sessione
«oggetti» (oggetti 08-1B in coda c05b e posta 22): l'attività 22 / 12D6 le
chiama dalla tabella 130A, per cui le catture hanno «dopo» a PC=12E8.
`cattura_oggetti.sh` replica quello stato (oggetti e posta ripetuti due
volte, perché 13AB, 13B9 e 14D4 si eseguano almeno due volte) passando per
`cattura.sh`; i file hanno tag `gioco_oggetti[_n2|_forzaNN]`. Le altre voci
usano `stanzaNN` (tipo 5: stanze 02, 11, 17; tipo 4: 03, 12) e `finale`
(7412, che nelle stanze non scatta in cattura; seconda cattura `finale_n2`).

**Catture forzate** (`SNAP_FORZA`, suffisso `forza*`, righe FORZA nei `.log`):
non contano per lo stato `verificata`. 4F40 (oggetti): `iy+1d`/`iy+1e` per
ognuna delle sei voci di 4FAC (14FC, 13AB, 139C, 13B9, 1413, 14D4), il ramo
4FB9 (indice 6), il ramo con `iy+1d:00` (80A5 e traduzione della direzione), le
uscite di 738F e i due rami dopo la distanza; 73A3 (stanza03): `iy+1d:07` con
`ix+1:00` (primo ramo di 73C7), con `iy+4:0d`, con `ix+1:10` e `ix+8` a 20 e 05
(ramo 7409 e `ret c`), il ramo con c238 portato a 9 (cella assoluta, con
scostamento a 16 bit da IX), le fasi 0-3 e 6 di (IY+1D) e gli urti di 74AB; 7288
(stanza11): fasi 0-3 con vari `ix+1`, `ix+8`, `iy+1a` e la cella davanti (c170,
con scostamento a 16 bit) per 72B6 e 72DA; 7412 (finale): `ix+8` a 20 e 00; 13FC
con `a:36` (apre 14FC); 13B9 con c25a a 0200 (134B oltre il limite). 14FC, 4FC5
e 73C7 non hanno voce nel banco (il breakpoint d'ingresso non scatta mai:
nessuna sessione le ha eseguite) e sono verificate dalle catture forzate dei
chiamanti e dalla copertura gcov.

Copertura gcov del banco: `nemici_comparsa.c` 98,46 %, `nemici_comparsab.c`
99,32 %; 14FC, 4FC5 e 73C7 al 100 %. Righe mai eseguite: in `nemici_comparsa.c`
il ramo di 734D senza bit accesi nei bit 0-3 (N1, `ld (0000h),a`); nei due
file il `default` degli `switch` sui `rst 38` (indice sempre in tabella).

| ingresso | funzione | file | byte | descrizione | stato | catture OK | stati | note |
|---|---|---|---:|---|---|---|---|---|
| 0x134B | sub_134b | nemici_comparsa.c | 16 | (c25a) += DE; con il bit 9 acceso vale 0200 | verificata | 2 | oggetti | oggetti_n2 f2702; oggetti f2402 |
| 0x139C | sub_139c | nemici_comparsa.c | 15 | 4FAC[2]: (c1ad) = 0b, posta l'attività 19, 13FC con A = 2a | verificata | 2 | oggetti | oggetti_n2 f2802; oggetti f2502 |
| 0x13AB | sub_13ab | nemici_comparsa.c | 14 | 4FAC[1]: (c1ad) = 0c, posta l'attività 19, 13FC con A = 2b | verificata | 2 | oggetti | oggetti_n2 f3502; oggetti f2802 |
| 0x13B9 | sub_13b9 | nemici_comparsa.c | 10 | 4FAC[3]: 134B con DE = 10, 13FC con A = 2a | verificata | 3 | oggetti | oggetti_n2 f3102; oggetti f2402 — 1 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x13FC | sub_13fc | nemici_comparsa.c | 23 | tabella ROM 150C (3 byte per voce) sommata con 1549, posta l'attività 05; con A = 0d va a 14FC | verificata | 3 | oggetti | oggetti_n2 f2403; oggetti f2402 — 1 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x1413 | sub_1413 | nemici_comparsa.c | 17 | 4FAC[4]: c259 sale fino a 9, tabellone (349F), 13FC con A = 2a | verificata | 2 | oggetti | oggetti_n2 f2802; oggetti f2502 |
| 0x14D4 | sub_14d4 | nemici_comparsa.c | 40 | 4FAC[5]: cerca la maschera libera in c25c, la somma e ridisegna (349F), 13FC con A = 2a | verificata | 2 | oggetti | oggetti_n2 f3602; oggetti f2902 |
| 0x14FC | sub_14fc | nemici_comparsa.c | 16 | statica, 4FAC[0] e salto di 13FC con A = 0d: suono 12, c237 += 1, barra 31E9, 13FC con A = 2c | tradotta | - | - | senza cattura diretta: coperta dalle catture forzate di 4F40 e 13FC |
| 0x1549 | sub_1549 | nemici_comparsa.c | 14 | (c1aa) += DE, (c1ac) += L + riporto | verificata | 2 | oggetti | oggetti_n2 f2403; oggetti f2402 |
| 0x2548 | sub_2548 | nemici_comparsab.c | 9 | HL += 8 * A, E = A * 8, D = 0 | verificata | 2 | stanza03, stanza12 | stanza03 f1953; stanza12 f1953 |
| 0x4F40 | sub_4f40 | nemici_comparsa.c | 121 | tipo 0 di 4F1B: comparsa dell'entità; 738F, 4FC5, suono 15, `rst 38` su 4FAC (6 voci), poi 80A5 | verificata | 20 | oggetti | oggetti_n2 f2458; oggetti f2456 — 18 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x4FC5 | sub_4fc5 | nemici_comparsa.c | 15 | c05a = 80, (c058) = posizione (IY+0E, IY+15); condivisa da 4F40 e 73C7 | tradotta | - | - | senza cattura diretta: coperta dalle catture forzate di 4F40 e 73A3 |
| 0x7288 | sub_7288 | nemici_comparsab.c | 4 | tipo 5 di 4F1B: `rst 38` a 728B su (IY+1D) (4 voci) | verificata | 39 | stanza02, stanza11 | stanza02 f1988; stanza11 f2052 — 37 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x7294 | sub_7294 | nemici_comparsab.c | 34 | tipo 5, fase 0: a 10 fotogrammi fase 3 | verificata | 2 | stanza02, stanza11 | stanza02 f1988; stanza11 f2052 |
| 0x72B6 | sub_72b6 | nemici_comparsab.c | 24 | tipo 5, fase 2: cella vicina (734D) con il bit 7, fase 1 | verificata | 2 | stanza02, stanza11 | stanza02 f2006; stanza11 f2070 |
| 0x72CE | sub_72ce | nemici_comparsab.c | 12 | tipo 5, fase 3: con (IY+0B) o (IY+12) non nulli fase 2 | verificata | 2 | stanza02, stanza11 | stanza02 f2005; stanza11 f2068 |
| 0x72DA | sub_72da | nemici_comparsab.c | 115 | tipo 5, fase 1: bit 1 e 0 di (IX+1), cella vicina (734D), 4938, 4AF6 o 80A5 | verificata | 2 | stanza11, stanza17 | stanza11 f2111; stanza17 f2124 |
| 0x738F | sub_738f | nemici_comparsa.c | 20 | distanza |(IX+4)| < B e |(IX+5)| < C, riporto in F | verificata | 2 | stanza03, stanza12 | stanza03 f1953; stanza12 f1953 |
| 0x73A3 | sub_73a3 | nemici_comparsab.c | 20 | tipo 4 di 4F1B: 738F (08, 08) poi `rst 38` a 73B6 su (IY+1D) (8 voci) | verificata | 22 | stanza03, stanza12 | stanza03 f1953; stanza12 f1953 — 20 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x73C7 | sub_73c7 | nemici_comparsab.c | 75 | statica, 73A3[7]: prima volta suono 32, c250 += 1, c238 sale (a 10 barra), 4FC5, 13FC; poi a 10 fotogrammi 80A5 | tradotta | - | - | senza cattura diretta: coperta dalle catture forzate di 73A3 |
| 0x7412 | sub_7412 | nemici_comparsab.c | 11 | tipo 4, fase 0: a 20 fotogrammi fase 2 | verificata | 4 | finale | finale_n2 f2809; finale f2806 — 2 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x741D | sub_741d | nemici_comparsab.c | 57 | tipo 4, fase 1: velocità da 7468 con i segni di (IX+1) e (IX+11); coda 7456 | verificata | 2 | stanza03, stanza12 | stanza03 f1957; stanza12 f1957 |
| 0x7456 | sub_7456 | nemici_comparsab.c | 18 | statica, coda di 741D e 7470: fase 6 e velocità azzerate | tradotta | - | - | senza cattura diretta: coperta dalle catture forzate di 741D e 7470 |
| 0x7470 | sub_7470 | nemici_comparsab.c | 59 | tipo 4, fase 2: come 741D con gli assi scambiati; coda 7456 | verificata | 2 | stanza03, stanza12 | stanza03 f2052; stanza12 f2053 |
| 0x74AB | sub_74ab | nemici_comparsab.c | 161 | tipo 4, fase 6: urti contro le celle (756C, 758E, 759F) e tabelle 754C e 755C | verificata | 2 | stanza03, stanza12 | stanza03 f1953; stanza12 f1953 |
| 0x756C | sub_756c | nemici_comparsab.c | 34 | distanza con segno fra due parole, riporto in F | verificata | 2 | stanza03, stanza12 | stanza03 f1953; stanza12 f1953 |
| 0x758E | sub_758e | nemici_comparsab.c | 17 | confronto di DE con 0050 / FFB0, riporto in F | verificata | 2 | stanza03, stanza12 | stanza03 f1953; stanza12 f1953 |
| 0x759F | sub_759f | nemici_comparsab.c | 11 | HL += 8 * (B - 0A), A = (IX+6) ^ C | verificata | 2 | stanza03, stanza12 | stanza03 f1953; stanza12 f1953 |

## Motore dei nemici (lotto N5)

Tipi 10-13 di 4F1B (brief 31): `harness/nemici_tipi_10_13.c` (55C4 e 6C1D del
tipo 12, 719E del tipo 10, con le routine dei lotti N1 e N2) e
`harness/nemici_tipi_10_13b.c` (tipo 13: 54D8, 54EA, 551A, 5529, 5584, 6BCD;
tipo 11: 56E9, 5713, 5726, 573C, 5749), divisi per stare sotto le 700 righe.
Nessun `ld a,r`: niente wrapper `banco_r_*`. Tutti e quattro i tipi (10, 11,
12, 13) hanno ora ogni fase tradotta; 55C4, 6C1D e 719E non fermano più il
replay perché gli stub sono tolti. Le catture hanno «dopo» a PC=4EFB (ritorno a
4F0F); quelle di 6BCD a PC=6B78. 551A si cattura solo dallo stato `finale` e
5584 solo da `stanza30` (nelle altre stanze non scattano): la seconda cattura
non forzata è `finale_n2` / `stanza30_n2` (occorrenza 2).

**`sub_6c2c` ora imposta il riporto** (bit 0 di `z->f`, come 5507 e 7379), che 551A
e 6C1D leggono: 1 se e solo se (IX+1) & 21 == 21 all'ingresso, 0 negli altri
rami. Le sue catture (2) restano OK perché il banco non confronta F.

**Catture forzate** (`SNAP_FORZA`, suffisso `forzaNN`, righe FORZA nei `.log`;
non contano per lo stato `verificata`): 54D8 `iy+1d` a 00, 01, 02, 03, 05 (le
cinque fasi); 54EA (stanza02) il ramo senza riporto (561B) e la fase 5 con bit 1
acceso e spento; 551A (finale) riporto con 80A5, riporto con suono, 6C2C senza
riporto nei due passaggi; 5529 (stanza02) le uscite 5573, 5549, le due attese 64
e 32 con suono 27, i rami con maschera 20 (`iy+1a:30`) e 5567; 5584 (stanza30)
7379 + 6BCD, i `ret c` e 55A3; 6BCD le quattro combinazioni dei bit 0-1 di C;
6C1D (stanza07) riporto con 80A5, `ret c` con suono, `ret nc`, primo passaggio;
56E9 (stanza01) le sette voci di 5704, il ramo (C1A4) e 57B3; 5713 e 5726 i due
rami; 5749 `iy+12:01`, i rami con il bit 3 di (IX+1) (`ret c` e 5917 + 6EB7),
il bit 6 acceso e il ramo 5763; 719E (stanza02) tutti i rami fino a 4938.
Non raggiunti: `ret nc` di 7379 (5590, 5772, 720D), i rami di 5749 oltre 5763
(cella occupata) e il `ret` di 5572 e il 5545 di 5529 (le forzature del brief
non lo aprono: `res 1` con bit 1 acceso e riporto di 5507). Copertura gcov:
`nemici_tipi_10_13b.c` 91,53 %; `sub_55c4` e `sub_6c1d` 100 %, `sub_719e`
98,46 % (manca il `ret` di 7379).

| ingresso | funzione | file | byte | descrizione | stato | catture OK | stati | note |
|---|---|---|---:|---|---|---|---|---|
| 0x54D8 | sub_54d8 | nemici_tipi_10_13b.c | 4 | tipo 13 di 4F1B: `rst 38` a 54DB su (IY+1D) (7 voci) | verificata | 7 | stanza02, stanza06 | stanza02 f1953; stanza06 f1953 — 5 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x54EA | sub_54ea | nemici_tipi_10_13b.c | 29 | tipo 13, fase 2: 5507 (soglia 18), senza riporto 561B; fase 5, bit 1 di (IY+1E) da (IY+1A) & 30 | verificata | 5 | stanza02, stanza06 | stanza02 f1958; stanza06 f2063 — 3 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x551A | sub_551a | nemici_tipi_10_13b.c | 15 | tipo 13, fase 1: 6C2C con A = 2, con riporto e (IX+0D) >= 18 chiude con 80A5 | verificata | 6 | finale | finale_n2 f3748; finale f2813 — 4 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x5529 | sub_5529 | nemici_tipi_10_13b.c | 91 | tipo 13, fase 5: tre chiamate a 5507, attesa 32/64, suono 27, fase 2 con bit 4 di (IX+1) | verificata | 10 | stanza02, stanza06 | stanza02 f1953; stanza06 f1953 — 8 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x5584 | sub_5584 | nemici_tipi_10_13b.c | 46 | tipo 13, fase 0: slot libero (7379) e oggetto portato con 6BCD, poi attesa 48 e fase 2 | verificata | 6 | stanza30 | stanza30_n2 f2360; stanza30 f2357 — 4 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x55C4 | sub_55c4 | nemici_tipi_10_13.c | 5 | tipo 12, fase 3 di 55B5: fase 2 | verificata | 2 | stanza07, stanza10 | stanza07 f3176; stanza10 f3491 |
| 0x56E9 | sub_56e9 | nemici_tipi_10_13b.c | 28 | tipo 11 di 4F1B: BC = (C1A2)/(C1A4), 57AA e 57B3, `rst 38` a 5704 su (IY+1D) (7 voci) | verificata | 11 | stanza01, stanza06 | stanza01 f2002; stanza06 f2002 — 9 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x5713 | sub_5713 | nemici_tipi_10_13b.c | 10 | tipo 11, fase 6: con il bit 5 di (IY+0) spento fase 5 | verificata | 4 | stanza01, stanza06 | stanza01 f2002; stanza06 f2002 — 2 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x5726 | sub_5726 | nemici_tipi_10_13b.c | 22 | tipo 11, fase 1: azzera (IX+0B..0C) se il bit 3 di (IY+4) è spento, bit 6 di (IX+1), fase 2 | verificata | 4 | stanza01, stanza06 | stanza01 f2022; stanza06 f2095 — 2 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x573C | sub_573c | nemici_tipi_10_13b.c | 13 | tipo 11, fasi 2 e 3: (IY+12) = 5, fase 5, bit 1 di (IY+1E) | verificata | 2 | stanza01, stanza06 | stanza01 f2022; stanza06 f2096 |
| 0x5749 | sub_5749 | nemici_tipi_10_13b.c | 97 | tipo 11, fase 5: cella davanti (5928), slot (7379), 58CF, 5917 e 6EB7 | verificata | 8 | stanza01, stanza06 | stanza01 f2023; stanza06 f2097 — 6 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x6BCD | sub_6bcd | nemici_tipi_10_13b.c | 62 | oggetto portato in 48AF spostato di 0b sull'asse scelto da C, agganciato con 4AF6 | verificata | 6 | stanza02, stanza11 | stanza02 f1987; stanza11 f2050 — 4 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x6C1D | sub_6c1d | nemici_tipi_10_13.c | 15 | tipo 12, fase 1 di 55B5: 6C2C con A = 3, con riporto e (IX+0D) >= 0d chiude con 80A5 | verificata | 6 | stanza07, stanza19 | stanza07 f3175; stanza19 f2469 — 4 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |
| 0x719E | sub_719e | nemici_tipi_10_13.c | 151 | tipo 10, fase 1 di 714E: stacco e ripresa dell'oggetto portato (6EB7, 4AF6, 734D, 7379, 4938) | verificata | 10 | stanza02, stanza05 | stanza02 f2000; stanza05 f2084 — 8 con stato forzato (SNAP_FORZA, righe FORZA nei .log) |

## Motore dei nemici (lotto N6)

Tipo 6 di 4F1B, seconda metà (brief 32): `harness/nemici_tipo06b.c` (5274,
5289, 5292, 52AA, 52C2, 52DB, 52EC, 531C, in coda alle 52FD, 5345 e 54C5 di N3)
e `harness/nemici_tipo06a.c`, dove si sono anticipate da N10 cinque routine del
tipo 6 che le prime richiedono: 5068 (coda condivisa), 51FA (`rst 38` a 5207 su
14 voci), 5224, 523C e 5254. N10 le trova fatte. 51FA legge R tramite 51D7 e
54C5: la voce del banco è il wrapper `banco_r_51fa` (`nemici_r.c`). **531C**:
se una delle due celle è piena l'originale scarta l'indirizzo di ritorno e
termina in 5068, tornando al chiamante del chiamante; in C `sub_531c` esegue
`sub_5068` e ritorna con il riporto (bit 0 di `z->f`) a 1 = uscita anomala (il
chiamante deve ritornare subito), a 0 = uscita normale; HL in quel caso non è
riprodotto. Il ramo anomalo non è catturabile (`snap.lua` aspetta il ritorno
all'indirizzo in cima alla pila) e resta scoperto da gcov. Copertura gcov:
`nemici_tipo06a.c` 97,83 % di 92 (manca il ramo fuori tabella di 51FA, non
raggiungibile con E <= 13), `nemici_tipo06b.c` 97,74 % di 177 (manca il ramo
anomalo di 531C e il `break` di 54C5 con C = 0).

| ingresso | funzione | file | byte | descrizione | stato | catture OK | stati | note |
|---|---|---|---:|---|---|---|---|---|
| 0x5068 | sub_5068 | nemici_tipo06a.c | 42 | coda condivisa: (IY+0) bit 5-6 a 01, (IY+4) += 3, (IY+3) = 0d, (IY+1E) = 2, (IY+1A) = B, bit 6 di (IY+0) | verificata | 2 | finale | finale e finale_n2 (PC=4EFB) |
| 0x51FA | sub_51fa | nemici_tipo06a.c | 14 | 51D7, (IX+10) = A se il bit 0 di (IX+1) è spento, `rst 38` a 5207 (14 voci) | verificata | 30 | finale, stanza50 | wrapper `banco_r_51fa`; 4 non forzate (finale, finale_n2, stanza50, stanza50_n2) e 26 forzate |
| 0x5224 | sub_5224 | nemici_tipo06a.c | 24 | C = 1 se (IX+10) > 6 con bit 0 di (IX+1) acceso; (IY+1E) = C; bit 0 acceso | verificata | 3 | finale, stanza50 | finale, finale_n2, stanza50; le forzate sono quelle di 51FA |
| 0x523C | sub_523c | nemici_tipo06a.c | 24 | come 5224 ma bit 0 di (IX+1) spento | tradotta | - | - | senza cattura propria, coperta dalle forzate di 51FA |
| 0x5254 | sub_5254 | nemici_tipo06a.c | 32 | come 5224 con (IY+1D) = 0 e bit 2 di (IX+0) spento | tradotta | - | - | senza cattura propria, coperta dalle forzate di 51FA |
| 0x5274 | sub_5274 | nemici_tipo06b.c | 21 | (IY+1D) = (IY+1E) = 0, bit 7 di (IX+1) acceso | verificata | 2 | finale | finale e finale_n2 (dopo a 510D / 5114) |
| 0x5289 | sub_5289 | nemici_tipo06b.c | 9 | bit 1 di (IY+1E) acceso, bit 0 di (IX+1) spento | tradotta | - | - | senza cattura propria, coperta dalle forzate di 51FA |
| 0x5292 | sub_5292 | nemici_tipo06b.c | 24 | C = 1 (0 con bit 0 acceso e (IX+10) <= 6); (IY+1E) = C | tradotta | 4 | finale | una sola cattura non forzata; 3 forzate dirette (`ix+1:00`, `ix+1:01,ix+10:07`, `ix+1:01,ix+10:03`) |
| 0x52AA | sub_52aa | nemici_tipo06b.c | 24 | identica a 5292 | tradotta | - | - | senza cattura propria, coperta dalle forzate di 51FA |
| 0x52C2 | sub_52c2 | nemici_tipo06b.c | 25 | C = 1 (0 se bit 0 di (IX+1) acceso); (IY+1D) = 0, (IY+1E) = C | tradotta | - | - | senza cattura propria, coperta dalle forzate di 51FA |
| 0x52DB | sub_52db | nemici_tipo06b.c | 17 | (IY+1D) = 0, (IY+1E) = 1, bit 0 di (IX+1) e bit 2 di (IX+0) spenti | tradotta | - | - | senza cattura propria, coperta dalle forzate di 51FA |
| 0x52EC | sub_52ec | nemici_tipo06b.c | 17 | (IY+1D) = 0, bit 1 di (IY+1E) acceso, bit 0 di (IX+1) e bit 2 di (IX+0) spenti | tradotta | - | - | senza cattura propria, coperta dalle forzate di 51FA |
| 0x531C | sub_531c | nemici_tipo06b.c | 41 | due celle di c0bd (L e L+6); entrambe libere riporto 0, una piena 5068 e riporto 1 | verificata | 4 | finale, stanza50 | finale, finale_n2, stanza50, stanza50_n2; ramo anomalo non catturabile |

**Catture forzate** (`SNAP_FORZA`, suffisso `forzaNN`): 51FA (finale) `BC = 1 << j`
per j = 0..13 (14, `forza00`-`forza13`) e, per j = 0, 2, 4, 7, 9, 11, le varianti
`forzaJJb` (`ix+1:01,ix+10:07`, (IX+10) > 6) e `forzaJJc` (`ix+1:01,ix+10:03`)
(12); 5292 (finale) `forza01` (`ix+1:00`), `forza02` (`ix+1:01,ix+10:07`),
`forza03` (`ix+1:01,ix+10:03`). Tutte hanno aperto il ramo voluto.

## Motore dei nemici (lotto N7)

Corpo 0A, vie di 4632 (brief 33): `harness/nemici.c`, in coda, 4649, 46A3
(smistate da `salta_4644` con `rst 20h`), 470A, 4717 e 4767. 3B62, la sesta
del lotto, era già fatta in N1. **4767** è condivisa con 37B4 (chiamata a
37D2, ritorno a 37D5), non tradotta e fuori lotto: la cattura `finale` e
`stanza02` di 4767 vengono da lì. L'etichetta 480A cade dentro il ramo 47E5
ed è una funzione `static` (`ramo_480a`). Il riporto in uscita di 4767 non è
modellato (0): conta solo nel secondo giro di 4649 dopo il ramo 47B0, mai
visto. **Non esercitabili**: i due rami di 46A3 con `sub_7379` senza slot
libero (46AE-46B3 e 46DF-46E4), tradotti ma scoperti da gcov (6 righe).

| ingresso | funzione | file | byte | descrizione | stato | catture OK | stati | note |
|---|---|---|---:|---|---|---|---|---|
| 0x4649 | sub_4649 | nemici.c | 90 | se (c2a9)&3F < 36 esce; per i bit 7 e 6 di (c459) azzera 3 byte del record 488D, inizializza l'oggetto 48AF (4AF6, 4722, 4767), bit 6 di (IX+0); (c459) = 20 | verificata | 4 | porta, stanza06, stanza07 | `porta` è l'uscita `ret c`; 3 con `SNAP_COND` (tag `_c`), non forzate |
| 0x46A3 | sub_46a3 | nemici.c | 103 | per i bit 7 e 6 di (c459): slot libero (7379), nemico dalla lista (c2b1)/(c2b3) con 470A su FF, 472E, 4717; (c459)++ | verificata | 3 | porta, stanza06, stanza07 | `stanza07` solo il bit 6; i rami senza slot non esercitabili |
| 0x470A | sub_470a | nemici.c | 13 | lista terminata da FF: HL = HL - (byte successivo) a 16 bit, A = (HL) | verificata | 3 | porta, stanza13, stanza14 | lista in ROM (`leggi`) |
| 0x4717 | sub_4717 | nemici.c | 11 | 4AF6 con A=E5, E=03, BC=0 | verificata | 3 | porta, stanza06, stanza07 | |
| 0x4767 | sub_4767 | nemici.c | 264 | A = codice, C = indice: smista per intervalli di A sulle tabelle 488D/48AF; 47B0 copia 10 byte da 47C7 con 3569 | verificata | 22 | finale, porta, stanza02, stanza07 | 4 non forzate (A = 0D, 48, 34, 4E) e 18 forzate |

**Catture forzate** (`SNAP_FORZA="a:VV"`, suffisso `forzaVV`, stato `porta`) di
4767: 10, 11, 12, 13 (47B0, record 0-3 della tabella 47C7), 16, 19 (47E5),
1c (481D), 20, 24 (486B senza bit 3), 28, 2c (486B con bit 3), 40 (4813), 50,
54 (481D), 68 (4832), 80 (483F), a0 (4859), a4 (4798). Tutte hanno aperto il
ramo voluto.

## Motore dei nemici (lotto N8)

Tipo 07 di 4F0F, prima metà (brief 34): modulo nuovo `harness/nemici_tipo07a.c`
con 5CFB-5F82 e 5FAC (anticipata da N11); 4FEF (anticipata da N10) in coda a
`nemici_tipo06a.c`. 5DDF legge R con due `ld a,r`: nel banco la ricerca di R
passa dai wrapper `banco_r_5ddf` e `banco_r_5e1a` (5E1A chiama 5DDF), con
`nemici_r_somma_r1` che dà anche R1 (5DDF lascia C = R1). Restano stub
(`nemici_stub.c`): 610C (chiamata da 5CFB), 5FC8 (destinazione 2 di 5E1A) e
627F (coda di 5E9D). Il ramo di 5CFB 5D08-5D28 (610C, 4FEF, 5E1A) non è nel
banco finché N11 non traduce 610C: le 4 catture di 5CFB hanno il bit 2 di
(IX+0) acceso e percorrono 5D29. 5E60, 5F82 e 5FAC non hanno un ingresso
catturabile da MAME: sono `tradotte`, coperte dalle forzate.

| ingresso | funzione | file | byte | descrizione | stato | catture OK | stati | note |
|---|---|---|---:|---|---|---|---|---|
| 0x4FEF | sub_4fef | nemici_tipo06a.c | 10 | BC = parola n. (A-0x10) della tabella 4FF9 (anticipata da N10) | verificata | 2 | finale, stanza50 |  |
| 0x5CFB | sub_5cfb | nemici_tipo07a.c | 46 | tipo 07 di 4F0F: bit 2 di (IX+0) acceso prosegue con 5D29; altrimenti 610C, 4FEF e, se (IX+0B/0C) >= BC, 5E1A | verificata | 4 | finale, stanza50 | 4 con `SNAP_COND` (bit 2 acceso, tag `_c`); il ramo 5D08-5D28 non è nel banco finché N11 non traduce 610C |
| 0x5D29 | sub_5d29 | nemici_tipo07a.c | 172 | passo con (IX+0D) >= 3: tabella 84C8 a passi di 4 byte su (IY+0E/15), fine tabella (5D80) lancia con 4AF6, oppure chiude (5DC3) | verificata | 13 | finale, stanza50 | 4 non forzate e 9 forzate (`forza01-09`) |
| 0x5D76 | sub_5d76 | nemici_tipo07a.c | 10 | A = nibble basso con segno (bit 3 esteso a F_) | verificata | 4 | finale, stanza50 |  |
| 0x5DDF | sub_5ddf | nemici_tipo07a.c | 42 | due `ld a,r`: con i bit di D orienta (IX+5) e (IX+4) con 5E09 | verificata | 3 | finale | 2 non forzate e 1 forzata; legge R: `banco_r_5ddf` |
| 0x5E09 | sub_5e09 | nemici_tipo07a.c | 17 | B = differenza dimezzata, A = |B'|, riporto = (A < 0x18) | verificata | 4 | finale | 3 non forzate e 1 forzata (`forza01`, `a:00,c:00`: ramo `neg`); riporto in z->f |
| 0x5E1A | sub_5e1a | nemici_tipo07a.c | 43 | sceglie la mossa: 5DDF se bit 5 spento, azzera (IX+0B/0C/0D), `rst 38` a 5E40 su 4·((IY+1D)-0C)+(IY+1E) | verificata | 15 | finale, stanza50 | 6 non forzate e 9 forzate; legge R: `banco_r_5e1a`; la destinazione 2 (5FC8) è stub e non è esercitata |
| 0x5E4D | sub_5e4d | nemici_tipo07a.c | 15 | (IY+1D) = B, (IY+1E) = C col bit 6 di (IY+0) spento durante la scrittura | verificata | 4 | finale, stanza50 |  |
| 0x5E60 | sub_5e60 | nemici_tipo07a.c | 47 | bit 7 di (IX+4) spento: 5E4D con BC = 0003; acceso: 4AF6 su IY | tradotta | 0 | - | nessun ingresso catturabile; coperta dalle forzate di 5E1A (forza02, forza03) |
| 0x5E8F | sub_5e8f | nemici_tipo07a.c | 14 | E = |2B|, A = |2C| (con segno nel bit 7) | verificata | 4 | finale, stanza50 |  |
| 0x5E9D | sub_5e9d | nemici_tipo07a.c | 64 | mossa verso il giocatore con prova della cella (36F0); coda 627F (stub) | verificata | 3 | finale | 2 non forzate (solo le occorrenze 1 e 4 non passano da 627F) e 1 forzata |
| 0x5EDD | sub_5edd | nemici_tipo07a.c | 47 | destinazioni 4 e 5 di 5E1A: (B, C) da 5E8F, (IY+1E) e segno | verificata | 4 | finale | 2 non forzate e 2 forzate |
| 0x5F0C | sub_5f0c | nemici_tipo07a.c | 39 | quattro celle 2x2 della griglia: i loro bit 7 in C, A = C | verificata | 4 | finale, stanza50 |  |
| 0x5F33 | sub_5f33 | nemici_tipo07a.c | 37 | secondo il bit 7 di (IX+5): 5F0C e 5F4B (DE = 1008) oppure 5F82 (DE = 10F8); poi 5E4D | verificata | 6 | finale, stanza50 | 4 non forzate e 2 forzate |
| 0x5F4B | sub_5f4b | nemici_tipo07a.c | 42 | tabella 608C: (B, C) della voce, o coda 5FAC, o scelta con 5E8F | verificata | 20 | finale, stanza50 | 4 non forzate e 16 forzate (`forza00-0f`, `a:VV`) |
| 0x5F82 | sub_5f82 | nemici_tipo07a.c | 42 | come 5F4B con la tabella 60AC | tradotta | 0 | - | nessun ingresso catturabile; coperta dalle forzate di 5F33 e 5E1A (forza05) |
| 0x5FAC | sub_5fac | nemici_tipo07a.c | 28 | coda di 5F4B e 5F82: spegne il bit 2 di (IX+0), 4FEF, (IX+0B/0C) = FFF8, BC da (IY+1D/1E) (anticipata da N11) | tradotta | 0 | - | nessun ingresso catturabile; coperta dalle forzate di 5F4B (a = 00, 01, 04, 05) |

**Catture forzate** (`SNAP_FORZA`, suffisso `forzaNN`, stato `finale`, N=1).
5E1A: `forza01` (`iy+1d:0b`), `forza02` e `forza03` (E=0, bit 7 di (IX+4)
spento/acceso), `forza04` (E=1, 5E9D), `forza05` e `forza06` (E=3, 5F33 verso
5F82/5F4B), `forza07` e `forza08` (E=4, 5; 5EDD), `forza09` (`ix+00:00,iy+1d:0b`: bit 5 spento, chiama 5DDF); non E=2 (5FC8 è stub).
5F33: `forza01` (`ix+05:00`), `forza02` (`ix+05:80`). 5D29: `forza01`
(`ix+0d:02`), `forza02`-`03` (5DC3, bit 0 di (IY+1E) spento/acceso),
`forza04`-`08` (5D80: `ret c`, `ret z`, 5D96 nei tre casi), `forza09`
(percorso generale). 5F4B: `forza00`-`forza0f` (`a:00` ... `a:0f`).
Altre forzate: 5DDF `forza01` (`iy+0e:7f,iy+15:7f`); 5E09 `forza01`
(`a:00,c:00`); 5E9D `forza01` (`ix+04:81,ix+05:00`, B negativo: BC = 0002);
5EDD `forza01` (`ix+04:00,ix+05:01`) e `forza02` (`ix+04:20,ix+05:00`).
Totale lotto: 90 catture (49 non forzate e 41 forzate).

**Non coperte da gcov** (`nemici_tipo07a.c`): 5D08-5D28 di 5CFB (610C stub,
N11); la chiamata a `sub_5fc8` e i casi `0` e `default` dello `switch` di 5E1A;
`sub_627f` e la sua uscita in 5E9D (5ED6-5EDA); in 5E9D i rami sulla cella
davanti (5EBB-5EC6, servono celle della griglia c0bd, non forzabili con
`ix`/`iy`); in 5F82 il ramo con bit 2 spento (coda 5FAC) e la scelta finale,
perché 5F82 non è un ingresso catturabile e 5F0C dà l'indice in base alle
celle; il `ret c` di 5E04 in 5DDF (dipende da R e dalle celle). Il corrispondente
di 5F4B è invece coperto dalle forzate `forza00-0f`.
