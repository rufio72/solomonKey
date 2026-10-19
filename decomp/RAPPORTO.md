# Sondaggio: decompilazione in C del Z80 principale di Solomon's Key

Data: 2026-10-19. Set MAME `solomon`, MAME 0.264. Tutto riproducibile con gli
script di questa cartella (vedi «Come rifare le misure» in fondo).

## Numeri (traccia di 5500 frame: avvio, demo, moneta, start, 3600 frame di input casuali)

```
righe di traccia: 42797751
byte eseguiti: 10183
byte mai eseguiti (su 53248 di ROM): 43065
PC distinti: 5618
destinazioni call/rst distinte (PC dopo un call/rst): 141
destinazioni call/rst distinte (operando, chiamata avvenuta): 134
ingressi NMI 0066: 5435
rst $20 eseguiti (tabelle inline): 123825
jp (hl) eseguiti: 0
jp (ix)/(iy) eseguiti: 0
PC >= c000: 0
tabelle dati stimate: 106
intervalli codice: 192
intervalli dati: 193
violazioni autoverifica lunghezza: 0
```

Sessione di controllo a 1900 frame (identica a quella del brief): 15.610.624
righe, 4490 PC distinti, 7815 byte eseguiti, 119 destinazioni di call/rst
(124 con la definizione «PC dopo un call/rst», che conta anche la NMI entrata
subito dopo un call e i call condizionati non presi).

- ROM del programma: 0000-bfff (48 KB) + f000-ffff (4 KB, primi 4096 byte di
  8.3jk). **f000-ffff è dati**: mai eseguito, contiene tabelle di puntatori.
- Codice eseguito: 10.183 byte su 53.248 (19%). Grandi zone mai eseguite:
  80a5-bfff (16 KB, quasi certamente dati: livelli, testi, tabelle),
  12ab-2349 (4 KB), 5cfb-63a3, 4f1c-5506, 0cf3-1287, 6c48-714a, 6785-6c1c,
  3f93-4397. Il disassembly statico ha 299 destinazioni di `call` in ROM:
  129 eseguite, 170 mai (in parte codice non raggiunto in un minuto di gioco
  casuale: livelli successivi, bonus, fine partita; in parte dati letti come
  codice).
- Nessun codice in RAM, nessun `jp (hl)`/`jp (ix)`/`jp (iy)` eseguito.
- `rst $20` (salto su tabella inline dopo l'istruzione) eseguito 123.825
  volte: è il principale ostacolo strutturale, i dati stanno dentro il codice.
- Z80 audio (`:audiocpu`, 1.3jk): 16 KB, fuori dal sondaggio.

## Grafo delle chiamate (`out/callgraph.csv`)

135 routine (134 destinazioni di call/rst eseguite più la NMI 0066), 10.176
byte nelle estensioni. Routine «normali» (nessun motivo di bruttezza): 117,
7699 byte. Routine «brutte»: 18.

| entry | byte | invocazioni | aree | motivo |
|---|---|---|---|---|
| 0020 | 7 | 123825 |  | rst20 |
| 0043 | 12 | 163 | ram porte | porte |
| 0066 | 318 | 5435 | ram sprite palette porte audio | porte |
| 2551 | 277 | 9 | ram video sprite palette porte | porte |
| 2980 | 71 | 2 | ram porte | porte |
| 2AE2 | 19 | 1525 | video porte | porte |
| 2BC3 | 6 | 71135 | porte | porte |
| 2BF3 | 35 | 5 | porte | porte |
| 2C1A | 98 | 56 | ram video porte | porte+rst20 |
| 2D8A | 91 | 1 | ram sprite | rst20 |
| 2E90 | 199 | 6398 | ram porte | porte |
| 326C | 231 | 12 | ram | rst20 |
| 342D | 18 | 3 | ram porte | porte |
| 3569 | 224 | 9 | ram porte | porte |
| 3812 | 347 | 4 | ram video porte | porte |
| 3ADF | 141 | 4 | ram video | rst20 |
| 75AA | 251 | 4 | ram video porte | porte+rst20 |
| 7C84 | 132 | 3 | ram video porte | porte+rst20 |

Quasi tutte le «porte» sono innocue (scrittura di `e600` maschera NMI, `e604`
ribaltamento, lettura di ingressi e DIP): in C diventano chiamate a funzioni
di piattaforma. Le uniche porte vere sono la protezione `e603` (letta da
015E nella NMI, 4CED, 62BE; il valore dipende dal PC) e il latch audio `e800`.

Limite noto del grafo: l'estensione di una routine va fino al prossimo
ingresso in ordine di indirizzo, quindi sovrastima le routine con code
condivise e spezza quelle con ingressi alternativi (05B3/05B5).

Altre osservazioni: il programma usa più pile (SP intorno a d000 nel ciclo
principale, cff0-cffa in gioco, dbfa dentro la NMI con pila a dc00): il ciclo
principale a 3AFA scandisce una tabella di attività in RAM, probabilmente uno
scheduler cooperativo con una pila per attività. Questo va capito prima di
tradurre il flusso di controllo ad alto livello.

## Prova di metodo (`harness/`)

| routine | contenuto | esito |
|---|---|---|
| 0x05B3 | tabella sprite c8ee da 24 oggetti a c46e | OK, 96 byte c8ee-c94d e registri coincidono |
| 0x2402 | scrive DE a (HL), HL += 2, DE += BC, A-- | OK, HL=E402 DE=0001 |

Il C è una traduzione leggibile (nomi per i campi degli oggetti, ciclo `for`,
niente bandiere), confrontato con lo stato prima/dopo catturato in MAME
(`snap.lua`): RAM c000-e5ff byte per byte e registri A, BC, DE, HL, IX, IY.
La prima occorrenza di 05B3 (frame 67) ha tutti gli oggetti spenti; con una
cattura di prova a gioco iniziato (frame 1902, 5 oggetti visibili, ramo con
correzione della y) il harness dà ugualmente OK. Terza routine non tradotta
(facoltativa secondo la decisione 4).

Tempo osservato: 05B3 (79 byte, 31 istruzioni) circa 10 minuti di lavoro
agente fra lettura del disassembly, traduzione e verifica; 2402 circa 2
minuti. Il banco di prova e la cattura sono già pronti e si riusano.

## Costo stimato

- Routine da tradurre: 117 normali + 18 brutte già viste; contando il codice
  non raggiunto (170 destinazioni statiche non eseguite, da depurare dei
  falsi) il totale realistico è 250-300 routine, 18-25 KB di codice.
- A circa 8 byte di codice per minuto (ritmo di 05B3) servono 40-50 ore di
  agente per la sola traduzione, più la scrittura di una cattura MAME per
  routine (una sessione da 1-2 minuti ciascuna) e il lavoro sulle routine
  brutte: tabelle `rst $20`, scheduler a più pile, protezione `e603`, NMI.
- Per coprire il codice non raggiunto servono sessioni di gioco guidate
  (input registrati che arrivano ai livelli successivi), non input casuali.

## Giudizio: fattibile con riserve

1. Il metodo funziona: traccia, copertura, grafo e confronto prima/dopo con
   MAME sono automatici e la prova su due routine passa al primo colpo.
2. Il codice è pulito: niente codice in RAM, niente salti indiretti via
   registro, autoverifica delle lunghezze a 0 violazioni su 42,8 M righe.
3. Le routine foglia RAM→RAM sono tante e si traducono in pochi minuti.
4. Riserva 1: `rst $20` con tabelle inline (123.825 esecuzioni) obbliga a
   estrarre a mano le tabelle di salto prima di tradurre i chiamanti.
5. Riserva 2: lo scheduler a più pile del ciclo principale e la NMI non si
   traducono come funzioni C ordinarie; serve un modello (coroutine o
   macchina a stati) deciso prima.
6. Riserva 3: solo il 19% della ROM è eseguito; il resto va raggiunto con
   sessioni guidate, altrimenti la verifica contro MAME non è possibile.
7. Riserva 4: la protezione `e603` va emulata come tabella per PC.
8. Ordine di grandezza: 250-300 routine, una cinquantina di ore di agente.
9. Il passo successivo sensato è tradurre le 117 routine normali già coperte
   dalla traccia, in lotti, con lo stesso banco di prova.
10. Il Z80 audio (16 KB) resta fuori e può restare emulato.

## Come rifare le misure

Da questa cartella, con `DISPLAY=:1` e le opzioni MAME del brief:
`sh build_rom.sh`, `sh disasm.sh`, `mame solomon ... -debug -debugger none
-autoboot_script trace.lua`, `python3 coverage.py --cancella`,
`python3 callgraph.py`, `SNAP_ENTRY=0x05B3 mame ... -autoboot_script snap.lua`,
`make -C harness && ./harness/harness 0x05B3`.
