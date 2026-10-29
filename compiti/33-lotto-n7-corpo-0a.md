# Compito 33: lotto N7 del motore dei nemici, corpo 0A (4649, 46A3, 470A, 4717, 4767)

Data: 2026-10-29. Brief scritto dall'agente analista per un agente
sviluppatore. Questo file è tutto il contesto che serve: non c'è una chat
da leggere.

## Contesto: file da leggere

Da leggere (e basta):
1. `decomp/out/nemici_n7.dasm`, da generare (vedi sotto): 334 righe di
   disassemblato, le 5 routine del lotto (contiene anche la tabella dati
   47C7-47E4, disassemblata come istruzioni: sono 30 byte di dati).
2. `decomp/harness/nemici.h` (260 righe): r. 1-20 (riporto), r. 217-245
   (sezione N6 e stub).
3. Le convenzioni qui sotto («Convenzioni di traduzione»).

Da aprire perché si modificano, solo per la parte indicata:
`decomp/harness/nemici.c` (205 righe: r. 1-35 intestazione e `#define`,
r. 128-158 `salta_4644` e `sub_4632`, che non cambiano), `nemici_stub.c`
(26 righe: tutto), `routine_tab.c` (solo r. 100-112), `routine.h` (solo i
prototipi di `sub_0008`, `sub_0010`, `sub_0018`, `sub_3569`, `sub_4722`,
`sub_472e`, `sub_4af6`, `sub_7379`), `ram.h` (macro `M`, `BC`, `SET_HL`,
`leggi`), `decomp/ROUTINE.md` (solo dalla r. 468 alla fine: formato della
sezione N6), `decomp/CENSIMENTO-NEMICI.md` (solo il paragrafo «Lotto N6
fatto», r. 429-436).

È vietato leggere `STATO.md`, `README.md`, `ROUTINE.md` per intero, gli
altri brief in `compiti/` e i moduli `nemici_*.c` diversi da `nemici.c`.

Generazione del disassemblato (da `/home/rufio72/OpenSolomonsKey-master`,
`awk` è mawk: il confronto è fra stringhe esadecimali minuscole):
```
cd decomp && awk 'BEGIN{n=split("4649 46a3 470a 4717 4767",lo," "); split("46a2 4709 4716 4721 488c",hi," ")}
/^[A-Za-z_0-9]+:/ {lab=$0; next}
/;[0-9a-f][0-9a-f][0-9a-f][0-9a-f][ \t]*$/ {a=substr($0,length($0)-3); ok=0; for(i=1;i<=n;i++) if(a>=lo[i] && a<=hi[i]) ok=1; if(ok){ if(lab!="") print lab; print} lab=""; next}
{lab=""}' out/main64.dasm > out/nemici_n7.dasm; cd ..
```
Uscita attesa (misurata): `wc -l < decomp/out/nemici_n7.dasm` → `334`;
`grep -c 'ld a,r' decomp/out/nemici_n7.dasm` → `0`. Il file è ignorato da
git (`decomp/out/*`): si aggiunge al commit con `git add -f`.

## Obiettivo

Tradurre in C le 5 routine rimaste del lotto N7 (4649, 46A3, 470A, 4717,
4767: 481 byte; 3B62, la sesta, è già `sub_3b62` in `nemici.c` dal lotto N1),
in coda a `harness/nemici.c`, togliere gli stub di 4649 e 46A3, e portare le
5 routine nel banco con catture MAME. Replay e `sched_proto` non devono
cambiare (misurato: il replay non raggiunge 4649 e 46A3: finisce al frame
5499 con exit 0 con gli stub che registrerebbero l'ingresso).

Vincoli assoluti:
- Si modifica solo dentro `/home/rufio72/OpenSolomonsKey-master`. Non si
  toccano `cattura.sh`, `snap.lua`, `sessioni.sh`, `replay.*`, `sched*.c`,
  `rst20*`, `rst38_tables.h`, `censimento_nemici.py`, `STATO.md`,
  `README.md` né le catture esistenti in `decomp/out/`.
- Niente installazioni. MAME gira solo come fa `cattura.sh` (senza finestra,
  senza audio, provider a `none`): niente partite a mano, niente giri su
  tutte le stanze. Una cattura dura 3-25 secondi (si possono lanciare fino a
  6 in parallelo con `xargs -P6`, file di uscita distinti; attenzione: `xargs`
  toglie le virgolette, per i comandi con `SNAP_COND` usare un ciclo `for`).
- Commenti, messaggi e documenti in italiano. Nessun file di codice sopra le
  800 righe e nessun `nemici*.c` sopra le 700.
- Niente git distruttivo (`stash`, `reset --hard`, `filter-branch`); si
  ripristina un file solo con `git checkout -- <file>`.
- Un solo agente alla volta su `decomp/harness`.

## Cosa esiste già (fatti verificati)

Misure del 2026-10-29 sui binari già compilati (da `decomp/harness`):
- `./harness --tutte`: exit 0, 671 righe `^cattura`, 0 `FALLITO`, 209 righe
  `^RIEPILOGO` (`RIEPILOGO 0x4E87`, `0x4AF6`, `0x472E`, `0x3569`, `0x7379`:
  `2 catture, 2 OK` ciascuna).
- Replay: `exit 0`, fino al frame 5499; `sched_proto`: `106124, 0, 131`
  (righe complete al punto 8-9 delle verifiche).
- `grep -c STUB_NEMICI nemici_stub.c` → 17 (16 stub più la riga `#define`);
  due sono di questo lotto: `STUB_NEMICI(4649)` e `STUB_NEMICI(46a3)`.
- Nessun `ld a,r` nelle 5 routine né nelle routine che chiamano (4AF6, 4722,
  472E, 7379, 3569, rst 8/10/18): niente wrapper `banco_r_*`.
- `nemici.c`: 205 righe; il censimento stima 0,6-0,68 righe per byte: 481 B
  → 289-327 righe nuove → `nemici.c` finale 494-532 righe, sotto le 700:
  **nessuna spezzatura**, nessun modulo nuovo (il Makefile prende `nemici*.c`
  con un `wildcard`).
- gcov: `nemici.c` è coperto dal banco solo al 5,08 % (118 righe): il resto è
  esercitato dal replay. Il criterio di copertura di questo lotto è quindi
  sulle sole righe nuove (punto 11 delle verifiche).

Chiamate (da `out/censimento_nemici.txt` e dal dasm):
- `salta_4644` (`nemici.c` r. 131-138) chiama già `sub_4649(z)` e
  `sub_46a3(z)`: dichiarate in `nemici.h` r. 244-245 (sezione «Stub»), da
  spostare nella sezione nuova. Non si cambia `sub_4632`.
- 4767 ha un secondo chiamante a 37D2 (dentro 37B4, **non tradotta e fuori
  dal lotto**: dopo la `call` c'è `pop de; ret` a 37D5). Le catture di 4767
  fatte da lì ritornano a 37D5. L'etichetta 480A (dentro 4767, in coda al
  ramo 47E5) compare in `call z,sub_480ah ;a059`, ma a A059 ci sono dati: non
  è una routine a sé; non va tradotta separatamente.

| ingresso | byte | chiamanti | cosa fa |
|---|---:|---|---|
| 4649 | 90 | 4632 (via `rst 20` a 4644, A = c459 & 7 = 1, C = c459) | se (c2a9) & 3F < 36: `ret c` con A = (c2a9) & 3F; altrimenti per i bit 7 e 6 di C (HL = c45b, +2 per giro, B = 2) accesi: prende da (HL), (HL+1) indice e tipo, azzera 3 byte del record 488D, prende l'oggetto 48AF (IX), `sub_4af6` (A=A0, BC=0500), `sub_4722`, (IX+3), `sub_4767`, set 6 di (IX+0); in uscita (c459) = 20 |
| 46A3 | 103 | 4632 (A = 0) | per bit 7 e poi bit 6 di (c459): `sub_7379` (slot libero; senza slot: c459 = 20 oppure bit 6 spento), riempie c45b/c45d dalla lista a (c2b1)/(c2b3) (se il byte è FF: `sub_470a`), `sub_472e`, `sub_4717`; poi (c459)++ |
| 470A | 13 | 46A3 | lista terminata da FF: HL = HL − (byte successivo) (aritmetica a 16 bit con neg/adc), A = (HL); non scrive RAM |
| 4717 | 11 | 46A3 | `sub_4af6` con A=E5, E=03, BC=0 |
| 4767 | 264 | 4649, 37B4 (non tradotta) | A = codice (c45c), C = indice del record: `switch` per intervalli di A (vedi sotto) sulle tabelle 488D/48AF; il ramo 47B0 copia 10 byte dalla tabella dati 47C7 con `sub_3569` (A=4) |

Le descrizioni sono un riassunto: fa fede il dasm. Già tradotte e da usare
(prototipi in `routine.h`): `sub_0008` (DE = A, HL += 2·A), `sub_0010` (DE =
parola a HL, HL += 2), `sub_0018` (come rst 10 ma HL = parola, DE = HL
d'ingresso + 2), `sub_3569` (A = indice del record di 3574; sorgente HL, anche
ROM), `sub_4722` (C = A = 7 + soglie di 9484 ≤ A), `sub_472e` (IX = DE =
oggetto 48AF[C], HL invariato, (IX+0E)/(IX+15) = H/L d'ingresso), `sub_4af6`
(A, BC, E di ingresso; inizializza IX), `sub_7379` (riporto 1 = slot libero:
HL = slot, A = indice; il riporto sta nel bit 0 di `z->f`).

Ramificazioni di 4767 (dal dasm; A in ingresso → etichetta): <10 → 4798;
10-15 → 47B0; 16-1B → 47E5; 1C-1F → 481D; 20-3F → 486B (il bit 3 di A decide
`set 6`); 40-4F → 4813; 50-67 → 481D; 68-7F → 4832; 80-9F → 483F; A0-A3 →
4859; ≥ A4 → 4798. 47E5 cade in `sub_480a` (azzera (+1D) e (+1E) dell'oggetto
HL+1D): in C una funzione `static` o codice in comune, a scelta.

### Convenzioni di traduzione

- Ogni routine è `void sub_xxxx(struct z80 *z)`, tutte non statiche (hanno una
  voce di `routine_tab.c`). Registri `z->a b c d e h l`, `z->ix`, `z->iy`,
  `z->f`. Memoria con `M(indirizzo)`; ROM con `leggi(indirizzo)` (ram.h; la
  tabella 47C7 è ROM); coppie con `BC(z) DE(z) HL(z)` e `SET_BC SET_DE
  SET_HL`. `nemici.c` non ha macro `IXB/IYB`: definirle in testa se servono
  (come in `nemici_tipo06b.c`: `#define IXB(o) M(z->ix + (o))`).
- Il banco confronta RAM (c000-e5ff, salvo 0x40 byte sotto SP) e A, BC, DE,
  HL, IX, IY all'uscita, non F: anche i registri «sporchi» devono coincidere
  (C, B, DE, HL e A in particolare; per `ret c` di 4649 A = (c2a9) & 3F).
- `ret c` di 4649: nel C `return` subito (nessun chiamante legge il riporto).
- I `rst 8/10/18` si traducono con `sub_0008/0010/0018(z)` (come in
  `nemici_tipo09.c`); le tabelle 488D e 48AF sono in ROM.
- Catture `forza*` valgono come catture OK ma non contano per lo stato
  `verificata`, che richiede almeno 2 catture non forzate. Una cattura con
  `SNAP_COND` (filtro dell'occorrenza, nessuna modifica di stato) è non
  forzata (precedente: 4E87 in `ROUTINE.md`).

## Struttura da realizzare

**1. `nemici.c`** (in coda, dopo l'ultima funzione, così le righe nuove sono
tutte oltre la 205): `sub_470a`, `sub_4717`, `sub_4767`, `sub_46a3`,
`sub_4649` (ordine libero: le chiamate verso funzioni dichiarate in
`nemici.h` non richiedono prototipi). Aggiornare il commento di intestazione
(lotto N7, ingressi 4649/46A3 e 4767).

**2. `nemici_stub.c`**: togliere `STUB_NEMICI(4649)` e `STUB_NEMICI(46a3)`.

**3. `nemici.h`**: togliere i prototipi `sub_4649`/`sub_46a3` dalla sezione
«Stub» e aggiungere, dopo la sezione N6, «Lotto N7 (brief 33)» con i prototipi
di `sub_4649`, `sub_46a3`, `sub_470a`, `sub_4717`, `sub_4767`.

**4. `routine_tab.c`**: voci `{ "0xXXXX", sub_xxxx, 0, 0 }` in ordine di
indirizzo: 0x4649, 0x46A3, 0x470A, 0x4717 prima di 0x4722; 0x4767 fra 0x472E
e 0x4AF6.

**5. Catture.** Da `decomp/`. Misurate il 2026-10-29 in una copia di lavoro
con questi stessi comandi: tutte riescono (35 `_regs.txt`). Il «dopo» è a
PC=3AF3 (ritorno del corpo) per 4649 e 46A3, 46C5 per 470A, 46D3/4705 per
4717, 4693 per 4767 chiamata da 4649 e 37D5 se chiamata da 37D2.
```
cd decomp
for s in porta stanza06 stanza07; do SNAP_COND='((b@c2a9)&3f)>=36' sh cattura.sh 0x4649 $s 1 c; done
sh cattura.sh 0x4649 porta 1
for s in porta stanza06 stanza07; do sh cattura.sh 0x46A3 $s 1; done
for s in porta stanza13 stanza14; do sh cattura.sh 0x470A $s 1; done
for s in porta stanza06 stanza07; do sh cattura.sh 0x4717 $s 1; done
for s in porta stanza02 stanza07 finale; do sh cattura.sh 0x4767 $s 1; done
for v in 10 11 12 13 16 19 1c 20 24 28 2c 40 50 54 68 80 a0 a4; do
  SNAP_FORZA="a:$v" sh cattura.sh 0x4767 porta 1 forza$v
done
```
Le 3 catture di 4649 con tag `_c` (filtro `(c2a9)&3F >= 36`) percorrono il
ciclo (`porta_c` e `stanza06_c`: bit 7 e 6 di C, 29-30 byte cambiati;
`stanza07_c`: solo il bit 6, c459 = 61, 20 byte); `4649 porta` senza filtro è
l'uscita `ret c` (0 byte cambiati, A = 01). Le 3 di 46A3 hanno bit 7 e 6 di
c459 (`porta`, `stanza06`: lista già a FF, quindi toccano anche 470A) e solo
il bit 6 (`stanza07`). Le 4 non forzate di 4767: A = 0D (`finale`, da 37D2,
ramo 4798), 48 (`porta`, 4813), 34 (`stanza02`, da 37D2, 486B senza bit 3),
4E (`stanza07`, 4813).

Forzature di 4767 **misurate** (stato `porta`, N=1: C = 10, B = 05 d'ingresso;
l'effetto è l'uscita letta dal confronto prima/dopo):

| SNAP_FORZA | ramo aperto | dopo: AF, BC, DE, HL |
|---|---|---|
| `a:10` / `a:11` | 47B0, record 0 / 1 della tabella 47C7 | 0048 0000 C976 47CF / 0048 0000 C976 47D9 |
| `a:12` / `a:13` | 47B0, record 2 / 3 (il 3° è fuori tabella, legge 47E5-47EE) | 0068 0000 C976 47E3 / 0068 0000 C976 47ED |
| `a:16` / `a:19` | 47E5, con (E−1) bit 2 acceso / spento | 0044 0010 001D C81C (entrambe) |
| `a:1c` | 481D (bit 2 di A) | 0100 1C10 C7FE C81C |
| `a:20` / `a:24` | 486B, bit 3 spento | 0044 2010 001D C81C / 0100 2410 001D C81C |
| `a:28` / `a:2c` | 486B, bit 3 acceso (`set 6`) | 0200 2810 001D C81C / 0304 2C10 001D C81C |
| `a:40` | 4813 | 0044 4010 C7FE C81C |
| `a:50` / `a:54` | 481D, bit 2 spento / acceso | 0044 5010 C7FE C81C / 0100 5410 C7FE C81C |
| `a:68` | 4832 | 108C 0510 C7FE C81B |
| `a:80` | 483F | 1088 8010 001D C81B |
| `a:a0` | 4859 | 1088 0510 001D C81C |
| `a:a4` | 4798 (≥ A4) | 0148 0510 000C C81B |

Restano **non esercitabili**: i due rami di 46A3 con `sub_7379` senza slot
libero (46AE-46B3 e 46DF-46E4; il ramo «esauriti» di 7379 non è mai stato
visto nelle tracce e servirebbe occupare tutti i 17 slot).

**6. Copertura del C** (gcov, fuori dal repository; prefisso `h-`; `$INI` =
numero della prima riga nuova di `nemici.c`, cioè la prima dopo l'ultima
funzione di oggi, 206 se si scrive subito dopo l'ultima riga):
```
D=/tmp/cov33 && rm -rf $D && mkdir $D && cp decomp/harness/*.c decomp/harness/*.h $D/ \
 && ln -s "$PWD/decomp/out" $D/out && (cd $D && gcc -std=c11 -O0 --coverage -o h \
 harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c z80_*.c \
 nemici*.c attivita.c attivita_01_10.c && ./h --tutte > /dev/null; \
 gcov h-nemici.gcda > /dev/null 2>&1; \
 awk -F: -v ini=$INI '$2+0>=ini && $1 ~ /#####/' nemici.c.gcov)
```
Nel rapporto: le righe non eseguite (quelle attese sono i due rami di 46A3
senza slot, 5-7 righe) e il motivo.

**7. Documenti.** `ROUTINE.md`: in fondo (dopo la r. 506), sezione
`## Motore dei nemici (lotto N7)` con un paragrafo (modulo `nemici.c`, 4767
condivisa con 37B4 non tradotta, rami non esercitabili) e la tabella come
quella della N6 (ingresso, funzione, file, byte, descrizione, stato, catture
OK, stati, note), una riga per routine (5), più l'elenco delle forzate di
4767. Stato `verificata` per tutte e 5 (almeno 2 non forzate: 4, 3, 3, 3, 4).
`CENSIMENTO-NEMICI.md`: dopo il paragrafo «Lotto N6 fatto» un paragrafo
«**Lotto N7 fatto** (brief 33)» con le 5 routine, 3B62 già fatta in N1,
gli stub di `nemici_stub.c` scesi da 16 a 14 (tolti 4649 e 46A3). Le righe della tabella dei
moduli non cambiano.

## Verifiche obbligatorie

Tutte da `/home/rufio72/OpenSolomonsKey-master`; uscita attesa esatta salvo
dove è scritto `>=` o «attesa, non misurata». Abbreviazione:
`V="4649 46A3 470A 4717 4767"`.

1. Disassemblato: `wc -l < decomp/out/nemici_n7.dasm; grep -c 'ld a,r' decomp/out/nemici_n7.dasm`
   → `334` e `0` (misurate).
2. Compilazione: `cd decomp/harness && make harness replay sched_proto > /tmp/m33.txt 2>&1; echo "make $?"; grep -c -E 'warning|error' /tmp/m33.txt`
   → `make 0` e `0`.
3. Banco: `cd decomp/harness && ./harness --tutte > /tmp/h33.txt; echo "exit $?"; grep -c FALLITO /tmp/h33.txt; grep -c '^RIEPILOGO' /tmp/h33.txt`
   → `exit 0`, `0`, `214` (209 + 5 voci nuove; attesa, dedotta).
4. Catture per voce: `for a in $V; do grep "^RIEPILOGO 0x$a:" /tmp/h33.txt; done`
   → esattamente (attesa, dedotta dal numero di catture elencate):
   ```
   RIEPILOGO 0x4649: 4 catture, 4 OK
   RIEPILOGO 0x46A3: 3 catture, 3 OK
   RIEPILOGO 0x470A: 3 catture, 3 OK
   RIEPILOGO 0x4717: 3 catture, 3 OK
   RIEPILOGO 0x4767: 22 catture, 22 OK
   ```
5. Catture non forzate per voce: `cd decomp && for a in $V; do ls out/snap_0x${a}_*_regs.txt | grep -vc forza; done | tr '\n' ' '`
   → `4 3 3 3 4 `.
6. Totale catture: `grep -c '^cattura' /tmp/h33.txt` → `706` (671 + 35; attesa, dedotta).
7. Voci precedenti invariate: `for a in 4E87 4AF6 472E 3569 7379; do grep "^RIEPILOGO 0x$a:" /tmp/h33.txt; done`
   → cinque righe `RIEPILOGO 0x…: 2 catture, 2 OK` (misurate prima della modifica).
8. `sched_proto`: `cd decomp && harness/sched_proto out/sched_log.txt | tail -1`
   → `spedizioni confrontate: 106124, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
9. Replay: `cd decomp && harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 > /tmp/r33.txt; echo "exit $?"; tail -4 /tmp/r33.txt`
   → `exit 0` e (misurate):
   ```
   spedizioni eseguite: 64686, fino al frame 5499
   NMI confrontate: 3090 (nello spedidore 1486, a metà attività 1604); divergenti: nello spedidore 0, a metà attività 885
   prima divergenza a metà attività (attesa): frame 2415
   nessuna divergenza nello spedidore
   ```
10. Stub: `grep -c STUB_NEMICI decomp/harness/nemici_stub.c; grep -c -E '4649|46a3' decomp/harness/nemici_stub.c`
    → `15` e `0` (attesa, dedotta: 17 − 2).
11. Copertura: il comando del punto 6 di «Struttura». Uscita attesa (attesa,
    non misurata): al più 8 righe `#####` (i due rami senza slot di 46A3).
    Controllo: `… | wc -l` → numero ≤ 8.
12. Dimensioni: `find decomp \( -name '*.c' -o -name '*.h' -o -name '*.lua' -o -name '*.py' -o -name '*.sh' \) -not -path '*/out/*' | xargs wc -l | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto; `wc -l decomp/harness/nemici*.c | awk '$1>700 && $2!="totale" && $2!="total"'` → vuoto.
13. File vietati intatti: `git diff --name-only HEAD | grep -E 'cattura\.sh|snap\.lua|sessioni|replay\.|sched|rst20|rst38_tables|censimento_nemici\.py|STATO\.md|README\.md'` → vuoto.
14. Ambito: `git status --short | grep -v -E '^( M|\?\?|A ) (compiti/33-|decomp/(harness/(nemici\.c|nemici\.h|nemici_stub\.c|routine_tab\.c)|ROUTINE\.md|CENSIMENTO-NEMICI\.md|out/(snap_0x(4649|46A3|470A|4717|4767)_|nemici_n7\.dasm)))'`
    → vuoto; `git status --short | grep -c -E '\.(gcda|gcno)$'` → `0`.
15. Documenti: `grep -c '^## Motore dei nemici (lotto N7)$' decomp/ROUTINE.md; grep -c 'Lotto N7 fatto' decomp/CENSIMENTO-NEMICI.md`
    → `1` e `1`.

## Consegna

- Un commit su `main`, senza push, con messaggio
  `Lotto N7 del motore dei nemici: corpo 0A (4649, 46A3, 470A, 4717, 4767), catture nel banco`
  e riga finale `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`
  (se il lotto passa a Opus per escalation, `Claude Opus 5.5`). Il commit
  comprende il disassemblato `decomp/out/nemici_n7.dasm` (`git add -f`) e le
  catture nuove (`prima.bin`, `dopo.bin`, `regs.txt`, `.log`; con `git add -f`
  per sicurezza).
- Rapporto finale di massimo 30 righe: uscita delle verifiche 1-15; l'elenco
  delle voci con numero di catture e stato; le forzature della tabella che
  non hanno aperto il ramo voluto; le righe gcov non coperte; le deviazioni
  dal brief.

## Domande aperte

1. **Posizione del codice**: in coda a `nemici.c` (come da censimento; 494-532
   righe finali, sotto le 700, nessuna spezzatura). Proposta: sì. — «preferenza»
2. **Copertura gcov solo sulle righe nuove** (il file è coperto al 5 % dal
   banco perché il resto lo esercita il replay). Proposta: criterio sul
   numero di righe `#####` oltre la riga iniziale, al più 8. — «preferenza»
3. **4767 con secondo chiamante in 37B4 (non tradotta, fuori lotto)**:
   si traduce 4767 qui con prototipo in `nemici.h`; 37B4 resta da fare in un
   lotto futuro (non è nel censimento dei nemici). Proposta: come scritto. —
   «preferenza»
4. **Rami di 46A3 senza slot libero non esercitabili** (7379 senza slot).
   Proposta: tradotti, scoperti da gcov, elencati nel rapporto. — «preferenza»
5. **Cattura con `SNAP_COND` conta come non forzata** (3 catture di 4649
   senza le quali la routine avrebbe solo il `ret c`). Proposta: sì, come il
   precedente di 4E87. — «preferenza»
6. **`STATO.md` e `README.md`**: lo sviluppatore non li legge né li tocca.
   Proposta: li aggiorna la chat principale dopo la verifica. — «preferenza»

## Risoluzioni (chat principale, 2026-10-29)

Tutte le domande sono «preferenza»: accolte tutte con la proposta
dell'analista.
1. Codice in coda a `nemici.c`, senza modulo nuovo: sì.
2. Criterio gcov solo sulle righe nuove: sì.
3. 4767 tradotta qui, 37B4 a un lotto futuro: sì.
4. Rami di 46A3 senza slot libero tradotti ma scoperti, elencati nel
   rapporto: sì.
5. Catture `SNAP_COND` valide come non forzate (precedente 4E87): sì.
6. `STATO.md` e `README.md` li aggiorna la chat principale: sì.
