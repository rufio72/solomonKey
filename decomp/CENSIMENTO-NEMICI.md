# Censimento del motore dei nemici (corpo 0A, ingresso 4DA2)

Brief 24 (2026-10-28). Base dei lotti di traduzione del corpo 0A: in questo
lotto non si traduce nulla. Le tabelle qui sotto sono generate da
`censimento_nemici.py` (`out/censimento_nemici_tabelle.md`); i file grezzi
sono `out/rst38.txt`, `out/censimento_nemici.txt` e
`out/censimento_nemici_lotti.txt`, e l'intestazione per il C è
`harness/rst38_tables.h`.

## Metodo

Il cammino statico parte dalla ROM decodificata (`out/main64.bin`) al 4DA2 e
segue caduta, `jp`/`jr`/`djnz`/`call` statici, `rst 20h` (voci di
`out/rst20.txt`) e `rst 38h` (gestore a 0038 = `jp tabella[E]`, ret a 0042).
Si ferma sulle routine già tradotte (`harness/routine.h` più le statiche di
`attivita.c`). Le tabelle `rst 38` vengono da `voci_euristiche` di `rst20.py`,
ripetuta finché il cammino non cambia. Ogni tabella si chiude sulla prima
destinazione nota, cioè una voce, un bersaglio di salto o di call, oppure un
operando `ld rr,nn`. Il limite viene anche dall'indice in E, quando si può
dedurre. La copertura viene dalla sessione del replay:

    SESS_NOME=replay_gioco SESS_INPUT=casuale SESS_TRACE_DA=2400 SESS_TRACE_A=5500 \
    SDL_VIDEODRIVER=dummy mame solomon -rompath "$(cd .. && pwd)/roms" \
      -video none -sound none -keyboardprovider none -mouseprovider none \
      -joystickprovider none -lightgunprovider none -skip_gameinfo -nothrottle \
      -debug -debugger none -autoboot_script sessione.lua
    python3 censimento_nemici.py --traccia out/replay_gioco_trace.txt
    python3 coverage.py --traccia out/replay_gioco_trace.txt --prefisso replay_gioco --cancella
    python3 censimento_nemici.py      # di nuovo, dalla cache: stesse uscite

La traccia occupava 379 MB (23 410 593 righe, 2698 PC distinti) ed è stata
cancellata. Senza `--traccia` lo script usa `out/rst38_dinamici.txt` e
`out/replay_gioco_eseguiti.txt`.

## Numeri

- Siti `rst 38` nel cammino: **21**. Sono i nove noti (4DA6, 4F1B, 4FE0,
  5127, 5207, 5E40, 6130, 728B, 73B6) più 12 scoperti dal cammino. Nel
  replay ne vanno in onda 5 (4DA6, 4F1B, 55B5, 714E, 7F39), con 0
  destinazioni fuori tabella e nessun sito dinamico fuori dal cammino.
- Siti `rst 20` nel cammino: 4644 e 6F1C. La tabella di **6F1C** ha 16
  voci, non le 22 di `out/rst20.txt`: da 6F3D in poi ci sono dati letti da
  `ld hl,6F3D` a 6ECF (`9A 00 E6 00 33 01`, che sembrerebbero voci verso il
  gestore della NMI). `rst20.py` e `rst20_tables.h` non sono stati toccati
  (vincolo del brief): lo script tronca la tabella solo nel cammino.
- Routine: **220**, di cui 14 ingressi interni (code condivise), su **8833
  byte** e 4331 PC. Coperte dal replay: SI 13, PARZIALE 14, NO 193. Dei 4331
  PC, 3593 stanno nell'unione storica `out/eseguiti_unione.txt`: il
  perimetro statico è quasi tutto codice vero.
- Il perimetro di REPLAY.md (~3600 B, 36 routine) contava solo le istruzioni
  eseguite in qualche sessione. Il cammino statico conta anche i rami mai
  presi, le 18 voci del 4F1B e le routine fuori zona (13xx-14xx dalla
  tabella di 4FAC, 24xx, 4592-4C2F): da qui i 2,5 volte tanto.
- Il replay esegue soltanto i tipi 10 (714B), 12 (55B2) e 17 (7F68) del
  4F1B. Gli altri tipi non vanno in onda nella finestra 2400-5500.

## Siti `rst $38`

Indice in E. Due tabelle hanno il limite dedotto dal codice: **4FAC**, con
`cp 06h; jr nc` attraverso `call 0028` che conserva A ed E, dà 6 voci;
**6130**, con `and 0Fh`, dà 16 voci. Le altre hanno l'indice letto da
memoria (`ld e,(iy+1Dh)` in quasi tutte, `(iy+03) - D` al 4F1B, `c2b8` al
4DA6): la lunghezza è quella dell'euristica. Per ognuna è stato controllato
a mano che il primo byte dopo la tabella sia l'inizio di un'istruzione (es.
4DAD, 4F40, 4FB9, 513A, 7F48). L'indice massimo osservato non supera mai la
lunghezza. 3B62 è un `ret` solitario, cioè la voce nulla.

| sito | voci | indice max | stato | tabella |
|---|---:|---:|---|---|
| 4DA6 | 3 | 2 | eseguito | 4DAD 4632 4EE6 |
| 4F1B | 18 | 17 | eseguito | 4F40 3B62 3B62 3B62 73A3 7288 4FD4 5CFB 6ECA 57CD 714B 56E9 55B2 54D8 6CE8 6C0B 6B43 7F68 |
| 4FAC | 6 | - | statico | 14FC 13AB 139C 13B9 1413 14D4 |
| 4FE0 | 7 | - | statico | 5005 5023 5092 3B62 3B62 50DF 50BA |
| 5127 | 9 | - | statico | 513A 513E 514C 516B 5186 51A4 51B1 51B5 51D3 |
| 5207 | 14 | - | statico | 5224 5274 523C 5274 5254 5274 5289 5292 52DB 52AA 52DB 52C2 52DB 52EC |
| 54DB | 7 | - | statico | 5584 551A 54EA 3B62 3B62 5529 3B62 |
| 55B5 | 7 | 5 | eseguito | 5693 6C1D 561B 55C4 55C9 5640 3B62 |
| 5704 | 7 | - | statico | 571D 5726 573C 573C 3B62 5749 5713 |
| 57D3 | 8 | - | statico | 57FB 582B 580A 3B62 57E4 5995 594F 58A3 |
| 5BFC | 14 | - | statico | 5C19 5C23 5C19 5C23 5C23 5C23 3B62 5C1E 5C29 5C1E 5C29 5C29 5C29 5B09 |
| 5E40 | 6 | - | statico | 5E60 5E9D 5FC8 5F33 5EDD 5EDD |
| 6130 | 16 | - | statico | 627F 6165 6165 61B1 6151 6261 6183 6196 6151 6179 6270 6196 61B1 61FA 6221 61B1 |
| 6B58 | 3 | - | statico | 6B5F 6B8F 6B9B |
| 6C0E | 7 | - | statico | 3B62 6C1D 6D37 6C5F 6DCE 6C8E 6EA1 |
| 6D03 | 7 | - | statico | 571D 6D1B 6D37 6D12 6DCE 6E55 6EA1 |
| 714E | 3 | 2 | eseguito | 7155 719E 7235 |
| 728B | 4 | - | statico | 7294 72DA 72B6 72CE |
| 73B6 | 8 | - | statico | 7412 741D 7470 3B62 3B62 3B62 74AB 73C7 |
| 7F39 | 7 | 4 | eseguito | 3B62 7FA6 7F48 7FCF 7F51 3B62 3B62 |
| 7F72 | 7 | - | statico | 7F81 7FA6 7F8A 7FCF 801D 8042 3B62 |

## Routine

Le colonne:
- **estensione**: da min a max dei PC della routine. Ogni routine contiene
  i PC raggiunti dal suo ingresso senza passare per un altro ingresso.
- **byte**: somma delle lunghezze delle istruzioni, senza le tabelle in
  linea.
- **replay**: copertura in `out/replay_gioco_eseguiti.txt`. SI vuol dire
  tutti i PC, PARZIALE una parte, NO nessuno.
- **unione**: la stessa misura sull'unione storica delle sessioni.

Le chiamate di ogni routine, comprese quelle verso la frontiera tradotta
(segnate `(t)`), stanno in `out/censimento_nemici.txt`.

| ingresso | estensione | byte | chiamanti | replay | unione | lotto | modulo | note |
|---|---|---:|---|---|---|---|---|---|
| 134B | 134B-135A | 16 | 13B9 | NO | PARZIALE | N4 | nemici_comparsa | fuori dalla zona 4xxx-8xxx |
| 139C | 139C-13AA | 15 | 4F40 | NO | SI | N4 | nemici_comparsa | fuori dalla zona 4xxx-8xxx |
| 13AB | 13AB-13B8 | 14 | 4F40 | NO | SI | N4 | nemici_comparsa | fuori dalla zona 4xxx-8xxx |
| 13B9 | 13B9-13C2 | 10 | 4F40 | NO | SI | N4 | nemici_comparsa | fuori dalla zona 4xxx-8xxx |
| 13FC | 13FC-1412 | 23 | 139C 13AB 13B9 1413 14D4 14FC 4F40 73C7 | NO | SI | N4 | nemici_comparsa | fuori dalla zona 4xxx-8xxx |
| 1413 | 1413-1423 | 17 | 4F40 | NO | SI | N4 | nemici_comparsa | fuori dalla zona 4xxx-8xxx |
| 14D4 | 14D4-14FB | 40 | 4F40 | NO | PARZIALE | N4 | nemici_comparsa | fuori dalla zona 4xxx-8xxx |
| 14FC | 14FC-150B | 16 | 13FC 4F40 | NO | NO | N4 | nemici_comparsa | fuori dalla zona 4xxx-8xxx |
| 1549 | 1549-1556 | 14 | 13FC | NO | SI | N4 | nemici_comparsa | fuori dalla zona 4xxx-8xxx |
| 2474 | 2474-2481 | 14 | 4B20 | NO | SI | N12 | nemici_comuni | anche corpi 01/10; fuori dalla zona 4xxx-8xxx |
| 2482 | 2482-248C | 11 | 4C11 | SI | SI | N1 | nemici_comuni | anche corpi 01/10; fuori dalla zona 4xxx-8xxx |
| 248D | 248D-249D | 17 | 4C11 | SI | SI | N1 | nemici_comuni | anche corpi 01/10; fuori dalla zona 4xxx-8xxx |
| 24BE | 24BE-24DE | 33 | 4938 7F8A 808D | SI | SI | N1 | nemici_comuni | anche corpi 01/10; fuori dalla zona 4xxx-8xxx |
| 24DF | 24DF-250D | 47 | 5345 | NO | SI | N10 | nemici_tipo06a | fuori dalla zona 4xxx-8xxx |
| 2548 | 2548-2550 | 9 | 759F | NO | SI | N4 | nemici_comparsa | fuori dalla zona 4xxx-8xxx |
| 3B62 | 3B62-3B62 | 1 | 4F0F 4FD4 54D8 55B2 56E9 57CD 5995 6C0B 73A3 7F68 | NO | SI | N7 | nemici | fuori dalla zona 4xxx-8xxx; solo `ret`: voce nulla delle tabelle |
| 4592 | 4592-4631 | 160 | 582B | PARZIALE | SI | N1 | nemici_comuni | anche corpi 01/10 |
| 4632 | 4632-4644 | 19 | 4DA2 | PARZIALE | SI | N1 | nemici | - |
| 4649 | 4649-46A2 | 90 | 4632 | NO | SI | N7 | nemici | - |
| 46A3 | 46A3-4709 | 103 | 4632 | NO | PARZIALE | N7 | nemici | - |
| 470A | 470A-4716 | 13 | 46A3 | NO | SI | N7 | nemici | - |
| 4717 | 4717-4721 | 11 | 46A3 | NO | SI | N7 | nemici | - |
| 4767 | 4767-488C | 264 | 4649 | NO | PARZIALE | N7 | nemici | - |
| 4938 | 4938-49AD | 118 | 4592 58CF 6DCE 719E 72DA | PARZIALE | SI | N1 | nemici_comuni | anche corpi 01/10 |
| 49AE | 49AE-4AD9 | 300 | 58CF | PARZIALE | PARZIALE | N1 | nemici_comuni | anche corpi 01/10 |
| 4ADA | 4ADA-4AF5 | 28 | 49AE | NO | SI | N12 | nemici_comuni | anche corpi 01/10 |
| 4B20 | 4B20-4B83 | 100 | 49AE | PARZIALE | SI | N1 | nemici_comuni | anche corpi 01/10 |
| 4C11 | 4C11-4C2F | 31 | 4938 808D | SI | SI | N1 | nemici_comuni | anche corpi 01/10 |
| 4DA2 | 4DA2-4DA6 | 5 | - | SI | SI | N1 | nemici | - |
| 4DAD | 4DAD-4DED | 65 | 4DA2 | SI | SI | N1 | nemici | - |
| 4EE6 | 4EE6-4F0E | 41 | 4DA2 | SI | SI | N1 | nemici | - |
| 4F0F | 4F0F-4F1B | 13 | 4EE6 | SI | SI | N1 | nemici | smistatore dei tipi 4F1B: E = (iy+03) - D |
| 4F40 | 4F40-4FC4 | 121 | 4F0F | NO | PARZIALE | N4 | nemici_comparsa | - |
| 4FC5 | 4FC5-4FD3 | 15 | 4F40 73C7 | NO | NO | N4 | nemici_comparsa | osservata dal vivo: 6 DEST in partita_viva_20261027.log, 1 nel 27b; probabile comparsa di un'entità (fata) |
| 4FD4 | 4FD4-4FE0 | 13 | 4F0F | NO | SI | N10 | nemici_tipo06a | - |
| 4FEF | 4FEF-4FF8 | 10 | 4FD4 51B5 5CFB 5FAC | NO | SI | N10 | nemici_tipo06a | - |
| 5005 | 5005-5022 | 30 | 4FD4 | NO | SI | N10 | nemici_tipo06a | - |
| 5023 | 5023-5034 | 18 | 4FD4 | NO | SI | N10 | nemici_tipo06a | - |
| 5035 | 5035-5065 | 49 | 5023 5092 | NO | PARZIALE | N10 | nemici_tipo06a | ingresso interno (coda condivisa) |
| 5066 | 5066-5067 | 2 | 5023 5035 | NO | SI | N10 | nemici_tipo06a | ingresso interno (coda condivisa) |
| 5068 | 5068-5091 | 42 | 5066 5092 51B5 531C | NO | SI | N10 | nemici_tipo06a | ingresso interno (coda condivisa) |
| 5092 | 5092-50B9 | 40 | 4FD4 | NO | NO | N10 | nemici_tipo06a | - |
| 50BA | 50BA-50DE | 37 | 4FD4 | NO | SI | N10 | nemici_tipo06a | - |
| 50DF | 50DF-5114 | 54 | 4FD4 | NO | SI | N10 | nemici_tipo06a | - |
| 5115 | 5115-5123 | 15 | 50DF | NO | PARZIALE | N10 | nemici_tipo06a | - |
| 5124 | 5124-5127 | 4 | 50DF | NO | SI | N10 | nemici_tipo06a | - |
| 513A | 513A-513D | 4 | 5124 | NO | NO | N10 | nemici_tipo06a | - |
| 513E | 513E-514B | 14 | 5124 | NO | PARZIALE | N10 | nemici_tipo06a | - |
| 514C | 514C-516A | 31 | 5124 | NO | PARZIALE | N10 | nemici_tipo06a | - |
| 516B | 516B-5185 | 27 | 5124 | NO | PARZIALE | N10 | nemici_tipo06a | - |
| 5186 | 5186-51A3 | 30 | 5124 | NO | NO | N10 | nemici_tipo06a | - |
| 51A4 | 51A4-51B0 | 13 | 5124 | NO | NO | N10 | nemici_tipo06a | - |
| 51B1 | 51B1-51B4 | 4 | 5124 51A4 | NO | NO | N10 | nemici_tipo06a | - |
| 51B5 | 51B5-51D2 | 30 | 5124 | NO | SI | N10 | nemici_tipo06a | - |
| 51D3 | 51D3-51D6 | 4 | 5124 | NO | NO | N10 | nemici_tipo06a | - |
| 51D7 | 51D7-51F9 | 35 | 51FA 5995 | NO | SI | N10 | nemici_tipo06a | - |
| 51FA | 51FA-5207 | 14 | 50DF | NO | SI | N10 | nemici_tipo06a | - |
| 5224 | 5224-523B | 24 | 51FA | NO | PARZIALE | N10 | nemici_tipo06a | - |
| 523C | 523C-5253 | 24 | 51FA | NO | NO | N10 | nemici_tipo06a | - |
| 5254 | 5254-5273 | 32 | 51FA | NO | NO | N10 | nemici_tipo06a | - |
| 5274 | 5274-5288 | 21 | 51FA | NO | NO | N6 | nemici_tipo06b | - |
| 5289 | 5289-5291 | 9 | 51FA | NO | NO | N6 | nemici_tipo06b | - |
| 5292 | 5292-52A9 | 24 | 51FA | NO | NO | N6 | nemici_tipo06b | - |
| 52AA | 52AA-52C1 | 24 | 51FA | NO | NO | N6 | nemici_tipo06b | - |
| 52C2 | 52C2-52DA | 25 | 51FA | NO | PARZIALE | N6 | nemici_tipo06b | - |
| 52DB | 52DB-52EB | 17 | 51FA | NO | NO | N6 | nemici_tipo06b | - |
| 52EC | 52EC-52FC | 17 | 51FA | NO | SI | N6 | nemici_tipo06b | - |
| 52FD | 52FD-531B | 31 | 4FD4 57CD 5CFB | NO | SI | N6 | nemici_tipo06b | - |
| 531C | 531C-5344 | 41 | 50BA 50DF | NO | PARZIALE | N6 | nemici_tipo06b | - |
| 5345 | 5345-54C4 | 384 | 50DF 5995 | NO | PARZIALE | N6 | nemici_tipo06b | - |
| 54C5 | 54C5-54D7 | 19 | 51D7 | NO | SI | N6 | nemici_tipo06b | - |
| 54D8 | 54D8-54DB | 4 | 4F0F | NO | SI | N5 | nemici_tipi_10_13 | - |
| 54EA | 54EA-5506 | 29 | 54D8 | NO | SI | N5 | nemici_tipi_10_13 | - |
| 551A | 551A-5528 | 15 | 54D8 | NO | PARZIALE | N5 | nemici_tipi_10_13 | - |
| 5529 | 5529-5583 | 91 | 54D8 | NO | SI | N5 | nemici_tipi_10_13 | - |
| 5584 | 5584-55B1 | 46 | 54D8 | NO | PARZIALE | N5 | nemici_tipi_10_13 | - |
| 55B2 | 55B2-55B5 | 4 | 4F0F | SI | SI | N1 | nemici_tipi_10_13 | - |
| 55C4 | 55C4-55C8 | 5 | 55B2 | NO | SI | N5 | nemici_tipi_10_13 | - |
| 55C9 | 55C9-561A | 82 | 55B2 | PARZIALE | PARZIALE | N1 | nemici_tipi_10_13 | - |
| 561B | 561B-563F | 37 | 54EA 55B2 | SI | SI | N1 | nemici_tipi_10_13 | - |
| 5640 | 5640-5692 | 83 | 55B2 | SI | SI | N1 | nemici_tipi_10_13 | - |
| 5693 | 5693-56E8 | 86 | 55B2 | PARZIALE | PARZIALE | N1 | nemici_tipi_10_13 | - |
| 56E9 | 56E9-5704 | 28 | 4F0F | NO | PARZIALE | N5 | nemici_tipi_10_13 | - |
| 5713 | 5713-571C | 10 | 56E9 | NO | PARZIALE | N5 | nemici_tipi_10_13 | - |
| 571D | 571D-5725 | 9 | 56E9 6CE8 | NO | NO | N5 | nemici_tipi_10_13 | - |
| 5726 | 5726-573B | 22 | 56E9 | NO | PARZIALE | N5 | nemici_tipi_10_13 | - |
| 573C | 573C-5748 | 13 | 56E9 | NO | SI | N5 | nemici_tipi_10_13 | - |
| 5749 | 5749-57A9 | 97 | 56E9 | NO | PARZIALE | N5 | nemici_tipi_10_13 | - |
| 57AA | 57AA-57B2 | 9 | 56E9 6B9B 6CE8 | NO | SI | N5 | nemici_tipi_10_13 | - |
| 57B3 | 57B3-57CC | 26 | 56E9 6B9B 6CE8 | NO | PARZIALE | N5 | nemici_tipi_10_13 | - |
| 57CD | 57CD-57D3 | 7 | 4F0F | NO | SI | N3 | nemici_tipo09 | - |
| 57E4 | 57E4-57FA | 23 | 57CD | NO | SI | N3 | nemici_tipo09 | - |
| 57FB | 57FB-5809 | 15 | 57CD | NO | SI | N3 | nemici_tipo09 | - |
| 580A | 580A-582A | 33 | 57CD | NO | SI | N3 | nemici_tipo09 | - |
| 582B | 582B-5893 | 105 | 57CD | NO | PARZIALE | N3 | nemici_tipo09 | - |
| 5894 | 5894-58A2 | 15 | 580A 582B | NO | SI | N3 | nemici_tipo09 | ingresso interno (coda condivisa) |
| 58A3 | 58A3-58CE | 44 | 57CD | NO | SI | N3 | nemici_tipo09 | - |
| 58CF | 58CF-5916 | 72 | 5693 5749 58A3 | PARZIALE | PARZIALE | N1 | nemici_tipo09 | - |
| 5917 | 5917-5927 | 17 | 5693 5749 58A3 | SI | SI | N1 | nemici_tipo09 | - |
| 5928 | 5928-594E | 39 | 5693 5749 58CF 5995 | PARZIALE | SI | N1 | nemici_tipo09 | - |
| 594F | 594F-5994 | 70 | 57CD | NO | SI | N3 | nemici_tipo09 | - |
| 5995 | 5995-5BFC | 597 | 57CD | NO | PARZIALE | N3 | nemici_tipo09 | - |
| 5B09 | 5B09-5B11 | 9 | 5995 | NO | NO | N3 | nemici_tipo09 | - |
| 5B12 | 5B12-5B1B | 10 | 5995 | NO | SI | N3 | nemici_tipo09 | - |
| 5C19 | 5C19-5C1D | 5 | 5995 | NO | SI | N3 | nemici_tipo09 | - |
| 5C1E | 5C1E-5C22 | 5 | 5995 | NO | SI | N3 | nemici_tipo09 | - |
| 5C23 | 5C23-5C28 | 6 | 5995 | NO | SI | N3 | nemici_tipo09 | - |
| 5C29 | 5C29-5C2C | 4 | 5995 | NO | SI | N3 | nemici_tipo09 | - |
| 5C2D | 5C2D-5C31 | 5 | 5C23 5C29 | NO | SI | N3 | nemici_tipo09 | ingresso interno (coda condivisa) |
| 5CFB | 5CFB-5D28 | 46 | 4F0F | NO | SI | N8 | nemici_tipo07a | - |
| 5D29 | 5D29-5DDE | 172 | 5CFB | NO | PARZIALE | N8 | nemici_tipo07a | - |
| 5D76 | 5D76-5D7F | 10 | 5D29 | NO | SI | N8 | nemici_tipo07a | - |
| 5DDF | 5DDF-5E08 | 42 | 5E1A | NO | PARZIALE | N8 | nemici_tipo07a | - |
| 5E09 | 5E09-5E19 | 17 | 5DDF | NO | PARZIALE | N8 | nemici_tipo07a | - |
| 5E1A | 5E1A-5E5F | 43 | 5CFB | NO | PARZIALE | N8 | nemici_tipo07a | - |
| 5E4D | 5E4D-5E5B | 15 | 5E60 5E9D 5EDD 5F33 5FC8 6151 6165 618B 6196 61B1 61FA 6221 | NO | SI | N8 | nemici_tipo07a | - |
| 5E60 | 5E60-5E8E | 47 | 5E1A | NO | PARZIALE | N8 | nemici_tipo07a | - |
| 5E8F | 5E8F-5E9C | 14 | 5E9D 5EDD 5F4B 5F82 5FDC 6046 | NO | PARZIALE | N8 | nemici_tipo07a | - |
| 5E9D | 5E9D-5EDC | 64 | 5E1A | NO | PARZIALE | N8 | nemici_tipo07a | - |
| 5EDD | 5EDD-5F0B | 47 | 5E1A | NO | NO | N8 | nemici_tipo07a | - |
| 5F0C | 5F0C-5F32 | 39 | 5F33 5FC8 | NO | SI | N8 | nemici_tipo07a | - |
| 5F33 | 5F33-5F81 | 37 | 5E1A | NO | PARZIALE | N8 | nemici_tipo07a | - |
| 5F4B | 5F4B-5F74 | 42 | 5F33 | NO | PARZIALE | N8 | nemici_tipo07a | - |
| 5F82 | 5F82-5FAB | 42 | 5F33 | NO | NO | N8 | nemici_tipo07a | - |
| 5FAC | 5FAC-5FC7 | 28 | 5F4B 5F82 5FDC 6046 | NO | NO | N11 | nemici_tipo07b | ingresso interno (coda condivisa) |
| 5FC8 | 5FC8-6045 | 40 | 5E1A | NO | PARZIALE | N11 | nemici_tipo07b | - |
| 5FDC | 5FDC-6031 | 86 | 5FC8 | NO | PARZIALE | N11 | nemici_tipo07b | - |
| 6046 | 6046-608B | 70 | 5FC8 | NO | NO | N11 | nemici_tipo07b | - |
| 610C | 610C-6130 | 37 | 5CFB | NO | PARZIALE | N11 | nemici_tipo07b | - |
| 6151 | 6151-6164 | 20 | 610C 61B1 | NO | SI | N11 | nemici_tipo07b | - |
| 6165 | 6165-6178 | 20 | 610C 61B1 | NO | SI | N11 | nemici_tipo07b | - |
| 6179 | 6179-6182 | 10 | 610C | NO | PARZIALE | N11 | nemici_tipo07b | - |
| 6183 | 6183-618A | 8 | 610C | NO | NO | N11 | nemici_tipo07b | - |
| 618B | 618B-6195 | 11 | 6179 6183 | NO | NO | N11 | nemici_tipo07b | ingresso interno (coda condivisa) |
| 6196 | 6196-61B0 | 27 | 610C 6179 6183 61B1 | NO | SI | N11 | nemici_tipo07b | - |
| 61B1 | 61B1-61F9 | 73 | 610C 6261 6270 | NO | PARZIALE | N11 | nemici_tipo07b | - |
| 61FA | 61FA-6220 | 39 | 610C 6261 | NO | NO | N11 | nemici_tipo07b | - |
| 6221 | 6221-6247 | 39 | 610C 6270 | NO | NO | N11 | nemici_tipo07b | - |
| 6248 | 6248-6251 | 10 | 6179 6183 | NO | SI | N11 | nemici_tipo07b | - |
| 6252 | 6252-6260 | 15 | 6151 6165 | NO | PARZIALE | N11 | nemici_tipo07b | - |
| 6261 | 6261-626F | 15 | 610C | NO | NO | N11 | nemici_tipo07b | - |
| 6270 | 6270-627E | 15 | 610C | NO | NO | N11 | nemici_tipo07b | - |
| 627F | 627F-62B5 | 55 | 5E9D 610C | NO | SI | N11 | nemici_tipo07b | - |
| 6B43 | 6B43-6B58 | 22 | 4F0F | NO | SI | N9 | nemici_tipi_15_17 | - |
| 6B5F | 6B5F-6B8E | 48 | 6B43 | NO | SI | N9 | nemici_tipi_15_17 | - |
| 6B8F | 6B8F-6B99 | 11 | 6B43 | NO | NO | N9 | nemici_tipi_15_17 | - |
| 6B9B | 6B9B-6BC4 | 42 | 6B43 | NO | SI | N9 | nemici_tipi_15_17 | - |
| 6BCD | 6BCD-6C0A | 62 | 5584 6B5F | NO | SI | N5 | nemici_tipi_10_13 | - |
| 6C0B | 6C0B-6C0E | 4 | 4F0F | NO | SI | N9 | nemici_tipi_15_17 | - |
| 6C1D | 6C1D-6C2B | 15 | 55B2 6C0B | NO | PARZIALE | N5 | nemici_tipi_10_13 | - |
| 6C5F | 6C5F-6C8D | 47 | 6C0B | NO | SI | N9 | nemici_tipi_15_17 | - |
| 6C8E | 6C8E-6CE7 | 90 | 6C0B | NO | PARZIALE | N9 | nemici_tipi_15_17 | - |
| 6CE8 | 6CE8-6D03 | 28 | 4F0F | NO | PARZIALE | N2 | nemici_tipi_08_14 | - |
| 6D12 | 6D12-6D1A | 9 | 6CE8 | NO | SI | N2 | nemici_tipi_08_14 | - |
| 6D1B | 6D1B-6D36 | 28 | 6CE8 | NO | PARZIALE | N2 | nemici_tipi_08_14 | - |
| 6D37 | 6D37-6DCD | 151 | 6C0B 6CE8 | NO | PARZIALE | N2 | nemici_tipi_08_14 | - |
| 6DCE | 6DCE-6E54 | 135 | 6C0B 6CE8 | NO | PARZIALE | N2 | nemici_tipi_08_14 | - |
| 6E55 | 6E55-6E82 | 46 | 6CE8 | NO | PARZIALE | N2 | nemici_tipi_08_14 | - |
| 6E83 | 6E83-6E8A | 8 | 6C8E 6E55 | NO | NO | N2 | nemici_tipi_08_14 | ingresso interno (coda condivisa) |
| 6E8B | 6E8B-6E8F | 5 | 6C8E 6E55 6E83 | NO | SI | N2 | nemici_tipi_08_14 | ingresso interno (coda condivisa) |
| 6E90 | 6E90-6EA0 | 17 | 6C8E 6E55 | NO | SI | N2 | nemici_tipi_08_14 | - |
| 6EA1 | 6EA1-6EB6 | 22 | 6C0B 6CE8 | NO | PARZIALE | N2 | nemici_tipi_08_14 | - |
| 6EB7 | 6EB7-6EC0 | 10 | 5749 5894 58A3 6DCE 6EA1 719E | NO | SI | N3 | nemici_tipo09 | - |
| 6EC1 | 6EC1-6EC9 | 9 | 6DCE 6EB7 | NO | SI | N3 | nemici_tipo09 | - |
| 6ECA | 6ECA-70A7 | 165 | 4F0F | NO | PARZIALE | N2 | nemici_tipi_08_14 | - |
| 6F49 | 6F49-6F4A | 2 | 6ECA | NO | SI | N2 | nemici_tipi_08_14 | - |
| 6F4B | 6F4B-6F5D | 19 | 6ECA | NO | SI | N2 | nemici_tipi_08_14 | - |
| 6F5E | 6F5E-6F6D | 16 | 6ECA | NO | SI | N2 | nemici_tipi_08_14 | - |
| 6F6E | 6F6E-6F80 | 19 | 6ECA | NO | SI | N2 | nemici_tipi_08_14 | - |
| 6F81 | 6F81-6F90 | 16 | 6ECA | NO | SI | N2 | nemici_tipi_08_14 | - |
| 6F91 | 6F91-6FB2 | 34 | 6F4B 6F5E 6F6E 6F81 | NO | PARZIALE | N2 | nemici_tipi_08_14 | ingresso interno (coda condivisa) |
| 6FB3 | 6FB3-6FC2 | 16 | 6F91 | NO | PARZIALE | N2 | nemici_tipi_08_14 | - |
| 6FC3 | 6FC3-6FD0 | 14 | 6F91 | NO | NO | N2 | nemici_tipi_08_14 | - |
| 6FD1 | 6FD1-6FDE | 14 | 6F91 | NO | PARZIALE | N2 | nemici_tipi_08_14 | - |
| 6FDF | 6FDF-6FEC | 14 | 6F91 | NO | NO | N2 | nemici_tipi_08_14 | - |
| 6FED | 6FED-6FF6 | 10 | 6ECA | NO | SI | N2 | nemici_tipi_08_14 | - |
| 6FF7 | 6FF7-7000 | 10 | 6ECA | NO | PARZIALE | N2 | nemici_tipi_08_14 | - |
| 7001 | 7001-700A | 10 | 6ECA | NO | SI | N2 | nemici_tipi_08_14 | - |
| 700B | 700B-7014 | 10 | 6ECA | NO | SI | N2 | nemici_tipi_08_14 | - |
| 7015 | 7015-7024 | 16 | 6ECA 7001 | NO | SI | N2 | nemici_tipi_08_14 | - |
| 7025 | 7025-7035 | 17 | 6ECA 700B | NO | SI | N2 | nemici_tipi_08_14 | - |
| 7036 | 7036-7044 | 15 | 6ECA 7001 | NO | SI | N2 | nemici_tipi_08_14 | - |
| 7045 | 7045-7055 | 17 | 6ECA 700B | NO | SI | N2 | nemici_tipi_08_14 | - |
| 70A8 | 70A8-70AF | 8 | 6F4B 6F5E 6F6E 6F81 7015 7025 7036 7045 | NO | SI | N2 | nemici_tipi_08_14 | - |
| 70B0 | 70B0-70CD | 30 | 6FED 70A8 | NO | SI | N2 | nemici_tipi_08_14 | - |
| 70CE | 70CE-70ED | 32 | 6FF7 70A8 | NO | SI | N2 | nemici_tipi_08_14 | - |
| 70FE | 70FE-714A | 77 | 6ECA | NO | SI | N2 | nemici_tipi_08_14 | - |
| 714B | 714B-714E | 4 | 4F0F | SI | SI | N1 | nemici_tipi_10_13 | - |
| 7155 | 7155-7195 | 65 | 714B | PARZIALE | SI | N1 | nemici_tipi_10_13 | - |
| 719E | 719E-7234 | 151 | 714B | NO | PARZIALE | N5 | nemici_tipi_10_13 | - |
| 7235 | 7235-7287 | 83 | 714B | PARZIALE | SI | N1 | nemici_tipi_10_13 | - |
| 7288 | 7288-728B | 4 | 4F0F | NO | SI | N4 | nemici_comparsa | - |
| 7294 | 7294-72B5 | 34 | 7288 | NO | SI | N4 | nemici_comparsa | - |
| 72B6 | 72B6-72CD | 24 | 7288 | NO | SI | N4 | nemici_comparsa | - |
| 72CE | 72CE-72D9 | 12 | 7288 | NO | SI | N4 | nemici_comparsa | - |
| 72DA | 72DA-734C | 115 | 7288 | NO | PARZIALE | N4 | nemici_comparsa | - |
| 734D | 734D-7378 | 44 | 719E 7235 72B6 72DA | PARZIALE | PARZIALE | N1 | nemici_comparsa | - |
| 738F | 738F-73A2 | 20 | 4F40 73A3 | NO | SI | N4 | nemici_comparsa | - |
| 73A3 | 73A3-73B6 | 20 | 4F0F | NO | PARZIALE | N4 | nemici_comparsa | - |
| 73C7 | 73C7-7411 | 75 | 73A3 | NO | NO | N4 | nemici_comparsa | - |
| 7412 | 7412-741C | 11 | 73A3 | NO | SI | N4 | nemici_comparsa | - |
| 741D | 741D-7455 | 57 | 73A3 | NO | SI | N4 | nemici_comparsa | - |
| 7456 | 7456-7467 | 18 | 741D 7470 | NO | SI | N4 | nemici_comparsa | ingresso interno (coda condivisa) |
| 7470 | 7470-74AA | 59 | 73A3 | NO | SI | N4 | nemici_comparsa | - |
| 74AB | 74AB-754B | 161 | 73A3 | NO | SI | N4 | nemici_comparsa | - |
| 756C | 756C-758D | 34 | 74AB | NO | SI | N4 | nemici_comparsa | - |
| 758E | 758E-759E | 17 | 74AB | NO | SI | N4 | nemici_comparsa | - |
| 759F | 759F-75A9 | 11 | 74AB | NO | SI | N4 | nemici_comparsa | - |
| 7F48 | 7F48-7F50 | 9 | 7F68 | NO | SI | N9 | nemici_tipi_15_17 | - |
| 7F51 | 7F51-7F67 | 23 | 7F68 | PARZIALE | SI | N1 | nemici_tipi_15_17 | - |
| 7F68 | 7F39-7F72 | 12 | 4F0F | PARZIALE | SI | N1 | nemici_tipi_15_17 | salta (jr z) a 7F39, sito rst 38 dentro sub_7f2d già tradotta |
| 7F81 | 7F81-7F89 | 9 | 7F68 | NO | NO | N9 | nemici_tipi_15_17 | - |
| 7F8A | 7F8A-7FA5 | 28 | 7F68 | NO | SI | N9 | nemici_tipi_15_17 | - |
| 7FA6 | 7FA6-7FCE | 41 | 7F68 | NO | SI | N9 | nemici_tipi_15_17 | - |
| 7FCF | 7FCF-801C | 78 | 7F68 | NO | PARZIALE | N9 | nemici_tipi_15_17 | - |
| 801D | 801D-8041 | 37 | 7F68 | NO | NO | N9 | nemici_tipi_15_17 | - |
| 8042 | 8042-8088 | 71 | 7F68 | NO | PARZIALE | N9 | nemici_tipi_15_17 | - |
| 808D | 808D-809B | 15 | 7FCF 8042 | NO | NO | N9 | nemici_tipi_15_17 | ingresso interno (coda condivisa) |
| 809C | 809C-80A4 | 9 | 7F51 7FCF 801D 808D | NO | SI | N9 | nemici_tipi_15_17 | ingresso interno (coda condivisa) |
| 80A5 | 80A5-80B2 | 14 | 4F40 551A 571D 6C1D 72DA 73C7 7F81 | NO | SI | N4 | nemici_comparsa | ingresso interno (coda condivisa) |

## Punti aperti

- **`jp (hl)`/`jp (ix)`/`jp (iy)`**: nessuno nel cammino.
- **Lunghezze delle tabelle**: 19 tabelle su 21 si chiudono solo per
  euristica. Le uniche osservate oltre la prima voce sono 4DA6 (fino a 2 su
  3), 4F1B (17 su 18), 55B5 (5 su 7), 714E (2 su 3) e 7F39 (4 su 7). Una voce
  in più alla fine di una tabella vorrebbe dire che il primo byte di codice
  dopo la tabella è letto come parola: il controllo a mano sui confini lo
  esclude, ma una tabella più corta del vero non si vede finché non va in
  onda. Il lotto che la traduce la riverifica in cattura.
- **7F68** salta (`jr z`) a 7F39, che è il sito `rst 38` dentro `sub_7f2d`,
  già tradotta. Il C di 7F68 deve rifare lo smistamento di 7F39, con la
  tabella `rst38_7F39`, oppure fattorizzarlo con `sub_7f2d`.
  **Risolto in N1** (brief 25): 7F39 non sta dentro `sub_7f2d`, che finisce
  col `ret` a 7F38; 7F39 è l'etichetta subito dopo e ci si arriva solo dal
  `jr z` di 7F68. Lo smistamento delle due tabelle 7F39 e 7F72 è in
  `sub_7f68` (`nemici_tipi_15_17.c`), `sub_7f2d` non cambia.
- **3B62** è un `ret` solo, la voce nulla di molte tabelle: è tradotta già
  in N1 come `sub_3b62` vuota in `nemici.c` (non è uno stub) e N7 la trova
  fatta. Gli stub di N1 sono quindi 27, in `harness/nemici_stub.c`.
- **Ritorni spinti in pila**: `ld hl,nn; push hl; rst 38/20` a 4FA8
  (ritorno 4FC2) e a 6F18 (ritorno 7056). Il ritorno è codice della stessa
  routine, e il cammino lo segue.
- **Routine in comune con i corpi 01 e 10**: 2474, 2482, 248D, 24BE, 4592,
  4938, 49AE, 4ADA, 4B20, 4C11. Sono tutte in `nemici_comuni.c`, e il lotto
  dei corpi 01/10 (REPLAY.md, «Cose da sistemare» punto 2) le troverà già lì
  se N1 lo precede. Nel replay sono coperte, ma la traccia non dice se le
  esegua 0A o 01/10.
- **4FC5**: tipi 0 (4F40) e 4 (73C7). Dal vivo `DEST 4FC5` compare 6 volte
  in `out/partita_viva_20261027.log` e 1 in `..._27b.log`. Probabile gestore
  della comparsa di un'entità (fata); nel replay non va in onda.

## Piano dei lotti

Metro: 0,6 righe C per byte Z80 (`attivita.c`). Il totale di 8833 B fa
circa 5300 righe, quindi servono **11 moduli**, non i 3-4 attesi dal brief
con il perimetro di ~3600 B. Il modulo più grande arriva a 660 righe
stimate, sotto la soglia di 700.

Controlli (stampati dallo script):
- ogni routine sta in esattamente un lotto;
- N1 contiene tutte e sole le routine coperte più 4DA2;
- la somma dei byte dei lotti fa 8833, pari al totale;
- il massimo per modulo è 660 righe, sotto 700.

**Lotto N1**, il corpo 0A e le routine coperte dal replay: 27 routine, 1564
B, circa 938 righe. La stima sta sotto le 1200 righe, quindi N1 resta un
lotto unico (risoluzione 3, nessuna spezzatura in N1a/N1b). È l'insieme che
il giro 2410-5500 esegue: tradurlo sblocca il replay oltre il frame 2410.
La verifica è il replay stesso, con lo stato «tradotta (replay)» in una
sezione separata di `ROUTINE.md`. Ingressi per modulo:
- `nemici.c`: 4DA2, 4DAD, 4EE6, 4F0F, 4632
- `nemici_comuni.c`: 2482, 248D, 24BE, 4592, 4938, 49AE, 4B20, 4C11
- `nemici_tipi_10_13.c`: 55B2, 55C9, 561B, 5640, 5693, 714B, 7155, 7235
- `nemici_tipo09.c`: 58CF, 5917, 5928
- `nemici_comparsa.c`: 734D
- `nemici_tipi_15_17.c`: 7F51, 7F68

Le routine PARZIALE si traducono intere. Le 28 routine non coperte che N1
chiama o smista diventano stub che fermano il replay con il messaggio
«routine mancante» (elenco in fondo alla tabella dei moduli). Il replay non
le raggiunge.

**Lotto N2 fatto** (brief 27, 2026-10-28): i tipi 8 (6ECA) e 14 (6CE8)
in `nemici_tipi_08_14.c`, 33 ingressi (6E83 e 6E8B dentro `sub_6e55`,
6FC3 e 6FDF statiche, la coda 7056 statica). Con N2 sono state anticipate
6 dipendenze di altri lotti, ognuna nel modulo prescritto: 571D, 57AA,
57B3 da N5 (`nemici_tipi_10_13.c`), 80A5 da N4 (`nemici_comparsa.c`), 6EB7
e 6EC1 da N3 (`nemici_tipo09.c`). Da N2 in poi i moduli `nemici*.c`
entrano anche nel banco (`harness`) e ogni lotto si verifica con catture
MAME (stato `stanzaNN` di `cattura.sh`, forzature `SNAP_FORZA`); elenco e
stati in `ROUTINE.md`, sezione «Motore dei nemici (lotto N2)».

**Lotto N3 fatto** (brief 29, 2026-10-29): il tipo 9 (57CD) in
`nemici_tipo09.c` (57CD, 57E4, 57FB, 580A, 582B, 58A3, 594F, 5894 statica) e
`nemici_tipo09b.c` (5995, 5B12, 5C19, 5C1E, 5C23, 5C29, 5B09 e 5C2D statiche),
diviso in due per stare sotto le 700 righe. Con N3 sono state anticipate 5
dipendenze di altri lotti, ognuna nel modulo prescritto: 52FD, 5345, 54C5 da N6
(`nemici_tipo06b.c`) e 51D7, 24DF da N10 (`nemici_tipo06a.c`). 18 voci nuove
nel banco; 5B09 senza cattura (mai eseguita). Il registro R dei due `ld a,r`
(54C5, 59E5) non sta nelle catture: lo cerca il banco (`nemici_r.c`), vedi
`ROUTINE.md`, sezione «Motore dei nemici (lotto N3)». N6 e N10 non hanno più
da tradurre quelle cinque routine.

**Lotto N4 fatto** (brief 30, 2026-10-29): la comparsa (tipi 0, 4 e 5 di 4F1B),
28 routine, divisa in `nemici_comparsa.c` (4F40, 4FC5, 738F, 13xx/14xx, 1549 e
le 734D e 80A5 già tradotte) e `nemici_comparsab.c` (7288, 73A3 e le loro fasi,
756C, 758E, 759F, 2548), per stare sotto le 700 righe. 24 voci nuove nel banco;
14FC, 4FC5 e 73C7 (e la coda 7456) sono tradotte senza voce propria, perché
nessuna sessione le ha mai eseguite: le coprono le catture forzate di 4F40 e
73A3 e gcov. Nuovo script `cattura_oggetti.sh` per lo stato «oggetti» (4F40 e
13xx/14xx), vedi `ROUTINE.md`, sezione «Motore dei nemici (lotto N4)».

**Lotto N5 fatto** (brief 31, 2026-10-29): i tipi 10-13 di 4F1B, 14 routine,
divise fra `nemici_tipi_10_13.c` (55C4, 6C1D, 719E) e `nemici_tipi_10_13b.c`
(le altre 11: tipo 13 con 6BCD e tipo 11), per stare sotto le 700 righe. 14 voci
nuove nel banco. `sub_6c2c` (già verificata) ora imposta il riporto in F, che 551A
e 6C1D leggono; le sue catture restano OK. Vedi `ROUTINE.md`, sezione «Motore dei
nemici (lotto N5)».

**Lotto N6 fatto** (brief 32, 2026-10-29): tipo 6 seconda metà, 8 routine
(5274, 5289, 5292, 52AA, 52C2, 52DB, 52EC, 531C) in `nemici_tipo06b.c`, più
le 5 anticipate da N10 (51FA, 5224, 523C, 5254, 5068), che N10 non deve più
fare, in `nemici_tipo06a.c`. 6 voci nuove nel banco (51FA con il wrapper
`banco_r_51fa`). Convenzione di 531C: riporto 1 in `z->f` = uscita anomala
(cella piena, nell'originale `pop hl; jp 5068`), il chiamante (50BA, 50DF)
deve ritornare subito; riporto 0 = uscita normale. Vedi `ROUTINE.md`, sezione
«Motore dei nemici (lotto N6)».

**Lotto N7 fatto** (brief 33, 2026-10-29): corpo 0A, 5 routine (4649, 46A3,
470A, 4717, 4767) in coda a `nemici.c`; 3B62, la sesta, era già fatta in N1.
Gli stub di `nemici_stub.c` scendono da 16 a 14 (tolti 4649 e 46A3). 4767 è
condivisa con 37B4 (non tradotta, fuori dal censimento). 5 voci nuove nel
banco, 35 catture. Vedi `ROUTINE.md`, sezione «Motore dei nemici (lotto N7)».

**Lotto N8 fatto** (brief 34, 2026-10-30): tipo 07 prima metà, 15 routine
(5CFB, 5D29, 5D76, 5DDF, 5E09, 5E1A, 5E4D, 5E60, 5E8F, 5E9D, 5EDD, 5F0C,
5F33, 5F4B, 5F82) in `nemici_tipo07a.c`, più due anticipate: 4FEF (da N10, in
`nemici_tipo06a.c`) e 5FAC (da N11). Gli stub di `nemici_stub.c` salgono da
14 a 16 (tolto 5cfb; aggiunti 610c, 5fc8 e 627f). 14 voci nuove nel banco,
84 catture. N10 e N11 non rifanno 4FEF e 5FAC. Vedi `ROUTINE.md`, sezione
«Motore dei nemici (lotto N8)».

**Lotti N2-N12**: le routine non coperte, un lotto per modulo. Sono in
ordine di utilità, cioè di byte eseguiti nell'unione storica delle sessioni.
Ingressi completi in `out/censimento_nemici_lotti.txt`.

| lotto | routine | byte | righe C stimate | moduli |
|---|---:|---:|---:|---|
| N1 | 27 | 1564 | 938 | nemici.c, nemici_comparsa.c, nemici_comuni.c, nemici_tipi_10_13.c, nemici_tipi_15_17.c, nemici_tipo09.c |
| N2 | 33 | 1030 | 618 | nemici_tipi_08_14.c |
| N3 | 18 | 972 | 583 | nemici_tipo09.c |
| N4 | 29 | 996 | 598 | nemici_comparsa.c |
| N5 | 17 | 632 | 379 | nemici_tipi_10_13.c |
| N6 | 11 | 612 | 367 | nemici_tipo06b.c |
| N7 | 6 | 482 | 289 | nemici.c |
| N8 | 15 | 677 | 406 | nemici_tipo07a.c |
| N9 | 16 | 561 | 337 | nemici_tipi_15_17.c |
| N10 | 27 | 647 | 388 | nemici_tipo06a.c |
| N11 | 19 | 618 | 371 | nemici_tipo07b.c |
| N12 | 2 | 42 | 25 | nemici_comuni.c |

**Moduli C nuovi** (li creano i lotti futuri). Le dichiarazioni condivise,
cioè prototipi delle routine e stub, vanno in `harness/nemici.h`. Ogni lotto
aggiunge i suoi moduli a `SORGENTI_REPLAY` nel Makefile, e i moduli
includono `rst38_tables.h`. Dal brief 27 il Makefile prende `nemici*.c`
con un `wildcard` sia per `harness` sia per `replay`: un modulo nuovo entra
in entrambi senza toccare il Makefile.

Il gruppo di una routine si assegna così:
- `comuni`: le routine di 01/10;
- `corpo`: le routine raggiunte dal 4DA2 senza passare per le voci del 4F1B;
- altrimenti: il tipo del 4F1B con l'indice più basso che la raggiunge.

Un tipo sopra le 700 righe si spezza per indirizzo (06a/06b, 07a/07b). Il
censimento non identifica ancora i nemici per tipo, per questo i temi sono
gli indici del 4F1B. L'unica ipotesi semantica è `comparsa`, per 4FC5.

| modulo | tema | routine | byte | righe C stimate |
|---|---|---:|---:|---:|
| `nemici.c` | corpo 0A (4DA2), smistatori 4DA6 e 4F1B, ramo 4632, voce nulla 3B62 | 11 | 625 | 375 |
| `nemici_comuni.c` | routine condivise con i corpi 01 e 10 | 10 | 812 | 487 |
| `nemici_comparsa.c` | tipi 0, 4 e 5 di 4F1B; 4FC5 (comparsa) condivisa da 0 e 4 | 30 | 1040 | 624 |
| `nemici_comparsab.c` | seconda parte di `nemici_comparsa.c` (brief 30): 7288, 73A3 e le loro fasi, 756C, 758E, 759F, 2548 | - | - | - |
| `nemici_tipo06a.c` | tipo 6 di 4F1B (4FD4), prima metà per indirizzo | 27 | 647 | 388 |
| `nemici_tipo06b.c` | tipo 6 di 4F1B (4FD4), seconda metà | 11 | 612 | 367 |
| `nemici_tipo07a.c` | tipo 7 di 4F1B (5CFB), prima metà per indirizzo | 15 | 677 | 406 |
| `nemici_tipo07b.c` | tipo 7 di 4F1B (5CFB), seconda metà | 19 | 618 | 371 |
| `nemici_tipi_08_14.c` | tipi 8 (6ECA) e 14 (6CE8) di 4F1B | 33 | 1030 | 618 |
| `nemici_tipo09.c` | tipo 9 di 4F1B (57CD) | 21 | 1100 | 660 |
| `nemici_tipi_10_13.c` | tipi 10-13 di 4F1B (714B, 56E9, 55B2, 54D8) | 25 | 1076 | 646 |
| `nemici_tipi_10_13b.c` | seconda parte di `nemici_tipi_10_13.c` (brief 31): tipo 13 (54D8 e le sue fasi, 6BCD) e tipo 11 (56E9 e le sue fasi) | - | - | - |
| `nemici_tipi_15_17.c` | tipi 15-17 di 4F1B (6C0B, 6B43, 7F68) | 18 | 596 | 358 |

Stub di N1: 2474 3B62 4649 46A3 4ADA 4F40 4FD4 54D8 55C4 56E9 57CD 5CFB 6B43 6C0B 6C1D 6CE8 6ECA 719E 7288 73A3 7F48 7F81 7F8A 7FA6 7FCF 801D 8042 809C
