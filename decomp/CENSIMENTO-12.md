# Censimento del lotto 12: routine nuove dei quadri bonus e del finale

Brief 16 (2026-10-22). Destinazioni statiche di `call` fra le 104 mai osservate prima del lotto (`out/call_mai_prima_lotto12.txt`) che le sessioni del blocco `lotto12` di `sessioni.sh` hanno eseguito. Base dei lotti di traduzione; al brief 16 nessuna di queste routine era tradotta, oggi lo sono tutte tranne le 8 documentate in fondo (sezione in fondo).

Colonne: **estensione** = blocco di PC eseguiti contigui (passo ≤ 4 byte) che contiene l'ingresso, unione delle sessioni del lotto; può unire routine adiacenti o fermarsi prima della fine su rami mai presi. **Invocazioni** = somma delle chiamate osservate (`out/<sessione>_chiamate.txt`), con i siti chiamanti `sito×numero`. Le occorrenze attraversate dalla NMI si vedono solo in cattura (`cattura.sh`), non qui.

Sessioni: `porta`, `bonus_04`, `bonus_42`, `finale`, `finale2`, `oggetti`, `attract2`, `att_16`, `att_1D`, `att_25`, `att_26`, `att_27`.

Coperte: 40 destinazioni su 104, in 25 blocchi (voci del censimento).

| ingresso | estensione | invocazioni | chiamanti | sessioni | note |
|---|---|---|---|---|---|
| 02A2 | 026C-0313 | 0 | - | finale2 oggetti | nessuna call osservata: raggiunta per salto o caduta |
| 02A4 | 026C-0313 | 0 | - | finale2 oggetti | nessuna call osservata: raggiunta per salto o caduta |
| 02A5 | 026C-0313 | 0 | - | finale2 oggetti | nessuna call osservata: raggiunta per salto o caduta |
| 03EE | 03B8-071A | 1 | 010B×1 | bonus_04 | - |
| 0CFD | 0CE3-0D17 | 547 | 0CF9×547 | finale2 oggetti | - |
| 0E0B | 0E04-0E47 | 0 | - | oggetti | nessuna call osservata: raggiunta per salto o caduta |
| 11A5 | 11A5-11D1 | 0 | - | oggetti | nessuna call osservata: raggiunta per salto o caduta |
| 134B | 1332-1352 | 2 | 13BC×1 13C6×1 | oggetti | - |
| 1471 | 13FC-14EE | 3 | 1429×1 1444×1 1447×1 | oggetti | - |
| 1474 | 13FC-14EE | 3 | 142F×1 144E×1 1452×1 | oggetti | - |
| 147F | 13FC-14EE | 2 | 1491×1 14AA×1 | oggetti | - |
| 1533 | 1533-1556 | 4 | 13D5×2 13E6×2 | oggetti | - |
| 2425 | 237E-2449 | 632 | 4090×632 | porta bonus_04 bonus_42 finale finale2 | - |
| 2430 | 237E-2449 | 5 | 40B5×5 | porta bonus_04 bonus_42 finale finale2 | - |
| 2441 | 237E-2449 | 1 | 4302×1 | oggetti | - |
| 250E | 2457-251C | 1849 | 5C9E×1849 | finale2 oggetti | - |
| 3CC1 | 3CAD-3CC9 | 162 | 3BC8×1 3BCD×1 3BD4×1 3BD8×1 3BE4×130 3C0B×28 | att_25 | - |
| 3CC2 | 3CAD-3CC9 | 20 | 3BF6×9 3BFB×9 3C04×1 3C12×1 | att_25 | - |
| 3D20 | 3D20-3E71 | 1 | 3C8C×1 | att_25 | - |
| 3D2F | 3D20-3E71 | 1 | 3CAD×1 | att_25 | - |
| 3D43 | 3D20-3E71 | 3 | 3C7E×1 3C95×1 3CB6×1 | att_25 | - |
| 3DA6 | 3D20-3E71 | 5 | 3D8E×4 3E25×1 | att_25 att_26 | - |
| 3DEA | 3D20-3E71 | 2 | 3DE2×1 3E37×1 | att_25 att_26 | - |
| 3FB2 | 3F88-3FE7 | 6 | 3C38×1 3F9C×5 | porta bonus_04 bonus_42 finale finale2 att_25 | - |
| 3FCE | 3F88-3FE7 | 36 | 3FBF×18 3FC6×18 | porta bonus_04 bonus_42 finale finale2 att_25 | cattura pilota `porta` (frame 2386, fuori NMI) |
| 40FF | 3FF7-4170 | 0 | - | porta bonus_04 bonus_42 finale finale2 | spedidore a passi (rst 20 a 4170); nessuna call osservata: raggiunta per salto o caduta |
| 4302 | 42D4-431D | 1 | 42AA×1 | oggetti | - |
| 4319 | 42D4-431D | 1 | 4288×1 | oggetti | - |
| 4ADA | 4ADA-4CA1 | 1 | 4AB6×1 | bonus_04 | - |
| 5115 | 50BA-511E | 1 | 510E×1 | finale | - |
| 5124 | 5123-5127 | 4 | 5107×4 | finale | - |
| 51FA | 51D7-5207 | 5 | 510A×4 5111×1 | finale | cattura pilota `finale` (frame 3311, fuori NMI) |
| 5DDF | 5DDF-5DFA | 1 | 5E1E×1 | finale | - |
| 5E09 | 5E09-5E40 | 1 | 5DF0×1 | finale | - |
| 5FDC | 5FC8-5FE5 | 1 | 5FD5×1 | finale | - |
| 6248 | 6248-6260 | 1 | 6179×1 | finale | - |
| 6976 | 6976-69F0 | 1 | 6849×1 | att_16 | - |
| 7B64 | 7B3B-7BAF | 0 | - | finale2 | nessuna call osservata: raggiunta per salto o caduta |
| 7B95 | 7B3B-7BAF | 242 | 7B58×121 7B64×121 | finale2 | - |
| 7B9E | 7B3B-7BAF | 5 | 7B78×1 7B7E×1 7B84×1 7B8A×1 7B90×1 | finale2 | - |

Nessuna estensione contiene accessi diretti alle porte e6xx/e8xx. Le
voci con più ingressi nello stesso blocco (3CAD-3CC9, 3D20-3E71, 13FC-14EE,
237E-2449, 7B3B-7BAF) sono probabilmente più routine adiacenti: il taglio
fine spetta ai lotti di traduzione.

## Note sulle sessioni

- `porta`, `bonus_04`, `bonus_42`: 1C dal frame 2300 al 2577-2578, poi 4000
  posta 07 (carico quadro). Il ramo 403D delle stanze nascoste (`set 1,(c24e)`,
  `inc (c24f)`) è eseguito in `bonus_04` e `bonus_42`, non in `porta`; 4032
  (E = 0x32, finale) solo in `finale`.
- Catture pilota: `sh cattura.sh 0x3FCE porta 1` e `sh cattura.sh 0x51FA
  finale 1` (file `out/snap_0x3FCE_porta_*`, `out/snap_0x51FA_finale_*`).
- `finale`: 4032 posta da solo la 24 al frame 2578 (FASE `14 24`); la partita
  prosegue nella stanza finale, `finale2` copre 4500-6900 e 7700-9400.
- `oggetti`: oggetti 08-1B, tre per posta della 22 (coda c05b), una posta
  ogni 100 frame dal 2300; copre i gestori della tabella 130A.
- `attract2`: la attract completa non spedisce mai la 16 (6496); in `finale2`
  la 16 compare al 5269 senza che 6496 sia eseguita. La copre `att_16`.
- `att_27`: forzatura artificiale (in gioco la posta solo la NMI con e603
  bit 0 = 1); coverage.py segnala PC in RAM (violazione C04A -> C04D).
- Sessione `servizio` non fatta: 0CFD, 0E0B, 426A, 6480 risultano coperte
  dalle altre sessioni (condizione della risoluzione 2 del brief).


## Destinazioni ancora scoperte (59 + 5 tradotte)

Lotto 40: delle 64 destinazioni mai osservate nelle sessioni, 5 sono nel frattempo tradotte (4FC5, 5F82, 6046, 6FC3, 6FDF) e per le altre 59 c'è un verdetto (53 `DATO`, 0 `MORTO`, 6 `RAGGIUNGIBILE`) con motivo e ricette in `ZONE-MAI-ESEGUITE.md`.

`sito_eseguito` da `copertura_call.py --classifica`: NO = il sito della prima call statica non è mai stato eseguito, quindi probabile dato disassemblato come codice o codice raggiungibile solo da zone a loro volta mai eseguite; SI = sito eseguito ma con la call mai presa (ramo condizionale).

| destinazione | prima call statica | sito_eseguito | verdetto | note |
|---|---|---|---|---|
| 0071 | 85EA | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 029D | 969E | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 09A2 | 9DBF | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 1020 | A308 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 1604 | A387 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 17A4 | 9CAE | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 2114 | 1330 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 2533 | 6879 | NO | RAGGIUNGIBILE | codice vero con ingresso condizionato, vedi ZONE-MAI-ESEGUITE.md |
| 2FC3 | 93D3 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 3254 | 8FD3 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 3504 | A1CD | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 4364 | 64A8 | NO | RAGGIUNGIBILE | codice vero con ingresso condizionato, vedi ZONE-MAI-ESEGUITE.md |
| 4A05 | 80BB | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 4FC5 | 4F8F | NO | tradotta | vedi «Già tradotte» in ZONE-MAI-ESEGUITE.md |
| 540A | A09C | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 5564 | 647C | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 5604 | A38E | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 560A | A053 | NO | RAGGIUNGIBILE | codice vero con ingresso condizionato, vedi ZONE-MAI-ESEGUITE.md |
| 570A | A056 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 5F82 | 5F7B | NO | tradotta | vedi «Già tradotte» in ZONE-MAI-ESEGUITE.md |
| 6046 | 603F | NO | tradotta | vedi «Già tradotte» in ZONE-MAI-ESEGUITE.md |
| 6379 | 6310 | NO | RAGGIUNGIBILE | codice vero con ingresso condizionato, vedi ZONE-MAI-ESEGUITE.md |
| 68B0 | AD10 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 6FC3 | 6FAA | NO | tradotta | vedi «Già tradotte» in ZONE-MAI-ESEGUITE.md |
| 6FDF | 6F9C | NO | tradotta | vedi «Già tradotte» in ZONE-MAI-ESEGUITE.md |
| 7E03 | 7DA0 | NO | RAGGIUNGIBILE | codice vero con ingresso condizionato, vedi ZONE-MAI-ESEGUITE.md |
| 7E14 | 7DC6 | NO | RAGGIUNGIBILE | codice vero con ingresso condizionato, vedi ZONE-MAI-ESEGUITE.md |
| 8010 | A289 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 801E | A271 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 8514 | 1D25 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 856C | 1EA1 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 8719 | 1EDF | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 9111 | 1C0B | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 9557 | 57DC | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| 982C | 3B1F | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| A001 | 8590 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| A804 | A21A | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| A904 | A217 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AB04 | A23B | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AB14 | 4FAD | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AEE2 | A88E | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AEF6 | A8A6 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF00 | A8A9 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF0A | A8DC | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF14 | A8B2 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF1E | A8CA | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF28 | A8CD | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF32 | A8C4 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF3C | A8BE | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF46 | A8C1 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF50 | A972 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF5A | A975 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF64 | A978 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AF6E | A97B | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AFA0 | AA1A | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AFAA | AA1D | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AFB4 | AA20 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AFBE | AA32 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AFC8 | AA35 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AFD2 | AA38 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AFDC | AA62 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AFE6 | AA71 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| AFF0 | AA80 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |
| BE2B | 2D83 | NO | DATO | dato o byte a metà di istruzione, vedi ZONE-MAI-ESEGUITE.md |

## Destino delle 40 destinazioni (brief 38)

23 tradotte nel lotto 38, 9 già tradotte dai lotti N3-N12, 8 solo documentate; di queste 0CFD, 0E0B e 11A5 (la chiave, tabella 0D18) sono poi state tradotte nel lotto 39.

| ingresso | destino |
|---|---|
| 03EE | tradotta nel lotto 38, `nmi.c` |
| 1471 | tradotta nel lotto 38, `z80_1xxx.c` |
| 1474 | tradotta nel lotto 38, `z80_1xxx.c` |
| 147F | tradotta nel lotto 38, `z80_1xxx.c` |
| 1533 | tradotta nel lotto 38, `z80_1xxx.c` |
| 2425 | tradotta nel lotto 38, `z80_2xxx.c` |
| 2430 | tradotta nel lotto 38, `z80_2xxx.c` |
| 2441 | tradotta nel lotto 38, `z80_2xxx.c` |
| 250E | tradotta nel lotto 38, `z80_2xxx.c` |
| 3CC1 | tradotta nel lotto 38, `z80_38xx.c` |
| 3CC2 | tradotta nel lotto 38, `z80_38xx.c` |
| 3D20 | tradotta nel lotto 38, `z80_38xx.c` |
| 3D2F | tradotta nel lotto 38, `z80_38xx.c` |
| 3D43 | tradotta nel lotto 38, `z80_38xx.c` |
| 3DA6 | tradotta nel lotto 38, `z80_38xx.c` |
| 3DEA | tradotta nel lotto 38, `z80_38xx.c` |
| 3FB2 | tradotta nel lotto 38, `z80_38xx.c` |
| 3FCE | tradotta nel lotto 38, `z80_38xx.c` |
| 4302 | tradotta nel lotto 38, `z80_4xxx.c` |
| 4319 | tradotta nel lotto 38, `z80_4xxx.c` |
| 6976 | tradotta nel lotto 38, `z80_6xxx.c` |
| 7B95 | tradotta nel lotto 38, `z80_7xxx.c` |
| 7B9E | tradotta nel lotto 38, `z80_7xxx.c` |
| 134B | già tradotta e verificata (lotti N3-N12), `nemici_comparsa.c` |
| 4ADA | già tradotta (lotti N3-N12), `nemici_comuni.c` |
| 5DDF | già tradotta, `nemici_tipo07a.c` (col wrapper `banco_r_5ddf`) |
| 5E09 | già tradotta, `nemici_tipo07a.c` |
| 5FDC | già tradotta, `nemici_tipo07b.c` |
| 6248 | già tradotta, `nemici_tipo07b.c` |
| 5115 | già tradotta, `nemici_tipo06a.c` (senza ingresso catturabile) |
| 5124 | già tradotta e verificata, `nemici_tipo06a.c` |
| 51FA | già tradotta e verificata, `nemici_tipo06a.c` |
| 02A2 | documentata: etichetta interna di `sub_027c` (`nmi_oggetti.c`) |
| 02A4 | documentata: etichetta interna di `sub_027c` (`nmi_oggetti.c`) |
| 02A5 | documentata: etichetta interna di `sub_027c` (`nmi_oggetti.c`) |
| 40FF | documentata: interno del ciclo 40FE del corpo dell'attività 1C (spedidore a passi, non tradotto) |
| 7B64 | documentata: interno del passo 7B3B-7B94 (`djnz 7B64`) |
| 0CFD | tradotta nel lotto 39 (`z80_chiave_a.c`/`b.c`): dispatcher della tabella 0D18, continuazione 0CF3 di `sub_0ce6` |
| 0E0B | tradotta nel lotto 39 (`z80_chiave_a.c`/`b.c`, 0E0B dentro 0D8E) |
| 11A5 | tradotta nel lotto 39 (`z80_chiave_a.c`/`b.c`, 11A5 dentro 1172) |
