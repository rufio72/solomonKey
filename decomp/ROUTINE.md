# Routine del Z80 principale tradotte in C

Aggiornato da ogni lotto (schema in `compiti/05-lotto-01-routine.md`).
Stato `tradotta` = C scritto, meno di 2 catture OK; `verificata` = almeno 2
catture tutte OK e regole di qualità rispettate. Verifica da `decomp/harness`:
`./harness 0xXXXX` o `./harness --tutte`. Catture: `sh cattura.sh <ENTRY> <stato> [N]`
da `decomp/`; i frame sono quelli della cattura riuscita.

| ingresso | funzione | file | byte | descrizione | stato | catture OK | stati | note |
|---|---|---|---:|---|---|---|---|---|
| 0x0008 | sub_0008 | z80_0xxx.c | 6 | `rst 08`: DE = A, HL += 2·A (indice in tabella di parole) | verificata | 2 | titolo, gioco | titolo f1002, gioco f2402; solo registri |
| 0x0010 | sub_0010 | z80_0xxx.c | 5 | `rst 10`: DE = parola a HL (anche ROM), HL += 2 | verificata | 2 | titolo, gioco | titolo f1002, gioco f2402; solo registri |
| 0x026C | sub_026c | z80_0xxx.c | 16 | dalla NMI: se c08d bit 7, parola c090-c091 += c08f | verificata | 2 | titolo, gioco | titolo f1002 (scrive), gioco f2402 (`ret z`) |
| 0x05B3 | sub_05b3 | routine.c | 79 | tabella sprite c8ee dagli oggetti a c46e | tradotta | 1 | base | seconda cattura nel lotto 2 |
| 0x237E | sub_237e | z80_2xxx.c | 35 | (HL) = ((HL)+1) & 7f; parola big-endian (HL+1) -= 0150 | verificata | 2 | titolo, gioco | titolo f1399, gioco f1776 (occorrenza 2) |
| 0x23A1 | sub_23a1 | z80_2xxx.c | 11 | parola (HL) += parola (DE), HL e DE += 2 | verificata | 2 | titolo, gioco | titolo f1399, gioco f1775 (occorrenza 2) |
| 0x2402 | sub_2402 | routine.c | 9 | parola DE a HL, HL += 2, DE += BC, A-- | tradotta | 1 | base | seconda cattura nel lotto 2 |
| 0x2418 | sub_2418 | z80_2xxx.c | 13 | HL = 32·D + E (cella di griglia), D = 0 | verificata | 2 | titolo, gioco | titolo f1292, gioco f2419; solo registri |
| 0x249E | sub_249e | z80_2xxx.c | 19 | (HL) += C, parola (HL+3) += D:C, (HL+5) += C, HL += 5 + DE | verificata | 2 | gioco, tardo | gioco f2402, tardo f4002 (scrive) |
| 0x24B1 | sub_24b1 | z80_2xxx.c | 13 | (HL+1..3) = E, A ruotato col riporto, C; E = B, B = C = C+1 | verificata | 2 | gioco, tardo | gioco f2402 (scrive), tardo f4002; usa il riporto in ingresso |
| 0x30EF | sub_30ef | z80_3xxx.c | 18 | A = tabella ROM 3111 a specchio su 64 passi | verificata | 2 | titolo, gioco | titolo f1399 (A=00), gioco f1775 (occorrenza 2, A=20); solo registri |
| 0x326C | sub_326c | fondale.c | 238 | decodifica del fondale (callee 23BB, 23CB, 23EC, 336C) | tradotta | 1 | base | seconda cattura nel lotto 2 |
| 0x3A1C | sub_3a1c | z80_3xxx.c | 18 | due cifre ASCII di A in D, E; A = D | verificata | 2 | titolo, gioco | titolo f1189, gioco f1546 (occorrenza 5); A = 01 in entrambe, ciclo delle decine mai eseguito |
| 0x3F10 | sub_3f10 | z80_3xxx.c | 41 | tempo BCD c093-c095 in 6 cifre a c061, terminatore ff | verificata | 2 | titolo, gioco | titolo f1196, gioco f2403 |
| 0x4AF6 | sub_4af6 | z80_4xxx.c | 24 | inizializza IX+00,04,05,1B,1C,1D,1E di un oggetto | verificata | 2 | titolo, gioco | titolo f1292, gioco f2406 |
| 0x4B0E | sub_4b0e | z80_4xxx.c | 18 | cella (riga:colonna in A) in pixel: H = y, L = x | verificata | 2 | titolo, gioco | titolo f1190, gioco f2406; solo registri |
| 0x5C78 | sub_5c78 | z80_5xxx.c | 22 | classe del byte a DE in C (0..3) | verificata | 2 | gioco, tardo | gioco f2462, tardo f4002 |
| 0x673F | sub_673f | z80_6xxx.c | 11 | azzera la tabella attività c000-c03f | verificata | 2 | titolo, gioco | titolo f3403, gioco f1534 |
