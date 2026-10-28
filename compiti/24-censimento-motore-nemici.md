# Compito 24: censimento del motore dei nemici (corpo 0A) e piano dei lotti

2026-10-28, scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Censire il motore dei nemici dietro il corpo di attività 0A (ingresso 4DA2):
tabelle `rst $38` vere (indirizzi e lunghezze dalla ROM), elenco definitivo
delle routine con confini, dimensioni e grafo delle chiamate, copertura da
parte della sessione del replay, e piano di spezzatura in lotti di traduzione
con i moduli C nuovi. **In questo lotto non si traduce nulla**: il prodotto è
il censimento (`decomp/CENSIMENTO-NEMICI.md`), gli strumenti che lo generano
e la tabella `harness/rst38_tables.h` per i lotti futuri.

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/` e `compiti/`; commenti, messaggi e
  documenti in italiano.
- Nessuna modifica ai sorgenti C esistenti del banco e del replay: l'unico
  file nuovo in `harness/` è `rst38_tables.h` (generato) più la riga di
  dipendenza nel `Makefile` (vedi Struttura §5). `harness --tutte`,
  `sched_proto` e `replay` devono dare l'uscita identica a oggi (misurata
  nelle Verifiche).
- MAME sempre senza finestra e con gli ingressi dell'host spenti: copiare la
  riga di `replay.sh` (`SDL_VIDEODRIVER=dummy`, `-video none -sound none
  -keyboardprovider none -mouseprovider none -joystickprovider none
  -lightgunprovider none -skip_gameinfo -nothrottle -debug -debugger none`).
- Niente file sopra 50 MB; la traccia MAME va cancellata a fine elaborazione
  (`coverage.py --cancella`). Prodotti in `decomp/out/` (gitignorati).
- Nessun file di codice sopra le 800 righe (oggi in `harness/` nessuno lo è,
  misurato).
- Non toccare le catture esistenti in `out/` né i risultati del banco.

## Cosa esiste già (fatti verificati)

| file | righe | ruolo |
|---|---:|---|
| `decomp/out/main64.bin` | 65536 B | ROM decodificata del Z80 principale (fonte delle tabelle) |
| `decomp/out/main48.dasm`, `main64.dasm` | — | disassemblati z80dasm; formato riga `\t<istr>\t;<addr>`, etichette `lXXXXh:`/`sub_XXXXh:`; `rst 38h` vi compare 3126/3947 volte, quasi tutte byte FF nei dati: i siti veri si trovano col cammino dal 4DA2, non col grep (da grep) |
| `decomp/rst20.py` | 241 | tool gemello per `rst $20`: da copiare/adattare (sotto) |
| `decomp/coverage.py` | 260 | copertura da traccia MAME: `--traccia`, `--prefisso`, `--cancella`; produce `out/<pre>_eseguiti.txt` (PC `%04X` uno per riga), `_istruzioni.txt`, `_chiamate.txt` (`sito\tdest\tnumero`), `_copertura.txt`, `_numeri.txt`; funzioni statiche riusabili `lunghezza(rom, pc)` e `destinazione(mnem, operandi)` (da `grafo outline` e lettura) |
| `decomp/sessione.lua` | 189 | sessione con finestre di traccia: `SESS_NOME`, `SESS_INPUT=casuale` (seme 1, dal frame 1900), `SESS_MONETA=1` (moneta 1510-1530, start 1560-1580), `SESS_TRACE_DA`/`SESS_TRACE_A` (l'ultima fine chiude); scrive `out/<nome>_trace.txt` |
| `decomp/replay.sh` | 33 | riga MAME di riferimento con i 4 provider spenti |
| `decomp/REPLAY.md` | 149 | perimetro misurato di 0A: ~1986 istruzioni, ~3600 byte, stima 36 routine per eccesso; «Cose da sistemare» punto 1 = questo censimento |
| `decomp/CENSIMENTO-12.md` | — | modello di forma per il documento di censimento |
| `decomp/harness/routine.h` | 154 | le 116 routine tradotte (`void sub_XXXX(struct z80 *z)`): frontiera del censimento |
| `decomp/harness/rst20_tables.h` | 136 | esempio del formato di intestazione generata |
| `decomp/harness/attivita.c` | 350 | 7 corpi tradotti (~600 B di Z80): ~0,6 righe C per byte Z80, metro per stimare i moduli |
| `STATO.md` | — | «Prossimi passi, in ordine» alla riga 53: da aggiornare |

Fatti sul meccanismo `rst $38` (dal disassemblato, gestore a 0038):

    ex (sp),hl        ;0038
    ld d,000h         ;0039
    add hl,de         ;003b   HL = indirizzo dopo il rst + 2*E
    add hl,de         ;003c
    ld e,(hl)         ;003d
    inc hl            ;003e
    ld d,(hl)         ;003f
    ex de,hl          ;0040
    ex (sp),hl        ;0041
    ret               ;0042   salta alla voce

Cioè `jp tabella[E]` con la tabella di parole little-endian subito dopo il
byte FF del `rst 38h`. Identico a `rst $20` (gestore 0020, indice in A, ret
del gestore a 0026) salvo il registro indice e il PC del ret: **0042**.

Fatti sul corpo 0A e sul motore (da REPLAY.md e dal disassemblato):
- Ingresso 4DA2: `ld a,(lc2b8h); ld e,a; rst 38h` (sito 4DA6). I byte dopo
  il sito sono `AD 4D 32 46 E6 4E`: tabella di 3 voci **4DAD, 4632, 4EE6**
  (misurato dalla ROM; coincide con le 3 destinazioni osservate; l'euristica
  di `rst20.py` si ferma giusta perché 4DAD è la più piccola destinazione
  maggiore del sito).
- Altri sette siti `rst $38` noti nel motore: **4F1B, 4FE0, 5127, 5207,
  5E40, 6130, 728B, 73B6**; le loro lunghezze non sono note e vanno ricavate
  (euristica + indice massimo osservato in traccia). Il cammino statico può
  scoprirne altri: vanno inclusi.
- Routine già tradotte nella zona del motore = frontiera, non vanno censite
  come "da tradurre" (da `routine.h`): sub_452b, sub_4722, sub_472e,
  sub_4af6, sub_4b0e, sub_4d05, sub_4d37, sub_4d87, sub_4dee, sub_4e31,
  sub_4e87, sub_5507, sub_5c32, sub_5c78, sub_673f, sub_6771, sub_6c2c,
  sub_7379, sub_75aa, sub_797e, sub_7c84, sub_7dea, sub_7e1d, sub_7e24,
  sub_7e32, sub_7eca, sub_7efe, sub_7f2d; più le statiche di `attivita.c`
  (sub_4b98, sub_250e). L'elenco completo della frontiera si legge da
  `routine.h` con la regex `sub_([0-9a-f]{4})`.
- Candidati noti del motore (prime voci osservate, stima per eccesso): 4F0F,
  4FEF, 5115, 5124, 51D7, 51FA, 52FD, 531C, 5345, 54C5, l'intervallo
  5D29-6252, 734D, 738F, 758E, 759F. Il censimento li sostituisce con
  l'elenco definitivo.
- **4FC5** cade nel motore ed è stata osservata dal vivo: 6 occorrenze
  `DEST 4FC5` in `out/partita_viva_20261027.log`, 1 in
  `out/partita_viva_20261027b.log` (script `copertura_viva.lua`); probabile
  gestore di comparsa di un'entità (fata). Annotarlo nella colonna note.
- `out/eseguiti_unione.txt` (11395 PC, unione storica) contiene 4DA2 e 4EE6
  (misurato): la copertura nuova deve contenerli anch'essa.
- Il replay oggi: il corpo 0A è spedito a ogni frame della finestra di gioco
  ed è la prima spedizione dopo F0=2410; senza di esso il giro non supera il
  frame 2410.

Misure di oggi (comandi eseguiti, uscite esatte):

    $ cd decomp/harness && ./harness --tutte      # exit 0
      220 righe «cattura», 0 «FALLITO»
    $ cd decomp && harness/sched_proto out/sched_log.txt | tail -1
      spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131
    $ harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410   # exit 1
      SANITA F0 2410: hash C A350CF53 MAME A350CF53 coincide
      CORPO 0A (4DA2) non tradotto al frame 2410
      NMI confrontate: 1 (nello spedidore 1, a metà attività 0); divergenti: nello spedidore 0, a metà attività 0
      nessuna divergenza nello spedidore

## Struttura da realizzare

### 1. Traccia di copertura del replay: `out/replay_gioco_*`

Da `decomp/`, una sessione con gli stessi ingressi convenzionali del replay,
host spento, finestra di traccia 2400-5500 (il contatore di `sessione.lua` è
`frame_done`, sfasato di ~1 rispetto al replay: irrilevante, la finestra è
larga):

    SESS_NOME=replay_gioco SESS_INPUT=casuale SESS_TRACE_DA=2400 SESS_TRACE_A=5500 \
    SDL_VIDEODRIVER=dummy mame solomon -rompath "$(cd .. && pwd)/roms" \
      -video none -sound none -keyboardprovider none -mouseprovider none \
      -joystickprovider none -lightgunprovider none -skip_gameinfo -nothrottle \
      -debug -debugger none -autoboot_script sessione.lua

Poi, **in quest'ordine** (il secondo cancella la traccia):
`python3 censimento_nemici.py --traccia out/replay_gioco_trace.txt ...` e
`python3 coverage.py --traccia out/replay_gioco_trace.txt --prefisso
replay_gioco --cancella`.

### 2. Strumento: `decomp/censimento_nemici.py` (nuovo)

Un solo script (vedi Domande aperte 1), modellato su `rst20.py` (importare da
`rst20` e `coverage` le funzioni riusabili: `leggi_dasm`,
`trasferimento_incondizionato`, `voci_euristiche`, `lunghezza`,
`destinazione`; stesse regex). Fa tre cose:

a) **Cammino statico dal 4DA2**: insieme dei PC raggiungibili seguendo
   caduta, `jp`/`jr`/`djnz`/`call` statici, `rst 20h` (voci da
   `out/rst20.txt`, formato `SITO n stato idx voci...`) e `rst 38h` (voci
   calcolate al punto b). Il cammino si ferma sulle routine della frontiera
   (regex su `routine.h`) e su `jp (hl)`/`jp (ix)` (da segnalare come punti
   aperti nel censimento). I siti `rst 38h` si scoprono così, non col grep.
b) **Tabelle `rst $38`** per ogni sito scoperto o dinamico:
   `voci_euristiche(rom, sito, ingressi)` sulla ROM `out/main64.bin`, due
   passate come `rst20.py`; indice massimo osservato dalla traccia (sito =
   riga `XXXX: rst  $38`, destinazione = PC della riga dopo `0042: ret`,
   con la pila per gli annidamenti come `leggi_traccia`). Il limite statico
   di `rst20.py` ragiona su A: per `rst 38` l'indice è E, dedotto da `ld e,a`
   preceduto da `and`/`cp` su A, o da `ld e,N`; dove non deducibile, voci
   dall'euristica e indice massimo dalla traccia.
c) **Routine e grafo**: ingressi = 4DA2 + destinazioni di `call` + voci
   delle tabelle `rst 20`/`rst 38` dentro il perimetro; per ogni ingresso i
   PC raggiunti senza attraversare un altro ingresso; dimensione in byte
   (somma delle lunghezze), estensione min-max, archi chiamante→chiamata.

Uscite: `out/rst38.txt` (stesso formato di `out/rst20.txt`),
`harness/rst38_tables.h` (stesso formato di `rst20_tables.h`, prefisso
`rst38_`, commento «indice in E»), `out/censimento_nemici.txt` (una riga per
routine: `ingresso estensione byte coperta chiamate chiamanti`, `coperta` =
SI se tutti i PC dell'ingresso eseguiti stanno in
`out/replay_gioco_eseguiti.txt`, PARZIALE/NO altrimenti).

### 3. Documento: `decomp/CENSIMENTO-NEMICI.md` (nuovo)

Modellato su `CENSIMENTO-12.md`. Sezioni: metodo (due righe e comandi);
tabella dei siti `rst $38` (sito, n. voci, voci, indice max osservato,
stato); tabella delle routine (ingresso, estensione, byte, chiamanti,
coperta dal replay, note — 4FC5 con l'osservazione dal vivo); punti aperti
(`jp (hl)`, tabelle con lunghezza incerta); **piano dei lotti** (sotto).

### 4. Piano dei lotti (dentro CENSIMENTO-NEMICI.md)

- **Lotto N1** = il corpo 0A (4DA2) più tutte le routine del motore coperte
  dal replay (`coperta` = SI o PARZIALE): è l'insieme che il giro 2410-5500
  esegue, quindi tradurlo sblocca il replay oltre il frame 2410. Verifica
  dei lotti futuri: il replay stesso, stato «tradotta (replay)» in sezione
  separata di `ROUTINE.md` (convenzione del lotto 23).
- **Lotti N2+**: le routine non coperte, raggruppate per vicinanza nel grafo
  (stesso smistatore `rst 38`), ordinate per utilità (prima quelle
  raggiungibili da indici di tabella plausibili in partita).
- Per ogni lotto: elenco di ingressi, byte totali, stima righe C a 0,6
  righe/byte, modulo di destinazione.
- **Moduli C nuovi** (prescritti qui, creati dai lotti futuri):
  `harness/nemici.c` (corpo 0A, smistatori `rst 38` e routine comuni) più
  `harness/nemici_<tema>.c` con temi in italiano scelti dal censimento (es.
  `nemici_comparsa.c` se 4FC5 e affini); dichiarazioni condivise in
  `harness/nemici.h` nuovo; nessun modulo oltre ~700 righe stimate (margine
  sulla soglia di 800); i lotti li aggiungeranno a `SORGENTI_REPLAY` nel
  Makefile. Il motore ~3600 B ≈ 2100-2300 righe C: attesi 3-4 moduli.

### 5. Ritocchi

- `Makefile`: aggiungere `rst38_tables.h` alle dipendenze del bersaglio
  `replay` (riga `replay: $(SORGENTI_REPLAY) ram.h routine.h sched.h e603.h
  rst20_tables.h` → aggiungere `rst38_tables.h`). Nessun sorgente lo include
  ancora: va bene così.
- `REPLAY.md`, «Cose da sistemare» punto 1: due righe che rimandano a
  `CENSIMENTO-NEMICI.md` e al piano dei lotti.
- `STATO.md`, «Prossimi passi»: 2-3 righe (censimento fatto, prossimo =
  lotto N1).

## Verifiche obbligatorie

1. `cd decomp/harness && make` → compila senza errori né warning nuovi
   (attesa, non misurata: nessun sorgente cambia).
2. Banco invariato: `./harness --tutte` → 220 righe `cattura`, 0 `FALLITO`,
   exit 0 (misurato oggi). `cd decomp && harness/sched_proto
   out/sched_log.txt | tail -1` → `spedizioni confrontate: 106124,
   differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
3. Replay invariato: `harness/replay out/replay_oracolo.txt
   out/replay_disp.txt out/replay_2410.bin 2410` → le 4 righe esatte
   riportate sopra in «Misure di oggi», exit 1 (misurato oggi).
4. Intestazione generata compilabile: da `decomp/harness/`,
   `echo '#include "rst38_tables.h"' | gcc -std=c11 -Wall -Wextra
   -fsyntax-only -xc -I. -` → nessuna uscita, exit 0 (attesa).
5. Tabella 4DA6: la riga di `out/rst38.txt` per il sito 4DA6 ha 3 voci
   `4DAD 4632 4EE6` (misurato dalla ROM). Gli altri otto siti noti (4F1B,
   4FE0, 5127, 5207, 5E40, 6130, 728B, 73B6) presenti con almeno 1 voce;
   lo script stampa `destinazioni fuori tabella: 0` sulla traccia
   `replay_gioco` (attesa, non misurata).
6. Copertura: `grep -c '^4DA2$\|^4EE6$' out/replay_gioco_eseguiti.txt` → 2
   (attesa: entrambi stanno nell'unione storica, misurato lì).
7. Coerenza del piano: ogni routine del censimento in esattamente un lotto;
   il lotto N1 contiene tutte e sole le routine coperte più 4DA2; somma dei
   byte dei lotti = totale del censimento; stima righe per modulo ≤ 700
   (controlli dichiarati nel rapporto, con i numeri).
8. `find decomp/harness -name '*.c' -o -name '*.h' | xargs wc -l | awk
   '$1>800 && $2!~/totale/'` → vuoto (misurato oggi: vuoto);
   `find decomp/out -size +50M` → vuoto (misurato: vuoto); traccia
   `out/replay_gioco_trace.txt` cancellata.
9. `git status --short` → solo: `decomp/censimento_nemici.py`,
   `decomp/CENSIMENTO-NEMICI.md`, `decomp/harness/rst38_tables.h`,
   `decomp/harness/Makefile`, `decomp/REPLAY.md`, `STATO.md`, questo brief
   (i prodotti in `decomp/out/` sono gitignorati).

## Consegna

Commit in locale su `main` con messaggio «Censimento del motore dei nemici
(corpo 0A): tabelle rst 38, routine, piano dei lotti» e riga
`Co-Authored-By: <modello> <noreply@anthropic.com>`. Nessun push.
Rapporto finale: massimo trenta righe con l'uscita delle verifiche 1-9, i
numeri del censimento (siti rst 38, routine, byte, coperte/totali, lotti,
moduli) e le deviazioni dal brief.

## Domande aperte

1. **Uno script o due** — `censimento_nemici.py` unico (tabelle rst 38 +
   cammino + grafo) oppure `rst38.py` separato come gemello di `rst20.py`
   più lo script del censimento. Proposta: unico, perché le tabelle `rst 38`
   sensate sono solo quelle raggiunte dal cammino (3126 falsi `rst 38h` nei
   dati di main48.dasm). **Preferenza.**
2. **Nomi dei moduli** — schema `nemici.c` + `nemici_<tema>.c` +
   `nemici.h`, temi in italiano scelti dal censimento. Proposta: così.
   **Preferenza.**
3. **Perimetro del lotto N1** — se l'insieme coperto dal replay supera ~1200
   righe C stimate, spezzarlo in N1a/N1b per grafo, sapendo che il replay si
   sblocca solo a fine N1b (verifiche intermedie: compilazione e banco
   invariato, replay con stub che segnala la routine mancante). Proposta:
   prevederlo nel piano solo se la stima lo impone. **Scelta di progetto.**
4. **Finestra della copertura** — traccia 2400-5500 con il contatore
   `frame_done` di `sessione.lua` (sfasato di ~1 frame rispetto al replay).
   Proposta: così, la finestra larga assorbe lo sfasamento. **Preferenza.**

## Risoluzioni (2026-10-28)

- Domanda 1: ACCETTATA la proposta — script unico `censimento_nemici.py`.
- Domanda 2: ACCETTATA la proposta — schema `nemici.c` + `nemici_<tema>.c`
  + `nemici.h`, temi in italiano.
- Domanda 4: ACCETTATA la proposta — finestra 2400-5500 con `frame_done`.
- Domanda 3: RISOLTA con l'utente — ACCETTATA la proposta: il piano prevede
  la spezzatura in N1a/N1b solo se la stima del lotto N1 supera ~1200 righe
  C; altrimenti N1 resta unico.

## Esito (2026-10-28)

PASSATO. Sviluppatore: 9/9 verifiche; verificatore: 8/9 passate, la 9
(`git status` con 7 file) fallita solo perché lo sviluppatore aveva già
committato (924042b, albero pulito) — non è un errore di contenuto.

Scoperta principale: il motore vero è 8833 byte e 220 routine (non ~3600 B
e 36): la vecchia stima contava solo il codice già eseguito. Piano: lotti
N1-N12, 11 moduli C (max 660 righe stimate). N1 = 27 routine, 1564 byte,
~938 righe C stimate: sotto la soglia dei 1200, resta unico. Deviazioni
annotate nel rapporto: tabella rst 20 di 6F1C accorciata a 16 voci solo nel
cammino dello script; 10 routine comuni ai corpi 01/10 in nemici_comuni.c;
temi dei moduli per numero di tipo (tabella 4F1B) tranne nemici_comparsa.c
(4FC5); 7F68 salta dentro sub_7f2d, smistamento di 7F39 da rifare in N1.

Token: analista Fable ~89k, sviluppatore Opus ~135k, verificatore Sonnet
~18k. Nulla su OpenRouter.
