# Dati delle stanze e tabelle del Z80 principale (set `solomon`)

Data: 2026-10-19. Indirizzi nello spazio del Z80 principale; f000-ffff sono
i primi 4096 byte di `8.3jk` (in `out/main64.bin` agli stessi indirizzi).
Indice di stanza `N` = `c243`, 0-based (0-51). Strumenti: `stanze.py`
(decodificatore), `rst20.py`, `e603.lua`, `dump_stanza.lua`, `harness/`.

## Strutture

Le tabelle lette a runtime sono in `tecmo/` come sorgente C (`rom_dati_*.c`,
una tabella per voce), generate da `decomp/estrai_rom.py` (brief 62); elenco ed
estensioni delle voci in `CENSIMENTO-ROM.md`.

| indirizzo | chi la legge | formato | limiti noti |
|---|---|---|---|
| F000 | 33BA | 52 parole, una per stanza → record palette: testa `h`, poi `2*(h&0x7f)` byte copiati a c9ca (c9ca-ca49 azzerati prima da 33AA); la NMI copia c9ca → e500 (palette BG, voci 128-255) quando c044 bit 6 = 1 | il passo con `rld` dopo la copia dipende dal bit 0 di C (`rrc c`), non dal bit 7 di `h`; al frame 2000 c9ca contiene la copia semplice (51 stanze su 51 dump) |
| F068 | 3231 | 52 voci da 4 byte: parola 1 = flusso base (decodificato da 326C con BC=1000), parola 2 = H:L, indici 1-based di due sovrapposizioni (0 = assente; prima H poi L) | — |
| F138 | 325E | voci da 4 byte, indice `i-1`: parola = flusso, poi C, B (parametro di 326C) | indici osservati fino a 0x2C |
| 326C | 3241, 3268 | decodificatore del fondale, vedi sotto | tradotto in `harness/fondale.c`, verificato su due catture |
| 1557 | 37D7 → 3812 | 48 byte per stanza: due mappe di bit da 24 byte (12 righe × 16 celle, bit 7 per primo). La prima scrive 0xB7 (blocco marrone, tile 10 colore 20), la seconda 0xF8 (blocco bianco, tile 14 colore 50) nella griglia a c0cd. La cella 0 di ogni riga è il bordo (sempre 1 nella seconda mappa) | — |
| c0bd | 37D7, 358F, 3829 | griglia FG in RAM: 0xe0 byte, righe da 16; c0bd-c0cc e c18d-c19c = FF (bordo), celle vuote = 0x37; cella (r, k) = c0cd + 16r + k, k = 1..15 → tile riga 4+2r, colonna 2k-1 | — |
| 8EA4 | 358F (letture 359D-3651) | 52 parole → record a lunghezza variabile: 10 byte di testa, poi coppie `posizione, valore` fino a FF (posizioni relative a c0bd) | vedi sotto |
| 8578 | 3A2E | coppie (codice, colore) per i valori di cella 0-7 e per i blocchi (indice 8 per 0x40-0xBF, 9 per 0xC0-0xF7, 10 per ≥ 0xF8) | — |
| 858E | 3A2E | coppie (codice, colore) per i valori 0x08-0x3F (indice v-8) | — |
| rst $20 | gestore 0020 | `jp tabella[A]` con tabella inline dopo il byte E7; elenco in `out/rst20.txt` e `harness/rst20_tables.h` | vedi sotto |
| e603 | 015E, 4CED, 62BE | protezione, valore secondo il PC; tabella in `harness/e603.h` | 015E osservato (valore 00); 4CED modellato come `C & 08` (lotto 59); 62BE non misurato |

### Record 8EA4 (testa da 10 byte)

| byte | effetto (358F) |
|---|---|
| 0 | posizione della porta: cella = 0x01 (tile C4, colore 70); salvata in c247 |
| 1, 2 | posizioni con cella = 0x04 (tile C0, colore 00), coordinate in c45f e c461 |
| 3 (D), 4 (E) | se c248 ≠ D: c248 = E, c249 = D, cella E = (cella & 0x80) + c24a + 5 |
| 5-8 | quattro valori, uno scelto con `(e605 >> 2) & 3` → c08f |
| 9 | valore BCD per c090-c095 (tempo) |
| coppie | `pos, h`: cella pos = h; con c24e bit 0 già a 1 (stanza già visitata) e `(h&0x3f) ≥ 0x32` si scrive `(h&0xc0)+0x29`; `(h&0x3f) = 0x36` suona l'effetto 0x23 |

Valori di cella: 0x37 vuota; 0x40-0x76 non disegnati (oggetti nascosti,
vedi «Oggetti nascosti (special)» in fondo);
gli altri disegnati da 3829 come blocco 2x2 (codici c, c+1 / c+2, c+3, un
solo colore) con la tabella 8578 o 858E. c24a e c248 sono stato di gioco,
non ROM: `stanze.py` legge c24a dal dump (2 se manca) e suppone c248 ≠ D.

### Grammatica di 326C (flusso del fondale)

Ingresso DE = flusso, BC = parametro: B bit 0-4 larghezza (+1), bit 6
partenza a destra, bit 7 righe verso l'alto; C bit 4-7 riga, bit 0-3 colonna.
Variabili c082 riga, c083 colonna, c084 colonna d'inizio (valore << 1 |
verso), c085 contatore, c086 ricarica, c087 attributi (nibble scambiati),
c088 ribaltamenti per un solo blocco, c089 ripetizione. Byte del flusso,
tipo nei bit 0-1:
- 0: blocco 2x2 di codice base `c & 0xfc` (codici base+0..3, scambiati dai
  ribaltamenti) a riga 4+2D, colonna 1+2E di dc00/d800; il cursore avanza e
  va a capo quando c085 arriva a 0 (le righe dopo la prima hanno c086-1 blocchi);
- 1: attributi (333C; con bit 7 a 1 la variante 23CB);
- 2: inizio ripetizione (335A): c089 = bit 4-7 giri, bit 2-3 lunghezza del gruppo;
- 3: fine del flusso.
Fuori dalla griglia 15 × 12 il blocco non si disegna.

## Verifiche

- BG (videoram e colorram, righe 4-27 colonne 1-30) e palette c9ca: 0
  differenze su tutte le 52 stanze contro `out/dump_stanza_N_2000.bin`.
- FG: 0 differenze su 38 stanze; le altre 14:
  - stanza 29: 520 tile diversi, la stanza è piena di tile 0x24 colore 01
    (valore di cella 3) che non vengono né dalle mappe né dal record: li
    scrive un'altra routine, non trovata;
  - stanze 3, 12, 21, 31, 40 (testa `5f 5f 5f 5f 20`) e 50: 12 celle della
    riga 14 con colorram 50 e codice 00, scritte da altro codice;
  - stanze 16, 33, 41, 42, 43, 45, 50, 51: 4-12 tile, effetti di gioco già
    avvenuti al frame 2000 (blocchi creati o rotti, tile 20 colore 11) e
    stabili fino al 2300, quindi non esclusi.
- `ram_room_12.bin` in `roms/decoded/mame/rooms/` non è la stanza 11: c243 =
  0 e piani BG vuoti; `out/dump_stanza_11_2000.bin` rigenerato è
  decodificato con 0 differenze BG.
- Celle FG escluse (cambiano fra i frame 2000 e 2300): 0 per le stanze 0 e 1,
  fino a 356 (stanza 32) dove il gioco va avanti senza input.
  Il frame 1700 indicato nel brief non serve: i blocchi FG compaiono fra il
  frame 1850 e il 1900 (misurato sulla stanza 0), al 1700 il FG è vuoto.

## rst $20

25 siti statici in 0000-bfff (`out/rst20.txt`: `sito voci stato
indice_max voci...`). Euristica: parole in 0000-bfff fino alla più piccola
destinazione maggiore del sito o a un ingresso noto di un'altra tabella,
poi il limite dalle istruzioni prima del salto (`cp n; jr nc` → n, `and m`
→ m+1, ogni `dec a` → -1; si segue il salto che porta all'etichetta, come per
3335). Eseguiti nella traccia a 1900 frame: 2CD2 (26), 2DB8 (6), 2DD9 (7),
3335 (3), 3B10 (40), 75EB (12), 7CEC (8), nessuna destinazione fuori tabella.
3B1D cade dentro la tabella di 3B10; 3B7C è un sito vero (3B77 è la voce 37
di 3B10) e la tabella di 3B68 finisce a 3B76. 8C93, A497, A4A8 sono in zona dati.

## e603

`e603.lua` (5500 frame, input casuali di trace.lua): una sola chiave,
PC 015E valore 00, 5435 letture. 4CED e 62BE non letti. `leggi_e603()` in
`harness/porte.c` restituisce 0 con un avviso per PC non in tabella.

Lotto 59 (brief 59): a 4CED (`sub_4c89`, `attivita_piccoli.c`) la lettura
non passa più da `leggi_e603()`: è modellata come `C & 08` (C = c1b9),
cioè il valore che il gestore di MAME restituisce (`BC & 08`, documenti
`compiti/12-filone-porte.md` e `compiti/03-*.md`; tap a 4CF0 in
`out/att_25_e603.txt`: 00, 00, 08, coerente). Il confronto `xor c; and 08`
passa quindi sempre e la deviazione a 4D00 (`sub_75aa`) non si prende mai,
come in MAME. Dedotto dai documenti, non dal sorgente di MAME. 62BE
(`attivita_27.c`) resta non misurato e passa ancora da `leggi_e603()`.

## Avanzamento di stanza e stanze nascoste

Brief 16 (2026-10-21), da disassemblato e dalle sessioni `porta`, `bonus_04`,
`bonus_42`, `finale`, `finale2` di `sessioni.sh` (blocco `lotto12`).

### Tabella 8DD4

52 record da 4 byte, uno per stanza `c243`, copiati da **34FA** (passo 1
dell'attività 07, a ogni carico di quadro) in c24a-c24d se il bit 0 di c24e
è 0:

| byte | RAM | significato |
|---|---|---|
| 0 | c24a | indice (0-2) della candidata scelta fra le tre seguenti |
| 1-3 | c24b-c24d | stanze candidate; FF = assente; bit 7 = stanza nascosta (bonus) |

Dopo la copia 34FA scorre la lista delle stanze visitate (c251…, chiusa da
FF) e mette FF al posto delle candidate già viste; se la voce scelta è FF
prende la prima non-FF. Il percorso normale è lineare (voce scelta =
c243+1) tranne dove salta le nascoste: 4 → 6, 14 → 16, 21 → 23, 24 → 26,
34 → 36, 41 → 44 (le stanze 42 e 43 si raggiungono solo dalla nascosta 42,
il cui record sceglie già 0xAB). Candidate nascoste:

| record (stanza di partenza) | voce | byte | stanza nascosta |
|---|---|---|---|
| 4 | 0 | 0x85 | 5 |
| 14 | 0 | 0x8F | 15 |
| 21 | 0 | 0x96 | 22 |
| 24 | 0 | 0x99 | 25 |
| 34 | 0 | 0xA3 | 35 |
| 41 | 0 | 0xAA | 42 |
| 42 | 2 (voce scelta di default) | 0xAB | 43 |

Finale: record 49 = `02 ff ff 32`, candidata 0x32 (non una stanza giocabile
del percorso: innesca l'attività 24). Record 50 = `02 00 ff ff`, 51 =
`01 01 00 ff`.

### Stato in RAM

| indirizzo | ruolo |
|---|---|
| c243 | stanza corrente 0-51 (scrittori 404B avanzamento, 7630 demo, 78FE/7CD0 = 0, 7A11/7B46 = 0x32 schermate attract) |
| c24a | indice della candidata; 4A87 lo ruota sulle voci non-FF di c24b-c24d per gli oggetti 8-0F (instradamento alle nascoste) |
| c24b-c24d | le tre candidate della stanza corrente |
| c24e | bit 0 = stanza già visitata (34FA non ricopia il record; 358F altera le celle), bit 1 = entrati in una nascosta, bit 7 = finale |
| c24f | contatore delle stanze nascoste raggiunte (`inc` a 403D) |
| c246 | contatore delle stanze normali (`inc` nel ramo senza bit 7) |
| c251… | lista delle stanze visitate, chiusa da FF (4000 vi accoda c243) |
| c05b… | coda degli oggetti speciali, slot da 2 byte (valore di cella, parametro E), scritta da 12BD-12CF e letta dall'attività 22 (12D6) |
| c19d, c19e | passo e sotto-passo condivisi dagli spedidori a passi (1C, 24, 07): vanno azzerati prima di postare 1C o 24 a mano |

### 4000-405E (uscita dalla porta)

Posta 14; se `c045 & 6 == 0` posta 17 ed esce. Altrimenti E = M(c24b +
c24a); accoda c243 alla lista c251; se E = 0x32 → C = 0x24 e `set 7,(c24e)`;
se bit 7 di E → `set 1,(c24e)` e `inc (c24f)` (403D), altrimenti
`inc (c246)`; `c243 = E & 0x7f`; posta C (07 = carico quadro normale, 24 =
finale); azzera c299 e c19d; c248 = FF.

### Attività 1C (porta) e 24 (finale)

- **1C**, ingresso 3F40: spedidore a passi su c19d, tabella inline a 3F46
  (9 voci): 40E1 41A8 40A7 405F 3F58 41A8 3F93 41B2 3FF7; 3FF7 prosegue in
  4000. Posta statica unica a 133A (porta aperta, valore di cella 2, da 12AB
  via 1332). L'attività 1D (3F39) chiama 349F e diventa codice 05.
- **24**, ingresso 2C72: spedidore a passi su c19d, tabella inline a 2C78
  (sito rst 2C77, 19 voci): 3861 2DEE 37D7 358F 3225 3A7D 2E89 2C9E 2E5D
  2DCE 2E9C 3709 3829 321E 31C5 2EF5 2E6D 2F1A 2E1E, cioè i passi del carico
  quadro (sito 2CD2) più 2C9E. Nella sessione `finale` la 24 compare al frame
  2578 (postata da 4000 dopo la 1C forzata) e la partita prosegue nella
  stanza finale (16 al 5269, di nuovo 1C al 7788). La catena 3DC9 (posta
  14), 3DCE (posta 25 → 3B77), 3E47 (posta 26 → 3B63) non compare nelle
  finestre di `finale`/`finale2`: il blocco 3D20-3E71 è coperto solo dalle
  corse dedicate `att_25` e `att_26` (vedi `CENSIMENTO-12.md`).

## Oggetti nascosti (special)

Compito 60 (2026-11-05). Mappe di tutte le stanze in `docs/special/`
(`stanza_NN.png`, NN = c243), generate con
`python3 mappe_special.py` da `decomp/` (palette dai dump
`out/dump_stanza_N_2300.bin`; `--nei-blocchi` aggiunge gli oggetti dentro i
blocchi in `docs/special/nei_blocchi/`, `--confronta-mame` confronta i tile
con MAME, `--rendi-dump` rende un dump intero col renderizzatore python
`render_py.py`, identico a `harness/video_rendi.c` pixel per pixel).

- **Regola**: una cella di valore 0x40-0x76 è un oggetto nascosto di classe
  `v & 0x3F`: vuota e attraversabile (`sub_5c78` dà classe di collisione 0
  per 0x37-0x7F), non disegnata da 3829.
- **4A02-4A0A** (creazione del mattone, `nemici_comuni.c`): se `v >= 0x40`
  fa `v -= 0x40`, poi `v |= 0x80`; la classe resta nei bit 5-0 del blocco.
- **4938** (rottura del blocco): con A in 80-F7 la cella diventa `v & 0x3F` e
  si ridisegna con la tabella di 3A2E (`stanze.tile_fg`): blocco 2x2 (c, c+1 /
  c+2, c+3) di un solo colore, come gli oggetti visibili. Anche 4ADA usa 3A2E.
- I valori `0x80|classe` / `0xC0|classe` sono oggetti già dentro un blocco
  (87 celle in tutto, p.es. la giara rossa 0x9A della stanza 1, cella 118):
  non sono «nascosti», si vedono solo rompendo il blocco.
- Con la stanza già visitata (bit 0 di c24e) una classe ≥ 0x32 diventa 0x29;
  le mappe mostrano la prima visita.

Tabella delle classi. Il nome inglese è quello delle immagini (pubbliche);
nessun testo del gioco dà nomi, sono descrittivi dal tile. Punti dalla
tabella 0x150C (parole da 3 byte: 100, 200, 500, 1000, 2000, 5000, 10000,
20000, 50000, 100000, 200000, 500000, 1000000) indicizzata da `A − 0x29`
dopo il gestore (`sub_13fc`); gestori dalla tabella 130A di `sub_12f0`
(voce = classe − 8).

| classe | tile (descrizione) | nome nelle immagini | effetto alla raccolta | nascoste |
|---|---|---|---|---|
| 00 | — | — | niente | 0 |
| 01 | porta | door | — | 0 |
| 02 | porta/muro rosso | — | 12AB: posta il corpo 1C | 0 |
| 03-04 | specchi di partenza | — | — | 0 |
| 05-07 | chiavi | key | 12AB (1340): suono 11, corpo 18 | 0 |
| 08 | diamante blu | blue diamond | 13CD: 500 punti | 3 |
| 09 | ampolla blu piccola | small blue jar | 13D5: 200 punti | 0 |
| 0A | moneta gialla | yellow coin | 13D1: 2000 punti | 0 |
| 0B | sfera rossa | red orb | 13B9: 200 punti | 0 |
| 0C | gemma | gem | 13D1: 2000 punti | 0 |
| 0D | ampolla blu | blue jar | 13E6: 500 punti | 0 |
| 0E | pergamena | scroll | 1413 | 0 |
| 0F | campana | bell | 139C: 200 punti | 0 |
| 10 | ampolla verde | green potion A | 1424: raddoppia BCD c093-c098, 100 punti | 1 |
| 11 | ampolla verde | green potion B | 1436: ×5 BCD c093-c098, 500 punti | 11 |
| 12 | clessidra rossa | red hourglass | 14AA: byte del quadro ×5 in c094-c095, 200 punti | 5 |
| 13 | clessidra blu | blue hourglass | 1491: byte del quadro ×16 in c094-c095, 500 punti | 5 |
| 14 | sfera verde | green orb | 13C3: 134B con DE=4, 100 punti | 20 |
| 15 | ampolla blu | blue jar A | 13D5: coppia libera 55, 200 punti | 2 |
| 16 | ampolla blu | blue jar B | 13E6: coppia libera AA, 500 punti | 1 |
| 17 | pergamena | scroll | 1413: c259 +1 fino a 9, ridisegna tabellone, 200 punti | 3 |
| 18 | campana arancio | orange bell | 139C: c1ad=0B, posta attività 19, 200 punti | 4 |
| 19 | campana gialla | yellow bell | 13AB: c1ad=0C, 500 punti | 7 |
| 1A | giara rossa | red jar (fire bottle) | 135B: scorre 17 oggetti da c4fe, bit 1 se campo +4−40 in 00-0F o 28-37, suono 33 e attività 12; 500 punti | 3 (stanze 14, 23, 37) |
| 1B | giara rossa | red jar | 14D4: 200 punti | 0 |
| 1C-28 | giara rossa (stesso tile) | — | nessun gestore (fuori tabella) | 0 |
| 29 | lingotto d'oro | gold bar | 13F7: 100 punti | 1 |
| 2A | diamante giallo | yellow diamond | 13F7: 200 punti | 0 |
| 2B | zaffiro | sapphire | 13F7: 500 punti | 8 |
| 2C | moneta d'oro | gold coin | 13F7: 1000 punti | 4 |
| 2D | monete doppie | coins | 13F7: 2000 punti | 11 |
| 2E | rubino/lanterna | ruby | 13F7: 5000 punti | 6 |
| 2F | moneta stellata | star coin A | 13F7: 10000 punti | 12 |
| 30 | moneta stellata | star coin B | 13F7: 20000 punti | 0 |
| 31 | gemma verde | green gem | 13F7: 50000 punti | 6 |
| 32 | gru/ornamento giallo | yellow ornament | 13F7: 100000 punti | 7 |
| 33 | sigillo (stella di Davide) | seal | 13F7: 200000 punti | 4 |
| 34 | ala/X dorata | winged ornament | 13F7: 500000 punti | 1 |
| 35 | muso di tigre | tiger | 13F7: 1000000 punti | 1 |
| 36 | medaglia/vaso | medal | 13F7 → 14FC: suono 12, c237 +1, barra (31E9); al carico suona l'effetto 0x23; indice 0x0D oltre la tabella dei punti («special» nelle immagini) | 2 |

Le classi 1A e 1B hanno lo stesso tile: nelle immagini la classe è scritta in
esadecimale. Confronto con MAME (`--confronta-mame`): 21 classi nascoste su
24 hanno codice e colore uguali al blocco visibile della stessa classe nei
dump (0x31 animata, coincide al frame 2000); 0x16, 0x17 e 0x35 non sono mai
visibili in chiaro e restano senza confronto (coda: una prova di rivelazione
reale in MAME, solo se una mappa mostra un oggetto sospetto).

Conteggi per stanza (128 celle nascoste, tutte nella griglia 15x12, 24
classi): `0:1 1:3 2:1 3:0 4:1 5:3 6:8 7:4 8:3 9:11 10:2 11:2 12:0 13:0 14:3
15:3 16:0 17:19 18:1 19:3 20:3 21:0 22:1 23:4 24:6 25:3 26:2 27:2 28:4 29:2
30:3 31:0 32:2 33:1 34:0 35:1 36:0 37:3 38:2 39:1 40:0 41:2 42:1 43:2 44:5
45:0 46:2 47:2 48:3 49:3 50:0 51:0`.

Cella E della testa: con `c248 ≠ D` la cella E diventa
`(cella & 0x80) + c24a + 5`, cioè la chiave di classe 05, 06 o 07 secondo
c24a; c24a iniziale = `main64.bin[0x8DD4 + 4N]` (1 o 2 in tutte le stanze;
la stanza 50 nel dump `_2300` ha 0). Nelle 52 stanze la cella E è visibile;
le immagini la contornano in arancio. I byte 5-8 della testa non sono
oggetti: sono i quattro passi del contatore del tempo (c08f, letto da
`sub_026c`: `c090-c091 += c08f`), scelti con `(e605 >> 2) & 3`; le immagini
li elencano solo in legenda.
