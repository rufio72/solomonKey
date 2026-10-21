# Compito 09: lotto 5 di traduzione in C delle routine del Z80 principale

Data: 2026-10-21. Schema, convenzioni, vincoli e strumenti: brief 05 («Schema
dei lotti»), 06, 07 e 08. MAME `/usr/games/mame` 0.264 con
`PATH=$PATH:/usr/games`; nessuna operazione git; non toccare `roms/` né
`.gitignore`; nessun file sopra le 800 righe; nomi `sub_xxxx`.

## Cosa esiste già (misurato oggi, dopo il lotto 4)

Dati derivati in `out/` come nei brief 07-08 (sched_log 106095). Banco: 60
routine registrate, `--tutte` → 60 RIEPILOGO, 0 FALLITO; ROUTINE.md 60 righe
(58 `verificata`, 2 `tradotta`: 3678, 36C3); 116 catture `regs` con tag.

## Routine del lotto 5 (4: le eleggibili sono meno di 15)

Criteri del brief 05 §6; dopo il lotto 4 sono diventate eleggibili le
chiamanti di 36F0, 3097, 4398, 7E32. Estensioni reali da `main48.dasm`.

| ingresso | estensione | byte | t / s / g | siti | effetto (da dasm) | catture |
|---|---|---:|---|---|---|---|
| 3045 | 3045-305E | 26 | 1 / 1 / 0 | 2FFC* | inizializza 16 oggetti da c46e (passo 0x30) con `sub_4af6` (A=45, E=40, B=4, C=0); il blocco a 305F NON fa parte della routine | titolo, gioco |
| 5C32 | 5C32-5C77 | 70 | 0 / 404 / 351 | 565A | classi delle 4 celle attorno all'oggetto IY: coordinate da (IY+15)+10/6 (bit 0 di IY+1E) e 18+(IY+0E), `sub_36f0` per la cella, 4 chiamate a `sub_5c78` scritte in c07e-c081 (vicina ±1, poi +10 e +10 di fila) | gioco, tardo |
| 7E1D | 7E1D-7E23 | 7 | 0 / 3 / 0 | 7D3F, 7DAD, 7DC7 | `sub_7e32` (destinazione dalla tabella 7E3A) poi `sub_6771` (stringa in due piani) | gioco, tardo |
| 7E24 | 7E24-7E31 | 14 | 0 / 3 / 0 | 7B95, 7BB8, 7D81 | come 7E1D più `inc de` e la stringa fissa 7EA3 (`sub_6771`) | gioco, tardo |

(*) il sito di 3045 è nel blocco 3009-3044, non ancora tradotto.

Escluse di nuovo, con ragione: 1288 (il corpo statico prosegue nella zona
mai eseguita 12ab-2349 con `rst 28`, `rst 38` e chiamate a 0493: come 0CE6,
non traducibile senza indovinare), 2F57 (chiama 3F88 → 3569, porte), 7DEA e
7E03/7E14 (chiamano 7E1D di questo lotto: 7DEA al lotto 6; 7E03/7E14 non
sono nel callgraph, mai eseguite). Restano escluse quelle dei brief 05-08.

## Struttura

- `z80_3xxx.c`: `sub_3045`; `z80_5xxx.c`: `sub_5c32`; `z80_7xxx.c`:
  `sub_7e1d`, `sub_7e24`.
- `routine.h`: blocco «Lotto 5 (compito 09)»; `routine_tab.c`: 4 voci
  (3045: blocco 0,0; 5C32: c07e-c081; 7E1D/7E24: 0,0).
- Catture come da colonna; regole di qualità del brief 05 §1.
- `ROUTINE.md`: 4 righe nuove.

## Verifiche obbligatorie (da `decomp/`)

1. `make -C harness 2>&1 | grep -ciE 'warning|error'` → `0` (attesa).
2. Ognuna delle 4: `(cd harness && ./harness 0xXXXX | tail -1)` →
   `RIEPILOGO 0xXXXX: 2 catture, 2 OK`, uscita 0 (attesa).
3. `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `64`;
   `grep -c FALLITO` → `0` (attesa).
4. `(cd harness && ./sched_proto ../out/sched_log.txt | tail -1)` →
   `spedizioni confrontate: 106095, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
5. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale" && $2!="total"'` → vuoto (misurato: vuoto).
6. `grep -c '| verificata |' ROUTINE.md` → `62`; `grep -c '^| 0x' ROUTINE.md`
   → `64` (attesa; 3678 e 36C3 restano `tradotta`).
7. `ls out/snap_0x*_*_regs.txt | wc -l` → `124` (116 + 8) (attesa).
8. `prima.bin` diverse per ciascuna delle 4 (ciclo `cmp`) → vuoto (attesa).
9. `grep -n -e '-window' -e 'video soft' cattura.sh snap.lua` → vuoto (misurato).
10. `find .. -size +50M` → vuoto (misurato). `pgrep -c mame` → `0`.
11. `git status --short` → solo i file del lotto.

## Domande aperte e decisioni (chat principale, 2026-10-21)

1. Lotto da 4 (eleggibili < 15). Accettato, annotato nel rapporto. «scelta di progetto»
2. 1288 scartata in selezione (zona mai eseguita) invece che tradotta a
   metà. Conservativa. «scelta di progetto»

## Chiusura ciclo (chat principale, 2026-10-21)

Esito: PASSATO. Ciclo nella stessa sessione cloud dei lotti 3-4; recupero con
diff5.b64 e 8 catture rigenerate in locale: 0 fallite, regs identici bit a
bit. Verificatore locale: tutte passate; unica deviazione il conteggio dello
sched_log (storico 106124 vs 106095 del log rigenerato in cloud, differenze
0, nota ormai standard). 4 routine, tutte verificata con 2 catture.
Totale: 64 righe in ROUTINE.md, 62 verificate.
Token: ciclo in cloud su Fable 5 (crediti cloud); verificatore locale Sonnet 16k.
