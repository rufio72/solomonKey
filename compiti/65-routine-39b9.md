# Compito 65: famiglia 3861 completa (3874, 38BA, 38CA, 38D9, 398B, 3986, 39B9) e 365F/366C della fase 1

2026-11-06, scritto dall'agente analista per un agente sviluppatore. Questo
file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

PARTE A (39B9) e PARTE B (resto della famiglia 3861, scoperto nel collaudo del
quadro 4) sono un solo lotto; la parte C (365F/366C) è separabile (domanda 4).

Nel gioco libero (`gioca` senza `--oracolo`) il FERMO «routine 39B9 mancante»
interrompe la demo/attract quando il gioco resta senza ingressi (frame 3385 dal
boot; semi 4 e 67 a ~25-26k frame). 39B9 è l'ultima istruzione-coda di
`fase_07_3861` (fase 0 del corpo 07): scrive la stringa «DEMO» per c243 = 33.
Si traduce (3 righe di C), si cattura da MAME il comportamento, si misura dove
arriva il giro dopo il fix e si annotano gli hash nuovi.

## Vincoli assoluti

- Niente modifiche fuori da `/home/rufio72/OpenSolomonsKey-master`. Si toccano:
  `decomp/harness/attivita_attract07a.c`, `decomp/corpi_catture.txt`, le catture
  nuove `decomp/out/snap_0x2CCD_att07_f0q33*`, e i documenti `STATO.md`,
  `decomp/GIRO.md`, `decomp/ROUTINE.md`, `decomp/CENSIMENTO-ROM.md` (solo le
  menzioni di 39B9).
- MAME SOLO con `-video none` (lo garantisce `cattura_corpi.sh`; non lanciarlo a
  mano). Mai `pkill` largo su `mame`/`gioca`. Alla fine `pgrep -c mame` = `0`.
- `gioca` va lanciato sempre con `-n N` e `--muto`: senza `-n` apre una finestra.
- Niente git distruttivo. Niente push. Non toccare `decomp/ricette/*.reg`.
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe (`attivita_attract07a.c` oggi 278 righe, da `wc -l`).
- Se dopo il fix compare un NUOVO FERMO FUORI dalla famiglia 3861 (elenco nelle
  parti B e C): non tradurlo, censirlo (messaggio, frame, hash) nell'esito e in `STATO.md`.
  Un FERMO dentro la famiglia 3861 (3861-39B9) va tradotto in questo lotto.
- Se la sentinella «lettura fuori voce» avvisa su stderr: allargare le voci con la
  procedura del lotto 62 (`estrai_rom.py`) e annotarlo. Per 39B9 non è atteso:
  la voce `testi_39c0` copre 39C0-3A0D (da `CENSIMENTO-ROM.md` r. 161, `tecmo/rom_voci.c` r. 25).

## Cosa esiste già (fatti verificati)

### Disassemblato (da `decomp/out/main64.dasm` r. 10949-10955 e `xxd` su `out/main64.bin`)

```
39B0 call 6771          ; HL = 39FA (sub_6771 lascia HL dopo l'FF finale)
39B3 ld a,(c243)
39B6 cp 33
39B8 ret nz
39B9 ld de,D60C
39BC call 6771          ; HL = 3A04 (proseguimento del 39B0)
39BF ret
```

Byte ROM misurati: 39FA = `70 00 9A 9B 9E 9F CA CB 00 FF` (la stringa del 39B0, FF a 3A03);
**3A04 = `10 00 00 44 45 4D 4F 00 00 FF`** (attributo 10, poi `00 00 'D' 'E' 'M' 'O' 00 00`,
FF a 3A0D; la tabella 3A0E, `tab_quadri_3a0e`, comincia subito dopo). Quindi 39B9
scrive «  DEMO  » (8 celle) a D60C con attributo 0x10 (e il tile nel piano
D20C via `res 2,h` di `sub_6771`).

### Il punto da modificare (da `grafo symbol`/lettura di `attivita_attract07a.c` r. 270-278)

Ultime righe di `void fase_07_3861(void)` (r. 28 – 278):

```c
    NEMICI_PUNTO(0x39b3, 0x39b8);
    if (M(0xc243) == 0x33)                     /* 39B3-39B8 ret nz */
        nemici_manca(0x39b9);
}
```

Nel resto della funzione: `struct z80 z = {0};` dichiarata all'inizio; la chiamata
a 39B0 è `SET_HL(&z, 0x39fa); SET_DE(&z, 0xdfb8); sub_6771(&z);` e `z` conserva HL.
`sub_6771` (da `grafo symbol`, `decomp/harness/z80_6xxx.c:28`, firma
`void sub_6771(struct z80 *z)`): legge `c = leggi(HL)`, poi i byte fino a FF scrivendo
tile e attributo; al ritorno `A = FF`, `HL = (indirizzo dell'FF) + 1`, `DE` = destinazione
dopo l'ultima cella. Il rami 3861 non hanno punti di sincronia (mai eseguiti nel giro).

### Contesto del FERMO (da `STATO.md` r. 71, `decomp/GIRO.md` r. 627-634, esito brief 59)

- 39B9 è raggiunto solo con c243 = 33 (stanza dopo la finale) e c045 bit 1-2 spenti
  (39A4-39A9 `ret nz` altrimenti), cioè nella demo post-game-over.
- Famiglia vicina già tradotta (lotto 59: 3901/3903/3960 e 2D1F): usarlo come modello.
  Catture esistenti `snap_0x2CCD_att07_f0q{02,03,04,0C,10,24,28,32}` (`out/`) e
  righe `corpo 07 0x2CCD f0qXX 1 2300:0xc19d:0,2300:0xc243:0xXX` in
  `decomp/corpi_catture.txt` r. 105-112; il banco le esegue con `banco_r_2ccd`
  (`nemici_r.c:191`, voce `"0x2CCD"` in `routine_tab.c:103`, `solo_ram`), nessun
  codice del banco da toccare.

### Misure di partenza (eseguite oggi, binari già costruiti)

- `harness/gioca -n 4000 --muto 2>&1 | grep -a 'FERMO\|FRAME\|HASH ultimo\|AUDIO hash'` →
  `FERMO routine 39B9 mancante al frame 3385`, `FRAME rese 3319`,
  `HASH ultimo 383d885eb6e0cd95`, `AUDIO hash ed1cb697b9c3c557`.
- `harness/gioca -n 600 --muto` → `HASH ultimo ecd5acf694285489`, `AUDIO hash 8aedf41c0424df54`.
- `harness/gioca --riproduci ricette/partita_1a.reg -n 4000 --muto` → `FRAME rese 4000`,
  `HASH ultimo 474cc37bfa97faeb`, `AUDIO hash b6758278078e86fe`.
- `python3 -I censimento_rom.py --verifica | tail -1` → `CENSIMENTO OK: 201 occorrenze, 0 non classificate`.
- Dal brief 59: `riferimento_nuova.reg -n 16000` → `HASH ultimo a040200b69679371`,
  `AUDIO hash caa004b13ad0a661`; giro completo e oracolo come in verifiche 4-5.

## Struttura da realizzare

1. In `decomp/harness/attivita_attract07a.c`, fine di `fase_07_3861`: sostituire il ramo
   `nemici_manca(0x39b9)` con la traduzione, e correggere il commento di intestazione
   (r. ~20-27) aggiungendo che per c243 = 33 scrive «DEMO» a D60C:

```c
    NEMICI_PUNTO(0x39b3, 0x39b8);
    if (M(0xc243) == 0x33) {                   /* 39B3-39B8 ret nz */
        SET_DE(&z, 0xd60c);                    /* 39B9: HL = 3A04, da 39B0 */
        sub_6771(&z);                          /* 39BC «  DEMO  » */
    }                                          /* 39BF ret */
```

   Controllare che HL valga 0x3a04 dopo la chiamata a 39B0 (SET_HL non va ripetuto:
   è il lascito di `sub_6771`); se si preferisce esplicitarlo, `SET_HL(&z, 0x3a04)`
   è equivalente (libero).
2. `decomp/corpi_catture.txt`: aggiungere dopo la riga `f0q32` (r. 112):
   `corpo 07 0x2CCD f0q33 1 2300:0xc19d:0,2300:0xc243:0x33`
3. Catturare: `grep 'f0q33' corpi_catture.txt > /tmp/c65.txt; sh cattura_corpi.sh /tmp/c65.txt | tail -1`.
   Controllare nel `_dopo.bin` (e nel `.log`) che D60C contenga «DEMO» a D610-D613 e
   che la fase non esca prima di 39B3 (c045 & 6 = 0). Se la cattura non passa per
   39B9 (per esempio 38D0 cambia ramo o c24e & 2 è acceso a 3970), correggere le
   poke (tipicamente aggiungere `2300:0xc045:0x00` o `2300:0xc24e:0x00`), dichiarando
   la deviazione. Il banco confronta la fase 0 (`banco_r_2ccd`): deve dare OK.
4. Misurare il nuovo comportamento (verifica 6) e aggiornare i documenti: `STATO.md`
   (r. 71, code minori: 39B9 chiusa; nuova riga di lotto con hash nuovi),
   `decomp/GIRO.md` (r. 134 elenco rami; r. 627-634 misure: sostituire «39B9
   mancante» con il nuovo esito), `decomp/ROUTINE.md` r. 166 (fase 0 completa),
   `decomp/CENSIMENTO-ROM.md` r. 54 (il giro libero senza ingressi non si ferma più
   a 39B9, o si ferma altrove: dichiararlo).

## PARTE B: il resto della famiglia 3861 (tutti i rami `nemici_manca` della funzione)

Ricognizione (da `grep -n 'nemici_manca(0x' decomp/harness/attivita_attract07a.c`
e disassemblato `out/main64.dasm` 3861-39B8): `fase_07_3861` contiene sei ritorni
anticipati `nemici_manca`, oltre a 39B9. Tutti piccoli: si traducono TUTTI in
questo lotto. Il FERMO osservato dall'utente, `3874` (quadro 4 finito: c243 passa
a 5 e la fase 0 mostra il quadro nuovo), è il primo. Nessun altro `nemici_manca` in
`attivita_attract07a.c` fuori da `fase_07_3861` (le altre fasi: parte C).

| sito | riga C (07a.c) | condizione | cosa fa l'originale | traduzione |
|---|---|---|---|---|
| 3874 | r. 33-37 (ciclo 386D-3876) | c243 uguale a una delle 7 coppie di 3A0E `05 13 0F 14 16 13 19 14 23 13 2A 14 2B 13` | `ld c,(hl)`: C = secondo byte della coppia (13 o 14); il ciclo non esce, finisce le 7 coppie; poi 3879 `c09a = C` | nel ciclo: `if (a == leggi(0x3a0e+2*i)) c = leggi(0x3a0e+2*i+1);` senza `return`; mantenere `c` come variabile non-const |
| 38BA | r. 70-73 | `(c045 & 6) == 6` | `HL=39CB; DE=DC55; call 6771`, poi prosegue a 38C3 | `SET_HL(&z,0x39cb); SET_DE(&z,0xdc55); sub_6771(&z);` e continua |
| 38CA | r. 74-77 | `b = c045 (letto a 38AF)` bit 0 acceso | `HL = D855` (il `ld hl,e055` di 38CA è sovrascritto da 38CD) invece di D840 | variabile `uint16_t cella = (b & 1) ? 0xd855 : 0xd840;` usata a 38EF/38F2 al posto di `0xd840`/`0xd841` |
| 38D9 | r. 78-81 | `leggi_porta(0xe604)` bit 1 spento | `inc c` : C = 1 invece di 0 | `uint8_t cc = (leggi_porta(0xe604) & 2) ? 0 : 1; uint8_t v = cc & b; scrivi_porta(0xe604, v);` e poi `c045 = (c045 & 0xcf) \| ((v & 1) ? 0x20 : 0)` (38E0-38EC: `rr c` porta il bit 0 di v nel riporto → `or 20h`); E per 3912 resta il valore scritto in c045 |
| 398B | r. 147-151 | `c24e & 2` | `(DFBE) = D7D6` (parola: DFBE = D6, DFBF = D7), poi salta a 3991 | `M(0xdfbe)=0xd6; M(0xdfbf)=0xd7;` e continua a 3991 (non eseguire 3977-3988) |
| 3986 | r. 153-157 | cifra decine di (c246+1) diversa da '0' (cioè c246 ≥ 9; è la «nona stanza normale» del quadro) | `(DFBE) = D` prima di `(DFBF) = E` | `M(0xdfbe) = z.d;` poi `M(0xdfbf) = z.e;` (togliere il `return`) |

Aggiornare il commento di intestazione (r. 15-27) togliendo l'elenco dei rami mancanti.
Dopo il 38D9, il flusso 38DF-38EC va riscritto una volta sola per il caso generale
(la riga `scrivi_porta(0xe604, 0x00)` e `M(0xc045) & 0xcf` di oggi sono il caso `v = 0`);
tenere i `NEMICI_PUNTO` come sono (r. 83-87), invariati.

### Catture MAME (aggiungere a `decomp/corpi_catture.txt`, dopo `f0q33`; stesso formato)

```
corpo 07 0x2CCD f0q05 1 2300:0xc19d:0,2300:0xc243:0x05
corpo 07 0x2CCD f0q0F 1 2300:0xc19d:0,2300:0xc243:0x0F
corpo 07 0x2CCD f0q2B 1 2300:0xc19d:0,2300:0xc243:0x2B
corpo 07 0x2CCD f0p6 1 2300:0xc19d:0,2300:0xc243:0x02,2300:0xc045:0x06
corpo 07 0x2CCD f0p1 1 2300:0xc19d:0,2300:0xc243:0x02,2300:0xc045:0x01
corpo 07 0x2CCD f0w2 1 2300:0xc19d:0,2300:0xc243:0x02,2300:0xc24e:0x02
corpo 07 0x2CCD f0w3 1 2300:0xc19d:0,2300:0xc243:0x02,2300:0xc246:0x09
```

(`f0q05` = esattamente il passaggio al quadro 5; `0F` copre il valore 14, `2B` l'ultima coppia;
`p6`→38BA, `p1`→38CA, `w2`→398B, `w3`→3986.) Per 38D9 (e604 bit 1 spento) la poke
`2300:0xe604:0x00` può non essere accettata da MAME (e604 è una porta): provarla come
`f0e0 ... 2300:0xc243:0x02,2300:0xe604:0x00`; se non cattura, tradurre comunque dal
disassemblato e dichiararlo (verifica per composizione, come nel brief 52). Nei `_dopo.bin`
controllare: q05 → `c09a = 13`, q0F → `c09a = 14`, p6 → stringa a DC55, p1 → cella D855/D856 = 60,
w2 → DFBE/DFBF = D6/D7, w3 → DFBE = '1'. Le catture scartate (NMI) si dichiarano.

## PARTE C: 365F e 366C nella fase 1 (`fase_07_358f`, `attivita_attract07b.c`) — separabile

Ricognizione fatta sul binario con `--stanza N` (vedi misure): dopo la famiglia 3861 il
FERMO successivo di ogni nuovo quadro è nel ciclo di decodifica dei record di stanza della
fase 1 (r. 108-133 di `attivita_attract07b.c`, 3642-3668). Siti: `nemici_manca(0x366c)` r. 128
(record con `byte & 3F == 36`) e `nemici_manca(0x365f)` r. 132 (`byte & 3F >= 32`, ≠ 36). Disassemblato:

```
3639 bit 0,(hl)  363B set 0,(hl)  363D ld l,00  363F jr nz,3642  3641 inc l   ; HL=c24e: L = 1 se il bit 0 era SPENTO, altrimenti 0
3650 ld a,(bc) ; inc bc ; ld h,a ; and 3F ; cp 36 ; 3657 call z,366C ; 365A cp 32 ; ld a,h ; 365D jr c,3667
365F bit 0,l ; 3661 jr nz,3667 ; 3663 and C0 ; 3665 add a,29 ; 3667 ld (de),a
366C push af,hl,de ; ld a,23 ; call 0493 ; pop de,hl,af ; ret     ; accoda il codice suono 23
```

Traduzione: prima del `M(0xc24e) |= 0x01` (r. 111) salvare `uint8_t l = (M(0xc24e) & 1) ? 0 : 1;`
(il commento «L non usato» a r. 111 è sbagliato: L serve a 365F). Nel ciclo, dopo
`h = leggi(bc++)`, `m = h & 0x3f`: se `m == 0x36` eseguire `z.a = 0x23; sub_0493(&z);`
(nessun ritorno; il flusso prosegue a 365A con A = H, e dato che `m = 0x36 ≥ 0x32` entra nel ramo
365F); se `m >= 0x32`: `v = h; if (!l) v = (uint8_t)((h & 0xc0) + 0x29);` e poi `M(dest) = v`
(altrimenti `M(dest) = h`). Il `struct z80 z` per `sub_0493` è già dichiarato nella funzione
(verificare: r. 35-60) e l'uso di `sub_0493` è identico a `attivita_26.c:57`.
Catture (stanze misurate: 7 e 9 → 365F, 27 → 366C; c24e = 0 dà L = 1, c24e = 1 dà L = 0):

```
corpo 07 0x2CCD f1s07a 1 2300:0xc19d:1,2300:0xc24e:0,2300:0xc243:0x07
corpo 07 0x2CCD f1s07b 1 2300:0xc19d:1,2300:0xc24e:1,2300:0xc243:0x07
corpo 07 0x2CCD f1s1B 1 2300:0xc19d:1,2300:0xc24e:0,2300:0xc243:0x1B
```

### Misure sul binario di oggi (non modificato)

Seme 4 (`python3 ricette/genera_ingressi.py 4 /tmp/s4.txt 8000 8`), poi
`harness/gioca --stanza N -n 3000 --muto --riproduci /tmp/s4.txt` per N = 0..51
(ogni N ~4 s). Tutti al frame 1562 (la prima fase 0 del quadro N), N decimale:
- `3874`: 5 15 22 25 35 42 43 (= c243 05 0F 16 19 23 2A 2B, esattamente la tabella 3A0E);
- `365F`: 7 9 24 26 29 30 34 37 39 45 47 48 49; `366C`: 27 41;
- nessun FERMO in 3000 frame: 0-4 6 8 10-14 16-21 23 28 31-33 36 38 40 44 46 50 51.
Esempio: `--stanza 5 -n 8000` → `FERMO routine 3874 mancante al frame 1562`, `HASH ultimo 7dacebce43e1fdd2`;
`--stanza 4` → nessun FERMO, `HASH ultimo f9a74022a5e4e067`; `--stanza 21` → `c945c699a00b3da6`.
(Le stanze di PARTE B possono mostrare 365F/366C dopo il fix: è l'ordine atteso, vedi parte C.)

## Verifiche obbligatorie

Da `decomp/` salvo dove indicato. Le uscite «dopo» sono attese, non misurate:
lo sviluppatore le dichiara.

1. Compilazione: `cd harness && make harness replay gioco_libero gioca sched_proto 2>&1 | grep -c 'warning\|error'` → `0`.
2. Catture: `grep -E 'f0q33|f0q05|f0q0F|f0q2B|f0p|f0w|f0e0|f1s' corpi_catture.txt > /tmp/c65.txt; sh cattura_corpi.sh /tmp/c65.txt | tail -1`
   → `CATTURE OK n su n` con n = 11 (parti A+B: 8 righe + f0e0 se accettata; parte C: 3) (attesa, non misurata; dichiarare le scartate); poi `pgrep -c mame` → `0`.
3. Banco: `cd harness && ./harness --tutte > /tmp/h65.txt; echo "exit $?"; grep -c FALLITO /tmp/h65.txt; grep -c '^RIEPILOGO' /tmp/h65.txt; grep -c '^OK' /tmp/h65.txt`
   → `exit 0`, `0`, `335`, `1423` (attesa, non misurata: prima 1422; +1 per cattura OK, quindi 1422 + n; `RIEPILOGO 0x2CCD: 15 + n catture, tutte OK`).
4. Giro: `sh giro_completo.sh | tail -1` → `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`.
5. Oracolo: `harness/gioca -n 5434 --muto --oracolo out/giroboot_oracolo.txt out/giroboot_disp.txt --riproduci out/ingressi_replay.txt 2>/dev/null | tail -6`
   → contiene `HASH ultimo a239d0a940bec0b9`, `AUDIO hash c4b40fea06623a96`.
6. Il fix: `harness/gioca -n 4000 --muto 2>&1 | grep -a 'FERMO\|FRAME rese\|HASH ultimo\|AUDIO hash'`
   → non più `39B9 al frame 3385`; dichiarare frame/FERMO nuovo (o `FRAME rese 4000` senza FERMO)
   e i due hash nuovi (attesa, non misurata). Ripetere il comando due volte: hash identici (determinismo).
   Se il comando non si ferma a 4000, provare `-n 30000` e dichiarare dove arriva.
7. Libero breve invariato: `harness/gioca -n 600 --muto` → `HASH ultimo ecd5acf694285489`, `AUDIO hash 8aedf41c0424df54` (misurato oggi).
8. Ricette (ciascuna `harness/gioca --riproduci ricette/X.reg -n <ultimo frame + 600> --muto`):
   `partita_1a.reg -n 4000` → `HASH ultimo 474cc37bfa97faeb`, `AUDIO hash b6758278078e86fe` (misurato);
   `riferimento_nuova.reg -n 16000` → `a040200b69679371` / `caa004b13ad0a661` (dal brief 59);
   `bottiglia_rossa.reg -n 11167` e `stanze_bonus.reg -n 12387`: nessun FERMO. Misurare prima del
   fix gli hash di queste ultime due (git stash vietato: basta eseguirle sul binario già costruito
   prima di ricompilare) e confrontarli dopo.
9. Semi: `python3 ricette/genera_ingressi.py 4 /tmp/s4.txt 40000 8; harness/gioca -n 40000 --muto --riproduci /tmp/s4.txt 2>&1 | grep -a 'FERMO\|FRAME rese\|HASH ultimo\|AUDIO hash'`
   (circa 3 min) → non più `39B9 al frame 25937` (prima: `16ea230eea591d30` / `d57c1403819e226f`);
   dichiarare frame e hash nuovi. Idem seme 67 (prima 39B9 al 25198, `16ea230eea591d30` / `6b3df28c5f385ce6`).
   Un nuovo FERMO oltre quei frame è un esito accettato e si censisce.
9b. **Passaggio di quadro (prova di fine lotto)**: `python3 ricette/genera_ingressi.py 4 /tmp/s4.txt 8000 8; for N in 5 15 22 25 35 42 43; do harness/gioca --stanza $N -n 8000 --muto --riproduci /tmp/s4.txt 2>&1 | grep -a 'FERMO\|HASH ultimo'; done`
    → nessuna riga `FERMO routine 3874` (prima: `3874 al frame 1562`, `HASH ultimo 7dacebce43e1fdd2` per tutte e sette). Spazzata completa: `for N in $(seq 0 51); do echo "$N: $(harness/gioca --stanza $N -n 3000 --muto --riproduci /tmp/s4.txt 2>&1 | grep -a FERMO)"; done | grep -v ': $'`
    (circa 3-4 min) → con la parte C vuota (nessun FERMO per nessuno dei 52 quadri a 3000 frame); senza parte C resteranno solo le righe `365F`/`366C`. Dichiarare l'uscita e, per le stanze 5 e 35, il `HASH ultimo` nuovo. Il passaggio quadro 4→5 è la sola stanza 5 (c243 = 05): la cattura `f0q05` (verifica 2) ne è la prova MAME. Un FERMO nuovo dentro 3861-39B9 va tradotto; fuori famiglia, censito.
10. Censimento: `python3 -I censimento_rom.py --verifica | tail -1` → `CENSIMENTO OK: 201 occorrenze, 0 non classificate`;
    nessun avviso «fuori voce» su stderr nelle esecuzioni 6 e 9.
11. Dimensioni: `find harness -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'` → vuota.
12. Ambito: `git status --short` → solo `attivita_attract07a.c`, `corpi_catture.txt`, le catture
    `out/snap_0x2CCD_att07_f0*`/`f1s*`, `attivita_attract07b.c` (parte C), i documenti elencati e `?? compiti/65-routine-39b9.md`.

## Consegna

- Un commit in italiano con codice, catture, `corpi_catture.txt` e documenti, e la riga
  `Co-Authored-By: <modello che ha lavorato> <noreply@anthropic.com>`.
- In fondo a questo file la sezione «Esito» con le uscite delle verifiche e gli hash nuovi (6, 9).
- Rapporto finale: massimo trenta righe con uscita delle verifiche e deviazioni.

## Domande aperte

1. **Nuovi FERMO dopo il fix**. Proposta: se FUORI dalla famiglia 3861 (3861-39B9,
   parti A e B) non si traducono, si censiscono con frame e hash; la verifica 6 vale come
   passata se il FERMO è oltre il frame 3385. — preferenza.
4. **Parte C (365F/366C della fase 1, `attivita_attract07b.c`) nello stesso lotto.**
   Sono due rami piccoli ma in un'altra fase (358F) e fuori dalla famiglia 3861; sono però il
   FERMO subito successivo di 15 quadri su 52 (e dei sette di parte B). Proposta: inclusi,
   commit separato dopo A+B, così l'avanzamento dei quadri non si ferma più. — scelta di progetto (perimetro).
5. **Altri siti `nemici_manca` della fase 0/1/5 fuori perimetro** (`2d0f`, `2e04`, `2d9d`,
   `3065`: fasi 5, 16, 13 del corpo 07; `2d0f` ora tradurrebbe con `sub_3812` già esistente).
   Proposta: solo censiti, nessuno compare nelle 52 prove `--stanza`. — preferenza.
6. **Prova di passaggio di quadro automatica** con `--stanza N` e semi: non c'è una
   registrazione di una partita che completi il quadro 4 (le due dell'utente non sono nel
   repo). Proposta: la prova misurabile è la verifica 9b (quadro 5 e le altre sei voci di
   3A0E raggiunte direttamente con `--stanza`) più la cattura `f0q05`; la partita vera
   resta al collaudo dell'utente. — preferenza.
2. **Catture oltre `f0q33`** (per esempio c243 = 34 o 40, per esercitare il ramo «quadro ≥ 10»
   già coperto da 3960). Proposta: solo `f0q33` oltre alle catture della parte B/C elencate, il resto è già nel banco. — preferenza.
3. **Hash dei semi 4/67 dopo il fix** richiedono ~3 min ciascuno. Proposta: eseguirli
   entrambi, in sequenza, in primo piano. — preferenza.

## Decisioni (chat principale, 2026-11-06)

Tutte e tre le domande sono preferenze, accettate con le proposte:
1. Un eventuale FERMO successivo si censisce senza tradurlo in questo lotto.
2. Si cattura solo `f0q33`.
3. I semi 4 e 67 si eseguono entrambi, in sequenza.

## Decisioni aggiuntive (chat principale, 2026-11-06, dopo l'estensione B/C)

4. SÌ, la parte C (365F/366C della fase 1) entra in questo lotto, in un
   commit separato — scelta di progetto decisa dalla chat sul mandato
   dell'utente (il gioco non deve fermarsi ai passaggi di quadro).
5. `2d0f`, `2e04`, `2d9d`, `3065` restano solo censiti — preferenza accettata.
6. Prova automatica = verifica 9b + cattura `f0q05`; la partita vera che
   completa il quadro 4 resta al collaudo dell'utente — preferenza accettata.
Correzione alla decisione 2 di prima: non più «solo f0q33» ma tutte le
catture elencate nel brief (f0q33, f0q05, f0q0F, f0q2B, f0p6, f0p1, f0w2,
f0w3, f0e0 se MAME la accetta, f1s*).

## Esito (sviluppatore, Claude Opus 5.5, 2026-11-06)

Parti A, B e C tradotte; due commit (A+B, poi C). Nessun `nemici_manca`
resta in `fase_07_3861` né nel ciclo 3642-3668 di `fase_07_358f`.

Verifiche:
1. compilazione: `0`.
2. catture: `CATTURE OK 12 su 12` (8 di A+B, `f0e0`, 3 di C); `pgrep -c mame` → `0`.
3. banco: `exit 0`, `0`, `335`, `1434`; `RIEPILOGO 0x2CCD: 27 catture, 27 OK`.
4. giro: `GIRO COMPLETO 0-5499: saldatura 0 byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con hash coincidenti`.
5. oracolo: `HASH ultimo a239d0a940bec0b9`, `AUDIO hash c4b40fea06623a96`.
6. libero `-n 4000`: `FRAME rese 4000`, nessun FERMO, `HASH ultimo e9690b9c1421f21a`,
   `AUDIO hash 7fc67068a194ac59` (due esecuzioni identiche). `-n 30000`: nessun FERMO,
   `cfde832053b94095` / `58cc653154347157`.
7. `-n 600`: `ecd5acf694285489` / `8aedf41c0424df54` (invariati).
8. ricette tutte invariate e senza FERMO: `partita_1a` `474cc37bfa97faeb` / `b6758278078e86fe`;
   `riferimento_nuova` `a040200b69679371` / `caa004b13ad0a661`; `bottiglia_rossa`
   `69a54ac9aa6bea9e` / `b70dc75d81b355b9` (misurati prima e dopo); `stanze_bonus`
   `bbebf0f3c64e446a` / `cb78ab354330a616` (prima e dopo).
9. seme 4 `-n 40000`: nessun FERMO, `0b8b015f62457828` / `4a6401329f8897cb`;
   seme 67: nessun FERMO, `ec7b1103568e16ad` / `aa8cb02441cad47e`.
9b. nessun `3874`, `365F`, `366C`. Stanze 5 15 22 25 35 42 43 a 8000 frame: solo la 5
   ha `FERMO routine 2E04 mancante al frame 7790` (`3d9c0ae560d66f59`, fuori famiglia,
   censito); stanza 35 `a6a9a24e144e41ae`. Spazzata 0-51 a 3000 frame: unico FERMO
   `29: routine 2D0F mancante al frame 1562` (fuori famiglia, censito). Avvisi «fuori voce»
   in 13 stanze (2 6 11 16 18 20 23 30 32 33 38 44 50) a 66C4-66D3, 6BC7-6BC8, 6F45-6F48,
   746C-746F, 756E-756F, 7576-7577, B126-B14D: GIÀ PRESENTI col binario di HEAD (provato
   in un worktree temporaneo sulle stanze 2, 6, 18); non allargati, annotati in `STATO.md`.
10. `CENSIMENTO OK: 202 occorrenze, 0 non classificate` (202, non 201: la 3874 legge
    anche il secondo byte della coppia); nessun «fuori voce» nelle esecuzioni 6 e 9
    dopo l'allargamento sotto.
11. vuota. 12. vedi deviazioni (file in più).

Deviazioni:
- Parte C: 358F è la fase **4** del 07 (`attivita_attract07.c:293`), non la 1; le
  catture `f1s*` usano `2300:0xc19d:4` (con c19d = 1 si sarebbe catturata 34FA).
- `f0q33` con `2300:0xc045:0x00` in più (senza, c045 = 0A esce a 39A9).
  `f0e0` con `2300:0xc045:0x01` in più: la poke di e604 comunque non ha effetto in
  MAME (c045 resta 01, come `f0p1`); 38D9 tradotta dal disassemblato.
- Sentinella dopo il fix (verifica 6): 770F-7712. `copione_demo_767c` passa a
  767C-78DF (tre copioni puntati da 765B/760B: 767C, 76F6, 77D6; codice a 78E0):
  toccati `censimento_rom.py`, `estrai_rom.py` (90 voci, 20094 byte), `tecmo/rom_*`
  rigenerati, `CENSIMENTO-ROM.md`. Hash del libero dopo l'allargamento (prima,
  con la sentinella a 0: `30bb202eb2a5d714`).
- `censimento_rom.py`: numeri di riga dei siti di 07a/07b aggiornati; il file
  locale non versionato `out/censimento_rom_letti.txt` rimappato alle righe nuove.
