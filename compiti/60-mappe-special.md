# Compito 60: mappe delle stanze con gli oggetti nascosti («special»)

2026-11-05, scritto dall'agente analista per un agente sviluppatore. Questo
file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Generare un PNG per ognuna delle 52 stanze (c243 = 0-51, comprese le sette
nascoste/bonus e il finale) che mostri la stanza come in gioco (fondale BG +
blocchi FG) con gli oggetti nascosti già disegnati nella loro cella, con la
grafica vera del gioco, evidenziati; con evidenza diversa la chiave della cella
E del record (variabile con c24a). Script python rieseguibile in `decomp/`,
immagini in `docs/special/`, tabella dei codici in `decomp/DATI.md`.

## Vincoli assoluti

- Niente modifiche fuori da `/home/rufio72/OpenSolomonsKey-master`. Si
  creano/modificano SOLO: `decomp/mappe_special.py`, `decomp/render_py.py`
  (nuovi), `docs/special/*.png` e `docs/special/LEGGIMI.md` (nuovi),
  `decomp/DATI.md` e `STATO.md` (sezione nuova).
- NON si tocca il codice C del gioco (`decomp/harness/*`), il banco, le
  ricette, `decomp/stanze.py` (si importa, non si modifica), né
  `decomp/ricette/riferimento_nuova2.reg` (non tracciato, dell'utente: non
  aggiungerlo al commit).
- MAME solo con `-video none` e solo se i dump mancano (vedi sotto); niente
  finestre; `gioca` non serve. Mai `pkill`/`killall` su `gioca` o `mame`.
  A fine lavoro `pgrep -c mame` deve dare `0`.
- Niente push, niente git distruttivo (stash, reset, checkout di file).
- Commenti, messaggi, documenti in italiano (il `LEGGIMI.md` di `docs/special`
  e le legende nelle immagini in italiano: sono interni al progetto).
- Nessun file di codice sopra le 800 righe. Python sotto `decomp/`: il più
  grande oggi è 687 righe (`censimento_nemici.py`); i due file nuovi stanno
  sotto le 300.
- Python: PIL 10.2.0 e numpy 1.26.4 sono già installati (da `python3 -c`).
  Non installare nulla.

## Cosa esiste già (fatti verificati)

### Dati e strumenti

| cosa | dove | note |
|---|---|---|
| ROM principale decodificata | `decomp/out/main64.bin` (65536 byte) | già presente; `stanze.py` la legge con `--rom` |
| record stanza | `main64.bin` 0x8EA4 + 2·N → puntatore; 10 byte di testa, poi coppie `pos, valore` fino a FF | da `decomp/DATI.md` e `stanze.py` |
| `stanze.griglia_fg(rom, n, c24a=2, c248=None, prima_volta=True)` → `(griglia, testa)` | `decomp/stanze.py:191` | griglia da 0xE0 byte, cella (riga r 0-11, colonna k 0-15) = indice `0x10+16*r+k`; la colonna k=0 è il bordo | da `grafo outline` |
| `stanze.primo_piano(rom, n, c24a)` → `(video, colore, griglia)` | `stanze.py:238` | piani FG da 0x400 byte (32x32), blocchi 2x2 alla riga `4+2r`, colonna `1+2(k-1)`; SALTA le celle 0x37 e 0x40-0x76 |
| `stanze.fondale(rom, n)` → oggetto con `.video`, `.colore` (0x400 byte ciascuno) | `stanze.py:151` | BG; 0 differenze su 52 stanze contro MAME |
| `stanze.palette(rom, n)` | `stanze.py:164` | 128 byte = prima metà di e500; coincide con MAME su 45 stanze su 52 al frame 2300 (misurato) |
| `stanze.tile_fg(rom, v)` → `(codice, colore/attributo)` | `stanze.py:223` | tabelle 8578/858E: vale 0x08-0x3F via 858E, 0-7 via 8578 |
| dump RAM c000-ffff di MAME | `decomp/out/dump_stanza_N_2000.bin` e `_2300.bin`, N = 0-51 (104 file, presenti; cartella ignorata da git) | offset nel file = indirizzo − 0xC000: FG colore d000 → 0x1000, FG video d400 → 0x1400, BG colore d800 → 0x1800, BG video dc00 → 0x1C00, sprite e000 → 0x2000, palette e400-e5ff → 0x2400-0x25FF |
| rigenerazione dump | `ROOM=N DUMP_FRAME=2000,2300 mame solomon ... -video none -autoboot_script dump_stanza.lua` da `decomp/` (intestazione di `dump_stanza.lua`) | solo se un dump manca |
| grafica | ROM in `roms/ext/solomon/`: caratteri FG = `12.3t`+`11.3r` (32768 byte ciascuna), fondale BG = `10.3p`+`9.3m`, sprite = `2.5lm`,`3.6lm`,`4.7lm`,`5.8lm` (16384 ciascuno) | da `decomp/estrai_grafica.py` (elenco in testa) e `video_rendi.c` |

### Formato di rendering (da `decomp/harness/video_rendi.c`, 108 righe: copiare la logica)

- Schermo 256x224, righe di schermo 16-239 visibili (`RIGA_PRIMA` = 16): la
  cella di tile (riga R, colonna C) sta a pixel x = 8C, y = 8R − 16.
- Tile 8x8: `o = codice*32 + y*4 + x//2` nel file concatenato dei due ROM
  (caratteri o fondale); nibble alto se x pari, basso se dispari.
- Palette: 256 voci da 2 byte little-endian in e400-e5ff, canali
  `(w&15)*17`, `((w>>4)&15)*17`, `((w>>8)&15)*17` (R, G, B).
- BG: attributo `a` = colorram; `codice = video + 256*(a&7)`; base colori
  `128 + 16*((a>>4)&7)`; bit 0x80 = specchio in x, 0x08 = specchio in y; il
  pen 0 NON è trasparente.
- FG: `codice = video + 256*(a&7)`; base `16*((a>>4)&7)`; pen 0 trasparente; i
  bit 0x80 e 0x08 sono ignorati.
- Sprite (32 voci da 4 byte a e000, dalla 31 alla 0): `codice = e[0] +
  ((e[1]&0x10)<<4)`, base `16*((e[1]>>1)&7)`, x = `e[3]`, y = `241 − e[2]`,
  flip x = bit 0x40 di `e[1]`, flip y = bit 0x80; tile 16x16 da quattro piani
  (file separati): `o = codice*32 + (16 se y>=8) + (8 se x>=8) + (y&7)`, bit
  `7 − (x&7)` di ciascun piano, piano 0 = bit 3 del pen, piano 3 = bit 0.

### Semantica delle celle (da `stanze.py`, DATI.md, `nemici_comuni.c`)

- Cella = valore di 8 bit: bit 7-6 = tipo di blocco, bit 5-0 = classe
  dell'oggetto. 0x37 = vuota; 0xB7 = blocco marrone vuoto; 0xF8 = blocco
  bianco.
- **0x40-0x76 = oggetto nascosto di classe `v & 0x3F`** (cella vuota e
  attraversabile: `sub_5c78` in `z80_5xxx.c` dà classe di collisione 0 per
  0x37-0x7F; `primo_piano` e 3829 non la disegnano).
- Quando Dana crea un mattone su quella cella (`nemici_comuni.c` ~riga 427,
  routine 4A02-4A0A): se `v >= 0x40` fa `v -= 0x40`, poi `v |= 0x80`: la
  classe resta nei bit 5-0 dentro il blocco.
- Quando il blocco si rompe (`sub_4938`, `nemici_comuni.c`, commento «0x4938»):
  con A in 80-F7 la cella perde i bit 6-7 (`& 0x3F`) e si ridisegna col
  carattere della tabella di `sub_3a2e` (= `stanze.tile_fg`). Quindi il tile
  dell'oggetto rivelato è `tile_fg(rom, classe)`, un blocco 2x2 (codici
  c, c+1 in alto; c+2, c+3 in basso) con un solo colore, come gli oggetti
  visibili. Da ROUTINE.md riga 705 anche `sub_4ada` usa 3A2E.
- Vale anche `0x80|classe`/`0xC0|classe` per i blocchi che contengono già un
  oggetto (da `griglia_fg`): non sono «special nascosti»; vedi domanda 3.
- Se la stanza è già visitata (bit 0 di c24e) una classe ≥ 0x32 diventa 0x29
  (DATI.md riga 33): le immagini mostrano il caso prima visita
  (`prima_volta=True`, predefinito).
- Cella E della testa: byte 3 = D, byte 4 = E. Se `c248 != D` la cella E
  diventa `(cella & 0x80) + c24a + 5` (`griglia_fg` lo fa già), quindi un
  oggetto di classe 5, 6 o 7 (chiavi, tile 05/06/07). `c24a` iniziale =
  byte 0 del record di 8DD4 della stanza (`main64.bin[0x8DD4 + 4N]`,
  misurato: 1 o 2 per tutte le stanze, 0 mai; coincide con `c24a` dei dump
  `_2300` tranne la stanza 50, dove il dump dà 0 e la ROM 2; ricavato da
  una misura). Nei 52 casi la cella E è visibile (valore 06 o 07, nessun
  blocco).
- Byte 5-8 della testa: NON sono oggetti. Sono i quattro passi del contatore
  del tempo (`c08f`, «valore scelto dal record», letto da `sub_026c` in
  `z80_0xxx.c:24-29`: `c090-c091 += c08f`) scelti con `(e605>>2)&3`. Non
  hanno posizione né tile; vanno solo elencati nella legenda/DATI.md
  (vedi domanda 2).

### Tabella dei codici (classi) → oggetto

Tile da `tile_fg` (foglio contatti visto per tutte le classi 00-36 con la
palette della stanza 1); effetto dai gestori di raccolta, tabella 130A di
`sub_12f0` (`harness/attivita_22.c`, voce = classe − 8, solo voci 0-19) e
`raccolta_13xx.c` / `nemici_comparsa.c`. I nomi sono descrittivi dal tile
(nessun testo del gioco li dà): vanno messi in `DATI.md` così come sono
scritti qui. I punteggi vengono dalla tabella ROM 0x150C (parole da 3 byte
little-endian: 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000,
100000, 200000, 500000, 1000000; misurato), indicizzata da `A − 0x29`
dopo il gestore (`sub_13fc`).

| classe | tile | nome | effetto alla raccolta (da gestore) | celle nascoste (misurate, tutte le stanze) |
|---|---|---|---|---|
| 08 | diamante blu | diamante blu | 13CD: 500 punti | 3 |
| 10 | ampolla verde | pozione verde A | 1424: raddoppia BCD c093-c098, 100 punti | 1 |
| 11 | ampolla verde | pozione verde B | 1436: ×5 BCD c093-c098, 500 punti | 11 |
| 12 | clessidra rossa | clessidra rossa | 14AA: byte del quadro ×5 in c094-c095, 200 punti | 5 |
| 13 | clessidra blu | clessidra blu | 1491: byte del quadro ×16 in c094-c095, 500 punti | 5 |
| 14 | sfera verde | sfera verde | 13C3: 134B con DE=4, 100 punti | 20 |
| 15 | ampolla blu | ampolla blu A | 13D5: coppia libera 55, 200 punti | 2 |
| 16 | ampolla blu | ampolla blu B | 13E6: coppia libera AA, 500 punti | 1 |
| 17 | pergamena | pergamena | 1413: c259 +1 fino a 9, ridisegna tabellone, 200 punti | 3 |
| 18 | campana arancio | campana arancio | 139C: c1ad=0B, posta attività 19, 200 punti | 4 |
| 19 | campana gialla | campana gialla | 13AB: c1ad=0C, 500 punti | 7 |
| 1A | giara rossa | giara rossa (bottiglia di fuoco) | 135B: scorre 17 oggetti da c4fe, bit 1 se campo +4−40 in 00-0F o 28-37, suono 33 e attività 12; 500 punti | 3 (stanze 14, 23, 37) |
| 29 | lingotto d'oro | lingotto | 13F7: 100 punti | 1 |
| 2B | zaffiro | zaffiro | 13F7: 500 punti | 8 |
| 2C | moneta d'oro | moneta | 13F7: 1000 punti | 4 |
| 2D | monete doppie | monete | 13F7: 2000 punti | 11 |
| 2E | rubino/lanterna | rubino | 13F7: 5000 punti | 6 |
| 2F | moneta stellata | moneta stellata A | 13F7: 10000 punti | 12 |
| 31 | gemma verde | gemma verde | 13F7: 50000 punti | 6 |
| 32 | gru/ornamento giallo | ornamento giallo | 13F7: 100000 punti | 7 |
| 33 | sigillo (stella di Davide) | sigillo | 13F7: 200000 punti | 4 |
| 34 | ala/X dorata | ornamento alato | 13F7: 500000 punti | 1 |
| 35 | muso di tigre | tigre | 13F7: 1000000 punti | 1 |
| 36 | medaglia/vaso | medaglia | 13F7 → 14FC: suono 12, c237 +1, barra (31E9); DATI.md: suona l'effetto 0x23 al carico | 2 |

Altre classi, per completezza della tabella in `DATI.md` (non nascoste nelle
stanze): 00 niente; 01 porta; 02 porta/muro rosso (12AB: posta il corpo 1C);
03-04 specchi di partenza; 05-07 chiavi; 09 ampolla blu piccola (13D5, 200);
0A moneta gialla (13D1, 2000); 0B sfera rossa (13B9, 200); 0C gemma (13D1,
2000); 0D ampolla blu (13E6, 500); 0E pergamena (1413); 0F campana (139C);
1B giara rossa (14D4, 200); 1C-28 stesso tile di giara rossa senza gestore
(fuori tabella); 2A diamante giallo (13F7, 200); 30 moneta stellata B
(13F7, 20000). Le classi 5-7 sono raccolte da 12AB (1340) con suono 11 e
corpo 18, non da 12F0. Le classi 1A e 1B hanno lo stesso tile: non si
distinguono a vista; l'immagine deve scrivere la classe in esadecimale.

Misure fatte per il brief (da `stanze.griglia_fg` con la ROM, stesso risultato
dal parse indipendente dei record, vedi verifica 5):
- 128 celle nascoste in totale, nessuna fuori dalla griglia 15x12 (tutte
  alla riga 0-11, colonna 1-15), 24 classi diverse: 08, 10-1A, 29, 2B-2F,
  31-36.
- Per stanza (indice:numero): `0:1 1:3 2:1 3:0 4:1 5:3 6:8 7:4 8:3 9:11
  10:2 11:2 12:0 13:0 14:3 15:3 16:0 17:19 18:1 19:3 20:3 21:0 22:1 23:4
  24:6 25:3 26:2 27:2 28:4 29:2 30:3 31:0 32:2 33:1 34:0 35:1 36:0 37:3
  38:2 39:1 40:0 41:2 42:1 43:2 44:5 45:0 46:2 47:2 48:3 49:3 50:0 51:0`
  (12 stanze senza nascosti).
- Nella stanza 1 (livello 2): nascoste le celle 166 (0x6B, zaffiro) e 170
  (0x6B) e 168 (0x48, diamante blu) (indice cella = `0x10+16r+k`, quindi
  riga/colonna da calcolare); la cella 118 contiene 0x9A = giara rossa (1A)
  DENTRO un blocco, non nascosta (vedi domanda 3).
- Dei 24 codici nascosti, 21 compaiono anche visibili in almeno una stanza
  e il loro tile in MAME (dump `_2300`) coincide con `tile_fg`; i tre che non
  si vedono mai in chiaro sono 0x16, 0x17, 0x35 (verifica 4).

## Struttura da realizzare

### `decomp/render_py.py` (nuovo, ~100 righe)

Rendering software identico a `video_rendi.c`, solo con numpy/PIL. Interfaccia
fissa:

- `carica_grafica(cartella_rom)` → `(car, fon, spr)`: byte concatenati come
  in `video.h` (`car` = `12.3t`+`11.3r`; `fon` = `10.3p`+`9.3m`; `spr` = lista
  dei quattro file piano).
- `rendi_frame(fg_colore, fg_video, bg_colore, bg_video, spriteram, palette,
  grafica)` → array numpy `uint8 (224, 256, 3)`; ogni argomento è `bytes` di
  1024, 1024, 1024, 1024, 128, 512 byte. Implementa BG, FG e sprite come
  nella sezione «Formato di rendering» (gli sprite servono alla verifica 3,
  non alle mappe).
- `ram_da_dump(dump)` → i sei blocchi tagliati secondo la tabella degli
  offset dei dump.

### `decomp/mappe_special.py` (nuovo, ~200 righe; usa `stanze` e `render_py`)

Uso (da `decomp/`):
`python3 mappe_special.py [--rom out/main64.bin] [--rom-grafica ../roms/ext/solomon]
[--dump-dir out] [--uscita ../docs/special] [--scala 3] [--nei-blocchi]
[--stanza N] [--rendi-dump FILE.bin --ppm FILE.ppm]`

Per ogni stanza N:
1. `c24a = rom[0x8DD4 + 4N]`; `griglia, testa = stanze.griglia_fg(rom, N,
   c24a)`; `fondale` e `primo_piano` come in `stanze.main`.
2. Palette: i 512 byte e400-e5ff di `out/dump_stanza_N_2300.bin` (nei dump
   la FG ha 9 varianti di e400 e la BG è la copia di c9ca; la palette non è
   derivabile dalla sola ROM senza ripercorrere la NMI). Se il dump manca:
   errore chiaro con il comando di rigenerazione.
3. Rendi BG+FG (nessuno sprite: spriteram a zero) tagliando le colonne di
   tile 1-30 e le righe 4-27, cioè pixel x 8..248, y 16..208 (240x192).
4. Per ogni cella con `0x40 <= v <= 0x76` (colonna 1-15, riga 0-11):
   disegna il blocco 2x2 `tile_fg(rom, v & 0x3F)` nella posizione `(4+2r,
   1+2(k-1))` con la stessa funzione di blocco del FG, poi il contorno
   (rettangolo 16x16 del gioco, spessore 1 pixel del gioco ⇒ `scala` px)
   verde acceso (0,255,0) disegnato DOPO il ridimensionamento a `scala`
   (NEAREST) per restare netto.
5. Cella E (`testa[4]`): se `testa[3]` è D e il caso predefinito `c248 != D`
   (come `griglia_fg`), il tile della cella `g[E]` è già nel primo piano:
   contorno arancio (255,128,0) attorno a quella cella con una nota in
   legenda «cella E: classe c24a+5 = 0X (varia 05/06/07 con c24a)».
6. Legenda sotto l'immagine (testo PIL, font predefinito, sfondo grigio
   scuro): titolo «Stanza N (c243=N)»; una riga per ogni nascosto (classe
   esadecimale, nome, riga/colonna della cella, punti); riga per la cella E;
   riga con i quattro valori byte 5-8 della testa («passo del tempo: a,b,c,d,
   scelto con (e605>>2)&3»). Se non ci sono nascosti: «nessun oggetto
   nascosto».
7. Salva `docs/special/stanza_NN.png`, NN = N a due cifre (00-51); nessun
   metadato con data/ora (PIL non ne scrive per PNG semplice: l'uscita deve
   essere byte-identica fra due esecuzioni).

Stampa (stdout) una riga per stanza, esattamente nel formato
`stanza NN: nascosti K classi 0xAA,0xBB` (classi in ordine di cella,
esadecimale a 2 cifre maiuscolo; `classi -` se K = 0), poi `TOTALE nascosti
128`, `PNG scritte 52`, e una tabella `classe nome conteggio`.

`--nei-blocchi` (opzionale, spento di default): contorno azzurro
(0,200,255) alle celle con `v >= 0x80` e classe `v & 0x3F` in 08-36, con
l'icona dell'oggetto sovrapposta al blocco; scrive in `docs/special/
nei_blocchi/stanza_NN.png`. Se la domanda 3 viene respinta, non si
implementa.

`--rendi-dump FILE.bin --ppm FILE.ppm`: rende l'intero frame 256x224 (con
sprite) di un dump RAM completo e lo scrive come PPM P6 (`P6\n256 224\n255\n`
e 172032 byte); serve alla verifica 3.

### Documenti

- `decomp/DATI.md`: nuova sezione «Oggetti nascosti (special)» con: regola
  0x40-0x76, i passaggi 4A02 (creazione del mattone) e 4938 (rottura), la
  tabella dei codici di questo brief (completa 00-36), l'elenco dei conteggi
  per stanza, la cella E e i byte 5-8 (tempo), il comando dello script. Va
  corretta la riga 35 («non disegnati (oggetti nascosti)») solo
  aggiungendo il rimando alla sezione nuova.
- `docs/special/LEGGIMI.md`: cosa mostrano le immagini, la legenda dei
  colori, il comando per rigenerarle (5-10 righe).
- `STATO.md`: una riga nella sezione adatta con l'esito.

## Verifiche obbligatorie

Da `decomp/` salvo dove indicato. Le uscite «misurate» le ho prese io; quelle
«attese, non misurate» possono differire.

1. Generazione e numero di PNG (esito atteso: 52):
   `python3 mappe_special.py | tail -3 ; ls ../docs/special/stanza_*.png | wc -l`
   → l'ultima riga dello script contiene `PNG scritte 52`, `TOTALE nascosti
   128` compare prima, e `wc -l` stampa `52` (attesa, non misurata). Le
   immagini si guardano con `Read` almeno per le stanze 1, 6, 9, 17 e 49:
   nascosti in verde, tile veri, legenda leggibile.
2. Conteggio per stanza (e) con un parse indipendente dai record, che non usa
   `stanze.py` (misurato; deve coincidere riga per riga con le righe `stanza
   NN: nascosti K` dello script):
   ```
   python3 - <<'E'
   r=open('out/main64.bin','rb').read()
   t=0;o=[]
   for n in range(52):
       p=r[0x8ea4+2*n]|r[0x8ea5+2*n]<<8; q=p+10; c={}
       while r[q]!=0xff: c[r[q]]=r[q+1]; q+=2
       k=sum(1 for v in c.values() if 0x40<=v<=0x76); t+=k; o.append(k)
   print(t); print(' '.join('%d:%d'%(i,k) for i,k in enumerate(o)))
   E
   ```
   uscita misurata: `128` e la riga `0:1 1:3 2:1 3:0 4:1 5:3 6:8 7:4 8:3 9:11
   10:2 11:2 12:0 13:0 14:3 15:3 16:0 17:19 18:1 19:3 20:3 21:0 22:1 23:4
   24:6 25:3 26:2 27:2 28:4 29:2 30:3 31:0 32:2 33:1 34:0 35:1 36:0 37:3
   38:2 39:1 40:0 41:2 42:1 43:2 44:5 45:0 46:2 47:2 48:3 49:3 50:0 51:0`.
   Confronto con l'uscita dello script:
   `python3 mappe_special.py | sed -n 's/^stanza \([0-9]*\): nascosti \([0-9]*\).*/\1 \2/p' | awk '{printf "%d:%d ", $1+0,$2}'`
   deve dare la stessa sequenza (attesa, non misurata la forma esatta).
3. Il renderizzatore python è identico a quello C (H2) su un frame vero di
   MAME (attesa, non misurata): compilare il confrontatore FUORI dalla
   cartella del repo e tagliare il dump:
   ```
   T=$(mktemp -d); cd harness && cc -O1 -o $T/rendi_oracolo rendi_oracolo.c \
     video_rendi.c video_dati_car_?.c video_dati_fon_?.c video_dati_spr_?.c && cd ..
   python3 - "$T" <<'E'
   import sys
   d=open('out/dump_stanza_1_2300.bin','rb').read(); T=sys.argv[1]
   for nome,a,n in (('fg_colorram',0x1000,1024),('fg_videoram',0x1400,1024),
     ('bg_colorram',0x1800,1024),('bg_videoram',0x1c00,1024),
     ('spriteram',0x2000,128),('palette',0x2400,512)):
       open('%s/s1_%s.bin'%(T,nome),'wb').write(d[a:a+n])
   E
   python3 mappe_special.py --rendi-dump out/dump_stanza_1_2300.bin --ppm $T/py.ppm
   $T/rendi_oracolo $T s1 $T/py.ppm
   ```
   uscita attesa: `s1: 57344/57344 pixel uguali`, codice 0. Ripetere con le
   stanze 6 e 17. Se i pixel differiscono, si corregge `render_py.py`.
4. Confronto con MAME (b): tile e colore dell'oggetto disegnato = quelli che
   MAME disegna per la stessa classe. Per ogni classe nascosta che compare
   anche visibile in una stanza, il blocco 2x2 del dump `_2300` (FG video
   d400, FG colore d000, alla cella `(4+2r, 1+2(k−1))`) deve avere codice e
   colore uguali a `tile_fg(rom, classe)`. Lo script espone un'opzione
   `--confronta-mame` (da aggiungere all'elenco dell'uso) che stampa, per
   ognuna delle 24 classi nascoste, `classe 0xCC: MAME ok (stanza S)` oppure
   `classe 0xCC: non visibile in chiaro in nessuna stanza`. Uscita attesa
   (misurata sulla logica, forma esatta libera): 21 classi `ok` e 3 senza
   confronto (0x16, 0x17, 0x35), nessuna `DIVERSA`. Per le classi con
   pixel animati (colore cambia fra i frame 2000 e 2300, come in
   `stanze.confronta`), confrontare con il dump che coincide e dirlo.
   Il confronto con la rivelazione reale di un mattone creato e rotto in
   MAME NON è richiesto: la rivelazione usa 3A2E (`sub_4938`/`sub_4ada`),
   la stessa tabella di `tile_fg`; vedi domanda 1.
5. Idempotenza (c): due esecuzioni producono PNG byte-identici e non si toccano
   gioco né banco:
   ```
   (cd ../docs/special && sha256sum stanza_*.png) > /tmp/h1.txt
   python3 mappe_special.py > /dev/null
   (cd ../docs/special && sha256sum stanza_*.png) | diff - /tmp/h1.txt && echo IDENTICI
   git status --short | grep -v "^?? docs/special/\|^?? decomp/mappe_special.py\|^?? decomp/render_py.py\|^ M decomp/DATI.md\|^ M STATO.md\|^?? decomp/ricette/riferimento_nuova2.reg"
   ```
   attesa: `IDENTICI`; il `git status` filtrato è vuoto (nessun file di
   `decomp/harness`, `compiti`, ricette o `stanze.py` modificato).
6. Rieseguibile senza dump vecchi non è richiesto; ma lo script non deve
   scrivere in `decomp/out` (deve leggere soltanto): `git status --short
   decomp/out` vuoto (la cartella è ignorata) e `find out -newer
   mappe_special.py -type f | wc -l` → `0` (attesa, non misurata).
7. Nessun file di codice sopra le 800 righe:
   `find . -name '*.py' -o -name '*.c' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
   (da `decomp/`) → uscita vuota (misurata sullo stato attuale: vuota).
8. `pgrep -c mame` → `0`.

## Consegna

- Commit (mai push) con i file elencati nei vincoli, messaggio «Mappe delle
  stanze con gli oggetti nascosti (special), brief 60», riga finale
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Rapporto finale in al massimo trenta righe: uscita di ogni verifica,
  deviazioni dal brief, elenco delle classi senza confronto MAME, dimensione
  totale di `docs/special`.

## Domande aperte

1. Prova di rivelazione reale in MAME (mattone creato e rotto con ingressi
   registrati, o Lua che chiama 4ADA con registri forzati): proposta di NON
   farla in questo compito e di accettare come prova il confronto con le
   celle visibili della stessa classe (verifica 4) più il fatto che la
   rivelazione passa per 3A2E; copre 21 classi su 24 (non 0x16, 0x17,
   0x35). Se l'utente la vuole comunque, è un lotto a parte (nuovo Lua e
   registrazione, rischio di fragilità). — scelta di progetto.
2. I byte 5-8 della testa non sono oggetti (sono il passo del contatore del
   tempo): proposta di elencarli solo in legenda e in DATI.md, non di
   disegnarli. — preferenza.
3. Oggetti già dentro blocchi (`0x80|classe`, 87 celle in totale, fra cui la
   giara rossa 0x9A della stanza 1, cella 118): non sono «special» ma
   si rivelano rompendo il mattone. Proposta: non farli nelle immagini
   principali, ma tenere il flag `--nei-blocchi` che li disegna in
   `docs/special/nei_blocchi/`. Se si preferisce un'immagine sola, basta
   attivarlo di default con contorno azzurro. — preferenza.
4. Cartella e nomi: proposta `docs/special/stanza_NN.png`, NN = c243
   0-based a due cifre (00-51), scala 3 (720x576 più legenda). — preferenza.
5. Sprite (Dana, nemici) e HUD (punteggio, tempo) non sono nelle immagini:
   proposta di mostrare solo fondale, blocchi, porta, specchi di partenza e
   oggetti. — preferenza.
6. I nomi italiani delle classi sono descrittivi dal tile (il gioco non ne
   contiene); l'utente può volerli sostituire con i nomi ufficiali inglesi.
   Proposta: tenerli descrittivi e aggiungere la colonna del tile. —
   preferenza.

## Decisioni (chat principale, 2026-11-05)

1. Niente prova di rivelazione reale in MAME per ora (scelta di progetto,
   decisa in autonomia sul mandato dell'utente): le 3 classi non coperte
   (0x16, 0x17, 0x35) restano annotate come coda; si verifica solo se una
   mappa mostra un oggetto sospetto.
2-5. Preferenze accettate come proposte (byte 5-8 solo in legenda; oggetti nei
   blocchi dietro --nei-blocchi, non disegnati di default; docs/special/,
   NN=c243 da 0, scala 3; niente sprite né HUD).
6. MODIFICA: nomi degli oggetti e legenda IN INGLESE, e il file di
   accompagnamento è docs/special/README.md (non LEGGIMI.md) — le immagini
   sono destinate alle pagine pubbliche del repo (regola del progetto: tutto
   ciò che è rivolto al pubblico va in inglese).

## Esito (2026-11-05)

Ciclo passato al primo colpo (31º). Commit `a0d94f2`; verificatore: 8/8
passate — 52 PNG in docs/special/, 128 nascosti in 24 classi, renderer Python
identico al C pixel per pixel (3 stanze campione 57344/57344), 21 classi
confrontate con MAME ok, idempotente, niente file toccati fuori perimetro.
Coda: classi 0x16/0x17/0x35 mai visibili in chiaro, senza confronto MAME
(decisione: si verifica solo se un'immagine mostra un oggetto sospetto);
medaglia 0x36 senza punti in tabella (oltre la 0x150C). Token: analista
Sonnet ~123k, sviluppatore Opus ~78k, verificatore Sonnet ~26k.
