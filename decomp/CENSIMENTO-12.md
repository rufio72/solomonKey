# Censimento del lotto 12: routine nuove dei quadri bonus e del finale

Brief 16 (2026-10-22). Destinazioni statiche di `call` fra le 104 mai osservate prima del lotto (`out/call_mai_prima_lotto12.txt`) che le sessioni del blocco `lotto12` di `sessioni.sh` hanno eseguito. Base dei lotti di traduzione futuri: nessuna di queste routine è tradotta.

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


## Destinazioni ancora scoperte (64)

`sito_eseguito` da `copertura_call.py --classifica`: NO = il sito della prima call statica non è mai stato eseguito, quindi probabile dato disassemblato come codice o codice raggiungibile solo da zone a loro volta mai eseguite; SI = sito eseguito ma con la call mai presa (ramo condizionale).

| destinazione | prima call statica | sito_eseguito |
|---|---|---|
| 0071 | 85EA | NO |
| 029D | 969E | NO |
| 09A2 | 9DBF | NO |
| 1020 | A308 | NO |
| 1604 | A387 | NO |
| 17A4 | 9CAE | NO |
| 2114 | 1330 | NO |
| 2533 | 6879 | NO |
| 2FC3 | 93D3 | NO |
| 3254 | 8FD3 | NO |
| 3504 | A1CD | NO |
| 4364 | 64A8 | NO |
| 4A05 | 80BB | NO |
| 4FC5 | 4F8F | NO |
| 540A | A09C | NO |
| 5564 | 647C | NO |
| 5604 | A38E | NO |
| 560A | A053 | NO |
| 570A | A056 | NO |
| 5F82 | 5F7B | NO |
| 6046 | 603F | NO |
| 6379 | 6310 | NO |
| 68B0 | AD10 | NO |
| 6FC3 | 6FAA | NO |
| 6FDF | 6F9C | NO |
| 7E03 | 7DA0 | NO |
| 7E14 | 7DC6 | NO |
| 8010 | A289 | NO |
| 801E | A271 | NO |
| 8514 | 1D25 | NO |
| 856C | 1EA1 | NO |
| 8719 | 1EDF | NO |
| 9111 | 1C0B | NO |
| 9557 | 57DC | NO |
| 982C | 3B1F | NO |
| A001 | 8590 | NO |
| A804 | A21A | NO |
| A904 | A217 | NO |
| AB04 | A23B | NO |
| AB14 | 4FAD | NO |
| AEE2 | A88E | NO |
| AEF6 | A8A6 | NO |
| AF00 | A8A9 | NO |
| AF0A | A8DC | NO |
| AF14 | A8B2 | NO |
| AF1E | A8CA | NO |
| AF28 | A8CD | NO |
| AF32 | A8C4 | NO |
| AF3C | A8BE | NO |
| AF46 | A8C1 | NO |
| AF50 | A972 | NO |
| AF5A | A975 | NO |
| AF64 | A978 | NO |
| AF6E | A97B | NO |
| AFA0 | AA1A | NO |
| AFAA | AA1D | NO |
| AFB4 | AA20 | NO |
| AFBE | AA32 | NO |
| AFC8 | AA35 | NO |
| AFD2 | AA38 | NO |
| AFDC | AA62 | NO |
| AFE6 | AA71 | NO |
| AFF0 | AA80 | NO |
| BE2B | 2D83 | NO |
