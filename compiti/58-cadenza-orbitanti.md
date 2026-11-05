# Compito 58: cadenza del motore dei nemici nel gioco libero (orbitanti della stanza 3)

2026-11-05, scritto dall'agente analista per un agente sviluppatore. Questo
file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Nel gioco libero (`gioca` senza `--oracolo`), nella stanza 3 (c243 = 2) i due
nemici che girano intorno al blocco di pietra in alto a destra vanno 2,26 volte
più veloci che in MAME. Non è un errore di traduzione: è la cadenza della
macchina libera, che chiude il frame dopo 22 corpi qualunque sia il loro costo.
Si aggiunge un gancio di costo nel gestore per tipo dei nemici (4F0F), così la
macchina libera conta il lavoro del motore dei nemici e fa girare i corpi tante
volte per frame quante ne fa l'originale. Circa 30 righe di C più i documenti.

Vincoli assoluti:
- Niente modifiche fuori da `/home/rufio72/OpenSolomonsKey-master`; si toccano
  solo `decomp/harness/{ram.h,comune.c,nemici.c,macchina_libera.c}` e i
  documenti `decomp/GIRO.md`, `decomp/ROUTINE.md`, `STATO.md`.
- Non toccare `macchina.c`, `replay.c`, `sched.c`, `harness.c` né le catture:
  modo oracolo, replay, giro e banco restano identici byte per byte (il gancio
  è NULL fuori dalla macchina libera).
- Commenti, messaggi e documenti in italiano.
- MAME solo con `-video none` (in questo compito non serve lanciarlo).
- Nessun file di codice sopra le 800 righe (`nemici.c` ne ha 527,
  `macchina_libera.c` 166).
- Niente git distruttivo (stash, reset, filter-branch, checkout di file).
  `decomp/ricette/bottiglia_rossa.reg` è un file non tracciato dell'utente:
  non toccarlo e non aggiungerlo al commit.
- Niente push.

## Diagnosi (fatti misurati)

### Chi sono gli orbitanti

Nella stanza c243 = 2 ci sono tre oggetti con (IY+3) = 0E, cioè tipo 8 del
gestore 4F1B (indice = (IY+3) − 6): `sub_6eca` (`nemici_tipi_08_14.c:655`),
«velocità 6F3D, nuova posizione, celle attorno (70FE), direzioni 70EE»: segue
il bordo dei blocchi. In MAME sono gli oggetti 14 e 15 (posizioni y ≈ A4-BC,
x ≈ AF-C4: in alto a destra, i due orbitanti) e l'oggetto 8. Da `grafo symbol`:
`sub_6eca` somma la velocità (IX+0E/0F o IX+10/11, 009A = 154/256 px) alla
posizione (IY+0D/0E, IY+14/15) **a ogni chiamata**, senza il tempo trascorso
c2b7. Il passo per frame dipende quindi da quante volte per frame gira il ciclo
del corpo 0A.

### Il ciclo del corpo 0A

`attivita_0a_4da2` (`nemici.c:235`) smista sulla fase c2b8: 4DAD (sposta i
temporizzatori di c2b7 frame), 4632, 4EE6 (`sub_4ee6`, `nemici.c:208`: per i
17 oggetti da c4fe con (IY+0) ≥ C0 chiama `sub_4f0f`, poi c2b8 = 0). Un ciclo
completo = 3 spedizioni di 0A. `sub_4f0f` (`nemici.c:81-96`) esce se
(IY+4) < 6, altrimenti `rst 38` su 4F1B col tipo. Unico chiamante: `sub_4ee6`
(`nemici.c:219`).

### MAME: la cadenza dipende dalla stanza (misurato)

Script Lua con `-video none` (stanza forzata come `sessione.lua`: tap di
lettura su c243 fra i frame 1500 e 1950, poi scrittura; moneta 1510-1530,
start 1560-1580, nessun altro ingresso), conteggio delle spedizioni (lettura a
PC 3AFB, valore ≠ 0), delle spedizioni di 0A e dei cicli (scrittura di 0 in
c2b8), frame 2000-2700. S = slot attivi (spedizioni / spedizioni di 0A):

| c243 | spedizioni/frame | 0A/frame | cicli/frame | S | oggetti gestiti (tipo 8 / altri) |
|---|---:|---:|---:|---:|---|
| 0 | 19,51 | 4,88 | 1,63 | 4 | 0 / 4 |
| 2 | 9,72 | 2,43 | 0,81 | 4 | 3 / 6,65 |
| 7 | 14,20 | 3,36 | 1,12 | 4,23 | 0 / 7,83 |
| 8 | 5,85 | 1,46 | 0,49 | 4 | 12 / 1 |
| 10 | 25,27 | 5,41 | 1,80 | 4,67 | 0 / 1,24 |
| 11 | 10,47 | 2,62 | 0,87 | 4 | 6 / 3,02 |
| 22 | 8,45 | 2,11 | 0,70 | 4 | 5 / 6 |

Passo medio per frame degli oggetti di tipo 8 nella stanza 2 (somma |dy|+|dx|
della posizione a 16 bit fra frame consecutivi): 127,7 / 126,5 / 126,4 (1/256
px). Nella stanza 2 il ciclo non gira mai più di una volta per frame (cicli 0 o
1), nelle stanze 0 e 10 quasi due volte.

### Gioco libero: sempre 22 spedizioni per frame (misurato)

`macchina_libera.c` chiude il frame a `CORPI_PER_FRAME` = 22 spedizioni
(righe 26, 91, 106): con S = 4 sono 5,5 spedizioni di 0A per frame in ogni
stanza. Misura con gdb (script in «Verifiche», stanza forzata a `fase_07_3861`,
partita 1A fino al frame 2533 poi nessun ingresso, frame 2900-3699):
- stanza c243 = 2: **0A/frame 5,50**, passo medio dei tipo 8 288,8 / 285,5 /
  285,5 → 2,26 volte MAME. Tutti i corpi della stanza (anche Dana) girano 2,26
  volte troppo spesso; gli orbitanti si notano perché il tipo 8 si muove a
  chiamata.
- stanza c243 = 0: 0A/frame 5,50 (MAME 4,88).
- stanze 7 e 8: non misurabili nel libero forzando la stanza, «FERMO routine
  3901 mancante al frame 2534» (fase_07_3861 confronta c243 con il byte ROM
  82D3 = 03; coda 3901 nota e non tradotta).

### Perché non basta il frame pieno del brief 53

Un `MACCHINA_FRAME_PIENO()` in fondo a `sub_4ee6` limita il ciclo a 1 per frame:
stanza 2 da 1,83 a 1,0 (MAME 0,81), ma stanze 0 e 10 da 1,8 a 1,0 (MAME 1,63 e
1,80): nemici del 40 % più lenti. Il costo vero dipende dal numero e dal tipo
degli oggetti gestiti.

### Modello di costo scelto

Budget del frame invariato (22), ogni spedizione costa 1 come oggi, ogni oggetto
che passa il controllo (IY+4) ≥ 6 di 4F0F costa in più 3 se di tipo 8 ((IY+3) =
0E), 1 altrimenti. Cicli per frame attesi = 22 / (3·S + 3·n8 + n_altri),
confronto (attesa, dal modello; tra parentesi MAME, poi libero di oggi):

| c243 | modello | MAME | libero oggi |
|---|---:|---:|---:|
| 0 | 1,38 | 1,63 | 1,83 |
| 2 | 0,80 | 0,81 | 1,83 |
| 7 | 1,07 | 1,12 | 1,73 |
| 8 | 0,45 | 0,49 | 1,83 |
| 10 | 1,44 | 1,80 | 1,57 |
| 11 | 0,67 | 0,87 | 1,83 |
| 22 | 0,67 | 0,70 | 1,83 |

Il costo oltre il frame deve passare al frame seguente (stanza 8: un 4EE6 vale
~37 unità, cioè quasi due frame senza altri corpi, come in MAME).

## Cosa esiste già (fatti verificati)

| file | righe | ruolo |
|---|---:|---|
| `decomp/harness/ram.h` | 81 | `#define M(a) ram[...]` riga 9; gancio `macchina_frame_pieno` + `MACCHINA_FRAME_PIENO()` righe 73-79; `#endif` riga 81 |
| `decomp/harness/comune.c` | 55 | riga 15-16: `/* Frame pieno del gioco libero (ram.h); NULL = spento. */` `void (*macchina_frame_pieno)(void) = NULL;` |
| `decomp/harness/nemici.c` | 527 | `sub_4f0f` righe 81-96: uscita `if (z->a < z->d) { NEMICI_PONI_RIPORTO(z, 1); return; }` righe 85-88, poi `z->a = (uint8_t)(M(z->iy + 0x03) - z->d);` riga 89; include `routine.h` (che include `ram.h`) e `nemici.h` |
| `decomp/harness/macchina_libera.c` | 166 | intestazione righe 1-9; `CORPI_PER_FRAME 22` riga 26; `static long punti, corpi, letture;` riga 39; `nmi_libera` 60-79 (azzeramento `punti = corpi = letture = 0;` riga 78); `scansione_libera` 88-93 (condizione riga 91 `if (++letture >= LETTURE_PER_FRAME \|\| corpi >= CORPI_PER_FRAME)`); `frame_pieno_libero` 98-101; `involucro_libero` 103-114 (riga 106 `if (++corpi > CORPI_PER_FRAME)`); ganci dopo il boot righe 130-135 (`macchina_frame_pieno = frame_pieno_libero;` riga 135); azzeramento riga 138 |

- `comune.c` e `nemici.c` entrano in tutti i bersagli; `macchina_libera.c` solo
  in `gioca` e `gioco_libero` (Makefile riga 80, `SORGENTI_GIOCA`). Un gancio
  NULL fuori dalla macchina libera non cambia banco, replay, giro né oracolo.
- Simboli nel binario `gioca` (da `nm`/gdb): `fase_07_3861`,
  `attivita_0a_4da2`, `sub_0066`, `sub_4f0f`, `attivita_1a_411d`, `frame_corr`
  (statico), `ram`. CFLAGS `-std=c11 -Wall -Wextra -O0` (Makefile riga 2).
- Formato `.reg`: una riga per frame, `frame e600 e601 e602` in esadecimale;
  la partita 1A mette la moneta ai frame 2036-2040 (`00 00 01`) e lo start a
  2081-2085 (`00 00 02`); la presentazione del quadro (`fase_07_3861`) parte
  al frame 2534 (misurato con gdb).

## Struttura da realizzare

1. `ram.h`, prima di `#endif`, stesso stile del frame pieno:
   ```c
   /* Costo (gioco libero, brief 58): una routine che nell'originale costa
    * quanto n spedizioni lo dichiara con MACCHINA_COSTO(n); la macchina
    * libera lo somma al lavoro del frame e passa l'eccesso al frame seguente.
    * Altrove è NULL e non cambia nulla (definito in comune.c). */
   extern void (*macchina_costo)(int n);
   #define MACCHINA_COSTO(n) \
       do { if (macchina_costo) macchina_costo(n); } while (0)
   ```
2. `comune.c`, dopo la riga 16: `/* Costo del gioco libero (ram.h); NULL = spento. */`
   e `void (*macchina_costo)(int n) = NULL;`
3. `nemici.c`, in `sub_4f0f` dopo l'uscita delle righe 85-88 e prima della
   riga 89:
   ```c
   /* gioco libero (brief 58): costo del gestore in spedizioni, tarato su
    * MAME in 7 stanze; tipo 8 (6ECA, IY+3 = 0E) 3, gli altri 1. */
   MACCHINA_COSTO(M(z->iy + 0x03) == 0x0e ? 3 : 1);
   ```
4. `macchina_libera.c`:
   - riga 39: `static long punti, corpi, letture, debito;` (debito = costo
     dichiarato non ancora speso);
   - `nmi_libera`, al posto della riga 78:
     ```c
     /* l'eccesso del solo costo dichiarato passa al frame seguente (brief 58) */
     long eccesso = corpi + debito - CORPI_PER_FRAME;
     debito = eccesso <= 0 ? 0 : (eccesso < debito ? eccesso : debito);
     punti = corpi = letture = 0;
     ```
   - riga 91: `... || corpi + debito >= CORPI_PER_FRAME)`; riga 106:
     `if (++corpi + debito > CORPI_PER_FRAME)`;
   - accanto a `frame_pieno_libero`: `static void costo_libero(int n) { debito += n; }`
     con commento;
   - dopo la riga 135: `macchina_costo = costo_libero;`; riga 138: azzerare
     anche `debito`;
   - intestazione (righe 6-8): la cadenza è 22 unità per frame, una per corpo
     più il costo dichiarato con MACCHINA_COSTO() (oggi 4F0F, motore dei
     nemici; brief 58), salvo il frame pieno.
   Senza costi dichiarati (debito 0) il comportamento è identico a oggi.
5. Documenti:
   - `decomp/GIRO.md`, in coda: sezione `## Brief 58: cadenza del motore dei
     nemici nel gioco libero` con le due tabelle della diagnosi, le misure
     prima/dopo della verifica 2 e i nuovi hash;
   - `decomp/ROUTINE.md` riga 196 (voce 4F0F): aggiungere «nel gioco libero
     dichiara il costo (3 tipo 8, 1 gli altri; brief 58)»;
   - `STATO.md`: nella voce «Cadenza del gioco libero» (righe 53-61) una frase
     sul costo del motore dei nemici; una riga `[x] Brief 58` dopo quella del
     brief 53 (riga ~221).

I pesi 3 e 1 sono la proposta (domanda 2); il resto della struttura no.

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master/decomp` salvo dove scritto. Uscite
«prima» misurate il 2026-11-05 sul binario `harness/gioca` del 2026-11-04 17:11.

1. Compilazione: `cd harness && make harness replay gioco_libero gioca sched_proto
   > /tmp/m58.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/m58.txt`
   → `make 0` e `0`.
2. Cadenza nella stanza forzata. Ingressi:
   `awk '$1<2534' ricette/partita_1a.reg > /tmp/r58.reg; awk 'BEGIN{for(f=2534;f<=3800;f++) printf "%d 00 00 00\n",f}' >> /tmp/r58.reg`.
   Script `/tmp/c58.gdb` (STANZA = 2, poi una copia con 0):
   ```
   set debuginfod enabled off
   set pagination off
   set $forzata = 0
   set $n0a = 0
   break fase_07_3861
   commands
   silent
   if (long)frame_corr > 2085 && $forzata == 0
     set var ((unsigned char*)&ram)[0x243] = STANZA
     set $forzata = 1
     printf "FORZA %ld\n", (long)frame_corr
   end
   continue
   end
   break attivita_0a_4da2
   commands
   silent
   set $n0a = $n0a + 1
   continue
   end
   break sub_0066
   commands
   silent
   if (long)frame_corr >= 2900
     printf "F %ld %02x %d\n", (long)frame_corr, ((unsigned char*)&ram)[0x243], $n0a
   end
   set $n0a = 0
   continue
   end
   run
   quit
   ```
   `gdb -batch -x /tmp/c58.gdb --args harness/gioca --riproduci /tmp/r58.reg -n 3700
   --muto 2>/dev/null | grep -a '^F \|FORZA\|FERMO' > /tmp/c58.txt; grep -a 'FORZA\|FERMO' /tmp/c58.txt;
   awk '$1=="F"{s+=$4;n++} END{printf "0A/frame %.2f su %d frame\n",s/n,n}' /tmp/c58.txt`
   → prima (misurato): `FORZA 2534`, nessun FERMO, stanza 2 `0A/frame 5,50`
   (colonna 3 = `02`), stanza 0 `0A/frame 5,50`. Dopo (attesa, non misurata):
   `FORZA` presente, nessun FERMO, stanza 2 fra 2,2 e 2,7 (MAME 2,43),
   stanza 0 fra 3,8 e 4,5 (MAME 4,88). Se `FORZA` cade a un altro frame,
   controllare che la colonna 3 sia `02` / `00` e riportarlo.
3. Banco: `cd harness && ./harness --tutte > /tmp/h58.txt; echo "exit $?"; grep -c
   FALLITO /tmp/h58.txt; grep -c '^RIEPILOGO' /tmp/h58.txt; grep -c '^OK' /tmp/h58.txt`
   → `exit 0`, `0`, `335`, `1412` (misurato).
4. Invarianti (misurati, devono restare identici):
   - `sh giro_completo.sh | tail -1` → `GIRO COMPLETO 0-5499: saldatura 0 byte,
     spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`;
   - `harness/sched_proto out/sched_log.txt | tail -1` → `spedizioni confrontate:
     106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131`;
   - `harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 | tail -4`
     → `spedizioni eseguite: 64686, fino al frame 5499` / `NMI confrontate: 3090
     (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0,
     a metà attività 885` / `prima divergenza a metà attività (attesa): frame
     2415` / `nessuna divergenza nello spedidore`;
   - `harness/gioca -n 5434 --muto --oracolo out/giroboot_oracolo.txt
     out/giroboot_disp.txt --riproduci out/ingressi_replay.txt 2>/dev/null | tail -6` →
     `FRAME_VIDEO 5499: hash C a239d0a940bec0b9 MAME a239d0a940bec0b9 tabella
     a239d0a940bec0b9, coincide` / `FRAME rese 5434` / `HASH ultimo
     a239d0a940bec0b9` / `FRAME ultimo 5499` / `AUDIO campioni 4042500` /
     `AUDIO hash c4b40fea06623a96`.
5. Partita registrata 1A: `harness/gioca --riproduci ricette/partita_1a.reg -n 4000
   --muto 2>/dev/null | grep -a 'FERMO\|FRAME rese\|HASH ultimo\|AUDIO hash'` →
   prima (misurato): `FRAME rese 4000`, `HASH ultimo 3f25ba053eeb56b9`, `AUDIO
   hash c097308a8678392e`. Dopo: `FRAME rese 4000`, nessun FERMO; gli hash
   cambiano (attesa, non misurata: nella stanza 0 i corpi girano ~4,1 volte per
   frame invece di 5,5). Riportare i nuovi hash.
6. Ricetta 1A: script gdb con `break attivita_1a_411d` e `printf "A1A %ld\n",
   (long)frame_corr` sulla stessa riga di comando della 5 → prima (misurato)
   `A1A 3320`. Dopo: riportare la riga o la sua assenza (domanda 3); non è un
   fallimento del compito.
7. Registrazione fresca: `harness/gioca --riproduci ricette/bottiglia_rossa.reg -n 10502
   --muto 2>/dev/null | grep -a 'FERMO\|FRAME rese\|HASH ultimo\|AUDIO hash'` →
   prima (misurato): `FRAME rese 10502`, `HASH ultimo fea0687d9840b5ce`, `AUDIO
   hash 7b324fb363bc2758`. Dopo: nessun FERMO, hash nuovi da riportare (attesa).
8. Regressione casuale: `python3 ricette/genera_ingressi.py 10 /tmp/s10.txt 6000 8;
   harness/gioca -n 6000 --muto --riproduci /tmp/s10.txt 2>/dev/null | grep -a
   'FERMO\|FRAME rese\|HASH ultimo'` → prima (misurato): `FRAME rese 6000`,
   `HASH ultimo 923cbefe0efb5a9f`, nessun FERMO; dopo: `FRAME rese 6000`, nessun
   FERMO (hash da riportare).
9. Dimensioni: `find harness -name '*.c' -o -name '*.h' | xargs wc -l | awk
   '$1>800 && $2!="totale"'` → vuota (misurato oggi: vuota).
10. Ambito: `git status --short` → solo i quattro file di `decomp/harness`, i tre
    documenti, questo brief e `?? decomp/ricette/bottiglia_rossa.reg` (già
    presente, non toccato); `pgrep -c mame` → `0`.

## Consegna

Un commit `Gioco libero: costo del motore dei nemici (4F0F), cadenza delle
stanze con nemici come MAME` con i file delle verifica 10 tranne
`bottiglia_rossa.reg`, e la riga `Co-Authored-By: Claude Opus 5.5
<noreply@anthropic.com>` (o il modello che esegue). Rapporto finale in trenta
righe al massimo: uscite delle verifiche 2-8 (prima e dopo), nuovi hash,
deviazioni.

## Domande aperte

1. **Modello di costo nel motore dei nemici o correzione mirata?** Proposta:
   costo per oggetto in 4F0F (questo brief): cambia la cadenza del gioco
   libero in tutte le stanze con nemici, avvicinandola a MAME (errore da
   +126/+273 % a ±25 %). Alternative misurate e peggiori: frame pieno in coda a
   4EE6 (stanze 0 e 10 il 40 % più lente), costo solo sul tipo 8 (stanza 8
   0,31 contro 0,49, stanza 7 ancora +54 %). — scelta di progetto.
2. **Pesi 3/1.** Proposta: interi, tarati su 7 stanze; con pesi per tipo
   (es. tipo 17 a 0) la stanza 0 si avvicina a MAME ma la 7 peggiora; si
   ritoccano dopo il test a vista. — preferenza.
3. **Registrazioni del gioco libero.** Dopo la correzione `partita_1a.reg` e
   `bottiglia_rossa.reg` si desincronizzano (Dana riceve gli stessi ingressi
   ma i nemici sono altrove). La bottiglia rossa ha un'indagine aperta su
   quella registrazione. Proposta: fare la diagnosi della bottiglia (cattura
   MAME) prima di questo lotto o con il binario di oggi, poi l'utente rifà le
   due registrazioni con il binario nuovo; lo sviluppatore riporta soltanto se
   `A1A` arriva ancora. — scelta di progetto.
4. **Stanze oltre la 3 nel libero non misurabili forzando c243** (FERMO 3901,
   byte 82D3 = 03). Proposta: verifica a vista dell'utente giocando; nessuna
   traduzione di 3901 in questo lotto. — preferenza.

## Decisioni (chat principale con l'utente, 2026-11-05)

1. **Modello a costo** (`MACCHINA_COSTO`), non fix mirato: si corregge la cadenza del motore dei nemici in tutte le stanze. — scelta di progetto, decisa dall'utente.
2. Pesi **3 per il tipo 8, 1 per gli altri** — preferenza accettata.
3. Si procede subito col fix: `partita_1a.reg` e `bottiglia_rossa.reg` si desincronizzeranno e l'utente rifarà le registrazioni dopo (il bug della bottiglia rossa non si è riprodotto stamattina, chiuso come non riproducibile salvo ricomparsa). — scelta di progetto, decisa dall'utente.
4. Le stanze oltre la 3 si controllano a vista giocando — preferenza accettata.
