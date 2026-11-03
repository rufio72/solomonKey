# Compito 37: lotti N11 + N12 del motore dei nemici, tipo 07 seconda metà (5FC8, 610C, 627F) e 2474, 4ADA

Data: 2026-11-03. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere.

## Contesto: file da leggere

Da leggere (e basta):
1. `decomp/out/nemici_n11.dasm`, da generare (vedi sotto): 372 righe di
   disassemblato delle 20 routine. Dentro c'è una tabella di dati disassemblata
   come istruzioni, che non è codice: 6130-614F (le 16 parole del `rst 38` di
   610C, già in `rst38_tables.h` come `rst38_6130`).
2. `decomp/harness/nemici.h`: r. 1-60 (riporto, `nemici_rst38`), r. 236-262
   (sezione N8), r. 296-324 (registro R e stub).
3. `decomp/harness/nemici_tipo07a.c`: r. 1-45 (`sub_5cfb`), r. 276-315
   (`sub_5e9d`), r. 351-368 (`sub_5f0c`), r. 415-430 (`sub_5f4b`, modello del
   `rst 8` con `sub_0008` e di `sub_5fac` in coda), r. 236-253 (`sub_5e60`,
   modello di `push iy; ex (sp),ix` + `sub_4af6`).
4. `decomp/harness/nemici_stub.c` (15 righe), `harness/nemici_r.c` r. 126-174,
   `harness/routine_tab.c` (r. 100-116 e 160-185), `harness/nemici_comuni.c`
   r. 1-19 e r. 237-288 (`sub_4b20`, chiamante di 2474), `decomp/ROUTINE.md`
   (solo r. 534-587 sezione N8 e r. 630-676 sezione N10 come modello),
   `decomp/CENSIMENTO-NEMICI.md` (solo il paragrafo «**Lotto N10 fatto**»
   e r. 516).

È vietato leggere `STATO.md`, `README.md`, `ROUTINE.md` per intero, gli
altri brief in `compiti/` e i moduli `nemici_*.c` non elencati sopra.

Generazione del disassemblato (da `/home/rufio72/OpenSolomonsKey-master`;
`awk` è mawk):
```
cd decomp && awk 'BEGIN{n=split("2474 4ada 5fc8 610c",lo," "); split("2481 4af5 608b 62b5",hi," ")}
/^[A-Za-z_0-9]+:/ {lab=$0; next}
/;[0-9a-f][0-9a-f][0-9a-f][0-9a-f][ \t]*$/ {a=substr($0,length($0)-3); ok=0; for(i=1;i<=n;i++) if(a>=lo[i] && a<=hi[i]) ok=1; if(ok){ if(lab!="") print lab; print} lab=""; next}
{lab=""}' out/main64.dasm > out/nemici_n11.dasm; cd ..
```
Uscita attesa (misurata): `wc -l < decomp/out/nemici_n11.dasm` → `372`;
`grep -c 'ld a,r' decomp/out/nemici_n11.dasm` → `0` (nessuna routine del
lotto legge R direttamente; 5CFB arriva a 5DDF solo con il bit 5 di (IX+0)
spento: vedi «Il registro R»). Il file è ignorato da git (`decomp/out/*`): si
aggiunge al commit con `git add -f`.

## Obiettivo

Ultimo lotto del motore dei nemici. Tradurre in C le 20 routine che mancano
(5 stub di `nemici_stub.c` più 15 interne): 18 del lotto N11 (5FC8, 5FDC,
6046, 610C, 6151, 6165, 6179, 6183, 618B, 6196, 61B1, 61FA, 6221, 6248,
6252, 6261, 6270, 627F; 5FAC è già fatta in N8) nel modulo nuovo
`nemici_tipo07b.c`, e le 2 del lotto N12 (2474, 4ADA) in `nemici_comuni.c`.
Chiudere il ramo 5D08-5D28 di 5CFB. Svuotare e togliere `nemici_stub.c`:
il motore è completo. 14 voci nuove nel banco più 49AE, con 130 catture MAME;
5CFB e 5E1A salgono di 15 e 1 catture. Il replay e `sched_proto` non devono
cambiare.

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `cattura.sh`, `snap.lua`, `sessioni.sh`, `replay.*`, `sched*.c`,
  `rst20*`, `rst38_tables.h`, `censimento_nemici.py`, `STATO.md`,
  `README.md`, `compiti/` (tranne questo file non si scrive nulla), né le
  catture esistenti in `decomp/out/`.
- Niente installazioni. MAME gira solo come fa `cattura.sh` (senza finestra,
  senza audio, provider a `none`): niente partite a mano, niente giri su
  tutte le stanze. Una cattura dura 3-100 secondi (fino a 6 in parallelo con
  `xargs -P6`, file di uscita distinti).
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe e nessun `nemici*.c` sopra le 700 (`nemici_tipo07b.c` sopra le
  650: si spezzerebbe per indirizzo in `07b`/`07c`; la stima è 400).
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`); si
  ripristina un file solo con `git checkout -- <file>`. `git rm` è permesso
  solo per `decomp/harness/nemici_stub.c`.
- Un solo agente alla volta su `decomp/harness`.

## Cosa esiste già (fatti verificati)

**Routine da fare** (da `CENSIMENTO-NEMICI.md`, dal dasm e da `grafo`;
`(t)` = già tradotta; byte dal censimento):

| ingresso | byte | chiamanti | chiama | note dal dasm |
|---|---:|---|---|---|
| 5FC8 | 40 | 5E1A (dest. 2 di `rst 38` 5E40) | 5F0C(t), 5FDC, 6046, 5E4D(t) | bit 7 di (IX+4) spento e bit 1 di (IX+1) spento: DE=0800, 5F0C, 6046, 5E4D; altrimenti (5FCF): DE=1800, 5F0C, 5FDC, 5E4D |
| 5FDC | 86 | 5FC8 | 5E8F(t), 5FAC(t) | `rst 8` su tabella ROM 60CC (parole B, C); ret z, coda 5FAC, o rami 6018 / 5FF5; BC finale 0602, 0600, 0601, 0000, 0001 |
| 6046 | 70 | 5FC8 | 5E8F(t), 5FAC(t) | come 5FDC con tabella ROM 60EC; BC finale 0702, 0700, 0701 |
| 610C | 37 | 5CFB | `rst 38` 6130 | A=(IY+1A); `ret z` se = (IX+0E); filtro su (IY+1D)-1 < 2; (IX+0E)=A, E=A & 0F, `rst 38` |
| 6151, 6165 | 20 | 6130 (dest. 4/8 e 1/2), 61B1 | 6252, 5E4D(t) | (IY+0E) &= F0 + 0A / 06; BC=0C00 / 0C01; 5E4D |
| 6179, 6183 | 10, 8 | 6130 (dest. 9 e 6) | 6248, 618B | 6248 con riporto: 6196, altrimenti A=2 / 3 e 618B |
| 618B | 11 | 6179, 6183 | 5E4D(t) | **ingresso interno** (coda): C=A, B=3, 5E4D, (IX+0E)=FF |
| 6196 | 27 | 6130 (dest. 7 e 11), 6179, 6183, 61B1 | 5E4D(t) | arrotonda (IY+0E) e (IY+15); BC=0C02 |
| 61B1 | 73 | 6130 (dest. 3, 12, 15), 6261, 6270 | 5E4D(t), salti 6196, 6165, 6151 | E = A>>1 (rrca); tre rami su (IY+0E), (IY+15); BC=0C03 nel ramo lungo |
| 61FA, 6221 | 39 | 6130 (dest. 13, 14), 6261, 6270 | 5E4D(t) | (IY+15) arrotondato, (IY+0E), bit 6 di (IY+0) spento, BC=0D00 / 0D01 |
| 6248 | 10 | 6179, 6183 | - | A=((IY+0E)+2)&0F, `cp 4`: riporto = A<4 |
| 6252 | 15 | 6151, 6165 | - | (IY+15) arrotondato a multiplo di 16 |
| 6261, 6270 | 15 | 6130 (dest. 5 e 10) | salti 61FA / 6221 / 61B1 | A=((IY+0E)-6)&0F; `cp 7`: carry → 61FA / 6221, altrimenti 61B1 |
| 627F | 55 | 6130 (dest. 0), 5E9D (`jp 627F`) | 4AF6(t) | crea l'oggetto con 4AF6; **uscita anomala**: vedi sotto |
| 2474 | 14 | 4B20 (r. 280 di `nemici_comuni.c`) | - | D = ~A+5; E = (HL)+(HL+1); A = D-E; riporto = D<E in z->f |
| 4ADA | 28 | 49AE (4AB6, 4ACF) | 2457(t), 3A2E(t) | (c0bd+E)=A; C=A; 2457; DE salvato; 3A2E; (c07b..c07e) = E, D, C, B |

(2474, da dasm: `cpl; inc a` x5 poi `ld d,a`; `ld a,(hl); inc hl; add a,(hl); ld e,a;
ld a,d; sub e; ret`: lascia HL+1, D, E, A e il riporto di `sub e` = D < E.)

Già fatte e **non** da rifare: 5FAC (N8, in `nemici_tipo07a.c`), 5E4D, 5E8F,
5F0C, 4AF6, 4FEF, 4B20, 49AE (queste ultime due in `nemici_comuni.c`, senza voce
nel banco finché questo lotto non aggiunge 49AE).

Firme (da `grafo symbol`; tutte `void sub_xxxx(struct z80 *z)`): `sub_5e4d`
(B,C → (IY+1D), (IY+1E)), `sub_5e8f` (B,C → E, A), `sub_5f0c`, `sub_5fac`,
`sub_4af6`, `sub_2457`, `sub_3a2e`, `sub_0008` (`rst 8`: HL += 2·A, DE=A),
`nemici_rst38(z, sito, tab, voci)` (nemici.h r. 32-42). Tabella
`rst38_6130` (16 voci) in `rst38_tables.h`:
`627F 6165 6165 61B1 6151 6261 6183 6196 6151 6179 6270 6196 61B1 61FA 6221 61B1`.

### Convenzioni di traduzione

- Ogni routine `void sub_xxxx(struct z80 *z)`, non statica (prototipi in
  `nemici.h`), tranne la funzione d'appoggio `corpo_627f` (statica). Registri
  `z->a b c d e h l`, `z->ix` (slot), `z->iy` (oggetto), `z->f`; memoria con
  `M(indirizzo)`; ROM con `leggi(indirizzo)`; coppie `BC(z) DE(z) HL(z)`,
  `SET_BC SET_DE SET_HL`; `#define IXB(o) M(z->ix + (o))` e `IYB(o)` come in
  `nemici_tipo07a.c`; il modulo include `"routine.h"`, `"nemici.h"`,
  `"rst38_tables.h"`.
- Il banco confronta RAM (c000-e5ff, salvo 0x40 byte sotto SP) e A, BC, DE,
  HL, IX, IY all'uscita, non F: anche i registri «sporchi» devono coincidere.
- Il riporto, dove il chiamante lo legge, sta nel bit 0 di `z->f` con
  `NEMICI_PONI_RIPORTO(z, c)` / `NEMICI_RIPORTO_ACCESO(z)` (nemici.h r. 17-19):
  lo lasciano 6248 (A<4, dopo `cp 4`) e 2474 (D<E).
- Etichette del dasm con «+n» sono costanti: `ld bc,l0c00h+1` = BC 0C01,
  `l0c00h+2` = 0C02, `l0d00h+1` = 0D01; `ld bc,sub_0602h` (6008) = BC 0602;
  `ld de,l1800h` = DE 1800; `ld de,l0800h` = DE 0800.
- `bit` non cambia i registri e `ld` non cambia i flag: in 6010-6015 e simili
  `ret nz` usa lo Z di `bit 7,c` fatto prima di `ld bc,0600h`.
- `jr z,5FAC` (5FE8) e `jp z,5FAC` (6052) sono salti di coda: in C
  `sub_5fac(z); return;`, e il `ret` di 5FAC torna al chiamante di 5FDC /
  6046 (cioè 5FC8, che poi chiama 5E4D).
- `rst 8` a 5FDF / 6049: `SET_HL(z, 0x60cc)` (o `0x60ec`), `sub_0008(z)`, poi
  `z->b = leggi(HL(z))`, HL+1, `z->c = leggi(HL(z))` (come `sub_5f4b`).
- 610C (dasm 610C-612F): `A=(IY+1A)`; `cp (IX+0E); ret z`; `B=A`;
  `A=(IY+1D)-1; cp 2`: se A<2 (riporto) e A==(IY+1E) allora `A=B&0F`: se 0
  prosegue, se 0F prosegue, altrimenti `ret nz` (A e B restano); in tutti gli
  altri casi si va a 6129: `A=B; (IX+0E)=A; A&=0F; E=A`. Poi `rst 38` a 6130:
  `nemici_rst38(z, 0x6130, rst38_6130, 16)`, A resta il nibble (61B1 lo usa
  con `rrca`), poi `switch` sulla destinazione. Il nibble è sempre < 16: il
  `default` e il caso `0` non sono raggiungibili (restano scoperti in gcov).
  Le destinazioni ritornano al chiamante di 610C (5CFB, dopo 5D0F), tranne 627F.

### Uscita anomala di 627F (punto delicato)

Il blocco 627F-62B5 finisce con `pop hl; ret` (62B4): scarta l'indirizzo di
ritorno in cima alla pila e ritorna al chiamante del chiamante.
- Da 610C (destinazione 0): la cima è il ritorno a 5D0F dentro 5CFB. Dopo
  `pop hl; ret` 5CFB **non prosegue**: torna direttamente al chiamante di 5CFB
  e HL = 0x5D0F.
- Da 5E9D (`jp 627F`, r. 310 di `nemici_tipo07a.c`): la cima è il ritorno di
  5E1A a 5D28; 5E1A (chiamata solo da 5CFB, ultima istruzione) e 5CFB
  terminano, HL = 0x5D28.
Misurato nelle catture (HL di `dopo`): 5CFB `forza01` e `forza02` → HL=5D0F;
5CFB `forza10` e `forza11` → HL=5D28; il PC di ritorno è sempre 4EFB, cioè la
cattura di 5CFB è un'uscita normale (a 627F non si può catturare l'ingresso:
`snap.lua` aspetta il ritorno all'indirizzo in cima alla pila e non arriva
mai; 627F si prova solo da 5CFB).

Struttura in C:
```
static void corpo_627f(struct z80 *z, uint16_t scartato);   /* 627F-62B5 */
void sub_627f(struct z80 *z) { corpo_627f(z, 0x5D28); }     /* da 5E9D */
```
`corpo_627f`: `d=(IY+0); (IY+0) &= ~0x40; d |= 0x20; d &= ~0x10;
a=(IY+12)|(IY+13); c = a ? 0 : 2; e=(IY+4)-3; (IY+3)=0x0C; b=6; a=d;`
scambio IX/IY come in `sub_5e60` (`ix` salvato, `z->ix = z->iy`),
`sub_4af6(z)`, ripristino di IX, `a=(IX+1) & 0x81; (IX+1)=a;`, `SET_HL(z,
scartato)`. (DE finale viene da 4AF6.) In `sub_610c` il caso `0x627F` chiama
`corpo_627f(z, 0x5D0F)` e segna l'uscita anomala: tutte le uscite di
`sub_610c` terminano con `NEMICI_PONI_RIPORTO(z, anomala)` (1 solo dal caso
627F, 0 negli altri casi e nei `ret` anticipati). In `sub_5cfb`
(`nemici_tipo07a.c`, r. 31) subito dopo `sub_610c(z);` aggiungere
`if (NEMICI_RIPORTO_ACCESO(z)) return;` (stessa convenzione di 531C in 50BA e
50DF, brief 36). Aggiornare nello stesso file il commento di testa (r. 11:
«610C, 5FC8 e 627F restano stub» → tradotte in `nemici_tipo07b.c`).

### Il registro R

Nessuna routine del lotto legge R, ma 5CFB (con il bit 2 di (IX+0) spento) arriva
a 5E1A e, con il bit 5 spento, a 5DDF (due `ld a,r`). Per le nuove catture
naturali di 5CFB serve il wrapper: aggiungere in coda a `nemici_r.c`
```
void banco_r_5cfb(struct z80 *z) { prova_con_r(z, "0x5CFB", sub_5cfb); }
```
e il prototipo accanto agli altri `banco_r_*` (nemici.h r. 307-315); in
`routine_tab.c` la voce `0x5CFB` passa a `banco_r_5cfb` (oggi `sub_5cfb`). Le
quattro catture `_c` già nel banco restano valide (`prova_con_r` non cambia nulla
se R non è letto). Le forzate di 5CFB hanno il bit 5 acceso (`ix+00:20`) proprio
per non leggere R.

### Misure del 2026-11-03 sui binari già compilati (da `decomp/harness`)

- `./harness --tutte`: exit 0, 977 righe `^cattura`, 0 `FALLITO`, 250 righe
  `^RIEPILOGO`; `RIEPILOGO 0x5CFB: 4 catture, 4 OK`, `0x5E1A: 15 catture, 15 OK`,
  `0x5E9D: 3 catture, 3 OK`.
- Replay: exit 0, fino al frame 5499. `sched_proto`: `106124, 0, 131`.
- Nessun file `nemici*.c` sopra le 700 righe; `nemici_comuni.c` 434 righe,
  `nemici_tipo07a.c` 463, `nemici.h` 324, `routine_tab.c` 260.
- `grep -n 'nemici_manca(' decomp/harness/*.c | grep -v 'default:'` → solo
  `nemici.c:33` (definizione) e `nemici_stub.c:9` (la macro).

## Struttura da realizzare

**1. `harness/nemici_tipo07b.c`** (nuovo, stima 400 righe): intestazione di
commento (lotto N11, brief 37; tipo 07 seconda metà; RIPORTO e uscita anomala di
627F; tabelle ROM 60CC e 60EC lette con `sub_0008`), le funzioni `sub_5fc8`,
`sub_5fdc`, `sub_6046`, `sub_610c`, `sub_6151`, `sub_6165`, `sub_6179`,
`sub_6183`, `sub_618b`, `sub_6196`, `sub_61b1`, `sub_61fa`, `sub_6221`,
`sub_6248`, `sub_6252`, `sub_6261`, `sub_6270`, `sub_627f` e la statica
`corpo_627f`. Definire prima di usare, o dichiarare in `nemici.h`. I salti
`jp 6196`, `jp 6165`, `jp 6151`, `jp 61FA`, `jp 6221`, `jp 61B1` sono
`sub_xxxx(z); return;`. 6179 e 6183 che «cadono» in 618B: `sub_618b(z)` in coda
(A = 2 o 3 nel registro `z->a`). Commenti come nei moduli esistenti: indirizzo
del dasm accanto a ogni gruppo di istruzioni.

**2. `harness/nemici_comuni.c`** (434 righe): aggiungere `sub_2474` e `sub_4ada`
(dopo `sub_4b20`; l'intestazione cita «Ingressi: ...»: aggiungervi 2474 e 4ADA).
`sub_4ada` usa `M(0xc0bd + DE)` con D=0 (`ld d,0`), `sub_2457(z)`, `sub_3a2e(z)`
e scrive `M(0xc07b)=E`, `M(0xc07a)=D`, `M(0xc079)=C`, `M(0xc078)=B` (dal dasm
4AEB-4AF4). Stima finale 470 righe.

**3. `harness/nemici_tipo07a.c`**: due modifiche sole (sopra): riporto dopo
`sub_610c` in `sub_5cfb`, e il commento di testa. `sub_5e9d` non cambia
(`sub_627f(z)` ora è la funzione vera).

**4. `harness/nemici_stub.c`**: svuotato → `git rm decomp/harness/nemici_stub.c`.
Il Makefile prende `nemici*.c` con un `wildcard`: nessuna modifica. Restano
`nemici_manca`, `nemici_mancante` e `nemici_fuori_tabella` in `nemici.c` (usati dai
`default:` degli `switch` e da `replay.c`): non toccarli.

**5. `harness/nemici.h`**: togliere la sezione «Stub (nemici_stub.c)» (r. 317-322)
e aggiornare il commento di testa (r. 2-4: «stub (nemici_stub.c)» → togliere il
riferimento; «lotti N1-N12»). Prima di «Il registro R» una sezione «Lotto N11
(brief 37, nemici_tipo07b.c) e N12 (nemici_comuni.c)» con i prototipi delle 20
funzioni (tutte tranne `corpo_627f`); `banco_r_5cfb` tra i `banco_r_*`.

**6. `harness/nemici_r.c`**: la riga di `banco_r_5cfb`.

**7. `harness/routine_tab.c`**: 14 voci nuove, `{ "0x....", sub_xxxx, 0, 0 }`:
`0x2474` e `0x49AE` in ordine di indirizzo con le vicine; `0x5FC8`, `0x5FDC`,
`0x6046`, `0x610C`, `0x6165`, `0x6179`, `0x6196`, `0x61B1`, `0x61FA`, `0x6221`,
`0x6248`, `0x6252` dopo `0x5F4B`. `0x5CFB` → `banco_r_5cfb`. Senza voce: 6151,
6183, 618B, 6261, 6270, 627F, 4ADA (non raggiunte da MAME come ingressi, o interne:
coperte dalle forzate di 610C, 5CFB e 49AE).

**8. Catture.** Da `decomp/`. Misurate il 2026-11-03 in una copia di lavoro
(fuori dal repository, con `cattura.sh` e `snap.lua` originali): partendo da `out/`
vuota riescono tutte e 130 (`ls out/*_regs.txt | wc -l` → 130, 0 `FALLITA`).
Stati: `finale` parte dal frame 2300, `stanza50` dal 1951, `gioco` dal 2400,
`tardo` dal 4000, `porta` e `bonus` dal 2300.

Non fallire tutto se un frame fallisce: `cattura.sh` prova i frame in sequenza.

Non forzate (39 + 4 di 5CFB). File `/tmp/n11_nat.txt` (`ENTRY stato N [suffisso]`):
```
0x2474 finale 1
0x2474 finale 2 n2
0x2474 stanza50 1
0x2474 stanza50 2 n2
0x5FC8 finale 1
0x5FC8 finale 2 n2
0x5FDC finale 1
0x5FDC finale 2 n2
0x6046 finale 1
0x6046 finale 2 n2
0x610C finale 1
0x610C finale 2 n2
0x610C stanza50 1
0x610C stanza50 2 n2
0x6165 finale 1
0x6165 finale 2 n2
0x6165 stanza50 1
0x6179 finale 1
0x6179 finale 2 n2
0x6196 finale 1
0x6196 finale 2 n2
0x61B1 finale 1
0x61B1 finale 2 n2
0x61B1 stanza50 1
0x61B1 stanza50 2 n2
0x61FA finale 1
0x61FA finale 2 n2
0x6221 finale 1
0x6248 finale 1
0x6248 finale 2 n2
0x6252 finale 1
0x6252 finale 2 n2
0x6252 stanza50 1
0x49AE finale 1
0x49AE stanza50 1
0x49AE gioco 1
0x49AE tardo 1
0x49AE porta 1
0x49AE bonus 1
```
`xargs -P6 -L1 sh cattura.sh < /tmp/n11_nat.txt`. Le quattro di 5CFB con il bit 2 di
(IX+0) spento (filtro `SNAP_COND`, nessuna modifica di stato; le quattro `_c` già
nel banco sono quelle con il bit acceso):
```
for s in finale stanza50; do
  SNAP_COND='((b@c411)&4)==0' sh cattura.sh 0x5CFB $s 1 b &
  SNAP_COND='((b@c411)&4)==0' sh cattura.sh 0x5CFB $s 2 b2 &
done; wait
```
Non catturabili in naturale (FALLITA, misurato in `finale`, `stanza50`, e per 4ADA, 6151,
6183, 618B, 6261, 6270, 627F anche in `gioco`, `tardo`, `porta`, `bonus`): 4ADA, 6151,
6183, 618B, 6261, 6270, 627F; 6221 ha una sola occorrenza (la n2 fallisce); 6165 e 6252
in `stanza50` hanno solo la 1.

Forzate (87), `SNAP_FORZA` applicato all'ingresso, N=1, stato `finale` (sintassi
`snap.lua`: `ix+HH:VV`, `iy+HH:VV`, registri `a b c d e h l`, valori esadecimali).
Script e ciclo:
```
cat > /tmp/go37.sh <<'EOF'
SNAP_FORZA="$4" sh cattura.sh $1 $2 1 $3
EOF
xargs -P6 -L1 sh /tmp/go37.sh < /tmp/n11_forze.txt
```
con `/tmp/n11_forze.txt` (formato `ENTRY stato tag valori`):
```
0x5E1A finale forza10 iy+1d:0c,iy+1e:02
0x5FC8 finale forza01 ix+04:80
0x5FC8 finale forza02 ix+04:00,ix+01:00
0x5FC8 finale forza03 ix+04:00,ix+01:02
0x5FDC finale forza01 a:00
0x5FDC finale forza02 a:01
0x5FDC finale forza03 a:05,ix+04:00,ix+05:00
0x5FDC finale forza04 a:05,ix+04:20,ix+05:00
0x5FDC finale forza05 a:05,ix+04:00,ix+05:a0
0x5FDC finale forza06 a:0a,ix+04:00,ix+05:00
0x5FDC finale forza07 a:0a,ix+04:20,ix+05:00
0x5FDC finale forza08 a:0a,ix+04:00,ix+05:a0
0x5FDC finale forza09 a:0d,ix+04:00,ix+05:00
0x5FDC finale forza10 a:0d,ix+04:20,ix+05:00,iy+0e:00
0x5FDC finale forza11 a:0d,ix+04:20,ix+05:00,iy+0e:ff
0x5FDC finale forza12 a:0d,ix+04:00,ix+05:a0
0x6046 finale forza01 a:00
0x6046 finale forza02 a:04
0x6046 finale forza03 a:05,ix+04:00,ix+05:00
0x6046 finale forza04 a:05,ix+04:20,ix+05:00
0x6046 finale forza05 a:05,ix+04:00,ix+05:a0
0x6046 finale forza06 a:0a,ix+04:00,ix+05:00
0x6046 finale forza07 a:0a,ix+04:20,ix+05:00
0x6046 finale forza08 a:0a,ix+04:00,ix+05:a0
0x6046 finale forza09 a:0d,ix+04:00,ix+05:00
0x6046 finale forza10 a:0d,ix+04:20,ix+05:00
0x6046 finale forza11 a:0d,ix+04:00,ix+05:a0
0x610C finale forza01 iy+1a:01,ix+0e:ff,iy+1d:00
0x610C finale forza02 iy+1a:02,ix+0e:ff,iy+1d:00
0x610C finale forza03 iy+1a:03,ix+0e:ff,iy+1d:00
0x610C finale forza04 iy+1a:04,ix+0e:ff,iy+1d:00
0x610C finale forza05 iy+1a:05,ix+0e:ff,iy+1d:00
0x610C finale forza06 iy+1a:06,ix+0e:ff,iy+1d:00
0x610C finale forza07 iy+1a:07,ix+0e:ff,iy+1d:00
0x610C finale forza08 iy+1a:08,ix+0e:ff,iy+1d:00
0x610C finale forza09 iy+1a:09,ix+0e:ff,iy+1d:00
0x610C finale forza10 iy+1a:0a,ix+0e:ff,iy+1d:00
0x610C finale forza11 iy+1a:0b,ix+0e:ff,iy+1d:00
0x610C finale forza12 iy+1a:0c,ix+0e:ff,iy+1d:00
0x610C finale forza13 iy+1a:0d,ix+0e:ff,iy+1d:00
0x610C finale forza14 iy+1a:0e,ix+0e:ff,iy+1d:00
0x610C finale forza15 iy+1a:0f,ix+0e:ff,iy+1d:00
0x610C finale forza16 iy+1a:33,ix+0e:33
0x610C finale forza17 iy+1d:01,iy+1e:00,iy+1a:35,ix+0e:ff
0x610C finale forza18 iy+1d:01,iy+1e:00,iy+1a:3f,ix+0e:ff
0x610C finale forza19 iy+1d:01,iy+1e:01,iy+1a:35,ix+0e:ff
0x6165 finale forza01 iy+15:00
0x6165 finale forza02 iy+15:08
0x61FA finale forza01 iy+15:00
0x61FA finale forza02 iy+15:08
0x6221 finale forza01 iy+15:00
0x6221 finale forza02 iy+15:08
0x6252 finale forza01 iy+15:00
0x6252 finale forza02 iy+15:08
0x6179 finale forza01 iy+0e:00
0x6179 finale forza02 iy+0e:02
0x6248 finale forza01 iy+0e:00
0x6248 finale forza02 iy+0e:02
0x6196 finale forza01 iy+0e:00,iy+15:00
0x61B1 finale forza01 iy+0e:00
0x61B1 finale forza02 iy+0e:04,iy+15:00,a:00
0x61B1 finale forza03 iy+0e:04,iy+15:00,a:08
0x61B1 finale forza04 iy+0e:04,iy+15:00,a:0c
0x61B1 finale forza05 iy+0e:04,iy+15:04,a:00
0x61B1 finale forza06 iy+0e:0c,iy+15:0c,a:00
0x5CFB finale forza01 ix+00:20,ix+08:00,ix+09:ff,ix+0a:ff,ix+0e:ff,iy+1a:00,iy+1d:00,iy+12:00,iy+13:00
0x5CFB finale forza02 ix+00:20,ix+08:00,ix+09:ff,ix+0a:ff,ix+0e:ff,iy+1a:00,iy+1d:00,iy+12:01,iy+13:00
0x5CFB finale forza03 ix+00:20,ix+08:00,ix+09:ff,ix+0a:ff,ix+0e:ff,iy+1a:04,iy+1d:00,ix+0b:00,ix+0c:00
0x5CFB finale forza04 ix+00:20,ix+08:00,ix+09:ff,ix+0a:ff,ix+0e:ff,iy+1a:04,iy+1d:0c,ix+0b:ff,ix+0c:ff
0x5CFB finale forza05 ix+00:20,ix+08:00,ix+09:ff,ix+0a:ff,ix+0e:ff,iy+1a:07,iy+1d:0c,ix+0b:ff,ix+0c:ff
0x5CFB finale forza06 ix+00:20,ix+08:00,ix+09:ff,ix+0a:ff,ix+0e:ff,iy+1a:0d,iy+1d:0c,ix+0b:ff,ix+0c:ff
0x5CFB finale forza07 ix+00:20,ix+08:00,ix+09:ff,ix+0a:ff,ix+0e:ff,iy+1a:0e,iy+1d:0c,ix+0b:ff,ix+0c:ff
0x5CFB finale forza08 ix+00:20,ix+08:00,ix+09:ff,ix+0a:ff,ix+0e:ff,iy+1a:03,iy+1d:0c,ix+0b:ff,ix+0c:ff
0x5CFB finale forza09 ix+00:20,ix+08:00,ix+09:ff,ix+0a:ff,ix+0e:ff,iy+1a:01,iy+1d:0c,ix+0b:ff,ix+0c:ff,ix+04:00,ix+05:00
0x5CFB finale forza10 ix+00:20,ix+08:00,ix+09:ff,ix+0a:ff,ix+0e:ff,iy+1a:01,iy+1d:0c,ix+0b:ff,ix+0c:ff,ix+04:20,ix+05:00
0x5CFB finale forza11 ix+00:20,ix+08:00,ix+09:ff,ix+0a:ff,ix+0e:ff,iy+1a:01,iy+1d:0c,ix+0b:ff,ix+0c:ff,ix+04:20,ix+05:00,iy+12:01
0x49AE finale forza01 a:05
0x49AE finale forza02 a:06
0x49AE finale forza03 a:07
0x49AE finale forza04 a:08
0x49AE finale forza05 a:09
0x49AE finale forza06 a:0a
0x49AE finale forza07 a:0b
0x49AE finale forza08 a:0c
0x49AE finale forza09 a:0d
0x49AE finale forza10 a:0e
0x49AE finale forza11 a:0f
```
Rami aperti (dedotti dal dasm; la cattura riesce, misurato; l'apertura del ramo è
«attesa, non misurata», tranne dove detto): 610C `forza01-15` una per
destinazione (indice 0 escluso: è l'uscita anomala di 627F), `forza16` `ret z`,
`forza17` `ret nz`, `forza18` nibble 0F, `forza19` filtro non soddisfatto; 5CFB
`forza01-02` → 610C indice 0 → 627F con C=2 / C=0 (misurato: HL=5D0F), `forza03` `ret c`
dopo 4FEF, `forza04-08` → 5E1A con destinazioni 5E60, 5FC8, 5EDD (due), 61B1,
`forza09-11` → 5E9D (`forza10-11` misurato: 627F con HL=5D28); 5E1A `forza10` → 5FC8
(destinazione 2, mai esercitata prima); 49AE `forza01-03` (A=5-7) chiamano 4ADA
da 4AB6, `forza04-11` (A=8-0F) da 4ACF. Non fare forzate con indice fuori tabella.
Se una cattura nel banco fallisce perché il C di una routine fuori lotto (49AE, 4B20,
4AF6) differisce dall'originale, vedi «Domande aperte» n. 4.

Totali (misurati in copia): 130 catture (43 non forzate, 87 forzate), in 16 voci.
Per voce (non forzate + forzate = totale nel banco, escluso il preesistente di 5CFB e
5E1A): 2474 4+0=4; 49AE 6+11=17; 5CFB 4+11=15 (19 con le 4 `_c`); 5E1A 0+1=1 (16 con le
15); 5FC8 2+3=5; 5FDC 2+12=14; 6046 2+11=13; 610C 4+19=23; 6165 3+2=5; 6179 2+2=4; 6196
2+1=3; 61B1 4+6=10; 61FA 2+2=4; 6221 1+2=3; 6248 2+2=4; 6252 3+2=5.

**9. Copertura del C** (gcov, fuori dal repository; prefisso `h-`):
```
D=/tmp/cov37 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
 && ln -s "$PWD/decomp/out" $D/out && (cd $D && gcc -std=c11 -O0 --coverage -o h \
 harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c z80_*.c \
 nemici*.c attivita.c attivita_01_10.c && ./h --tutte > /dev/null; \
 for f in nemici_tipo07b nemici_comuni nemici_tipo07a; do gcov h-$f.gcda > /dev/null 2>&1; \
 echo "== $f"; awk -F: '$1 ~ /#####/' $f.c.gcov; done)
```
Nel rapporto: le righe non eseguite e il motivo. Attese (stima, non misurata): `case 0` e
`default` dello `switch` di 610C (indice sempre < 16); alcuni rami di 61B1, 5FDC e
6046 che dipendono da celle o da (IY+0E)/(IY+15) combinati (aggiungere una forzata,
stesso schema, tag `forzaNN` successivo, solo se il ramo è raggiungibile senza passare da
routine non catturabili); in `nemici_comuni.c` tutto ciò che 49AE/4B20 non esercitano
(non è compito di questo lotto coprirlo, ma `sub_2474` e `sub_4ada` devono essere
interamente coperte); in `nemici_tipo07a.c` le righe 5D08-5D28 di 5CFB, 5ED6-5EDA di
5E9D e la chiamata a `sub_5fc8` devono ora risultare coperte.

**10. Documenti.**
`ROUTINE.md`: in fondo (dopo r. 676) la sezione `## Motore dei nemici (lotti N11 e
N12)` con un paragrafo (moduli `nemici_tipo07b.c` e `nemici_comuni.c`; `nemici_stub.c`
tolto, motore completo; già fatte 5FAC e 5E4D; 49AE entra nel banco per coprire 4ADA;
uscita anomala di 627F e convenzione del riporto; wrapper `banco_r_5cfb`; 6151, 6183,
618B, 6261, 6270, 627F, 4ADA senza ingresso catturabile) e la tabella come quella della
N10 (una riga per routine: 20 più 49AE), poi l'elenco delle forzate e le righe gcov non
coperte. Stato `verificata` per le voci con 2 o più non forzate (2474, 49AE, 5FC8, 5FDC,
6046, 610C, 6165, 6179, 6196, 61B1, 61FA, 6248, 6252), `tradotta` per 6221 (1 non
forzata) e per le 7 senza voce. Nella sezione N8 aggiornare le righe di 5CFB (catture
OK 19; note: «15 nuove con il bit 2 spento: il ramo 5D08-5D28 è nel banco») e di 5E1A
(16; la destinazione 2 (5FC8) è esercitata da `forza10`), e sostituire in fondo alla
sezione N8 le «Non coperte da gcov» di 5D08-5D28, 5FC8 e 627F con un rinvio alla nuova
sezione. `CENSIMENTO-NEMICI.md`: dopo «Lotto N10 fatto» un paragrafo «**Lotto N11 e N12
fatti** (brief 37)» con le 20 routine, lo stub da 5 a 0 (`nemici_stub.c` tolto), le voci
nuove nel banco (14, compresa 49AE; 5CFB e 5E1A cresciute), le 130 catture e la frase «con N11 e N12 il motore dei nemici è completo».
Aggiornare a r. 516 la riga «Stub di N1» con «(tutti tradotti)».

## Verifiche obbligatorie

Tutte da `/home/rufio72/OpenSolomonsKey-master`; uscita attesa esatta salvo dove è
scritto «attesa, dedotta». Abbreviazione:
`V="2474 49AE 5CFB 5E1A 5FC8 5FDC 6046 610C 6165 6179 6196 61B1 61FA 6221 6248 6252"`.

1. Disassemblato: `wc -l < decomp/out/nemici_n11.dasm; grep -c 'ld a,r' decomp/out/nemici_n11.dasm`
   → `372` e `0` (misurate).
2. Compilazione: `cd decomp/harness && make harness replay sched_proto > /tmp/m37.txt 2>&1; echo "make $?"; grep -c -E 'warning|error|undefined' /tmp/m37.txt`
   → `make 0` e `0` (il link non trova simboli mancanti: tutte le `sub_` chiamate
   esistono).
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h37.txt; echo "exit $?"; grep -c FALLITO /tmp/h37.txt; grep -c '^RIEPILOGO' /tmp/h37.txt; grep -c '^cattura' /tmp/h37.txt`
   → `exit 0`, `0`, `264`, `1107` (250+14; 977+130; attese, dedotte).
4. Catture per voce: `for a in $V; do grep "^RIEPILOGO 0x$a:" /tmp/h37.txt; done`
   → (attese, dedotte dai file elencati sopra)
   ```
   RIEPILOGO 0x2474: 4 catture, 4 OK
   RIEPILOGO 0x49AE: 17 catture, 17 OK
   RIEPILOGO 0x5CFB: 19 catture, 19 OK
   RIEPILOGO 0x5E1A: 16 catture, 16 OK
   RIEPILOGO 0x5FC8: 5 catture, 5 OK
   RIEPILOGO 0x5FDC: 14 catture, 14 OK
   RIEPILOGO 0x6046: 13 catture, 13 OK
   RIEPILOGO 0x610C: 23 catture, 23 OK
   RIEPILOGO 0x6165: 5 catture, 5 OK
   RIEPILOGO 0x6179: 4 catture, 4 OK
   RIEPILOGO 0x6196: 3 catture, 3 OK
   RIEPILOGO 0x61B1: 10 catture, 10 OK
   RIEPILOGO 0x61FA: 4 catture, 4 OK
   RIEPILOGO 0x6221: 3 catture, 3 OK
   RIEPILOGO 0x6248: 4 catture, 4 OK
   RIEPILOGO 0x6252: 5 catture, 5 OK
   ```
5. Non forzate per voce: `cd decomp && for a in $V; do ls out/snap_0x${a}_*_regs.txt | grep -vc forza; done | tr '\n' ' '`
   → `4 6 8 0 2 2 2 4 3 2 2 4 2 1 2 3 ` (misurate nella copia; 5CFB: 4 nuove più
   le 4 `_c` esistenti = 8).
6. Totale file: `cd decomp && for a in $V; do ls out/snap_0x${a}_*_regs.txt; done | wc -l`
   → `149` (130 nuovi + 4 `_c` di 5CFB + 15 di 5E1A: 4+17+19+16+5+14+13+23+5+4+3+10+4+3+4+5;
   attesa, dedotta dalle misure per voce).
7. Voci precedenti invariate: `for a in 5E4D 5E8F 5E9D 5F0C 5F4B 4AF6 4B0E 2457 3A2E 36F0 4FEF; do grep "^RIEPILOGO 0x$a:" /tmp/h37.txt; done`
   → (misurate prima della modifica) `5E4D: 4 catture, 4 OK`; `5E9D: 3 catture, 3 OK`;
   `4AF6`, `4B0E`, `2457`, `3A2E`, `36F0`, `4FEF`: `2 catture, 2 OK`; per il resto salvare
   l'uscita di `./harness --tutte` PRIMA di modificare qualunque file in `/tmp/h36_prima.txt`
   e confrontare: `diff <(grep '^RIEPILOGO' /tmp/h36_prima.txt)
   <(grep '^RIEPILOGO' /tmp/h37.txt)` → solo le righe di 5CFB, 5E1A e le 14 nuove, nessuna
   riga persa o cambiata altrove; attesa, dedotta).
8. `sched_proto`: `cd decomp && harness/sched_proto out/sched_log.txt | tail -1`
   → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
9. Replay: `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r37.txt; echo "exit $?"; tail -4 /tmp/r37.txt; grep -c -i manca /tmp/r37.txt`
   → `exit 0`, e (misurate sul binario di oggi):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
   e `0` (nessuna routine mancante raggiunta). Se i numeri cambiano perché il replay
   ora esegue codice nuovo (610C, 5FC8), riportare i nuovi numeri nel rapporto e fermarsi:
   «nello spedidore» deve restare 0.
10. Motore completo: `test ! -e decomp/harness/nemici_stub.c && echo assente; grep -rn -E 'STUB_NEMICI|nemici_stub' decomp/harness --include='*.c' --include='*.h' --include='Makefile' | wc -l; grep -n 'nemici_manca(' decomp/harness/*.c | grep -v 'default:'`
    → `assente`, `0`, e la sola riga `decomp/harness/nemici.c:33:void nemici_manca(uint16_t ingresso)`
    (attese, dedotte; oggi la seconda riga mostra anche `nemici_stub.c:9`).
11. Ogni ingresso del lotto ha una funzione: `for a in 2474 4ada 5fc8 5fdc 6046 610c 6151 6165 6179 6183 618b 6196 61b1 61fa 6221 6248 6252 6261 6270 627f; do grep -c -E "^void sub_$a\(struct z80 \*z\)$" decomp/harness/*.c | awk -F: -v a=$a '{s+=$2} END{printf "%s:%d ", a, s}'; done; echo`
    → `2474:1 4ada:1 5fc8:1 5fdc:1 6046:1 610c:1 6151:1 6165:1 6179:1 6183:1 618b:1 6196:1 61b1:1 61fa:1 6221:1 6248:1 6252:1 6261:1 6270:1 627f:1 `
    (attesa, dedotta).
12. Copertura: il comando del punto 9 di «Struttura». Uscita attesa (attesa, non
    misurata): ogni riga `#####` spiegata nel rapporto; `sub_2474` e `sub_4ada` senza
    righe `#####`; 5D08-5D28 di 5CFB, 5ED6-5EDA di 5E9D e la chiamata a `sub_5fc8` coperte.
13. Dimensioni: `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/nemici*.c | awk '$1>700 && $2!="totale" && $2!="total"'` → vuoto.
14. File vietati intatti: `git diff --name-only HEAD | grep -E 'cattura\.sh|snap\.lua|sessioni|replay\.|sched|rst20|rst38_tables|censimento_nemici\.py|STATO\.md|README\.md|compiti/'` → vuoto.
15. Ambito: `git status --short | grep -v -E '^( M|\?\?|A |D | D) (compiti/37-|decomp/(harness/(nemici_tipo07a\.c|nemici_tipo07b\.c|nemici_comuni\.c|nemici_r\.c|nemici\.h|nemici_stub\.c|routine_tab\.c)|ROUTINE\.md|CENSIMENTO-NEMICI\.md|out/(snap_0x(2474|49AE|5CFB|5E1A|5FC8|5FDC|6046|610C|6165|6179|6196|61B1|61FA|6221|6248|6252)_|nemici_n11\.dasm)))'`
    → vuoto; `git status --short | grep -c -E '\.(gcda|gcno)$'` → `0`.
16. Documenti: `grep -c '^## Motore dei nemici (lotti N11 e N12)$' decomp/ROUTINE.md; grep -c 'Lotto N11 e N12 fatti' decomp/CENSIMENTO-NEMICI.md`
    → `1` e `1`.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotti N11 e N12 del motore dei nemici: tipo 07 seconda metà, 2474 e 4ADA; nemici_stub.c tolto, motore completo`
  e riga finale `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`
  (se il lotto passa a Opus per escalation, `Claude Opus 5.5`). Il commit comprende
  `decomp/out/nemici_n11.dasm` e le catture nuove (`prima.bin`, `dopo.bin`, `regs.txt`,
  `.log`; con `git add -f`) e la cancellazione di `nemici_stub.c`.
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-16; l'elenco delle voci
  con numero di catture e stato; le righe gcov non coperte; le deviazioni dal brief
  (cattura che fallisce o scartata, voce scartata, cambio di stato).

## Domande aperte

1. **2474 e 4ADA in `nemici_comuni.c`** (il censimento le assegna a quel modulo,
   «routine condivise con i corpi 01/10»), non in `nemici_tipo07b.c`; il modulo 07b
   resta di sole routine del tipo 7. Proposta: come scritto. — «preferenza»
2. **`nemici_stub.c` tolto con `git rm`** (non lasciato come file di soli commenti): il
   Makefile prende `nemici*.c` con `wildcard`; `nemici_manca` resta in `nemici.c`.
   Proposta: come scritto. — «preferenza»
3. **Uscita anomala di 627F e riporto di `sub_610c`**: `sub_5cfb` smette di proseguire
   se `sub_610c` lascia il riporto acceso (stessa convenzione di 531C nei brief 32 e 36);
   `sub_627f` ricostruisce HL (5D28 da 5E9D, 5D0F da 610C). Cambia l'interfaccia di
   `sub_610c` e il corpo di `sub_5cfb` (già nel banco, 4 catture). Proposta: come scritto.
   — «preferenza» (precedente ratificato)
4. **49AE entra nel banco come voce nuova** (non è nel perimetro N11/N12, oggi
   «tradotta (replay)» senza catture), perché 4ADA non ha alcun ingresso catturabile e
   49AE è l'unico modo per esercitarla (11 forzate `a:05-0f` più 6 naturali). Se qualche
   cattura di 49AE fallisce nel banco per un errore di 49AE o 4B20 e non di 4ADA/2474,
   proposta: correggere solo se la causa è chiara e confinata in `nemici_comuni.c`
   (riportarlo nel rapporto), altrimenti togliere la cattura e lo stato di 49AE resta
   `tradotta`, con 4ADA dichiarata non verificata. — «scelta di progetto»
5. **Wrapper `banco_r_5cfb`** in `nemici_r.c` e voce `0x5CFB` sul wrapper (le nuove
   catture naturali con il bit 2 spento possono arrivare a 5DDF); regola già ratificata.
   — «preferenza»
6. **6221 con una sola cattura non forzata** (la seconda occorrenza non esiste negli
   stati provati): entra nel banco con stato `tradotta` (più 2 forzate). — «preferenza»
7. **Sette routine senza voce** (6151, 6183, 618B, 6261, 6270, 627F, 4ADA): `tradotta`,
   coperte dalle forzate di 610C, 5CFB, 49AE e da 6179 (618B). — «preferenza»
8. **`STATO.md` e `README.md`**: li aggiorna la chat principale dopo la verifica (banco
   a 1107 catture e 264 voci, motore dei nemici completo). — «preferenza»

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Domande 1-3 e 5-8 sono preferenze, accolte con la proposta dell'analista:
1. Sì: 2474 e 4ADA in `nemici_comuni.c`, come da censimento.
2. Sì: `nemici_stub.c` tolto con `git rm` (e riferimenti nel Makefile/header sistemati di conseguenza).
3. Sì: convenzione del riporto per l'uscita anomala di 627F (come 531C), con le modifiche a `sub_610c` e `sub_5cfb` e HL ricostruito come misurato.
5. Sì: wrapper `banco_r_5cfb` in `nemici_r.c`.
6. Sì: 6221 con una sola cattura non forzata entra come `tradotta`.
7. Sì: le sette routine senza voce (6151, 6183, 618B, 6261, 6270, 627F, 4ADA) si segnano `tradotta`, coperte dalle forzate dei genitori.
8. Sì: STATO.md e README.md li aggiorna la chat principale dopo la verifica.

Domanda 4 (scelta di progetto, risolta dalla chat principale sotto il mandato
dell'utente «fai il lotto N11 con N12 accorpato», coerente con la prassi
ratificata dei lotti precedenti): sì, 49AE entra nel banco come voce nuova.
Se una cattura fallisce per un errore di 49AE o 4B20 si corregge solo a causa
chiara e confinata in `nemici_comuni.c`; altrimenti si toglie la cattura, si
annota in ROUTINE.md e 4ADA resta non verificata. La risoluzione va segnalata
all'utente nel rapporto di fine ciclo.

## Esito (chat principale, 2026-11-03)

Passato al primo colpo, nono ciclo con lo schema economico (analista
Sonnet ~184k token, sviluppatore Sonnet ~100k, verifica a script senza
modello). IL MOTORE DEI NEMICI È COMPLETO: 18 routine del tipo 07b in
`nemici_tipo07b.c`, 2474 e 4ADA in `nemici_comuni.c`, `nemici_stub.c`
tolto con `git rm`, 220/220 routine tradotte. Banco a 264 voci e 1110
catture con 0 fallite, replay a fine oracolo (frame 5499) senza
divergenze nello spedidore, `sched_proto` invariato. La domanda 4
(scelta di progetto, 49AE nel banco) si è risolta nel migliore dei
modi: 49AE verificata con 17 catture, nessuna cattura fallita o
scartata, 4ADA coperta per intero. Deviazioni: 3 forzate in più su
610C per coprire 6183 e i salti 6261/6270→61B1 (1110 catture invece
di 1107); BC finale di 5FDC seguiva il disassemblato (0600/0601),
non il brief. Bug trovato e corretto in corsa: 2474 leggeva la
tabella ROM 9495 con `M()` invece di `leggi()`. Scoperture gcov
residue: solo i casi `0` e `default` dello switch di 610C (nibble
sempre < 16, indici fuori tabella non forzati per regola).
