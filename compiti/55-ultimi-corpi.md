# Compito 55: ultimi corpi del gioco libero (15 16 24 27), voci vuote 1E 21, chiusura della tabella 3B11

Data: 2026-11-04. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.
Riferimento di metodo: `compiti/54-corpi-residui.md` (con l'esito in fondo) e
`compiti/52-corpi-gioco-libero.md`. Qui non si ripete ciò che hanno già fatto
(banco dei corpi, `SNAP_CORPO`, modo `stato`, convenzioni dei corpi a passi).

## Obiettivo

Il gioco libero (`harness/gioca`) a fine partita si ferma oggi con «FERMO corpo
15 non tradotto» (seme 10 al frame 7329). Questo lotto traduce in C i corpi 15
(6466), 16 (6496), 24 (2C72), 27 (62B6), più le voci vuote 1E e 21, i piccoli
helper che i corpi chiamano (251D, 2529, 2533, 6379, 7A56) e li verifica nel
banco con catture MAME. A fine lotto i 40 codici della tabella 3B11 sono tutti
tradotti e una partita del gioco libero arriva a fine partita e attract senza
«FERMO corpo». Resta, per decisione, la sentinella «lettura del registro R non
modellata» (corpo 12 → 4302), lotto a parte.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si toccano
  `replay.c`, `sched.c`, `sched_proto.c`, `macchina.c`, `video_*`, `audio_*`,
  `rst20_tables.h`, `rst38_tables.h`, `snap.lua`, `cattura.sh`, `compiti/`
  (tranne questo file: solo la sezione finale con gli esiti).
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le 800
  righe (soglia di lavoro 650 per i moduli nuovi). `gcc` dal Makefile
  (`-std=c11 -Wall -Wextra -O0`), nessuna dipendenza nuova, niente binari né `.o` nel commit.
- MAME solo senza finestra né audio (lo fa `cattura.sh`); a fine lavoro
  `pgrep -c mame` → `0`. Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`).
  Un solo agente alla volta su `decomp/harness`.
- Nessuna riscrittura di corpi già tradotti. Modifiche ammesse su codice esistente:
  togliere `static` (e dichiarare in header) a `fase_2dee` e `fase_3225`
  (`attivita_attract07.c`), a `passo_6655` (`attivita_26.c`), e a `attesa_13`
  (`attivita_12_13.c`, da rinominare `attesa_64ce`, tre punti d'uso), senza cambiarne il corpo.
- `z80_2xxx.c` è a 796 righe: non si accresce. Gli helper nuovi vanno in file nuovi.

## Cosa esiste già (fatti verificati)

Misure di oggi (binari già compilati, da `decomp/harness`, uscite identiche a quelle del brief 54):
- `./harness --tutte`: exit 0, `FALLITO` 0, `^RIEPILOGO` 327, `^cattura` 1371, `^OK` 1371,
  `registri non confrontati` 80.
- `./gioca --riproduci ../ricette/partita_1a.reg -n 4000 --muto 2>/dev/null | tail -4` →
  `FRAME rese 4000` / `HASH ultimo 3f25ba053eeb56b9` / `AUDIO campioni 2988510` / `AUDIO hash c097308a8678392e`.
- Da `decomp/`: `harness/gioca -n 5434 --muto --oracolo out/giroboot_oracolo.txt out/giroboot_disp.txt
  --riproduci out/ingressi_replay.txt 2>/dev/null | tail -2` → `AUDIO campioni 4042500` / `AUDIO hash c4b40fea06623a96`.
- Fine partita (da `decomp/`, `python3 ricette/genera_ingressi.py SEME /tmp/sSEME.txt 40000 8`, poi
  `harness/gioca -n 40000 --muto --riproduci /tmp/sSEME.txt 2>&1 | grep -a 'FERMO\|FRAME rese'`):
  semi 1 2 3 10 → `FERMO corpo 15 non tradotto al frame 7329` e `FRAME rese 7263`;
  seme 67 → frame 10634, `FRAME rese 10568`; seme 4 → frame 11370, `FRAME rese 11304`.
  Il corpo 15 lo posta 6655 (`passo_6655`) o 1C/26; con `c237` a zero 6655 posta il 16.

| file | righe | nota |
|---|---:|---|
| `harness/attivita.c` | 392 | tabella `corpo_attivita_replay` r. 358-: mancano 0x15 0x16 0x24 0x27 e 0x1e 0x21 (0x00 non è mai spedito: `spedisci_una` salta gli slot a 0) |
| `harness/attivita_libera.h` | 86 | prototipi dei corpi dei lotti 52 e 54 |
| `harness/banco_corpi.c` | 26 | macro `BANCO_CORPO(sfx, fn)`: slot = `[SP] - 0xC000` |
| `harness/routine_tab.c` | 340 | voci `{ "0xNNNN", fn, 0, 0, solo_ram }`, ordine di indirizzo (non rigoroso) |
| `harness/attivita_attract07.c` | 307 | `fase_2dee` r. 45 e `fase_3225` r. 107 sono `static`; `attivita_07_2ccd` r. 271-307 è il modello dei corpi a passi |
| `harness/attivita_attract07.h` | - | prototipi esportati: `fase_07_3861 _37d7 _358f _3a7d _2dce _31c5 _2e0a _2e9c _3709 _3829 _321e _2ef5 _2f1a`, `fase_07_2e1e(uint8_t slot)`, `attesa_07(uint16_t hl, uint8_t c, uint16_t sito)` |
| `harness/attivita_26.c` | 121 | `sub_6502` (esportata), `passo_6655` r. 92 (`static`, da esportare) |
| `harness/attivita_12_13.c` | 202 | `static void attesa_13(uint8_t c)` r. 88: HL = c19d, C = soglia, `sub_2e90` (la voce `ld c,N; call 2e90; pop hl; ret`) |
| `harness/z80_2xxx.c` | 796 | contiene `sub_2e90`; NON accrescerlo. `Makefile`: `SORGENTI_BASE` ha `$(wildcard z80_*.c)` (un `z80_25xx.c` entra da solo), i moduli `attivita_*.c` vanno aggiunti a `SORGENTI_NEMICI` |
| `decomp/corpi_catture.txt`, `cattura_corpi.sh` | 112, 40 | 93 righe di dati; modi `corpo`, `sub`, `stato` |

Routine già tradotte (da `routine_tab.c`/`sub_`): 0493 0028 673F 75AA 4AF6 4B0E 6502 3569 33BA 6771
3A54 3F88 3A1C 4D37 4D87 4398 2457 2D8A 31E9 342D 6976 2F57 2E90. Sono quelle segnate (T) sotto.

### Corpo 15 (ingresso 6466, tabella 646B, 10 passi, ~232 istr.; da `main64.dasm`)
Contatore **c19d**; modello: `uint8_t passo = M(0xc19d); M(0xc19d) = passo+1; switch (passo)`,
`default: nemici_fuori_tabella(0x646b, passo)`. Il passo 0 inizia con `pop hl` (64D3).
| idx | voce | istr. | note |
|---:|---|---:|---|
| 0 | 64D3 | 20 | 0493 4AF6 4B0E 6502 (T) |
| 1 | 64B4 | 5 | attesa `attesa_64ce(0x06)` |
| 2 | 6539 | 34 | 4AF6 (T) |
| 3 | 64C4 | 5 | attesa 0x14 |
| 4 | 657B | 64 | 2457 4398 (T); **nuovi** 251D, 2529 e i blocchi locali 65BF, 65C7 |
| 5 | 65D0 | 88 | 2529 (nuovo), 65BF 65C7 (locali) |
| 6 | 64C8 | 5 | attesa 0x48 |
| 7 | 6644 | 7 | 3569 (T) |
| 8 | 64CC | 4 | attesa 0x78 |
| 9 | 6655 | - | `passo_6655()` di attivita_26.c (75AA: dura più di un frame) |

### Corpo 16 (ingresso 6496, tabella 649B, 10 passi, ~510 istr.: il censimento 288 era sbagliato)
Contatore c19d. Tabella da `rst20_tables.h`; istruzioni contate seguendo i salti.
| idx | voce | istr. | note |
|---:|---|---:|---|
| 0 | 674A | 9 | 33BA (T) |
| 1 | 64BC | 5 | attesa 0x20 |
| 2 | 675B | 5 | 6771 (T) |
| 3 | 64C0 | 5 | attesa 0x60 |
| 4 | 6842 | ~216 | **75AA a 684C**; classifica (record in `ca4c`); chiama 3A1C 3A54 3F88 4398 4D37 4D87 6771 6976 (T), 2533 e 7A56 (nuovi). Senza record fa `c19d += 2` (6869-686D) e salta il passo 5 |
| 5 | 6785 | 101 | nessuna chiamata |
| 6 | 64CC | 4 | attesa 0x78 |
| 7 | 6A43 | 39 | 3569 3A54 3F88 6771 75AA (T) |
| 8 | 69A7 | 80 | 2D8A 31E9 342D 3A1C 3A54 4D87 (T); 75AA solo in coda (6A3C) |
| 9 | 6ADE | 43 | 0493 3F88 673F 75AA (T) |

**Attenzione**: `main64.dasm` decodifica male 6837-6841 (dati) e 683B-6848: l'istruzione a **6842**
non compare. Byte veri: `21 4E C2 CB 56 CB 96 C4 76 69 CD AA 75` = `ld hl,c24e; bit 2,(hl); res 2,(hl);
call nz,6976; call 75aa`. Da 6849 in poi il disassemblato è corretto. Verificare i byte con
`xxd -s 0x6837 -l 32 decomp/out/main64.bin`. Il blocco 6870-6932 (inserimento record) chiama 2533 a 6879
e 7A56 a 68CA; 7A56 (81 istr., chiamata anche da 7A2B) disegna con 3A1C 3F88 4D37 4D87 6771 (T).
Le routine 2533 (13 istr.), 251D (12), 2529 (10) sono helper da disassemblato (2523 è l'etichetta intermedia di 251D).

### Corpo 24 (ingresso 2C72, tabella 2C77, 19 passi, finale; posto da 6464 del 17 e da 4032)
Contatore c19d. Quasi tutto è già tradotto come fasi del 07; l'unica voce nuova è 2C9E (22 istr.:
`c0bd+c248` e `c0bd+c247` = 0x37, 4B0E, scritture in IX = c85e (+0E, +15), 2F57 4AF6 4B0E (T)).
| idx | voce | traduzione |
|---:|---|---|
| 0 | 3861 | `fase_07_3861()` |
| 1 | 2DEE | `fase_2dee()` (da esportare) |
| 2 | 37D7 | `fase_07_37d7()` |
| 3 | 358F | `fase_07_358f()` |
| 4 | 3225 | `fase_3225()` (da esportare) |
| 5 | 3A7D | `fase_07_3a7d()` |
| 6 | 2E89 | `attesa_07(0xc19d, 0x60, 0x2e89)` |
| 7 | 2C9E | **nuova**, `passo_2c9e()` |
| 8 | 2E5D | `attesa_07(0xc19d, 0x0d, 0x2e5d)` |
| 9 | 2DCE | `fase_07_2dce()` |
| 10 | 2E9C | `fase_07_2e9c()` |
| 11 | 3709 | `fase_07_3709()` |
| 12 | 3829 | `fase_07_3829()` |
| 13 | 321E | `fase_07_321e()` |
| 14 | 31C5 | `fase_07_31c5()` |
| 15 | 2EF5 | `fase_07_2ef5()` |
| 16 | 2E6D | `attesa_07(0xc19d, 0x1e, 0x2e6d)` |
| 17 | 2F1A | `fase_07_2f1a()` |
| 18 | 2E1E | `fase_07_2e1e(slot)` |
(stessi argomenti dei `case` corrispondenti di `attivita_07_2ccd`, r. 271-307: controllarli lì.)

### Corpo 27 (ingresso 62B6, tabella 62B9: 2 voci, **indice in c1ae**, non c19d)
62B6 `ld a,(c1ae); rst 20h` senza `inc`: il passo si avanza da solo.
- Passo 0, 62E0 (92 istr.): 673F, 75AA, 3A1C, 4D37, 4D87 (T) e 6379 sette volte (nuova, 25 istr.: legge
  `(IY+0)`, incrementa IY, 4D37, 4D87); chiude a 6371-6378 con `inc (c1ae); pop hl; (hl) = 27; ret`.
- Passo 1, 62BE (20 istr.): `a=(e603); rrca; pop hl; ret c`; poi `ex de,hl`, `c045` bit 7 a 0, `c1ae = 0`,
  slot = 06, azzera 4096 byte da `d400` (loop 16×256). Postato solo da 017B (NMI con e603 bit 0 = 1).
Entrambi i passi durano più di un frame: **non catturabili** (provato).

## Catture provate dall'analista

Provate in una copia fuori dal repo (`decomp/` ridotto a `cattura.sh`, `snap.lua`, `cattura_corpi.sh`,
`roms` collegato), 41+8+3+3 tentativi in ~45 s con `xargs -P6`. Per i corpi, `_prima.bin` (16 KB, da c000) ha
c19d = K all'ingresso. Esiti:
- 15: k0-k8 OK; k9 (6655, 75AA) FALLITA.
- 16: k0 k1 k2 k3 k5 k6 k8 OK. k4 e k7 (75AA) riescono ma sono del passo seguente (`_prima` c19d = 8): duplicati di
  k8, **non vanno nell'elenco**. k9 FALLITA. Il k8 non esercita il ramo 6A3C→75AA: lo stesso ramo si verifica per composizione.
- 24: k0-k18 OK tranne k4 (3225 è lungo: la cattura è del passo 6, duplicato di k6, fuori elenco).
- 27: `corpo 27` con c1ae = 0 e 1 (anche con e603 = 0): FALLITA (come previsto).
- Sub-routine: `sub 27 0x6379` (poke `2290:0xc1ae:0`, ricetta di `ZONE-MAI-ESEGUITE.md`) OK,
  `prima PC=6379 AF=FF40 BC=0A50 DE=0020 HL=D65C IX=C062 IY=C224 SP=CFFA`, `dopo PC=6313 ... IY=C22E SP=CFFC`;
  `sub 15 0x251D` con k4 OK (`prima ... HL=C2C1 SP=CFFA`, `dopo PC=658C`); `sub 15 0x2529` con k4 e k5 OK
  (due contesti: SP=CFF8 e SP=CFFA); `sub 16 0x2533` con i quattro poke del punteggio OK (ricetta di
  `ZONE-MAI-ESEGUITE.md`); **7A56 FALLITA** in tre varianti (dura più di un frame) → composizione.
- Voci vuote: `corpo 1E 0x3B61` e `corpo 21 0x3B61` OK; `corpo 00` FALLITA (0 non è un codice: nulla viene spedito).

Righe da aggiungere a `decomp/corpi_catture.txt` (formato del brief 52, tutte provate; **41 righe**):
```
corpo 15 0x6466 kK 1 2300:0xc19d:K          (K = 1..8; k0 con poke "-")
corpo 16 0x6496 kK 1 2300:0xc19d:K          (K = 1 2 3 5 6 8; k0 con "-")
corpo 24 0x2C72 kK 1 2300:0xc19d:K          (K = 1..18 tranne 4; k0 con "-")
corpo 1E 0x3B61 base 1 -
corpo 21 0x3B61 base 1 -
sub   27 0x6379 s1 1 2290:0xc1ae:0
sub   15 0x251D h4 1 2300:0xc19d:4
sub   15 0x2529 h4 1 2300:0xc19d:4
sub   15 0x2529 h5 1 2300:0xc19d:5
sub   16 0x2533 pt 1 2290:0xc239:0x99,2290:0xc23a:0x99,2290:0xc23b:0x99,2290:0xc23c:0x99
```
Conteggio: 15 → 9, 16 → 7, 24 → 18, 3B61 → 2, sub → 5. Totale elenco 93 + 41 = 134.
Dopo ogni rigenerazione i `.log` delle catture vecchie differiscono (rumore di MAME): ripristinarli con
`git checkout -- <file.log>` (solo quelli tracciati modificati). Le catture nuove aggiungono 164 file `out/snap_*`.
Casi con rami non coperti dalla cattura base (il disassemblato lo mostra) si coprono con catture aggiuntive a
poke liberi; ogni riga aggiunta va provata e riportata.

## Struttura da realizzare

Moduli nuovi (nomi per codice; tutti i prototipi in `attivita_libera.h`; i `.c` di attivita in `SORGENTI_NEMICI`):
- `harness/attivita_15.c` (~330 righe): `attivita_15_6466(uint8_t slot)`, passi 64D3 6539 657B 65D0 6644 e i blocchi locali
  65BF 65C7 (funzioni `static`); attese con `attesa_64ce`; passo 9 con `passo_6655()`.
- `harness/attivita_16.c` (~280 righe): `attivita_16_6496(uint8_t slot)`, passi 674A 675B 6785 6A43 69A7 6ADE.
- `harness/attivita_16b.c` (~330 righe): `passo_6842(void)` (classifica, con 6976 T) e `sub_7a56(struct z80 *z)`.
- `harness/attivita_24.c` (~80 righe): `attivita_24_2c72(uint8_t slot)` e `static passo_2c9e`.
- `harness/attivita_27.c` (~170 righe): `attivita_27_62b6(uint8_t slot)` (indice c1ae), passi 62E0 e 62BE, `sub_6379(struct z80 *z)`.
- `harness/z80_25xx.c` (~60 righe, entra dal wildcard): `sub_251d`, `sub_2529`, `sub_2533` (firma `void sub_XXXX(struct z80 *z)`).
- Voci vuote: in `attivita_27.c` o in un file a scelta, `void attivita_vuota_3b61(uint8_t slot) { (void)slot; }`
  (3B61 è `pop hl; ret`: lo slot non cambia), per i codici 1E e 21.
Modifiche a file esistenti: `attivita.c` (`[0x15]`, `[0x16]`, `[0x1e]`, `[0x21]`, `[0x24]`, `[0x27]`, commento di testa),
`attivita_attract07.c/.h` (esportazione di `fase_2dee`, `fase_3225`), `attivita_26.c` (esporta `passo_6655`),
`attivita_12_13.c` (`attesa_64ce`), `banco_corpi.c` (`BANCO_CORPO(6466, attivita_15_6466)`, `6496`, `2c72`,
`3b61` su `attivita_vuota_3b61`), `routine_tab.c` (`0x251D 0x2529 0x2533 0x2C72 0x3B61 0x6379 0x6466 0x6496`:
i wrapper dei corpi con `solo_ram` = 1, le sub con 0), `Makefile`, `cattura_corpi.sh` (nessuna modifica prevista:
i modi bastano) e `corpi_catture.txt`.
Il corpo 27 e i passi non catturabili non hanno voce nel banco (verifica per composizione): per il 27 il banco
prova 6379, 673F, 3A1C, 4D37, 4D87 e il giro intero prova che il C non si ferma.
Regola: prima le catture (con lo script), poi la traduzione, poi il banco; ogni voce si guida dal diff del banco.

### Documenti
- `decomp/SCHEDULER.md`: righe della tabella 3B11 di 15/16 (r. 95), 24 (r. 104), 27 (r. 108), e 00/1E/21; aggiungere la frase
  «tutti i 40 codici della tabella 3B11 sono tradotti (lotto 55)».
- `decomp/GIRO.md`: sezione «Lotto 55» breve (corpi, esito del fine partita, ridotti «FERMO»), e correggere
  la frase «Restavano non tradotti: ... 15 16 24 27 (lotto 55)» (r. ~424) in «tradotti nel lotto 55».
- `STATO.md` r. 231: spuntare il lotto 55; indicare che restano il registro R e, se presenti, i rami «routine mancante»
  elencati nel rapporto (domanda 1). `decomp/ROUTINE.md`: righe delle routine nuove, se il file le elenca.

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master` salvo dove scritto. Uscita esatta salvo «attesa, non misurata».

1. Compilazione: `cd decomp/harness && make harness replay gioco_libero gioca sched_proto > /tmp/m55.txt 2>&1;
   echo "make $?"; grep -c -E 'warning|error' /tmp/m55.txt` → `make 0` e `0`.
2. Catture: `cd decomp && sh cattura_corpi.sh | tail -1` → `CATTURE OK 134 su 134` (93 + 41, attesa: le 41 sono provate).
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h55.txt; echo "exit $?"; grep -c FALLITO /tmp/h55.txt;
   grep -c '^RIEPILOGO' /tmp/h55.txt; grep -c '^cattura' /tmp/h55.txt; grep -c '^OK' /tmp/h55.txt` →
   `exit 0`, `0`, `335`, `1412`, `1412` (327+8 voci, 1371+41; attese, non misurate).
4. Una riga per voce: `for a in 6466 6496 2C72 3B61 6379 251D 2529 2533; do grep "^RIEPILOGO 0x$a:" /tmp/h55.txt; done` →
   `0x6466: 9 catture, 9 OK`, `0x6496: 7 catture, 7 OK`, `0x2C72: 18 catture, 18 OK`, `0x3B61: 2 catture, 2 OK`,
   `0x6379: 1 catture, 1 OK`, `0x251D: 1 catture, 1 OK`, `0x2529: 2 catture, 2 OK`, `0x2533: 1 catture, 1 OK` (attese, non misurate;
   il testo «1 catture» segue il formato esistente).
5. `grep -c 'registri non confrontati' /tmp/h55.txt` → `116` (80 + 9 + 7 + 18 + 2; attesa, non misurata).
6. Invarianti, come nelle «Misure di oggi»: da `decomp/`, `sh giro_completo.sh | tail -1` → `GIRO COMPLETO 0-5499: saldatura 0 byte,
   spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`; `gioca --oracolo` →
   `AUDIO hash c4b40fea06623a96`; partita 1A (`./gioca --riproduci ../ricette/partita_1a.reg -n 4000 --muto`, da
   `decomp/harness`) → `FRAME rese 4000`, `HASH ultimo 3f25ba053eeb56b9`, `AUDIO hash c097308a8678392e`, nessun FERMO
   (i nuovi corpi non sono mai raggiunti da questa partita: gli hash non cambiano).
7. **Fine partita** (verifica nuova), da `decomp/`: per `S in 10 4 67`: `python3 ricette/genera_ingressi.py $S /tmp/s$S.txt 40000 8;
   harness/gioca -n 40000 --muto --riproduci /tmp/s$S.txt 2>&1 | grep -a 'FERMO\|FRAME rese'` → nessuna riga `FERMO corpo`
   (vecchi: frame 7329, 11370, 10634). Atteso `FRAME rese 40000` senza FERMO (non misurata). Se compare un altro FERMO
   (per esempio «routine XXXX mancante» in un ramo del 07, o la sentinella del registro R) riportare messaggio e frame,
   non nasconderlo (domanda 1). Ripetere il seme 10 due volte: stessi `HASH ultimo` e `AUDIO hash` (determinismo).
8. Semi 4 e 67 a 6000 frame e seme 10 a 6000 frame: ancora `FRAME rese 6000` senza FERMO (regressione del lotto 54).
9. Dimensioni: `find decomp/harness -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 && $2!="totale"'` → vuota;
   nessun modulo nuovo oltre 650 righe.
10. Ambito: `git status --short` mostra solo file di `decomp/` (sorgenti, header, documenti, elenco, `out/snap_*` nuovi: 164),
    `compiti/55-…` e `STATO.md`; `pgrep -c mame` → `0`; nessun `.o` né binario.

## Consegna

Commit unico (o due: codice e banco, poi documenti) con messaggio in italiano «Lotto 55: corpi 15 16 24 27, voci vuote
1E 21 e chiusura della tabella 3B11» e riga `Co-Authored-By: <modello> <noreply@anthropic.com>`. Includere le
catture `decomp/out/snap_*` nuove. In fondo a questo file la sezione «Esito» con le uscite di 1-10, la lista delle righe dubbie
dei passi non catturabili (6842, 7A56, 6A43, 69A7 coda, 6ADE, 62E0, 62BE) e le deviazioni. Rapporto finale: massimo trenta righe.

## Domande aperte

1. **Rami «routine XXXX mancante» della famiglia 07** (`nemici_manca`): ~20 siti nelle fasi che il corpo 24 e l'attract
   dopo il fine partita riusano (`attivita_attract07a.c`: 386A 3874 38BA 38CA 38D9 3901 3903 3960 3986 398B 39B9;
   `attivita_attract07.c`: 2D0F 2D1F 2E04 2D9D; `07c`: 3065; `attivita_attract08.c`: 2C61 7DA0 7DC6 7DDE), mai eseguiti nei frame 0-2410.
   Il fine partita potrebbe raggiungerne qualcuno (non misurabile prima). Proposta: fuori perimetro per default; se la verifica 7
   si ferma su uno, tradurre quel ramo (catture se possibile, altrimenti composizione) fino a 3 siti distinti e non oltre 60
   istruzioni l'uno; oltre, chiudere il lotto riportando i siti. Rischio: lotto che cresce o fine partita non
   raggiunto in questo ciclo. **Scelta di progetto** (perimetro).
2. **Passi non catturabili verificati per composizione**: 6842 (classifica, ~216 istr. con 7A56), 6A43, coda di 69A7, 6ADE, 62E0, 62BE,
   voce 3225 del 24. Rischio maggiore: 6842 e 7A56 (nessuna prova diretta, solo le routine chiamate sono nel banco; 2533 e 6976 sì).
   Proposta: tradurre dal disassemblato rileggendo riga per riga e riportare le righe dubbie; stesso criterio già accettato
   per il passo 7 del 13 e i passi con 75AA dei lotti 52 e 54. **Scelta di progetto** (copertura della verifica).
3. **Voci vuote 1E e 21** (`pop hl; ret` a 3B61): proposta di tradurle con `attivita_vuota_3b61` e due catture, così
   `corpo_attivita_replay` ha tutte le voci 01-27. **Preferenza**.
4. **Rinomina `attesa_13` → `attesa_64ce`** ed esportazione con prototipo in `attivita_libera.h`; stessa cosa per `passo_6655`,
   `fase_2dee`, `fase_3225`. **Preferenza**.
5. **Ordine di priorità se serve spezzare**: 15 (con 251D 2529 e le voci vuote), poi 16, poi 24, poi 27. Il fine partita richiede 15 e 16;
   il 27 è postato solo dalla NMI con e603 bit 0 = 1 e il 24 dal finale (non raggiungibile con ingressi casuali): per questi due la
   prova è solo nel banco e per composizione. **Preferenza**.
6. **Nomi** dei moduli, funzioni e tag delle catture (`kK`, `s1`, `h4`, `h5`, `pt`, `base`). **Preferenza**.

## Risoluzioni delle domande aperte (chat principale sotto mandato utente, 2026-11-04)

1. Rami «routine mancante» della famiglia 07 (~20 siti): FUORI PERIMETRO,
   ma se la verifica del fine partita si ferma su uno di essi se ne
   traducono FINO A 3, come da proposta. Se ne servissero di più, fermarsi
   e segnalarlo nell'esito: si farà un lotto di coda.
2. Passi non catturabili (62E0, 62BE, 7A56, 6842, 6A43, 6ADE, coda di
   69A7) verificati PER COMPOSIZIONE: APPROVATO, criterio ormai standard.
   Per 6842+7A56 (classifica, il rischio maggiore e disassemblato rotto
   in quel punto): tradurre dai byte veri riportati nel brief, rileggere
   riga per riga come fatto per 3BAC nel lotto 54.
3. Voci vuote 1E e 21 con due catture: preferenza ACCETTATA.
4. Rinomina attesa_13→attesa_64ce ed esportazioni: preferenze ACCETTATE.
5. Ordine di priorità 15, 16, 24, 27 se serve spezzare: ACCETTATO.
6. Nomi di moduli, funzioni e tag: preferenze ACCETTATE.

## Esito (sviluppatore, Opus 5.5, 2026-11-04)

Uscite delle verifiche:
1. `make 0`, `0` warning/error.
2. `CATTURE OK 134 su 134` (i 93 `.log` vecchi riscritti da MAME ripristinati con `git checkout`).
3. `exit 0`, `0`, `335`, `1412`, `1412`.
4. `0x6466: 9 catture, 9 OK`, `0x6496: 7/7`, `0x2C72: 18/18`, `0x3B61: 2/2`, `0x6379: 1/1`,
   `0x251D: 1/1`, `0x2529: 2/2`, `0x2533: 1/1`.
5. `116`.
6. `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame
   resi con hash coincidenti`; oracolo `AUDIO hash c4b40fea06623a96`; partita 1A `FRAME rese 4000`,
   `HASH ultimo 3f25ba053eeb56b9`, `AUDIO hash c097308a8678392e`, nessun FERMO.
7. Nessun `FERMO corpo`. Non si arriva a 40000: seme 10 «FERMO lettura del registro R non modellata al
   frame 24396» (`FRAME rese 24330`), seme 4 al 22412 (22346), seme 67 al 21676 (21610), sempre corpo 12 →
   4302 (backtrace in gdb), stesso `HASH ultimo a964cd9375d7a827`. Seme 10 due volte: `HASH ultimo
   a964cd9375d7a827`, `AUDIO hash c185e0fb15e096d1` entrambe le volte.
8. Semi 4, 67, 10 a 6000 frame: `FRAME rese 6000`, nessun FERMO.
9. Nessun file sopra 800 righe; moduli nuovi tra 37 e 334 righe.
10. Solo `decomp/` (164 `out/snap_*` nuovi), questo brief e `STATO.md`; `pgrep -c mame` → `0`; nessun `.o`.

Rami extra tradotti (domanda 1, tre siti, tutti del corpo 06 dell'attract, non del 07): dopo i corpi il
fine partita si fermava su «FERMO routine 7A0B mancante» (fase 5, classifica, 17 istr. + 7A56), poi su
«indice 0A fuori dalla tabella rst del sito 75EB». Fatto contrario al codice: la tabella 75EC ha 12 voci,
non 10 come dichiara `rst20_tables.h` (non toccato): la 10 è 760B, la 11 è 7604 (4 istr., c467 = c1ae = 0).
Tradotte in `attivita_06b.c` le fasi 5 (7A0B), 8 (7B3B, 37 istr. con 7B95 e 7B9E) e 11 (7604); la 10
riusa `fase_760b`. Il `switch` di `attivita_06_75e7` ha 4 `case` cambiati o aggiunti (deviazione dal
vincolo «nessuna riscrittura»: solo i rami mancanti).

Righe dubbie dei passi non catturabili (verificati per composizione):
- 6842: la sottrazione 685A-685E (record vinto con punteggio uguale: `jr nc`, quindi pari = sopra);
  l'`ex de,hl` di 6875 lascia HL = FFFC nel ramo con 2533 (ininfluente); 68E9 `jp m` su A = 0A - E.
- 7A56: lettura del nome con `leggi` da (c1b2); `rst 8` = HL + 2·A con A = 3·(B-1) (6 byte per voce).
- 6A43: `pop hl` di 6A54 rende HL = c19d (il `push hl` d'ingresso).
- 69A7 coda: `pop hl; ld (hl),17` = `tabella[slot] = 0x17`; il C della 3A1C resta c04c.
- 6ADE: `and 6; jr nz,6B16` salta all'istruzione seguente (nessun effetto).
- 62E0/62BE: `leggi_e603(0x62be)` non misurato (avviso e 0, come gli altri PC non in tabella).
- 6785 (catturato al k5) e 65C7: scrivono a indirizzi calcolati che con un riquadro non impostato
  cadono sotto c000 (ROM, scrittura persa): aggiunto `scrivi_mem` in `attivita_libera.h` (deviazione:
  senza, il banco andava in segmentation fault sul k5 del 15).

Altre deviazioni: `attivita_attract.c` include `attivita_libera.h`; nuovo modulo `attivita_06b.c` in
`SORGENTI_NEMICI`; `ROUTINE.md` con una riga di riepilogo del lotto.

## Esito del ciclo (chat principale, 2026-11-04)

Verificatore (Sonnet, ~22k token): le 10 verifiche rifatte, TUTTE PASSATE
(la 7 con la riserva attesa: i semi si fermano SOLO sulla sentinella del
registro R ai frame 21676-24396, determinismo confermato sul seme 10; la
10 segnava solo i 134 snap_*.log rigenerati con rumore MAME, ripristinati).
Analista Sonnet ~135k, sviluppatore Opus ~182k. Lotto 55 CHIUSO con
a043c62. TUTTI I 40 CODICI DELLA TABELLA 3B11 SONO TRADOTTI. Decisione
chat sotto mandato utente (criterio «se in partita non cambia nulla si
salta»): il registro R SI MODELLA in forma minima, perché ogni partita
libera si ferma su di esso dopo ~6 minuti — lotto dedicato a seguire.
