# Compito 56: registro R minimo nel solo gioco libero

Data: 2026-11-04. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.
Riferimento di forma: `compiti/55-ultimi-corpi.md` (verifiche e misure dei semi).

## Obiettivo

Nel gioco libero (`harness/gioca` senza `--oracolo`) ogni lettura di `ld a,r` ferma
oggi il gioco con «FERMO lettura del registro R non modellata»: i semi 10, 4, 67 si
fermano ai frame 24396, 22412, 21676 (corpo 12 → `sub_4302`). Si dà al gioco libero
un valore di R surrogato, minimo e deterministico (non un modello a cicli), così la
partita prosegue. Banco, replay, modo `--oracolo`, `gioco_libero` e `sched_proto`
restano bit-identici.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. File di codice
  toccabili: `decomp/harness/macchina_libera.c`, `decomp/harness/nemici_r.c` (solo
  `nemici_r_somma_r1` e il commento d'intestazione), `decomp/harness/nemici.h` (solo il
  blocco del registro R, r. 351-360). Più i documenti elencati sotto e la sezione
  «Esito» di questo file.
- Non si toccano: i wrapper `banco_r_*` e `prova_con_r`/`trova_cattura` di
  `nemici_r.c`, `banco_corpi.c`, `macchina.c` (il suo `r_letto` resta la sentinella
  del modo oracolo), `replay.c`, `sched.c`, `sched_proto.c`, `harness.c`, le routine
  tradotte che leggono R, `rst20_tables.h`, `snap.lua`, `cattura.sh`.
- Nessun modello a cicli e nessun conteggio di istruzioni.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le 800
  righe. `gcc` dal Makefile, nessuna dipendenza nuova, niente binari né `.o` nel commit.
- MAME non serve a questo compito: non lanciarlo (se servisse, solo senza finestra,
  `-video none`). A fine lavoro `pgrep -c mame` → `0`. Niente git distruttivo
  (`stash`, `reset --hard`, `filter-branch`, `checkout` di file non tuoi).

## Cosa esiste già (fatti verificati)

### Censimento dei siti `ld a,r` (da grep su `decomp/out/main64.dasm`: 10 righe, 5 coppie; nessun `ld r,a` nella ROM)

Ogni sito è una coppia `ld a,r ; ld l,a ; ld a,r ; add a,l`: R2 = R1 + 3 sui 7 bit
bassi (già così in `nemici_r_somma_r1`). Al reset R = 0 e nessuno lo scrive, quindi
il bit 7 di R è sempre 0: basta un R1 in 00-7F. Tutti i siti passano da
`nemici_r_somma()` o `nemici_r_somma_r1()` (da grep: nessun altro punto legge R).

| sito | C (file:riga) | uso del valore | chi lo raggiunge |
|---|---|---|---|
| 2D1F | `attivita_attract07.c:65` `fase_2d07` | `(R1+R2) & 1F`, poi un giro su c0bd (2D2C-2D48, mai tradotto): nel C chiama `nemici_r_somma()` e subito `nemici_manca(0x2d1f)` | corpo 07 dell'attract, solo con bit 4 di c045 acceso |
| 430A | `z80_4xxx.c:522` `sub_4302` | indice `(somma & 1E) >> 1` in una riga di 16 della tabella ROM 82DA: oggetto pescato a caso | corpo 12 (`attivita_12_13.c:50`, 42AA): è il sito che ferma oggi i semi |
| 54C5 | `nemici_tipo06b.c:43` `sub_54c5` | numero a caso minore di C (indice di un bit acceso della maschera, via 51D7) | nemici tipo 06 e 09: 51D7 ← 51FA ← 50DF ← 4FD4, e 5995 |
| 59E5 | `nemici_tipo09b.c:137` (in `sub_5995`) | `somma & 0F`: scelta della mossa del nemico tipo 09 | `sub_5995` (tipo 09, `nemici_tipo09.c:322`) |
| 5DDF | `nemici_tipo07a.c:170` `sub_5ddf` | D = somma (bit 0 e 1 scelgono gli orientamenti), C = R1 | `sub_5e1a` (r. 207) ← `sub_5cfb`, nemico tipo 07 |

Wrapper del banco che provano un R compatibile (da `nemici_r.c` r. 166-176, NON
toccare): `banco_r_54c5 _51d7 _5995 _57cd _51fa _5ddf _5e1a _4fd4 _50df _5cfb _4302`.

### Codice da modificare

`decomp/harness/nemici_r.c` (180 righe), r. 26-47 oggi:
```c
static uint8_t r_valori[2];
static unsigned r_usati;

void (*nemici_r_gancio)(void) = NULL;

uint8_t nemici_r_somma_r1(uint8_t *r1)
{
    if (nemici_r_gancio)
        nemici_r_gancio();
    uint8_t v = r_valori[r_usati < 2 ? r_usati : 1];
    r_usati++;
    uint8_t r2 = (uint8_t)((v & 0x80) | ((v + 3) & 0x7f));
    *r1 = v;
    return (uint8_t)(v + r2);
}
```
`r_valori` è riempito solo da `prova_con_r` (banco); fuori dal banco vale 0.
Il commento d'intestazione r. 7 dice «fuori dal banco (replay) vale 0».

`decomp/harness/nemici.h` (374 righe), r. 351-360: prototipi `nemici_r_somma`,
`nemici_r_somma_r1` e `extern void (*nemici_r_gancio)(void);` con il commento
«Sentinella del registro R (brief 45) ... NULL nel banco e nel replay».

Chi assegna `nemici_r_gancio` (da grep): solo `macchina.c:625` (`r_letto`, modo
oracolo: stampa «R letto prima della NMI...» e ferma) e `macchina_libera.c:126`.

`decomp/harness/macchina_libera.c` (158 righe):
- r. 1-8 commento d'intestazione; r. 37-40 stato `frame_corr` (parte da 66), `in_nmi`,
  `punti, corpi, letture`, `uscita`;
- r. 42-47 `fermo(motivo)` stampa `FERMO <motivo> al frame N` e `longjmp(uscita, 3)`;
- r. 51-71 `nmi_libera()`: una NMI = un frame, `frame_corr++` a r. 69;
- r. 84-87 `r_letto_libero()` → `fermo("lettura del registro R non modellata")`;
- r. 112-157 `macchina_libera_esegui()`: dopo `boot_2833()` (r. 121) imposta i ganci,
  tra cui `nemici_r_gancio = r_letto_libero;` (r. 126); poi il ciclo dello
  spedidore, che dopo ogni corpo ferma su `nemici_mancante` («routine XXXX mancante»,
  r. 146-150) e su `nemici_fuori_sito`.

Build (da `decomp/harness/Makefile`): `gioca` include `macchina_libera.c`; `harness`,
`replay`, `gioco_libero`, `sched_proto` no (`gioco_libero` usa `macchina.c`). Tutti
linkano `nemici_r.c` tramite `SORGENTI_NEMICI` / `SORGENTI`.

### Misure di oggi (binari già compilati da HEAD 6ad0ea6, da `decomp/`)

Ingressi: `python3 ricette/genera_ingressi.py S /tmp/sS.txt 40000 8`, poi
`harness/gioca -n 40000 --muto --riproduci /tmp/sS.txt 2>&1 | grep -a 'FERMO\|FRAME rese\|HASH ultimo\|AUDIO hash'`:
- seme 10: `FERMO lettura del registro R non modellata al frame 24396`, `FRAME rese 24330`,
  `HASH ultimo a964cd9375d7a827`, `AUDIO hash c185e0fb15e096d1` (62 s);
- seme 4: frame 22412, `FRAME rese 22346`, `HASH ultimo a964cd9375d7a827`, `AUDIO hash e5aac2184d3982d2` (56 s);
- seme 67: frame 21676, `FRAME rese 21610`, `HASH ultimo a964cd9375d7a827`, `AUDIO hash 418cec876398b21e` (55 s).
- Partita 1A (da `decomp/harness`, `./gioca --riproduci ../ricette/partita_1a.reg -n 4000 --muto 2>/dev/null | tail -4`):
  `FRAME rese 4000`, `HASH ultimo 3f25ba053eeb56b9`, `AUDIO campioni 2988510`, `AUDIO hash c097308a8678392e`.
- Oracolo (da `decomp/`, `harness/gioca -n 5434 --muto --oracolo out/giroboot_oracolo.txt out/giroboot_disp.txt
  --riproduci out/ingressi_replay.txt 2>/dev/null | tail -2`): `AUDIO campioni 4042500`, `AUDIO hash c4b40fea06623a96`.
- Banco e giro (dall'esito del brief 55, non rimisurati): `./harness --tutte` exit 0, 0 `FALLITO`,
  335 `RIEPILOGO`, 1412 `cattura`, 1412 `OK`; `sh giro_completo.sh | tail -1` → `GIRO COMPLETO 0-5499:
  saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`.

## Struttura da realizzare

1. **Interfaccia nuova** (`nemici.h`, nel blocco r. 351-360, e `nemici_r.c`):
   ```c
   /* Sorgente di R1 nel gioco libero (brief 56): se non NULL,
    * nemici_r_somma_r1 prende R1 da qui invece che da r_valori.
    * NULL nel banco, nel replay e nel modo oracolo. */
   extern uint8_t (*nemici_r_valore)(void);
   ```
   In `nemici_r.c`: `uint8_t (*nemici_r_valore)(void) = NULL;` accanto a
   `nemici_r_gancio`, e in `nemici_r_somma_r1` la sola riga del valore diventa
   ```c
   uint8_t v = nemici_r_valore ? (uint8_t)(nemici_r_valore() & 0x7f)
                               : r_valori[r_usati < 2 ? r_usati : 1];
   ```
   Il gancio resta chiamato prima, `r_usati++` e la formula di R2 restano uguali.
   Con il puntatore NULL il comportamento è identico al byte: banco e replay invariati.
   Aggiornare il commento d'intestazione (r. 7) e quello di `nemici_r_gancio` in `nemici.h`.

2. **Surrogato in `macchina_libera.c`**: un LCG a 32 bit con stato statico.
   ```c
   /* Registro R surrogato (brief 56): nessun conteggio delle istruzioni. Lo
    * stato avanza di un passo a ogni NMI (il tempo passa) e di uno a ogni
    * lettura (due letture nello stesso frame danno valori diversi); R1 sono
    * i 7 bit alti. Deterministico a parità di ingressi. */
   #define R_SEME 0x4302u
   static uint32_t r_stato;
   static void r_avanza(void) { r_stato = r_stato * 1103515245u + 12345u; }
   static uint8_t r_libero(void) { r_avanza(); return (uint8_t)(r_stato >> 25); }
   ```
   - `r_avanza()` in `nmi_libera()` accanto a `frame_corr++` (r. 69);
   - in `macchina_libera_esegui()`: `r_stato = R_SEME;` all'inizio (prima di
     `boot_2833()`), e al posto della r. 126 `nemici_r_gancio = NULL;` più
     `nemici_r_valore = r_libero;` (stesso punto: dopo il boot);
   - togliere `r_letto_libero` (r. 84-87) e aggiornare il commento d'intestazione
     (una riga: «R: surrogato deterministico, vedi r_libero»).
   Costanti e nomi liberi; vincolati: 7 bit, deterministico, avanzamento a ogni NMI
   e a ogni lettura, nessun effetto fuori da `macchina_libera.c`.

3. **Sito 2D1F** (vedi domanda 1): con R modellato `fase_2d07` non si ferma più su
   R ma su «FERMO routine 2D1F mancante» se l'attract accende il bit 4 di c045.
   Non tradurlo in questo lotto salvo risoluzione diversa della domanda 1; se
   compare, riportarlo con seme e frame.

4. **Documenti**: `decomp/GIRO.md` r. 421-424 (la voce «Registro R» diventa: surrogato
   LCG nel solo gioco libero, lotto 56) e una sezione breve «Lotto 56» in fondo con le
   uscite della verifica 4; `STATO.md` dopo r. 239 una voce `[x] Lotto 56 (brief 56,
   2026-11-04)` e la riga «Resta il registro R...» (r. 237) aggiornata.

## Verifiche obbligatorie

Da `/home/rufio72/OpenSolomonsKey-master` salvo dove scritto. Uscita esatta salvo «attesa, non misurata».

1. Compilazione: `cd decomp/harness && make harness replay gioco_libero gioca sched_proto > /tmp/m56.txt 2>&1;
   echo "make $?"; grep -c -E 'warning|error' /tmp/m56.txt` → `make 0` e `0`.
2. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h56.txt; echo "exit $?"; grep -c FALLITO /tmp/h56.txt;
   grep -c '^RIEPILOGO' /tmp/h56.txt; grep -c '^cattura' /tmp/h56.txt; grep -c '^OK' /tmp/h56.txt` →
   `exit 0`, `0`, `335`, `1412`, `1412`.
3. Invarianti: da `decomp/`, `sh giro_completo.sh | tail -1` → `GIRO COMPLETO 0-5499: saldatura 0 byte,
   spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`; oracolo (comando
   delle misure) → `AUDIO campioni 4042500` / `AUDIO hash c4b40fea06623a96`; partita 1A → `FRAME rese 4000`,
   `HASH ultimo 3f25ba053eeb56b9`, `AUDIO hash c097308a8678392e`, nessun FERMO.
4. Semi: per `S in 10 4 67` il comando delle misure (ingressi nuovi in `/tmp/s$S.txt`) → nessuna riga
   `FERMO lettura del registro R`. Atteso `FRAME rese 40000` (attesa, non misurata). Se compare un altro
   `FERMO` (per esempio `routine 2D1F mancante`, un altro `routine XXXX mancante`, `indice ... fuori dalla
   tabella`) riportare messaggio, seme e frame nell'esito, senza nasconderlo né tradurre fuori perimetro.
   Riportare per ogni seme `FRAME rese`, `HASH ultimo`, `AUDIO hash`.
5. Determinismo: ripetere il seme 10 (stesso file di ingressi) → `FRAME rese`, `HASH ultimo` e
   `AUDIO hash` identici alla prima esecuzione della verifica 4.
6. Variabilità: `grep -c 'nemici_r_valore' decomp/harness/*.c decomp/harness/*.h | grep -v ':0'` →
   solo `macchina_libera.c`, `nemici_r.c`, `nemici.h` (nessun altro modulo assegna la sorgente).
7. Dimensioni: `find decomp/harness -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 && $2!="totale"'` → vuota.
8. Ambito: `git status --short` → solo `decomp/harness/macchina_libera.c`, `decomp/harness/nemici_r.c`,
   `decomp/harness/nemici.h`, `decomp/GIRO.md`, `STATO.md`, `compiti/56-registro-r.md`; `pgrep -c mame` → `0`;
   nessun `.o` né binario. `git diff --stat decomp/harness/nemici_r.c` → poche righe, nessuna nelle r. 140-176.

## Consegna

Commit unico con messaggio «Lotto 56: registro R surrogato nel solo gioco libero» e riga
`Co-Authored-By: <modello> <noreply@anthropic.com>`. In fondo a questo file la sezione «Esito» con le
uscite di 1-8 e le deviazioni. Rapporto finale: massimo trenta righe.

## Domande aperte

1. **Sito 2D1F dell'attract** (`fase_2d07`): dopo il modello l'attract, se accende il bit 4 di c045,
   arriva a `nemici_manca(0x2d1f)` e il gioco si ferma su «routine 2D1F mancante». Il ramo 2D1F-2D48 è
   ~19 istruzioni con le tabelle ROM 2D4A (8 byte) e 2D52 (32 byte, indici in c0bd). Proposta: fuori
   perimetro; se la verifica 4 si ferma lì, un lotto di coda lo traduce per composizione. Alternativa:
   tradurlo qui se compare (rischio: lotto che cresce, nessuna cattura disponibile senza R nel banco).
   **Scelta di progetto** (perimetro).
2. **Forma del surrogato**: LCG a 32 bit avanzato a ogni NMI e a ogni lettura, R1 = 7 bit alti.
   Alternativa: `(frame*K + letture) & 7F` (più vicino all'hardware ma con correlazioni forti tra frame
   vicini: due nemici che leggono nello stesso ordine in frame consecutivi vedrebbero valori in
   progressione). Rischio dell'LCG: nessuno sulla regressione; la distribuzione non è quella del Z80
   (che dipende dal numero di istruzioni) ma il gioco usa R solo come dado. **Scelta di progetto** (semantica).
3. **Interfaccia**: puntatore nuovo `nemici_r_valore` in `nemici_r.c` invece di cambiare la firma di
   `nemici_r_gancio` (che `macchina.c` usa e non si tocca). **Preferenza**.
4. **Nomi e costanti** (`R_SEME`, `r_libero`, moltiplicatore LCG). **Preferenza**.

## Risoluzioni delle domande aperte (chat principale sotto mandato utente, 2026-11-04)

1. Sito 2D1F (attract): FUORI PERIMETRO. Se con R modellato l'attract si
   ferma su «routine 2D1F mancante», lo sviluppatore lo SEGNALA nell'esito
   senza tradurlo: se ne farà un lotto di coda.
2. Forma del surrogato: GENERATORE PSEUDO-CASUALE a 32 bit (proposta
   dell'analista), avanzato a ogni frame e a ogni lettura, valore a 7 bit.
   Variabilità migliore della formula frame*K+letture, stesso determinismo.
3. Puntatore nuovo nemici_r_valore invece di cambiare la firma del gancio:
   preferenza ACCETTATA.
4. Nomi e costanti: preferenze ACCETTATE.
