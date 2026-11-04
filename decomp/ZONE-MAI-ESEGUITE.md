# Destinazioni mai eseguite del Z80 principale

Data: 2026-11-03 (lotto 40). Analisi e documentazione, nessuna traduzione.
L'elenco viene da `copertura_call.py --classifica` sulle sessioni di MAME
(`out/*_eseguiti.txt`): delle 299 destinazioni statiche di `call` del Z80
principale (sotto C000), 64 non sono mai state osservate in esecuzione
(`out/call_mai_osservate.txt`, colonna 2 = sito della prima call statica che le
raggiunge). 5 sono nel frattempo state tradotte (sezione «Già tradotte»);
restano 59 verdetti. «Sito mai eseguito» vuol dire che anche l'istruzione
`call` che le nomina non è mai stata eseguita: in tutte le 59 righe il sito è
infatti un dato o un byte di tabella letto come istruzione dal disassemblatore,
tranne 6879, 6310 e 7DA0-7DDE che sono codice vero. Verdetti: `DATO` = la
destinazione non è codice (dato, oppure un byte a metà di un'altra istruzione);
`MORTO` = codice, o falso codice, irraggiungibile da qualunque ingresso;
`RAGGIUNGIBILE` = codice vero con un ingresso reale non ancora esercitato
(condizione scritta; ricetta MAME se provata). Esito: 53 `DATO`, 0 `MORTO`,
6 `RAGGIUNGIBILE` (di cui 5 svegliate o già esercitate con una forzatura).

## Verdetti

| destinazione | verdetto | sito | motivo | ricetta |
|---|---|---|---|---|
| 0071 | DATO | 85EA | cade a metà di `ld hl,lc044h` (0070, eseguita): non è un ingresso; il sito 85EA è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 029D | DATO | 969E | cade a metà di `jr c,l02a1h` (029C, eseguita): non è un ingresso; il sito 969E è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 09A2 | DATO | 9DBF | cade a metà di `ld a,001h` (09A1, eseguita): non è un ingresso; il sito 9DBF è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 1020 | DATO | A308 | cade a metà di `and 00fh` (101F, eseguita): non è un ingresso; il sito A308 è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 1604 | DATO | A387 | cade nella zona dati 1556-234A (byte `00 80 00 80`); il sito A387 è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 17A4 | DATO | 9CAE | cade nella zona dati 1556-234A (byte `02 00 00 00`); il sito 9CAE è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 2114 | DATO | 1330 | cade nella zona dati 1556-234A (byte `FF 00 FF 00`); il sito 1330 è byte 14 21 a cavallo di due parole della tabella della `rst 38h` a 1309 (codice eseguito, tabella mai indicizzata fin qui) | - |
| 2533 | RAGGIUNGIBILE | 6879 | piccola funzione (`ld l,a ... ret`, 2533-253F) subito dopo un `ret`; chiamata dal vero codice 6879, nel blocco 6870-6932 che si raggiunge dal `jr nc` eseguito a 6860 e da `dec c; jr z` non preso a 6876: condizione = punteggio (c239-c23c) maggiore o uguale a una delle 10 voci della classifica a ca4c, con la voce non ultima (C > 1) | svegliata (1 tentativo): `cd /tmp/z40/decomp; SNAP_DIR=/tmp/z40/decomp/out SNAP_POKE=2250:0xc19d:0,2250:0xc19e:0,2290:0xc239:0x99,2290:0xc23a:0x99,2290:0xc23b:0x99,2290:0xc23c:0x99 sh cattura.sh 0x2533 att16 1 z40a` => `CATTURA 0x2533 att16_z40a frame_da=2300 OK`, catturata al frame 2432 (prima HL=FFFC DE=CA4C, dopo PC=687C); senza il poke del punteggio fallisce (limite di 6000 frame) |
| 2FC3 | DATO | 93D3 | cade a metà di `ld ix,lc85eh` (2FC1, eseguita): non è un ingresso; il sito 93D3 è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 3254 | DATO | 8FD3 | cade a metà di `jr z,l3258h` (3253, eseguita): non è un ingresso; il sito 8FD3 è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 3504 | DATO | A1CD | cade a metà di `ld a,(lc243h)` (3502, eseguita): non è un ingresso; il sito A1CD è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 4364 | RAGGIUNGIBILE | 64A8 | istruzione `add hl,de` a metà del blocco 4360-437B (azione di sub_4319 per A-0F = 5 o 6, `jr c,4360` a 4324), già tradotto in `z80_4xxx.c` r. 504; il call dal sito 64A8 è falso (byte della tabella `rst 20h` a 649B), ma il codice si esegue se l'oggetto portato ha codice 0x14-0x15 | già svegliata in precedenza: cattura forzata `forza08` di sub_4319 (`out/snap_0x4319_gioco_oggetti_forza08_regs.txt`, prima AF=1512 cioè A=0x15; ricetta in ROUTINE.md r. 772: `a:15` e poke di c42a); non rifatta; condizione naturale: oggetto portato di codice 0x14 o 0x15 con bit 3 di (HL-7) acceso |
| 4A05 | DATO | 80BB | cade a metà di `jr c,l4a08h` (4A04, eseguita): non è un ingresso; il sito 80BB è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 540A | DATO | A09C | cade a metà di `set 0,c` (5409, non eseguita): non è un ingresso; il sito A09C è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 5564 | DATO | 647C | cade a metà di `ld c,027h` (5563, eseguita): non è un ingresso; il sito 647C è byte della tabella della `rst 20h` a 646B (in realtà 10 parole, 646C-647F, Note 3) | - |
| 5604 | DATO | A38E | cade a metà di `res 1,(iy+01eh)` (5602, non eseguita): non è un ingresso; il sito A38E è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 560A | RAGGIUNGIBILE | A053 | `ld (iy+01dh),002h` di sub_55c9 (tipo 12, fase 4, `nemici_tipi_10_13.c` r. 78), raggiunta per fall-through da 5606 e non per call; il call dal sito A053 è falso (dato); condizione: (iy+12) diverso da 0, (iy+1a) con bit 4 (bit 5 se (iy+1e) bit 0) spento e bit 1 di c081 e c080 spenti | non svegliata: 0 tentativi MAME; (iy+1a) e c07e-c081 sono ricalcolati da sub_5c32 (celle davanti al nemico) a ogni passo, quindi un poke non regge; richiede un nemico di tipo 12 (stanza07 o stanza10) con via libera sul percorso |
| 570A | DATO | A056 | cade nella tabella di parole della `rst 38h` a 5704 (tipo 10, byte 57 di una parola 57xx, 5705-5710); il sito A056 è un dato della zona 80B2-BFFF | - |
| 6379 | RAGGIUNGIBILE | 6310 | corpo di disegno chiamato 7 volte (6310-6358) dal vero codice dello stato `62E0` dell'attività 27 (tabella `rst 20h` a 62B9: 62E0, 62BE, indice c1ae); la corsa att_27 esegue 62B6/62B9 ma con c1ae fuori tabella, quindi 62E0 non parte | svegliata (1 tentativo): `cd /tmp/z40/decomp; SNAP_DIR=/tmp/z40/decomp/out SNAP_POKE=2250:0xc19d:0,2250:0xc19e:0,2290:0xc1ae:0 sh cattura.sh 0x6379 att27 1 z40a` => `CATTURA 0x6379 att27_z40a frame_da=2300 OK`, catturata al frame 2311 (occorrenza 3, le prime due scartate per NMI); senza il poke di c1ae fallisce |
| 68B0 | DATO | AD10 | cade a metà di `call sub_6771h` (68AF, non eseguita): non è un ingresso; il sito AD10 è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 7E03 | RAGGIUNGIBILE | 7DA0 | routine vera (`ld a,003h ... ret`) chiamata da 7DA0 e 7DB3, voci 2 e 6 della tabella `rst 20h` a 7CEC; indice = bit 5-7 di e605 (DSW2 Bonus Life), osservato solo 0 (voce 7D6A) | svegliata (1 tentativo): dip Bonus Life = `30k 200k` (valore 64, indice 2) con `<port tag=":DSW2" type="DIPSWITCH" mask="224" defvalue="0" value="64"/>` in `/tmp/z40/home/.mame/cfg/solomon.cfg`; `cd /tmp/z40/decomp; HOME=/tmp/z40/home SNAP_DIR=/tmp/z40/decomp/out sh cattura.sh 0x7E03 gioco 1 z40a` => `CATTURA 0x7E03 gioco_z40a frame_da=1500 OK`, frame 1542, dopo PC=7DA3 (chiamata da 7DA0) |
| 7E14 | RAGGIUNGIBILE | 7DC6 | routine vera (`ld a,004h ... ret`) chiamata da 7DC6, 7DD2 e 7DDE, voci 1, 3 e 5 della tabella `rst 20h` a 7CEC; indice = bit 5-7 di e605 (DSW2 Bonus Life), osservato solo 0 | svegliata (1 tentativo): come 7E03 con dip Bonus Life = `30k` (valore 32, indice 1, `value="32"` nel cfg): `cd /tmp/z40/decomp; HOME=/tmp/z40/home SNAP_DIR=/tmp/z40/decomp/out sh cattura.sh 0x7E14 gioco 1 z40a` => `CATTURA 0x7E14 gioco_z40a frame_da=1500 OK`, frame 1542, dopo PC=7DC9 (chiamata da 7DC6) |
| 8010 | DATO | A289 | cade a metà di `ld a,(ix+00eh)` (800E, eseguita): non è un ingresso; il sito A289 è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 801E | DATO | A271 | cade a metà di `bit 1,(iy+001h)` (801D, non eseguita): non è un ingresso; il sito A271 è dato della zona 80B2-BFFF (mai eseguita, Note 1), letto come call | - |
| 8514 | DATO | 1D25 | cade nella zona dati 80B2-BFFF (byte `00 00 00 00`); il sito 1D25 è dato della zona 1556-234A (mappe di bit delle stanze da 1557 e tabelle, Note 1), letto come call | - |
| 856C | DATO | 1EA1 | cade nella zona dati 80B2-BFFF (byte `F2 E0 D0 E0`); il sito 1EA1 è dato della zona 1556-234A (mappe di bit delle stanze da 1557 e tabelle, Note 1), letto come call | - |
| 8719 | DATO | 1EDF | cade nella zona dati 80B2-BFFF (byte `9A 1B 76 1B`); il sito 1EDF è dato della zona 1556-234A (mappe di bit delle stanze da 1557 e tabelle, Note 1), letto come call | - |
| 9111 | DATO | 1C0B | cade nella zona dati 80B2-BFFF (byte `54 4A 54 1B`); il sito 1C0B è dato della zona 1556-234A (mappe di bit delle stanze da 1557 e tabelle, Note 1), letto come call | - |
| 9557 | DATO | 57DC | cade nella zona dati 80B2-BFFF (byte `15 97 69 97`); il sito 57DC è byte a cavallo di due parole della tabella della `rst 38h` a 57D3 (tipo 09, 8 voci) | - |
| 982C | DATO | 3B1F | cade nella zona dati 80B2-BFFF (byte `02 2F 9F 02`); il sito 3B1F è byte a cavallo di due parole della tabella della `rst 20h` a 3B10 (40 voci, DATI.md) | - |
| A001 | DATO | 8590 | cade nella zona dati 80B2-BFFF (byte `80 47 4C 0A`, nessuna esecuzione fin qui); il sito 8590 è un dato letto come call | - |
| A804 | DATO | A21A | cade nella zona dati 80B2-BFFF (byte `26 AD 26 AD`, nessuna esecuzione fin qui); il sito A21A è un dato letto come call | - |
| A904 | DATO | A217 | cade nella zona dati 80B2-BFFF (byte `E2 AE FC E2`, nessuna esecuzione fin qui); il sito A217 è un dato letto come call | - |
| AB04 | DATO | A23B | cade nella zona dati 80B2-BFFF (byte `00 E2 AE 00`, nessuna esecuzione fin qui); il sito A23B è un dato letto come call | - |
| AB14 | DATO | 4FAD | cade nella zona dati 80B2-BFFF (byte `E2 AE 6C E2`); il sito 4FAD è byte 14 AB a cavallo delle parole 14FC e 13AB della tabella della `rst 38h` a 4FAC (6 voci) | - |
| AEE2 | DATO | A88E | dato della famiglia AEE2-AFF0 (byte `00 00 03 00 00 00 00 02 00 00`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito A88E letto come call (Note 2) | - |
| AEF6 | DATO | A8A6 | dato della famiglia AEE2-AFF0 (byte `E8 00 02 00 00 00 00 02 C0 00`), indirizzato da un gruppo di tre byte `DC lo hi` della tabella A88E-AA80, che è il sito A8A6 letto come call (Note 2) | - |
| AF00 | DATO | A8A9 | dato della famiglia AEE2-AFF0 (byte `E8 00 02 00 00 00 00 02 40 FF`), indirizzato da un gruppo di tre byte `DC lo hi` della tabella A88E-AA80, che è il sito A8A9 letto come call (Note 2) | - |
| AF0A | DATO | A8DC | dato della famiglia AEE2-AFF0 (byte `E8 00 02 00 00 00 00 02 00 00`), indirizzato da un gruppo di tre byte `DC lo hi` della tabella A88E-AA80, che è il sito A8DC letto come call (Note 2) | - |
| AF14 | DATO | A8B2 | dato della famiglia AEE2-AFF0 (byte `E8 00 02 E8 01 00 00 03 00 00`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito A8B2 letto come call (Note 2) | - |
| AF1E | DATO | A8CA | dato della famiglia AEE2-AFF0 (byte `E8 00 02 00 00 00 00 02 C0 00`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito A8CA letto come call (Note 2) | - |
| AF28 | DATO | A8CD | dato della famiglia AEE2-AFF0 (byte `E8 00 02 00 00 00 00 02 40 FF`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito A8CD letto come call (Note 2) | - |
| AF32 | DATO | A8C4 | dato della famiglia AEE2-AFF0 (byte `E8 00 02 00 00 00 00 02 00 00`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito A8C4 letto come call (Note 2) | - |
| AF3C | DATO | A8BE | dato della famiglia AEE2-AFF0 (byte `E8 00 02 00 00 00 00 02 80 00`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito A8BE letto come call (Note 2) | - |
| AF46 | DATO | A8C1 | dato della famiglia AEE2-AFF0 (byte `E8 00 02 00 00 00 00 02 80 FF`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito A8C1 letto come call (Note 2) | - |
| AF50 | DATO | A972 | dato della famiglia AEE2-AFF0 (byte `00 00 03 00 00 00 00 03 00 02`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito A972 letto come call (Note 2) | - |
| AF5A | DATO | A975 | dato della famiglia AEE2-AFF0 (byte `00 00 03 00 00 00 00 03 00 FE`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito A975 letto come call (Note 2) | - |
| AF64 | DATO | A978 | dato della famiglia AEE2-AFF0 (byte `00 00 03 00 02 00 00 03 00 00`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito A978 letto come call (Note 2) | - |
| AF6E | DATO | A97B | dato della famiglia AEE2-AFF0 (byte `00 00 03 00 FE 00 00 03 00 00`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito A97B letto come call (Note 2) | - |
| AFA0 | DATO | AA1A | dato della famiglia AEE2-AFF0 (byte `A9 00 04 67 03 E0 00 03 10 02`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito AA1A letto come call (Note 2) | - |
| AFAA | DATO | AA1D | dato della famiglia AEE2-AFF0 (byte `A9 00 04 67 03 20 00 03 F0 FD`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito AA1D letto come call (Note 2) | - |
| AFB4 | DATO | AA20 | dato della famiglia AEE2-AFF0 (byte `F0 00 04 8D 01 00 00 03 00 00`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito AA20 letto come call (Note 2) | - |
| AFBE | DATO | AA32 | dato della famiglia AEE2-AFF0 (byte `C0 00 03 00 02 E0 00 03 10 02`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito AA32 letto come call (Note 2) | - |
| AFC8 | DATO | AA35 | dato della famiglia AEE2-AFF0 (byte `C0 00 03 00 02 20 00 03 F0 FD`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito AA35 letto come call (Note 2) | - |
| AFD2 | DATO | AA38 | dato della famiglia AEE2-AFF0 (byte `F0 00 03 00 00 00 00 03 00 00`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito AA38 letto come call (Note 2) | - |
| AFDC | DATO | AA62 | dato della famiglia AEE2-AFF0 (byte `C0 00 03 00 02 E0 00 03 00 02`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito AA62 letto come call (Note 2) | - |
| AFE6 | DATO | AA71 | dato della famiglia AEE2-AFF0 (byte `C0 00 03 00 02 20 00 03 00 FE`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito AA71 letto come call (Note 2) | - |
| AFF0 | DATO | AA80 | dato della famiglia AEE2-AFF0 (byte `F8 00 03 00 00 00 00 03 00 00`), indirizzato da un gruppo di tre byte `FC lo hi` della tabella A88E-AA80, che è il sito AA80 letto come call (Note 2) | - |
| BE2B | DATO | 2D83 | cade nella zona dati 80B2-BFFF (byte `FF 00 FF 00`); il sito 2D83 è byte della tabella di parole 2D4A-2D89, dopo il `jr` di 2D48 | - |

## Già tradotte

Verificate con la definizione `void sub_XXXX(` nei sorgenti C di `decomp/harness/`.

| destinazione | stato | dove | nota |
|---|---|---|---|
| 4FC5 | TRADOTTA | `nemici_comparsa.c` r. 223 | fata; 7 scatti dal vivo, sito 4F8F eseguito 1 volta (CENSIMENTO-NEMICI.md) |
| 5F82 | TRADOTTA | `nemici_tipo07a.c` r. 435 | chiamata da `scegli_5f4b_5f82`/5F7B; coperta dalle forzate di 5F33 e 5E1A |
| 6046 | TRADOTTA | `nemici_tipo07b.c` r. 332 | in `routine_tab.c`, 13 catture, 2 non forzate |
| 6FC3 | TRADOTTA | `nemici_tipi_08_14.c` r. 423 | `static`, raggiunta da 6F91 (forzata `stanza08_forza6fc3`) |
| 6FDF | TRADOTTA | `nemici_tipi_08_14.c` r. 443 | `static`, idem (`stanza08_forza6fdf`) |

## Note

1. **Zone dati.** 80B2-BFFF: nessun PC eseguito fra 80B2 e C000 in tutte le
   sessioni (`out/eseguiti_unione.txt`: ultimo PC 80B2, il `ret` che chiude
   la routine 80A5; poi solo C000-C82E, RAM/porte). Il contenuto sono tabelle
   a record corti di 3-4 byte (byte, parola), puntatori e script di sprite
   (esempi: A048 `0A 57 4C 0A 48 0C`, A210 `A8 44 04 A9 44 04`, A270
   `80 DC 1E 80 DB 1E`), più le tabelle note di DATI.md (8578, 858E, 8DD4,
   8EA4, 8C93). Il disassemblato le legge come `call` quando una parola
   little-endian è preceduta da un byte che vale un opcode di call (C4, CC,
   D4, DC, E4, EC, F4, FC, CD). Per le 44 destinazioni che hanno un sito lì
   non c'è alcun ingresso di codice: nessun `jp`/`call` eseguito entra in
   80B2-BFFF. 1556-234A: mappe di bit delle stanze da 1557 (48 byte per
   stanza × 52, fino a 1F17) e tabelle seguenti (DATI.md); mai eseguita
   (`RAPPORTO.md` r. 31-36). Ipotesi non verificata: i dati ≥ 80B2 sono letti
   da codice eseguito con puntatori caricati da `ld hl,nnnn` (è il modo in cui
   DATI.md ricostruisce 8578/858E/8EA4); non ho cercato i lettori dei singoli
   script di sprite.
2. **Famiglia AEE2-AFF0.** 21 destinazioni (AEE2, AEF6, AF00, AF0A ... AFF0)
   con siti in A88E-AA80: in quella zona stanno gruppi di tre byte `FC lo hi`
   (a volte `DC lo hi`) ripetuti, per esempio `FC E2 AE` a ripetizione in
   A88E-A8A3 e `DC 0A AF` in A8DC; la parola è un puntatore a dati in
   AEE2-AFF0 (21 puntatori distinti fra AEE2 e AFF0 nell'intervallo
   A880-AA90). Le destinazioni contengono sequenze regolari
   (`E8 00 02 00 00 00 00 02 C0 00`, `00 00 03 00 00 00 00 03 00 02`, con
   `00 03`/`00 02` ricorrenti ogni 5 byte circa): ipotesi non verificata,
   record di movimento o di script di sprite, non codice (byte come E8, F0,
   C0 come primo byte dei record non formano istruzioni sensate). Nessun
   `ld rr,nnnn` eseguito carica indirizzi in A800-AAFF o AE00-AFFF e nessun
   `jp (hl)`/`rst` eseguito usa quelle tabelle come indirizzi di codice
   (domanda aperta 5 del brief: DATO); le basi calcolate non sono state
   indagate.
3. **Tabelle `rst 20h` più lunghe di quanto dichiarato.** `rst20_tables.h`
   dà 3 voci alla tabella di 646B, ma le dieci parole 646C-647F sono tutte
   ingressi di codice già eseguiti (64D3 64B4 6539 64C4 657B 65D0 64C8
   6644 64CC 6655): le destinazioni 5564 (sito 647C) e le analoghe sono byte
   di quella tabella letti sfasati. Lo stesso per 649B: parole
   674A 64BC 675B 64C0 6842 6785 64CC 6A43 69A7 6ADE (649C-64AF); il sito 64A8
   (destinazione 4364) è fatto dal byte alto di 64CC e dal byte basso di
   6A43. Non è stato toccato `rst20_tables.h` (fuori perimetro).
   **Corretto nel brief 51 (2026-11-04)**: `rst20_646B` ha ora le 10 voci,
   anche nel generatore `rst20.py` (eccezione `VOCI_NOTE`); banco identico.
4. **Cosa resta da svegliare.** Delle 6 `RAGGIUNGIBILE` sono svegliate con
   ricetta propria 2533, 6379, 7E03, 7E14; 4364 è già esercitata dalla
   forzata `forza08` di sub_4319; resta non svegliata solo 560A (0 tentativi
   di MAME, motivo nella riga). Nessuna richiede una partita dal vivo
   (domanda 6 del brief non attivata).
5. **MORTO: nessuna.** Le candidate a codice morto (blocchi di vero codice
   senza ingresso) si sono rivelate tutte raggiungibili da un ramo
   condizionale: il blocco 4360-437B è ramo di sub_4319 (`jr c,4360` a 4324),
   il blocco 6870-6932 è ramo di `jr nc,6870` (6860), il blocco 55F7-560E è il
   fall-through di 55F6. La destinazione 2533 resta funzione senza altro
   chiamante che 6879.
6. **Ricette MAME.** Tutte in una copia di lavoro fuori dal repository
   (`/tmp/z40/`, `roms` collegata, copie non modificate di `cattura.sh` e
   `snap.lua`, `SNAP_DIR=/tmp/z40/decomp/out`). Il dip switch non si fissa da
   `snap.lua`: si usa un `solomon.cfg` in un `HOME` dedicato
   (`/tmp/z40/home/.mame/cfg/solomon.cfg`, vedi righe 7E03/7E14);
   `cattura.sh` lo eredita senza modifiche. Costo totale di MAME: circa 1 minuto
   (8 corse da 1-15 s, tutte con `-video none -sound none` e senza finestra).
   Controprove: `att27` senza poke di c1ae e `att16` con punteggio 0 falliscono
   (limite di 6000 frame), quindi i poke sono necessari.
7. **Fatti del brief confermati.** Siti e destinazioni coincidono con
   `call_mai_osservate.txt`; i commenti `;AAAA` dei `.dasm` sono indirizzi
   esadecimali minuscoli (anche quando contengono solo cifre: `;1330` è 0x1330).
   Il tetto di 40 minuti di MAME non è stato avvicinato.

## Come si riproduce

Da `decomp/` (cartelle di uscita scratch, mai `out`):

```
mkdir -p /tmp/z40/out
python3 copertura_call.py --classifica --uscite /tmp/z40/out out/*_eseguiti.txt
grep -v '^#' /tmp/z40/out/call_mai_osservate.txt | cut -f1 | grep -v -E '^(4FC5|5F82|6046|6FC3|6FDF)$' | wc -l   # 59
grep -n ';SITO' out/main64.dasm        # righe intorno al sito (SITO in esadecimale minuscolo)
```

Le ricette MAME sono nelle righe 2533, 6379, 7E03, 7E14 della tabella.
Prima di rilanciarle: `mkdir -p /tmp/z40/decomp/out; ln -sfn <repo>/roms /tmp/z40/roms;
cp cattura.sh snap.lua /tmp/z40/decomp/`. Il file `/tmp/z40/home/.mame/cfg/solomon.cfg` per 7E03:

```
<?xml version="1.0"?>
<mameconfig version="10">
    <system name="solomon">
        <input>
            <port tag=":DSW2" type="DIPSWITCH" mask="224" defvalue="0" value="64" />
        </input>
    </system>
</mameconfig>
```
(`value="32"` per 7E14.)
