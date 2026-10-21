# Compito 12: filone porte — censimento e proposte di disegno (lotto 8, analisi)

Data: 2026-10-21. Solo analisi: nessuna modifica a codice C, script esistenti o
ROUTINE.md. Schema e convenzioni: brief 05-11. Dati derivati rigenerati oggi su
VM nuova con gli script del repo (ROM estratte in `roms/ext/solomon/`, md5 di
`main48.bin`/`main64.bin` `daaf50c3…`/`0bca296a…` come nel brief 07; z80dasm
1.1.6; MAME 0.264; `main48.dasm` 35732 righe; traccia di controllo 1900 frame;
sessioni `titolo`/`stanza_00`/`gameover`). Censimento riproducibile con il
nuovo `decomp/censimento_porte.py` (uscita attesa in fondo).

## Fatti di base (verificati oggi)

- Il Z80 principale **non ha spazio di I/O**: in `solomon.cpp` (MAME 0.264) la
  maincpu ha solo `AS_PROGRAM`, nessuna io map. Le 11 istruzioni `in`/`out`
  presenti nel disassembly statico cadono tutte in zone classificate «dati»
  da `copertura.txt` e **nessuna è mai eseguita** nelle tracce: sono
  spazzatura da disassemblaggio di tabelle. Il filone porte riguarda solo
  l'hardware **mappato in memoria**.
- Mappa hardware della maincpu da `solomon.cpp` (letture / scritture):
  e600 P1 / maschera NMI (`nmi_mask_w`, bit 0); e601 P2 / —; e602 SYSTEM
  (start 1-2 bit 0-1, coin 1-2 bit 2-3) / —; e603 protezione (`_0xe603_r`,
  valore secondo il PC) / —; e604 DSW1 / ribaltamento schermo
  (`flipscreen_w`, bit 0); e605 DSW2 / —; e606 lettura nop («watchdog?»,
  torna 0) / —; e800 — / latch audio (`sh_command_w`). **e646, e802, e803
  non sono mappati**: letture 0, scritture senza effetto.
- Campi DIP (da `solomon.cpp`): DSW1 bit 0 Demo Sounds, bit 1 Cabinet,
  bit 2-3 Lives, bit 4-5 Coin B, bit 6-7 Coin A; DSW2 bit 0-1 Difficulty,
  bit 2-3 Timer Speed, bit 4 Extra, bit 5-7 Bonus Life.
- **I dump di `snap.lua` contengono già i valori vivi delle porte**: la
  scarica di c000-ffff passa dagli handler MAME, quindi negli `snap_*.bin`
  esistenti gli offset di e600-e605 valgono P1, P2, SYSTEM, 0, DSW1, DSW2
  all'istante della cattura. Verificato oggi sui file in git: e604 = 02
  (DSW1 predefinito, Cabinet=Upright) in tutte le catture; e600 = 04 (P1 Up
  premuto dagli input casuali) in `snap_0x0AC3_gioco_dopo.bin`. e603 nel
  dump vale 0 (il PC del dump non è fra quelli gestiti), e606-e7ff e e800+
  valgono 0.
- Protezione e603 già coperta: tap delle sessioni di oggi → un solo PC di
  lettura (0161, NMI, valore 00; file `out/<sessione>_e603.txt`), gestito
  dalla tabella per-PC di `harness/porte.c`. In MAME il PC 4CF0 (sito 4CED,
  mai eseguito nelle tracce) torna `BC & 0x08`: dipende da un registro, la
  tabella per-PC da sola non basterà.

## Censimento completo (da `censimento_porte.py`, tracce di oggi)

34 siti reali di accesso (31 con operando diretto + 3 via HL), più 1 falso
(4DA9, `ld (0e646h),a` dentro la zona dati 4da7-4dac) e 11 `in`/`out`
letterali mai eseguiti. Frequenze t/s/g = esecuzioni nelle sessioni
titolo / stanza_00 / gameover. «Routine» = routine contenitrice e suo stato
in ROUTINE.md (74 righe; tutte le contenitrici sono non tradotte salvo dove
indicato).

| sito | porta | dir | routine (stato) | semantica dedotta | t / s / g |
|---|---|---|---|---|---|
| 0001 | e600 | W | reset 0000 (strutturale) | A=0: maschera NMI spenta al reset | 1 / 0 / 0 |
| 004A | e604 | R | 0043 (non tradotta, 12 B) | filtro suoni in attract: se c045&6=0 e DSW1 bit 0 (Demo Sounds Off) esce con carry; chiamata dal gestore `rst 28` (002B) | 5 / 2 / 0 |
| 00B9 | e600-e602 | R (ldir) | ciclo di servizio 00C4 (strutturale) | copia P1, P2, SYSTEM in c050-c052 una volta per giro: è la lettura ingressi del gioco | 1349 / 700 / 700 |
| 015E | e603 | R | NMI 0066 (filone NMI) | protezione: bit 0 scarta un ramo della NMI (PC tap 0161 → 00) | 1435 / 700 / 700 |
| 019A | e606 | R | NMI 0066 | lettura a vuoto (A sovrascritto a 01A0): probabile watchdog | 1435 / 700 / 700 |
| 01A4 | e800 | W | NMI 0066 | spedisce al latch audio la testa della coda suoni c054-c057 e la fa scorrere | 5 / 27 / 3 |
| 01BB/01BF | e600 | W | NMI 0066 | impulso 0→1 sulla maschera NMI a fine interrupt (riarmo) | 1435 / 700 / 700 ciascuno |
| 285D | e600 | W | avvio 2833-295B (jp-chain, strutturale) | A=1: abilita la NMI dopo l'azzeramento RAM | 1 / 0 / 0 |
| 28E3 | e600 | W | avvio (ramo retry test) | A=0: NMI spenta prima di ripetere il test | mai eseguito |
| 28F1 | e604 | W | avvio | ribaltamento schermo = valore tornato da sub_3678 (tradotta) | 1 / 0 / 0 |
| 294C | e803 | W | avvio | scrive B7 su indirizzo non mappato (no-op; probabile resto del protocollo di protezione) | 1 / 0 / 0 |
| 298A | e604 | R | 2980 (non tradotta, 71 B) | estrae i campi monete (bit 4-7 di DSW1) nelle celle credito | 2 / 0 / 0 |
| 2ACB | e800 | W | corpo di 2A03 (blocco l2ac8h) | comando audio 3D | mai eseguito |
| 2AE2/2AEB | e604/e605 | R | 2AE2 (non tradotta, 19 B) | disegna gli 8 bit di DSW1 e DSW2 in d765/d785 (riga video di servizio), via sub_2af5 già verificata | 1525 / 0 / 0 ciascuno |
| 2BC3 | e600-e602 | R via HL | 2BC3 (non tradotta, 6 B + coda l2353h ~43 B) | lettura ingressi di avvio/attract: OR di P1 e P2, maschere, AND con SYSTEM | 71135 / 0 / 0 |
| 2C08 | e602 | R | 2BF3 (non tradotta, 35 B) | lettura a vuoto dentro il test RAM (A subito sovrascritto) | 66304 / 0 / 0 |
| 2C1F | e602 | R | 2C1A (non tradotta, 98 B) | come 2C08 | 70400 / 0 / 0 |
| 2EC6 | e605 | R | 2E90 (non tradotta, 199 B) | DSW2 & 3 = Difficulty, indicizza le tabelle 8b3d/8d11 | 2 / 2 / 0 |
| 322E | e802 | W | 322A (non tradotta, 52 B) | azzera un registro non mappato (no-op) | 2 / 2 / 1 |
| 33BB | e802 | W | **33BA (verificata)** | come 322E; nel C già `M(0xe802) = 0` (precedente per le scritture innocue) | 4 / 4 / 1 |
| 342D | e605 | R | 342D (non tradotta, 18 B) | bit 5-7 di DSW2 = Bonus Life, indicizza la tabella 343F | 1 / 2 / 0 |
| 35F6 | e605 | R | 3569 (non tradotta, 224 B) | bit 2-3 di DSW2 = Timer Speed | 1 / 1 / 1 |
| 38D0/38DC | e604 | R+W | 3812 (non tradotta, 347 B) | legge Cabinet (bit 1) e scrive il ribaltamento schermo per il 2° giocatore | 1 / 1 / 1 ciascuno |
| 42B3 | e604 | R via HL | corpo di 3F88 (zona 3f93-4397 mai eseguita) | bit 5 di DSW1 (Coin B) via `bit 5,(hl)` | mai eseguito |
| 4CED | e603 | R | corpo di 4C11 (zona 4b20-4d04 mai eseguita) | protezione a metà partita: PC 4CF0 → MAME torna BC&08, confronto con la copia c1b9 | mai eseguito |
| 4CF8 | e803 | W | corpo di 4C11 | aggiorna il valore di protezione (non mappato, no-op) e la copia c1b9 | mai eseguito |
| 4DA9 | e646 | W | **falso**: zona dati 4da7-4dac | byte di tabella disassemblati come codice | mai eseguito |
| 62BE | e603 | R | zona mai eseguita dopo 5C78 (tabella rst 20 a 62BA) | protezione, PC 62C1 non gestito da MAME (torna 0 con avviso) | mai eseguito |
| 760C | e604 | W | 75AA (non tradotta, 251 B) | A=0: ribaltamento spento | 1 / 0 / 0 |
| 7CB0 | e604 | W | 7C84 (non tradotta, 132 B) | A=0: ribaltamento spento | 0 / 1 / 0 |
| 7CE4 | e605 | R | 7C84 | bit 5-7 di DSW2 = Bonus Life, tabella via rst 20 | 0 / 1 / 0 |

Riepilogo: 27 siti eseguiti nelle tracce, 6 reali mai eseguiti (28E3, 2ACB,
42B3, 4CED, 4CF8, 62BE), 1 falso. Porte lette: e600-e606; scritte: e600,
e604, e800 più i non mappati e802/e803. Nessuna routine legge due volte la
stessa porta aspettandosi valori diversi al proprio interno: i poll stanno
nei chiamanti (es. il ciclo a 29EB che richiama 2BC3).

## Grappoli che si sbloccherebbero

Con le porte rappresentate nell'harness diventano traducibili (byte =
estensione eseguita dal `callgraph.csv` di oggi):

| routine porte | byte | chiamanti (siti) | sblocca |
|---|---:|---|---|
| 3569 | 224 | 3559 (in 354B), 3F8A (in 3F88), 664E | 354B (18 B), 3F88 (11 B) e, a cascata, 2F57 (29 B, chiama 3F88 da 2F5A) e parte di 4767 (115 B) |
| 3812 | 347 | 37F2/37F7 (in 37B4) | 37B4 (86 B) |
| 2E90 | 199 | 2E8B (in 2E11), 64CE | 2E11 (107 B) e il chiamante in 64xx |
| 342D | 18 | 340D (in 33EB) | 33EB (60 B) |
| 2AE2 | 19 | 2AAA | quasi solo colla su sub_2af5 già verificata |
| 7C84 | 132 | 761A, 7C4B, 7C9B | con 75AA, la zona 76xx-7Cxx (fra cui 797E, 117 B) |
| 75AA | 251 | 6655, 761D, 78E4, 7C4E, 7C98 | id. (ha anche rst 20) |
| 0043 | 12 | 002B (gestore rst 28) | il gestore rst 28 (accodamento suoni, filone strutturale) |
| 2980, 2BC3+2353, 2BC9, 2BF3, 2C1A | 71+49+42+35+98 | flusso d'avvio 2833-295B | test RAM e lettura ingressi d'avvio (filone strutturale) |
| NMI 0066 (015E, 019A, 01A4, 01BB, 01BF) | — | — | filone NMI |

Totale indicativo: ~1,5 KB di routine porte + ~0,5 KB di candidate oggi
bloccate, oltre a togliere il prerequisito porte ai filoni NMI e
strutturale. Restano fuori per altre ragioni 2A03 e 322A (`ex (sp),hl`).

## Proposte di disegno

**A. Funzioni di piattaforma sul dump esistente (raccomandata).**
In `porte.c`: `uint8_t leggi_porta(uint16_t a)` che per e600-e606 ritorna
`ram[a - 0xc000]` (il valore vivo già catturato, vedi fatti di base; e603
resta su `leggi_e603`); `void scrivi_porta(uint16_t a, uint8_t v)` che
scrive in `ram[]` (zona fuori confronto, visibile al debug), come già fa
`sub_33ba` con e802.
- Pro: fedeltà piena per le letture (valore esatto all'istante
  dell'ingresso; la CPU è ferma al breakpoint, il dump è atomico); zero
  modifiche a `cattura.sh`/`snap.lua`; tutte le catture esistenti e la
  procedura dei lotti restano valide; lavoro: poche ore.
- Contro: le scritture non hanno oracolo (e600/e604/e800 non ricompaiono
  nei dump: in lettura quegli indirizzi mostrano gli ingressi); una
  futura routine che rilegge una porta aspettandosi un cambiamento non
  sarebbe riproducibile (oggi non ne esistono, vedi censimento).
- Rischio principale già smontato: il contenuto dei dump alle porte è
  stato verificato sui file in git (e604=02 ovunque, e600=04 dove l'input
  era premuto).

**B. Stub con valori fissi e nota.**
`leggi_porta` ritorna costanti (P1/P2/SYSTEM = 0, DSW1 = 02, DSW2 = 00,
e606 = 0); scritture no-op.
- Pro: lavoro minimo, nessuna modifica a script o formato.
- Contro: fedeltà bassa; le catture `gioco` con input casuale premuto
  (osservate) fallirebbero il confronto dei registri; i DIP vanno tenuti
  allineati a mano alla configurazione MAME; peggio, una cattura può
  passare «per caso» con il valore sbagliato. Sconsigliata: costa quanto
  si risparmia.

**C. Estensione del formato di cattura (log degli accessi).**
`snap.lua` installa tap di lettura e scrittura su e600-e8ff fra l'ingresso
e il rientro e scrive `snap_<ENTRY>_<tag>_porte.txt` (sequenza: n, PC,
indirizzo, R/W, valore; i tap vanno rimossi prima della scarica finale per
non registrare il dump stesso). L'harness, se il file esiste, fa consumare
a `leggi_porta` la sequenza in ordine (verificando indirizzo e PC) e fa
confrontare a `scrivi_porta` indirizzo e valore attesi: anche le scritture
— incluso il latch audio e800 — entrano nell'oracolo.
- Pro: fedeltà massima (riletture con valori diversi, ordine e valore
  delle scritture); catture vecchie valide (log assente → comportamento A).
- Contro: modifiche a `snap.lua` e all'harness (parser, stato), un file
  per cattura in più in git; interazione tap/NMI da provare; lavoro: una
  giornata buona. `cattura.sh` resta invariato.

Raccomandazione: **A subito per il lotto 9** (sblocca i grappoli
DIP/ribaltamento con il minimo di meccanica nuova), **C come estensione**
quando si arriverà alla NMI o all'audio, dove l'oracolo delle scritture
su e800 vale il costo. B solo come ripiego se A rivelasse sorprese.

## Verifiche future (uscita attesa, misurate oggi)

1. `python3 censimento_porte.py | head -1` →
   `# Accessi diretti (operando e6xx/e8xx): 31`.
2. `python3 censimento_porte.py | grep -c '^[0-9A-F]'` → `45`
   (31 diretti + 3 indiretti + 11 `in`/`out` letterali).
3. `python3 censimento_porte.py | grep 'letterali'` → `: 11`, e ogni riga
   seguente con `esecuzioni totali=0`.
4. `cat out/*_e603.txt` → solo `0161 00 <n>` (un PC di lettura).
5. `od -A none -t x1 -j 0x2604 -N 1 out/snap_0x2368_titolo_prima.bin` →
   `02` (DSW1 nel dump).
6. `make -C harness 2>&1 | grep -ciE 'warning|error'` → `0`;
   `(cd harness && ./harness --tutte | grep -c '^RIEPILOGO')` → `74`,
   `grep -c FALLITO` → `0` (nessuna modifica al codice in questo lotto).
7. `git status --short` → solo `compiti/12-filone-porte.md` e
   `decomp/censimento_porte.py`.

## Domande aperte

1. API: una coppia `leggi_porta(indirizzo)`/`scrivi_porta(indirizzo,
   valore)` in `porte.c`, con gli indirizzi hardware espliciti nel C
   tradotto (mai `M()` sulle porte, che resta per la RAM), e commento con
   il significato MAME della porta a ogni sito. Risolta con la proposta
   dell'analista. «preferenza»
2. Scritture su indirizzi non mappati (e802, e803): nel nuovo C passano da
   `scrivi_porta` come le altre; `sub_33ba` (verificata) resta com'è con
   `M(0xe802) = 0` finché un lotto di pulizia non uniformerà. Risolta in
   modo conservativo (nessuna modifica al C esistente). «preferenza»
3. I 6 siti reali mai eseguiti (28E3, 2ACB, 42B3, 4CED, 4CF8, 62BE)
   restano al filone «zone mai eseguite con sessioni guidate»; nessun
   lavoro ora. Risolta in modo conservativo. «preferenza»
4. **Quale proposta adottare per il lotto 9** (A, B, C o A+C):
   raccomandazione A, con C rinviata a quando serviranno NMI/audio.
   «scelta di progetto»
5. **Se dare un oracolo alle scritture su porte già nel lotto 9** (cioè
   anticipare la parte W di C): raccomandazione no — nel lotto 9 le
   scritture eseguite in routine traducibili sono solo i ribaltamenti
   e604 (38DC, 760C, 7CB0) e il no-op e802; il log scritture rende di
   più sul filone NMI (e800). «scelta di progetto»
6. **Protezione e603 al PC 4CF0** (valore = BC&08, la tabella per-PC non
   basta): raccomandazione rinviare al filone zone mai eseguite ed
   estendere allora `leggi_e603` con il parametro BC, documentando il
   comportamento MAME. «scelta di progetto»

## Decisioni (chat principale con l'utente, 2026-10-21)

- Domande 1-3 (preferenze): accettate con la proposta dell'analista.
- Domanda 4 (scelta di progetto): adottata la proposta A — funzioni di
  piattaforma `leggi_porta`/`scrivi_porta` in `porte.c` sui dump esistenti.
  La proposta C (log degli accessi) resta come estensione futura per NMI
  strutturale e audio; la B è scartata.
- e603 / PC 4CF0: rinviata al filone «zone mai eseguite», come raccomandato.

## Chiusura ciclo di analisi (chat principale, 2026-10-21)

Esito: PASSATO. Sola analisi in sessione cloud (branch lotto-08-analisi,
PR #2), nessun codice toccato. Verifiche del censimento rifatte in locale:
31 accessi diretti, 45 righe totali, 11 in/out letterali mai eseguiti,
DSW1=02 nel dump di 2368 — tutte conformi. Il lotto 9 implementerà la
proposta A.
Token: analisi in cloud su Fable 5 (crediti cloud); verifica locale diretta.
