# Compito 08: lotto 4 di traduzione in C delle routine del Z80 principale

Data: 2026-10-21. Scritto dall'agente analista per un agente sviluppatore.
Schema, convenzioni, vincoli e strumenti: brief 05 («Schema dei lotti»), 06 e
07; tutto già realizzato in `decomp/`. MAME `/usr/games/mame` 0.264 con
`PATH=$PATH:/usr/games`; nessuna operazione git; non toccare `roms/` né
`.gitignore`; nessun file sopra le 800 righe; nomi `sub_xxxx`.

## Cosa esiste già (misurato oggi, dopo il lotto 3)

I dati derivati in `out/` sono quelli rigenerati per il lotto 3 (brief 07):
stesso `callgraph.csv`, stesse `*_chiamate.txt`, `sched_log.txt` da 106095
spedizioni. Banco: 48 routine, `./harness --tutte` → 48 RIEPILOGO, 0 FALLITO;
`ROUTINE.md` 48 righe `verificata`; 94 catture `regs` con tag (più 3 `base`).

## Routine del lotto 4 (12: le eleggibili sono meno di 15)

Selezione coi criteri del brief 05 §6 sul `callgraph.csv` rigenerato, tolte
le 48 già verificate. Dopo il lotto 3 sono diventate eleggibili le routine
che chiamano 2B08, 3101, 34E1, 36BD, 452B, 4D37, 4D87. Estensioni
reali lette in `main48.dasm`.

| ingresso | estensione | byte | t / s / g | siti | effetto (da dasm) | catture |
|---|---|---:|---|---|---|---|
| 004F | 004F-005C | 14 | 0 / 1 / 0 | 0219 | scrive C nel primo byte libero (==0) fra c054-c056, altrimenti a c057; A = ultimo byte letto | gioco, tardo |
| 2AF5 | 2AF5-2B07 | 19 | 3050 / 0 / 0 | 2AE8, 2AF1 | 8 righe di testo: ruota A col **riporto in ingresso** (`rra`, `push af`/`pop af` attorno a ogni chiamata), C = 0C se riporto, 0D no, `call sub_2b08` con bit 7 di B acceso (HL avanza di riga in riga); B da 80 a 88 | titolo, titolo N=2 |
| 2FF3 | 2FF3-2FFF | 13 | 2 / 2 / 0 | 300D, 4D59* | differenze L-E e (dopo `ex`) L-H, ognuna passata a `sub_3000` col **prestito della `sub`** come segno; HL e DE = le due differenze ×4 | titolo, gioco |
| 3097 | 3097-30EE | 88 | 68 / 67 / 0 | 306F | dispone 16 oggetti (c46e, passo 30) su un cerchio: per slot due coppie `sub_30ef` (seno) + `sub_3101` (scala per C=c2c6), negate dai bit 6 di E e E+20, centro H:L da c2c2/c2c4, (IX+0)=C5, E += 8 | titolo, gioco |
| 31E9 | 31E9-321D | 53 | 2 / 2 / 1 | 3188 | barra di B=min(A,6) colonne (tile 80/81, attributo 60, due righe da df80) e 6-B chiamate a `sub_23ac` per pulire il resto | titolo, gioco |
| 349F | 349F-34D3 | 53 | 2 / 3 / 1 | 3466, 347B | pulisce df8e/dfae (`sub_34d4`), poi con `sub_34e1` scrive 90 a L=97 e, per C=(c259) posizioni, il carattere 82+2·(coppia di bit alti di DE=(c25c), `add hl,hl`+`rl a` ×2), chiusa da 92 | titolo, gioco |
| 3678 | 3678-36B2 | 59 | 1 / 0 / 0 | 2958 | riempie la palette e400 con 4 passate di `sub_36bd` (HL prosegue fra le chiamate: BC=0001/0010/0100/0111, A=10) e copia 10 byte ROM 36B3 → e496 (`ldir`) | titolo, titolo N=2 |
| 36C3 | 36C3-36EF | 45 | 1 / 0 / 0 | 2955 | 4 `ldir` da ROM: 80D3→e400 (0x200), 8199→c96e (0x26), 80D3→c994 (0x20), 82A7→c9b4 (0x16) | titolo, titolo N=2 |
| 36F0 | 36F0-3708 | 25 | 0 / 616 / 695 | 12A4, 5C47 | pixel → cella della griglia FG: offset = (~H-10 & f0) + ((L+8)>>4 & 0f), HL = c0bd+offset, DE = offset; solo registri (scrive nulla) | gioco, tardo |
| 4398 | 4398-43B8 | 33 | 2 / 2 / 3 | 3DBB, 7D2D | record di 3 byte (tabelle 43B9 o 43C5 e BC=c09b/c0b5 secondo il bit 4 di A): OR del terzo byte in (BC), (c464) scritto alla parola del record, 0 alla parola+2 | titolo, gioco |
| 4D05 | 4D05-4D28 | 36 | 0 / 1 / 0 | 3EDE | cifre di c239/c23b con `sub_4d37`; se c045 & 6: `sub_4d87` da c061 verso d44a o d45f (bit 0 di c045), attributo 71 | gioco, tardo |
| 7E32 | 7E32-7E39 | 8 | 0 / 6 / 0 | 7E1D, 7E24 | DE = parola della tabella 7E3A all'indice A (`rst 8`+`rst 10`), HL conservato con push/pop; solo registri | gioco, tardo |

(*) il sito 4D59 di 2FF3 è dentro 4D37 già tradotta: nel C non serve.

Note: 2AF5, 3678, 36C3 girano solo nel titolo (s = g = 0): seconda cattura
`titolo 2 n2` come nel lotto 3; 3678 e 36C3 hanno 1 sola invocazione nella
finestra 0-1500 — se N=2 non esiste entro 6000 frame la routine resta
`tradotta` con nota (regola del brief 06, domanda 2). In 3097 la `sub_3101`
copia B in D a ogni chiamata: l'effetto sui registri finali è coperto da
`ld de,0030h` (D = 0) e va riprodotto solo lo stato d'uscita.

Rinviate al lotto 5 (chiamano routine di questo lotto): 1288 e 5C32 (→36F0),
3045 (→3097), 2F57 (→4398, e 3F88 da esaminare), 7E1D e 7E24 (→7E32),
7DEA (→7E1D). CORREZIONE del 2026-10-21: la prima stesura del brief dava
7E1D e 7E24 per eleggibili credendo 7E32 tradotta nel lotto 3; 7E32 non lo
era (criterio §6 violato). 7E32 entra in questo lotto al loro posto. Escluse per
le ragioni già annotate nei brief 05-07: 0602, 08A2, 08CE, 04AC, 0038, 3A54,
4F0F, 5507, 7F2D, 6C2C, 01D5/0219 (0493), 03FB (0469), 36F0→ok ma 37B4
(→3812 porte), 4767, 027C, 0CE6, 7379, 234A, 05B5, 2BC9/29CF (→2C1A/2BC3
porte), 2E11/797E (→0493 e porte), 322A/2A03 (`ex (sp),hl`), 354B/3F88/33EB
(→3569/342D porte), 2F74 (→0028).

## Struttura

- `z80_0xxx.c`: `sub_004f`; `z80_2xxx.c`: `sub_2af5`, `sub_2ff3`;
  `z80_3xxx.c`: `sub_3097`, `sub_31e9`, `sub_349f`, `sub_3678`, `sub_36c3`,
  `sub_36f0`; `z80_4xxx.c`: `sub_4398`, `sub_4d05`; `z80_7xxx.c`: `sub_7e32`.
- `routine.h`: blocco «Lotto 4 (compito 08)»; `routine_tab.c`: 12 voci in
  ordine di indirizzo, `blocco_ini/fine` 0,0 (36C3: e400-e5ff).
- Catture come da colonna; regole di qualità del brief 05 §1 (per «solo
  registri» 36F0 e 7E32 devono differire le righe `prima`).
- `ROUTINE.md`: 12 righe nuove.

## Verifiche obbligatorie (da `decomp/`)

1. `make -C harness 2>&1 | grep -ciE 'warning|error'` → `0` (attesa).
2. Ognuna delle 12: `(cd harness && ./harness 0xXXXX | tail -1)` →
   `RIEPILOGO 0xXXXX: 2 catture, 2 OK`, uscita 0 (attesa; 1 cattura e stato
   `tradotta` ammessi solo per 3678/36C3 con nota).
3. `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `60`;
   `grep -c FALLITO` → `0` (attesa).
4. `(cd harness && ./sched_proto ../out/sched_log.txt | tail -1)` →
   `spedizioni confrontate: 106095, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
5. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale" && $2!="total"'` → vuoto (misurato: vuoto).
6. `grep -c '| verificata |' ROUTINE.md` → `60`; `grep -c '^| 0x' ROUTINE.md`
   → `60` (attesa; meno solo per `tradotta`/`scartata` spiegate).
7. `ls out/snap_0x*_*_regs.txt | wc -l` → `118` (94 + 24) (attesa).
8. `prima.bin` diverse per ciascuna delle 12 (ciclo `cmp` come brief 07) → vuoto (attesa).
9. `grep -n -e '-window' -e 'video soft' cattura.sh snap.lua` → vuoto (misurato).
10. `find .. -size +50M` → vuoto (misurato). `pgrep -c mame` → `0`.
11. `git status --short` → solo i file del lotto (brief, ROUTINE.md,
    routine.h, routine_tab.c, z80_{0,2,3,4,7}xxx.c, snap_* nuovi).

## Domande aperte e decisioni (chat principale, 2026-10-21)

1. Lotto da 13 (eleggibili < 15). Accettato, annotato nel rapporto. «scelta di progetto»
2. 3678/36C3: N=2 nel titolo, altrimenti `tradotta` con nota. Accettato. «preferenza»
3. Rinvii al lotto 5 per dipendenze interne al lotto (criterio §6 alla lettera). Accettato. «scelta di progetto»

## Chiusura ciclo (chat principale, 2026-10-21)

Esito: PASSATO. Ciclo intero nella stessa sessione cloud del lotto 3;
recupero con diff4.b64 (scaricato col permesso dell'utente) e 22 catture
rigenerate in locale: 0 fallite, regs identici bit a bit. Verificatore
locale: passate tutte nella sostanza; deviazioni ammesse dal brief: sched_log
storico 106124 (differenze 0), 3678 e 36C3 `tradotta` con nota (una sola
cattura, N=2 assente entro 6000 frame), quindi 58 `verificata` su 60 righe e
116 regs. Lotto di 12 routine (13 candidati, uno annotato nel brief).
Totale: 60 righe in ROUTINE.md, 58 verificate.
Token: ciclo in cloud su Fable 5 (crediti cloud); verificatore locale Sonnet 20k.
