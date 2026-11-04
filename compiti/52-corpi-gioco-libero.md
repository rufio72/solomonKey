# Compito 52: corpi di attività del gioco libero, lotto 52 (13 corpi + banco dei corpi + ricetta 1A)

Data: 2026-11-04. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

La tabella 3B11 ha 40 codici: 16 hanno il corpo in C (`corpo_attivita_replay`,
`decomp/harness/attivita.c` r. 351-368), 21 hanno un ingresso vero e non sono
tradotti; quando il gioco libero ne spedisce uno si ferma con «FERMO corpo XX
non tradotto» (`macchina_libera.c` r. 94). Questo lotto fa tre cose:
1. costruisce l'infrastruttura per catturare e verificare un **corpo** nel banco
   (oggi `snap.lua` e `harness.c` non lo sanno fare, vedi «Il banco dei corpi»);
2. traduce 13 dei 21 corpi: **04 05 09 0F 12 13 17 18 19 1A 1B 1D 23**;
3. produce una ricetta riproducibile per il corpo 1A (magia sulla chiave).
Gli altri 8 (1C 22 25 26 | 15 16 24 27) restano ai lotti 53 e 54, descritti in
«Spezzatura». Il banco (1278 OK / 307 RIEPILOGO / 0 falliti), `giro_completo.sh` e
l'hash audio di `gioca --oracolo` restano invariati.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si toccano
  `replay.c`, `sched.c`, `sched_proto.c`, `macchina.c`, `video_*`, `audio_*`,
  `rst20_tables.h`, `rst38_tables.h`, `compiti/` (tranne questo file: solo la
  sezione finale con gli esiti). `snap.lua` si estende (4 righe, comportamento
  invariato senza la nuova variabile); `cattura.sh` non si tocca.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le 800
  righe (soglia di lavoro 650 per i moduli nuovi). `gcc` dal Makefile
  (`-std=c11 -Wall -Wextra -O0`), nessuna dipendenza nuova, niente binari né `.o`
  nel commit. `cargo` non c'entra.
- MAME solo senza finestra né audio (lo fa già `cattura.sh`); a fine lavoro
  `pgrep -c mame` → `0`. Niente git distruttivo (`stash`, `reset --hard`,
  `filter-branch`). Un solo agente alla volta su `decomp/harness`.
- Nessuna riscrittura dei corpi già tradotti: si aggiungono voci, non si cambia
  la semantica dei 16 esistenti.

## Cosa esiste già (fatti verificati)

**Convenzioni dei corpi** (da `attivita_01_10.c`, letto): `void attivita_XX_YYYY(uint8_t
slot)`; `pop hl; ret` = `return`; `pop hl; ld (hl),n; ret` = `tabella[slot] = n;
return`. Le routine già tradotte hanno firma `void sub_xxxx(struct z80 *z)`
(`routine.h` 214 righe, `nemici.h` 374); il registro z serve solo a passare
argomenti. Prototipi dei corpi attuali in `nemici.h` r. 73-94. Lo spedidore
(`sched.c` r. 24-46, da `grafo`) chiama `corpo_attivita[codice](slot)`.

**Pila all'ingresso di un corpo** (misurato con la cattura sotto, `CORPO
slot=C000 ritorno=3AF3`, SP=CFFC): `[SP]` = indirizzo dello slot (C000+slot, lo
toglie il `pop hl` del corpo), `[SP+2]` = 3AF3. Anche all'ingresso di un passo
di un corpo a passi (dopo `ld hl,c19d; ld a,(hl); inc (hl); rst 20h`) è così.
`snap.lua` oggi aspetta il ritorno all'indirizzo in `[SP]` (r. 169-170: `ritorno =
mem:read_u16(sp)`, `sp_ritorno = (sp + 2) & 0xffff`), che per un corpo è lo
slot: non scatta mai, le catture di un corpo falliscono.

**Il banco** (`harness.c`, 193 righe, letto per intero in r. 63-123): `prova()`
carica `snap_<ENTRY>_<tag>_prima.bin/_dopo.bin` (c000-ffff) e le righe `prima` /
`dopo` di `_regs.txt`, chiama `v->fn(&z)`, confronta RAM c000-e5ff (esclusi 0x40
byte sotto SP) e poi A, BC, DE, HL, IX, IY. `struct voce_routine`
(`routine.h` r. 8-12): `{ entry, fn, blocco_ini, blocco_fine }`, tabella
`tabella_routine[]` in `routine_tab.c` (317 righe, ordine di indirizzo). I file
`decomp/out/snap_*` **non** sono ignorati da git (`.gitignore` r. 51): le
catture si committano.

**Misure di oggi** (da `decomp/harness` e `decomp`, binari già compilati):
- `./harness --tutte` → exit 0; `grep -c '^cattura'` 1278; `grep -c '^OK'` 1278;
  `grep -c FALLITO` 0; `grep -c '^RIEPILOGO'` 307.
- `sh giro_completo.sh` → `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0
  divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`.
- `harness/sched_proto out/sched_log.txt | tail -1` → `spedizioni confrontate:
  106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`.
- `harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin
  2410 | tail -4` → `spedizioni eseguite: 64686, fino al frame 5499` / `NMI
  confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti:
  nello spedidore 0, a metà attività 885` / `prima divergenza a metà attività
  (attesa): frame 2415` / `nessuna divergenza nello spedidore`.
- `harness/gioca -n 5434 --muto --oracolo out/giroboot_oracolo.txt
  out/giroboot_disp.txt --riproduci out/ingressi_replay.txt | tail -6` → `FRAME_VIDEO
  5499: hash C a239d0a940bec0b9 MAME a239d0a940bec0b9 tabella a239d0a940bec0b9,
  coincide` / `FRAME rese 5434` / `HASH ultimo a239d0a940bec0b9` / `FRAME ultimo
  5499` / `AUDIO campioni 4042500` / `AUDIO hash c4b40fea06623a96`.

**Il Makefile** (`harness/Makefile`): i corpi entrano in banco, replay, giro e
`gioca` tramite `SORGENTI_NEMICI = $(wildcard nemici*.c) attivita.c
attivita_01_10.c attivita_attract.c attivita_attract07*.c attivita_attract08.c`
(r. 6): ogni file nuovo va aggiunto lì. `sched_proto` non li usa.

### Censimento dei 21 ingressi

Righe di istruzioni raggiungibili dall'ingresso seguendo salti e `call` verso
routine NON tradotte (script di flusso sul disassemblato `decomp/out/main64.dasm`;
stima, non misura sulla traduzione; i passi dei corpi a passi sono contati con
le voci della tabella `rst 20`, che il generatore dichiara «statico» ma che sono
contigue e terminano dove inizia il codice del passo seguente). «Dopo» = byte
dello slot dopo il corpo, **misurato** nelle catture sotto (stato att: vedi
«Catture»). «Poste da» = `ld a,XX; call 0493` trovati nel disassemblato (non
significa che altri non esistano).

| cod | ingresso | tipo | istr. | routine NON tradotte | dopo (slot) | poste da / note |
|---|---|---|---:|---|---|---|
| 04 | 4C30 | semplice | 10 | - | 03 | `pop hl; ret c` se c463 < 0A, altrimenti c46e=41, slot 03; vista in sessioni stanza_29/30 |
| 05 | 4C42 | semplice | 100 | 4C89 | 09 | slot 09 a 4C48; poi 4AF6, 24BE, 4C11, 4D05 (tradotte); 4C89 solo se (c1aa,c1ac) ≠ 0; la 1D porta qui |
| 09 | 4D29 | semplice | 8 | - | 00 | stanza_06, oggetti |
| 0F | 640E | semplice | 13 | - | 03 | chiama 349F e 4B98 (tradotte) |
| 12 | 426A | semplice | 82 | - | 00 | posta da NMI a 0306-0308; chiama 0493, 4319, 4AF6, `rst 28` e 4302 (**legge R**, vedi sotto) |
| 13 | 6480 | a passi c19d | 99 | 8 passi: 64B0 6676 66D4 66F3 64B8 6702 64B8 670F | resta 13 | posta da NMI a 03DB; i passi postano 14, 16, 17 |
| 17 | 6429 | semplice | 33 | - | 07 | chiama 2D8A (tradotta); la posta 13 (672F) |
| 18 | 4124 | a passi **c19e** | 164 | 6735 e 10 passi (tabella 4171): 41B7 41CF 41A3 419D 41A8 4185 2E79 421E 41AD 4247; 41A8 è raggiunta anche per salto dagli altri passi | resta 18 | posta a 1345; passo 0 (c19e=0) a 4143; posta 0A a 4131 |
| 19 | 3792 | semplice | 20 | - | 00 | chiama 37B4 e 7379 (tradotte), `rst 18/28/8`; poste a 13A1, 13B0 |
| 1A | 411D | semplice | 4 | - | 1B | `call 354B; slot 1B`. Postata dal corpo 01 (4924: `tabella[slot] = A` quando 49AE dà A=1A, cella 5-7) |
| 1B | 4111 | semplice | 7 | - | 02 | attesa c463 ≥ 12, poi slot 02 |
| 1D | 3F39 | semplice | 4 | - | 05 | `call 349F; slot 05` |
| 23 | 3E4E | semplice | 24 | - | resta 23 | poste a 366F, 42CD, 7B3F; chiama 3569 (tradotta) e `rst 8` |

Nota su 18: 2E79 compare come voce anche nelle tabelle di 26 e 25 (non conteggiata nelle 164 istruzioni); ha una
sola implementazione da condividere (`static` in un modulo, o in comune se i
lotti 53 la riusano: dichiararla in `attivita_libera.h`).

Gli altri 8 (non in questo lotto): 1C 3F40 (a passi, 246 istr.), 25 3B77 (288), 26
3B63 (84), 22 12D6 (23 istr. + 12F0 con `rst 38` a 1309 di 33 voci, ~300 totali);
15 6466 (232), 16 6496 (288), 24 2C72 (19 passi, 510), 27 62B6 (114).

### Catture dei corpi: provate dall'analista

Provate in una copia di lavoro (`decomp/` ridotto a `cattura.sh` + `snap.lua`
modificato + `roms` collegato) con questa modifica a `snap.lua`, che sostituisce
le righe 169-170:
```lua
    local d = os.getenv('SNAP_CORPO') and 2 or 0
    ritorno = mem:read_u16(sp + d)
    sp_ritorno = (sp + d + 2) & 0xffff
```
(stampa di controllo facoltativa: `CORPO slot=%04X ritorno=%04X`). Comando, da
`decomp/`, uno per corpo (stato `attNN` di `cattura.sh`: poke c19d=c19e=0 al
frame 2250, posta del codice NN nel primo slot libero al 2300; ~2 secondi
ciascuno, 6 in parallelo con `xargs -P6` in 9 secondi totali):
```
SNAP_CORPO=1 sh cattura.sh 0x411D att1A 1 base
```
Risultato misurato (20 su 21 ingressi, tutti `frame_da=2300 OK`, catturati al
frame 2302, `scartate 0`): i 13 corpi di questo lotto più 1C 22 24 25 26, 15 16.
Il 27 (62B6) fallisce: il passo 0 (62E0, 4096 byte azzerati) dura più di un
frame, ogni occorrenza è contaminata dalla NMI e viene scartata (vedi lotto 54).

Variazioni provate (stesso comando con `SNAP_POKE` che si aggiunge ai predefiniti:
passare `SNAP_POKE=2250:0xc19d:0,2250:0xc19e:0,<altri>`):
- passo k di un corpo a passi: poke del contatore al 2300, ad es. `2300:0xc19d:K`
  (c19e per il 18). Provati e catturati: 1C K=4, 26 K=2, 24 K=5, 25 K=3, 15 K=5,
  18 K=5 (`prima` ha c19e=5, byte 0x19E di `_prima.bin`). Il passo catturato è il
  valore del contatore in `_prima.bin` (offset 0x19D/0x19E), **non** sempre K: se
  la prima occorrenza è contaminata dalla NMI (`scartate 1`, caso 1C K=4: prima
  c19d=5) la cattura è del passo dopo. Il passo reale va quindi letto da
  `_prima.bin` e scritto nel tag (`k5`), con `od -An -tx1 -j$((0x19d)) -N1`.
  1C K=8 fallisce (limite di 6000 frame): nessun passo da catturare lì in questo lotto.
- 05 con 4C89: `SNAP_POKE=2250:0xc19d:0,2250:0xc19e:0,2300:0xc1aa:0x01` →
  `SNAP_CORPO=1 sh cattura.sh 0x4C42 att05 1 c1aa` OK; la stessa senza
  `SNAP_CORPO` su `0x4C89` OK (prima `PC=4C89 AF=0100 BC=0000 DE=FE00 HL=0001
  IX=C88E`, dopo `PC=4C85`). Senza il poke di c1aa 4C89 non viene chiamata.
- Routine non-corpo catturate senza `SNAP_CORPO` (ritorno normale): 6735 con
  `att18` OK.

**Ricerca con ingressi casuali (misurata)**: `gioca -n N --muto --riproduci F`
esegue 2500 frame in 3,7 s (-O0) e ferma il gioco libero a «FERMO corpo XX». Con
ingressi come quelli registrati (moneta 1511-1530 e600/e602 come da
`out/ingressi_replay.txt`, poi un valore a caso fra 01 02 04 08 10 20 in e600
ogni 8 o 4 frame dal 1900), su 130 semi fermano il gioco: corpo 04 (semi 6, 10,
30, 37, 49, 64, 67: frame 5141, 3733, 4213, 2051, 4717, 2381, 4877) e corpo 13
(semi 4 e 124: 5475, 4812). **Nessuna sosta sul 1A**. Il generatore (da salvare
in `decomp/ricette/genera_ingressi.py`, argomenti `seme file fine passo`):
```python
import random,sys
seed=int(sys.argv[1]); out=sys.argv[2]; fine=int(sys.argv[3]); passo=int(sys.argv[4]) if len(sys.argv)>4 else 8
r=random.Random(seed); v=[0x01,0x02,0x04,0x08,0x10,0x20]
cur=0
with open(out,'w') as f:
    for fr in range(66,fine):
        e602=0
        if 1511<=fr<1531: e602=4
        if 1561<=fr<1581: e602=1
        if fr>=1900 and (fr-1900)%passo==0: cur=r.choice(v)
        e600=cur if fr>=1900 else 0
        f.write("%d %02X 00 %02X\n"%(fr,e600,e602))
```
Riproduzioni utili come controllo di regressione dopo la traduzione: seme 10,
passo 8, 6000 frame: «FERMO corpo 04 non tradotto al frame 3733» **prima**;
dopo il lotto il gioco deve passare oltre il frame 3733 (il corpo 04 non è più
l'arresto). Idem seme 4: corpo 13 al 5475.

**Il registro R**: il corpo 12 arriva a `sub_4302` (z80_4xxx.c r. 517), che legge R
due volte con `nemici_r_somma_r1()` (nemici_r.c); fuori dal banco il gioco libero
chiama `r_letto_libero()` (`macchina_libera.c` r. 79), che si ferma con
«lettura del registro R non modellata». Il corpo 12 si traduce comunque (non
riscrive 4302), ma nel gioco libero il percorso 12 → 4302 si ferma lì finché R
non è modellato (domanda aperta 3).

## Spezzatura (3 lotti)

Il totale è ~2600 istruzioni Z80 (~2900 righe C a 1,1 righe per istruzione, metro
dei lotti N): più di due lotti da 1200 righe. Proposta, in ordine di quel che
serve al gioco libero (i corpi che il gioco spedisce quando si gioca: 04 e 13
sono già comparsi sopra con ingressi casuali; 12 e 13 sono postati dalla NMI
nella routine 028A-0308/03DB, che gira a ogni frame di gioco):
- **52 (questo)**: infrastruttura + 04 05 09 0F 12 13 17 18 19 1A 1B 1D 23: ~570
  istruzioni, ~650 righe C.
- **53**: 1C (porta), 25, 26, 22 (con 12F0 e il suo `rst 38`): ~920 istruzioni.
- **54**: 15, 16, 24 (finale, 19 passi), 27 (si ferma sul passo 0: verifica per
  composizione): ~1140 istruzioni.

## Struttura da realizzare

### 1. Il banco dei corpi (infrastruttura)

a. `decomp/snap.lua`: la modifica sopra (`SNAP_CORPO`), più la riga del
   commento di testa che la descrive. Nessun cambiamento senza la variabile.
b. `harness/routine.h`: nuovo ultimo campo in `struct voce_routine`:
   `uint8_t solo_ram;` (r. 11 → aggiungere dopo `blocco_fine`). Le 300 voci
   esistenti restano valide (i membri finali omessi valgono 0).
c. `harness/harness.c`: in `prova()` (r. 112-120) se `v->solo_ram` non si
   confrontano A, BC, DE, HL, IX, IY (i registri di uscita di un corpo non
   hanno senso per lo spedidore): stampare `registri non confrontati (corpo)` al
   posto della riga `registri C:`. Il confronto della RAM, delle pile e del
   blocco resta identico. Il file resta sotto le 200 righe.
d. `harness/banco_corpi.c` (nuovo, ~60 righe): un wrapper per corpo con una
   macro, ad es.
   ```c
   #define BANCO_CORPO(sfx, fn) void banco_corpo_##sfx(struct z80 *z) \
       { fn((uint8_t)((M(z->sp) | (M(z->sp + 1) << 8)) - 0xc000)); }
   ```
   (SP di ingresso, `[SP]` = C000+slot; il corpo C non tocca z). Prototipi in
   `attivita_libera.h`. Aggiungere il file a `SORGENTI_NEMICI` del Makefile.
e. `harness/routine_tab.c`: le voci nuove, in ordine di indirizzo, es. `{
   "0x411D", banco_corpo_411d, 0, 0, 1 },`. Le voci dei passi non esistono: il
   passo si cattura dall'ingresso del corpo con il contatore forzato.
f. `decomp/cattura_corpi.sh` (nuovo, eseguibile con `sh`): legge
   `decomp/corpi_catture.txt` (una riga per cattura: `modo cod entry tag N
   poke`, modo = `corpo` o `sub`, `poke` = elenco `frame:addr:val` che si
   aggiunge a `2250:0xc19d:0,2250:0xc19e:0`, o `-`), lancia 6 catture in
   parallelo (`xargs -P6`), ognuna `SNAP_CORPO=1` se modo `corpo`, con
   `sh cattura.sh <entry> att<cod> <N> <tag>`, e stampa alla fine `CATTURE OK n
   su m`. Rieseguibile: cancella e rifà `snap_<entry>_att<cod>_<tag>_*` per le
   sole righe dell'elenco. Prima riga di esempio: `corpo 05 0x4C42 c1aa 1
   2300:0xc1aa:0x01`.

### 2. I corpi in C

Header nuovo `harness/attivita_libera.h` (prototipi dei corpi del lotto, di
`sub_4c89`, `sub_6735`, dei wrapper `banco_corpo_*`; includere `sched.h` per
`attivita_fn`). Moduli nuovi (nomi per codice, nessuna interpretazione del ruolo):
- `harness/attivita_piccoli.c` (~300 righe): 04, 05 (con `sub_4c89`), 09, 0F, 17,
  19, 1A, 1B, 1D, 23.
- `harness/attivita_12_13.c` (~250 righe): 12 (4302 e 4319 già tradotte, in
  z80_4xxx.c; R tramite 4302), 13 a passi: `switch` sul byte c19d letto come
  fa `attivita_attract07.c` per il 07 (modello: contatore in RAM, un passo per
  chiamata, `dec (hl)` per restare sullo stesso passo; vedi r. 1-60 di quel file).
- `harness/attivita_18.c` (~220 righe): 18 a passi con contatore **c19e**, `sub_6735`
  e i passi 41B7 41CF 41A3 419D 41A8 4185 2E79 421E 41AD 4247 (nessuno è già
  tradotto: `sub_41a8` e `sub_2e79` non esistono in harness/*.c; la dimensione di
  2E79, non misurata, si legge dal disassemblato); `2E79` come funzione condivisa dichiarata nell'header.
- `harness/attivita.c`: solo le righe nuove nella tabella `corpo_attivita_replay`
  (r. 351-368) e `#include "attivita_libera.h"`. Il commento di testa (r. 1-17)
  si aggiorna. Non superare le 400 righe.
Tradurre dal disassemblato `decomp/out/main64.dasm` (`awk '/;4c30[ \t]*$/,/;4c41[ \t]*$/'
...`); i passi con `ld a,r` sono da non toccare finché non si decide (domanda 3).

### 3. Documenti

`decomp/SCHEDULER.md` (tabella 3B11, r. 64-102: cambiare «non osservate» in
«tradotta (lotto 52)» per i 13 corpi e annotare «banco: SNAP_CORPO»);
`decomp/GIRO.md` (una sezione breve: ricetta 1A e come si riesegue);
`STATO.md` (spunta del lotto 52 e riga dei lotti 53-54).

### 4. Ricetta del corpo 1A

Obiettivo: un file di ingressi `decomp/ricette/chiave_1a.txt` (stesso formato di
`out/ingressi_replay.txt`: `frame e600 e601 e602`) e uno script
`decomp/ricette/chiave_1a.sh` che lo rigenera, tali che:
- **prima** del lotto `harness/gioca -n 4000 --muto --riproduci chiave_1a.txt`
  stampa «FERMO corpo 1A non tradotto al frame N»;
- **dopo** il lotto la stessa riga non compare, l'uscita contiene `FRAME rese
  4000` e il gioco ha superato il frame N.
Ordine di lavoro: (1) **prima di scrivere codice** trovare la ricetta contro il
binario attuale e salvarla; (2) tradurre; (3) rieseguire. Il 1A scatta quando il
corpo 01 (`attivita_01_48d1`, attivita_01_10.c r. 18-50) riceve A=1A da `sub_49ae`
(cella davanti a Dana di valore 5-7, nemici_comuni.c r. 292-305, ramo 4AB9-4AC3).
Strategia suggerita, in ordine: (a) estendere la ricerca per semi con
`genera_ingressi.py` (130 semi non l'hanno trovato; usare passi 2, 3 e 16, più
semi, e `-n 3000`); (b) ricerca guidata: dal binario, con `--hash-frame` o un
piccolo programma C di ricerca che usa `macchina_libera`, individuare la stanza
iniziale e la cella (valore 5-7 in `ld a,(hl)` a 48FC) e comporre a mano i
movimenti di Dana e il tasto BUTTON1 (e600 bit 0x20). Se dopo un impegno
ragionevole (~50 prove manuali o ~1000 semi) non si trova, usare il ripiego
della domanda 5 e dirlo nel rapporto.

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master` salvo dove scritto. Uscita esatta
salvo `>=` o «attesa, non misurata».

1. Compilazione: `cd decomp/harness && make harness replay gioco_libero gioca sched_proto >
   /tmp/m52.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/m52.txt` →
   `make 0` e `0`. (Senza `sdl2-config` il bersaglio `gioca` non c'è: allora
   togliere `gioca` dalla riga e segnalarlo.)
2. Catture: `cd decomp && sh cattura_corpi.sh | tail -1` → `CATTURE OK n su n`
   (n = numero di righe di `corpi_catture.txt`, ≥ 13 corpi + 4C89 + 6735 + i
   passi di 13 e 18 che l'analista non ha ancora tutti provato: attesa, non misurata).
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h52.txt; echo "exit $?";
   grep -c FALLITO /tmp/h52.txt; grep -c '^RIEPILOGO' /tmp/h52.txt` → `exit 0`, `0`,
   `322` (307 + 13 corpi + 4C89 + 6735; attesa, dedotta). `grep -c '^cattura'` ≥ 1278
   + n; `grep -c '^OK'` uguale a `grep -c '^cattura'`.
4. Una riga per voce: `for a in 4C30 4C42 4D29 640E 426A 6480 6429 4124 3792 411D
   4111 3F39 3E4E 4C89 6735; do grep "^RIEPILOGO 0x$a:" /tmp/h52.txt; done` →
   15 righe `RIEPILOGO 0x…: K catture, K OK` con K ≥ 1 (≥ 2 per 6480 e 4124).
5. Registri non confrontati solo per i corpi: `grep -c 'registri non confrontati'
   /tmp/h52.txt` = numero di catture dei 13 corpi (attesa, non misurata);
   le vecchie voci invariate: `for a in 4B98 4302 4319; do grep "^RIEPILOGO 0x$a:"
   /tmp/h52.txt; done` → uguali a quelle di prima del lotto (salvarle prima di iniziare).
6. Invarianti: `cd decomp && sh giro_completo.sh | tail -1` → `GIRO COMPLETO 0-5499:
   saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame
   resi con hash coincidenti`; `harness/sched_proto out/sched_log.txt | tail -1`,
   `harness/replay …` e `harness/gioca -n 5434 … --oracolo …` esattamente come
   nelle misure di oggi (stesse sei righe, `AUDIO hash c4b40fea06623a96`).
7. Ricetta 1A: i due controlli di «Ricetta del corpo 1A», prima e dopo, con il
   frame N e le uscite incollate nel rapporto.
8. Regressione casuale: `python3 decomp/ricette/genera_ingressi.py 10 /tmp/s10.txt 6000
   8; harness/gioca -n 6000 --muto --riproduci /tmp/s10.txt | grep -a FERMO`
   → prima: `FERMO corpo 04 non tradotto al frame 3733`; dopo: nessuna riga
   per 04 (può comparire un'altra sosta, da riportare nel rapporto).
9. Dimensioni: `find decomp/harness -name '*.c' -o -name '*.h' | xargs wc -l | awk
   '$1>800 && $2!="totale"'` → vuota (nessun file nuovo oltre 650: controllare a
   mano `attivita_piccoli.c`, `attivita_12_13.c`, `attivita_18.c`).
10. Ambito: `git status --short` mostra solo file di `decomp/`, `compiti/52-…`,
    `STATO.md`; `pgrep -c mame` → `0`.

## Consegna

Commit unico (o due: infrastruttura+corpi, ricetta+documenti) con messaggio in
italiano «Lotto 52: banco dei corpi, 13 corpi di attività, ricetta 1A» e riga
`Co-Authored-By: <modello> <noreply@anthropic.com>`. Includere le catture
`decomp/out/snap_*` nuove (non ignorate). Aggiungere in fondo a questo file la
sezione «Esito» con le uscite delle verifiche. Rapporto finale: massimo
trenta righe con l'uscita di 1-10, il frame N della ricetta 1A e le deviazioni.

## Domande aperte

1. **Tre lotti invece di due** (52: infrastruttura + 13 corpi; 53: 1C 22 25 26;
   54: 15 16 24 27). Il totale stimato (~2900 righe C) supera due lotti da 1200.
   Proposta: tre, nell'ordine dato. **Scelta di progetto** (perimetro).
2. **Banco dei corpi**: campo `solo_ram` in `struct voce_routine`, `SNAP_CORPO`
   in `snap.lua` e wrapper `banco_corpo_*`: i registri di uscita di un corpo
   non si confrontano, si confronta la RAM (incluso il byte dello slot).
   Alternativa scartata: tradurre i corpi con firma `struct z80 *` (rompe la
   convenzione dei 16 già fatti). **Scelta di progetto** (cambia l'interfaccia
   del banco).
3. **Registro R nel gioco libero** (corpo 12 → 4302). Proposta: non modellarlo in
   questo lotto; il gioco si ferma da solo sulla sentinella «R non modellata»
   quando il corpo 12 lo raggiunge, e il lotto lo riporta. Modello futuro
   (contatore deterministico non fedele a MAME, escluso dal confronto con
   l'oracolo) da decidere a parte. **Scelta di progetto** (semantica).
4. **Passo 0 del 27 (62E0)** non catturabile (NMI sempre dentro la routine):
   proposta per il lotto 54, tradurlo verificando 62BE e 6379 (catturato dai
   brief 40 con `att27` e poke c1ae) e il resto per composizione di routine
   tradotte. **Preferenza**.
5. **Ripiego per la ricetta 1A** se la ricerca non la trova: scenario forzato in
   C+MAME (post del codice 1A al frame 2300 come `att1A`, e confronto della RAM
   dei frame 2302-2500 fra `gioca` con un'opzione di debug `--posta F:COD` e un
   `snap`/dump di MAME). Non è la «magia sulla chiave» vera ma prova che il
   gioco prosegue dopo 1A → 1B → 02. **Scelta di progetto** (cambia la forma
   della verifica richiesta).
6. Nomi dei moduli (`attivita_piccoli.c`, `attivita_12_13.c`, `attivita_18.c`),
   dell'header (`attivita_libera.h`), cartella `decomp/ricette/` e nome
   `corpi_catture.txt`. **Preferenza**.
7. La tabella `corpo_attivita_replay` serve anche al replay (r. 350): le voci nuove
   non cambiano il replay (nessun nuovo codice spedito nei 5500 frame, verifica 6).
   Proposta: restare con una tabella sola. **Preferenza**.

## Risoluzioni delle domande aperte (chat principale con l'utente, 2026-11-04)

1. Spezzatura in TRE lotti (52/53/54): APPROVATA dall'utente.
2. Banco dei corpi (SNAP_CORPO in snap.lua, campo solo_ram, wrapper
   banco_corpo_*): APPROVATO dall'utente.
3. Registro R nel gioco libero: RIMANDATO a un lotto dedicato per decisione
   utente; il corpo 12 si traduce ma il gioco libero continuerà a fermarsi
   su di esso finché R non è modellato (annotarlo nei documenti).
4. Passo 0 del corpo 27 verificato per composizione nel lotto 54:
   preferenza ACCETTATA con la proposta.
5. Ricetta 1A: l'UTENTE REGISTRA una partita vera con `--registra`
   usando la magia sulla chiave; la registrazione (percorso annotato qui
   sotto appena fatta) diventa la regressione ufficiale del 1A — il gioco
   in riproduzione non deve più fermarsi al frame della magia. Lo scenario
   forzato via poke resta come ripiego solo se la registrazione non
   riproduce lo stop.
6. Nomi di moduli, header, cartella ricette/ ed elenco catture:
   preferenze ACCETTATE con la proposta.
7. Una sola tabella corpo_attivita_replay: preferenza ACCETTATA.

## Ricetta 1A registrata (chat principale, 2026-11-04)

L'utente ha registrato la partita: `decomp/ricette/partita_1a.reg` (44588
byte). Il gioco si ferma usando la magia sulla chiave. Riproduzione
verificata deterministica:
`decomp/harness/gioca --riproduci decomp/ricette/partita_1a.reg -n 4000 --muto`
→ `FERMO corpo 1A non tradotto al frame 3320`, `FRAME rese 3254`,
`HASH ultimo f6497cab72bf0df8`, `AUDIO campioni 2440200`,
`AUDIO hash fc7492a1ce608f54`, uscita 3.
VERIFICA FINALE DEL LOTTO: lo stesso comando, a corpi tradotti, NON deve
più fermarsi al frame 3320 (deve superarlo o finire i 4000 frame), uscita 0.

FATTO OSSERVATO durante la stessa partita (fuori perimetro del lotto 52,
da NON affrontare qui, va al lotto che copre il corpo 22/zona 12xx):
il log stampa molte volte «1288: continuazione 12AB (raccolta) non
tradotta: nessun effetto» — la raccolta di certi oggetti nel gioco libero
oggi non ha effetto. Annotarlo nei documenti di stato se non già noto.

## Esito (sviluppatore, 2026-11-04)

1. Compilazione: `make 0`, `0` avvisi/errori (con `gioca`, SDL presente).
2. Catture: `CATTURE OK 32 su 32` (13 corpi, 4C89, 6735, passi 0-6 del 13,
   passi 0-9 e ramo >= 0A del 18; contatore di `_prima.bin` = K in tutti).
3. Banco: `exit 0`, `0` FALLITO, `322` RIEPILOGO, 1310 cattura, 1310 OK.
4. 15 righe `RIEPILOGO` tutte OK: 4C30 1, 4C42 2, 4D29 1, 640E 1, 426A 1,
   6480 7, 6429 1, 4124 11, 3792 1, 411D 1, 4111 1, 3F39 1, 3E4E 1, 4C89 1, 6735 1.
5. `registri non confrontati` 30 (= catture dei 13 corpi). 4302 «3 catture,
   3 OK» e 4319 «9 catture, 9 OK» come prima; 4B98 non ha voce nel banco
   (nessuna riga, né prima né dopo).
6. Invarianti identici alle misure del brief (giro, sched_proto, replay, le sei
   righe di `gioca --oracolo` con `AUDIO hash c4b40fea06623a96`).
7. Ricetta 1A (`ricette/partita_1a.reg`): prima `FERMO corpo 1A non tradotto
   al frame 3320`, uscita 3; dopo `FRAME rese 4000`, `HASH ultimo
   3f25ba053eeb56b9`, `AUDIO hash 79f9a550028a41b9`, uscita 0.
8. Seme 10: prima `FERMO corpo 04 non tradotto al frame 3733`, dopo `FRAME
   rese 6000` senza soste. Seme 4: prima corpo 13 al 5475, dopo `FERMO
   routine 352E mancante al frame 5602`; su 12 semi provati solo 4 e 67 si
   fermano, entrambi su 352E.
9. Nessun file sopra 800 righe; moduli nuovi 265, 202, 211 righe.
10. `git status` solo `decomp/`, `compiti/52-…`, `STATO.md`; `pgrep -c mame` 0.

Deviazioni: il passo 7 del 13 (670F) non si cattura (NMI in ogni occorrenza)
ed è tolto dall'elenco; per le voci `solo_ram` la pila libera include anche la
parola dello slot in [SP] (le call dopo il `pop hl` la riscrivono in MAME);
`routine_tab.c` ha il quinto campo esplicito in tutte le voci (senza, `-Wextra`
dava 1236 avvisi `missing-field-initializers`); `chiave_1a.txt/.sh` non
prodotti, sostituiti da `partita_1a.reg` come da risoluzione 5.
