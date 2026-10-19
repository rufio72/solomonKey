# Lo spedidore delle attività del Z80 principale (3AF3) e la NMI

Brief 04, 2026-10-19. Misure su MAME 0.264, set `solomon`, 5500 frame con la
sequenza di ingresso di `trace.lua` (moneta 1510-1530, start 1560-1580, input
casuali dal 1900, seme 1), registrate da `sched_log.lua` in `out/sched_log.txt`
e riprodotte da `harness/sched_proto`.

## Stato in RAM

| indirizzo | ruolo |
|-----------|-------|
| c000-c03f | tabella delle attività: un byte per slot, codice dell'attività (0 = slot vuoto) |
| c04f | indice del prossimo slot che lo spedidore esamina (0..3f) |
| c04e | indice di ricerca dello slot libero usato da 0493; cresce senza maschera, si usa `& 3f` |

Scrittori di c04e/c04f oltre a 0493 (049B) e allo spedidore (3B0F): solo il
test RAM (2C1B, 8 passate), l'init 2868 (frame 64) e 2942/2944 (frame 601), che
li azzerano. Azzeramento dell'intera tabella: 2868 (init) e 6742-6746 (`ld
(hl),c; inc hl; djnz`, chiamata da 673F/2C3A nell'attività 0B; 5 volte in 5500 frame).

## Lo spedidore 3AF3-3B10

```
3AF3 ld a,(c04f) ; ld l,a ; ld h,c0     HL = c000 + indice
3AF9 xor a
3AFA or (hl)                            codice nello slot
3AFB jr nz,3B02
3AFD inc l ; res 6,l ; jr 3AFA          slot successivo, ciclico su 64
3B02 ld b,a
3B03 ld de,3AF3 ; push de               ritorno allo spedidore
3B07 push hl                            puntatore allo slot per l'attività
3B08 inc l ; res 6,l ; ld a,l ; ld (c04f),a   prossimo indice
3B0F ld a,b
3B10 rst 20h                            salto a tabella_3B11[codice]
```

È un polling round-robin: ogni attività eseguita torna a 3AF3, che riprende
la scansione dallo slot dopo quello appena servito. L'indice c04f sopravvive
al cambio di frame: il giro non è allineato alla NMI. Se la tabella fosse
vuota la scansione girerebbe per sempre (solo la NMI potrebbe riempirla).

## La primitiva di posta 0493

```
0493 ld hl,c04e ; ld d,a
0496 ld a,3f ; and (hl) ; inc (hl)      indice = c04e & 3f, poi c04e++
     ... se c000+indice != 0 torna a 0496
04AA ld (hl),d ; ret                    scrive il codice nello slot libero
```

Siti di chiamata nel disassemblato: 017B (NMI, codice 27, solo con e603 bit 0
= 1: mai in MAME), 0267, 0308, 03DD, 042B, 0465, 0513, 12D2-140F (7), 2951,
2956, 2C52, 2C6E, 2E35, 3671, 3DCB, 3DD0, 3E49, 4005, 404F, 4131, 42CF,
64FE-6731 (6), 6AFF, 6B2D, 78EC, 78F7, 7979, 7B41, 7C53, 7CA6, 7CE0, 7D65.

**La NMI posta attività.** Contrariamente al brief, la NMI scrive la tabella:
dei 131 `posta_attivita` misurati, 116 partono dalla NMI (indirizzo di ritorno
sulla pila: 042E da 00F3 via 03FB/042F, codice 01; 0468 da 0101, codice 0E;
0516 da 0114 via 0CE6, codice 10; 026A, codice 08, la moneta). Di questi, 32
cadono a metà della scansione dello spedidore (fra due letture a 3AFA): per
riprodurre l'ordine esatto il registro annota per ogni scrittura quante letture
della scansione sono già avvenute, e il prototipo le inietta nello stesso punto.

## Tabella 3B11: codice, ingresso, modo di uscita osservato

Uscite: `pop hl; ret` = resta nello slot; `pop hl; ld (hl),n; ret` = cambia il
proprio codice; azzerata = qualcun altro scrive 0 nello slot. Spedizioni e
scritture sul proprio slot durante il corpo, da `out/sched_log.txt`.

| cod | ingresso | spedizioni | uscita osservata |
|----|------|------:|------------------|
| 00 | 3B61 | 0 | `pop hl; ret` (voce vuota) |
| 01 | 48D1 | 45 | diventa 02 (4925 ×14, 4937 ×31) |
| 02 | 4B84 | 2647 | resta (4B8B) o diventa 03 (4B95/4B97 ×45) |
| 03 | 4BC6 | 3689 | resta (4BCD) o azzerata (4C10 ×104) |
| 04 | 4C30 | 0 | diventa 03 (4C3F), dal disassemblato |
| 05 | 4C42 | 0 | diventa 09 (4C48), dal disassemblato |
| 06 | 75E7 | 8203 | resta; diventa 07 (7619 ×1); azzerata (6746) |
| 07 | 2CCD | 6593 | a passi su c19d; azzerata (2E58 ×2) |
| 08 | 7C98 | 1 | azzerata (6746): moneta |
| 09 | 4D29 | 0 | non osservata |
| 0A | 4DA2 | 17411 | resta (legge c2b8) |
| 0B | 2C27 | 1 | azzera la tabella (6742 via 673F) |
| 0C | 7BF6 | 192 | resta; azzerata (6746) |
| 0D | 5C8E | 17410 | resta (legge c29a) |
| 0E | 63A4 | 59 | diventa 03 (63C2 ×59) |
| 0F | 640E | 0 | non osservata |
| 10 | 457B | 11 | diventa 11 (4591 ×2) o azzerata (4591 ×9) |
| 11 | 456E | 55 | resta o azzerata (457A ×2) |
| 12, 13 | 426A, 6480 | 0 | non osservate |
| 14 | 44B5 | 32395 | resta (sempre presente dal frame 610) |
| 15, 16, 17 | 6466, 6496, 6429 | 0 | fine partita/attract: non nella misura a 5500 frame |
| 18-1D | 4124, 3792, 411D, 4111, 3F40, 3F39 | 0 | non osservate |
| 1E, 21 | 3B61 | 0 | `pop hl; ret` (voci vuote) |
| 1F | 43C8 | 2 | diventa 14 (43E4 ×2) |
| 20 | 3E90 | 17410 | resta (3E98) |
| 22-25 | 12D6, 3E4E, 2C72, 3B77 | 0 | non osservate |
| 26 | 3B63 | 0 | a passi, stesso schema di 07 |
| 27 | 62B6 | 0 | postata solo da 017B (NMI con e603 bit 0 = 1) |

## Attività a passi

2CCD (codice 07) fa `ld hl,c19d; ld a,(hl); inc (hl); rst 20h`: spedisce sul
passo in c19d (26 voci) e lo fa avanzare; un passo che deve aspettare fa `dec
(hl)` e resta sullo stesso passo al giro dopo. Sotto-passo in c19e (2DAD `cp 6`,
2DCE `cp 7`). Stesso schema a 3B63 (codice 26). In C il contatore resta in RAM
e la funzione dell'attività fa uno `switch` su di esso, un passo per chiamata.

## La NMI 0066

Una per frame (5435 in 5500, disabilitata nei primi ~65 frame). Salva AF,
banco alternativo, IX/IY in c040/c042; copia c994→e400, c9ca→e500 e gli sprite
(`ldir` a 00A1-00C2), `call 05B3`; poi 00F0/00FE/010B-0117 (ingressi, moneta,
che postano attività, vedi sopra), 0150 `call nz,01C5`, `inc (c2b7)` (0156) e
`inc (c29a)` (015A), contatori di frame letti dalle attività (4DA2 legge c2b8,
5C8E legge c29a), 015B `call 026C`, protezione e603 (015E), audio e800, e600,
`retn` a 01C3.

## Una sola pila

`ld sp` solo a 283A (`dc00`, init) e 286E (`d000`); nessun `ld sp,(nn)` né `ld
(nn),sp`; `ex (sp),hl` solo nei gestori rst (0020/0025/0038/0041) e a
2A23-2A38, 323C. Le «pile» cff0-cffa e dbfa del rapporto 02 sono profondità di
chiamata sulla pila d000 e la pila dc00 dell'init.

## Numeri misurati (5500 frame, `sched_log.lua`)

- Spedizioni (letture a 3AFB con valore ≠ 0): 106.124, di cui 30.725 nei frame
  ≤ 1900; per frame (1900-5500) fra 15 e 29, media 21. Letture totali della
  tabella a 3AFB 2.073.166 (dal brief, non rimisurato).
- Scritture nella tabella: 1298. Per PC (dopo l'istruzione): 2C1B 512 (test
  RAM: 8 passate da 64 con 01, 02, ... 80), 6746 320, 04AB 131 (0493), 4C10 104,
  2868 64, 63C2 59, 4B97 45, 4937 31, 4925 14, 4591 11, 457A 2, 43E4 2, 2E58 2,
  7619 1. Il brief indicava 04AB 110 e 6746 64 con lo stesso totale: la somma
  delle sue voci faceva 949, non 1298; i numeri qui sopra sono quelli rimisurati.
- Prima spedizione al frame 601 (slot 00, codice 1F).

## Modello C adottato: funzioni, non coroutine

`harness/sched.h` e `harness/sched.c`, prova in `harness/sched_proto.c`.

| originale | C |
|-----------|---|
| slot c000+i | `tabella[i]` (= `&M(0xc000)`) |
| c04f, c04e | `indice`, `prossimo_libero` (byte in RAM) |
| 0493 | `uint8_t posta_attivita(uint8_t codice)`, restituisce lo slot |
| 3AF3-3B10 | `int spedisci_una(uint8_t *slot, uint8_t *codice)` |
| tabella 3B11 | `attivita_fn corpo_attivita[40]`, una funzione `attivita_XX_YYYY(uint8_t slot)` per codice |
| `pop hl; ret` | `return` |
| `pop hl; ld (hl),n; ret` | `tabella[slot] = n; return` |
| contatore di passo c19d | `switch` sul byte in RAM |

Il ciclo principale è `for (;;) spedisci_una(&s, &c);` più la NMI chiamata a
ogni frame. Perché non servono coroutine:
1. c'è una sola pila e nessuna attività la cambia (vedi sopra);
2. un'attività entra sempre dalla testa della routine indicata in 3B11, con
   in pila solo lo slot e il ritorno a 3AF3;
3. cede il controllo solo ritornando (`ret` verso 3AF3), mai a metà di una
   chiamata annidata: non c'è un punto di ripresa da conservare;
4. tutto lo stato che sopravvive fra due giri (passo c19d/c19e, contatori,
   codice nello slot) è in RAM, non in registri o sulla pila;
5. quindi ogni ingresso è una normale chiamata di funzione che fa un passo e
   ritorna, e lo spedidore è un ciclo che chiama funzioni: è esattamente il
   modello a funzioni, senza cambio di contesto.

Le attività sono ancora stub: in `sched_proto` ogni stub applica le scritture
registrate da MAME fino alla spedizione successiva (le `W` con pc 04AB passano
per `posta_attivita` e se ne confronta lo slot); le scritture della NMI a metà
scansione si applicano dal gancio `sched_gancio_scansione` dopo lo stesso
numero di letture. Il confronto prova lo spedidore, la posta e il ciclo, non il
contenuto delle attività. Risultato: 106.124 spedizioni, 0 differenze; 131
`posta_attivita` con lo slot coincidente.

Nel port vero la NMI arriverà a un punto di sincronia (fine frame), non fra
due letture: un'attività postata dalla NMI potrà partire un giro prima o dopo
rispetto a MAME. Per la prova di equivalenza serve il punto esatto, che il
registro fornisce.

## Formato di `out/sched_log.txt`

```
D <frame> <slot> <codice>                  spedizione
W <frame> <pc> <slot> <valore> <letture>   scrittura nella tabella
I <frame> <pc> <indirizzo> <valore>        scrittura di c04e/c04f fuori da 0493/3AF3
```
frame = frame completati prima dell'evento; pc = PC dopo l'istruzione (04AA →
04AB); letture = letture a 3AFA dalla spedizione precedente (0 = durante il
corpo dell'attività).
