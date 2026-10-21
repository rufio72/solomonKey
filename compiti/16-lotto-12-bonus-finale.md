# Compito 16: lotto 12 — copertura dei quadri bonus e del finale

Data: 2026-10-21. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Far eseguire in MAME il codice del Z80 principale mai osservato (104
destinazioni statiche di `call` su 299, blocco grosso in 8xxx-Axxx: quadri
bonus/nascosti e finale), con sessioni guidate e forzature mirate di RAM;
catturare le tracce, misurare quante delle 104 restano scoperte, censire le
routine nuove (stima 25-35, **non tradurle**), aggiornare i cancelli di
copertura. Il vincolo «bonus e finale non vanno tentati» del brief 04 cade
con questo lotto.

Vincoli assoluti:
- Nessuna modifica fuori da `decomp/` (non toccare `roms/`, `compiti/`,
  `.gitignore`, `README.md`, `STATO.md`). Nessuna operazione git.
- Non toccare `harness/` (nessuna traduzione in questo lotto), `trace.lua`,
  `coverage.py`, `callgraph.py`, `stanze.py`, `rst20.py`, `build_rom.sh`,
  `disasm.sh`.
- MAME `/usr/games/mame` 0.264 (`PATH=$PATH:/usr/games`), sempre
  `-video none -sound none` e `SDL_VIDEODRIVER=dummy` (già negli script);
  mai `-window` né `-video soft`; mai `-debugger imgui`; max 4 corse in
  parallelo; tutto in locale.
- Tracce cancellate dopo l'elaborazione (90 MB ogni ~700 frame; con la
  traccia attiva MAME fa ~6 frame/s: finestre corte, mai tracciare la
  sessione intera). ROM e chip estratti mai nel repo.
- Commenti, messaggi e documenti in italiano; nessun file di codice sopra
  le 800 righe.

## Cosa esiste già (fatti verificati)

### Infrastruttura (misurata oggi)

- `sessione.lua` (151 righe): variabili `SESS_NOME`, `SESS_ROOM` (tap in
  lettura su c243 fra i frame 1500 e 1950), `SESS_TRACE_DA`/`SESS_TRACE_A`
  (finestre multiple separate da virgola), `SESS_INPUT`
  (nessuno|casuale|file.lua), `SESS_SEED`, `SESS_CASUALE_DA`, `SESS_MONETA`;
  moneta/start ai frame 1510-1580; stampa `FASE <frame> <codici>` a ogni
  cambio di c000-c03f; scrive `out/<nome>_e603.txt`. Tap in variabile
  globale, `mem:read_u8`/`write_u8` su `cpu.spaces["program"]`.
- `sessioni.sh` (46 righe): funzione `sessione <nome>` = MAME con
  `-debug -debugger none -autoboot_script sessione.lua`, poi
  `coverage.py --traccia out/<nome>_trace.txt --prefisso <nome> --cancella`
  e `copertura_call.py`. Contiene ancora la nota «bonus e finale fuori
  perimetro» da togliere.
- `cattura.sh` (43 righe) + `snap.lua`: stati `titolo|gioco|tardo`, prova i
  frame di partenza in ordine; `SNAP_COND` dall'ambiente; scrive
  `out/snap_<ENTRY>_<tag>_{prima.bin,dopo.bin,regs.txt}`; scarta le
  occorrenze attraversate dalla NMI.
- `copertura_call.py` (79 righe): su `out/*_eseguiti.txt` stampa la riga
  per file e `unione: N su 299 (P%)`; scrive `out/eseguiti_unione.txt` e
  `out/call_mai_osservate.txt` (destinazione + prima call statica);
  ignora il file unione in ingresso.
- Misurato oggi: `python3 copertura_call.py out/*_eseguiti.txt | tail -1`
  → `unione: 195 su 299 (65.2%)`; 55 file `*_eseguiti.txt` (unione esclusa);
  `out/call_mai_osservate.txt` = 104 destinazioni (67 < 0x8000, 37 ≥ 0x8000).
- Banco di regressione (misurato nel brief 15, invariato qui):
  `make -C harness` 0 warning/error; `./harness --tutte` → 112 RIEPILOGO,
  0 FALLITO; `./sched_proto ../out/sched_log.txt | tail -1` →
  `spedizioni confrontate: 106095, differenze: 0; posta_attivita: 131, slot coincidenti: 131`.

### Meccanica dell'avanzamento di stanza (da dasm e tracce, verificata oggi)

- `c243` = stanza corrente (0-based, 0-51). Scrittori: 404B (avanzamento),
  7630 (demo, da tabella 765B), 78FE/7CD0 (=0, avvio), 7A11/7B46 (=0x32,
  schermate attract).
- **34FA** (passo 1 dell'attività 07, eseguito a ogni carico di quadro): se
  bit 0 di c24e = 0 copia 4 byte da **8DD4 + 4·c243** in c24a-c24d
  (c24a = indice della voce scelta, c24b-c24d = 3 stanze candidate); poi
  scorre la lista delle stanze visitate (c251…, chiusa da FF) e sostituisce
  con FF le candidate già viste; se la voce scelta è FF prende la prima
  non-FF.
- **Tabella ROM 8DD4** (52 record da 4 byte, letta da `out/main48.bin` a
  0x8DD4): percorso normale lineare; **candidate col bit 7 = stanze
  nascoste (bonus)**: 0x85 (stanza 5, voce 0 del record 4), 0x8F (15, rec.
  14), 0x96 (22, rec. 21), 0x99 (25, rec. 24, voce 0), 0xA3 (35, rec. 34),
  0xAA (42, rec. 41), 0xAB (43, voce normale del rec. 42!); **finale**:
  record 49 = `02 ff ff 32` → candidata 0x32. Record 50 = `02 00 ff ff`,
  51 = `01 01 00 ff`.
- **4000-405E** (mai eseguito): posta 14; se `c045 & 6 == 0` posta 17 ed
  esce; altrimenti E = M(c24b + c24a); accoda c243 alla lista c251; se
  E = 0x32 → C = 0x24 e `set 7,(c24e)`; se bit 7 di E → `set 1,(c24e)` e
  `inc (c24f)`, altrimenti `inc (c246)`; `c243 = E & 0x7f`; posta C (07
  normale, **24 = finale**); azzera c299 e c19d; c248 = FF.
- **Attività 1C = sequenza della porta** (ingresso 3F40, mai eseguito):
  spedidore a passi su c19d, tabella inline a 3F46 (9 voci): 40E1 41A8
  40A7 405F 3F58 41A8 3F93 41B2 3FF7; **3FF7 prosegue in 4000**. Unica
  posta statica: 133A. Attività 1D (3F39): `call 349F` poi diventa codice 05.
- **Attività 24 = finale** (ingresso 2C72, mai eseguito): spedidore a passi
  su c19d, tabella inline a 2C78 (da `out/rst20.txt`, sito 2C77, 19 voci):
  3861 2DEE 37D7 358F 3225 3A7D 2E89 **2C9E** 2E5D 2DCE 2E9C 3709 3829
  321E 31C5 2EF5 2E6D 2F1A 2E1E — gli stessi passi del carico quadro
  (sito 2CD2) più 2C9E mai eseguito. La catena del finale posta altro:
  3DC9 posta 14, 3DCE posta 25 (3B77), 3E47 posta 26 (3B63) — blocco
  3Cxx-3Exx mai eseguito (3CC1, 3CC2, 3D20, 3D2F, 3D43, 3DA6, 3DEA fra le 104).
- **Oggetti speciali**: 12AB (eseguita solo in stanza_06) riceve in A il
  valore di cella: ≥ 8 → accoda (A, E) nella coda c05b (slot da 2 byte,
  scrittore 12BD-12CF) e posta 22 (ingresso 12D6, che legge c05b);
  5-7 → 1340 (posta 18); = 2 (porta aperta) → 1332 → 133A posta 1C.
  4A87 (mai eseguito): per oggetti 8-0F ruota c24a sulle voci non-FF di
  c24b-c24d → instradamento alle stanze nascoste.
- **Attività mai spedite** (ingresso assente da `out/eseguiti_unione.txt`):
  0F→640E, 12→426A, 16→6496 (attract interrotta nel brief 04), 19→3792,
  1A→411D, 1B→4111, 1C→3F40, 1D→3F39, 24→2C72, 25→3B77, 26→3B63, 27→62B6.
  Poste statiche trovate: 05 a 140D, 12 a 0306, 13 a 03DB, 18 a 1345,
  19 a 13A1/13B0, 16 a 666C. Passo 16 (2D9D) dell'attività 07 mai eseguito
  (attende c19e = 0x0A e `jp 416B`).
- **Attenzione ai falsi positivi in 8xxx-Axxx**: 80B3 è sorgente dati di
  una `ldir` (sub_3569 con A=6, record a 3574), quindi la «call 4A05 a
  80BB» è dato disassemblato come codice; idem probabile per parte delle
  37 destinazioni ≥ 0x8000 (es. 3504 ← A1CD cade a metà di `ld a,(c243)`).
  La classificazione fa parte del lotto.
- c19d/c19e sono il passo/sotto-passo condiviso dai spedidori a passi:
  vanno azzerati prima di postare 1C o 24 a mano.

## Struttura da realizzare

Tutto in `/home/rufio72/OpenSolomonsKey-master/decomp/`. Prima di ogni
altra cosa: `cp out/call_mai_osservate.txt out/call_mai_prima_lotto12.txt`.

1. **`sessione.lua`**: due variabili nuove, documentate in testa:
   `SESS_POKE` = elenco `frame:addr:val` separati da virgola (esadecimali
   con 0x ammessi), eseguiti con `mem:write_u8` al frame detto, stampa
   `POKE <frame> <addr> <val>`; `SESS_POSTA` = elenco `frame:codice`:
   al frame detto scrive il codice nel primo slot libero (= 0) di
   c000-c03f e stampa `POSTA <frame> <slot> <codice>`. Resta sotto le 200 righe.
2. **`snap.lua`**: stesse variabili (`SNAP_POKE`, `SNAP_POSTA`, stessa
   sintassi), così le catture future partono dagli stessi inneschi.
3. **`cattura.sh`**: stati nuovi `porta`, `bonus`, `finale` che impostano
   gli inneschi predefiniti dei punti 5a-5c (POKE/POSTA ai frame 2250/2300,
   `INGRESSI=traccia`, `FRAMES="2400 2600 3000"`).
4. **`sessioni.sh`**: togliere la nota «bonus e finale fuori perimetro»;
   aggiungere in coda il blocco `lotto12` con le sessioni del punto 5;
   con argomento `lotto12` esegue solo quel blocco (senza argomento:
   tutto, comportamento storico).
5. **Sessioni nuove** (ognuna chiude con coverage.py come le esistenti;
   input `casuale` da 1600 dove non detto; finestre di traccia da adattare
   se le stampe FASE/POSTA mostrano che la sequenza prosegue oltre):
   - a) `porta`: partita normale (stanza 0), `SESS_POKE=2250:0xc19d:0,2250:0xc19e:0`,
     `SESS_POSTA=2300:0x1c`, traccia 2250-3300. Attesi: 3F40, i passi
     3F46, 4000-405E, avanzamento alla stanza 1.
   - b) `bonus_04` (`SESS_ROOM=4`, in più POKE `2250:0xc24a:0`) e
     `bonus_42` (`SESS_ROOM=42`, senza poke su c24a): come `porta`;
     attesi: ramo 403D (set 1,c24e; inc c24f) e carico della stanza
     nascosta (5 e 43).
   - c) `finale`: partita normale, POKE
     `2250:0xc24b:0x32,2250:0xc24c:0x32,2250:0xc24d:0x32,2250:0xc19d:0,2250:0xc19e:0`,
     POSTA `2300:0x1c`, traccia 2250-4500: il codice vero a 4032 posta da
     solo la 24. Se al 4500 la FASE mostra ancora codici nuovi, seconda
     corsa `finale2` con finestra spostata in avanti. Ripiego se la
     sequenza si pianta (c19d fermo): POSTA diretta di 0x24 con POKE
     `c243=0x32` e c19d/c19e=0.
   - d) `oggetti`: partita normale, POKE `2250:0xc05b:0x08,2250:0xc05c:0,2250:0xc05d:0`
     e POSTA `2300:0x22`; ripetere nella stessa corsa a +400 frame con
     codici 0x0B e 0x0E; traccia 2250-3300. Se 12D6 non consuma la coda
     (nessun PC nuovo), provare come parametro (c05c) l'indice 0-16 di uno
     slot oggetto.
   - e) `attract2`: la sessione `attract` del brief 04 completa (nessuna
     moneta, finestre 7400-8200 e 19400-19900): era stata interrotta;
     copre l'attività 16 (6496).
   - f) `att_XX`: per ogni codice fra 0F 12 13 19 1A 1B 1D 25 26 27 il cui
     ingresso resta fuori da `out/eseguiti_unione.txt` dopo a-e: corsa
     dedicata con POKE c19d/c19e=0 a 2250, POSTA del codice a 2300,
     traccia 2250-2800.
6. **`copertura_call.py`**: opzione `--classifica` che aggiunge a
   `out/call_mai_osservate.txt` una terza colonna `sito_eseguito` SI/NO
   (il PC della prima call statica appartiene all'unione): NO ⇒ probabile
   dato o codice raggiungibile solo da zone a loro volta mai eseguite.
7. **`CENSIMENTO-12.md`** (nuovo): una riga per ogni destinazione delle
   104 coperta dal lotto: ingresso, estensione osservata (min-max dei PC
   contigui da `out/<sessione>_istruzioni.txt`), invocazioni e chiamanti
   osservati (da `out/<sessione>_chiamate.txt`), sessioni che la toccano,
   note (porte hardware toccate, attraversata dalla NMI, spedidore a
   passi). In coda: le destinazioni ancora scoperte con la classificazione
   del punto 6. Questo file è la base dei lotti di traduzione futuri.
8. **`DATI.md`**: sezione nuova «Avanzamento di stanza e stanze nascoste»
   con la tabella 8DD4 (formato e le 7 nascoste + finale), c24a-c24d,
   i bit di c24e (0 visitata, 1 nascosta, 7 finale), c24f/c246, la lista
   c251, la coda c05b, le attività 1C/24 con le loro tabelle di passi.
9. **Due catture pilota** con gli stati nuovi di `cattura.sh`: due routine
   nuove a scelta fra quelle censite con ≥ 2 occorrenze non attraversate
   dalla NMI (candidate: i passi corti della catena porta/finale); basta
   una occorrenza per routine (`N=1`): provano il metodo, le doppie
   catture restano ai lotti di traduzione.

## Verifiche obbligatorie (da `decomp/`)

1. `wc -l < out/call_mai_prima_lotto12.txt` → `105` (104 + intestazione;
   misurato oggi sul file corrente).
2. `sh sessioni.sh lotto12 > out/sessioni12.log 2>&1` termina senza
   `ERRORE:` nel log (attesa; durata stimata 30-60 minuti).
3. `python3 copertura_call.py $(ls out/*_eseguiti.txt | grep -v unione) | tail -1`
   → `unione: N su 299 (P%)` con N > 195 (base misurata oggi: 195);
   obiettivo N ≥ 240 (attesa, non misurata). Questo è il cancello di
   copertura nuovo, senza esclusioni.
4. `python3 - <<EOF` che conta quante destinazioni di
   `out/call_mai_prima_lotto12.txt` non sono più in
   `out/call_mai_osservate.txt` → stampa `coperte del lotto: K su 104`
   con K ≥ 60 (attesa, non misurata; la stima 25-35 routine riguarda le
   voci del censimento, più d'una destinazione per routine).
5. `grep -h '^FASE' out/sessioni12.log | grep -m1 ' 24'` → una riga
   (l'attività 24 compare nella tabella durante `finale`; attesa).
6. Per gli ingressi 3F40, 4000, 2C72: presenti in
   `out/eseguiti_unione.txt` (attesa; riportare quali altri fra 640E 426A
   6496 3792 411D 4111 3F39 3B77 3B63 62B6 12D6 restano fuori).
7. `ls out/*trace*.txt 2>/dev/null` → vuoto; `find .. -size +50M` → vuoto;
   `pgrep -c mame` → `0`.
8. `make -C harness 2>&1 | grep -ciE 'warning|error'` → `0` e
   `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `112` con
   `grep -c FALLITO` → `0` (regressione, misurata nel brief 15: il lotto
   non tocca `harness/`).
9. `ls out/snap_0x*_{porta,bonus,finale}*_regs.txt 2>/dev/null | wc -l` →
   ≥ 2 (catture pilota; attesa).
10. `wc -l *.lua *.py *.sh | awk '$1>800 && $2!="totale" && $2!="total"'`
    → vuoto.
11. `grep -n -e '-window' -e 'video soft' sessioni.sh cattura.sh *.lua` → vuoto.
12. `git -C .. status --short` → solo `decomp/` (sessione.lua, snap.lua,
    cattura.sh, sessioni.sh, copertura_call.py, CENSIMENTO-12.md, DATI.md,
    file nuovi in `decomp/out/`) e questo brief.

## Consegna

- File modificati/nuovi del punto «Struttura»; in `out/`: i derivati delle
  sessioni nuove, `call_mai_prima_lotto12.txt`, il log `sessioni12.log`,
  le 2 catture pilota; nessuna traccia grezza.
- Nessun commit (lo fa la chat principale); se richiesto, messaggio in
  italiano con riga `Co-Authored-By: <modello> <noreply@anthropic.com>`.
- Rapporto finale: massimo trenta righe con l'uscita delle verifiche 1-12,
  copertura prima/dopo, quante delle 104 coperte e quante classificate
  «sito mai eseguito», numero di voci del censimento, attività ancora mai
  spedite, deviazioni e tempo MAME speso.

## Domande aperte

1. «scelta di progetto» Metodo: solo sessioni guidate con forzature
   (poste di attività nella tabella c000-c03f + poke mirati su c24b/c05b/
   c19d), nessuna partita umana tracciata. Pro: senza finestra,
   riproducibile frame per frame, minuti di macchina, riusabile da
   `cattura.sh` per le catture dei lotti futuri; si forza solo l'innesco e
   l'avanzamento corre sul codice vero (il finale lo posta 4032 da solo).
   Contro: stato parzialmente artificiale (una porta «attraversata» senza
   Dana sulla porta), possibile qualche sequenza che si pianta (previsti i
   ripieghi). La partita umana richiederebbe una finestra video, l'abilità
   di raggiungere la stanza 49 e le nascoste, e non sarebbe riproducibile.
   Proposta: forzature.
2. «scelta di progetto» Sessione `servizio` (schermate di test via DIP
   «Service Mode», coprirebbe 29CF/2A03 e forse le poste 0306/03DB): è
   fuori dal perimetro bonus/finale ma costa una corsa. Proposta:
   includerla solo se il campo esiste in `:DSW1`/`:DSW2` e le altre
   sessioni lasciano scoperti 0CFD/0E0B/426A/6480; altrimenti rinviarla al
   filone «zone mai eseguite».
3. «preferenza» Nome e sede del censimento: `decomp/CENSIMENTO-12.md`
   (non in ROUTINE.md, che resta il registro delle traduzioni).
4. «preferenza» Cancelli numerici (N ≥ 240 su 299 e K ≥ 60 su 104): attese
   non misurate; i valori veri misurati vanno nel rapporto e diventano la
   base dei lotti successivi.
5. «preferenza» Catture pilota: scelta delle due routine lasciata allo
   sviluppatore fra le censite con occorrenze pulite (fuori NMI).

## Risoluzioni (2026-10-21)

1. **Risolta con l'utente: solo forzature.** Nessuna partita umana
   tracciata; sessioni guidate con forzature di RAM come proposto.
2. **Risolta con l'utente: inclusione condizionata.** La sessione
   `servizio` si fa solo se il campo esiste in `:DSW1`/`:DSW2` e le altre
   sessioni lasciano scoperti 0CFD/0E0B/426A/6480; altrimenti è rinviata
   al filone «zone mai eseguite».
3. **Preferenza accettata:** censimento in `decomp/CENSIMENTO-12.md`.
4. **Preferenza accettata:** cancelli N ≥ 240 e K ≥ 60 come attese non
   misurate; i valori veri misurati vanno nel rapporto.
5. **Preferenza accettata:** le due catture pilota le sceglie lo
   sviluppatore fra le censite con occorrenze pulite (fuori NMI).

## Esito (parziale)

Ciclo interrotto dall'utente il 2026-10-21 in serata, da riprendere:
l'analista ha finito (Fable, ~124k token, brief buono al primo colpo),
lo sviluppatore (Opus) è stato fermato a metà lavoro. Nel commit di
salvataggio ci sono le sue modifiche parziali e NON verificate a
`decomp/` (DATI.md, cattura.sh, copertura_call.py, sessione.lua,
sessioni.sh, snap.lua); `decomp/CENSIMENTO-12.md` non è ancora stato
creato. Alla ripresa: rilanciare lo sviluppatore sul brief, che riparte
dalle modifiche presenti; poi verificatore sulle 12 verifiche.
