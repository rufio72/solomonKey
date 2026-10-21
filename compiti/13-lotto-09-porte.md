# Compito 13: lotto 9 — porte come funzioni di piattaforma (proposta A)

Data: 2026-10-21. Scritto dall'agente analista per un agente sviluppatore;
questo file è tutto il contesto che serve. Schema, convenzioni, vincoli e
strumenti: brief 05 («Schema dei lotti»), 06-12. Le decisioni del brief 12
(sezione «Decisioni») sono definitive: proposta A adottata, B scartata,
C rinviata a NMI/audio, e603 al PC 4CF0 rinviato alle zone mai eseguite.

Vincoli assoluti (come i lotti 1-8): MAME `/usr/games/mame` 0.264 con
`PATH=$PATH:/usr/games`, solo `-video none -sound none`, corse in sottofondo,
max 4 in parallelo; nessuna operazione git; non toccare `roms/` né
`.gitignore`; nessun file sopra le 800 righe; commenti e documenti in
italiano; nomi `sub_xxxx`. Non toccare `sessioni.sh`, `trace.lua`,
`sessione.lua`, `sched*.c/h`, `fondale.c`, `routine.c`, `build_rom.sh`,
`disasm.sh` né le traduzioni dei lotti 1-7. **Eccezione a «non toccare
`porte.c`» dei brief precedenti: questo lotto aggiunge due funzioni a
`decomp/harness/porte.c`** (sotto); `leggi_e603` resta com'è.

## Cosa esiste già (misurato oggi, 2026-10-21, dopo il lotto 8)

VM nuova: dati derivati rigenerati oggi con gli script del repo (ROM
estratte in `roms/ext/solomon/`, md5 di `main48.bin`/`main64.bin`
`daaf50c3…`/`0bca296a…`; z80dasm 1.1.6; `main48.dasm` 35732 righe; traccia
di controllo 1900 frame; sessioni `titolo`/`stanza_00`/`gameover`;
`sched_log.txt` 5500 frame). Banco: 74 routine, `--tutte` → 74 RIEPILOGO,
0 FALLITO; `sched_proto` → `spedizioni confrontate: 106095, differenze: 0;
posta_attivita: 131, slot coincidenti: 131`; ROUTINE.md 74 righe (71
`verificata`, 3 `tradotta`); 147 catture `regs`. Censimento porte conforme
al brief 12: 31 accessi diretti, 45 righe, 11 `in`/`out` letterali mai
eseguiti; `out/<sessione>_e603.txt` = solo `0161 00 <n>`; DSW1 = 02 negli
`snap_*.bin` (offset 0x2604).

## Correzione al censimento del brief 12 (verificata oggi sul dasm)

Le «routine contenitrici» del brief 12 erano gli intervalli del
`callgraph.csv`, che sovrastima l'estensione fino all'ingresso successivo.
La lettura del dasm mostra che **molti siti porta stanno in corpi di fase
dello scheduler** (bersagli delle tabelle `rst 20` 2C77/2CD2, chiusi da
`pop hl`/`ret` o da `rst 20`), non in routine chiamabili: 2EC6 (e605) sta
nel corpo 2E9C, 35F6 (e605) nel corpo 358F, 38D0/38DC (e604 R+W) nel corpo
3861, 7CB0/7CE4 (e604 W, e605 R) nel corpo 7C98, 760C (e604 W) nel blocco
760B (che manipola la pila a 7616). Questi corpi appartengono al filone
strutturale (come già l'avvio 2833-295B con 28F1/294C e la NMI 0066): le
porte vi diventeranno traducibili con questo stesso `porte.c`, ma i corpi
non sono registrabili nel banco e **restano fuori dal lotto**. Le routine
chiamabili dei grappoli, lette per estensione reale, sono quelle sotto.

## Livello di piattaforma: `porte.c` (proposta A, decisione del brief 12)

In `decomp/harness/porte.c` aggiungere (prototipi in `ram.h`, accanto a
`leggi_e603`; `ram.h` è la scelta coerente: è l'accesso alla memoria —
«preferenza» risolta così):

```c
/* Porte mappate in memoria (solomon.cpp, MAME 0.264): e600 P1 / maschera
 * NMI; e601 P2; e602 SYSTEM (start 1-2 bit 0-1, coin 1-2 bit 2-3); e603
 * protezione (solo via leggi_e603(pc)); e604 DSW1 / ribaltamento schermo;
 * e605 DSW2; e606 lettura a vuoto (0). I dump di snap.lua passano dagli
 * handler MAME: ram[e6xx - c000] contiene il valore vivo all'istante
 * della cattura (proposta A del brief 12). */
uint8_t leggi_porta(uint16_t a);   /* e600-e606: ram[a - 0xc000] */
void scrivi_porta(uint16_t a, uint8_t v);  /* scrive ram[a - 0xc000] */
```

- `leggi_porta`: per e600-e602 e e604-e606 ritorna `ram[a - 0xc000]`; per
  e603 avviso su stderr («usare leggi_e603») e valore dal dump; per ogni
  altro indirizzo avviso su stderr e comunque `ram[a - 0xc000]`
  (conservativo e visibile, mai silenzioso).
- `scrivi_porta`: scrive `ram[a - 0xc000]` per qualunque a ≥ 0xe600, come
  già fa `sub_33ba` con e802. La zona e600-ffff è **fuori confronto** (il
  banco confronta c000-e5ff, `harness.c` riga 109: 0x2600 byte), quindi la
  scrittura è visibile al debug e innocua per l'oracolo.
- In questo lotto **nessun sito eseguito eleggibile scrive porte** (le
  scritture e600/e604/e800 stanno in avvio/NMI/corpi di fase): la funzione
  entra ora per fissare l'API dei filoni futuri. «scelta di progetto»
  (conservativa: nessun uso senza oracolo).
- Nota per il futuro (annotata, nessun lavoro ora): su e604 lettura (DSW1)
  e scrittura (ribaltamento) sono registri hardware diversi; una routine
  che scrivesse e rileggesse la stessa porta richiederebbe la proposta C.
- Regola per il C tradotto: ai siti porta **indirizzo hardware esplicito**
  (`leggi_porta(0xe604)`), mai `M()` sulle porte; a ogni sito un commento
  con il significato MAME (es. «DSW1 bit 0 = Demo Sounds»). `M()` resta per
  la RAM. `sub_33ba` resta com'è (decisione 2 del brief 12).

## Routine del lotto 9 (21: 20 registrate + 797E a cattura singola)

Estensione reale letta nel dasm (il CSV sovrastima); t/s/g dalle
`out/<sessione>_chiamate.txt` (per 3404, coda non chiamata, dal PC
d'ingresso nelle `*_istruzioni.txt`). Catture con le regole del brief 05
§1; stati scelti dai conteggi. Porte lette nel lotto: 10 siti.

| ingresso | estensione | byte | t / s / g | come si arriva | effetto (da dasm) | catture |
|---|---|---:|---|---|---|---|
| 0043 | 0043-004E (coda = sub_004f) | 12 | 5 / 26 / 3 | 002B (gestore rst 28) | filtro suoni in attract: se c045&6 = 0 legge **e604 (DSW1 bit 0 = Demo Sounds)**, `rrca`, se carry esce; altrimenti prosegue in sub_004f (accoda C in c054-c057); A d'uscita = c045&6 o DSW1 ruotato | titolo, gioco |
| 234A | 234A-2352 | 9 | 2 / 0 / 0 | 29BA (da 2980) | (HL) = C, (HL+1) = A, A = bit 6-7 di B (campo Coin A) | titolo, titolo N=2 `n2`; se le `prima` coincidono (DSW1 fissa): `tradotta` con nota |
| 2980 | 2980-29C6 (tabelle 29C7/29CB) | 71 | 2 / 0 / 0 | 28F4, 28FA (avvio) | azzera c046-c04b (6 byte); legge **e604 (DSW1 bit 4-7 = Coin B/A)**; se Coin A = Coin B = 0 scrive 80 in c048-c049; estrae i due campi nelle celle credito via sub_234a (c04a-c04b) e c04d dalla tabella 29CB | titolo, titolo N=2 `n2` |
| 2AE2 | 2AE2-2AF4 | 19 | 1525 / 0 / 0 | 2AAA (attract) | disegna gli 8 bit di **e604 (DSW1)** in d765 e di **e605 (DSW2)** in d785 (riga di servizio) via sub_2af5 (verificata) | titolo, titolo N=2 `n2` |
| 2BC3 | 2BC3-2BC8 + coda 2353-2367 (esclusiva: unico `jp` da 2BC6) | 27 | 71135+1525 / 0 / 0 | 29EB (ciclo attract), 2AAD | HL = e600; A = ((**P1** \| **P2**) & 3F) \| (**SYSTEM** & 3): lettura ingressi di avvio/attract via HL (nel C: `leggi_porta` di e600, e601, e602) | titolo; gioco `moneta` con `SNAP_COND='b@e602>=4'` (vera ai frame moneta 1510-1530; i primi frame della lista falliscono e si arriva a 1500) |
| 2BC9 | 2BC9-2BF2 + coda 2C16-2C19 (condivisa con 2BF3) | 46 | 2 / 0 / 0 | 2892, 289E (test RAM d'avvio) | test RAM a nibble: per 8 pattern E, riempe con sub_2c1a e rilegge confrontando il nibble basso; carry d'uscita 1 = ok, 0 = guasto (emularlo in z->f con commento) | titolo, titolo N=2 `n2` |
| 2BF3 | 2BF3-2C15 + coda 2C16-2C19 | 39 | 5 / 0 / 0 | 2843, 284F, 2877, 2883, 28AD (avvio) | come 2BC9 ma confronto a byte intero; nel ciclo legge **e602 a vuoto (A subito sovrascritto: probabile watchdog)**; carry come 2BC9 | titolo, titolo N=2 `n2` |
| 2C1A | 2C1A-2C26 | 13 | 56 / 0 / 0 | 2BCE, 2BF8 (e ingresso = ciclo: `jr nz,sub_2c1ah`) | riempe BC byte da HL col pattern rotante D; a ogni giro legge **e602 a vuoto (watchdog)** | titolo, titolo N=2 `n2` |
| 2E11 | 2E11-2E1D | 13 | 2 / 2 / 0 | 2DFF, 2E0B (corpi di fase) | c19e = 0; confronta c248 (stanza corrente) con c249: Z d'uscita usato dal chiamante (flags non confrontate dal banco, nota in commento) | titolo, gioco |
| 2E90 | 2E90-2E9B | 12 | 3080 / 3318 / 3603 | 2E8B (trampolino dei corpi di fase) | cancello a conto alla rovescia: `dec (HL)`; se c467 < C ritorna, altrimenti c467 = 0 e `inc (HL)` (annulla) | titolo, gioco |
| 2F57 | 2F57-2F73 | 29 | 1 / 1 / 1 | 2F2D | carica il fondale di gioco: sub_3f88 (HL = 80D3), azzera d5c0 (96 byte), sub_4398 con A=1 e A=0, c467 = 0 | titolo, gioco |
| 33EB | 33EB-3403 (coda = 3404) | 25 | 1 / 2 / 0 | 2C31, 2C37 (avvio), 7620 | azzera c237-c267 (49 byte), c244 = FF, copia 6 byte ROM 3427 → c258; prosegue senza `ret` in 3404 (il C chiama `sub_3404(z)` in coda) | titolo, gioco |
| 3404 | 3404-3426 | 35 | 1 / 2 / 0 | coda di 33EB (mai chiamata: cattura dal PC d'ingresso, rientro da (SP), come i gestori del lotto 7) | c237 = c04d; sub_349f (tabellone); sub_342d (bonus); se c045&6: azzera dc55 (3 byte); c24e = 4 | titolo, gioco |
| 342D | 342D-343E (tabella 343F, 8 parole) | 18 | 1 / 2 / 0 | 340D (da 3404) | legge **e605 (DSW2 bit 5-7 = Bonus Life)**: indice ×2 nella tabella 343F via rst 8/rst 10 (nel C: `leggi()` della ROM), puntatore in c23d-c23e | titolo, gioco |
| 354B | 354B-355C (tabella 355D) | 18 | 1 / 1 / 1 | 3547 | HL = 355D + 4·c24a, A = 0, sub_3569: copia il record di 4 byte della stanza-bonus nella destinazione del record 0 di 3574 | titolo, gioco |
| 3569 | 3569-3573 | 11 | 4 / 5 / 4 | 3559, 3F8A | copia generica: record (DE = destinazione, C = lunghezza) dalla tabella 3574 + 3·A via sub_23f8 (verificata), poi `ldir` da HL | titolo, gioco |
| 3812 | 3812-3828 | 23 | 2 / 2 / 2 | 37F2, 37F7 (corpo di fase 37D7) | espande una mappa di bit (24 righe × 8) da IX nella griglia c0cd+: scrive C dove il bit è 1 | titolo, gioco |
| 3F88 | 3F88-3F92 | 11 | 3 / 4 / 2 | 2F5A, 388E, 790C, 7C48, 7CAC | A = 6, sub_3569 (copia da HL nel piano video), poi `set 6,(c044)` | titolo, gioco |
| 75AA | 75AA-75E6 | 61 | 2 / 2 / 1 | 761D, 78E4, 7C4E, 7C98 | azzeramenti: piani video d480/d080 (0x300), c299 (0x10), slot oggetti c46e (0x580), slot c2c1 (0x298), `res 0,(c258)` | titolo, gioco |
| 7C84 | 7C84-7C97 | 20 | 1 / 2 / 0 | 761A, 7C4B, 7C9B | azzera ddaa e ddca (14 + 14 byte: righe del pannello) | titolo, gioco |
| 797E | 797E-79A3 | 38 | 1 / 0 / 0 | 7970 | due cifre BCD di (HL) in ASCII a (DE) e (DE+8); il ramo 798B-7993 (riporto oltre '2') non è mai eseguito: tradurre dal dasm statico con nota | titolo sola: occorrenza unica nelle tracce → `tradotta` con nota (N=2 atteso fallire) |

Totale ~550 byte di estensione reale. Il disassembly si legge come nei
brief precedenti, da `decomp/`:
`sed -n "$(grep -n ';2bc9$' out/main48.dasm | cut -d: -f1),+40p" out/main48.dasm`
e le istruzioni eseguite con
`awk -F'\t' '$1>="2BC9" && $1<="2C26"' out/istruzioni.txt`.
Attenzione al dasm disallineato attorno a 2980 (la zona precedente è dati):
lì fanno fede le `*_istruzioni.txt`.

### Escluse e perché (nessun lavoro in questo lotto)

- Corpi di fase con porte (2E9C, 358F, 3861, 7C98, 760B) e chiamanti
  d'avvio/attract (2833-295B, 29CF/29EB, 2AAA, 002B): filone strutturale.
- NMI 0066 (015E, 019A, 01A4, 01BB, 01BF): filone NMI (brief 12).
- 37B4 (35 byte reali): a 37D2 chiama 4767, smistatore su A con rami in
  gran parte mai eseguiti e `rst 38` (esclusione del brief 11, invariata).
- 322A, 2A03: `ex (sp),hl` (brief 11). 28E3, 2ACB, 42B3, 4CED, 4CF8,
  62BE: mai eseguiti → filone zone mai eseguite (decisione 3, brief 12).

## Struttura

- `porte.c`: le due funzioni sopra (file resta ~60 righe); prototipi in
  `ram.h` accanto a `leggi_e603`.
- `z80_0xxx.c`: `sub_0043` in fondo. `z80_2xxx.c` (238 righe): 234A, 2980,
  2AE2, 2BC3 (con la coda 2353 nel corpo), 2BC9, 2BF3, 2C1A, 2E11, 2E90,
  2F57 in fondo, in ordine di indirizzo; stima finale ~520 righe (< 600).
- `z80_3xxx.c` (482 righe): 33EB, 3404, 342D, 354B, 3569, 3812, 3F88.
  Supererà le 600 righe: spostare in blocco le funzioni da 0x3800 in su
  (3812 nuova, le esistenti 380A…3F10 nell'ordine attuale, senza
  riordinarle) in un nuovo `z80_38xx.c` (regola del brief 05 §4).
- `z80_7xxx.c` (84 righe): 75AA, 797E, 7C84 in fondo.
- La coda condivisa 2C16-2C19 (`or a; pop hl; pop bc; ret`): in C è il
  ramo d'errore dentro 2BC9 e 2BF3 (nessuna funzione propria). «preferenza»
- `routine.h`: blocco «Lotto 9 (compito 13)» con i 21 prototipi;
  `routine_tab.c`: 21 voci in ordine di indirizzo, `blocco_ini/fine`:
  2980 → (0xc046, 0xc04d); tutte le altre (0, 0).
- `ROUTINE.md`: 21 righe nuove (95 totali).
- Catture: 41 attese (20 routine × 2 + 797E singola; per 3404 la cattura
  parte dal PC d'ingresso: vale il rientro da (SP), come nel lotto 7).

## Verifiche obbligatorie (da `decomp/`)

1. `make -C harness 2>&1 | grep -ciE 'warning|error'` → `0` (attesa).
2. Per ognuna delle 20 registrate con 2 catture: `(cd harness && ./harness
   0xXXXX | tail -1)` → `RIEPILOGO 0xXXXX: 2 catture, 2 OK`, uscita 0;
   `./harness 0x797E` → `1 catture, 1 OK` (attesa; 1 cattura con nota dove
   la regola del brief 06 domanda 2 lo ammette).
3. `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `95`;
   `grep -c FALLITO` → `0` (attesa).
4. `(cd harness && ./sched_proto ../out/sched_log.txt | tail -1)` →
   `spedizioni confrontate: 106095, differenze: 0; posta_attivita: 131,
   slot coincidenti: 131` (misurato oggi).
5. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale" && $2!="total"'` → vuoto.
6. `grep -c '^| 0x' ROUTINE.md` → `95`; `grep -c '| verificata |' ROUTINE.md`
   → `91` attesa (71 + 20), meno le eventuali rimaste `tradotta` con nota
   (797E già conteggiata fuori).
7. `ls out/snap_0x*_*_regs.txt | wc -l` → `188` attesa (147 + 41), meno le
   eventuali catture annotate come assenti.
8. `prima.bin` (o righe `prima` dei `regs` per le solo-registri) diverse
   per ognuna delle 20 con 2 catture (ciclo `cmp`) → vuoto (attesa).
9. `python3 censimento_porte.py | head -1` → `# Accessi diretti (operando
   e6xx/e8xx): 31`; `| grep -c '^[0-9A-F]'` → `45` (il lotto non tocca la ROM).
10. `grep -n 'M(0xe6\|M(0xe8' harness/z80_*.c` → solo la riga storica di
    `sub_33ba` (`M(0xe802)`, z80_3xxx.c o z80_38xx.c dopo lo spostamento);
    `grep -c 'leggi_porta(0xe6' harness/z80_*.c` (totale) → `10`
    (004A, 298A, 2AE2, 2AEB, 3 in 2BC3, 2C08, 2C1F, 342D).
11. `grep -n -e '-window' -e 'video soft' cattura.sh snap.lua` → vuoto.
12. `find .. -size +50M` → vuoto; `pgrep -c mame` → `0`.
13. `git status --short` → solo i file del lotto (porte.c, ram.h,
    routine.h, routine_tab.c, z80_0xxx.c, z80_2xxx.c, z80_3xxx.c,
    z80_38xx.c nuovo, z80_7xxx.c, ROUTINE.md, catture nuove in out/,
    questo brief).

## Domande aperte e risoluzioni dell'analista

1. Prototipi di `leggi_porta`/`scrivi_porta` in `ram.h` (accanto a
   `leggi_e603`), non in `routine.h`: sono piattaforma, non lotto.
   Risolta con la proposta dell'analista. «preferenza»
2. I corpi di fase con porte (2E9C, 358F, 3861, 7C98, 760B) restano al
   filone strutturale nonostante il brief 12 li contasse nei ~1,5 KB: la
   lettura del dasm (sopra) mostra che non sono routine chiamabili; il
   lotto traduce le 21 routine reali dei grappoli (~550 byte) e fissa
   l'API porte. Risolta in modo conservativo e annotata. «scelta di progetto»
3. `scrivi_porta` entra senza siti eseguiti (tutte le scritture eleggibili
   stanno nei filoni rinviati): API fissata ora, primo uso nei filoni
   NMI/strutturale. Risolta in modo conservativo. «scelta di progetto»
4. Lotto da 21 routine (> 15 del brief 05): quasi tutte sotto i 30 byte e
   legate dallo stesso prerequisito porte; spezzarle in due lotti
   raddoppierebbe la meccanica senza ridurre il rischio. Risolta in modo
   conservativo (si traducono solo i grappoli censiti). «scelta di progetto»
5. Carry d'uscita di 2BC9/2BF3 (esito del test RAM) e Z di 2E11: emulati
   su `z->f` con commento (il banco non confronta F; servono ai chiamanti
   del filone strutturale). Risolta con la proposta dell'analista.
   «preferenza»
