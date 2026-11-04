# Compito 54: corpi residui del gioco libero (1C 22 25 26), routine 352E e raccolta 12AB

Data: 2026-11-04. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.
Riferimento di metodo: `compiti/52-corpi-gioco-libero.md` (banco dei corpi,
catture con `SNAP_CORPO`, ricetta 1A). Qui non si ripete ciò che il 52 ha già fatto.

## Obiettivo

Il gioco libero (`harness/gioca`) si ferma oggi in tre punti: «FERMO corpo XX non
tradotto» per i corpi 1C 22 25 26, «FERMO routine 352E mancante» (semi 4 e 67 di
`ricette/genera_ingressi.py`), e ignora la raccolta di certi oggetti
(«1288: continuazione 12AB (raccolta) non tradotta: nessun effetto», 224 righe in
4000 frame della partita registrata `ricette/partita_1a.reg`). Questo lotto
traduce in C i quattro corpi, i rami mancanti della routine 34FA (fase 1 del
corpo 07) e la continuazione 12AB con i 20 gestori di raccolta di 12F0, e li
verifica nel banco con catture MAME. Il corpo 27 e i corpi 15 16 24 restano al lotto 55.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si toccano
  `replay.c`, `sched.c`, `sched_proto.c`, `macchina.c`, `video_*`, `audio_*`,
  `rst20_tables.h`, `rst38_tables.h`, `snap.lua`, `cattura.sh`, `compiti/`
  (tranne questo file: solo la sezione finale con gli esiti).
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le 800
  righe (soglia di lavoro 650 per i moduli nuovi). `gcc` dal Makefile
  (`-std=c11 -Wall -Wextra -O0`), nessuna dipendenza nuova, niente binari né `.o` nel commit.
- MAME solo senza finestra né audio (lo fa già `cattura.sh`); a fine lavoro
  `pgrep -c mame` → `0`. Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`).
  Un solo agente alla volta su `decomp/harness`.
- Nessuna riscrittura di corpi già tradotti. Unica modifica di semantica
  ammessa su codice esistente: `sub_1288` (la continuazione 12AB) e
  `fase_07_34fa` (rami 352E, 351E, 353B), che oggi chiamano `fprintf` o `nemici_manca`.
- I `NEMICI_PUNTO` esistenti in `fase_07_34fa` restano invariati e nello stesso
  ordine; i rami nuovi (mai eseguiti nei 5500 frame del giro) non ne hanno.

## Cosa esiste già (fatti verificati)

Convenzioni dei corpi, banco, Makefile (`SORGENTI_NEMICI` r. 6, ogni file `.c`
nuovo va aggiunto lì) e formato `routine_tab.c` (quinto campo `solo_ram` esplicito
in ogni voce): vedi brief 52. Misurati oggi (da `grafo`, `wc`, binari già compilati):

| file | righe | nota |
|---|---:|---|
| `harness/attivita.c` | 385 | tabella `corpo_attivita_replay` r. 355-385; si aggiungono 4 voci (+ `#include`), resta < 400 |
| `harness/attivita_libera.h` | 54 | prototipi dei corpi del lotto 52, `passo_2e79(uint16_t)` |
| `harness/banco_corpi.c` | 26 | macro `BANCO_CORPO(sfx, fn)`: slot = `[SP] - 0xC000` |
| `harness/routine_tab.c` | 335 | voci `{ "0xNNNN", fn, 0, 0, solo_ram }` in ordine di indirizzo |
| `harness/attivita_attract07a.c` | 226 | `fase_07_34fa` (r. 168-190: la fase 1 del corpo 07) |
| `harness/z80_1xxx.c` | 133 | `sub_1288` r. 17-33 |
| `harness/attivita_18.c` | 211 | `passo_2e79`, `attesa_18` (static), `sub_6735` |
| `harness/attivita_attract07.c` | 307 | `attesa_07(uint16_t hl, uint8_t c, uint16_t sito)` esportata (header `attivita_attract07.h`) |
| `decomp/corpi_catture.txt`, `decomp/cattura_corpi.sh` | 43 (32 righe di dati), 40 | elenco e script del lotto 52 |

**Misure di oggi** (binari già compilati, da `decomp/`):
- `harness/sched_proto out/sched_log.txt | tail -1` → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`
- `harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 | tail -4` →
  `spedizioni eseguite: 64686, fino al frame 5499` / `NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885` / `prima divergenza a metà attività (attesa): frame 2415` / `nessuna divergenza nello spedidore`
- `harness/gioca -n 5434 --muto --oracolo out/giroboot_oracolo.txt out/giroboot_disp.txt --riproduci out/ingressi_replay.txt 2>/dev/null | tail -6` →
  `FRAME_VIDEO 5499: hash C a239d0a940bec0b9 MAME a239d0a940bec0b9 tabella a239d0a940bec0b9, coincide` / `FRAME rese 5434` / `HASH ultimo a239d0a940bec0b9` / `FRAME ultimo 5499` / `AUDIO campioni 4042500` / `AUDIO hash c4b40fea06623a96`
- Banco: `RIEPILOGO 0x1288: 2 catture, 2 OK`, `RIEPILOGO 0x4302: 3 catture, 3 OK`; totali (brief 52) 1310 OK, 322 RIEPILOGO, 0 FALLITO.
- Partita 1A (da `decomp/harness`, 10 s): `./gioca --riproduci ../ricette/partita_1a.reg -n 4000 --muto` →
  `FRAME rese 4000`, `HASH ultimo 3f25ba053eeb56b9`, `AUDIO campioni 2988510`,
  `AUDIO hash 2b2b3e7c4ca136fc`; su stderr 224 righe identiche
  «1288: continuazione 12AB (raccolta) non tradotta: nessun effetto».
- Semi (`python3 ricette/genera_ingressi.py SEME FILE 6000 8`, poi
  `harness/gioca -n 6000 --muto --riproduci FILE`): seme 4 → `FERMO routine 352E
  mancante al frame 5602` (`FRAME rese 5536`); seme 67 → stesso messaggio al frame 4866 (`FRAME rese 4800`).

### Le tabelle dei quattro corpi (da `rst20_tables.h`, `grafo symbol`, script di flusso)

Ogni corpo a passi inizia con `ld hl,c19d; ld a,(hl); inc (hl); rst 20h` (come il 07,
`attivita_07_2ccd`, attivita_attract07.c r. 271-307, il modello da copiare:
`uint8_t fase = M(0xc19d); M(0xc19d) = fase+1; switch (fase)`, `default:
nemici_fuori_tabella(sito, fase)`). Il contatore è **c19d** per tutti e quattro.
Le istruzioni sono del solo codice raggiungibile non ancora tradotto (stima dal
disassemblato `decomp/out/main64.dasm`: seguendo salti e `call`, fermandosi alle
routine con `sub_xxxx` già in C, segnate «(T)»). Tradurre dal disassemblato.

**26 (ingresso 3B63, tabella 3B68, 7 passi, ~121 istr.)**
| idx | voce | istr. | note |
|---:|---|---:|---|
| 0 | 3E34 | 36 | chiama **6502** (non tradotta, 24 istr., 3E34 → `call 6502`), 0493 (T) 2F74 (T) 3DEA (T) 673F (T) |
| 1, 3, 5 | 2E79 | - | attesa: `passo_2e79(0xc19d)` (attivita_libera.h) |
| 2 | 3E0E | 14 | 2FF3 3DA6 4AF6 4B0E (T) |
| 4 | 3D59 | 9 | nessuna chiamata |
| 6 | 6655 | 16 | 0493 673F 75AA (T): **75AA dura più di un frame** |

**25 (ingresso 3B77, tabella 3B7C, 10 passi, ~228 istr.)**
| idx | voce | istr. | note |
|---:|---|---:|---|
| 0 | 3DC6 | 17 | 0493 3DEA 673F (T) |
| 1 | 3DB3 | 5 | - |
| 2, 4, 6, 8 | 2E79 | - | `passo_2e79(0xc19d)` |
| 3 | 3D6A | 26 | 2FF3 3DA6 4AF6 4B0E (T) |
| 5 | 3D4C | 15 | - |
| 7 | 3BAC | 132 | 33BA 3A54 3CC1 3CC2 3D20 3D2F 3D43 3F88 3FB2 4398 4C89 4D05 4D37 6771 75AA (T): **75AA** |
| 9 | 3B91 | 9 | - |

**1C (ingresso 3F40, tabella 3F45, 9 passi, ~158 istr.)**
| idx | voce | istr. | note |
|---:|---|---:|---|
| 0 | 40E1 | 23 | 31E9 33BA 6735 (T) |
| 1 | 41A8 | 2 | `ld c,14h; jp 2e8b`: `attesa_07(0xc19d, 0x14, 0x41a8)` |
| 2 | 40A7 | 26 | 2430 2FF3 3045 4B0E 6735 (T) |
| 3 | 405F | 6 | - |
| 4 | 3F58 | 9 | 6771 75AA (T): **75AA** |
| 5 | 41A8 | 2 | come idx 1 |
| 6 | 3F93 | 13 | 3F88 3FB2 4C89 4D05 4D87 (T) |
| 7 | 41B2 | 2 | `ld c,0c0h; jp 2e8b`: `attesa_07(0xc19d, 0xc0, 0x41b2)` |
| 8 | 3FF7 | 52 | 0493 3F88 673F 75AA (T): **75AA** |

(`sub_75aa` è `z80_7xxx.c` r. 91; i passi con 75AA non si catturano in MAME: l'NMI cade
sempre dentro e `snap.lua` scarta ogni occorrenza. Precedente: passo 7 (670F) del
corpo 13, lotto 52.)

**22 (ingresso 12D6, 23 istr. + 12F0 + 20 gestori, ~265 istr.)** Il corpo **non**
inizia con `pop hl`: `[SP]` è lo slot fino alla fine. Dal disassemblato:
```
12D6 hl=c05b; b=3; c=0
12DC ripeti 3 volte: a=(hl); (hl++)=0; e=(hl); (hl++)=0;     ; coda di 3 coppie (classe, E)
     se a != 0: sub_12f0 con B=a, A=e (sono salvati bc e hl attorno alla call)
12EC pop hl; (hl)=0; ret                                      ; tabella[slot] = 0
12F0 b=a; a=e; sub_4b0e(a) -> c058,c059 = HL; c05a = 0; a = b - 8;
     se a >= 0x21: salta a 13F7 (23 istr.)
     c=0x15; rst 28 (suono 0x15); e=a; rst 38 con indice E nella tabella 130A
```
`rst 38` legge la parola `130A + 2*E`; la tabella ha **20 voci valide** (130A-1331;
dal 1332 c'è codice, non dati). Indice (classe − 8) → gestore:
00:13CD 01:13D5 02:13D1 03:13B9 04:13D1 05:13E6 06:1413 07:139C 08:1424 09:1436
0A:14AA 0B:1491 0C:13C3 0D:13D5 0E:13E6 0F:1413 10:139C 11:13AB 12:135B 13:14D4.
Gli indici 14-20 (classi 1C-28) leggerebbero byte di codice: nessuna cella reale
li ha; tradurli con `nemici_fuori_tabella(0x1309, indice)` (domanda 4).
I 15 gestori distinti + 13F7 sono ~226 istruzioni uniche (12BD-1509, tutte le
chiamate sono (T): 0493 1471 1474 147F 134B 1533 1549 31E9 349F, sub_4b0e); molti
condividono code in coda. **Attenzione**: `decomp/out/main64.dasm` decodifica 1330 come
`call nc,2114h` (D4 14 21) e l'istruzione a 1332 (`ld hl,c82e; res 6,(hl); ld c,25h; rst 28; ld a,1ch;
call 0493; ret`, byte 21 2E C8 CB B6 0E 25 EF 3E 1C CD 93 04 C9) non compare: è il
ramo «classe 2» di 12AB, non un gestore.

**Continuazione 12AB** (`sub_1288` in `z80_1xxx.c` r. 17-33 oggi stampa e ritorna
dopo `A = classe - 0x37` con carry; A, HL = cella `c0bd + E`, DE = E = offset di cella, IX = c82e).
Dal disassemblato, con classe = valore della cella (< 0x37):
```
12AB classe==0 -> ret.  classe >= 8 -> 12BD.  5..7 -> 1340.  ==2 -> 1332.  1, 3, 4 -> ret.
12BD (hl)=0x37 (cella vuota); hl=c05b; b=2; ripeti: c=(hl); rlc c; se zero esci;
     hl+=2; djnz; poi (hl)=classe; hl++; (hl)=e; a=0x22; call 0493 (posta il corpo 22); ret
     (con la coda piena scrive comunque nel terzo posto: c05f/c060)
1340 (hl)=0x37; c=0x11; rst 28; a=0x18; call 0493 (posta il corpo 18); ret
1332 hl=c82e; res 6,(hl); c=0x25; rst 28; a=0x1c; call 0493 (posta il corpo 1C); ret
```
Quindi la raccolta di classi 5-7 (quella della partita 1A: **classe 07, cella C168,
E=AB**, misurata con una copia del binario che stampa la classe) azzera la cella,
suona 0x11 e posta il corpo **18** (già tradotto); la classe 2 posta il **1C**
(questo lotto); le classi ≥ 8 accodano per il corpo 22. `sub_0493`, `sub_0028`
(`rst 28`, conserva AF BC HL) esistono.

**Fase 1 del corpo 07, routine 34FA** (`fase_07_34fa`, attivita_attract07a.c r. 168-190).
Oggi: `c24e` bit 0 acceso → `nemici_manca(0x352e)`; dopo la copia ROM → se `c251 != FF`
→ `nemici_manca(0x351e)`; se `(c24b + c24a) == FF` → `nemici_manca(0x353b)`. Dal
disassemblato (3519-354A, 46 istr. in tutto, chiamata finale `sub_354b` (T)):
```
3519 de=c251 ripeti: a=(de++); se a==FF vai a 352E;
     cerca a in c24b..c24d (cpir, bc=3): se trovato (hl-1)=FF; ripeti da 3519   ; ramo 351E
352E de=c24a; a=(de++); hl=de+a; a=(hl); se a != FF vai a 3547                  ; già tradotto sotto
353B b=FF; ripeti: a=(de++); b++; se a==FF ripeti; c24a=b                        ; ramo 353B
3547 sub_354b
```
`c24e` vale 05 (bit 0 acceso) nello stato `attNN` di MAME e nei semi 4 e 67: è il
ramo 352E, non un'eccezione.

## Catture provate dall'analista

Provate in una copia fuori dal repo (`decomp/` ridotto a `cattura.sh`, `snap.lua`
attuale con `SNAP_CORPO`, `cattura_corpi.sh`, `roms` collegato), 53 righe, in 48 s con
`xargs -P6`: **51 su 53 OK**, falliscono i due passi con 75AA (`1C k8`, `26 k6`).
Per `1C k4` e `25 k7` la cattura riesce ma è del passo seguente (prima occorrenza
scartata per NMI; `_prima.bin` ha c19d = 5 e 8): sono duplicati di k5 e k8 e
**non vanno nell'elenco**. Non catturabili dunque (tutti per 75AA): **1C idx 4 (3F58) e 8 (3FF7),
25 idx 7 (3BAC), 26 idx 6 (6655)**: si traducono dal disassemblato e si verificano
per composizione (routine già nel banco) e col giro (domanda 3).

Righe da aggiungere a `decomp/corpi_catture.txt` (formato del brief 52, tutte provate):
```
corpo 1C 0x3F40 k0 1 -
corpo 1C 0x3F40 k1 1 2300:0xc19d:1
corpo 1C 0x3F40 k2 1 2300:0xc19d:2
corpo 1C 0x3F40 k3 1 2300:0xc19d:3
corpo 1C 0x3F40 k5 1 2300:0xc19d:5
corpo 1C 0x3F40 k6 1 2300:0xc19d:6
corpo 1C 0x3F40 k7 1 2300:0xc19d:7
corpo 26 0x3B63 k0 1 -
corpo 26 0x3B63 k1 1 2300:0xc19d:1       (k1..k5 analoghe, poke 2300:0xc19d:K)
corpo 25 0x3B77 k0 1 -                   (k0..k6, k8, k9; poke 2300:0xc19d:K; NIENTE k7)
corpo 22 0x12D6 cCC 1 2300:0xc05b:0xCC,2300:0xc05c:0x95   (CC = 08 09 0A ... 1B, 29, 36: 22 righe)
corpo 22 0x12D6 tre 1 2300:0xc05b:0x08,2300:0xc05c:0x95,2300:0xc05d:0x1A,2300:0xc05e:0x95,2300:0xc05f:0x10,2300:0xc060:0x95
corpo 07 0x2CCD f1d 1 2300:0xc19d:1
corpo 07 0x2CCD f1b 1 2300:0xc19d:1,2300:0xc24e:1,2300:0xc24a:0,2300:0xc24b:0xFF,2300:0xc24c:0xFF,2300:0xc24d:3
corpo 07 0x2CCD f1c 1 2300:0xc19d:1,2300:0xc24e:0,2300:0xc251:5
corpo 07 0x2CCD f1e 1 2300:0xc19d:1,2300:0xc24e:0
corpo 07 0x2CCD f1f 1 2300:0xc19d:1,2300:0xc24e:0,2300:0xc251:1
stato gioco 0x1288 rCC 1 2400:0xc152:0xCC            (CC = 00 01 02 03 04 05 07 08 1B 36: 10 righe)
stato gioco 0x1288 rQ08 1 2400:0xc152:0x08,2400:0xc05b:1,2400:0xc05d:1
```
Conteggio: 1C 7, 26 6, 25 9, 22 23 (20 classi 08-1B, 29, 36, `tre`), 07 5, 1288 11 =
**61** catture nuove (totale elenco 93). Misurato su queste catture:
- 12D6 (tutte le classi): `c058-c05a = 48 50 00` dopo; il corpo azzera la coda e lo
  slot; gli indirizzi toccati vanno da c000 a cffb e in alcune classi a dfb6 (video).
- 2CCD: `f1d` (c24e=05, c24a=02) → ramo 352E→3547 senza cambiare c24a-c251;
  `f1b` → ramo 353B (`c24a` da 00 a 02); `f1c` → ramo 351E, byte 5 non trovato,
  nessuna scrittura; `f1f` → ramo 351E con byte trovato (`c24d` da 01 a FF, `c24a` da 02 a 03);
  `f1e` → ramo già tradotto (copia ROM), controllo.
- 1288 (frame 2402, Dana nella cella C152, E=95, posa 6): classi 00 01 03 04 → solo cff2-cff3
  cambiano (`ret`); classe 02 → `c82e` cambia (res 6), DE=C021, HL=C04E, A=1C, BC=0125; classi 05 e 07
  → `c152`=37, DE=C021, HL=C04E, A=18, BC=0111; classi ≥ 08 → `c05b/c05c` scritti, A=22, BC=0200
  (con la coda già occupata in c05b e c05d: BC=0002, scritti c05f/c060).
  La voce `0x1288` del banco **confronta anche i registri**: le uscite sopra (A, BC, DE,
  HL, IX) vanno riprodotte da `sub_1288` anche per le vie nuove.

Estensione di `cattura_corpi.sh` (unica modifica agli strumenti): nuovo modo `stato`
nell'elenco, dove il campo `cod` è il nome dello stato di `cattura.sh` (`gioco`),
senza `SNAP_CORPO` né poke predefiniti: `SNAP_POKE=$poke sh cattura.sh $entry $cod
$n $tag`, e la pulizia iniziale cancella `out/snap_${entry}_${cod}_${tag}_*` (per
`corpo`/`sub` resta `att$cod`). Il file `corpi_catture.txt` documenta il modo nuovo.
Il `.log` rigenerato di una cattura vecchia differisce da quello committato solo per
la riga «Average speed»: ripristinarli con `git checkout -- <file.log>` (solo quei log).

## Struttura da realizzare

Moduli nuovi (nomi per codice, nessuna interpretazione del ruolo; tutti nel
Makefile, prototipi in `attivita_libera.h`):
- `harness/attivita_1c.c` (~190 righe): `attivita_1c_3f40(uint8_t slot)`, i passi 40E1,
  40A7, 405F, 3F58, 3F93, 3FF7; le attese con `attesa_07`.
- `harness/attivita_26.c` (~150 righe): `attivita_26_3b63`, passi 3E34, 3E0E, 3D59, 6655,
  `sub_6502` (24 istr.). I passi 3D6A/3DC6/3E0E/3E34 condividono routine con il 25:
  le sub-routine comuni (3DA6, 3DEA e simili) sono già (T); se un passo del 25 e uno del 26
  coincidono nel codice, una sola funzione dichiarata nell'header.
- `harness/attivita_25.c` (~280 righe): `attivita_25_3b77`, passi 3DC6, 3DB3, 3D6A, 3D4C,
  3BAC (132 istr.; se supera ~200 righe, il passo 3BAC in `attivita_25b.c`), 3B91.
- `harness/attivita_22.c` (~120 righe): `attivita_22_12d6(uint8_t slot)` (la coda di 3 coppie,
  `tabella[slot] = 0`) e `sub_12f0` con `switch` sull'indice (classe − 8) per i 20 gestori.
- `harness/raccolta_13xx.c` (~330 righe): i 15 gestori distinti (135B 139C 13AB 13B9 13C3 13CD
  13D1 13D5 13E6 13F7 1413 1424 1436 1491 14AA 14D4) come funzioni `gestore_XXXX`; le code
  condivise sono funzioni statiche. Prototipi in `attivita_libera.h` (`void gestore_XXXX(struct z80 *z)`).
Modifiche a file esistenti:
- `z80_1xxx.c`: `sub_1288` chiama una funzione `raccolta_12ab(struct z80 *z, uint8_t classe)` per
  le vie 12AB-1340-1332-12BD (nuova, nello stesso file; ~50 righe, file ~190). Il commento di
  testa di `sub_1288` si aggiorna; l'`fprintf` sparisce.
- `attivita_attract07a.c`: i tre rami mancanti di `fase_07_34fa` (ordine di traduzione:
  352E con 3547, poi 353B, poi 351E); resta sotto 260 righe.
- `attivita.c`: `[0x1c]`, `[0x22]`, `[0x25]`, `[0x26]` in `corpo_attivita_replay`;
  aggiornare il commento di testa.
- `banco_corpi.c`: cinque wrapper `BANCO_CORPO(12d6, attivita_22_12d6)`, `2ccd`
  (`attivita_07_2ccd`), `3b63`, `3b77`, `3f40`; prototipi in `attivita_libera.h`.
- `routine_tab.c`: cinque voci `{ "0x12D6", banco_corpo_12d6, 0, 0, 1 }` e analoghe per
  0x2CCD, 0x3B63, 0x3B77, 0x3F40, in ordine di indirizzo.
- `Makefile`: i cinque `.c` nuovi in `SORGENTI_NEMICI`.
- `cattura_corpi.sh` e `corpi_catture.txt` come sopra.
Regola: prima le catture (con lo script), poi la traduzione, poi il banco: ogni corpo
e ogni gestore si guida dal diff del banco, non a occhio.

### Documenti

`decomp/SCHEDULER.md` (tabella 3B11: 1C, 22, 25, 26 «tradotta (lotto 54)», cercare
le righe con `grep -n '1C\|22\|25\|26'`); `decomp/GIRO.md` (sezione «Lotto 54» breve, e
correggere le frasi a r. ~134 (elenco delle routine mancanti con 352E), ~412-415
(12AB «non ha ancora effetto»), ~419 (seme 4 «FERMO routine 352E»), «Restano non
tradotti: 1C 22 25 26 (lotto 54), 15 16 24 27 (lotto 55)»); `decomp/ROUTINE.md` r. 50
(riga di 0x1288); `STATO.md` (spunta del lotto 54, riga del lotto 55: 15 16 24 27, e
il registro R ancora non modellato).

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master` salvo dove scritto. Uscita esatta salvo
«attesa, dedotta» (non misurata: può differire).

1. Compilazione: `cd decomp/harness && make harness replay gioco_libero gioca sched_proto >
   /tmp/m54.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/m54.txt` → `make 0` e `0`.
2. Catture: `cd decomp && sh cattura_corpi.sh | tail -1` → `CATTURE OK 93 su 93`
   (32 + 61; le 61 sono provate una per una dall'analista salvo la sola riga `f1f`/`f1e`
   provata a parte, stesso esito).
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h54.txt; echo "exit $?"; grep -c FALLITO
   /tmp/h54.txt; grep -c '^RIEPILOGO' /tmp/h54.txt` → `exit 0`, `0`, `327` (322 + 5 voci;
   attesa, dedotta). `grep -c '^cattura'` e `grep -c '^OK'` uguali fra loro, 1371 (1310 + 61;
   attesa, dedotta).
4. Una riga per voce: `for a in 12D6 2CCD 3B63 3B77 3F40 1288; do grep "^RIEPILOGO 0x$a:"
   /tmp/h54.txt; done` → `RIEPILOGO 0x12D6: 23 catture, 23 OK`, `0x2CCD: 5 catture, 5 OK`,
   `0x3B63: 6 catture, 6 OK`, `0x3B77: 9 catture, 9 OK`, `0x3F40: 7 catture, 7 OK`,
   `0x1288: 13 catture, 13 OK` (2 vecchie + 11; attese, dedotte).
5. Registri non confrontati solo per i corpi: `grep -c 'registri non confrontati'
   /tmp/h54.txt` → `80` (30 + 7 + 6 + 9 + 23 + 5; attesa, dedotta); `RIEPILOGO 0x4302` ancora
   `3 catture, 3 OK`.
6. Invarianti, esattamente come nelle «Misure di oggi»: `sh decomp/giro_completo.sh | tail -1` →
   `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese),
   13 frame resi con hash coincidenti`; le tre righe-gruppo di `sched_proto`, `replay` e
   `gioca --oracolo` (sei righe, `AUDIO hash c4b40fea06623a96`).
7. Raccolta con effetto, partita 1A (da `decomp/harness`): `./gioca --riproduci
   ../ricette/partita_1a.reg -n 4000 --muto 2> /tmp/e54.txt | tail -4; grep -c 12AB /tmp/e54.txt;
   grep -a -c FERMO /tmp/e54.txt` → `FRAME rese 4000`, `0` righe «12AB», `0` FERMO (anche
   sullo standard output: ripetere con `| grep -a FERMO` vuoto). **Gli hash cambiano per
   costruzione** (oggi `3f25ba053eeb56b9` video e `2b2b3e7c4ca136fc` audio): la classe 07 nella
   cella C168 ora azzera la cella, suona 0x11 e posta il corpo 18. Riportare nel rapporto i
   nuovi `HASH ultimo` e `AUDIO hash` (attesi diversi dai vecchi; non misurati) e ripetere la
   riproduzione due volte: stessi valori (determinismo). Se compare un FERMO nuovo (un altro
   corpo/routine) riportarlo con il frame, non nasconderlo (domanda 2).
8. Semi 4 e 67: per `S in 4 67`: `python3 decomp/ricette/genera_ingressi.py $S /tmp/s$S.txt
   6000 8; decomp/harness/gioca -n 6000 --muto --riproduci /tmp/s$S.txt 2>&1 | grep -a
   'FERMO\|FRAME rese'` → nessuna riga con `352E`. Può comparire un'altra sosta (per esempio
   il registro R del corpo 12, ancora non modellato, o un altro corpo del lotto 55): riportare
   messaggio e frame; i vecchi erano 5602 e 4866.
9. Regressione di un seme già buono: seme 10 (6000 frame, passo 8) → `FRAME rese 6000` come
   dopo il lotto 52, nessun FERMO (attesa, non misurata oggi per questo seme; se differisce,
   riportare).
10. Dimensioni: `find decomp/harness -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 &&
    $2!="totale"'` → vuota; nessun modulo nuovo oltre 650 righe (controllare a mano).
11. Ambito: `git status --short` mostra solo file di `decomp/` (sorgenti, header, documenti,
    elenco, script, `out/snap_*` nuovi), `compiti/54-…` e `STATO.md`; `pgrep -c mame` → `0`;
    nessun `.o` né binario in `git status`.

## Consegna

Commit unico (o due: banco e corpi, poi documenti) con messaggio in italiano «Lotto 54:
corpi 1C 22 25 26, routine 352E e raccolta 12AB» e riga `Co-Authored-By: <modello>
<noreply@anthropic.com>`. Includere le catture `decomp/out/snap_*` nuove (non ignorate
da git). Aggiungere in fondo a questo file la sezione «Esito» con le uscite di 1-11, i
nuovi hash della partita 1A e le deviazioni. Rapporto finale: massimo trenta righe.

## Domande aperte

1. **Perimetro: un lotto solo per tutto** (4 corpi, 3 rami di 34FA, 12AB, 20 gestori;
   ~800 istruzioni, ~1100 righe C in sei moduli: il lotto più grande finora). Proposta:
   tutto insieme, nessun rinvio al lotto 55. Ordine di priorità se serve spezzare: 1°
   rami 352E/353B/351E, 2° 12AB + corpo 22 + gestori, 3° corpi 26, 1C, 25. Rischio di
   sbagliare: se il lavoro non entra in un ciclo, si chiude il lotto sulla parte
   verificata e il resto passa al 55. **Scelta di progetto** (perimetro).
2. **Gli hash della partita 1A cambiano** (verifica 7): l'oggi `3f25ba053eeb56b9` /
   `2b2b3e7c4ca136fc` non può restare perché la raccolta della classe 07 ora ha effetto.
   Proposta: i nuovi valori diventano il riferimento ufficiale della partita (annotati
   in GIRO.md e nel rapporto), con la prova indipendente del banco (11 catture di `0x1288`).
   Rischio: in gioco libero non c'è un oracolo MAME per i frame; se una via di 12AB è
   sbagliata e il banco non la copre (classi 5-7 e 8+ coperte, 1/3/4/0 coperte) la
   partita lo nasconderebbe. **Scelta di progetto** (cambia il riferimento di regressione).
3. **Passi non catturabili** (1C idx 4 e 8, 25 idx 7, 26 idx 6: tutti con 75AA): proposta
   di tradurli dal disassemblato e verificarli per composizione (le routine chiamate sono
   nel banco; 75AA azzera e basta), come il passo 7 del 13 al lotto 52. Il rischio maggiore
   è 3BAC (132 istr., senza prova diretta); mitigazione: rileggere la traduzione riga per
   riga contro il disassemblato e riportare nel rapporto la lista delle righe dubbie.
   Alternativa scartata: estendere `snap.lua` con un'uscita anticipata (tocca lo strumento
   comune). **Scelta di progetto** (copertura della verifica).
4. **Classi 1C-28 nella tabella di `rst 38` a 130A** (letture oltre la fine della
   tabella, in realtà codice): `nemici_fuori_tabella(0x1309, indice)` e basta. **Preferenza**.
5. **Modo `stato` in `cattura_corpi.sh`** per le catture di `0x1288` nello stato `gioco`
   (non esiste un `att` per la raccolta). **Preferenza**.
6. **Nomi** dei moduli (`attivita_1c.c`, `attivita_22.c`, `attivita_25.c`, `attivita_26.c`,
   `raccolta_13xx.c`), delle funzioni (`raccolta_12ab`, `gestore_XXXX`) e dei tag delle
   catture (`kK`, `cCC`, `tre`, `f1b…f1f`, `rCC`, `rQ08`). **Preferenza**.

## Risoluzioni delle domande aperte (chat principale sotto mandato utente, 2026-11-04)

1. UN LOTTO SOLO per tutto il perimetro (~800 istruzioni): APPROVATO.
   Se lo sviluppatore si incaglia, spezza secondo l'ordine di priorità del
   brief e lo segnala nell'esito.
2. NUOVI HASH della partita 1A come riferimento ufficiale: APPROVATO —
   la raccolta che prende effetto li cambia per forza. Misurarli DUE volte
   per il determinismo e annotarli in GIRO.md e qui nell'esito.
3. Passi che chiamano 75AA tradotti dal disassemblato e verificati per
   composizione: APPROVATO (stesso criterio del passo 7 del corpo 13 e del
   passo 0 del corpo 27).
4. Classi 1C-28 oltre la fine della tabella rst 38: preferenza ACCETTATA
   con la proposta.
5. Modo «stato» in cattura_corpi.sh per le catture di 0x1288: preferenza
   ACCETTATA.
6. Nomi di moduli, funzioni e tag: preferenze ACCETTATE.

## Esito (sviluppatore, Opus 5.5, 2026-11-04)

Lotto unico, nessun rinvio al 55. Uscite delle verifiche:
1. `make 0`, `0` warning/errori.
2. `CATTURE OK 93 su 93` (prima le 61 nuove da sole: 61 su 61). I `.log`
   delle 32 catture vecchie, rigenerati, ripristinati con `git checkout`.
3. `exit 0`, `0` FALLITO, `327` RIEPILOGO; `cattura` 1371, `OK` 1371.
4. `0x12D6` 23/23, `0x2CCD` 5/5, `0x3B63` 6/6, `0x3B77` 9/9, `0x3F40` 7/7,
   `0x1288` 13/13 (tutte come dedotto).
5. `80` «registri non confrontati»; `RIEPILOGO 0x4302: 3 catture, 3 OK`.
6. `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a
   metà attività (attese), 13 frame resi con hash coincidenti`; sched_proto,
   replay e `gioca --oracolo` identici alle «Misure di oggi» (`AUDIO hash
   c4b40fea06623a96`).
7. Partita 1A, due misure identiche: `FRAME rese 4000`, `HASH ultimo
   3f25ba053eeb56b9` (uguale a prima), `AUDIO campioni 2988510`, `AUDIO hash
   c097308a8678392e` (prima `2b2b3e7c4ca136fc`); 0 righe 12AB, 0 FERMO su
   stderr e stdout. Con `--hash-frame` ogni 100 frame, confrontato con il
   binario di HEAD: i frame 1900-2600 diversi, gli altri uguali (la partita
   riconverge). Nuovo riferimento annotato in `decomp/GIRO.md`.
8. Semi 4 e 67: `FRAME rese 6000`, nessun FERMO (nessuna nuova sosta).
9. Seme 10: `FRAME rese 6000`, nessun FERMO.
10. Nessun file sopra 800 righe; moduli nuovi: 1c 205, 22 82, 25 265, 26
    121, raccolta_13xx 219; z80_1xxx 181, attract07a 259, attivita.c 392.
11. `git status`: solo `decomp/`, `STATO.md` e questo file, 244 file
    `out/snap_*` nuovi, niente binari; `pgrep -c mame` → `0`.

Deviazioni dal brief:
- I gestori 139C, 13AB, 13B9, 1413, 14D4 e la coda 13FC/14FC esistevano già
  in `nemici_comparsa.c` (con voci nel banco): riusati, non riscritti;
  `raccolta_13xx.c` contiene solo gli 11 mancanti (135B 13C3 13CD 13D1 13D5
  13E6 13F7 1424 1436 1491 14AA).
- Il passo 3 del 1C (405F) non è di 6 istruzioni senza chiamate: dopo
  `ret z` prosegue (4067-40A6) con 3097 e 2425 (già tradotte); tradotto
  per intero, verificato dalla cattura k3.
- La verifica 6 va lanciata da `decomp/` (`sh giro_completo.sh`): dalla
  radice lo script non trova l'oracolo (lo dice la sua intestazione).
- La voce `0x3B63`/`0x3B77` di `routine_tab.c` sta prima di `0x3F10`, che
  era già fuori ordine.
- Righe dubbie del passo 3BAC (25, non catturabile): nessuna individuata
  alla rilettura; da notare che con c237 = 0 il ciclo 3D20 fa 256 giri,
  come sul Z80.
