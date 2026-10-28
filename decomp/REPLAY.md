# Replay: la partita di MAME contro il giro di gioco in C (lotto 23)

Passo 4 del metodo: una partita registrata in MAME con l'impronta della RAM a
ogni NMI (oracolo), il giro di gioco in C (NMI + spedidore + corpi delle
attività) alimentato dagli stessi ingressi, il confronto frame per frame.
Brief: `compiti/23-lotto-replay.md`.

**Esito di questo lotto: il replay si ferma alla prima spedizione, perché il
corpo dell'attività 0A non è tradotto.** L'infrastruttura (oracolo, giro,
confronto, diff per zona) funziona ed è riproducibile; dei 10 corpi della
finestra ne sono tradotti 7. I tre che mancano (01, 0A, 10) sono molto più
grandi di quanto stimato nel brief (sotto, «Perimetro misurato»).

## Strumenti

| file | ruolo |
|---|---|
| `replay.lua`, `replay.sh` | oracolo: `sh replay.sh` da `decomp/` (circa 90 s a 5500 frame) |
| `harness/replay` (`replay.c`, `attivita.c`, `comune.c`) | giro in C e confronto |
| `replay_diff.py` | byte diversi fra due dump, raggruppati per zona |

Prodotti in `out/` (gitignorati): `replay_oracolo.txt` (righe
`F <f> <e600> <e601> <e602> <ritorno> <hash>`, 162 KB), `replay_disp.txt`
(righe D/W/I di `sched_log.lua` più `N <f> <ritorno> <letture>`, 1,4 MB),
`replay_<f>.bin` (dump c000-ffff a F0 e ai frame di `REPLAY_DUMPS`).

Uso del giro, da `decomp/`:

    harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410 [fine] [--oltre]
    python3 replay_diff.py out/replay_<f>.bin <altro dump>

Codice d'uscita di `replay`: 0 se il giro arriva in fondo senza divergenze
nello spedidore; 1 se si ferma (corpo non tradotto, spedizione diversa,
divergenza) o se l'impronta a F0 non coincide; 2 per errori di ingresso.
Oggi esce sempre con 1, voluto, perché si ferma al corpo 0A.

## Parametri della registrazione

- Ingressi convenzionali (moneta 1510-1530, start 1560-1580, dal 1900 un
  tasto casuale di P1 ogni 8 frame con `math.randomseed(1)`), fine al 5500.
- MAME 0.264 con `-debug -debugger none -video none -sound none -nothrottle`;
  punto di interruzione a 0066 gestito da `emu.register_periodic`.
- **Contatore dei frame.** A debugger fermo MAME continua a ridisegnare e
  chiama `frame_done` senza emulare, un numero di volte che dipende dal tempo
  reale. Contare le chiamate (come `sched_log.lua`) dava una NMI ogni due
  «frame» e due registrazioni diverse fra loro dal frame 382. Anche contare
  solo le chiamate con il tempo emulato avanzato non basta. `replay.lua` usa
  il numero di frame dello schermo (`screen:frame_number() + 1`), che non si
  muove a debugger fermo.
- **Ingressi dell'host spenti.** `replay.sh` passa `-keyboardprovider none
  -mouseprovider none -joystickprovider none -lightgunprovider none`. Senza
  queste opzioni, un tasto o un joystick dell'host toccato mentre MAME gira
  entra nelle porte anche senza finestra. È successo in due registrazioni su
  cinque: e600 = 02 ai frame 1692-1724 in una, dal 3594 in un'altra, con
  impronte diverse da lì in poi. Con le opzioni, tre registrazioni di fila
  coincidono byte per byte (oracolo e `disp.txt`). Vale anche per
  `cattura.sh`, `sessioni.sh` e gli altri lanci di MAME del progetto, che
  oggi non spengono gli ingressi dell'host.
- **Confronto con `out/sched_log.txt`.** Le righe D/W/I coincidono una per
  una a meno del campo frame. Il campo frame è uguale fino al 64; dal reset
  del ~600 in poi il replay ha un frame in più (sched_log 601 = replay 602).
  Lo stesso evento cade quindi un frame emulato più tardi con `-debug`. La
  sequenza di spedizioni e scritture però è identica, quindi la partita è la
  stessa. Il frame 2402 del replay è il 2401 di `sched_log`.
- Esclusioni dell'impronta: c040-c043 e cf00-cfff. La sanità a F0 passa così
  (`SANITA F0 2402: hash C 25775EE2 MAME 25775EE2 coincide`, così anche a
  2410): non c'è stato bisogno di tararle.

## Numeri dell'oracolo

- 5434 NMI registrate (frame 66-5499), **una per frame** dal 66 in poi;
  106104 spedizioni in tutto.
- Finestra 2402-5500: 3098 NMI, 64880 spedizioni (01×37, 02×2174, 03×3252,
  0A×14839, 0D×14839, 0E×52, 10×9, 14×14839, 20×14839), stessi codici del
  brief.
- Dove entra la NMI (ritorno letto in cima alla pila allo stop su 0066): 1030
  volte su 3098 nello spedidore (3AF3-3B10). Di queste, 131 cadono fra la
  lettura che trova l'attività e il corpo (ritorno 3AFB-3B10 con 0 letture
  dopo la D). Le altre 2068 cadono **a metà di un corpo**.
- A F0 = 2402 la NMI è entrata a metà del corpo 0A (ritorno 4F06). Il dump di
  F0 contiene quindi un corpo eseguito a metà, che il C non può riprendere.
  Il primo frame utile con la NMI nello spedidore è il **2410** (ritorno
  3AFD): `replay` lo segnala e il giro va fatto partire da lì (dump con
  `REPLAY_DUMPS=2410`).

## Modello della NMI nel giro

Il brief prevedeva la NMI al confine di frame. Siccome due NMI su tre
entrano a metà di un corpo, a ogni frame ci sarebbe una differenza dovuta
solo all'ordine di esecuzione (interleaving). `replay.c` inietta invece la
NMI nel punto misurato da `replay.lua` (riga N):
- ritorno 3AFB-3B10 con 0 letture: dopo la lettura e prima del corpo
  (involucro dei corpi);
- nello spedidore con m letture: prima della lettura m della scansione
  (`sched_gancio_scansione`, come `sched_proto`);
- a metà di un corpo: dopo il corpo, all'inizio della scansione seguente.
  Qui la divergenza è attesa: si conta ma non ferma il giro.

L'impronta del C si confronta con la riga F prima di ogni NMI. Una
divergenza a una NMI entrata nello spedidore («pulita») ferma il giro e
stampa i primi 32 byte diversi, se c'è il dump di quel frame. Con
`--oltre` il giro non si ferma e a fine corsa dice se il C riconverge.
Ogni spedizione si confronta anche con la riga D (slot e codice).

## Esito del replay

    $ harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410
    SANITA F0 2410: hash C A350CF53 MAME A350CF53 coincide
    CORPO 0A (4DA2) non tradotto al frame 2410

Con F0 = 2402 il risultato è lo stesso, più l'avviso sulla NMI a metà
attività. Il giro non arriva alla seconda NMI: 0A va in onda a ogni frame
ed è la prima spedizione dopo F0. Non c'è quindi ancora una prima
divergenza da diagnosticare. Classificazione: **corpo mancante**.

### Dopo il lotto N1 (brief 25)

Con il corpo 0A in `harness/nemici*.c`:

    $ harness/replay out/replay_oracolo.txt out/replay_disp.txt out/replay_2410.bin 2410
    SANITA F0 2410: hash C A350CF53 MAME A350CF53 coincide
    CORPO 01 (48D1) non tradotto al frame 2589
    NMI confrontate: 180 (nello spedidore 68, a metà attività 112); divergenti: nello spedidore 0, a metà attività 83
    prima divergenza a metà attività (attesa): frame 2413
    nessuna divergenza nello spedidore

Con `--oltre` l'uscita è la stessa (exit 1 per il corpo mancante). Le 68
NMI pulite fra 2410 e 2589 coincidono tutte con MAME: lì l'impronta copre
tutta la RAM, quindi i corpi del lotto 23 e le parti di N1 che vanno in
onda sono confermati in quel tratto. Le 83 divergenze a metà attività sono
quelle attese dal modello (corpo non spezzabile) e riconvergono alla NMI
pulita seguente. Il giro si ferma ora alla prima spedizione di 01 (`D 2589
27 01`). Routine di N1 che in questo tratto non vanno in onda (misura con
gcov): 4592, 49AE, 4B20, 4C11, 2482, 248D, 24BE, 55C9 e i rami `rst 20`
di 4632; le prime sono in comune con 01 e 10 e si verificano con loro.

**Correzione fuori da N1 (interleaving della NMI in `replay.c`).** Al
frame 2447 la prima corsa divergeva in un solo byte, `C04F: C 0E MAME 0C`.
La NMI di quel frame è pendente (`N 2447 3AFB 0`, dopo la lettura che ha
trovato lo slot 0D e prima del corpo), ma lo spedidore scrive c04f solo a
3B0C, mentre nel C `spedisci_una` lo aveva già scritto prima di chiamare
l'involucro. Ora `replay.c` ricorda c04f prima della spedizione e, per una
NMI pendente con ritorno < 3B0F, la esegue con il valore vecchio e poi
rimette quello nuovo (lo stesso criterio che il punto di partenza usava
già per F0). Nessuna correzione nei corpi del lotto 23 né nel C di N1.

## Perimetro misurato dei corpi

Istruzioni eseguite (unione delle sessioni `out/*_eseguiti.txt`) che si
raggiungono dall'ingresso seguendo salti, `call` verso routine non
tradotte, `rst 20` (tabelle di `rst20_tables.h`) e `rst 38` (tabelle in
linea):

| corpi | istruzioni | byte | routine non tradotte chiamate |
|---|---:|---:|---|
| 02, 03, 0D, 0E, 11, 14, 20 (tradotti) | 285 | 559 | 4B98, 250E (tradotte qui, statiche in `attivita.c`) |
| 01 (48D1) + 10 (457B) | 503 | 872 | 2474, 2482, 248D, 24BE, 4592, 4938, 49AE, 4ADA, 4B20, 4C11 |
| 0A (4DA2) | ~1986 | ~3600 | 36 routine: 4F0F, 4FEF, 5115, 5124, 51D7, 51FA, 52FD, 531C, 5345, 54C5, 5D29-6252, 734D, 738F, 758E, 759F, ... |

Il brief dava 0A come «gate su c2b8, 5 B». In realtà 4DA6 è un `rst 38`,
cioè un salto in tabella indicizzato da E = c2b8, verso 4DAD / 4632 /
4EE6. Da lì altri sette `rst 38` (4F1B, 4FE0, 5127, 5207, 5E40, 6130,
728B, 73B6) portano al motore dei nemici. Il numero per 0A è una stima per
eccesso: la lunghezza delle tabelle `rst 38` non è nota e sono state prese
le prime 8 voci eseguite. Anche 01 e 10 chiamano 49AE/4938/4592, che il
blocco contiguo misurato nel brief non conta.

## Cose da sistemare, in ordine

1. **Corpo 0A** (motore dei nemici): ~3,6 KB di Z80 sotto 4DA2. Va
   censito con le tabelle `rst 38` vere e spezzato in lotti con moduli nuovi,
   perché nessun file può superare le 800 righe. Senza 0A il replay non va
   oltre la prima spedizione.
   **Censito** (brief 24): `CENSIMENTO-NEMICI.md` dà 21 tabelle `rst 38`,
   220 routine e 8833 B statici, più del perimetro stimato qui. Il piano dei
   lotti parte da N1 (corpo 0A più le 27 routine coperte dal replay, ~938
   righe) e distribuisce il lavoro su 11 moduli `harness/nemici*.c`.
   **N1 fatto** (brief 25): corpo 0A, 26 routine e `sub_3b62` in sette
   moduli `harness/nemici*.c`, 27 stub in `nemici_stub.c` che fermano il
   giro con `ROUTINE xxxx mancante`. Il giro supera il 2410 senza
   divergenze nello spedidore e si ferma al corpo 01 al frame 2589.
2. **Corpi 01 e 10**: ~870 byte, con 49AE/4938/4592/4B20/4C11/4ADA e le
   piccole 2474/2482/248D/24BE. Sono un lotto a sé, con un modulo
   `attivita_*.c` nuovo (`attivita.c` ha già 350 righe).
3. Poi rifare il giro da F0 = 2410 con `--oltre` e i dump vicini alla prima
   divergenza (`REPLAY_DUMPS`), e classificare con `replay_diff.py`.
4. I 7 corpi tradotti (02, 03, 0D, 0E, 11, 14, 20) non sono ancora
   verificati: la loro verifica è il giro stesso, appena 0A c'è.
