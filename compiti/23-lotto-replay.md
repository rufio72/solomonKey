# Compito 23: lotto replay — oracolo per frame, giro di gioco in C, confronto

2026-10-22, scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Costruire il passo 4 del metodo (README, «Method»): una partita registrata in
MAME con impronta della RAM a ogni frame (oracolo), il giro di gioco intero in
C (NMI + spedidore + corpi delle attività) alimentato dagli stessi ingressi, e
il confronto frame per frame con rapporto riproducibile della prima divergenza.
Esito documentale in `decomp/REPLAY.md`: punto di divergenza, causa probabile,
lista delle cose da sistemare (input dei lotti di correzione).

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/` e `compiti/`; commenti, messaggi e
  documenti in italiano.
- MAME sempre senza finestra: `SDL_VIDEODRIVER=dummy`, `-video none -sound
  none -nothrottle` (come `cattura.sh`). Tutto in locale.
- Niente ROM o estratti binari nuovi nel repo; niente file sopra 50 MB da
  nessuna parte e niente tracce grosse lasciate su disco: l'oracolo produce
  impronte per frame (poche centinaia di KB), dump completi solo ai frame
  richiesti. I prodotti vanno in `decomp/out/` con prefisso `replay_`
  (gitignorati: `.gitignore` ha `decomp/out/*` con eccezione `!decomp/out/snap_*`).
- La partita registrata non deve entrare in stanze bonus/nascoste né arrivare
  al finale: con la sessione convenzionale qui prescritta non ci entra (fatto:
  nel lotto 12 bonus e finale sono stati raggiunti solo con forzature di RAM;
  qui nessun poke; la fine partita, codice 15, compare oltre il frame ~7300 e
  la finestra chiude a 5500).
- Nessun file di codice sopra le 800 righe (oggi in `harness/` nessuno lo è).
- Non toccare le catture esistenti in `out/` né i risultati del banco.

## Cosa esiste già (fatti verificati)

| file | righe | ruolo |
|---|---:|---|
| `decomp/harness/harness.c` | 214 | banco a routine singole; definisce `ram[0x4000]`, `rom[0x10000]`, `carica_rom()` statica (carica `out/main64.bin` o `../out/main64.bin`, righe 13-34) |
| `decomp/harness/nmi.c` | 264 | `sub_0066` = NMI completa tradotta: palette/sprite, ingressi e600-e602 → c050-c052, monete, stato di Dana, motore oggetti (24 slot da c46e), tabella sprite, contatori, copione, tempo, e603, watchdog, coda suoni su e800; non tocca i registri del chiamante |
| `decomp/harness/sched.c` + `sched.h` | 74+47 | `posta_attivita` (0493) e `spedisci_una` (3AF3-3B10) tradotti; `corpo_attivita[40]` oggi è di soli stub `STUB(xx, yyyy)` definiti in `sched.c` |
| `decomp/harness/sched_proto.c` | 142 | prova dello spedidore sul registro `out/sched_log.txt`; uso: `./sched_proto out/sched_log.txt` |
| `decomp/harness/porte.c`, `ram.h`, `e603.h` | 38+54+12 | porte in RAM (`leggi_porta`/`scrivi_porta` su `ram[a-0xc000]`), `leggi_e603(pc)` (tabella: solo PC 015E→00), macro `M(a)`, `struct z80` |
| `decomp/harness/routine_tab.c` | 121 | 114 voci `voce_routine` (116 routine in `ROUTINE.md`): tutte destinazioni di `call` |
| `decomp/harness/rst20_tables.h` | 136 | tabelle inline di `rst $20` generate da `rst20.py`, compresa la 3B11 (ingressi delle attività) |
| `decomp/sched_log.lua` | 80 | tap di lettura/scrittura su c000-c03f e c04e-c04f: righe `D/W/I` in `out/sched_log.txt`; ingressi convenzionali incorporati |
| `decomp/snap.lua` | 176 | cattura prima/dopo con breakpoint dal debugger e scarico di c000-ffff (`scarica()`, righe 103-109); schema del bp + `emu.register_periodic` |
| `decomp/sessione.lua`, `cattura.sh` | 189+67 | invocazione MAME di riferimento e ingressi da file |
| `decomp/SCHEDULER.md` | 188 | modello a funzioni dello spedidore, tabella 3B11 (codice→ingresso), formato di `sched_log.txt` |
| `out/main64.dasm` | — | disassemblato con etichette: fonte per tradurre i corpi |
| `out/sched_log.txt` | 107442 righe | registro D/W/I della sessione convenzionale a 5500 frame (gitignorato) |
| `out/snap_0x0066_gioco_regs.txt` | — | alla NMI del frame 2402 in gioco: SP=CFF8; titolo f1002: SP=CFFE, DE=3AF3 HL=C02F (NMI entrata a metà scansione) |

Fatti di architettura (da `SCHEDULER.md` e `grafo`):
- Il giro originale è `for(;;) spedisci_una(...)` più la NMI una volta a frame;
  una sola pila, le attività entrano dalla testa e cedono solo ritornando;
  tutto lo stato fra due giri è in RAM. `c04f`/`c04e` sopravvivono al frame.
- La NMI in MAME entra in un punto qualunque (a metà scansione o a metà
  attività): nel modello C la NMI gira al confine di frame, quindi le
  divergenze da interleaving sono attese e vanno classificate, non "corrette".
- I **corpi delle attività** (ingressi della tabella 3B11) sono raggiunti con
  `rst 20`, non con `call`: NON sono fra le 116 routine tradotte. Oggi sono
  stub. È questo che manca davvero per il giro intero.

Misure fatte per questo brief (da `out/sched_log.txt` e dall'unione dei
`out/*_eseguiti.txt`):
- Codici spediti nella finestra 2402-5500 (partita in corso, ingressi
  convenzionali): 01×37, 02×2174, 03×3252, 0A×14840, 0D×14840, 0E×52, 10×9,
  14×14840, 20×14840. Nessun altro codice. Dispatch per frame (2402-2501):
  min 15, max 27, media 21,0.
- Estensioni eseguite dei corpi necessari (blocco contiguo di PC eseguiti,
  passo ≤ 4, unione di tutte le sessioni):

| cod | ingresso | blocco eseguito | note (da SCHEDULER.md) |
|---|---|---|---|
| 01 | 48D1 | 48D1-491F (79 B) | diventa 02 a 4925/4937 |
| 02 | 4B84 | dentro 4ADA-4CA1 (statico 4B84-4BC5, 66 B) | resta (4B8B) o diventa 03; `sub_4af6`/`sub_4b0e` tradotte nel blocco |
| 03 | 4BC6 | dentro 4ADA-4CA1 (statico 4BC6-4C2F, 106 B) | resta (4BCD) o azzerata (4C10) |
| 0A | 4DA2 | 4DA2-4DA6 (5 B) | gate su c2b8; `sub_4d87` tradotta adiacente |
| 0D | 5C8E | dentro 5C19-5CE8 (gate ~12 B) | gate su c29a; `sub_5c32`/`sub_5c78` tradotte nel blocco |
| 0E | 63A4 | 63A4-643F (156 B) | diventa 03 a 63C2; il blocco contiene anche l'ingresso 0F (640E), mai spedito |
| 10 | 457B | dentro 452B-4644 | porta: diventa 11 o azzerata; `sub_452b` tradotta in testa al blocco |
| 11 | 456E | dentro 452B-4644 | resta o azzerata (457A) |
| 14 | 44B5 | 44B5-451B (103 B) | resta, sempre presente dal frame 610 |
| 20 | 3E90 | 3E90-3F45 (182 B) | resta (3E98); `sub_3f10` tradotta dentro; 3F39/3F40 sono gli ingressi 1C/1D, fuori finestra |

  Totale da tradurre: ~1 KB di Z80 in 10 corpi, quasi tutto smistamento che
  chiama routine già tradotte.
- Ingressi convenzionali (identici in `trace.lua`, `sched_log.lua`,
  `sessione.lua`, `snap.lua` modo `traccia`): moneta frame 1510-1530, start
  1560-1580, dal 1900 un tasto casuale fra i 6 di P1 ogni 8 frame con
  `math.randomseed(1)`. La rigenerazione delle catture con questi ingressi è
  bit a bit riproducibile (pratica consolidata del progetto, STATO.md).
- Banco oggi (misurato): `./harness --tutte` → 220 righe `cattura`, 0
  `FALLITO`, exit 0. `./sched_proto out/sched_log.txt` → ultima riga
  `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot
  coincidenti: 131`, exit 0.
- Zone legittimamente diverse fra MAME e C al punto di confronto: c040-c043
  (IX/IY del contesto interrotto, salvati dalla NMI) e la pila (SP alla NMI
  osservato a CFF8/CFFE, scende da D000; residui sotto SP che il C non
  riproduce — stessa ragione della «pila libera» esclusa dal banco,
  `harness.c` riga 105). La lettura di e603 fuori dal PC 015E (siti 4CED,
  62BE) non è mai raggiunta dai codici della finestra (attività 05/09/27 mai
  spedite).

## Struttura da realizzare

### 1. Fattorizzazione minima (per linkare un secondo main)

Nuovo `decomp/harness/comune.c`: definizioni di `ram[]`, `rom[]` e
`carica_rom()` (non più static, dichiarata in `ram.h`) spostate da
`harness.c`. Spostare gli `STUB(...)` e la definizione di `corpo_attivita[]`
da `sched.c` a `sched_proto.c` (unico utente degli stub): `sched.c` resta con
`posta_attivita` e `spedisci_una`; `sched.h` invariato.

### 2. Corpi delle attività: `decomp/harness/attivita.c` (nuovo)

Traduzione dal `out/main64.dasm` dei 10 corpi della tabella sopra, firma
`void attivita_XX_YYYY(uint8_t slot)` come `attivita_fn` di `sched.h`
(`pop hl; ret` → `return`; `pop hl; ld (hl),n; ret` → `tabella[slot] = n;
return`). Le chiamate interne vanno alle `sub_*` di `routine.h`; un `rst 20`
interno usa le tabelle di `rst20_tables.h` (se manca la tabella del sito,
rigenerarla con `rst20.py`). Stesso stile dei `z80_*.c` (commento con
indirizzi, nomi italiani). In fondo: `attivita_fn corpo_attivita_replay[SCHED_CODICI]`
con i 10 corpi tradotti e `NULL` altrove. Verifica dei corpi: il replay
stesso (confronto a ogni dispatch e impronta a ogni frame); registrarli in
`ROUTINE.md` in una sezione nuova «Corpi di attività (lotto 23)» con stato
«tradotta (replay)», senza toccare la tabella delle 116.

### 3. Oracolo: `decomp/replay.lua` + `decomp/replay.sh` (nuovi)

`replay.lua`, modellato su `sched_log.lua` (tap D/W/I identici) più un
breakpoint del debugger a `0x0066` gestito con `emu.register_periodic` come
`snap.lua`. Ingressi convenzionali incorporati. Variabili: `REPLAY_FINE`
(5500), `REPLAY_F0` (2402), `REPLAY_DUMPS` (lista di frame, vuota), `REPLAY_NOME`
(`replay`). A ogni stop su 0066 (frame `f` = contatore di `frame_done`):
- riga `F <f> <e600> <e601> <e602> <pc_sulla_pila?no:usare cpu.state> <hash>` in
  `out/<nome>_oracolo.txt`; e600-e602 letti con `mem:read_u8` (valori vivi
  delle porte); come PC di diagnosi registrare l'indirizzo di ritorno della
  NMI, `mem:read_u16(SP)` allo stop (dice se la NMI è entrata nello spedidore,
  PC in 3AF3-3B10, o a metà attività);
- hash FNV-1a a 32 bit di c000-e5ff **esclusi** c040-c043 e cf00-cfff
  (offset 2166136261, primo 16777619, `h = ((h ~ b) * 16777619) & 0xffffffff`;
  leggere il blocco con `mem:read_range` e iterare con `string.byte`);
- a `f == REPLAY_F0` e a ogni frame di `REPLAY_DUMPS`: scarico di c000-ffff in
  `out/<nome>_<f>.bin` (16384 byte, come `scarica()` di snap.lua).
Le righe D/W/I vanno in `out/<nome>_disp.txt`. `replay.sh` lancia MAME come
`cattura.sh` (stessa riga, `-autoboot_script replay.lua`), da `decomp/`;
durata attesa 10-30 minuti.

### 4. Giro di gioco: `decomp/harness/replay.c` (nuovo) + bersaglio `replay` nel Makefile

Uso: `./replay <oracolo.txt> <disp.txt> <dump_F0.bin> <F0> [frame_fine]`.
Sorgenti del bersaglio: tutti quelli di `harness` meno `harness.c`, più
`sched.c`, `attivita.c`, `replay.c`, `comune.c`. Logica:
1. `carica_rom()`; carica il dump in `ram[]`; `corpo_attivita[i] = corpo_attivita_replay[i]`.
2. Sanità: l'hash C (stessa funzione FNV-1a, stesse esclusioni) di `ram[]`
   deve coincidere con la riga `F` di F0. Se no, fermarsi: esclusioni da
   tarare (confrontare il dump col contenuto e allargare/stringere le zone,
   documentando in REPLAY.md).
3. Per ogni frame `f` da F0: scrivere e600-e602 della riga `F f` in
   `M(0xe600..0xe602)`; `sub_0066(&z)` con `struct z80 z = {0}` (la NMI non
   dipende dai registri del chiamante; c040-c043 è escluso dal confronto);
   poi, per ogni riga `D f slot codice` di `disp.txt`: se
   `corpo_attivita[codice] == NULL` → `CORPO <codice> (<ingresso>) non
   tradotto al frame <f>` e stop; altrimenti `spedisci_una(&s, &c)` e
   confronto con la D attesa → al primo disallineamento, rapporto
   `DISPATCH frame <f>: atteso slot/codice, ottenuto slot/codice` e stop.
4. A fine frame: hash C contro riga `F f+1`. Prima differenza →
   `DIVERGENZA frame <f+1>: hash C <x> MAME <y>; ritorno NMI oracolo <pc>;
   ultima D <slot> <codice>`; se esiste `out/replay_<f+1>.bin`, stampare i
   primi 32 byte diversi con indirizzo e stop (opzione `--oltre` per contare
   i frame divergenti senza fermarsi, utile a vedere se riconverge).

### 5. Diff per zona: `decomp/replay_diff.py` (nuovo)

`python3 replay_diff.py <dump_a.bin> <dump_b.bin>`: byte diversi raggruppati
per zona nominata (c000-c03f tabella attività; c040-c043 IX/IY salvati;
c046-c04d monete/crediti; c04e-c04f indici spedidore; c050-c053 ingressi e
comando; c054-c057 coda suoni; c463-c468 e c2a9 contatori; c46e-c8ed 24 slot
oggetti da 0x30 con numero di slot; c8ee-c94d tabella sprite; c994-c9b3 e
c9ca-ca49 palette; cf00-cfff pila; d880-dfff fondale; e020-e07f registri
sprite; e400-e5ff registri palette; resto «non mappata»). Con la zona e
l'ultima attività spedita (dal rapporto di `replay`) si attribuisce la
divergenza alla routine responsabile.

### 6. Rapporto: `decomp/REPLAY.md` (nuovo) e aggiornamenti

REPLAY.md: parametri della registrazione, numeri dell'oracolo (frame, NMI,
dispatch), esito del replay (primo frame divergente o nessuno), diagnosi
(zona, attività, ritorno NMI: a metà scansione/attività?), classificazione
delle cause (interleaving NMI atteso / corpo mancante / bug di traduzione da
correggere) e lista ordinata delle cose da sistemare. Aggiornare STATO.md
(2-4 righe nei «Prossimi passi») e la riga di ROUTINE.md detta sopra.

## Verifiche obbligatorie

1. `cd decomp/harness && make` → compila senza errori né warning nuovi
   (`-Wall -Wextra`; attesa, non misurata) e produce anche `replay`.
2. `./harness --tutte | grep -c '^cattura'` → `220`; `./harness --tutte |
   grep -c FALLITO` → `0`; exit 0 (misurato oggi: deve restare identico dopo
   la fattorizzazione di `comune.c`).
3. `cd decomp && harness/sched_proto out/sched_log.txt` → ultima riga
   `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot
   coincidenti: 131`, exit 0 (misurato oggi: invariante dopo lo spostamento
   degli stub).
4. Oracolo riproducibile: `sh replay.sh` due volte (seconda con
   `REPLAY_NOME=replay2`) → `diff out/replay_oracolo.txt out/replay2_oracolo.txt`
   vuoto (attesa dalla pratica delle catture bit a bit, non misurata su
   questo script).
5. Sanità F0: `harness/replay out/replay_oracolo.txt out/replay_disp.txt
   out/replay_2402.bin 2402` stampa la coincidenza dell'hash a F0 (deve
   passare; altrimenti taratura delle esclusioni, documentata).
6. Replay 2402→5500: **attesa, non misurata** — non sappiamo se regge 100 o
   10000 frame. Successo = o nessuna divergenza, o rapporto riproducibile
   (stesso frame e stessi byte a ogni esecuzione) della prima divergenza con
   diagnosi in REPLAY.md (zona da `replay_diff.py` sui dump richiesti con
   `REPLAY_DUMPS` ai frame vicini, attività dal log).
7. `find decomp/harness -name '*.c' -o -name '*.h' | xargs wc -l | awk
   '$1>800 && $2!~/totale/'` → vuoto (misurato oggi: vuoto).
8. `git status --short` → solo i file elencati in «Struttura» più questo
   brief; `git check-ignore decomp/out/replay_2402.bin` → la ignora;
   `find decomp/out -size +50M` → vuoto.

## Consegna

Commit in locale su `main` con messaggio «Lotto replay: oracolo per frame,
giro di gioco in C, prima divergenza diagnosticata» (adattare all'esito) e
riga `Co-Authored-By: <modello> <noreply@anthropic.com>`. Nessun push.
Rapporto finale: massimo trenta righe con l'uscita delle verifiche 1-8, il
frame della prima divergenza (o «nessuna») e le deviazioni dal brief.

## Domande aperte

1. **Perimetro del lotto** — i 10 corpi di attività (~1 KB di Z80) più tutta
   l'infrastruttura stanno in un lotto solo? Proposta: sì, perché i corpi
   sono smistamento sottile su routine già tradotte e il replay è la loro
   verifica naturale; in alternativa spezzare in 23a (corpi, verificati con
   una variante di snap.lua che fissa l'uscita a 3AF3 con SP+4) e 23b
   (oracolo+replay). **Scelta di progetto.**
2. **Verifica dei corpi col replay** invece che con 2 catture MAME ciascuno
   (convenzione di ROUTINE.md): il confronto a 60 Hz su migliaia di frame è
   più fitto di 2 catture; stato «tradotta (replay)» in sezione separata.
   Proposta: sì. **Preferenza.**
3. **Strumento di registrazione**: Lua scriptato (ingressi convenzionali già
   riproducibili bit a bit, tap e debugger già collaudati) invece di
   `-record/-playback` di MAME, che non dà impronte per frame né log delle
   spedizioni. Proposta: Lua. **Preferenza.**
4. **Esclusioni dell'impronta**: c040-c043 e cf00-cfff come insieme di
   partenza, tarato sulla sanità a F0. Proposta: così, margine di pila largo
   (256 byte) poi eventualmente stretto. **Preferenza.**
5. **Finestra**: F0=2402 (gioco, stanza 1) e fine 5500, come tutte le misure
   esistenti. Proposta: così. **Preferenza.**
