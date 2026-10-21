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
| 0x004F | sub_004f | z80_0xxx.c | 14 | scrive C nel primo slot libero fra c054-c056 (c057 se pieni) | verificata | 2 | gioco, tardo | gioco f2406, tardo f4037 (scrivono) |
| 0x01C5 | sub_01c5 | z80_0xxx.c | 16 | confronta L col byte corrente del copione c1b7, estrae il codice seguente in c053 | verificata | 2 | gioco, gioco_raro | gioco f1511 (`ret nz`); ramo principale con `SNAP_COND='l==b@(w@c1b7)'` f1515; tardo senza occorrenze entro 6000 frame |
| 0x026C | sub_026c | z80_0xxx.c | 16 | dalla NMI: se c08d bit 7, parola c090-c091 += c08f | verificata | 2 | titolo, gioco | titolo f1002 (scrive), gioco f2402 (`ret z`) |
| 0x05B3 | sub_05b3 | routine.c | 79 | tabella sprite c8ee dagli oggetti a c46e | verificata | 2 | base, gioco | gioco f2402 (scrive 2 byte) |
| 0x07DE | sub_07de | z80_0xxx.c | 196 | avanzamento dell'animazione di un record da 14 byte (tabelle ROM 9555) | verificata | 2 | titolo, gioco | titolo f1293, gioco f2402 (scrivono) |
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
| 0x2AF5 | sub_2af5 | z80_2xxx.c | 19 | 8 righe di testo via sub_2b08 (stringa 0c/0d dal bit di A ruotato col riporto in ingresso) | verificata | 2 | titolo, titolo_n2 | solo titolo/test RAM (s = g = 0): occorrenze 1 (f153) e 2 (f154) |
| 0x2B08 | sub_2b08 | z80_2xxx.c | 33 | stringa C (tab. 2B61) nella destinazione B (tab. 2B29, o HL col bit 7 di B), attributo 40 | verificata | 2 | titolo, titolo_n2 | solo titolo/test RAM (s = g = 0): occorrenze 1 (f152) e 2 (f153) |
| 0x2FF3 | sub_2ff3 | z80_2xxx.c | 13 | HL e DE = differenze di coordinate ×4 col segno (prestito a sub_3000) | verificata | 2 | titolo, gioco | titolo f1305, gioco f1774; solo registri |
| 0x3000 | sub_3000 | z80_3xxx.c | 9 | HL = A·4 con segno dal riporto in ingresso | verificata | 2 | titolo, gioco | titolo f1305, gioco f1774; solo registri; usa il riporto in ingresso |
| 0x3097 | sub_3097 | z80_3xxx.c | 88 | 16 oggetti c46e su un cerchio (sub_30ef + sub_3101, centro c2c2/c2c4, raggio c2c6) | verificata | 2 | titolo, gioco | titolo f1399, gioco f1774 (scrivono) |
| 0x30EF | sub_30ef | z80_3xxx.c | 18 | A = tabella ROM 3111 a specchio su 64 passi | verificata | 2 | titolo, gioco | titolo f1399 (A=00), gioco f1775 (occorrenza 2, A=20); solo registri |
| 0x3101 | sub_3101 | z80_3xxx.c | 16 | byte alto di H·L (moltiplicazione seriale), bit bassi in L | verificata | 2 | titolo, gioco | titolo f1399, gioco f1774; solo registri |
| 0x31CE | sub_31ce | z80_3xxx.c | 19 | scritta fissa 31E1 a dfa7 via sub_6771, poi cifra c238 in ASCII a dfac | verificata | 2 | titolo, gioco | titolo f1189, gioco f1564 (scrivono) |
| 0x31E9 | sub_31e9 | z80_3xxx.c | 53 | barra di min(A,6) colonne (tile 80/81, attr 60) a df80, resto pulito con sub_23ac | verificata | 2 | titolo, gioco | titolo f1189, gioco f1855 (scrivono) |
| 0x325E | sub_325e | z80_3xxx.c | 14 | sovrapposizione A del fondale: record F138 (flusso, C, B) a sub_326c | verificata | 2 | titolo, gioco | titolo f1195, gioco f1541 (occorrenza 2) |
| 0x326C | sub_326c | fondale.c | 238 | decodifica del fondale (callee 23BB, 23CB, 23EC, 336C) | verificata | 2 | base, gioco | gioco f1543 |
| 0x33AA | sub_33aa | z80_3xxx.c | 16 | azzera la palette di stanza c9ca-ca49, bit 6 di c044 | verificata | 2 | titolo, gioco | titolo f3409 (occorrenza 2), gioco f1534 |
| 0x33BA | sub_33ba | z80_3xxx.c | 54 | record palette F000[c243] a c9ca (ldir); passata rld col bit 0 del C d'ingresso | verificata | 2 | titolo, gioco | titolo f1196, gioco f1856 |
| 0x349F | sub_349f | z80_3xxx.c | 53 | tabellone: pulizia (sub_34d4) e caratteri 82+2·(coppie di bit di c25c) con sub_34e1, chiusi da 90/92 | verificata | 2 | titolo, gioco | titolo f1189, gioco f1563 (scrivono) |
| 0x34D4 | sub_34d4 | z80_3xxx.c | 13 | azzera 10 byte da HL in due piani (bit 2 di H) | verificata | 2 | titolo, gioco | titolo f1190 (occorrenza 2), gioco f1563 (scrive) |
| 0x34E1 | sub_34e1 | z80_3xxx.c | 25 | caratteri A e A+1 in colonna (righe da 0x20) a df00+L, attributo 70 | verificata | 2 | titolo, gioco | titolo f1189, gioco f1563 (scrivono) |
| 0x3678 | sub_3678 | z80_3xxx.c | 59 | palette e400 con 4 passate di sub_36bd e 10 byte ROM 36B3 a e496 | tradotta | 1 | titolo | solo titolo f152, occorrenza unica entro 6000 frame (N=2 e gioco falliti) |
| 0x36BD | sub_36bd | z80_3xxx.c | 6 | ripete sub_2402 finché A non arriva a 0 | verificata | 2 | titolo, titolo_n2 | solo titolo (s = g = 0): occorrenze 1 (f152) e 2 (f153) |
| 0x36C3 | sub_36c3 | z80_3xxx.c | 45 | 4 ldir da ROM: 80D3→e400 (0x200), 8199→c96e, 80D3→c994, 82A7→c9b4 | tradotta | 1 | titolo | solo titolo f603, occorrenza unica entro 6000 frame (N=2 e gioco falliti) |
| 0x36F0 | sub_36f0 | z80_3xxx.c | 25 | pixel (H, L) → cella della griglia FG: HL = c0bd+offset, DE = offset | verificata | 2 | gioco, tardo | gioco f2402, tardo f4002; solo registri |
| 0x380A | sub_380a | z80_3xxx.c | 8 | 16 byte 0xff da HL | verificata | 2 | titolo, gioco | titolo f1191 (occorrenza 2), gioco f1564; scrivono entrambe |
| 0x3A1C | sub_3a1c | z80_3xxx.c | 18 | due cifre ASCII di A in D, E; A = D | verificata | 3 | titolo, gioco, titolo_decine | titolo f1189, gioco f1546 (occorrenza 5), A = 01; ramo decine catturato con `SNAP_COND='a>=#10'` in titolo f3406 (A = 0C) |
| 0x3A2E | sub_3a2e | z80_3xxx.c | 38 | coppia B, C dalle tabelle ROM 8578/858E secondo A | verificata | 2 | titolo, gioco | titolo f1483 (occorrenza 3, A=B7), gioco f1856 (A=01); solo registri |
| 0x3F10 | sub_3f10 | z80_3xxx.c | 41 | tempo BCD c093-c095 in 6 cifre a c061, terminatore ff | verificata | 2 | titolo, gioco | titolo f1196, gioco f2403 |
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
| 0x5C78 | sub_5c78 | z80_5xxx.c | 22 | classe del byte a DE in C (0..3) | verificata | 2 | gioco, tardo | gioco f2462, tardo f4002 |
| 0x673F | sub_673f | z80_6xxx.c | 11 | azzera la tabella attività c000-c03f | verificata | 2 | titolo, gioco | titolo f3403, gioco f1534 |
| 0x6771 | sub_6771 | z80_6xxx.c | 20 | stringa fino a ff da HL+1 in due piani da DE, attributo (HL) | verificata | 2 | titolo, gioco | titolo f1189, gioco f1534 (scrivono); estensione reale 6771-6784 |
| 0x7E32 | sub_7e32 | z80_7xxx.c | 8 | DE = parola della tabella 7E3A all'indice A, HL conservato | verificata | 2 | gioco, gioco_n2 | occorrenze 1 (f1542) e 2 (f1543); tardo e titolo senza occorrenze; solo registri |
| 0x7EFE | sub_7efe | z80_7xxx.c | 47 | cella in d486/d496 da D, E, C: A e C in due piani | verificata | 2 | titolo, gioco | titolo f610, gioco f1544 (occorrenza 5); scrivono entrambe |
