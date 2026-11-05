# Compito 59: famiglia 3901/3903 (fase 0 del corpo 07) e lettura e603 a 4CED

2026-11-05, scritto dall'agente analista per un agente sviluppatore. Questo
file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Nel gioco libero (`gioca` senza `--oracolo`) il FERMO «routine 3903 mancante»
(e il gemello «3901 mancante») interrompe la partita: scatta in `fase_07_3861`
(fase 0 del corpo 07, quadro di presentazione al cambio stanza) quando c243
è oltre la prima voce della tabella ROM 82D3. Si traduce il ciclo 38FB-3901
sull'intera tabella, il blocco 3903-392F (stanze bonus/finale) e, perché la
riproduzione di riferimento vada oltre, il sito 2D1F (fase 5 del corpo 07,
acceso dal blocco 3903). In coda, parte B: la lettura e603 al PC 4CED.

Due commit separati: **A** (3901/3903 + 2D1F + 3960 + catture) e **B** (4CED).

## Vincoli assoluti

- Niente modifiche fuori da `/home/rufio72/OpenSolomonsKey-master`. Si toccano:
  `decomp/harness/{attivita_attract07a.c,attivita_attract07.c,nemici_r.c,
  routine_tab.c,nemici.h,attivita_piccoli.c}`, `decomp/corpi_catture.txt`, le
  catture nuove `decomp/out/snap_0x2CCD_att07_f0*` e `..._f5*`, e i documenti
  `STATO.md`, `decomp/GIRO.md`, `decomp/DATI.md`, `decomp/ROUTINE.md`.
- MAME SOLO con `-video none` (lo garantisce `cattura.sh`; non lanciarlo a
  mano). Niente finestre. Alla fine `pgrep -c mame` deve dare `0`.
- `gioca` va sempre lanciato con `-n N` e `--muto` nelle verifiche: senza `-n`
  apre una finestra SDL in tempo reale.
- Niente git distruttivo (stash, reset, checkout di file). Niente push.
- `decomp/ricette/riferimento_nuova2.reg` è una registrazione in corso
  dell'utente (non tracciata): non toccarla, non aggiungerla al commit.
  `decomp/ricette/bottiglia_rossa.reg` è tracciato: non toccarlo.
- Il banco, il giro, l'oracolo e il replay storico devono restare identici
  (i rami nuovi non sono mai eseguiti in frame 0-5499).
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe (oggi i più grandi toccati: `attivita_attract07.c` 307,
  `attivita_attract07a.c` 256, `nemici_r.c` ~190).

## Cosa esiste già (fatti verificati)

### Il disassemblato (da `decomp/out/main64.dasm`, righe ~10843-10870 e 8886-8940)

```
38F4 ld a,(c243)   38F7 ld c,a   38F8 ld hl,82D3
38FB cp (hl)       38FC inc hl   38FD jr z,3903   38FF jr c,3931   3901 jr 38FB
3903 push bc       3904 ld b,10  3906 ld hl,39E3  3909 cp 32  390B jr nz,3912
390D ld b,00       390F ld hl,39EA
3912 ld a,e  3913 or b  3914 ld (c045),a
3917 ld de,D5CA  391A call 6771          ; HL = 39E3 «BONUS» oppure 39EA «FINAL»
391D ld hl,39F1  3920 ld de,D5D0  3923 call 6771     ; «PRISON!»
3926 ld a,b  3927 or a  3928 jr z,392F  392A ld a,1E  392C ld (c09a),a
392F pop bc        3930 ld a,c   3931 ...(il resto è già tradotto)
```

- Il ciclo 38FB-3901 scorre la tabella ROM 82D3 = `03 0C 15 1F 28 32 FF`
  (`xxd -s 0x82d3` su `decomp/out/main64.bin`, misurato): `cp (hl)` con A = c243;
  uguale → 3903; minore (riporto) → 3931; maggiore → voce successiva. `FF`
  chiude sempre (c243 < FF). A non cambia durante il ciclo.
- Al blocco 3903: A = c243 (cp non la modifica). E = valore di c045 calcolato a
  38E0-38EB: con c = 0 (38D3, e604 bit 1 acceso) il riporto di `rr c` è 0, quindi
  E = c045 & CF = il valore appena scritto a 38EC. Il blocco scrive c045 = E | B
  (B = 10 per tutte le voci tranne c243 = 32, dove B = 0), poi due `sub_6771`
  (stringhe a D5CA e D5D0; HL ricaricato ogni volta), poi, se B ≠ 0, c09a = 1E.
  Al rientro `pop bc; ld a,c` ripristina A = c243 e si prosegue a 3931 (già C).
- Quindi per c243 = 03, 0C, 15, 1F, 28 (non 32) il blocco accende il **bit 4 di
  c045**: nella fase 5 del corpo 07 (2D07) questo porta alla lettura di R a 2D1F.
- Dati ROM (misurati con xxd su `main64.bin`): a 39E3 `50 'BONUS' FF`, a 39EA
  `50 'FINAL' FF`, a 39F1 `50 'PRISON!' FF` (il byte 50 iniziale è l'attributo). `sub_6771` è già tradotta (`routine.h:49`).
- 2D07 (fase 5): `pop hl; ld a,(c243); cp 1D; jr nz,2D19` (c243 = 1D: 3812, non
  tradotto, fuori perimetro); 2D19: `ld a,(c045); and 10; ret z`. Poi:
  ```
  2D1F ld a,r  2D21 ld l,a  2D22 ld a,r  2D24 add a,l  2D25 and 1F  2D27 ld b,a
  2D28 ld c,08  2D2A ld d,00
  2D2C dec c    2D2D ret m
  2D2E ld hl,2D4A  2D31 ld e,c  2D32 add hl,de  2D33 ld a,(hl)      ; a = ROM[2D4A+c]
  2D34 ld hl,2D52  2D37 ld e,b  2D38 add hl,de  2D39 ld e,(hl)      ; e = ROM[2D52+b]
  2D3A ld hl,C0BD  2D3D add hl,de  2D3E rl (hl)  2D40 jr nc,2D44
  2D42 add a,40    2D44 ld (hl),a  2D45 inc b  2D46 res 5,b  2D48 jr 2D2C
  ```
  Otto giri (c = 7..0): rimescola celle della griglia a C0BD con le tabelle ROM
  2D4A (8 byte) e 2D52 (32 byte). I due `ld a,r` sono il caso già modellato da
  `nemici_r_somma()` (R1 + R2 con R2 = R1 + 3, `nemici_r.c:42`): il valore
  `(R1+R2) & 1F` è B. `rl (hl)`: il riporto è il bit 7 vecchio della cella.
- 3960: se la cifra delle decine `d` di `sub_3a1c` (quadro ≥ 10) non è 30, `ld
  (hl),d` con HL = D610. Serve a c243 = 32 (quadro 11).

### Il C esistente (da lettura, `attivita_attract07a.c`, 256 righe)

- `void fase_07_3861(void)` r. 24-174 circa. Righe 86-93:
  ```c
  uint8_t q = M(0xc243);                     /* 38F4-38FB */
  if (q == leggi(0x82d3)) { nemici_manca(0x3903); return; }   /* 38FD */
  if (q > leggi(0x82d3)) { nemici_manca(0x3901); return; }    /* 38FF */
  ```
  Dopo c'è già il ramo 3931 (quadro/resto dalla tabella ROM 85EC) e, a r. ~123,
  `if (z.d != 0x30) { nemici_manca(0x3960); return; }`. `z` è `struct z80 z = {0}`.
  Il file usa `NEMICI_PUNTO(da, a)` (solo per replay); il commento in testa dice che i
  rami mai eseguiti nel giro non hanno punti: i rami nuovi restano **senza punti**
  (precedente: `fase_07_352e`, lotto 54).
- `attivita_attract07.c`, `static void fase_2d07(void)` r. 57-67: dopo il
  controllo c243 = 1D e il test del bit 4 fa `nemici_r_somma(); nemici_manca(0x2d1f);`.
  Chiamata dallo `switch` a r. 284 (`case 5`).
- `attivita_piccoli.c:131`, in `void sub_4c89(struct z80 *z)` (r. 70): `z->a =
  leggi_e603(0x4ced); /* 4CED */` preceduta da `z->c = z->a` con A = c1b9.
- Helper: `M(a)`, `leggi(a)` (RAM o ROM, `ram.h:16`), `scrivi_porta`, `leggi_porta`,
  `SET_HL/SET_DE`, `sub_6771(&z)` (HL = stringa, DE = destinazione).

### Riproduzione del problema e misure di partenza (tutte misurate oggi)

| Comando (da `decomp/`) | Risultato |
|---|---|
| `harness/gioca -n 14000 --muto --riproduci ricette/riferimento_nuova.reg 2>&1 \| grep -a 'FERMO\|FRAME rese\|HASH ultimo\|AUDIO hash\|e603'` (71 s) | `e603: lettura dal PC 4CED non misurata, restituisco 0`; `FERMO routine 3903 mancante al frame 13926`; `FRAME rese 13860`; `HASH ultimo 3d132d88e2aa3f71`; `AUDIO hash 869210bd4697615e` |
| `gioca -n 6000 --muto` (nessun ingresso) | `FERMO routine 3901 mancante al frame 3385`; `FRAME rese 3319`; `HASH ultimo 383d885eb6e0cd95`; `AUDIO hash ed1cb697b9c3c557` |
| seme 4: `python3 ricette/genera_ingressi.py 4 /tmp/s4.txt 40000 8; gioca -n 40000 --muto --riproduci /tmp/s4.txt` | `FERMO routine 3901 mancante al frame 25937`; `FRAME rese 25871`; `HASH ultimo 16ea230eea591d30`; `AUDIO hash d57c1403819e226f` |
| seme 67 (stessa forma) | 3901 al frame 25198; rese 25132; `HASH ultimo 16ea230eea591d30`; `AUDIO hash 6b3df28c5f385ce6` |

`riferimento_nuova.reg` ha 13860 righe, ultimo frame 13925 (l'utente l'ha
registrata fino al FERMO): oltre il file gli ingressi sono a zero e la partita
prosegue fino a `-n`. Il FERMO scatta completando la stanza 3 (c243 passa da 2 a
3: prima voce di 82D3, quindi 3903 con «BONUS»).

### Catture MAME (metodo)

- `decomp/cattura_corpi.sh [elenco]` legge righe `modo cod entry tag N poke` (formato
  in testa a `decomp/corpi_catture.txt`) e scrive `decomp/out/snap_<entry>_att<cod>_
  <tag>_{prima,dopo}.bin`, `_regs.txt`, `.log`; ultima riga `CATTURE OK n su m`.
  Le catture sono tracciate in git. Quelle del corpo 07 oggi: `corpo 07 0x2CCD f1b..f1f`
  (r. 97-101, fase 1, poke `2300:0xc19d:1,...`); la fase 0 e la fase 5 **non hanno
  catture** (sono coperte solo dal giro dal boot).
- Voce del banco: `routine_tab.c:103` `{ "0x2CCD", banco_corpo_2ccd, 0, 0, 1 }`
  (ultimo campo = `solo_ram`; wrapper in `banco_corpi.c`). Il banco prova ogni
  `snap_0x2CCD_*_prima.bin` trovato in `decomp/out`.
- Registro R: `nemici_r.c` ha `prova_con_r(z, entry, fn)` (r. ~137) che, nel banco,
  riconosce la cattura e prova i valori di R1/R2 finché RAM e registri coincidono con
  il «dopo» di MAME; wrapper `banco_r_XXXX` dichiarati in `nemici.h:~376` e usati in
  `routine_tab.c` (es. r. 162 `{ "0x4302", banco_r_4302, 0, 0, 0 }`). Attenzione:
  `coincide()` (r. 92) confronta anche A BC DE HL IX IY e la pila con `pila_fine = sp`;
  per un corpo `solo_ram` il banco normale (`harness.c:84-87`) ignora i registri e
  porta `pila_fine` a `sp + 2`. Per usare `prova_con_r` con un corpo serve quindi un
  parametro `solo_ram` che salti il confronto dei registri e faccia `pila_fine = sp + 2`.

### Censimento di e603 a 4CED

- Dove: `sub_4c89` (`attivita_piccoli.c:70-143`), chiamata dal corpo 05 (4C42) con
  c1aa ≠ 0 e dalle catture `sub 05 0x4C89`. Codice ROM: `4CE9 ld a,(c1b9); 4CEC ld c,a;
  4CED ld a,(e603); 4CF0 xor c; and 08; 4CF3 jr nz,4D00` (4D00 = `sub_75aa`);
  altrimenti `4CF5 a = c ^ 08` → porta e803 e c1b9.
- MAME (documento `compiti/12-filone-porte.md` r. 78 e `compiti/03-...md` r. 163): il
  gestore restituisce `BC & 08` a quel PC, cioè la lettura **passa sempre** il confronto.
  Misura: `decomp/out/att_25_e603.txt` (tap a PC 4CF0 = 4CED + 3): valore `00` due volte
  e `08` una volta, coerente con `C & 08`. Il C invece restituisce 0 con un avviso:
  se il bit 3 di c1b9 è 1 devia a `sub_75aa` (4D00) dove MAME no. È un errore di
  modello, non una traduzione mancante: **rientra nel lotto come parte B** (una
  riga). Il riferimento citato sopra stampa l'avviso, quindi la riga viene eseguita.
  L'oracolo (frame 0-5499) non legge mai 4CED (`out/e603.txt`: solo 015E), per cui B
  non può cambiare nulla dei suoi hash. «`C & 08` per BC&08» è letto dai documenti,
  non dal sorgente di MAME (attesa, non misurata sul sorgente).

## Struttura da realizzare

### Parte A (commit 1)

1. **`attivita_attract07a.c`, `fase_07_3861`**: sostituire le due `if` di r. 87-93 con
   il ciclo sulla tabella e il blocco 3903, nell'ordine dell'originale:
   - `uint16_t t = 0x82d3; int uguale = 0; for (;;) { uint8_t v = leggi(t++); if (q == v) { uguale = 1; break; } if (q < v) break; }`
     (38FB-3901, `jr nc` = prosegue; il ciclo termina sempre perché c'è FF).
   - se `uguale`: blocco 3903-392F sul modello del disassemblato: `b = 0x10`,
     `hl = 0x39e3`; se `q == 0x32` → `b = 0`, `hl = 0x39ea`; `M(0xc045) = (M(0xc045) | b)`
     (E vale il c045 appena scritto a 38EC: leggerlo di nuovo da RAM è equivalente);
     `SET_HL(&z, hl); SET_DE(&z, 0xd5ca); sub_6771(&z);` poi `SET_HL(&z, 0x39f1);
     SET_DE(&z, 0xd5d0); sub_6771(&z);` poi, se `b`, `M(0xc09a) = 0x1e`.
   - il resto (3931 in poi) resta com'è: usa `q`, non `z.a`.
   - `nemici_manca(0x3901)` e `nemici_manca(0x3903)` spariscono; aggiornare il commento
     di testa (fase 0: «c243 nella tabella 82D3: stanze bonus/finale»).
2. **`attivita_attract07a.c`, 3960**: sostituire il `nemici_manca(0x3960)` con
   `M(0xd610) = z.d;` (3960 `ld (hl),d` con HL = D610; poi prosegue a 3961). Il codice
   seguente scrive già D611 e oltre: controllare che il flusso di r. ~123-127 resti
   coerente con `3961 inc hl; ld (hl),e`.
3. **`attivita_attract07.c`, `fase_2d07`**: tradurre 2D1F-2D48 al posto del
   `nemici_manca(0x2d1f)`: `b = (uint8_t)(nemici_r_somma() & 0x1f);` poi per `c` da 7 a 0:
   `a = leggi(0x2d4a + c); e = leggi(0x2d52 + b); cella = 0xc0bd + e;` il riporto è il bit 7 vecchio di `M(cella)` (`rl (hl)`); la scrittura finale 2D44 sovrascrive la cella: `M(cella) = (uint8_t)(a + (riporto ? 0x40 : 0))`; poi `b = (b + 1) & 0x1f` (inc b; res 5,b). Nessun punto di sincronia. Aggiornare il commento
   sopra la funzione («con il bit acceso rimescola…»).
4. **Banco**: la fase 5 usa R, quindi la voce `0x2CCD` deve passare da una ricerca di R.
   Aggiungere in `nemici_r.c` `void banco_r_2ccd(struct z80 *z)` che chiama `prova_con_r`
   (con il parametro `solo_ram` descritto sopra, wrapper `banco_corpo_2ccd` da
   `attivita_libera.h`), dichiararla in `nemici.h` accanto a `banco_r_4302`, e in
   `routine_tab.c:103` sostituire `banco_corpo_2ccd` con `banco_r_2ccd` (stessi flag).
   Per le catture senza lettura di R la ricerca esegue una volta sola, come prima
   (`r_usati == 0`): le cinque catture f1b-f1f restano OK.
5. **Catture nuove** in `corpi_catture.txt` (blocco con commento «Lotto 59»). Tutte `corpo 07
   0x2CCD`, N = 1; il poke `2300:0xc19d:0` rende esplicita la fase 0; c243 è la stanza forzata:
   ```
   corpo 07 0x2CCD f0q02 1 2300:0xc19d:0,2300:0xc243:0x02
   corpo 07 0x2CCD f0q03 1 2300:0xc19d:0,2300:0xc243:0x03
   corpo 07 0x2CCD f0q04 1 2300:0xc19d:0,2300:0xc243:0x04
   corpo 07 0x2CCD f0q0C 1 2300:0xc19d:0,2300:0xc243:0x0C
   corpo 07 0x2CCD f0q10 1 2300:0xc19d:0,2300:0xc243:0x10
   corpo 07 0x2CCD f0q24 1 2300:0xc19d:0,2300:0xc243:0x24
   corpo 07 0x2CCD f0q28 1 2300:0xc19d:0,2300:0xc243:0x28
   corpo 07 0x2CCD f0q32 1 2300:0xc19d:0,2300:0xc243:0x32
   corpo 07 0x2CCD f5r1 1 2300:0xc19d:5,2300:0xc045:0x10
   corpo 07 0x2CCD f5r2 1 2307:0xc19d:5,2307:0xc045:0x10
   ```
   Copertura: 02 = rami già tradotti (controllo), 03 = 3903 BONUS (prima voce), 04 = una
   iterazione di 3901, 0C = seconda voce, 10 = terza iterazione senza uguaglianza, 24 = c09a
   1B + 3931 dopo quattro voci, 28 = BONUS in quinta voce, 32 = FINAL (b = 0, c09a invariato) e
   3960 (quadro 11), f5r1/f5r2 = 2D1F con due R diversi. I numeri 0x24, 0x28, 0x32 sono poi
   il caso reale del post-game-over. Se una cattura non si ottiene o termina in un ramo
   `nemici_manca` (verificare nel `.log` che `prima PC=2CCD` e il «dopo» siano coerenti), non
   inventare: provare un poke aggiuntivo su c045/c24e che neutralizzi il ramo estraneo
   (stessa tecnica dei poke di `f1b`, r. 98) e, se serve, riportarlo nell'esito. Se una cattura f0
   scrive un ramo non tradotto fuori perimetro (3960 con quadro ≥ 10 a parte), toglierla
   dall'elenco e dichiararlo.
6. **Gioco libero**: nessun codice specifico; `macchina_libera.c` non va toccato.

### Parte B (commit 2)

`attivita_piccoli.c:131`: sostituire `z->a = leggi_e603(0x4ced);` con
`z->a = (uint8_t)(z->c & 0x08);` e il commento: `/* 4CED: la protezione di MAME restituisce
BC & 08 (att_25_e603.txt) */`. Così non c'è più l'avviso e la deviazione a 4D00 sparisce.
Riportare nell'esito se l'hash della riproduzione di riferimento cambia fra il commit A e il B
(cambia solo se il bit 3 di c1b9 valeva 1 in quel punto). Documentare in `decomp/DATI.md`
(riga `e603` della tabella e sezione «e603»): 4CED è modellato come `C & 08`; 62BE
(`attivita_27.c:125`) resta non misurato e invariato.

### Documenti

`STATO.md` (nuova voce «Lotto 59» con numeri e hash nuovi; aggiornare la «coda residua»:
3901 chiusa), `decomp/GIRO.md` (le menzioni 3901/3903 nell'elenco dei rami, righe ~133-134 e
~608), `decomp/ROUTINE.md` (nota sulla voce 2CCD: fasi 0 e 5 con catture), `decomp/DATI.md` come sopra.

## Verifiche obbligatorie

Da `decomp/` salvo dove indicato. Misure di partenza già riportate sopra; le «dopo» sono
attese, non misurate, e lo sviluppatore le dichiara.

1. Compilazione: `cd harness && make harness replay gioco_libero gioca sched_proto 2>&1 | grep -c
   'warning\|error'` → `0`.
2. Catture: `grep -E 'f0q|f5r' corpi_catture.txt > /tmp/c59.txt; sh cattura_corpi.sh /tmp/c59.txt
   | tail -1` → `CATTURE OK 10 su 10` (o `n su n` se se ne scartano alcune, dichiarate); `pgrep -c mame`
   → `0`.
3. **(a)** Banco: `cd harness && ./harness --tutte > /tmp/h59.txt; echo "exit $?"; grep -c FALLITO
   /tmp/h59.txt; grep -c '^RIEPILOGO' /tmp/h59.txt; grep -c '^OK' /tmp/h59.txt` → `exit 0`, `0`,
   `335`, `1422` (prima misurato: `exit 0`, `0`, 335, 1412; +10 catture nuove).
4. **(b)** `sh giro_completo.sh | tail -1` → `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0
   divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti` (misurato oggi,
   identico).
5. **(c)** `harness/gioca -n 5434 --muto --oracolo out/giroboot_oracolo.txt out/giroboot_disp.txt
   --riproduci out/ingressi_replay.txt 2>/dev/null | tail -6` → contiene `HASH ultimo
   a239d0a940bec0b9`, `FRAME ultimo 5499`, `AUDIO campioni 4042500`, `AUDIO hash
   c4b40fea06623a96` (misurato oggi).
6. **(d)** `harness/gioca -n 16000 --muto --riproduci ricette/riferimento_nuova.reg 2>&1 | grep -a
   'FERMO\|FRAME rese\|HASH ultimo\|AUDIO hash\|e603'` (circa 90 s) → nessun FERMO su 3901/3903,
   `FRAME rese 16000` (attesa, non misurata), nessuna riga `e603:` dopo la parte B. **Dichiarare
   `HASH ultimo` e `AUDIO hash` nuovi.** Se si ferma su un altro sito (probabile candidati: 3812 con
   c243 = 1D, 398B/3986, 39B9 per c243 = 33, altro): non tradurre, censire messaggio, frame, hash
   (vedi domanda 4); in quel caso (d) è «superato 13926» se il nuovo FERMO è oltre quel frame.
7. **(e)** `harness/gioca --riproduci ricette/partita_1a.reg -n 4000 --muto 2>/dev/null | grep -a
   'FERMO\|FRAME rese\|HASH ultimo\|AUDIO'` → nessun FERMO, `FRAME rese 4000`, `AUDIO campioni
   2988510`; **misurato oggi**: `HASH ultimo 474cc37bfa97faeb`, `AUDIO hash b6758278078e86fe`.
   Dichiarare se restano invariati (attesi invariati: fase 0 con c243 = 0, 1, 2 non cambia).
8. **(f)** Libero senza ingressi: `harness/gioca -n 6000 --muto 2>&1 | grep -a 'FERMO\|FRAME
   rese\|HASH ultimo\|AUDIO hash'` → non più `3901 al frame 3385`: dichiarare frame/FERMO nuovo
   e hash (prima: 3385, rese 3319, `383d885eb6e0cd95`, `ed1cb697b9c3c557`). E i semi: `python3
   ricette/genera_ingressi.py 4 /tmp/s4.txt 40000 8; harness/gioca -n 40000 --muto --riproduci
   /tmp/s4.txt 2>&1 | grep -a 'FERMO\|FRAME rese\|HASH ultimo\|AUDIO hash'` (circa 3 min) → non
   più `3901 al frame 25937`; dichiarare frame e hash (idem per il seme 67, prima 25198). Un
   nuovo FERMO oltre quei frame è un esito accettato e si censisce.
9. Determinismo: ripetere due volte il comando 6 e confrontare `HASH ultimo`/`AUDIO hash` → identici.
10. **(g)** `find harness -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 && $2!="totale" &&
    $2!="total"'` → vuota (misurato oggi: vuota).
11. Ambito: `git status --short` → solo i file elencati nei vincoli, le catture
    `decomp/out/snap_0x2CCD_att07_f0*`/`f5*`, `decomp/ricette/riferimento_nuova.reg` (nuovo, da
    committare) e `?? decomp/ricette/riferimento_nuova2.reg` (non toccato, non nel commit).

## Consegna

- Commit 1 (parte A): codice, catture, `corpi_catture.txt`, `decomp/ricette/riferimento_nuova.reg`
  come regressione, documenti. Commit 2 (parte B): `attivita_piccoli.c` e `DATI.md`. Messaggi in
  italiano, ognuno con la riga `Co-Authored-By: <modello che ha lavorato> <noreply@anthropic.com>`.
- Aggiungere in fondo a questo file la sezione «Esito» con le uscite delle verifiche e gli hash
  nuovi (d, e, f), prima e dopo la parte B per la (d).
- Rapporto finale: massimo trenta righe con uscita delle verifiche e deviazioni.

## Domande aperte

1. **Il sito 2D1F entra nel perimetro** (è un secondo ramo non tradotto, ma è l'effetto diretto
   di 3903: bit 4 di c045 → fase 5). Proposta: sì, incluso come punto 3 della parte A; senza,
   la riproduzione di riferimento si fermerebbe pochi frame dopo. — preferenza.
2. **3960 incluso** (una sola istruzione, serve alla cattura c243 = 32). Proposta: sì. — preferenza.
3. **4CED come parte B nello stesso brief** (commit separato, una riga). Proposta: sì; se la
   verifica (d) cambiasse l'hash tra A e B lo si dichiara. — preferenza.
4. **Nuovi FERMO oltre la famiglia** (3812 per c243 = 1D, 398B/3986, 39B9 per c243 = 33, 4D00…):
   proposta: non si traducono in questo lotto, si censiscono con messaggio/frame/hash nell'esito e
   in `STATO.md`; la verifica (d) vale come superata se il FERMO nuovo è oltre il frame 13926.
   — preferenza.
5. **`banco_r_2ccd` sostituisce il wrapper normale della voce 0x2CCD** (e non una voce nuova),
   con un parametro `solo_ram` in `prova_con_r`/`coincide`. Proposta: sì, è l'unico modo di
   verificare 2D1F con R ignoto senza duplicare la voce. — preferenza.
6. **Catture con stanza forzata via poke di c243 al frame 2300** invece di una cattura nel gioco
   vero (stanza completata). Proposta: poke, come le catture f1x; la riproduzione di riferimento fa
   da verifica nel contesto reale. — preferenza.

## Decisioni (chat principale, 2026-11-05)

Tutte le domande aperte erano preferenze: accettate le proposte dell'analista
(1. 2D1F nel perimetro; 2. 3960 incluso; 3. parte B 4CED nello stesso brief con
commit separato; 4. altri FERMO solo censiti, verifica (d) superata se il nuovo
FERMO è oltre il frame 13926; 5. wrapper banco_r_2ccd con solo_ram; 6. catture
con poke di c243 al frame 2300, contesto reale verificato dalla riproduzione).
Nota: decomp/ricette/riferimento_nuova2.reg è troncata (pkill accidentale), non
usarla; la regressione è riferimento_nuova.reg.

## Esito (sviluppatore, 2026-11-05)

Commit `ae8c20a` (parte A) e il commit della parte B (`git log -2`). Verifiche sul binario finale:

1. Compilazione: `0`.
2. Catture: `CATTURE OK 10 su 10`; `pgrep -c mame` → `0`. Deviazione: `f5r2` con il poke
   `2307:...` catturava la fase 0 (c19d = 00: la cattura parte dal 2300, prima del poke);
   riga cambiata in `2250:0xc045:0x10,2300:0xc19d:5`, che dà un R diverso da `f5r1` (celle
   di c0bd toccate diverse). Controllato nei «dopo»: q03/q0C/q28 c09a = 1E, c045 = 1A,
   «BONUS» a d5ca; q32 «FINAL», c09a = 1B, d610 = 31; q02/q04/q10/q24 senza blocco.
3. (a) Banco: `exit 0`, `0`, `335`, `1422` (`RIEPILOGO 0x2CCD: 15 catture, 15 OK`).
4. (b) `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`.
5. (c) `HASH ultimo a239d0a940bec0b9`, `FRAME ultimo 5499`, `AUDIO campioni 4042500`, `AUDIO hash c4b40fea06623a96`.
6. (d) Nessun FERMO, `FRAME rese 16000`. Dopo A: riga `e603: lettura dal PC 4CED non misurata`,
   `HASH ultimo a040200b69679371`, `AUDIO hash caa004b13ad0a661`. Dopo B: nessuna riga `e603:`,
   hash identici a quelli dopo A (il bit 3 di c1b9 valeva 0 in quel punto).
7. (e) Partita 1A: nessun FERMO, `FRAME rese 4000`, `AUDIO campioni 2988510`, `HASH ultimo
   474cc37bfa97faeb`, `AUDIO hash b6758278078e86fe` (invariati).
8. (f) Libero senza ingressi: `FERMO routine 39B9 mancante al frame 3385`, `FRAME rese 3319`,
   hash invariati `383d885eb6e0cd95` / `ed1cb697b9c3c557`. Seme 4: 39B9 al frame 25937, rese
   25871, `16ea230eea591d30` / `d57c1403819e226f`; seme 67: 39B9 al frame 25198, rese 25132,
   `16ea230eea591d30` / `6b3df28c5f385ce6` (hash invariati). Nuovo FERMO negli stessi frame,
   non oltre: 39B9 è la coda della fase 0 con c243 = 33 (dopo la finale, demo post-game-over),
   censito in STATO.md e GIRO.md, non tradotto (domanda 4).
9. Determinismo: due riproduzioni della (d) con hash identici.
10. (g) vuota.
11. Ambito: dopo i commit restano solo `?? compiti/59-famiglia-3901.md` e
    `?? decomp/ricette/riferimento_nuova2.reg` (non toccato).

## Esito (2026-11-05)

Ciclo passato al primo colpo (30º). Commit `ae8c20a` (parte A) e `0e36cc9`
(parte B); verificatore: 11/11 passate, catture rifatte 10/10 OK, banco
335/1422, giro e oracolo invariati, riproduzione di riferimento a 16000 frame
senza FERMO (hash a040200b69679371 / caa004b13ad0a661), partita 1A invariata,
avviso e603 sparito. Coda censita e non tradotta: 39B9 (stanza dopo la finale,
solo demo post-game-over, stessi frame di prima). riferimento_nuova2.reg
troncata dal pkill accidentale, non committata. Token: analista Sonnet ~123k,
sviluppatore Opus ~83k, verificatore Sonnet ~24k.
