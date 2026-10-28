# Compito 25: lotto N1 del motore dei nemici (corpo 0A e routine coperte dal replay)

2026-10-28, scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Tradurre in C il corpo dell'attività 0A (4DA2) e le 26 routine del motore dei
nemici che il replay esegue (27 ingressi in tutto, 1564 byte Z80, circa 938
righe C stimate). Le 28 routine non coperte che N1 chiama o smista diventano
stub che fermano il replay dicendo quale routine manca. Risultato: il giro
`harness/replay` supera il frame 2410, dove oggi si ferma con «CORPO 0A».

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/` e `compiti/`. Commenti, messaggi e
  documenti in italiano.
- Nessun file di codice sopra le 800 righe. I moduli `nemici*.c` restano
  sotto le 700 (piano del censimento). Oggi in `decomp/harness` nessun file
  supera le 800 righe (misurato).
- Non toccare `rst20.py`, `rst20_tables.h`, `censimento_nemici.py` né
  `rst38_tables.h`: quest'ultimo è generato, si include e basta.
- Il banco storico (`harness`, `sched_proto`) non cambia: i moduli nuovi
  entrano solo in `SORGENTI_REPLAY`.
- MAME solo senza finestra e solo con `replay.sh`, che spegne video, audio e
  ingressi dell'host. Non serve rifare l'oracolo: `out/replay_oracolo.txt`,
  `out/replay_disp.txt` e `out/replay_2410.bin` ci sono già e sono
  riproducibili bit a bit. Si rilancia `replay.sh` solo per avere dump di
  diagnosi (`REPLAY_DUMPS=f1,f2 sh replay.sh`, circa 90 s).
- Niente git distruttivo (`stash`, `reset --hard`, `checkout -- file`,
  `filter-branch`). Niente push.

## Cosa esiste già (fatti verificati)

| file | righe | ruolo |
|---|---:|---|
| `decomp/CENSIMENTO-NEMICI.md` | 432 | piano dei lotti; tabella delle 220 routine con estensione, byte, copertura, lotto, modulo |
| `decomp/out/censimento_nemici.txt` | 221 | per ogni routine le chiamate, con `(t)` = già tradotta, e i chiamanti |
| `decomp/out/main64.dasm` | — | disassemblato z80dasm, indirizzo in commento a fine riga (`grep -n ';4da2$'`). Dopo un `rst 38h` o un `rst 20h` le «istruzioni» sono i byte della tabella: si usano le tabelle C, non il testo |
| `decomp/harness/rst38_tables.h` | 120 | `static const uint16_t rst38_XXXX[]`, 21 siti, indice in E (generato) |
| `decomp/harness/rst20_tables.h` | 136 | `rst20_XXXX[]`, indice in A. `rst20_4644[] = {0x46A3, 0x4649}` |
| `decomp/harness/attivita.c` | 350 | corpi 02, 03, 0D, 0E, 11, 14, 20; `corpo_attivita_replay[SCHED_CODICI]` alle righe 341-350; `static void sub_4b98(uint16_t ix)` alla riga 29 |
| `decomp/harness/replay.c` | 369 | giro e confronto; fermata `CORPO .. non tradotto` alle righe 333-337 e 353-356; `ferma(int)` alla riga 144 |
| `decomp/harness/routine.h` | 154 | prototipi `void sub_xxxx(struct z80 *z)` delle routine tradotte |
| `decomp/harness/ram.h` | 56 | `M(a)`, `struct z80 {a,f,b,c,d,e,h,l; ix,iy,sp}`, `BC/DE/HL`, `SET_BC/SET_DE/SET_HL` |
| `decomp/harness/sched.h` | 47 | `typedef void (*attivita_fn)(uint8_t slot)`, `tabella` = `&M(0xc000)` |
| `decomp/harness/Makefile` | — | `SORGENTI_REPLAY = $(filter-out harness.c,$(SORGENTI)) sched.c attivita.c replay.c`; il bersaglio `replay` dipende già da `rst38_tables.h` |
| `decomp/ROUTINE.md` | 149 | sezione «Corpi di attività (lotto 23)» dalla riga 129, stato «tradotta (replay)» |
| `decomp/REPLAY.md` | — | esito del replay e «Cose da sistemare»; il punto 1 è il corpo 0A |

### Le 27 routine di N1 e il modulo di destinazione

Da `CENSIMENTO-NEMICI.md` («Piano dei lotti») e `out/censimento_nemici.txt`.
Byte = istruzioni senza tabelle; copertura nel replay; chiamate, con `(t)` =
già tradotta.

| modulo | ingresso | estensione | byte | replay | chiama |
|---|---|---|---:|---|---|
| `nemici.c` | 4DA2 | 4DA2-4DA6 | 5 | SI | rst 38 4DA6 → 4DAD, 4632, 4EE6 |
| | 4DAD | 4DAD-4DED | 65 | SI | 249E(t), 4DEE(t), 4E31(t), 4E87(t) |
| | 4EE6 | 4EE6-4F0E | 41 | SI | 4F0F |
| | 4F0F | 4F0F-4F1B | 13 | SI | rst 38 4F1B (18 voci) |
| | 4632 | 4632-4644 | 19 | PARZIALE | rst 20 4644 → 46A3, 4649 |
| `nemici_comuni.c` | 2482 | 2482-248C | 11 | SI | - |
| | 248D | 248D-249D | 17 | SI | - |
| | 24BE | 24BE-24DE | 33 | SI | - |
| | 4592 | 4592-4631 | 160 | PARZIALE | 2457(t), 36F0(t), 3A54(t), 4938, 4AF6(t), 4B0E(t) |
| | 4938 | 4938-49AD | 118 | PARZIALE | 24BE, 3A2E(t), 3A54(t), 4AF6(t), 4B0E(t), 4C11 |
| | 49AE | 49AE-4AD9 | 300 | PARZIALE | 4ADA, 4AF6(t), 4B0E(t), 4B20 |
| | 4B20 | 4B20-4B83 | 100 | PARZIALE | 2474 |
| | 4C11 | 4C11-4C2F | 31 | SI | 2482, 248D |
| `nemici_tipi_10_13.c` | 55B2 | 55B2-55B5 | 4 | SI | rst 38 55B5 |
| | 55C9 | 55C9-561A | 82 | PARZIALE | 5C32(t) |
| | 561B | 561B-563F | 37 | SI | - |
| | 5640 | 5640-5692 | 83 | SI | 5507(t), 5C32(t) |
| | 5693 | 5693-56E8 | 86 | PARZIALE | 0008(t), 0010(t), 58CF, 5917, 5928, 7379(t) |
| | 714B | 714B-714E | 4 | SI | rst 38 714E |
| | 7155 | 7155-7195 | 65 | PARZIALE | 0008(t), 0010(t) |
| | 7235 | 7235-7287 | 83 | PARZIALE | 734D |
| `nemici_tipo09.c` | 58CF | 58CF-5916 | 72 | PARZIALE | 0008(t), 0010(t), 4938, 49AE, 5928 |
| | 5917 | 5917-5927 | 17 | SI | 0008(t), 0010(t), 4B98 (statica in `attivita.c`) |
| | 5928 | 5928-594E | 39 | PARZIALE | 36F0(t) |
| `nemici_comparsa.c` | 734D | 734D-7378 | 44 | PARZIALE | 36F0(t) |
| `nemici_tipi_15_17.c` | 7F51 | 7F51-7F67 | 23 | PARZIALE | jp 809C |
| | 7F68 | 7F68-7F72 (+ 7F39) | 12 | PARZIALE | rst 38 7F39 o 7F72 |

Somme per modulo (byte → righe stimate a 0,6 righe/byte): `nemici.c` 143 →
~86; `nemici_comuni.c` 770 → ~462; `nemici_tipi_10_13.c` 444 → ~266;
`nemici_tipo09.c` 128 → ~77; `nemici_comparsa.c` 44 → ~26;
`nemici_tipi_15_17.c` 35 → ~21. Totale 1564. Le routine PARZIALE si
traducono intere, compresi i rami mai presi.

**4FC5** (probabile comparsa della fata) **non è in N1**: è nel lotto N4,
modulo `nemici_comparsa.c`, chiamata da 4F40 e 73C7, ed è assente dal replay.
Non si traduce qui.

### Fatti di protocollo (da `out/main64.dasm` e `attivita.c`)

- **Corpo di un'attività.** Firma `attivita_fn`: `void f(uint8_t slot)`.
  `pop hl; ret` verso 3AF3 diventa `return`; `pop hl; ld (hl),n; ret` diventa
  `tabella[slot] = n; return` (commento di testa di `attivita.c`). Le chiamate
  alle routine tradotte passano gli argomenti in un `struct z80 z = {0}`
  locale, con commento dell'indirizzo a fine riga (es. `sub_349f(&z);  /* 5CD9 */`
  in `attivita_0d_5c8e`).
- **Corpo 0A.** `4DA2 ld a,(c2b8); ld e,a; rst 38h` con la tabella
  `rst38_4DA6` = {4DAD, 4632, 4EE6}. I tre rami terminano il corpo senza
  scrivere lo slot, quindi 0A resta in tabella:
  - 4DAD: lavoro su c2b7 e c2c9/c2c1, poi `pop hl` (4DE8), `inc (c2b8)`, `ret`;
  - 4632: `inc (c2b8)`, `pop hl` (4636), poi `ret z` su c459, oppure
    `rst 20h` con A = c459 & 7 sulla tabella `rst20_4644` (indice 0 → 46A3,
    1 → 4649, entrambe stub);
  - 4EE6: `ix = c2c1` (passo 0x18), `iy = c4fe` (passo 0x30), `b = 0x11`.
    Per ogni voce con `(iy+0) >= 0xc0` chiama 4F0F (`push bc`/`pop bc`
    intorno). Poi `pop hl` (4F08) e `ld (c2b8),0` e `ret`.
  In C: `void attivita_0a_4da2(uint8_t slot)` in `nemici.c`; 4DAD, 4632 e
  4EE6 sono `static void sub_4dad(struct z80 *z)` ecc. nello stesso file e
  il corpo ritorna dopo di loro.
- **`rst 38h` (0038-0042)**: `ex (sp),hl; ld d,0; add hl,de; add hl,de;
  ld e,(hl); inc hl; ld d,(hl); ex de,hl; ex (sp),hl; ret`. È un **salto**
  (non una call) a `tabella[E]`, con la tabella subito dopo il `rst`. All'uscita
  HL e A sono intatti; DE = indirizzo della tabella + 2E + 1, cioè il byte alto
  della voce; il riporto viene da `add hl,de`. La routine di destinazione
  ritorna al chiamante di chi ha fatto `rst 38`. In C: chiamata della
  destinazione e `return`. Se una destinazione legge DE prima di scriverlo,
  il C deve impostarlo come sopra.
- **4F0F**: `ld d,6; a=(iy+4); cp d; ret c; a=(iy+3)-d; ld e,a; rst 38h` su
  `rst38_4F1B` (18 voci). Un E ≥ 18 è fuori tabella.
- **55B2** e **714B**: `ld e,(iy+1Dh); rst 38h` su `rst38_55B5` = {5693,
  6C1D, 561B, 55C4, 55C9, 5640, 3B62} e `rst38_714E` = {7155, 719E, 7235}.
- **7F68**: `ld e,(iy+1Dh); a=(iy+4); cp 0A1h; jr z,7F39; rst 38h` su
  `rst38_7F72` = {7F81, 7FA6, 7F8A, 7FCF, 801D, 8042, 3B62}; a 7F39 c'è
  `rst 38h` su `rst38_7F39` = {3B62, 7FA6, 7F48, 7FCF, 7F51, 3B62, 3B62}.
  **`sub_7f2d` non c'entra.** In `z80_7xxx.c:215` traduce il ciclo 7F2D-7F38,
  che finisce con `ret` a 7F38. 7F39 è l'etichetta subito dopo e ci si arriva
  solo dal `jr z` di 7F68. Basta smistare le due tabelle dentro 7F68, senza
  toccare `sub_7f2d`. Il censimento la dava «dentro sub_7f2d» perché
  l'estensione di 7F68 parte da 7F39.
- **`rst 20h`** (0020): indice in A, tabella dopo il `rst`
  (`rst20_tables.h`: «rst20_XXXX[i] = destinazione per A = i»). Anche questo
  è un salto, con HL e DE conservati (`ex (sp),hl`/`push de`…`pop de`/`ex (sp),hl`).
- **3B62** è un `ret` solitario, la voce nulla di molte tabelle (4F1B indici
  1-3, 55B5 indice 6, 7F39 e 7F72).
- **4B98**: `attivita.c:29`, `static void sub_4b98(uint16_t ix)`, ha
  l'indirizzo dell'oggetto in IX. 5917 la chiama con `ld a,(ix+0Eh);
  ld hl,48AFh; rst 8; rst 10h; push de; ex (sp),ix; call 4B98; pop ix; ret`,
  quindi con IX = DE prodotto da `rst 8`/`rst 10h` (`sub_0008`, `sub_0010`
  in `routine.h`).
- Già tradotte e dichiarate in `routine.h` (da grep): `sub_0008`,
  `sub_0010`, `sub_0028`, `sub_2457`, `sub_249e`, `sub_349f`, `sub_36f0`,
  `sub_3a2e`, `sub_3a54`, `sub_4af6`, `sub_4b0e`, `sub_4dee`, `sub_4e31`,
  `sub_4e87`, `sub_5507`, `sub_5c32`, `sub_7379`. Nessun simbolo `sub_` delle
  27 di N1 o dei 28 stub esiste già in `harness/` (da grep).
- La tabella `rst 20` di 6F1C in `rst20_tables.h` ha 22 voci, ma quelle vere
  sono 16 (da 6F3D in poi sono dati). In N1 non conta (6ECA è uno stub), però
  chi la usa deve fermarsi a 16.

### Stub di N1 (28, da `CENSIMENTO-NEMICI.md` riga 432)

2474 3B62 4649 46A3 4ADA 4F40 4FD4 54D8 55C4 56E9 57CD 5CFB 6B43 6C0B 6C1D
6CE8 6ECA 719E 7288 73A3 7F48 7F81 7F8A 7FA6 7FCF 801D 8042 809C.
Nel replay 2400-5500 nessuno va in onda (copertura NO). 3B62 si traduce come
`ret` vero, vuoto, in `nemici.c` (vedi Domande aperte, 2): gli stub sono
quindi 27.

### Misure di oggi (2026-10-28, binari compilati alle 11:23)

```
$ cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410; echo "exit $?"
SANITA F0 2410: hash C A350CF53 MAME A350CF53 coincide
CORPO 0A (4DA2) non tradotto al frame 2410
NMI confrontate: 1 (nello spedidore 1, a metà attività 0); divergenti: nello spedidore 0, a metà attività 0
nessuna divergenza nello spedidore
exit 1
```
- `cd decomp/harness && ./harness --tutte`: 220 righe `^cattura`,
  0 `FALLITO`, exit 0.
- `cd decomp && harness/sched_proto out/sched_log.txt | tail -1`:
  `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`, exit 0.
- Spedizioni in `out/replay_disp.txt` dal frame 2410 (`awk '$1=="D" && $2>=2410'`):
  01×36, 02×2153, 03×3252, 0A×14802, 0D×14801, 0E×52, 10×9, 14×14802,
  20×14801. **La prima spedizione di 01 o 10 dopo il 2410 è `D 2589 27 01`**:
  i corpi 01 (48D1) e 10 (457B) restano non tradotti, quindi se il C è
  giusto il giro si ferma lì.

## Struttura da realizzare

1. **`harness/nemici.h`** (nuovo): prototipi `void sub_xxxx(struct z80 *z);`
   delle 26 routine di N1 tranne il corpo e dei 27 stub (minuscolo, come
   `routine.h`); `void attivita_0a_4da2(uint8_t slot);`;
   `void sub_4b98(uint16_t ix);`; il meccanismo degli stub:
   ```c
   /* Prima routine non tradotta raggiunta (0 = nessuna) e sito rst fuori tabella. */
   extern uint16_t nemici_mancante;
   extern uint16_t nemici_fuori_sito;
   extern uint8_t nemici_fuori_indice;
   void nemici_manca(uint16_t ingresso);      /* registra solo la prima */
   void nemici_fuori_tabella(uint16_t sito, uint8_t indice);
   ```
2. **Moduli** con le routine della tabella sopra: `nemici.c` (corpo 0A,
   4DAD/4632/4EE6 statiche, 4F0F, `sub_3b62` vuota, variabili e funzioni
   degli stub), `nemici_comuni.c`, `nemici_tipi_10_13.c`, `nemici_tipo09.c`,
   `nemici_comparsa.c`, `nemici_tipi_15_17.c`. Gli stub vanno tutti in
   `nemici_stub.c` (nuovo), uno per riga con una macro (es.
   `STUB_NEMICI(4f40)` → `void sub_4f40(struct z80 *z) { (void)z; nemici_manca(0x4f40); }`):
   ogni lotto N2-N12 toglie da lì le sue righe. Ogni modulo include
   `routine.h`, `nemici.h` e, se smista, `rst38_tables.h`/`rst20_tables.h`.
   Commento di testa con lotto, temi, ingressi. Commento dell'indirizzo a
   fine riga, nomi italiani, come in `attivita.c` e `z80_*.c`.
3. **Smistamento `rst 38`/`rst 20`**: da ogni tabella si va alla funzione con
   uno `switch` sull'indirizzo, in una funzione del modulo che smista
   (es. `static void salta_4f1b(struct z80 *z, uint16_t dest)`). Ogni
   destinazione chiama la sua `sub_xxxx`: tradotta, stub o `sub_3b62`. Un
   indice oltre la lunghezza dell'array (`sizeof/sizeof[0]`) chiama
   `nemici_fuori_tabella(sito, e)` e ritorna. Prima della chiamata, DE va
   impostato come fa `rst 38`.
4. **`attivita.c`**: `sub_4b98` non più `static` (firma invariata, prototipo
   in `nemici.h`); `#include "nemici.h"`; `[0x0a] = attivita_0a_4da2` in
   `corpo_attivita_replay`; commento di testa aggiornato (0A tradotto).
5. **`replay.c`**: `#include "nemici.h"`. Dopo `spedisci_una`, accanto al
   controllo di `corpo_mancante` (riga 353), aggiungere:
   - se `nemici_mancante`, stampare `ROUTINE %04X mancante (motore dei nemici) al frame %ld`
     e `ferma(1)`;
   - se `nemici_fuori_sito`, stampare
     `INDICE %02X fuori dalla tabella rst del sito %04X al frame %ld`
     e `ferma(1)`.
   Il controllo va fatto prima del confronto d'impronta della NMI seguente.
6. **`Makefile`**: in `SORGENTI_REPLAY` aggiungere `$(wildcard nemici*.c)`
   e, fra le dipendenze di `replay`, `nemici.h`. `SORGENTI` e
   `SORGENTI_SCHED` restano come sono.
7. **Documenti**:
   - `ROUTINE.md`: sezione nuova «Motore dei nemici (lotto N1)» dopo quella
     del lotto 23, con la stessa forma (ingresso, funzione, modulo,
     descrizione, stato «tradotta (replay)»). La riga 0A della tabella del
     lotto 23 diventa `attivita_0a_4da2`, «tradotta (replay)».
   - `REPLAY.md`: esito nuovo (uscita di `replay`), punto 1 di «Cose da
     sistemare» chiuso per N1, con il punto in cui il giro si ferma ora.
   - `CENSIMENTO-NEMICI.md`: in «Punti aperti», nota sotto 7F68 (7F39 è
     fuori da `sub_7f2d`, smistato in 7F68) e su 3B62 tradotta in N1.
   - `STATO.md` (radice): 2-4 righe nei prossimi passi (N1 fatto, prossimo).
8. **Se il giro diverge prima del 2589**, il primo sospetto è il C di N1.
   Diagnosi: `REPLAY_DUMPS=<f> sh replay.sh` (da `decomp/`, rifà l'oracolo
   identico e aggiunge `out/replay_<f>.bin`), poi `harness/replay ... 2410`
   per i 32 byte diversi e `python3 replay_diff.py` per la zona. Si
   correggono i bug di N1. Una divergenza che viene da un corpo del lotto 23
   o dall'interleaving della NMI si documenta in `REPLAY.md` e il lotto si
   chiude lì (Domande aperte, 1).

## Verifiche obbligatorie

1. `cd decomp/harness && make 2>&1 | grep -ci warning` → `0`, e `make`
   esce con 0 e produce `replay` (attesa, non misurata).
2. `cd decomp/harness && ./harness --tutte | grep -c '^cattura'` → `220`;
   `| grep -c FALLITO` → `0`; exit 0 (misurato oggi, deve restare uguale).
3. `cd decomp && harness/sched_proto out/sched_log.txt | tail -1` →
   `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`,
   exit 0 (misurato oggi, deve restare uguale).
4. `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410`:
   - nessuna riga `CORPO 0A`, nessuna `ROUTINE .... mancante`, nessuna `INDICE`;
   - prima riga `SANITA F0 2410: hash C A350CF53 MAME A350CF53 coincide`
     (misurata, invariata);
   - attesa, non misurata: `CORPO 01 (48D1) non tradotto al frame 2589`,
     exit 1, `nessuna divergenza nello spedidore` e circa 180 NMI
     confrontate. Se il giro si ferma prima su una divergenza nello
     spedidore, vale il punto 8 della struttura e il rapporto riporta
     frame, zona e attribuzione.
5. Con `--oltre` lo stesso comando arriva al 2589 e stampa il riepilogo
   (attesa, non misurata). Le righe vanno nel rapporto.
6. `cd decomp/harness && wc -l nemici*.c nemici.h | awk '$1>700 && $2!="totale"'`
   → vuoto (attesa).
7. `find decomp/harness -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 && $2!~/total/'`
   → vuoto (misurato oggi: vuoto).
8. Copertura degli ingressi: per ognuno dei 26 indirizzi di N1 diversi da
   4DA2 esiste `void sub_<ingresso minuscolo>(struct z80 *z)` in un modulo
   (`static` per 4DAD, 4632, 4EE6), più `sub_3b62`; da `decomp/harness`:
   `grep -ho 'sub_[0-9a-f]\{4\}(struct z80' $(ls nemici*.c | grep -v stub) | sort -u | wc -l`
   → `27`; `grep -c '^STUB_NEMICI(' nemici_stub.c` → `27` (attese).
9. `git status --short` → solo: `decomp/harness/Makefile`,
   `decomp/harness/attivita.c`, `decomp/harness/replay.c`,
   `decomp/harness/nemici.h`, i sette `decomp/harness/nemici*.c`,
   `decomp/ROUTINE.md`, `decomp/REPLAY.md`, `decomp/CENSIMENTO-NEMICI.md`,
   `STATO.md`, questo brief. I binari e `out/` sono gitignorati.

## Consegna

Commit in locale su `main` con messaggio «Lotto N1 del motore dei nemici:
corpo 0A e 26 routine coperte dal replay, stub delle mancanti» e riga
`Co-Authored-By: <modello> <noreply@anthropic.com>`. Nessun push.
Rapporto finale di trenta righe al massimo, con:
- l'uscita delle verifiche 1-9 e le righe esatte di `replay`, con e senza
  `--oltre`;
- le righe per modulo;
- le divergenze trovate e corrette;
- le deviazioni dal brief.

## Domande aperte

1. **Dove si chiude il lotto se il replay diverge.** Proposta: si
   correggono le divergenze imputabili al C di N1. Una divergenza dovuta ai
   corpi del lotto 23 (02, 03, 0D, 0E, 11, 14, 20) o all'interleaving della
   NMI si documenta in `REPLAY.md`, senza correggerla qui. Il lotto è
   passato se il giro supera il 2410 senza `CORPO 0A` né stub raggiunti.
   **Scelta di progetto.**
2. **3B62 in N1 o stub.** Il censimento la mette in N7 e fra gli stub, ma è
   un `ret` solo. Uno stub fermerebbe il replay su codice legittimo.
   Proposta: `sub_3b62` vuota in `nemici.c` già in N1, e N7 la trova fatta.
   **Preferenza.**
3. **Stub in un file a parte.** Proposta: `nemici_stub.c`, una macro per
   riga, che i lotti successivi svuotano. In alternativa gli stub vanno nei
   moduli di destinazione. **Preferenza.**
4. **Smistamento per indirizzo.** Proposta: uno `switch` sull'indirizzo
   di destinazione dentro il modulo che smista. In alternativa una tabella
   globale {indirizzo, funzione} in `nemici.c`. **Preferenza.**
5. **Corpi 01 e 10.** Tutte le loro routine, tranne 2474 e 4ADA, finiscono
   in `nemici_comuni.c` con N1. Tradurli subito porterebbe il giro oltre il
   2589. Proposta: non in questo lotto, ma con un brief a parte subito
   dopo, con il modulo `attivita_*.c` previsto da `REPLAY.md`.
   **Scelta di progetto.**

## Risoluzioni (2026-10-28)

- Domanda 1 (scelta di progetto, decisa dall'utente): **si corregge tutto.**
  Una divergenza nel replay si corregge anche quando è imputabile ai corpi
  del lotto 23 (02, 03, 0D, 0E, 11, 14, 20) o all'interleaving della NMI,
  non solo al C nuovo di N1. Ogni correzione fuori da N1 va comunque
  annotata in `REPLAY.md` (cosa divergeva, frame, causa) e nel rapporto.
  Il lotto è passato quando il giro supera il 2410 senza `CORPO 0A`, senza
  stub raggiunti e senza divergenze nello spedidore.
- Domanda 2: ACCETTATA la proposta — `sub_3b62` vuota in `nemici.c` già in
  N1, non stub.
- Domanda 3: ACCETTATA la proposta — stub tutti in `nemici_stub.c`, una
  macro per riga.
- Domanda 4: ACCETTATA la proposta — smistamento con `switch`
  sull'indirizzo dentro il modulo che smista.
- Domanda 5 (scelta di progetto, decisa dall'utente): ACCETTATA la
  proposta — i corpi 01 e 10 restano fuori da N1 e hanno un brief a parte
  subito dopo, con il modulo `attivita_*.c` previsto da `REPLAY.md`.
