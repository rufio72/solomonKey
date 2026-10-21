# Compito 10: lotto 6 di traduzione in C delle routine del Z80 principale

Data: 2026-10-21. Schema, convenzioni, vincoli e strumenti: brief 05 («Schema
dei lotti»), 06-09. MAME `/usr/games/mame` 0.264 con `PATH=$PATH:/usr/games`;
nessuna operazione git; non toccare `roms/` né `.gitignore`; nessun file
sopra le 800 righe; nomi `sub_xxxx`.

## Cosa esiste già (misurato oggi, dopo il lotto 5)

Dati derivati come nei brief 07-09 (sched_log 106095). Banco: 64 routine,
`--tutte` → 64 RIEPILOGO, 0 FALLITO; ROUTINE.md 64 righe (62 `verificata`,
2 `tradotta`: 3678, 36C3); 124 catture `regs` con tag.

## Routine del lotto 6 (2: le eleggibili sono meno di 15)

Criteri del brief 05 §6. Dopo il lotto 5 restano eleggibili solo le due qui
sotto: tutte le altre routine eseguite e non tradotte chiamano porte, `rst
20`/`rst 28`/`rst 38`, 0493/0038, o proseguono in zone mai eseguite (elenco
sotto). Estensioni reali da `main48.dasm`.

| ingresso | estensione | byte | t / s / g | siti | effetto (da dasm) | catture |
|---|---|---:|---|---|---|---|
| 2F74 | 2F74-2F8E | 27 | 3 / 3 / 5 | 2F48 | salva H/L in (IX+0E)/(IX+15), (IX+3)=0A, inizializza l'oggetto con `sub_4af6` (A=C0, E=0A), poi L += 10 e IX += 30 (oggetto successivo); i callee oltre il `ret` a 2F8E sono di altre routine | titolo, gioco |
| 7DEA | 7DEA-7E02 | 25 | 0 / 1 / 0 | 7D65* | tre stringhe d'esito con `sub_7e1d`: (A=3, HL=7E76), (A=4, HL=7E85), (A=5, HL=7E94) | gioco, tardo |

(*) 7DEA è chiamata dalla routine a 7D65 (non tradotta).

Esaminate e ancora escluse, con ragione: 33EB (prosegue senza `ret` in 3404
che chiama 342D, porte), 7ECA (chiama 7F2D → 0038), 354B (chiama 3569,
porte), 2BC9 (chiama 2C1A, porte+rst20), 0219/2E11/797E (0493 o porte),
29CF (2BC3 porte), 322A/2A03 (`ex (sp),hl`), 1288/0CE6/027C/03FB/04AC/
05B5/234A/7379/0602/08A2/08CE/01D5/0038/3A54/4F0F/5507/0028/37B4/4767/
3F88/2F57/7F2D/6C2C/2F74→no (questa entra), 7E03/7E14 (mai eseguite):
ragioni nei brief 05-09.

## Struttura

- `z80_2xxx.c`: `sub_2f74`; `z80_7xxx.c`: `sub_7dea`.
- `routine.h`: blocco «Lotto 6 (compito 10)»; `routine_tab.c`: 2 voci (0,0).
- Catture come da colonna; regole di qualità del brief 05 §1.
- `ROUTINE.md`: 2 righe nuove.

## Verifiche obbligatorie (da `decomp/`)

1. `make -C harness 2>&1 | grep -ciE 'warning|error'` → `0` (attesa).
2. `(cd harness && ./harness 0x2F74 | tail -1)` e `0x7DEA` →
   `RIEPILOGO …: 2 catture, 2 OK`, uscita 0 (attesa).
3. `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `66`;
   `grep -c FALLITO` → `0` (attesa).
4. `(cd harness && ./sched_proto ../out/sched_log.txt | tail -1)` →
   `spedizioni confrontate: 106095, differenze: 0; posta_attivita: 131, slot coincidenti: 131` (misurato).
5. `wc -l harness/*.c harness/*.h *.lua *.py *.sh | awk '$1>800 && $2!="totale" && $2!="total"'` → vuoto (misurato: vuoto).
6. `grep -c '| verificata |' ROUTINE.md` → `64`; `grep -c '^| 0x' ROUTINE.md`
   → `66` (attesa).
7. `ls out/snap_0x*_*_regs.txt | wc -l` → `128` (124 + 4) (attesa).
8. `prima.bin` diverse per entrambe → vuoto (attesa).
9. `grep -n -e '-window' -e 'video soft' cattura.sh snap.lua` → vuoto (misurato).
10. `find .. -size +50M` → vuoto (misurato). `pgrep -c mame` → `0`.
11. `git status --short` → solo i file del lotto.

## Domande aperte e decisioni (chat principale, 2026-10-21)

1. Lotto da 2 (eleggibili < 15): il filone delle routine foglia «facili» è
   esaurito; i lotti successivi richiedono di sciogliere i blocchi
   strutturali (0493/scheduler nel banco, porte come funzioni di
   piattaforma, 0038/rst 28, zone mai eseguite con sessioni guidate).
   Accettato, annotato nel rapporto. «scelta di progetto»

## Chiusura ciclo (chat principale, 2026-10-21)

Esito: PASSATO. Ciclo nella stessa sessione cloud dei lotti 3-5; recupero con
diff6.b64 e 3 catture rigenerate in locale: 0 fallite, regs identici bit a
bit. Verificatore locale: passate nella sostanza; deviazioni ammesse: 7DEA
resta `tradotta` con nota (occorrenza unica in gioco f1542; N=2, titolo e
tardo senza occorrenze), sched_log storico 106124 (differenze 0, nota
standard). Lotto da 2 routine: le eleggibili «foglia» sono esaurite; i lotti
successivi richiedono di sciogliere i blocchi con chiamate a routine non
tradotte. Totale: 66 righe in ROUTINE.md, 63 verificate.
Token: ciclo in cloud su Fable 5 (crediti cloud); verificatore locale Sonnet 15k.
