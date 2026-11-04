# Compito NN: stelline d'inizio quadro nel gioco libero

2026-11-04, scritto dall'agente analista per un agente sviluppatore. Questo
file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Nel gioco libero (`gioca` senza `--oracolo`) il cerchio di 16 stelline che a
inizio quadro si stringe sulla chiave si disegna male: avanza 11 passi per
frame invece di 1, quindi in 7 frame il raggio passa per lo zero, si ribalta e
il cerchio esplode fuori dallo schermo e finisce lontano dalla chiave. Bisogna
fare in modo che la macchina libera chiuda il frame dopo ogni passo del
cerchio, come l'originale. La correzione è di circa 20 righe di C più i
documenti.

Vincoli assoluti:
- Niente modifiche fuori da `/home/rufio72/OpenSolomonsKey-master`; si toccano
  solo `decomp/harness/{ram.h,comune.c,z80_3xxx.c,macchina_libera.c}` e i
  documenti `decomp/GIRO.md`, `decomp/ROUTINE.md` e `STATO.md`.
- Non toccare `macchina.c`, `replay.c`, `sched.c`, `harness.c` né le catture:
  il modo oracolo, il replay, il giro e il banco devono restare identici byte
  per byte.
- Commenti, messaggi e documenti in italiano.
- MAME solo con `-video none` (in questo compito non serve).
- Nessun file di codice sopra le 800 righe (`z80_3xxx.c` ne ha 579).
- Niente git distruttivo (stash, reset, filter-branch).

## Diagnosi (fatti misurati)

### La catena che disegna le stelline

| passo | dove | cosa fa |
|---|---|---|
| corpo 07 (2CCD), fase 13 | `attivita_attract07.c:292` → `fase_07_2dce` (`attivita_attract07c.c:199`) | sotto-passo in c19e, `rst 20` sulla tabella 2DDA (7 voci) |
| voce 0, 3009 | `t3_3009` (`attivita_attract07c.c:18`) | c82e sulla cella della chiave c249 (4B0E); passi del centro verso 8078 (2FF3) in c2cb/c2cd; c2c1-c2c7 dai 7 byte ROM 303E; 16 oggetti di riposo (3045); suono 34; **c463 = 0** |
| voce 1, 305F | `t3_305f` (`attivita_attract07c.c:60`) | c19e − 1 (resta sulla voce); se c463 < 40: `sub_3097`, poi `sub_237e` su c2c5 e due `sub_23a1`; se c463 ≥ 40 spegne i 16 oggetti e passa alla voce 2 |
| 3097 | `sub_3097` (`z80_3xxx.c:219-270`) | 16 oggetti da c46e (passo 30) sul cerchio: centro c2c2/c2c4, fase c2c5, raggio c2c6 (via `sub_30ef` tabella seni 3111 e `sub_3101` moltiplicazione) |
| 237E | `sub_237e` (`z80_2xxx.c:12`) | fase c2c5 + 1 (mod 80), raggio (parola c2c6:c2c7) − 0150 |
| 23A1 | `sub_23a1` (`z80_2xxx.c:28`) | centro c2c1-c2c2 += c2cb-c2cc, c2c3-c2c4 += c2cd-c2ce |
| c463 | `nmi.c:218` | contatore incrementato dalla NMI, una volta per frame |

Tutto da `grafo symbol`. Il cerchio dura quindi 64 **frame** (c463 da 00 a 3F),
ma avanza di un passo per **chiamata** di 305F: il disegno giusto dipende da
quante volte 305F gira in un frame. `sub_3097` ha un solo chiamante (305F).

### Originale: un passo per frame (misurato)

`decomp/out/sched_log.txt` (MAME, `sched_log.lua`), spedizioni per frame
`codice07/totale`, comando
`awk '$1=="D"{t[$2]++; if($4=="07") s[$2]++} END{...}' out/sched_log.txt`:
- frame 1690-1771 (fasi precedenti): 12-13 / 24-26;
- frame 1772: 5/10 (3009 e primo 305F); **frame 1773-1835: 1/2** (con due
  eccezioni 1792 2/3 e 1793 0/1, 1816/1817 uguali); frame 1836: 5/10;
- nei frame 1765-1840 gli slot attivi sono due: 0B (codice 14) e 0C (07).
La routine 3097 dura circa un frame (commento in `z80_3xxx.c:227-228`): dopo
ogni passo del cerchio arriva la NMI. 64 passi in 64 frame.

### Gioco libero: 11 passi per frame (misurato)

`macchina_libera.c` non ha un modello dei cicli: la NMI entra quando i corpi del
frame arrivano a `CORPI_PER_FRAME` = 22 (righe 24, 76-80, 89-91), qualunque sia
il loro costo. Con due slot attivi sono 11 giri di 07 e 11 di 14 per frame.
Misura con gdb sul binario attuale (`decomp/harness/gioca`, compilato
2026-11-04 11:57), script in «Verifiche», partita
`ricette/partita_1a.reg -n 4000`:
- `t3_3009` ai frame 1381 e 2737 (due inizi di quadro);
- `sub_3097` 1402 chiamate: frame 1381 → 8, frame 1382-1444 → 11 ciascuno;
  frame 2737 → 8, 2738-2800 → 11;
- stato (c463, fase c2c5, raggio c2c6, centro c2c2, c2c4) prima delle chiamate:
  - passo 0: `C 1381 00 00 60 80 78`
  - passo 63: `C 1387 06 3f 0d 41 48` (qui l'originale è all'ultimo passo, frame 1444)
  - passo 64: `C 1387 06 40 0c 40 48` (raggio 0C sul centro 40,48: la chiave)
  - passo 73-74: `C 1387 06 49 00 37 41`, `C 1388 07 4a fe 36 40` (raggio ribaltato a FE)
  - ultima: `C 1444 3f 3c c9 c4 6b` (700 passi: raggio C9, centro C4,6B)

Il difetto è questo: dopo ~7 frame il cerchio supera la chiave, il raggio
passa da 00 a FE e per i 57 frame restanti le stelline girano su un cerchio
enorme e spostato. In modo oracolo l'istante delle NMI viene da MAME
(`macchina.c`), per questo il video lì coincide.

### Sospetti scartati (con prova)

- **Registro R**: nel gioco libero ogni lettura di R ferma il gioco
  (`r_letto_libero`, `macchina_libera.c:82-85`, «FERMO lettura del registro R
  non modellata»): la partita supera i due inizi di quadro senza FERMO
  (`FRAME rese 4000`), quindi la catena non legge R. Le routine con `ld a,r`
  sono 4302, 54C5, 59E5, 5DDF (wrapper `banco_r_*` in `nemici_r.c`), nessuna
  nella catena sopra.
- **Avviso 1288/12AB**: le 224 righe «1288: continuazione 12AB (raccolta) non
  tradotta» escono ai frame 1876-2088 e 2103-2113 (gdb su `sub_1288`,
  stessa partita), lontano dagli inizi di quadro (1381-1444, 2737-2800). È la
  raccolta di un oggetto sotto Dana (`z80_1xxx.c:13-32`), altro lotto.

## Cosa esiste già (fatti verificati)

| file | righe | ruolo |
|---|---:|---|
| `decomp/harness/ram.h` | 73 | ganci globali: `macchina_punto` + `MACCHINA_PUNTO` (righe 62-71), `#endif` riga 73 |
| `decomp/harness/comune.c` | 53 | definizioni dei ganci: riga 14 `void (*macchina_punto)(uint16_t da, uint16_t a) = NULL;` |
| `decomp/harness/z80_3xxx.c` | 579 | `sub_3097` righe 219-270; ultima istruzione `z->ix = ix;` riga 269 |
| `decomp/harness/macchina_libera.c` | 147 | `CORPI_PER_FRAME 22` riga 24; contatori `static long punti, corpi, letture;` riga 36; `scansione_libera` 76-80; `r_letto_libero` 82-85; `involucro_libero` 87-98; in `macchina_libera_esegui` i ganci dopo il boot alle righe 113-116 (`macchina_punto = NULL; nemici_punto = NULL; sched_gancio_scansione = scansione_libera; nemici_r_gancio = r_letto_libero;`) |

- `macchina_libera.c` entra solo in `gioca` e `gioco_libero` (Makefile riga 80,
  `SORGENTI_GIOCA`); `comune.c` e `z80_*.c` in tutti i bersagli
  (`SORGENTI_BASE`, riga 3). Un gancio NULL fuori dalla macchina libera non
  cambia banco, replay, giro né oracolo.
- `gioca --oracolo` passa da `macchina_esegui` (`gioca.c:270-273`), il modo
  libero da `macchina_libera_esegui` (`gioca.c:275-276`).
- Simboli presenti nel binario `gioca` (da `nm`): `sub_3097`, `t3_3009`,
  `t3_305f`, `sub_1288`, `attivita_1a_411d`, `frame_corr` (statico), `ram`.

## Struttura da realizzare

1. `ram.h`, prima di `#endif`: un gancio nuovo, stesso stile di
   `macchina_punto`:
   ```c
   /* Frame pieno (gioco libero): una routine che nell'originale occupa da sola
    * il resto del frame lo dichiara con MACCHINA_FRAME_PIENO(); la macchina
    * libera chiude il frame al confine seguente. Altrove è NULL e non cambia
    * nulla (definito in comune.c). */
   extern void (*macchina_frame_pieno)(void);
   #define MACCHINA_FRAME_PIENO() \
       do { if (macchina_frame_pieno) macchina_frame_pieno(); } while (0)
   ```
2. `comune.c`, dopo la riga 14: `void (*macchina_frame_pieno)(void) = NULL;`
3. `z80_3xxx.c`, in `sub_3097` dopo `z->ix = ix;` (riga 269):
   `MACCHINA_FRAME_PIENO();` con un commento «nell'originale 3097 dura circa
   un frame: in MAME 305F gira una volta per frame (sched_log, frame 1773-1835)».
4. `macchina_libera.c`:
   - accanto a `r_letto_libero`: `static void frame_pieno_libero(void) { corpi = CORPI_PER_FRAME; }`
     (con commento): al confine seguente `scansione_libera` vede
     `corpi >= CORPI_PER_FRAME` e chiama `nmi_libera`;
   - dopo `nemici_r_gancio = r_letto_libero;` (riga 116):
     `macchina_frame_pieno = frame_pieno_libero;` (non durante il boot).
   - aggiornare il commento d'intestazione (righe 1-6): la cadenza è 22 corpi
     per frame salvo le routine che dichiarano il frame pieno.
5. Documenti: `decomp/GIRO.md`, nella parte sul gioco libero (righe ~395-425),
   un punto «Stelline d'inizio quadro» con le misure di questo file e le nuove
   uscite; `decomp/ROUTINE.md` riga 82 (voce 3097): «nel gioco libero dichiara
   il frame pieno»; `STATO.md`: una riga sotto il lotto 52.

Il nome del gancio è libero; la posizione nella routine no (dopo l'ultimo
oggetto, così il passo del frame è completo).

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master/decomp` salvo dove scritto. Prima di
toccare il codice salvare le uscite di 3, 4 e 5 in `/tmp` per il confronto.

1. Compilazione: `cd harness && make harness replay gioco_libero gioca sched_proto
   > /tmp/mst.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/mst.txt`
   → `make 0` e `0`.
2. Cadenza del cerchio, con questo script gdb (salvarlo in `/tmp/c.gdb`):
   ```
   set debuginfod enabled off
   set pagination off
   break sub_3097
   commands
   silent
   printf "C %ld %02x %02x %02x %02x %02x\n", (long)frame_corr, ((unsigned char*)&ram)[0x463], ((unsigned char*)&ram)[0x2c5], ((unsigned char*)&ram)[0x2c6], ((unsigned char*)&ram)[0x2c2], ((unsigned char*)&ram)[0x2c4]
   continue
   end
   run
   quit
   ```
   `gdb -batch -x /tmp/c.gdb --args harness/gioca --riproduci ricette/partita_1a.reg
   -n 4000 --muto 2>/dev/null | grep '^C ' > /tmp/c.txt; awk '{n[$2]++} END{for(f in n) if(n[f]!=1) print f, n[f]}' /tmp/c.txt; grep -c . /tmp/c.txt; grep '^C 1444' /tmp/c.txt`
   → prima della correzione (misurato): frame 1381 → 8, 1382-1444 → 11,
   2737 → 8, 2738-2800 → 11; 1402 righe; ultima `C 1444 3f 3c c9 c4 6b`.
   Dopo (attesa, non misurata): awk vuoto (un passo per frame), 128 righe (64
   per quadro; se il secondo quadro si sposta, 64 righe per ciascun quadro
   presente), `C 1444 3f 3f 0d 41 48` (il primo quadro parte ancora al 1381:
   prima di lì nulla cambia).
3. Banco: `cd harness && ./harness --tutte > /tmp/hst.txt; echo "exit $?"; grep -c
   FALLITO /tmp/hst.txt; grep -c '^RIEPILOGO' /tmp/hst.txt; grep -c '^OK' /tmp/hst.txt`
   → `exit 0`, `0`, e gli altri due numeri uguali a prima.
4. Invarianti (uscite di oggi dal brief 52, invariate dopo il lotto 52):
   - `sh giro_completo.sh | tail -1` → `GIRO COMPLETO 0-5499: saldatura 0 byte,
     spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`;
   - `harness/sched_proto out/sched_log.txt | tail -1` → `spedizioni confrontate:
     106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`;
   - `harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 | tail -4`
     → uguale a prima (riga `spedizioni eseguite: 64686, fino al frame 5499`);
   - `harness/gioca -n 5434 --muto --oracolo out/giroboot_oracolo.txt
     out/giroboot_disp.txt --riproduci out/ingressi_replay.txt | tail -6` →
     `FRAME_VIDEO 5499: hash C a239d0a940bec0b9 MAME a239d0a940bec0b9 tabella
     a239d0a940bec0b9, coincide` / `FRAME rese 5434` / `HASH ultimo
     a239d0a940bec0b9` / `FRAME ultimo 5499` / `AUDIO campioni 4042500` /
     `AUDIO hash c4b40fea06623a96`.
5. Partita registrata: `harness/gioca --riproduci ricette/partita_1a.reg -n 4000
   --muto 2>/dev/null` → prima (misurato): `FRAME rese 4000`, `HASH ultimo
   3f25ba053eeb56b9`, `AUDIO hash 79f9a550028a41b9`, nessun FERMO. Dopo: `FRAME
   rese 4000` e nessun FERMO; gli hash cambiano (attesa, non misurata: il
   corpo 14 gira 64 volte in meno per quadro). Riportare i nuovi hash.
6. Ricetta 1A ancora raggiunta: script gdb come in 2 con `break attivita_1a_411d`
   e `printf "A1A %ld\n", (long)frame_corr` → prima (misurato) `A1A 3320`;
   dopo una riga `A1A N` (N atteso 3320, non misurato). Se non compare,
   fermarsi e riportarlo (domanda 2).
7. Regressione casuale: `python3 ricette/genera_ingressi.py 10 /tmp/s10.txt 6000 8;
   harness/gioca -n 6000 --muto --riproduci /tmp/s10.txt 2>/dev/null | grep -a
   'FERMO\|FRAME rese'` → prima (misurato): `FRAME rese 6000`, nessun FERMO
   (`HASH ultimo 923cbefe0efb5a9f`); dopo: `FRAME rese 6000` (attesa).
8. Dimensioni: `find harness -name '*.c' -o -name '*.h' | xargs wc -l | awk
   '$1>800 && $2!="totale"'` → vuota.
9. Ambito: `git status --short` solo i quattro file di `decomp/harness`, i tre
   documenti e questo brief; `pgrep -c mame` → `0`.

## Consegna

Un commit `Gioco libero: stelline d'inizio quadro, un passo del cerchio per
frame (3097 dichiara il frame pieno)` con la riga
`Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>` (o il modello che
esegue). Rapporto finale in trenta righe al massimo: uscite delle verifiche
2-7 (prima e dopo), nuovi hash della partita 1A, deviazioni.

## Domande aperte

1. **Correzione mirata o modello di costo generale?** Proposta: mirata (il
   gancio «frame pieno» su 3097), unica discrepanza grossa misurata nell'inizio
   quadro; nelle altre fasi l'originale fa 9-13 giri di 07 per frame e il
   libero 11. Un modello di cicli generale (contare i punti o i cicli Z80 per
   corpo) è un lotto a parte. — scelta di progetto.
2. **Se la partita 1A non arriva più al corpo 1A** (verifica 6): proposta,
   rifare la registrazione con `gioca --registra` dopo la correzione (le
   registrazioni del gioco libero dipendono dalla cadenza, quella di oggi è
   stata fatta con le stelline sbagliate). — preferenza.
3. **Il fondale (corpo 07, fasi 6 e 08, `fondale.c` 322A) dura anch'esso più di
   un frame nell'originale** (punti di NMI interni) e nel libero si disegna in
   un colpo: possibile seconda differenza visiva su pavimento e mostri che si
   materializzano. Non misurata qui. Proposta: dopo questa correzione,
   confronto a vista dell'utente; se differisce, stessa tecnica con il gancio
   dentro il fondale in un compito successivo. — scelta di progetto.
4. **Numero del brief**: proposta, rinominare in `compiti/53-stelline.md` se il
   53 è libero (oggi l'ultimo è il 52; i «lotti 53-54» del piano sono lotti,
   non brief). — preferenza.

## Risoluzioni delle domande aperte (chat principale con l'utente, 2026-11-04)

1. CORREZIONE MIRATA (~20 righe, gancio MACCHINA_FRAME_PIENO): scelta
   dall'utente, coerente con la decisione storica «niente modello a cicli».
2. Registrazione 1A: preferenza ACCETTATA — se con la nuova cadenza la
   partita registrata non arriva più al corpo 1A, lo sviluppatore lo segnala
   nell'esito e la registrazione la rifà l'utente (non inventare ricette).
   Gli hash attesi della partita 1A vanno rimisurati e annotati.
3. FONDALE (fasi 6 e 08): l'utente ha scelto di MISURARE E CORREGGERE NELLO
   STESSO LOTTO se il meccanismo è lo stesso della cadenza; se risulta un
   difetto diverso, si annota e si rimanda.
4. Il file è stato rinominato in 53-stelline.md: preferenza accettata
   (i corpi residui slittano ai lotti 54 e 55).

## Nota per lo sviluppatore (richiesta utente, 2026-11-04)

Nell'aggiornare STATO.md, aggiungi alla sezione del lavoro in sospeso una
voce «Cadenza del gioco libero»: il modo libero chiude il frame con
un'euristica (oggi ~22 corpi per frame), non con il costo vero delle
istruzioni; la correzione di questo lotto è mirata alla sola animazione
d'inizio quadro. Altri difetti di cadenza possono esistere altrove e
VANNO RIVISTI in futuro (decisione utente: «ci ritorniamo») — ogni nuovo
difetto visivo o di ritmo nel gioco libero va prima sospettato come
cadenza, poi come traduzione.
