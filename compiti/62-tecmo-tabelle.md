# Compito 62: tabelle Tecmo in `tecmo/` e `gioca` senza `main64.bin`

Data: 2026-11-06. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Primo di tre lotti verso «il binario non legge più nessuna ROM». Creare la
cartella `tecmo/` alla radice del repository e portarci, come tabelle C con
nome (non un'immagine unica), tutti i dati del Z80 principale che il gioco
legge a runtime: le 87 voci «letto dalla ROM» più `tab_8089` e il riempimento
F997-FFFF (89 voci, 19384 byte), con estrattore rieseguibile e round-trip
byte per byte. A fine lotto `gioca` non apre più `out/main64.bin`:
`carica_rom()` ricompone `rom[]` dalle tabelle.

Vincoli assoluti:
- Nessuna modifica fuori da `tecmo/` (nuova), `decomp/harness/`,
  `decomp/estrai_rom.py` (nuovo), `decomp/censimento_rom.py`,
  `decomp/CENSIMENTO-ROM.md`, `decomp/DATI.md`, `STATO.md`, questo brief.
- Non toccare: `decomp/build_rom.sh`, il bersaglio `rom` del Makefile,
  l'audio (`1.3jk`, `audio_*.c`: è il lotto 63), `README.md` e `NOTICE.md`
  principali (lotto 64), `roms/`, le ricette in `decomp/ricette/`.
- Commenti, messaggi e documenti in italiano; **solo `tecmo/README.md` in
  inglese** (testo sotto).
- Nessun file di CODICE sopra le 800 righe. I file di soli dati generati
  seguono la convenzione di `video_dati_*.c` (comunque tutti sotto 800,
  stime sotto).
- Niente push. Build: `make -C decomp/harness` (gcc e SDL2 già installati);
  i comandi di gioco con `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy`, da
  `decomp/`.

## Cosa esiste già (fatti verificati)

Censimento (brief 61, commit c794308): `decomp/CENSIMENTO-ROM.md` e
`decomp/censimento_rom.py` — leggerli entrambi, sono la specifica di questo
lotto. Fatti chiave (da quei file e da grep/grafo):

- `VOCI` in `censimento_rom.py:45`: tuple `(spazio, nome, inizio, fine,
  stato, lettori, formato, estensione)`; `E, L = 'estratto', 'letto dalla
  ROM'`; spazio `'P'` principale, `'A'` audio; `fine` **inclusivo**.
  Z80 principale: 87 voci L (17739 byte) + 1 voce E (`tab_8089`, 4 byte).
  Le 2 voci `'A'` sono del lotto 63: **non estrarle**.
- `file_generato(v)` in `censimento_rom.py:463`: assegna ogni voce al file
  `rom_dati_{fondali,testi,oggetti,stanze,varie}.c`. Lo script ha la guardia
  `if __name__ == '__main__'`, quindi è importabile; con `python3 -I` il
  nuovo script deve fare `sys.path.insert(0, QUI)` prima di
  `from censimento_rom import VOCI, L, file_generato`.
- Corrispondenza indirizzo → file ROM (`build_rom.sh`, 19 righe):
  0000-3FFF = `6.3f[a]`; 4000-7FFF = `7.3h[a]` (seconda metà);
  8000-BFFF = `7.3h[a-0x8000]`; F000-FFFF = `8.3jk[a-0xF000]`. Nessuna voce
  attraversa un confine. ROM in `roms/ext/solomon/` (presenti, 12 file).
  `md5sum out/main64.bin` = `0bca296adf15411c009cf0d13718400d` (misurato).
- Modello d'estrattore: `decomp/estrai_grafica.py` (88 righe, da copiare
  nella forma): intestazione «Generato da … non modificare», 24 byte per
  riga `0x%02X,`, round-trip con rilettura del testo prodotto (uscita 2 se
  differisce), riscrittura solo se il contenuto cambia.
- `carica_rom()` (`harness/comune.c:37`, da grafo): apre `out/main64.bin`
  da `out/`, `../out/` o `<exe>/../out/` e riempie `rom[0x10000]`; uscita 2
  se manca. Chiamanti: `harness.c:186`, `replay.c:353`, `macchina.c:610`,
  `macchina_libera.c:136`.
- `leggi()` (`harness/ram.h:16`, testo esatto):
  `static inline uint8_t leggi(uint16_t a) { return a >= 0xc000 ? ram[a - 0xc000] : rom[a]; }`
- Usi diretti di `rom[` fuori da `leggi()` (da grep, lista completa): la
  definizione (`comune.c:11`) e i due memcpy della finestra F000:
  `macchina.c:613` e `macchina_libera.c:138`
  (`memcpy(&ram[0xf000 - 0xc000], &rom[0xf000], 0x1000);`). **Non toccarli**:
  con le voci F000-F996 più il riempimento F997-FFFF la finestra ricomposta
  è identica byte per byte a `main64.bin`.
- `rom[]` non è mai indicizzato in C000-EFFF (zeri in `main64.bin`):
  `leggi()` serve tutto ≥ C000 da `ram[]`.
- Trappola res 6 (`harness/nmi_oggetti.c:232-237`): il ciclo scrive
  `M(p) &= ~0x40` per `p >= 0xc000`; con `D != 0` arriva nella copia di
  F000-FFFF in `ram[]`, dove su scheda la scrittura si perderebbe (misurati
  F7CE F7DE F7EE F7FE F8FE FDEE, bit 6 già 0: nessun effetto visto).
- `tab8089[4] = { 0x04, 0x08, 0x08, 0x0c }` è una copia a mano in
  `nemici_tipi_15_17.c:369`; per `DE >= 4` la riga 386 fa `leggi(HL)` su
  808D+ (codice, mai misurato).
- `--verifica` di `censimento_rom.py` classifica i siti `leggi(` **per file
  e numero di riga** (dizionario `SITI`): se una modifica sposta le righe di
  un file di `harness/*.c` che contiene `leggi(`, `SITI` va aggiornato.
- Makefile (`harness/Makefile`, 113 righe): `SORGENTI_BASE` riga 3 (contiene
  `comune.c`), inclusa in `SORGENTI` e quindi in harness, replay,
  gioco_libero e gioca; `sched_proto` e gli audio non usano `rom[]` né
  `leggi()` (da grep). Modello per la prova d'immagine:
  `video_immagine.c` (39 righe) e il bersaglio alle righe 66-67.
- Nessun file `harness/*.c` supera le 800 righe (misurato);
  `comune.c` 57 righe, `ram.h` 89, `nmi_oggetti.c` 414, `Makefile` 113.
- `e603.h` non è ROM (valore della protezione da MAME): resta com'è.
  `rst20_tables.h`/`rst38_tables.h` restano com'è (tabelle di destinazioni).

## Struttura da realizzare

### 1. `decomp/estrai_rom.py` (nuovo, ~150 righe)

Sul modello di `estrai_grafica.py`. Legge i file ROM da
`../roms/ext/solomon/` (stessa ricerca di cartelle), **non** `main64.bin`.
Importa `VOCI, L, file_generato` da `censimento_rom.py` (con
`sys.path.insert`). Elenco da estrarre: le voci con spazio `'P'` (87 L più
`tab_8089` che è `E`), più una voce locale aggiuntiva
`('P', 'riempimento_f997', 0xF997, 0xFFFF, …)` (serve ai memcpy della
finestra F000; non va in `VOCI`). Controllo di coerenza: se le voci `'P'`
non sono 88 o i byte L non sono 17739, uscita 2 con messaggio.

Scrive in `../tecmo/` (crea la cartella):
- `rom_dati_varie.c` (44 voci, 4077 byte, ~300 righe), `rom_dati_testi.c`
  (12, 560, ~65), `rom_dati_stanze.c` (19, 3321, ~200),
  `rom_dati_oggetti.c` (8, 7330, ~340), `rom_dati_fondali.c` (6, 4096 —
  l'intera finestra F000-FFFF —, ~210). Assegnazione con `file_generato()`;
  `tab_8089` → varie, `riempimento_f997` → fondali. Ogni voce:
  `const uint8_t <nome>[<byte>] = { … };` con commento che riporta
  intervallo, ROM d'origine e «Generato da decomp/estrai_rom.py: non
  modificare a mano. Dati derivati dalla ROM Tecmo (tecmo/README.md,
  NOTICE.md)». Ogni file include `"rom_dati.h"`.
- `rom_dati.h`: guardia, `#include <stdint.h>`, un `extern const uint8_t
  <nome>[<byte>];` per voce, poi
  `typedef struct { uint16_t inizio; uint16_t lung; const uint8_t *dati; } RomVoce;`
  `extern const RomVoce rom_voci[];` `extern const unsigned rom_voci_n;`
- `rom_voci.c`: la tabella `rom_voci[]` (89 elementi, ordinati per
  indirizzo) e `rom_voci_n`.

Round-trip come in `estrai_grafica.py`: rilegge gli `0x..,` dal testo
prodotto e li confronta con i byte ROM, uscita 2 se differiscono; riscrive
un file solo se cambia. Riga finale (forma libera, ma con questi numeri):
`rom: 89 voci, 19384 byte, andata e ritorno ok`.

### 2. `tecmo/README.md` (in inglese, testo da copiare)

```markdown
# Tecmo data tables

The C tables in this directory are byte-for-byte data extracted from the
arcade ROM set of *Solomon's Key* (Tecmo, 1986) by
`decomp/estrai_rom.py`. Do not edit them by hand.

This content is **not** the property of the author of this repository:
graphics, palettes, levels, text and game data are copyright Tecmo
(today Koei Tecmo). It is included for study and preservation purposes
only. The source code of this project remains the work of its author
(see `NOTICE.md` in the repository root).
```

### 3. `carica_rom()` ricompone l'immagine (architettura decisa)

Variante scelta: **ricomposizione in `rom[]`**, non risoluzione in
`leggi()`. Motivo: `leggi()` ha 196 siti di chiamata che restano intatti, i
due memcpy di F000 restano intatti, il costo è un `memcpy` per voce al boot;
una risoluzione per indirizzo in `leggi()` toccherebbe il percorso più caldo
e tutti i binari per nessun vantaggio.

In `harness/comune.c`: nuovo corpo di `carica_rom()` (stesso nome, i 4
chiamanti non cambiano): `memset(rom, 0, sizeof rom);` poi per ogni voce
`memcpy(&rom[v->inizio], v->dati, v->lung)` e marcatura di
`rom_coperto[v->inizio … ]` per gli indirizzi < 0xC000. Niente più fopen.
Aggiungere `#include "../../tecmo/rom_dati.h"`, la definizione
`uint8_t rom_coperto[0xC000];` e la funzione sentinella:

```c
void rom_fuori_voce(uint16_t a)
{
    static uint8_t visti[0x10000 / 8];
    if (visti[a >> 3] & (1 << (a & 7))) return;
    visti[a >> 3] |= (uint8_t)(1 << (a & 7));
    fprintf(stderr, "AVVISO: lettura ROM fuori voce a %04X\n", a);
}
```

In `harness/ram.h`: `extern uint8_t rom_coperto[0xC000];`,
`void rom_fuori_voce(uint16_t a);` e `leggi()` diventa:

```c
static inline uint8_t leggi(uint16_t a)
{
    if (a >= 0xc000) return ram[a - 0xc000];
    if (!rom_coperto[a]) rom_fuori_voce(a);
    return rom[a];
}
```

(la sentinella avvisa e lascia leggere lo 0: il gioco non si ferma;
aggiornare il commento su `main64.bin` in `ram.h` e in testa a `comune.c`).

### 4. Trappola res 6 e procedure collegate

- `nmi_oggetti.c:235`: `if (p >= 0xc000)` → `if (p >= 0xc000 && p < 0xf000)`
  (su scheda la scrittura in F000-FFFF si perde; così il C smette di
  sporcare la copia dei fondali). Aggiornare il commento alle righe 232-234
  **senza cambiare il numero di righe del file** (ha 8 siti `leggi(` in
  `SITI`).
- Se una regressione sotto stampa «AVVISO: lettura ROM fuori voce»: chiudere
  l'indirizzo con il disassemblato (`decomp/disasm.sh`, `out/copertura.txt`),
  aggiungere la voce a `VOCI` in `censimento_rom.py` (con le altre colonne) e
  a `CENSIMENTO-ROM.md`, rilanciare `estrai_rom.py` e rifare le verifiche.
  È il meccanismo previsto: il banco (verifica 8) percorre rami mai misurati
  nel brief 61.

### 5. `harness/rom_immagine.c` (nuovo, ~45 righe) e Makefile

Sul modello di `video_immagine.c` ma al contrario: legge `out/main64.bin`
(o `../out/`), per ogni elemento di `rom_voci[]` confronta i byte, e
controlla che le voci F000-FFFF coprano l'intera finestra. Stampa
`rom_immagine: 89 voci, 19384 byte, coincide con out/main64.bin` e uscita 0;
uscita 2 con l'indirizzo al primo byte diverso.

Makefile: `TECMO = ../../tecmo`,
`SORGENTI_ROM = $(wildcard $(TECMO)/rom_dati_*.c) $(TECMO)/rom_voci.c`,
aggiunta di `$(SORGENTI_ROM)` a `SORGENTI_BASE` (riga 3) e bersaglio
`rom_immagine` (in `all` e nel `clean`); `$(TECMO)/rom_dati.h` fra le
dipendenze dei bersagli che già elencano `ram.h`. Il bersaglio `rom` e
`gioco: rom gioca` **restano** (a runtime serve ancora `1.3jk` per l'audio;
`main64.bin` ora serve solo a `rom_immagine`): aggiornare il solo commento
alla riga 92.

### 6. Documenti

- `CENSIMENTO-ROM.md`: sezione breve in coda «Esito del lotto 62» (cosa è
  stato estratto, voci eventualmente aggiunte).
- `DATI.md`: due righe in testa a «Strutture»: le tabelle sono in `tecmo/`,
  generate da `estrai_rom.py`, rimando a `CENSIMENTO-ROM.md`.
- `STATO.md`: una riga nei «Numeri» e spunta nei «Prossimi passi» se il
  lotto vi compare.

## Verifiche obbligatorie

Da `decomp/`, con `export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy`.
Le uscite 3-7 e 9-10 sono **misurate oggi sul binario attuale** e devono
restare identiche. In 3-8 non deve comparire nessun «AVVISO: lettura ROM
fuori voce» (se compare: procedura al punto 4 e verifiche da capo).

1. `python3 -I estrai_rom.py` due volte: la prima scrive e finisce con
   `… 89 voci, 19384 byte, andata e ritorno ok`; la seconda non cambia
   nessun file (`git status --short tecmo/` identico) — attesa, non misurata.
2. `make -C harness` senza errori né warning nuovi (`-Wall -Wextra`);
   `harness/rom_immagine` → `… coincide con out/main64.bin`, uscita 0 —
   attesa, non misurata.
3. `sh giro_completo.sh` → uscita 0 e, misurato:
   `SALDATURA 2410: byte diversi in c000-e5ff (escluse c040-c043, cf00-cfff): 0, coincide`
   `NMI confrontate: 5434, divergenti: 30`
   `dopo il boot: nello spedidore 2617 (divergenti 0), a metà attività 2280 (divergenti 30)`
   `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`
4. Le quattro ricette, misurate (`harness/gioca --riproduci ricette/<R>.reg
   -n <N> --muto`, uscita 0, righe `FRAME rese` / `HASH ultimo` / `AUDIO hash`):
   | ricetta | -n | HASH ultimo | AUDIO hash |
   |---|---|---|---|
   | bottiglia_rossa | 11167 | 69a54ac9aa6bea9e | b70dc75d81b355b9 |
   | partita_1a | 3919 | 40254b8858b57c6e | 09e3a1a90b2275f8 |
   | riferimento_nuova | 14525 | b9ab89317932188c | 902b2830066dba24 |
   | stanze_bonus | 12387 | bbebf0f3c64e446a | cb78ab354330a616 |
5. Modo oracolo, misurato: `harness/gioca -n 5434 --oracolo
   out/giroboot_oracolo.txt out/giroboot_disp.txt --riproduci
   out/ingressi_replay.txt --muto` → 13 righe `FRAME_VIDEO … coincide`,
   `FRAME rese 5434`, `HASH ultimo a239d0a940bec0b9`, `FRAME ultimo 5499`,
   `AUDIO campioni 4042500`, `AUDIO hash c4b40fea06623a96`.
6. Gioco libero, misurato: `harness/gioca -n 600 --muto` →
   `HASH ultimo ecd5acf694285489`, `AUDIO hash 8aedf41c0424df54`.
7. Banco, misurato: `harness/harness --tutte 2>&1 | awk '/^RIEPILOGO/
   {n+=$3; ok+=$5} END{print n" catture, "ok" OK"}'` → `1422 catture, 1422 OK`
   (più eventuali catture di voci aggiunte); `grep -c '^RIEPILOGO'` → 335.
8. Dieci semi (misurati nel brief 61): per S=1..10
   `python3 -I ricette/genera_ingressi.py S /tmp/ingr_S.txt 20000` poi
   `harness/gioca --riproduci /tmp/ingr_S.txt -n 20000 --muto` → uscita 0,
   nessun FERMO.
9. `gioca` non apre più `main64.bin`, misurabile solo dopo:
   `strace -f -e trace=openat harness/gioca -n 100 --muto 2>&1 | grep -c main64`
   → `0`; lo stesso comando con `grep -c 1.3jk` → ≥ 1 (l'audio resta,
   lotto 63) — attesa, non misurata.
10. `python3 -I censimento_rom.py --verifica` → ultima riga, misurata:
    `CENSIMENTO OK: 201 occorrenze, 0 non classificate` (aggiornare `SITI`
    se righe spostate; il numero può salire solo con voci aggiunte al punto 4).
11. Nessun file di codice sopra le 800 righe:
    `wc -l decomp/harness/*.c decomp/harness/*.h tecmo/*.c tecmo/*.h decomp/estrai_rom.py | awk '$1>800 && $2!="totale"'`
    → vuota (i `rom_dati_*.c` generati restano comunque sotto, stime al punto 1
    della struttura).
12. `git status --short` dalla radice: solo i file elencati in «Struttura» e
    «Documenti» (più `tecmo/` nuova); niente in `out/`, `roms/`, `ricette/`.

## Consegna

- Un commit sul ramo `main` con tutti i file del lotto, messaggio in
  italiano, per esempio «Tabelle Tecmo in tecmo/ e gioca senza main64.bin,
  brief 62», chiuso dalla riga
  `Co-Authored-By: <modello che esegue> <noreply@anthropic.com>`.
- Rapporto finale: massimo trenta righe con l'uscita delle verifiche 1-12,
  le voci eventualmente aggiunte a `VOCI` e ogni deviazione dal brief.

## Domande aperte

1. **Sentinella sempre attiva** in `leggi()` (un test su `rom_coperto` e
   avviso su stderr una volta per indirizzo) invece che dietro un flag di
   compilazione. Proposta: sempre attiva — costo trascurabile, e un dato
   fuori censimento si vede alla prima partita invece di dare byte 0 muti.
   «Preferenza».
2. **`tab8089` locale**: lasciare la copia a mano in
   `nemici_tipi_15_17.c:369` (la voce `tab_8089` è comunque estratta in
   `tecmo/` e ricomposta in `rom[]`); sostituirla sposterebbe le righe dei
   siti `leggi(` del file. Proposta: lasciarla. «Preferenza».
3. **`video_dati_*.c` e `audio_dati*.c` restano in `decomp/harness/`**: lo
   spostamento in `tecmo/` di grafica e musica, se si vuole, va nel lotto
   64 insieme a README/NOTICE. Proposta: non spostarli ora. «Preferenza».
4. **Bersaglio `rom` del Makefile invariato** (serve `1.3jk` all'audio e
   `main64.bin` a `rom_immagine`); la rimozione è del lotto 63. Proposta:
   solo aggiornare il commento. «Preferenza».

L'architettura (ricomposizione in `rom[]` dentro `carica_rom()` contro
risoluzione in `leggi()`) era delegata al brief ed è decisa e motivata al
punto 3 della struttura: non è una domanda aperta.

## Decisioni (chat principale, 2026-11-06)

Tutte e quattro le domande sono preferenze, accettate con le proposte:
1. Sentinella «fuori voce» sempre attiva.
2. La copia locale `tab8089` resta.
3. `video_dati_*`/`audio_dati*` NON si spostano in questo lotto — ma lo
   spostamento in `tecmo/` va fatto entro il lotto 63: il traguardo
   dell'utente è che tutto il materiale Tecmo stia in `tecmo/` prima della
   riscrittura di README/NOTICE (lotto 64).
4. Bersaglio `rom` del Makefile invariato, solo commento.
