# Compito 36: lotto N10 del motore dei nemici, tipo 06 prima metà (4FD4, 5005-51D3)

Data: 2026-11-03. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere.

## Contesto: file da leggere

Da leggere (e basta):
1. `decomp/out/nemici_n10.dasm`, da generare (vedi sotto): 248 righe di
   disassemblato delle 19 routine del lotto. Dentro ci sono due tabelle di
   dati disassemblate come istruzioni, che non sono codice: 4FE1-4FEE (parole
   del `rst 38` di 4FD4) e 5128-5139 (parole del `rst 38` di 5124).
2. `decomp/harness/nemici.h`: r. 1-60 (riporto, `nemici_rst38`), r. 214-232
   (sezione N6), r. 262-299 (registro R e stub).
3. `decomp/harness/nemici_tipo06a.c` (169 righe: tutto, è il file che cresce;
   ha già `sub_24df`, `sub_51d7`, `sub_5068`, `sub_5224`, `sub_523c`,
   `sub_5254`, `sub_51fa`, `sub_4fef`).
4. `decomp/harness/nemici_stub.c` (16 righe), `harness/nemici_r.c` r. 126-172
   (`prova_con_r` e i wrapper), `harness/routine_tab.c` (solo r. 120-126),
   `harness/nemici_tipo06b.c` r. 1-22 (modello di `IXB`/`IYB`) e r. 261-282
   (`sub_531c`), `decomp/ROUTINE.md` (solo r. 589-628: sezione N9 come
   modello), `decomp/CENSIMENTO-NEMICI.md` (solo il paragrafo «**Lotto N9
   fatto**»).

È vietato leggere `STATO.md`, `README.md`, `ROUTINE.md` per intero, gli
altri brief in `compiti/` e i moduli `nemici_*.c` non elencati sopra.

Generazione del disassemblato (da `/home/rufio72/OpenSolomonsKey-master`;
`awk` è mawk; i range escludono 4FEF e 5068, già tradotte):
```
cd decomp && awk 'BEGIN{n=split("4fd4 5005 5092",lo," "); split("4fee 5067 51d6",hi," ")}
/^[A-Za-z_0-9]+:/ {lab=$0; next}
/;[0-9a-f][0-9a-f][0-9a-f][0-9a-f][ \t]*$/ {a=substr($0,length($0)-3); ok=0; for(i=1;i<=n;i++) if(a>=lo[i] && a<=hi[i]) ok=1; if(ok){ if(lab!="") print lab; print} lab=""; next}
{lab=""}' out/main64.dasm > out/nemici_n10.dasm; cd ..
```
Uscita attesa (misurata): `wc -l < decomp/out/nemici_n10.dasm` → `248`;
`grep -c 'ld a,r' decomp/out/nemici_n10.dasm` → `0` (nessuna routine del
lotto legge R direttamente, ma 4FD4 e 50DF arrivano a `ld a,r` via 51FA, 51D7,
54C5: vedi «Il registro R»). Il file è ignorato da git (`decomp/out/*`): si
aggiunge al commit con `git add -f`.

## Obiettivo

Tradurre in C le 19 routine del lotto N10 ancora da fare (4FD4, 5005, 5023,
5035, 5066, 5092, 50BA, 50DF, 5115, 5124, 513A, 513E, 514C, 516B, 5186, 51A4,
51B1, 51B5, 51D3) nel modulo esistente `nemici_tipo06a.c` (169 righe, stima
finale circa 430); togliere lo stub 4FD4; portare 10 voci nel banco con 91
catture MAME; due wrapper `banco_r_*` per il registro R. Il replay e
`sched_proto` non devono cambiare: il replay non raggiunge 4FD4 (altrimenti lo
stub avrebbe fermato la spedizione; misurato: finisce al frame 5499 con exit 0).

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
  800 righe e nessun `nemici*.c` sopra le 700 (`nemici_tipo06a.c` sopra le 650:
  si spezzerebbe, ma la stima è 430).
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`); si
  ripristina un file solo con `git checkout -- <file>`.
- Un solo agente alla volta su `decomp/harness`.

## Cosa esiste già (fatti verificati)

**Stato reale delle 27 routine N10 del censimento** (da `CENSIMENTO-NEMICI.md`
e `ROUTINE.md`; la lista della richiesta iniziale dava 51B5 per fatta: non lo
è, ha solo la riga di censimento).

Già fatte, non si toccano (8): 24DF (N3, `verificata`, 2 catture), 51D7 (N3,
`verificata`, 2, wrapper `banco_r_51d7`), 5068 (N6, `verificata`, 2), 51FA
(N6, `verificata`, 30, wrapper `banco_r_51fa`), 5224 (N6, `verificata`, 3),
523C e 5254 (N6, `tradotta`, coperte dalle forzate di 51FA), 4FEF (N8,
`verificata`, 2). Il tipo 6 seconda metà (5274-531C, 52FD, 5345) è in
`nemici_tipo06b.c`, già tradotto.

Da fare (19), tutte in `nemici_tipo06a.c`; byte e chiamanti dal censimento
(`(t)` = già tradotta):

| ingresso | byte | chiamanti | chiama | note dal dasm |
|---|---:|---|---|---|
| 4FD4 | 13 | 4F0F (`salta_4f1b`, `nemici.c` r. 65, oggi stub) | 52FD(t), 4FEF(t), `rst 38` 4FE0 | A=(IY+4), BC dalla tabella 4FF9, E=(IY+1D) |
| 5005 | 30 | 4FD4 | - | bit 2 di (IX+0), (IX+0D) >= 3, (IY+1D) = 3 |
| 5023 | 18 | 4FD4 | 5035, 5066 | c2b6=ff, bit 7 di (IX+1) spento, B=(IY+15); cade in 5035 |
| 5035 | 49 | 5023, 5092 | 5066 | ingresso interno: nibble di B+3 < 6, ret |
| 5066 | 2 | 5023, 5035 | 5068(t) | B=3 e cade in 5068; ingresso interno |
| 5092 | 40 | 4FD4 | 5035, 5068(t) | c2b6=ff, ramo su (IY+0C), poi 5035 o B=6/9 e 5068 |
| 50BA | 37 | 4FD4 | 531C(t) | (IY+13)+1 < 2: 531C o riallinea (IY+15) |
| 50DF | 54 | 4FD4 | 531C(t), 5345(t), 5124, 5115, 51FA(t) | `sbc hl,bc`, shift su BC, bit 5 di (IX+0) |
| 5115 | 15 | 50DF | - | B &= 11, C &= a3, se B\|C = 0 allora BC = 0081 |
| 5124 | 4 | 50DF | `rst 38` 5127 (9 voci) | E=(IX+7) |
| 513A | 4 | 5124 | - | BC = 0040 |
| 513E | 14 | 5124 | - | `ret nz` sul risultato di `and c` |
| 514C | 31 | 5124 | - | `ret` in 3 punti, set bit 7 di (IX+1) |
| 516B | 27 | 5124 | - | `ret nz` sul risultato di `or b` |
| 5186 | 30 | 5124 | - | - |
| 51A4 | 13 | 5124 | - | - |
| 51B1 | 4 | 5124 | - | BC = 0080; ret (anche coda di 51A4) |
| 51B5 | 30 | 5124 | 4FEF(t), 5068(t) | A=(IY+4), BC da 4FEF, (IX+0B/0C) = BC-3, c2b6=ff, B=3c, coda 5068 |
| 51D3 | 4 | 5124 | - | BC = 0001 |

Attenzione: nel dasm 51A4 (51A4-51B0) arriva a `set 7,(ix+1)` (51AD) e poi
cade in 51B1 (`ld bc,0080; ret`, 51B1-51B4), che è anche voce di tabella di
5124. In C 51A4 chiama `sub_51b1(z)` dopo il `set`; il `ret nz` di 51AC resta
un `return`.

Tabelle `rst 38` già in `rst38_tables.h` (da grep): `rst38_4FE0` (7 voci:
5005, 5023, 5092, 3B62, 3B62, 50DF, 50BA) e `rst38_5127` (9 voci: 513A, 513E,
514C, 516B, 5186, 51A4, 51B1, 51B5, 51D3). `sub_3b62` e tutte le altre
destinazioni esistono (nemici.h r. 76).

Firme (da `grafo symbol`/grep; `void sub_xxxx(struct z80 *z)`): `sub_52fd`,
`sub_4fef`, `sub_5345`, `sub_531c`, `sub_5068`, `sub_51fa`, `sub_3b62`
(nemici.h). Convenzione di `sub_531c`
(`nemici_tipo06b.c` r. 261-282, da `grafo symbol`): riporto 0 in `z->f` =
uscita normale; riporto 1 = cella piena, la routine ha già chiamato
`sub_5068` (nell'originale `pop hl; jp 5068`) e il chiamante (50BA, 50DF) deve
ritornare subito.

Misure del 2026-11-03 sui binari già compilati (da `decomp/harness`):
- `./harness --tutte`: exit 0, 886 righe `^cattura`, 0 `FALLITO`, 240 righe
  `^RIEPILOGO`. Replay: exit 0, fino al frame 5499. `sched_proto`:
  `106124, 0, 131`.
- `grep -c STUB_NEMICI nemici_stub.c` → 7 (6 stub più la riga `#define`:
  2474, 4ada, 4fd4, 5fc8, 610c, 627f).
- Nessun file `nemici*.c` sopra le 700 righe (il più grande: 697).

### Convenzioni di traduzione

- Ogni routine è `void sub_xxxx(struct z80 *z)`, tutte non statiche (come N9).
  Registri `z->a b c d e h l`, `z->ix` (slot), `z->iy` (oggetto), `z->f`.
  Memoria con `M(indirizzo)`; ROM con `leggi(indirizzo)`; coppie con
  `BC(z) DE(z) HL(z)` e `SET_BC SET_DE SET_HL`. Aggiungere `#define IXB(o)
  M(z->ix + (o))` e `IYB(o)` come in `nemici_tipo06b.c`. Il modulo ha già
  `#include "routine.h"`, `"nemici.h"`, `"rst38_tables.h"`.
- Il banco confronta RAM (c000-e5ff, salvo 0x40 byte sotto SP) e A, BC, DE,
  HL, IX, IY all'uscita, non F: anche i registri «sporchi» devono coincidere
  (A, B, C, DE, HL lasciati dalle sottoroutine e dai rami `ret`). Le catture
  di `rst 38` lasciano DE = sito + 2 + 2·E (lo fa `nemici_rst38`).
- `rst 38` a 4FE0: `nemici_rst38(z, 0x4FE0, rst38_4FE0, 7)`; a 5127:
  `nemici_rst38(z, 0x5127, rst38_5127, 9)`; poi `switch` sulla destinazione
  come `sub_51fa` (nemici_tipo06a.c r. 134-157): `0` = indice fuori tabella
  (non fare nulla), `default` = `nemici_manca(dest)`. Le destinazioni
  ritornano al chiamante del genitore (nell'originale sono `jp` impliciti
  dalla `rst 38`): dopo lo `switch` il genitore termina.
- Le destinazioni 3B62 e 3B62 (indici 3 e 4 di 4FD4) sono la voce nulla:
  `sub_3b62(z)`.
- Salti di coda: `jp 5035` (50AA), `jr 5066` (5033, 503C), `jr 5068` (50B8),
  `jp 5068` (51D0) si traducono `sub_xxxx(z); return;`. 5023 finisce cadendo
  in 5035 (`sub_5035` in coda); 5066 cade in 5068.
- 50DF: dopo `sub_531c(z)` controllare `NEMICI_RIPORTO_ACCESO(z)` e
  ritornare subito. Poi `sbc hl,bc` con riporto entrante 0: HL = (IX+0B/0C) -
  BC resta nei registri anche se esce con `ret c` (HL deve coincidere). Dopo
  `sub_5345(z)`: `sla c; srl b; rr c` (il riporto di `sla` viene perso,
  `rr c` prende il bit 0 di B prima di `srl`), poi (IX+0E/0F) = C/B.
- 4FD4: l'ordine è `sub_52fd`, A=(IY+4), `sub_4fef`, E=(IY+1D), `rst 38`.
  52FD può cambiare (IX+8) e il bit 5 di (IX+0) prima dei rami.
- Catture `forza*` valgono come catture OK ma non contano per lo stato
  `verificata`, che richiede almeno 2 catture non forzate.

### Il registro R

4FD4 e 50DF chiamano 51FA, che arriva ai due `ld a,r` (51D7, 54C5). Regola
ratificata: niente R nelle catture, wrapper `banco_r_*` in `nemici_r.c`.
Aggiungere in coda a `nemici_r.c`:
```
void banco_r_4fd4(struct z80 *z) { prova_con_r(z, "0x4FD4", sub_4fd4); }
void banco_r_50df(struct z80 *z) { prova_con_r(z, "0x50DF", sub_50df); }
```
e i prototipi in `nemici.h` accanto agli altri `banco_r_*` (r. 285-291). In
`routine_tab.c` le voci 4FD4 e 50DF puntano ai wrapper. Le altre routine del
lotto non leggono R (5124 e le sue destinazioni, 5005, 5023, 5092, 50BA non
arrivano a 51FA).

## Struttura da realizzare

**1. `harness/nemici_tipo06a.c`** (esistente, 169 righe): aggiornare
l'intestazione (aggiungere «Lotto N10 (brief 36): 4FD4, 5005, 5023, 5035,
5066, 5092, 50BA, 50DF, 5115, 5124, 513A, 513E, 514C, 516B, 5186, 51A4, 51B1,
51B5, 51D3; tipo 6 = 4FD4») e aggiungere in ordine libero le 19 funzioni
`sub_4fd4`, `sub_5005`, `sub_5023`, `sub_5035`, `sub_5066`, `sub_5092`,
`sub_50ba`, `sub_50df`, `sub_5115`, `sub_5124`, `sub_513a`, `sub_513e`,
`sub_514c`, `sub_516b`, `sub_5186`, `sub_51a4`, `sub_51b1`, `sub_51b5`,
`sub_51d3`. Le funzioni vanno definite prima di essere usate o dichiarate in
`nemici.h`. Commenti come nei moduli esistenti: indirizzo del dasm accanto a
ogni gruppo di istruzioni. Costante utile: `c2b6` (RAM, scritta a ff da 5023,
5092, 51B5; usare `M(0xc2b6)`).

**2. `harness/nemici_stub.c`**: togliere la riga `STUB_NEMICI(4fd4)`. Restano
2474, 4ada, 5fc8, 610c, 627f.

**3. `harness/nemici.h`**: togliere `sub_4fd4` dalla sezione «Stub». Dopo la
sezione N9 («Lotto N9 (brief 35)») e prima di «Il registro R» una sezione
«Lotto N10 (brief 36)» con i prototipi delle 19 funzioni. Tra i `banco_r_*`
aggiungere `banco_r_4fd4` e `banco_r_50df`.

**4. `harness/nemici_r.c`**: le due righe dei wrapper (sopra).

**5. `harness/routine_tab.c`**: 10 voci in ordine di indirizzo dopo
`0x4F40` e prima/dopo quelle esistenti: `{ "0x4FD4", banco_r_4fd4, 0, 0 }`,
poi (dopo `0x4FEF`) `0x5005`, `0x5023` con `sub_xxxx`; dopo `0x5068`:
`0x50BA` (`sub_50ba`), `{ "0x50DF", banco_r_50df, 0, 0 }`, `0x5124`,
`0x513E`, `0x514C`, `0x516B`, `0x51D3` (prima di `0x51D7`). Senza voce:
5035, 5066, 5092, 5115, 513A, 5186, 51A4, 51B1, 51B5 (ingresso interno o non
raggiunto da nessuna sessione: vedi «Catture»).

**6. Catture.** Da `decomp/`. Misurate il 2026-11-03 in una copia di lavoro
con questi stessi comandi (`cattura.sh` e `snap.lua` originali): tutte
riescono, 91 file `_regs.txt` in tutto. Stati: `finale` parte dal frame 2300,
`stanza50` dal 1951.

Non forzate (26). File `/tmp/n10_nat.txt` con una cattura per riga (`ENTRY
stato N [suffisso]`), poi `xargs -P6 -L1 sh cattura.sh < /tmp/n10_nat.txt`:
```
0x4FD4 finale 1
0x4FD4 finale 2 n2
0x4FD4 stanza50 1
0x4FD4 stanza50 2 n2
0x5005 finale 1
0x5005 finale 2 n2
0x5023 finale 1
0x5023 finale 2 n2
0x5023 stanza50 1
0x5023 stanza50 2 n2
0x50BA finale 1
0x50BA finale 2 n2
0x50BA stanza50 1
0x50BA stanza50 2 n2
0x50DF finale 1
0x50DF finale 2 n2
0x50DF stanza50 1
0x50DF stanza50 2 n2
0x5124 finale 1
0x5124 finale 2 n2
0x5124 stanza50 1
0x5124 stanza50 2 n2
0x513E stanza50 1
0x514C finale 1
0x516B finale 1
0x51D3 finale 1
```
Misurato: 5005 in `stanza50`, 5092 in `finale` e `stanza50`, 5115, 513A, 5186,
51A4, 51B1, 51B5 in entrambi, e 513E (`finale`), 514C, 516B, 51D3 (`stanza50`)
**non producono cattura** (FALLITA); 513E, 514C, 516B, 51D3 hanno la sola
occorrenza 1 (la n2 fallisce). 5035, 5066 sono ingressi interni (cadute).

Forzate (65), `SNAP_FORZA` applicato all'ingresso, N=1 (sintassi di `snap.lua`:
`ix+HH:VV`, `iy+HH:VV`, registri `a b c d e h l`; valori esadecimali). Tag
`forzaNN`. Tutte riescono (misurato la cattura; il ramo aperto è dedotto dal
dasm, attesa, non misurata l'apertura). Script d'aiuto e ciclo:
```
cd decomp
cat > /tmp/go36.sh <<'EOF'
SNAP_FORZA="$4" sh cattura.sh $1 $2 1 $3
EOF
xargs -P6 -L1 sh /tmp/go36.sh < /tmp/n10_forze.txt
```
con `/tmp/n10_forze.txt` (formato `ENTRY stato tag valori`):
```
0x4FD4 finale forza00 iy+1d:00
0x4FD4 finale forza01 iy+1d:01
0x4FD4 finale forza02 iy+1d:02
0x4FD4 finale forza03 iy+1d:03
0x4FD4 finale forza04 iy+1d:04
0x4FD4 finale forza05 iy+1d:05
0x4FD4 finale forza06 iy+1d:06
0x4FD4 finale forza07 iy+1d:02,iy+0c:00,iy+1e:00
0x4FD4 finale forza08 iy+1d:02,iy+0c:00,iy+1e:01
0x4FD4 finale forza09 iy+1d:02,iy+0c:80,iy+15:02
0x4FD4 finale forza10 iy+1d:02,iy+0c:ff,iy+1e:00
0x4FD4 finale forza11 iy+1d:05,ix+08:00,ix+0b:ff,ix+0c:ff,ix+00:20
0x4FD4 finale forza12 iy+1d:05,ix+08:00,ix+0b:ff,ix+0c:ff,ix+00:00
0x5005 finale forza01 ix+00:00
0x5005 finale forza02 ix+00:04,ix+0d:02
0x5005 finale forza03 ix+00:04,ix+0d:03
0x5023 finale forza01 iy+1e:00
0x5023 finale forza02 iy+1e:02,iy+15:02
0x5023 finale forza03 iy+1e:02,iy+15:0d
0x5023 finale forza04 iy+1e:02,iy+15:05
0x50BA finale forza01 iy+13:01
0x50BA finale forza02 iy+13:00,iy+1e:00
0x50BA finale forza03 iy+13:00,iy+1e:02
0x50DF finale forza01 ix+0b:00,ix+0c:00
0x50DF finale forza02 ix+0b:ff,ix+0c:ff,ix+00:20
0x50DF finale forza03 ix+0b:ff,ix+0c:ff,ix+00:00
0x513E stanza50 forza01 c:00
0x513E stanza50 forza02 c:21
0x514C finale forza01 b:00,c:00
0x514C finale forza02 b:10,c:00
0x514C finale forza03 b:20,c:00
0x514C finale forza04 b:00,c:01
0x516B finale forza01 b:20
0x516B finale forza02 b:00,c:00
0x516B finale forza03 b:01,c:00
0x51D3 finale forza01 b:55,c:aa
0x5124 finale forza30 ix+07:02,b:10,c:00
0x5124 finale forza31 ix+07:04,b:00,c:20
```
più 27 righe per 5124 (`finale`, indice i = 0..8): `forza0i ix+07:0i`,
`forza1i ix+07:0i,b:00,c:00`, `forza2i ix+07:0i,b:30,c:ff` (esempio:
`0x5124 finale forza14 ix+07:04,b:00,c:00`). Le tre serie aprono i rami di
513E, 514C, 516B, 5186, 51A4 (B/C a zero, B/C pieni) e portano a 513A, 51B1,
51B5 (indici 0, 6, 7: non catturabili da soli) e a 51D3.

Rami aperti (dedotti): 4FD4 forza00-06 → un indice per destinazione (3 e 4 =
3B62); forza07-10 → rami di 5092 (via 5035 o B = 6/9); forza11-12 → 50DF con
5124 o 5115 (secondo il bit 5 di (IX+0) dopo 52FD). Non fare forzate con
indice fuori tabella (`iy+1d` ≥ 7 per 4FD4, `ix+07` ≥ 9 per 5124):
`nemici_rst38` registrerebbe il sito come errore. Se una cattura, nel banco,
fallisce perché entra in uno stub rimasto (2474, 4ada, 5fc8, 610c, 627f), si
scarta e si dice nel rapporto (non atteso: il tipo 6 non li raggiunge).

Totali: 26 + 65 = 91 catture, 10 voci. Per voce (non forzate + forzate): 4FD4
4+13, 5005 2+3, 5023 4+4, 50BA 4+3, 50DF 4+3, 5124 4+29, 513E 1+2, 514C 1+4,
516B 1+3, 51D3 1+1.

**7. Copertura del C** (gcov, fuori dal repository; prefisso `h-`):
```
D=/tmp/cov36 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
 && ln -s "$PWD/decomp/out" $D/out && (cd $D && gcc -std=c11 -O0 --coverage -o h \
 harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c z80_*.c \
 nemici*.c attivita.c attivita_01_10.c && ./h --tutte > /dev/null; \
 gcov h-nemici_tipo06a.gcda > /dev/null 2>&1; \
 awk -F: '$1 ~ /#####/' nemici_tipo06a.c.gcov)
```
Nel rapporto: le righe non eseguite e il motivo. Attese e note (stima, non
misurata): i casi `0` e `default` dei due `switch` (indici fuori tabella);
il ritorno anticipato di 50BA/50DF sul riporto di 531C (uscita anomala non
catturabile); alcuni rami di 5115 (dipendono da B e C prodotti da 5345, non
forzabili da fuori). Altre righe scoperte: aggiungere una forzata (stesso
schema, tag `forzaNN` successivo, stato come sopra, N=1) solo se il ramo è
raggiungibile senza passare dagli stub; altrimenti elencarle.

**8. Documenti.** `ROUTINE.md`: in fondo (dopo la r. 628), sezione
`## Motore dei nemici (lotto N10)` con un paragrafo (modulo
`nemici_tipo06a.c`; stub rimasti 2474, 4ada, 5fc8, 610c, 627f; già fatte e non
rifatte 24DF, 51D7, 5068, 51FA, 5224, 523C, 5254, 4FEF; wrapper `banco_r_4fd4`
e `banco_r_50df`; 5092, 5115, 513A, 5186, 51A4, 51B1, 51B5 senza ingresso
catturabile, coperte dalle forzate di 4FD4, 50DF e 5124; 5035 e 5066 ingressi
interni) e la tabella come quella della N9 (ingresso, funzione, file, byte,
descrizione, stato, catture OK, stati, note), una riga per routine (19).
Stato `verificata` per 4FD4, 5005, 5023, 50BA, 50DF, 5124 (2 o più non
forzate); `tradotta` per 513E, 514C, 516B, 51D3 (1 non forzata, più forzate)
e per le 9 senza voce. Poi l'elenco delle forzate e le righe gcov non
coperte. `CENSIMENTO-NEMICI.md`: dopo il paragrafo «Lotto N9 fatto» un
paragrafo «**Lotto N10 fatto** (brief 36)» con le 19 routine, lo stub di
`nemici_stub.c` da 6 a 5, 10 voci nuove nel banco, 91 catture, e la frase che
con N10 il tipo 6 è completo. Le righe della tabella dei moduli non cambiano.

## Verifiche obbligatorie

Tutte da `/home/rufio72/OpenSolomonsKey-master`; uscita attesa esatta salvo
dove è scritto «attesa, dedotta». Abbreviazione:
`V="4FD4 5005 5023 50BA 50DF 5124 513E 514C 516B 51D3"`.

1. Disassemblato: `wc -l < decomp/out/nemici_n10.dasm; grep -c 'ld a,r' decomp/out/nemici_n10.dasm`
   → `248` e `0` (misurate).
2. Compilazione: `cd decomp/harness && make harness replay sched_proto > /tmp/m36.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/m36.txt`
   → `make 0` e `0`.
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h36.txt; echo "exit $?"; grep -c FALLITO /tmp/h36.txt; grep -c '^RIEPILOGO' /tmp/h36.txt; grep -c '^cattura' /tmp/h36.txt`
   → `exit 0`, `0`, `250`, `977` (240 + 10 voci; 886 + 91; attese, dedotte).
4. Catture per voce: `for a in $V; do grep "^RIEPILOGO 0x$a:" /tmp/h36.txt; done`
   → (attese, dedotte dai file elencati sopra)
   ```
   RIEPILOGO 0x4FD4: 17 catture, 17 OK
   RIEPILOGO 0x5005: 5 catture, 5 OK
   RIEPILOGO 0x5023: 8 catture, 8 OK
   RIEPILOGO 0x50BA: 7 catture, 7 OK
   RIEPILOGO 0x50DF: 7 catture, 7 OK
   RIEPILOGO 0x5124: 33 catture, 33 OK
   RIEPILOGO 0x513E: 3 catture, 3 OK
   RIEPILOGO 0x514C: 5 catture, 5 OK
   RIEPILOGO 0x516B: 4 catture, 4 OK
   RIEPILOGO 0x51D3: 2 catture, 2 OK
   ```
5. Non forzate per voce: `cd decomp && for a in $V; do ls out/snap_0x${a}_*_regs.txt | grep -vc forza; done | tr '\n' ' '`
   → `4 2 4 4 4 4 1 1 1 1 ` (misurate nella copia di lavoro).
6. Totale file: `cd decomp && for a in $V; do ls out/snap_0x${a}_*_regs.txt; done | wc -l`
   → `91` (misurata).
7. Voci precedenti invariate: `for a in 24DF 4FEF 5068 51D7 51FA 5224 5274 52FD 531C 5345 54C5 4767 6C2C; do grep "^RIEPILOGO 0x$a:" /tmp/h36.txt; done`
   → (misurate prima della modifica) `24DF`, `4FEF`, `5068`, `51D7`, `5274`,
   `52FD`, `5345`, `54C5`, `6C2C`: `2 catture, 2 OK`; `51FA`: `30 catture, 30 OK`;
   `5224`: `3 catture, 3 OK`; `531C`: `4 catture, 4 OK`; `4767`: `22 catture, 22 OK`.
8. `sched_proto`: `cd decomp && harness/sched_proto out/sched_log.txt | tail -1`
   → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
9. Replay: `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r36.txt; echo "exit $?"; tail -4 /tmp/r36.txt`
   → `exit 0` e (misurate):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
10. Stub: `grep -c STUB_NEMICI decomp/harness/nemici_stub.c; grep -c -E '4fd4' decomp/harness/nemici_stub.c`
    → `6`, `0` (attese, dedotte: oggi 7; si toglie 1).
11. Copertura: il comando del punto 7 di «Struttura». Uscita attesa (attesa,
    non misurata): ogni riga `#####` spiegata nel rapporto.
12. Dimensioni: `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/nemici*.c | awk '$1>700 && $2!="totale" && $2!="total"'` → vuoto.
13. File vietati intatti: `git diff --name-only HEAD | grep -E 'cattura\.sh|snap\.lua|sessioni|replay\.|sched|rst20|rst38_tables|censimento_nemici\.py|STATO\.md|README\.md|compiti/'` → vuoto.
14. Ambito: `git status --short | grep -v -E '^( M|\?\?|A ) (compiti/36-|decomp/(harness/(nemici_tipo06a\.c|nemici_r\.c|nemici\.h|nemici_stub\.c|routine_tab\.c)|ROUTINE\.md|CENSIMENTO-NEMICI\.md|out/(snap_0x(4FD4|5005|5023|50BA|50DF|5124|513E|514C|516B|51D3)_|nemici_n10\.dasm)))'`
    → vuoto; `git status --short | grep -c -E '\.(gcda|gcno)$'` → `0`.
15. Documenti: `grep -c '^## Motore dei nemici (lotto N10)$' decomp/ROUTINE.md; grep -c 'Lotto N10 fatto' decomp/CENSIMENTO-NEMICI.md`
    → `1` e `1`.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto N10 del motore dei nemici: tipo 06 prima metà (4FD4, 5005-51D3), catture nel banco`
  e riga finale `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`
  (se il lotto passa a Opus per escalation, `Claude Opus 5.5`). Il commit
  comprende `decomp/out/nemici_n10.dasm` (`git add -f`) e le catture nuove
  (`prima.bin`, `dopo.bin`, `regs.txt`, `.log`; con `git add -f`).
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-15; l'elenco
  delle voci con numero di catture e stato; le righe gcov non coperte; le
  deviazioni dal brief (cattura che fallisce o scartata, cambio di N o di
  stato).

## Domande aperte

1. **Voci con una sola cattura non forzata** (513E, 514C, 516B, 51D3): la
   seconda occorrenza non esiste in nessuno stato provato. Proposta: entrano
   nel banco con 1 non forzata più le forzate e stato `tradotta`
   (`verificata` richiede 2 non forzate). — «preferenza»
2. **Nove routine senza ingresso catturabile** (5035, 5066 interni; 5092,
   5115, 513A, 5186, 51A4, 51B1, 51B5 mai raggiunte da MAME). Proposta:
   `tradotta`, coperte dalle forzate dei genitori (4FD4, 50DF, 5124). —
   «preferenza»
3. **Wrapper R**: `banco_r_4fd4` e `banco_r_50df` in `nemici_r.c`, regola già
   ratificata (niente R nelle catture). — «preferenza»
4. **Forzate con indice fuori tabella** (4FD4 `iy+1d` ≥ 7, 5124 `ix+07` ≥ 9)
   non fatte; i casi `0` e `default` degli `switch` restano scoperti in gcov.
   — «preferenza»
5. **Uscita anomala di 531C in 50BA e 50DF** (cella piena, `pop hl; jp 5068`):
   non catturabile (`snap.lua` aspetta il ritorno all'indirizzo in cima alla
   pila), il ritorno anticipato resta scoperto in gcov e si elenca in
   `ROUTINE.md`. — «preferenza»
6. **`STATO.md` e `README.md`**: li aggiorna la chat principale dopo la
   verifica (banco a 977 catture e 250 voci, tipo 6 completo). — «preferenza»

## Risoluzioni delle domande aperte (chat principale, 2026-11-03)

Tutte le domande sono preferenze, accolte con la proposta dell'analista:
1. Sì: le quattro voci con una sola cattura non forzata (513E, 514C, 516B, 51D3) entrano nel banco con stato `tradotta`.
2. Sì: le nove routine senza ingresso catturabile (5035, 5066, 5092, 5115, 513A, 5186, 51A4, 51B1, 51B5) si segnano `tradotta`, coperte dalle forzate di 4FD4, 50DF e 5124.
3. Sì: wrapper `banco_r_4fd4` e `banco_r_50df` in `nemici_r.c`, come da regola ratificata (niente R nelle catture).
4. Sì: niente forzate con indice fuori tabella (`iy+1d` ≥ 7 per 4FD4, `ix+07` ≥ 9 per 5124); le scoperture si annotano.
5. Sì: l'uscita anomala di 531C dentro 50BA e 50DF resta scoperta in gcov e si elenca in `ROUTINE.md`.
6. Sì: STATO.md e README.md li aggiorna la chat principale a verifica conclusa.

## Esito (chat principale, 2026-11-03)

Passato al primo colpo, ottavo ciclo con lo schema economico (analista
Sonnet ~113k token, sviluppatore Sonnet ~67k, verifica a script senza
modello). Le 19 routine restanti del tipo 06a sono in `nemici_tipo06a.c`
(501 righe), stub 4FD4 tolto (restano 5: 2474, 4ADA, 5FC8, 610C, 627F),
banco a 250 voci e 977 catture con 0 fallite, replay fino al frame 5499
senza divergenze nello spedidore, `sched_proto` invariato
(106124 spedizioni, 0 differenze). Nessuna deviazione dal brief.
Scoperture gcov annotate in `ROUTINE.md`: casi `0` e `default` dei tre
switch (indici fuori tabella non forzati), ramo BC=0081 di 5115 (B e C
vengono da 5345), ritorno anticipato sul riporto di 531C in 50DF.
