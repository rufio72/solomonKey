# Compito 66: bonifica delle 52 stanze (routine 2E04 e 2D0F, sette zone fuori voce)

2026-11-06, scritto dall'agente analista per un agente sviluppatore. Questo
file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Dopo il lotto 65 la spazzata delle 52 stanze (`gioca --stanza N -n 3000 --muto
--riproduci <seme 4>`) ha due FERMO (2E04 nella stanza 5, 2D0F nella 29) e
sette zone di ROM lette fuori voce in 13 stanze. Si traducono le due routine,
si allargano le voci, e la spazzata deve dare ZERO FERMO e ZERO AVVISO. Serve a
poter sostituire il binario della release v1.01.

## Vincoli assoluti

- Niente modifiche fuori da `/home/rufio72/OpenSolomonsKey-master`. Si toccano:
  `decomp/harness/attivita_attract07.c` (317 righe), `decomp/corpi_catture.txt`,
  le catture nuove `decomp/out/snap_0x2CCD_att07_*`, `decomp/estrai_rom.py`,
  `decomp/censimento_rom.py`, `tecmo/rom_*` (rigenerati, mai a mano),
  `decomp/CENSIMENTO-ROM.md`, `STATO.md`, `decomp/GIRO.md`, `decomp/ROUTINE.md`.
- MAME SOLO con `-video none` (lo garantisce `cattura_corpi.sh`; non lanciarlo a
  mano). Mai `pkill` largo su `mame`/`gioca`. Alla fine `pgrep -c mame` = `0`.
- `gioca` sempre con `-n N` e `--muto` (senza `-n` apre una finestra).
- Niente git distruttivo (no stash, no filter-branch). Niente push. Non toccare
  `decomp/ricette/*.reg`.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe (`censimento_rom.py` 640, `estrai_rom.py` 220: ok).
- Un FERMO o una zona NUOVA scoperti dopo questi fix: le zone dati piccole
  adiacenti a quelle del punto 2 si allargano direttamente; ogni altro FERMO
  non si traduce, si censisce (messaggio, stanza, frame, hash) nell'esito e in
  `STATO.md`.

## Cosa esiste già (fatti verificati)

### A. Le due routine (da `out/main64.dasm` r. 8888-8895 e 9059-9068, `grafo symbol sub_2e11`, `sub_3812`, lettura di `attivita_attract07.c`)

Dispatcher delle fasi del corpo 07 in `attivita_attract07.c` r. 293-300
(`case 5: fase_2d07()`, `case 11: fase_2dfa()`; il numero di `case` è quello
del codice; verificare in che base lo prende `c19d` prima di scrivere il poke).

2D0F, ramo di `fase_2d07` (r. 60-67) quando `c243 == 0x1D` (la stanza 29):

```
2D08 ld a,(c243) ; 2D0B cp 1D ; 2D0D jr nz,2D19
2D0F ld ix,2D72 ; 2D13 ld c,03 ; 2D15 call 3812 ; 2D18 ret
```

Oggi `nemici_manca(0x2d0f)` (r. 64). `sub_3812(struct z80 *z)` esiste già
(`z80_38xx.c` r. 99, dichiarata in `routine.h` r. 134): legge 24 byte da `z->ix`
con `leggi()`, per ogni bit a 1 (dal 7 al 0, con `cy = z->f & 1` iniziale che
entra dal bit 0) scrive `z->c` nella cella successiva da c0cd; esce con
`z->b = z->e = 0`, `z->a` ultimo byte, HL, IX avanzati. All'ingresso di 2D0F
A = 1D e `cp 1D` lascia riporto spento (`z.f = 0x40`, `z.a = 0x1D`).
La tabella **2D72-2D89 (24 byte) non appartiene a nessuna voce** (la voce
`tab_attract_2d4a` finisce a 2D71; 2D8A è codice, `sub_2d8a`): va creata
(vedi B) altrimenti `sub_3812` avvisa «fuori voce». Il sito di lettura è
`z80_38xx.c` r. 105 (`a = leggi(ix);`), che `censimento_rom.py` r. 209
classifica solo `tab_mappe_blocchi_1557`: va aggiunta la voce nuova.

2E04, ramo di `fase_2dfa` (r. 159-174, fase 11, il commento della funzione dice
«fase 11»; `STATO.md` dice «fase 16»: contano il `case 11` del codice):

```
2DFA xor a ; ld (c85e),a ; 2DFE pop de ; 2DFF call 2E11 ; 2E02 jr nz,2E06
2E04 inc (hl) ; 2E05 ret          ; 2E06 ld c,38 ; rst 28 ; ret
```

`sub_2e11` (`z80_2xxx.c` r. 466): azzera c19e, `c = c249`, `a = c248`, Z se
uguali. Oggi con Z (c248 == c249) → `nemici_manca(0x2e04)` (r. 168). `inc (hl)`
incrementa la cella a cui punta HL all'ingresso della fase: il commento di
r. 157 dice «c19d», ma NON è verificato. Va stabilito dalla cattura MAME
(prima/dopo) o dal modo con cui il dispatcher (2DB8, `rst 20`) lascia HL, e
scritto nel commento. Chi cambia cosa nello stato: la cattura lo mostra.

### B. Le sette zone (misurate oggi: spazzata 0-51, seme 4, 3000 frame; AVVISI su stderr)

| zona | stanze | letta da (da C del repo) | byte (da `xxd out/main64.bin`) | proposta |
|---|---|---|---|---|
| 66C4-66D3 (16) | 2 23 30 | `passo_6676`, `attivita_12_13.c` r. 131-133: `SET_HL(0x66c4)`, `A = 5`, `sub_3569` | `68 06 0F 05 69 06 46 04 99 09 66 06 44 04 77 07`, poi 66D4 è codice (`d1 eb 21 8b`) | voce NUOVA `tab_66c4`, 16 byte |
| 6BC7-6BC8 | 11 | `sub_6b9b`, `nemici_tipi_15_17.c` r. 153: `rst 8`/`rst 10` su 6BC5 | 6BC5-6BCC = `80 00 C0 00 00 01 40 01` (4 parole 0080, 00C0, 0100, 0140, indice `(IY+4)&3`) | allargare `tab_6bc5` da 6BC5-6BC6 a 6BC5-6BCC |
| 6F45-6F48 | 11 16 32 33 44 50 | `sub_6eca`, `nemici_tipi_08_14.c` r. 658: tabella 6F3D, indice `(IY+4)-0x16` | 6F3D-6F48 = `9A 00 E6 00 33 01 9A 00 E6 00 33 01` (6 parole), 6F49 è codice (`b7 c9`) | allargare `tab_velocita_6f3d` a 6F3D-6F48 |
| 746C-746F | 18 50 | `velocita_7468`, `nemici_comparsab.c` r. 289-294 (tabella 7468, `rst 8`/`rst 10`) | 7468-746F = `02 02 03 03 04 04 06 06`, 7470 è codice (`fd 7e 04`) | allargare `tab_velocita_7468` a 7468-746F |
| 756E-756F, 7576-7577 | 18 50 | `sub_756c`, `nemici_comparsab.c` r. 55-70 (`rst 10` su base+BC) | 756C in poi è codice: `09 4e 23 cb 79 28 01 05 d7 d5 d7 eb`; la voce `tab_urto_754c` finisce a 7567 | vedi domanda 2 |
| B126-B14D (40) | 6 20 38 50 | continuazione di `tab_campi_aee2` (AEE2-B125, `sub_0602`, `z80_0xxx.c` r. 276, censimento r. 200) | B120-B14D: record `00 02 .. e8 00 02 00 00 00 00 02 <x> <ff|00> e8 00 02 ...` terminati da `2d ff 00 ff` a B14A-B14D | allargare `tab_campi_aee2` a AEE2-B14D |

Le zone sono già nei sorgenti prima del lotto 65 (non sono regressioni). Il
fondo B14E-BFFF resta «mai letto» e resta fuori voce.

Voci in `censimento_rom.py`: lista `VOCI` (r. 45-135, tuple
`(spazio, nome, da, a, L, sito, natura, tipo)`: `tab_6653` r. 82, `tab_6bc5`
r. 87, `tab_velocita_6f3d` r. 88, `tab_urto_754c` r. 93, `tab_campi_aee2` r. 131);
siti di lettura `_s(file, righe, cat, *voci)` r. 146-210. `estrai_rom.py` r. 38-41:
`VOCI_ATTESE, BYTE_L_ATTESI = 90, 20094`. Voci della tabella a r. 127-250 e
«Zone mai lette» r. 396-410 di `CENSIMENTO-ROM.md`. Sezione di esito di
riferimento: «Esito del lotto 62» (r. 528).

### C. Regressioni di riferimento (esito del lotto 65, `compiti/65-routine-39b9.md`)

Hash `HASH ultimo` / `AUDIO hash`: oracolo `a239d0a940bec0b9`/`c4b40fea06623a96`;
`-n 600` `ecd5acf694285489`/`8aedf41c0424df54`; `partita_1a` `474cc37bfa97faeb`/
`b6758278078e86fe` (`-n 4000`); `riferimento_nuova` `a040200b69679371`/
`caa004b13ad0a661` (`-n 16000`); `bottiglia_rossa` `69a54ac9aa6bea9e`/
`b70dc75d81b355b9` (`-n 11167`); `stanze_bonus` `bbebf0f3c64e446a`/
`cb78ab354330a616` (`-n 12387`). Banco: 1434 catture, 335 RIEPILOGO.
Misure di oggi (binario di HEAD `f6096cc`): `gioca --stanza 29 -n 3000 --muto
--riproduci /tmp/s4.txt` → `FERMO routine 2D0F mancante al frame 1562`,
`HASH ultimo 7dacebce43e1fdd2`, `AUDIO hash ad743e71f87a0ef6`; `censimento_rom.py
--verifica` → `CENSIMENTO OK: 202 occorrenze, 0 non classificate`.

## Struttura da realizzare

Ordine consigliato, un commit per parte (1 e 2 separati):

**Parte 1: 2D0F e 2E04** (`attivita_attract07.c`)
1. Creare la voce `tab_2d72` (2D72-2D89, 24 byte, sito `3812 via 2D0F`, natura
   `statica`), aggiungere `'tab_2d72'` al sito `z80_38xx.c` r. 105 (`_s` r. 209),
   rigenerare (parte 2, passi 3-4) PRIMA di tradurre.
2. 2D0F: sostituire `nemici_manca(0x2d0f)` con `struct z80 z = {0}`, `z.ix =
   0x2d72`, `z.c = 3`, `z.a = 0x1d`, `z.f = 0x40`, `sub_3812(&z)` e il ritorno;
   commentare gli indirizzi (2D0F-2D18) come il resto del file. Verificare
   dalla cattura che le celle c0cd-c18c prendano 03 dove i bit di 2D72 sono 1.
3. 2E04: sostituire `nemici_manca(0x2e04)` con l'incremento della cella giusta
   (v. A) e `return`; aggiornare il commento di r. 157.
4. Catture MAME (aggiungere a `corpi_catture.txt`, formato `modo cod entry tag
   N poke`, vedi le righe 119-130): 
   `corpo 07 0x2CCD f5q1D 1 2300:0xc19d:5,2300:0xc243:0x1D,2300:0xc045:0x00` e
   `corpo 07 0x2CCD f11eq 1 2300:0xc19d:11,2300:0xc248:5,2300:0xc249:5`
   (poke di partenza proposti, da correggere se MAME non entra nella fase:
   c243 e c045 come `f0q33`, base di `c19d` come per `f1s07a`; per `f11eq`
   dichiarare cosa cambia tra prima e dopo). Il banco (`harness --tutte`)
   le confronta da solo.
5. Se `attivita_attract07.c` cambia riga: aggiornare i numeri di riga dei
   suoi siti in `censimento_rom.py` e il file locale non versionato
   `out/censimento_rom_letti.txt` (come nel lotto 65).

**Parte 2: zone fuori voce**
1. Modificare in `censimento_rom.py` le voci della tabella B (nome nuovo
   `tab_66c4`, siti: `sub_3569` via `attivita_12_13.c` r. 131-133 come
   `tab_6653`/`tab_bonus_355d` già fanno; i siti `_s` non cambiano per le
   voci solo allargate).
2. Aggiornare `VOCI_ATTESE` e `BYTE_L_ATTESI` in `estrai_rom.py`: con le
   proposte di questo brief voci 90 → 92 (`tab_2d72`, `tab_66c4`), byte 20094 →
   20204 (+24 +16 +6 +4 +4 +16 +40; attesa, non misurata).
3. `python3 -I estrai_rom.py` (da `decomp/`): rigenera `tecmo/rom_*`; deve
   uscire 0 e riscrivere solo i file cambiati; poi `harness/rom_immagine`
   (`make rom_immagine` in `harness/`) deve confrontare tutte le voci senza
   differenze.
4. Aggiornare `CENSIMENTO-ROM.md`: righe delle voci cambiate (tabella r. 127-250,
   tabella estrazione r. 320-390 con le colonne byte/righe/uscita), la riga
   «80A5-BFFF, fuori voce» di «Zone mai lette» (B126-BFFF 3802 → B14E-BFFF 3762)
   e una sezione «Esito del lotto 66» breve con la tabella prima/ora.
5. Spazzata finale (verifica 9) e, se compare altro, allargare solo le zone
   adiacenti; il resto si censisce.

**Parte 3: documenti** `STATO.md` (punto «Code minori» r. 76-80: tolti 2E04,
2D0F e le zone), `decomp/GIRO.md` (r. 135-137 e 656-657), `decomp/ROUTINE.md`
(se elenca 2D0F/2E04). Annotare gli hash nuovi.

## Verifiche obbligatorie

Da `decomp/` salvo dove indicato. `S4` = `python3 ricette/genera_ingressi.py 4
/tmp/s4.txt 8000 8` (una volta). Le uscite «dopo» sono attese, non misurate:
lo sviluppatore le dichiara.

1. Compilazione: `cd harness && make harness replay gioco_libero gioca sched_proto rom_immagine 2>&1 | grep -c 'warning\|error'` → `0`.
2. Generatore: `python3 -I estrai_rom.py; echo $?` → `0` (voci 92, byte 20204 attesi);
   `python3 -I estrai_rom.py; git status --short ../tecmo` identico al giro prima (idempotente);
   `harness/rom_immagine; echo $?` → `0`.
3. Catture: `grep -E 'f5q1D|f11eq' corpi_catture.txt > /tmp/c66.txt; sh cattura_corpi.sh /tmp/c66.txt | tail -1` →
   `CATTURE OK 2 su 2` (attesa; dichiarare le scartate e perché); poi `pgrep -c mame` → `0`.
4. Banco: `cd harness && ./harness --tutte > /tmp/h66.txt; echo "exit $?"; grep -c FALLITO /tmp/h66.txt; grep -c '^RIEPILOGO' /tmp/h66.txt; grep -c '^OK' /tmp/h66.txt`
   → `exit 0`, `0`, `335`, `1436` (1434 + 2 catture OK).
5. Giro: `sh giro_completo.sh | tail -1` → `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`.
6. Oracolo: `harness/gioca -n 5434 --muto --oracolo out/giroboot_oracolo.txt out/giroboot_disp.txt --riproduci out/ingressi_replay.txt 2>/dev/null | tail -6` → `HASH ultimo a239d0a940bec0b9`, `AUDIO hash c4b40fea06623a96`.
7. Libero breve: `harness/gioca -n 600 --muto` → `HASH ultimo ecd5acf694285489`, `AUDIO hash 8aedf41c0424df54`.
8. Ricette (`harness/gioca --riproduci ricette/X.reg -n <N> --muto`): hash del punto C, nessun FERMO, nessun AVVISO.
9. **Spazzata di bonifica**: `for N in $(seq 0 51); do echo "$N: $(harness/gioca --stanza $N -n 3000 --muto --riproduci /tmp/s4.txt 2>&1 | grep -a 'FERMO\|AVVISO')"; done | grep -v ': $'`
   (circa 3-4 min) → uscita VUOTA. Stanza 5 a 8000: `harness/gioca --stanza 5 -n 8000 --muto --riproduci /tmp/s4.txt 2>&1 | grep -a 'FERMO\|AVVISO\|HASH ultimo'` → solo `HASH ultimo`
   (prima: `FERMO routine 2E04 mancante al frame 7790`, `3d9c0ae560d66f59`). Stanza 29 a 3000: prima `7dacebce43e1fdd2`/`ad743e71f87a0ef6`, dichiarare i nuovi. Se compaiono altri FERMO o zone nuove, censirli (regola nei vincoli).
10. Semi lunghi (circa 3 min ciascuno): `python3 ricette/genera_ingressi.py 4 /tmp/l4.txt 40000 8; harness/gioca -n 40000 --muto --riproduci /tmp/l4.txt 2>&1 | grep -a 'FERMO\|AVVISO\|FRAME rese\|HASH ultimo\|AUDIO hash'` → senza FERMO né AVVISO (prima `0b8b015f62457828`/`4a6401329f8897cb`); idem seme 67 (prima `ec7b1103568e16ad`/`aa8cb02441cad47e`). Gli hash possono cambiare solo se le nuove celle entrano nel percorso: dichiararli.
11. Censimento: `python3 -I censimento_rom.py --verifica | tail -2` → `LETTI FUORI VOCE: 0 ...` e `CENSIMENTO OK: 202 occorrenze, 0 non classificate` (202 se non si aggiungono `leggi(`; se ne aggiungono, il numero nuovo va dichiarato).
12. Dimensioni: `find harness -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'` → vuota.
13. Ambito: `git status --short` → solo i file elencati nei vincoli (le catture `out/snap_0x2CCD_att07_f5q1D*`, `f11eq*`), `tecmo/rom_*`, i documenti e `?? compiti/66-bonifica-stanze.md`.

## Consegna

- Due commit in italiano (parte 1, parte 2 con documenti) con riga
  `Co-Authored-By: <modello che ha lavorato> <noreply@anthropic.com>`.
- In fondo a questo file la sezione «Esito» con le uscite delle verifiche e gli hash nuovi (9, 10).
- Rapporto finale: massimo trenta righe con uscita delle verifiche e deviazioni.

## Domande aperte

1. **Indice dei poke di `f11eq`**: `c19d` base 10 o 16 nel dispatcher del 07, e
   cosa punta HL in 2E04. Proposta: lo sviluppatore lo ricava dalla cattura e dal
   dispatcher (`attivita_attract07.c` r. 293-300) e lo scrive nel commento; se MAME
   non entra nella fase, 2E04 si traduce dal disassemblato come fu per 38D9. — preferenza.
2. **Zona 756E-756F e 7576-7577** (index fuori tabella di `sub_756c`: legge byte di
   codice come se fossero dati). Proposta: allargare `tab_urto_754c` da 754C-7567 a
   754C-7577 (natura `stimata`, nota che 756C-7577 sono byte di codice letti per
   indice fuori tabella, gli stessi che legge l'originale). Alternativa: voce a parte
   `tab_urto_756c`. — preferenza.
3. **Nome e confini di `tab_66c4`**: 16 byte esatti (66C4-66D3, record per `sub_3569`
   con A = 5). Proposta come in tabella B. — preferenza.
4. **Se la spazzata dopo i fix mostra FERMO nuovi** (i 52 quadri a 3000 frame non
   hanno dato altro oggi, ma stanza 5 a 8000 e i semi lunghi possono). Proposta:
   censirli e chiudere il lotto con ciò che è fatto, come per i lotti 62-65. — preferenza.

Nessuna scelta di progetto.

## Decisioni (chat principale, 2026-11-06)

Tutte e quattro le domande sono preferenze, accettate con le proposte:
1. Base dell'indice di c19d e bersaglio di HL in 2E04: li ricava lo
   sviluppatore dalla cattura MAME e dal dispatcher.
2. Zone 756E/7576: allargamento di `tab_urto_754c` fino a 7577.
3. `tab_66c4` voce nuova di 16 byte.
4. FERMO nuovi dalla spazzata: si censiscono nel rapporto e il lotto si
   chiude comunque.

## Esito (sviluppatore, 2026-11-06)

Commit: `df3dfe8` (parte 1: 2D0F, 2E04, `tab_2d72`, catture) e il commit
della parte 2 (zone e documenti). Domanda 1 risolta: `c19d` si scrive in
base 10 nei poke (`2300:0xc19d:11` entra nella fase 11); in 2E04 HL vale
c19d (caricato a 2CCD, il gestore 0020-0026 lo restituisce intatto, 2DFE fa
`pop de`, 2E11 non tocca HL). Cattura `f11eq`: c19d da 0B a 0D (fase 12
saltata); `f5q1D`: 130 celle di c0ce-c18d a 03 come i 130 bit di 2D72.

1. Compilazione: `0`.
2. `estrai_rom.py`: exit 0 («rom: 93 voci, 21849 byte», cioè 92 + il
   riempimento, 20204 byte L); secondo giro idempotente; `rom_immagine` exit 0.
3. `CATTURE OK 2 su 2`, nessuna scartata; `pgrep -c mame` → `0`.
4. Banco: `exit 0`, `0`, `335`, `1436` (rifatto anche dopo la parte 2).
5. `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`.
6. Oracolo: `a239d0a940bec0b9` / `c4b40fea06623a96`.
7. Libero `-n 600`: `ecd5acf694285489` / `8aedf41c0424df54`.
8. Ricette: tutti gli hash del punto C invariati, nessun FERMO né AVVISO.
9. Spazzata 0-51: uscita VUOTA. Stanza 29 `-n 3000`: `d515345ff3d3da8a` /
   `f69df8e33d3b3b78`, nessun FERMO. Stanza 5 `-n 8000`: **FERMO NUOVO
   `routine 2D9D mancante al frame 7871`** (fase 16 del 07, raggiunta ora
   perché 2E04 salta alla 13), `4f6770fd98a0f823` / `27f47f454157523b`:
   censito (STATO.md, GIRO.md, ROUTINE.md), non tradotto per vincolo.
10. Seme 4 `-n 40000`: `FRAME rese 40000`, `0b8b015f62457828` /
    `4a6401329f8897cb`; seme 67: `ec7b1103568e16ad` / `aa8cb02441cad47e`;
    invariati, nessun FERMO né AVVISO.
11. `LETTI FUORI VOCE: 0 ...`, `CENSIMENTO OK: 202 occorrenze, 0 non classificate`.
12. Dimensioni: vuota (`attivita_attract07.c` 327 righe).
13. Ambito: solo i file elencati.

Deviazioni: `tab_6bc5` passa a natura `statica`, `tab_velocita_6f3d` e
`tab_velocita_7468` a `zona`, `tab_urto_754c` a `stimata`. In
`CENSIMENTO-ROM.md` la tabella dei totali per file («stima del brief 61, non
aggiornata») è lasciata com'è; aggiornate le righe delle voci, la zona
80A5-BFFF e la sezione «Esito del lotto 66».

## Esito del ciclo (2026-11-06)

Ciclo passato al primo colpo (37º). Commit `df3dfe8` + `20f2ecf`;
verificatore: 13/13 passate — 2D0F e 2E04 tradotte (con voce nuova
tab_2d72), 7 zone allargate + tab_66c4 nuova (92 voci / 20204 byte letti,
93 col riempimento), banco a 1436, spazzata 0-51 a 3000 frame PULITA
(zero FERMO, zero avvisi), semi 4/67 a 40000 frame con hash invariati.
Resta la 2D9D (fase 16 del corpo 07, stanza 5 oltre il frame ~7871),
raggiungibile solo ora grazie ai fix: va nell'addendum 66B prima di
sostituire il binario della release. Token: analista Sonnet ~83k,
sviluppatore Opus ~90k, verificatore Sonnet ~33k.

## Esito 66B (sviluppatore, 2026-11-06)

- 2D9D (fase 16 del 07): `dec (hl)` su c19d; con c19e = 0A azzera c19e e
  la fase avanza, altrimenti `jp 416B`, i passi del corpo 18. Il
  dispatcher 416B-4170 di `attivita_18.c` è ora la funzione `passi_416b()`
  (dichiarata in `attivita_libera.h`), usata dal 18 e da `fase_2d9d`.
  Catture MAME `f16fine` (c19d 10→11, c19e 0A→00), `f16p0` (passo 41B7) e
  `f16p5` (passo 4185): tutte e tre OK nel banco.
- 3065 (`ret z` in 305F, `attivita_attract07c.c`): non si raggiunge, perché
  c19e vale 2 dopo 2DD3-2DD4 e il `dec (hl)` lo porta a 1. È tradotto come
  ritorno semplice. Nei file del corpo 07 non resta nessun `nemici_manca`.
- Siti `leggi(` spostati: 07.c 78-79, 07c.c 268, 281, 289 (aggiornati in
  `censimento_rom.py` e `out/censimento_rom_letti.txt`).

Verifiche:
- compilazione `0`;
- `estrai_rom.py` exit 0 (93 voci, 21849 byte), nessun cambio in `tecmo/`;
  `rom_immagine` exit 0;
- banco `exit 0`, `0` fallite, `335`, `1439`;
- giro completo: la riga attesa;
- oracolo `a239d0a940bec0b9` / `c4b40fea06623a96`;
- `-n 600` `ecd5acf694285489` / `8aedf41c0424df54`;
- le 4 ricette con gli hash del punto C, senza FERMO;
- semi 4 e 67 a 40000 frame con hash invariati;
- censimento `LETTI FUORI VOCE: 0`, `CENSIMENTO OK: 202 occorrenze`;
- nessun file sopra 800 righe;
- spazzata 0-51 a 8000 frame: uscita vuota. Stanza 5: `fe80c34bdbf49e09` /
  `dcf24f542f52de6f`.

## Esito del ciclo 66B (2026-11-06)

Addendum passato al primo colpo (38º ciclo). Commit `d56ef4e`; verificatore:
tutte passate — 2D9D (fase 16 del corpo 07) e 3065 tradotte, passi del corpo
18 condivisi in passi_416b(), NESSUN ramo mancante residuo nel corpo 07;
banco a 1439 (3 catture nuove f16*); SPAZZATA 52 STANZE A 8000 FRAME PULITA
(zero FERMO, zero avvisi; stanza 5 fe80c34bdbf49e09/dcf24f542f52de6f);
regressioni e semi invariati; censimento 202/0. Token sviluppatore Opus
~116k, verificatore Sonnet ~36k.
