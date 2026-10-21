# Dati delle stanze e tabelle del Z80 principale (set `solomon`)

Data: 2026-10-19. Indirizzi nello spazio del Z80 principale; f000-ffff sono
i primi 4096 byte di `8.3jk` (in `out/main64.bin` agli stessi indirizzi).
Indice di stanza `N` = `c243`, 0-based (0-51). Strumenti: `stanze.py`
(decodificatore), `rst20.py`, `e603.lua`, `dump_stanza.lua`, `harness/`.

## Strutture

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
| e603 | 015E, 4CED, 62BE | protezione, valore secondo il PC; tabella in `harness/e603.h` | solo 015E osservato (valore 00) |

### Record 8EA4 (testa da 10 byte)

| byte | effetto (358F) |
|---|---|
| 0 | posizione della porta: cella = 0x01 (tile C4, colore 70); salvata in c247 |
| 1, 2 | posizioni con cella = 0x04 (tile C0, colore 00), coordinate in c45f e c461 |
| 3 (D), 4 (E) | se c248 ≠ D: c248 = E, c249 = D, cella E = (cella & 0x80) + c24a + 5 |
| 5-8 | quattro valori, uno scelto con `(e605 >> 2) & 3` → c08f |
| 9 | valore BCD per c090-c095 (tempo) |
| coppie | `pos, h`: cella pos = h; con c24e bit 0 già a 1 (stanza già visitata) e `(h&0x3f) ≥ 0x32` si scrive `(h&0xc0)+0x29`; `(h&0x3f) = 0x36` suona l'effetto 0x23 |

Valori di cella: 0x37 vuota; 0x40-0x76 non disegnati (oggetti nascosti);
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
  stanza finale; la catena 3DC9 (posta 14), 3DCE (posta 25 → 3B77), 3E47
  (posta 26 → 3B63) chiude il gioco.
