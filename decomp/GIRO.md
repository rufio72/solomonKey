# Giro dal boot 0-5499 (lotti H4a, H4b1, H4b2, H4b3 e H4c, brief 44-48)

Primo pezzo dell'ex «giro libero»: il C parte dal vettore 0000 e gira senza
che l'oracolo di MAME lo guidi. Dell'oracolo restano il controllo (impronta
della RAM a ogni NMI) e gli ingressi registrati. Il giro si ferma, atteso e
misurabile, alla prima attività non tradotta: in H4a `CORPO 1F (43C8)` al
frame 602, da H4b1 `CORPO 07 (2CCD) non tradotto al frame 1188`, da H4b2
`ROUTINE 2DCE mancante (motore dei nemici) al frame 1398`. Da H4b3 il giro
arriva al frame 2410 senza arresti e si salda all'oracolo storico
(`SALDATURA 2410 ... coincide`). Da H4c (brief 48) prosegue fino al 5499 (sezione in fondo).

## Uso (da `decomp/`)

```
sh replay.sh               # solo se manca out/replay_oracolo.txt
sh estrai_ingressi.sh      # out/ingressi_replay.txt: <frame> <e600> <e601> <e602>
sh giro_boot.sh            # oracolo del boot, ~45 s: out/giroboot_*.txt e dump 602
cd harness && make gioco_libero && cd ..
harness/gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt \
    out/ingressi_replay.txt [frame_fine]
```

Uscita attesa (H4b3): `SANITA 602: ... coincide`, `SALDATURA 2410: ... 0,
coincide`, `spedizioni eseguite: 41396`, `NMI confrontate: 2345`, codice 0.
(All'epoca di H4a: `CORPO 1F (43C8) non tradotto al frame 602`, codice 1.)

## Come funziona

- `harness/macchina.c`: `boot_2833()` trascrive 2833-2959 chiamando le
  routine C già tradotte (più `sub_29cf` e `sub_2a03`, nuove in
  `z80_2xxx.c`); poi lo spedidore di `sched.c`, con l'involucro di `replay.c`
  (slot e codice contro la riga D, corpi da `corpo_attivita_replay[]`).
  SP e registro I non si modellano; IX è una variabile locale (in C i test
  RAM passano sempre, IX = 0, niente nuovo tentativo a 28D9).
- Porte: e604/e605 in lettura valgono 02/00 (DSW della registrazione);
  prima di ogni NMI e600-e602 prendono i valori del frame dal file ingressi
  (frame senza riga = 0). `porte_gancio_scrittura` (porte.c) devia le
  scritture a e600 (maschera NMI, variabile della macchina: a ogni NMI deve
  valere 1) e a e604 (ribaltamento schermo, che non deve sporcare la DSW).
- Dove entra la NMI. La riga F dell'oracolo dà ritorno e impronta per ogni
  frame. Le routine del percorso di avvio chiamano `MACCHINA_PUNTO(da, a)`
  (ram.h, gancio `macchina_punto`, NULL fuori dalla macchina) con
  l'intervallo di indirizzi Z80 in cui la RAM è quella corrente. La NMI
  pendente si esegue al primo punto che contiene il suo ritorno e in cui
  l'impronta del C coincide con quella di MAME; in un tratto a RAM costante
  se ne eseguono di seguito quante coincidono. Oltre 40000 punti senza
  coincidenza il giro si ferma con ritorno, hash C e hash MAME.
- Punti inseriti nelle routine verificate (a gancio spento nessun effetto):
  `sub_2bf3`, `sub_2bc9`, `sub_2c1a`, `sub_2b08`, `sub_2af5`, `sub_2ae2`,
  `sub_2bc3` (z80_2xxx.c) e `sub_3a54` (z80_3xxx.c), 29 chiamate di
  `MACCHINA_PUNTO`; nel codice nuovo `sub_29cf` e `sub_2a03` (47) e i cicli di azzeramento del boot
  (`azzera` in macchina.c, 2 per byte). La lettura di c467 e il confronto
  con E0 sono separati come nell'originale (29F0/29F3, 2AB2/2AB5).
- Impronta: come il replay (FNV-1a su c000-e5ff esclusi c040-c043 e
  cf00-cfff) più dbc0-dbff: fino a 286E la pila sta a dbxx e i frame 66-67
  vi lasciano i residui delle push della NMI, che il C non modella.
  dbc0-dbff è colorram di sfondo, quindi l'esclusione vale solo per
  `giro_boot.lua`; il buco è richiuso dal confronto byte per byte col dump
  del frame 602 (`out/giroboot_602.bin`), fatto prima della NMI di quel
  frame (`SANITA 602`), dove la zona è azzerata dal clear di 28B6.

## H4b1: NMI complete, corpi 1F e 06, sentinella R (brief 45)

Il giro ora prosegue dopo il boot fino al corpo 07: `CORPO 07 (2CCD) non
tradotto al frame 1188`, dopo 16407 spedizioni e 1123 NMI (537 del boot, 586
dopo; 466 nello spedidore e 120 a metà attività). Nessuna divergenza nello
spedidore; 9 a metà attività, tutte nel primo corpo 06 (frame 603-611).

- Ciclo a eventi (`giro_spedidore` in macchina.c): copia di `replay.c`. Il
  registro ha le righe D e N; le N prima di una D vanno nella scansione
  (`gancio_scansione`), la N pendente (ritorno 3AFB-3B10, 0 letture) si
  esegue prima del corpo in `involucro`, le N con 0 letture dopo la D si
  eseguono nel primo punto `NEMICI_PUNTO` che contiene il ritorno e dove
  l'impronta coincide. Differenza dal replay: le N di coda sono una coda
  (il primo corpo 06 ne riceve 9) e `nmi_scan` ne regge 256. Una divergenza
  di una NMI «pulita» (ritorno in 3AF3-3B10 o prima del corpo) ferma il giro
  con la diagnostica; quelle a metà attività si contano.
- Fine del boot: `macchina_punto = NULL`, `nemici_punto`, gancio di
  scansione e corpi passano ai loro valori del giro.
- Corpi (attivita_attract.c): 1F (43C8) copia le tabelle dei canali; 06
  (75E7) è una macchina a fasi sul contatore c1ae, `rst 20` sulla tabella
  75EC: 78E0 (primo quadro), 763E (attesa di c467 ≥ 40) e 760B (fase che
  prepara il quadro e passa l'attività a 07). Le fasi 5 (7A0B) e 8 (7B3B)
  non sono tradotte: `nemici_manca` ferma il giro.
- Punti di sincronia in 06 (`NEMICI_PUNTO`): 75E7-75EB e 0008-0026
  (ingresso e gestore `rst 20`); in 760B gli intervalli fra le scritture di
  c045, c1ae, slot, 7C84, 75AA, 33EB, c25d, c243, c1b7 (con 0010-0014 per il
  `rst 10` di 7634) e c053; in 763E 763E-7646, 7647-7650, 7653-7655,
  7657-7658 e 7659-765A; in 78E0 un intervallo per ogni scrittura e per
  ogni chiamata, 78E0-797D.
- Punti larghi (`PUNTO_LARGO`, gancio `attract_punto_largo`, NULL fuori
  dalla macchina): le NMI con ritorno dentro routine tradotte senza punti
  interni (75B4 in `sub_75aa`, 3291-3396 in 322A, 7F03/7F10 in 7ECA) si
  eseguono al confine della routine senza controllo d'impronta. Servono
  solo al primo corpo 06, dove nove NMI cadono in una sola spedizione;
  senza di loro c467 e c29a differiscono al frame 612 (NMI pulita). La
  NMI 603 (75B4) va prima di `sub_75aa`, perché la routine azzera c299-c2a8;
  le altre dopo la routine che le contiene.
- F000-FFFF è ROM (tabelle di fondali e palette) ma `leggi()` la cerca in
  `ram[]`: la macchina la copia dalla ROM prima del boot (il replay la ha
  nel dump). Senza, la palette di 33BA veniva letta a zero.
- Registro R: `sh registra_r.sh` ripete l'oracolo con un breakpoint
  sull'istruzione dopo ogni `ld a,r` (2D1F 2D22 430A 430D 54C5 54C8 59E5
  59E8 5DDF 5DE2) e scrive `out/regr_r.txt` (`R <frame> <PC> <A>`).
  Misura: 0 letture nei frame 0-5499 con gli ingressi registrati
  (controllo positivo: `REGR_SITI=3AF9 REPLAY_FINE=700` dà `R 602 3AF9 1F`).
  Il C non ha valori di R: `nemici_r_gancio` (nemici_r.c, NULL nel banco e
  nel replay) lo aggancia a `r_letto`, che ferma il giro con `R letto prima
  della NMI del frame N`. Se una registrazione futura (partita libera,
  H5) mostra letture, `nemici_r_somma` dovrà consumare quel file.
- `giro_boot.sh` ora arriva al frame 2411 con dump al 2410
  (`out/giroboot_2410.bin`, identico a `replay_2410.bin`): serve al confronto
  finale di H4b3.
- Spezzatura di H4b: H4b1 questo lotto; H4b2 corpo 07 (~690 istruzioni,
  macchina a fasi con tre `rst 20`, routine 2D8A, 3110, 37B4, 38C3 non
  tradotte); H4b3 corpi 08, 0C, 0B (~134) e confronto byte per byte al 2410
  (esclusi solo c040-c043 e cf00-cfff). La regione 6253-673F passa a H4c.

## Limiti

- `sub_2a03` non traduce il ramo di guasto 2AC8 (IX & 7F ≠ 0): esce con
  un avviso. `sub_29cf` e `sub_2a03` non sono catturabili dal banco (durano
  224 frame): la verifica è questo giro.

## H4b2: corpo 07 (2CCD), fasi 0-12 e routine 3ADF (brief 46)

- `attivita_attract07.c` (ingresso, fasi 2, 5, 6, 8-12 con la tabella `rst 20`
  di 2DB8 sul sotto-contatore c19e), `attivita_attract07a.c` (fasi 0, 1, 3),
  `attivita_attract07b.c` (fasi 4 e 7), intestazione `attivita_attract07.h`
  (`PUNTO_LARGO`). `[0x07] = attivita_07_2ccd` in `corpo_attivita_replay`.
  `sub_3adf` in `z80_38xx.c` (2 catture, entra nel banco: 305 voci, 1272 catture).
- Istruzioni tradotte: ingresso e fasi 0-12 come da brief (501 stimate). I rami
  mai eseguiti nei frame 0-2410 chiamano `nemici_manca(indirizzo)` (386A, 3874,
  38BA, 38CA, 38D9, 3901, 3903, 3960, 398B, 3986, 39B9, 351E, 353B, 352E,
  365F, 366C, 2D0F, 2E04; 351E, 353B e 352E tradotti nel lotto 54); il bit 4 di c045 in 2D07 passa da
  `nemici_r_somma()` (sentinella) e poi `nemici_manca(0x2d1f)`.
- Punti `NEMICI_PUNTO`: uno per scrittura in RAM o chiamata, secondo la regola
  di `nemici.h`; nei cicli (fase 0 d840 e c251, fase 2, fase 3, fase 4
  3642-3668, `ldir` di 34FA, fase 7 dc80/d780) dentro il ciclo; le attese
  (2E89, 2E5D, 2E79, 2E71, 2E7D) in `attesa()`, che rifà inline `sub_2e90`
  (stessa semantica) per mettere i punti fra il decremento, l'azzeramento di
  c467 e l'incremento (ritorni 2E8E, 2E8F, 2E90); `NEMICI_PUNTO(0x0008,
  0x0026)` per il gestore dei `rst 20`, `(0x0008, 0x0014)` per `rst 08/10` in 358F.
- `PUNTO_LARGO`: 3ADF (3ADF-3AF2, fase 7, due chiamate), 2F57 (2F57-2F5F, fase 9),
  3A54 (3A54-3A7C, fasi 9 e 12), 322A e figlie (3225-33B9, fase 6).
- Deviazione dal modello di 78E0: la fase 6 non chiama `sub_322a` intera ma
  `fondale_322a()`, copia della stessa sequenza con il `PUNTO_LARGO` prima di
  `sub_33ba`. 322A dura sette frame (1189-1195); `sub_33ba` accende il bit 6 di
  c044 (consegna della palette alla NMI) e, con le NMI dei frame 1190-1195
  eseguite a flag già acceso, la palette partiva sei frame prima di MAME
  (divergenza nello spedidore al frame 1196: c044 04 contro 44, e500-e52f).
- Esito del giro: `ROUTINE 2DCE mancante (motore dei nemici) al frame 1398`
  (fase 13), 21770 spedizioni, 1333 NMI confrontate (615 nello spedidore, 0
  divergenti; 181 a metà attività, 15 divergenti: quelle con ritorno dentro
  routine tradotte senza punti interni, come 322A, sono eseguite al confine
  della routine).
- Restano a H4b3: fasi 13-25 (gestori 2DCE, 31C5, 2E0A, 2E9C, 3709, 3829, 321E,
  2EF5, 2E6D, 2F1A, 2E1E; la voce 16, 2D9D, non è mai eseguita), 37B4, i corpi 08,
  0C e 0B e il confronto al frame 2410.

## H4b3: corpo 07 (fasi 13-25), corpi 08, 0C, 0B e saldatura al 2410 (brief 47)

- Moduli: `attivita_attract07c.c` (fase 13 con la tabella `rst 20` 2DDA di sette
  voci: 3009, 305F, 3131, attesa 2E65, 3161, attesa 2E81, 31B7; fasi 14, 15,
  17 e 21), `attivita_attract07d.c` (fasi 18, 19, 20, 22, 24, 25; la 23 è
  l'attesa 2E6D nel `switch`), `attivita_attract08.c` (corpi 08, 0C, 0B). La
  fase 16 (2D9D) non è mai eseguita nei frame 0-2410 e resta
  `nemici_manca(0x2d9d)`. `attesa()` di 07.c è ora `attesa_07()` (esportata) e
  `fondale_322a()` serve anche al corpo 08. `[0x08]`, `[0x0b]`, `[0x0c]` in
  `corpo_attivita_replay[]`.
- Routine nuove nel banco: `sub_37b4` (`z80_38xx.c`, 4 catture) e `sub_2d8a`
  (`z80_2dxx.c`, 2 catture, `att0B` banale: i due blocchi coincidono; la
  cattura `gioco 2 n2` non esiste, MAME non arriva alla seconda
  occorrenza). Banco: 307 voci, 1278 catture, 0 fallite.
- Saldatura (`macchina.c`): `diff_dump(frame, pila)`; con `pila = 1` il
  confronto include dbc0-dbff e salta solo c040-c043 e cf00-cfff. `saldatura()`
  gira prima della NMI del frame 2410 (`FRAME_SALDATURA`). Esito:
  `SALDATURA 2410: byte diversi in c000-e5ff (escluse c040-c043, cf00-cfff):
  0, coincide`, 41396 spedizioni, 2345 NMI confrontate (30 divergenti, tutte a
  metà attività: le 15 di H4b1/H4b2 più 1532-1541 nel corpo 08 e 1565-1569
  nella fase 6 della seconda corsa, ritorni dentro 75AA, 322A, 33BA, 7ECA e
  nel decodificatore 23xx, senza punti interni: `PUNTO_LARGO`), nello
  spedidore 1132 con 0 divergenti, a metà attività 676.
- Seconda corsa del corpo 07: il corpo 0B (frame 1562) azzera c19d e rimette 07
  in coda; le fasi si ripassano dal frame 1562 al 1884 alternate al corpo 14
  (una spedizione per ciascuno a ogni giro), come mostra il registro.
- Sequenza reale dei fermi (le fasi 13-25 e i tre corpi sono stati tradotti in
  un colpo solo, quindi non si è visto nessun `ROUTINE mancante` intermedio):
  1414 (NMI pulita, ritorni dentro 3097), 1544 (c29a: la NMI 1533 va dopo
  75AA), 1814 (NMI nel corpo 14 senza punti), 1838 (ritorni 0010 dentro 452B),
  2012 (NMI 2010 dentro 5693 dopo 7379), poi il giro arriva in fondo.
- Punti `NEMICI_PUNTO`/`PUNTO_GIRO` aggiunti: i nuovi moduli ne hanno uno
  per scrittura o chiamata come da regola di `nemici.h`; per i ritorni delle
  NMI dentro routine già verificate sono stati aggiunti ganci a puntatore
  NULL con la nuova macro `PUNTO_GIRO` (nemici.h), attiva solo se
  `gioco_libero.c` accende `giro_dal_boot`: il replay storico non ne risente
  (885 divergenze a metà attività, prima al frame 2415, come prima). Dove:
  `sub_3097` (z80_3xxx.c, 16 oggetti: i ritorni 30AA-310F sono la maggioranza
  delle NMI della fase 13), `sub_452b` e `sub_4dee`, `sub_4e31`, `sub_4e87`
  (z80_4xxx.c), il corpo 14 (`attivita.c`: letture di c09b e c464, che la NMI
  incrementa, e le chiamate a 452B), `sub_4dad` e l'ingresso 4DA2 di
  `nemici.c` (`sposta_249e()` rifà sub_249e, e sub_4e31 rifà sub_24b1 in
  linea, perché z80_2xxx.c non si tocca), `sub_5693` (nemici_tipi_10_13.c).
  Le letture di variabili che la NMI incrementa (c2b7 a 4DB0/4DB1, c2a9 a
  4E8A, c464 a 44B9) hanno il punto prima della lettura, non solo prima della
  scrittura: senza, una NMI con ritorno fra lettura e scrittura cambia il
  valore letto.
- `PUNTO_LARGO` aggiunti: 3045 (fase 13), 37B4 (fase 18), 3A54 (fase 19), 33BA
  prima della chiamata (fasi 20 e corpo 08: la routine accende il bit 6 di
  c044 alla fine), 75AA spezzato (75AA-75BC prima, 75BD-75E6 dopo: la routine
  azzera c299-c2a8 a 75C3), 6771 (6771-6784), 7ECA (7ECA-7F40), 2D8A,
  23BB-23FF e 3225-33E9 (322A e figlie).
- Deviazioni dal brief: i ganci `PUNTO_GIRO` in routine e corpi del motore
  dei nemici (sopra) non erano previsti ma servono alla saldatura; `nemici.c`,
  `nemici_tipi_10_13.c`, `attivita.c`, `z80_3xxx.c`, `z80_4xxx.c`,
  `gioco_libero.c` e `nemici.h` sono toccati solo per questo. I file
  `snap_*` delle catture sono nel commit come per 3ADF (anche i `.bin`).

## H4c: giro completo 0-5499 e hash dei framebuffer (brief 48)

- Oracolo esteso: `giro_boot.sh` arriva a REPLAY_FINE 5500 (13 s) e scarica i
  dump di 700, 1000, 1300, 1600, 1900, 2100, 2400, 2410, 3000, 3500, 4000,
  4500, 5000, 5499 (più 602). I vecchi file sono un prefisso dei nuovi; il
  registro esteso è identico a `out/replay_disp.txt`. 5434 righe F, 106104 D.
  L'impronta di `macchina.c` differisce da quella di `replay.c`: non si
  concatena con `replay_oracolo.txt`.
- `harness/giro_video.c`: a ognuno dei 13 frame campione, prima della NMI, la
  RAM del C e il dump di MAME sono resi con `rendi_frame` e si confronta
  l'hash FNV-1a 64 (come `vsdl_hash`) con la tabella. Righe `FRAME_VIDEO`.
  Il 1000 vale `ecd5acf694285489` come lo stato `titolo` validato col PNG;
  degli altri stati dell'oracolo video solo `titolo` è raggiunto dal giro.

| frame | hash | frame | hash |
|---|---|---|---|
| 700 | ecd5acf694285489 | 3000 | a095933c7af464d4 |
| 1000 | ecd5acf694285489 | 3500 | 151fa1f690ea7454 |
| 1300 | 72dcb8156116ea1a | 4000 | 477dc1ae3714e5ef |
| 1600 | 47ce347bb79ec1bb | 4500 | 57d536ce21b8e9cb |
| 1900 | 960052b7b34b1c54 | 5000 | 8b0bb97c5ef23965 |
| 2100 | 5f9c4bdae4b92d31 | 5499 | a239d0a940bec0b9 |
| 2400 | 7a0831f8cdb6805d | | |

- Regressione: `sh giro_completo.sh` (`REGEN=1` rigenera l'oracolo, `RIGHE=1`
  stampa i frame `MEZZA`). Riga finale: `GIRO COMPLETO 0-5499: saldatura 0
  byte, spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con
  hash coincidenti`; altrimenti `FALLITO (motivo)` e uscita 1.
- Divergenze a metà attività: 33 prima di H4c, 3 oltre il 2410 (frame 4029 e
  4508 con ritorno 4632, 5459 con ritorno 4635: NMI prima della fase 4632 del
  corpo 0A). Sanate con `PUNTO_GIRO(0x4632, 0x4635)` in `sub_4632`
  (`nemici.c`): restano 30. Sono i frame 604-611, 1188 e 1190-1195, 1532-1541 e
  1565-1569 (ritorni 32xx/33xx, 7F03-7F28, 75B1-75E0, 23F4), già noti da
  H4b3. Il replay storico non cambia (885 a metà attività, ha gli
  stessi ritorni senza i ganci del giro).
- Regione 6253-673F: nel giro 0-5499 non si esegue oltre 63A4-63C2 (corpo 0E,
  già tradotto) e 673F (banco): 15 indirizzi in `replay_gioco_eseguiti.txt`.
  Resta fuori dall'harness; da affrontare solo se un giro più lungo (H5, partita
  libera) la raggiunge.

## H5: gioca (brief 49)

`harness/gioca` (da `decomp/`; `make gioca`, richiede SDL2) unisce la macchina
del giro dal boot alla finestra SDL2 di H3. Sorgenti: `gioca.c` (opzioni,
registrazione, resa), `gioca_tasti.c` (tastiera/joystick), `gioca_dsw.c` (DSW),
`macchina_libera.c` (macchina senza oracolo), `macchina.h`.

- Uso: `harness/gioca [-s N] [-n N | -m N] [--registra F] [--riproduci F]
  [--hash-frame F,F] [DSW] [--stampa-dsw] [--mappa]`. `-n N` senza finestra (N
  frame resi), `-m N` con finestra, poi esce. Riepilogo su stdout: `FRAME rese`,
  `HASH ultimo` (e `FRAME ultimo` con `--oracolo`). `--hash-frame` stampa
  `HASH <frame> <hex16>` (FNV-1a del frame reso con la RAM prima della NMI).
- Registra/riproduci: stesso formato di `out/ingressi_replay.txt`
  (`<frame> <e600> <e601> <e602>`, hex maiuscolo a due cifre); il primo frame
  è il 66. Frame mancante = `00 00 00`. Con `--riproduci` il reset F2 è disattivato.
- Tasti (`gioca --mappa`; porte attive alte, bit da solomon.cpp): frecce/WASD
  direzioni (e600 01/02/04/08), X e Alt sx = BUTTON2 (10), Z, Ctrl sx e Spazio =
  BUTTON1 (20), 1 e Invio = START1 (e602 01), 2 = START2, 5 e C = COIN1 (04), 6 =
  COIN2; giocatore 2 (e601): L J I K direzioni, M BUTTON2, N BUTTON1. Direzioni
  opposte insieme si annullano. Joystick 0: assi oltre ±8000 e cappello,
  pulsanti 0 = BUTTON1, 1 = BUTTON2, 6 = COIN1, 7 = START1. P = pausa, F2 =
  reset (`execv` con gli stessi argomenti), ESC esce. Quale pulsante sia il
  salto e quale la magia: non risulta dal sorgente MAME, da annotare dopo la
  prova manuale con la finestra (non eseguita in questo lotto).
- DSW: `--vite 2|3|4|5`, `--cabinet upright|cocktail`, `--demo on|off`, `--bonus
  30k-200k-500k|100k-300k-800k|30k-200k|100k-300k|30k|100k|200k|nessuno`,
  `--difficolta facile|normale|dura|difficile`, `--timer lento|normale|veloce|
  velocissimo`, `--extra normale|difficile`, `--dsw1 HEX`, `--dsw2 HEX` (grezzi,
  vincono). Predefiniti 02/00. Arrivano alla macchina solo con la scrittura del
  boot (`macchina_dsw1/2`). Il codice tradotto legge pochi bit dei DSW: valori
  diversi dai predefiniti possono portare a codice non tradotto.
- Modo `--oracolo ORACOLO DISP --riproduci F`: la macchina di H4 (istante delle
  NMI dall'oracolo), ripresa in finestra. Riproduce esattamente la partita: hash
  del frame 5499 `a239d0a940bec0b9`, `FRAME rese 5434`.
- Modo libero (senza oracolo): il boot usa `MACCHINA_PUNTO` come orologio, una
  NMI ogni `PUNTI_PER_NMI_BOOT` = 1150 punti (misurato: 1000 -> boot finito al
  frame 615, 1100 -> 605, 1200 -> 598; il boot totale fa circa 660000 punti; con
  32 non finiva). Dopo il boot la NMI entra ai confini: ogni `CORPI_PER_FRAME` = 22
  corpi (media misurata sul registro di MAME 21,67) o `LETTURE_PER_FRAME` = 2000
  letture a vuoto della scansione. Non c'è modello dei cicli del Z80: in MAME
  2928 delle 4800 NMI dopo il frame 700 cadono a metà corpo, quindi la partita
  è plausibile ma non identica a MAME (l'hash del frame 5499 libero è
  `0150736758a51e3b`, diverso da `a239...`). Con gli ingressi registrati il modo
  libero arriva a 5434 frame senza fermarsi; i frame 700 e 1000 (titolo) danno
  `ecd5acf694285489`, come MAME.
- Limite di copertura: i corpi tradotti sono quelli della partita registrata. Una
  partita diversa (altre stanze, registro R, DSW diversi) può raggiungere codice
  non tradotto: il modo libero stampa `FERMO <motivo> al frame <f>`, chiude la
  registrazione e la finestra, riepiloga ed esce con codice 3.

## H6: il gioco suona (brief 50)

`gioca` collega il driver audio tradotto (`audio_*.c`, non modificato) tramite
`harness/scheda_audio.c` (sequenza dei comandi), `scheda_pcm.c` (emu2149, PCM) e
`gioca_audio.c` (dispositivo SDL). Il gancio è `macchina_latch_audio`, chiamato
da `gancio_porta` a ogni scrittura a `e800` (nella NMI del frame).

- Meccanismo (solomon.cpp): `e800` scrive il soundlatch e dà una NMI all'audio
  (0066, legge il latch a 8000); lo Z80 audio ha un IRQ periodico a 120 Hz, cioè
  2 per frame, senza legame di fase con la NMI. Misurato su MAME nel frame
  etichettato F: **IRQ A (0038 + passata 029E), latch/NMI, IRQ B**. Fase 1
  (predefinita) fa questo; fase 0 = latch, A, B (il vecchio modello del banco);
  fase 2 = A, B, latch. Prima chiamata (frame 66): avvio e recupero dei frame
  0-65 con soli A e B, che entra nell'hash ma non nel dispositivo.
- Oracolo: `sh partita_audio.sh` (MAME senza finestra né audio, 9 s) scrive
  `out/audio/partita_latch.txt` (165 scritture; md5 1480738c...) e
  `out/audio/partita_ay.txt` (20409 cambi; md5 c2db4b09...). Il file `_ay.txt`
  in `out/audio/` entra anche nel giro di `sh audio_wav.sh oracoli` (da 4 a 5
  "diversi": il nuovo è `partita`, che non ha un C di confronto nel suo modello).
- Opzioni di `gioca`: `--muto`, `--flusso-ay F`, `--registra-latch F`,
  `--fase-audio 0|1|2`. Con `-n` l'audio è sempre muto (il PCM si calcola e va
  nell'hash). Stdout: `AUDIO campioni`, `AUDIO hash`; `AUDIO dispositivo 44100 Hz
  mono` quando si apre il dispositivo.
- Latch: `gioca -n 5434 --oracolo ... --registra-latch F` dà esattamente
  `partita_latch.txt` (cmp uguale).
- Cambi AY, confronto `python3 partita_audio_confronta.py out/audio/partita_ay.txt
  <c_ay> 14 5499` (frame 14-5499, insiemi / stati / primo frame):
  fase 0: 2565 / 1870 / 1190; **fase 1: 604 / 312 / 1190** (scelta); fase 2:
  3828 / 3344 / 1189. Fissata come atteso esatto in `sh partita_audio.sh confronta`.
  Residuo fase 1: l'effetto di un comando compare un frame dopo in MAME (es. 3F
  del frame 1189: `07 FF` in 1191, nel C in 1190); in 152 frame dei 604 i cambi
  mancanti stanno nel frame adiacente, gli altri sono sequenze di note cadute in
  un altro punto della passata 029E. La causa probabile è la durata della
  passata 029E (la NMI la spezza); modellarla è l'eventuale H6b, se all'ascolto
  stona.
- Hash audio (regressione): `gioca -n 5434 --oracolo ... --riproduci ...` dà
  `AUDIO campioni 4042500` e `AUDIO hash c4b40fea06623a96` (fase 1); è uguale
  all'FNV-1a dei byte di `audio_wav` sul flusso scritto da `gioca`
  (`partita_audio_confronta.py --hash-wav`). Modo libero 2000 frame:
  `c8145bb8b3dac910`.

## H7: pacchetto (brief 51)

Nessun cambio di comportamento nel banco, nel replay, nel giro o nell'audio:
uscite identiche byte per byte a quelle del commit a293344 (prima del lotto) per
`harness --tutte` (307 / 1278 / 0), `sched_proto`, `replay`, `gioca --oracolo`
(hash a239d0a940bec0b9, audio c4b40fea06623a96) e `audio_wav.sh oracoli`
(uguali 40 diversi 5).

- `make -C decomp/harness gioco` (o `make -C decomp gioco`): senza `sdl2-config`
  si ferma con `ERRORE: SDL2 non trovato ...` (uscita 2); se mancano le ROM estrae
  `roms/solomon.zip` in `roms/ext/solomon/` (senza zip: `ERRORE: mancano le
  ROM ...`), costruisce `out/main64.bin` con `build_rom.sh` se manca, compila
  `gioca` e dice come lanciarlo. Provato su una copia fresca con il solo zip:
  `main64.bin` rifatto identico (cmp).
- A runtime servono solo 4 ROM: `6.3f`, `7.3h`, `8.3jk` (dentro `out/main64.bin`)
  e `1.3jk` (strace: gli unici file ROM aperti sono `main64.bin` e `1.3jk`).
  Grafica e dati audio sono C generato.
- Percorsi: `carica_rom()` (`comune.c`, funzione `percorso_exe`) e
  `aud_carica_rom()` provano in coda la cartella dell'eseguibile
  (`/proc/self/exe`): `<exe>/../out/main64.bin` e
  `<exe>/../../roms/ext/solomon/1.3jk`. `gioca` parte da qualunque cartella;
  `gioca -n 600` dà da ovunque `HASH ultimo ecd5acf694285489`, `AUDIO hash
  8aedf41c0424df54`.
- Joystick (`gioca_joy.c`): API joystick grezza di SDL, tutti i dispositivi
  aperti (al più 8), hotplug con `SDL_AddEventWatch`, eventi anche senza fuoco;
  direzioni da assi 0/1 (soglia ±8000) e hat 0, pulsanti con la mappa
  `--joy-mappa B1,B2,START,COIN` (predefinita 0,1,7,6, da ritoccare dopo la prova
  sul DAIJA); si somma alla tastiera. `--senza-joystick` non apre il sottosistema.
- Diagnostica `gioca --joystick [-t SEC]` (`gioca_diag.c`): righe `JOYSTICK`,
  `ASSE`, `PULSANTE`, `CAPPELLO`, `COLLEGATO`, `SCOLLEGATO` con l'azione di gioco
  tra parentesi. Provata con un joystick virtuale uinput
  (`decomp/prova_joystick_uinput.py`, solo joystick, nome «solomon-test»):
  collegamento, asse, hat, pulsante e scollegamento visti tutti.
- Opzioni: `--volume N` (solo il dispositivo, gli hash non cambiano), `--pieno`
  (schermo intero desktop, scala intera, 8:7 mantenuto), `--aiuto`/`-h`
  (`gioca_aiuto.c`); il parsing delle opzioni nuove è in `gioca_opzioni.c`.
- `rst20_646B`: 10 voci (era 3), anche nel generatore `rst20.py` (`VOCI_NOTE`);
  rigenerato senza `out/trace.txt`, i dati coincidono, cambiano solo i commenti
  «eseguito/indice osservato» delle tabelle viste in traccia.
- Resta all'utente la prova manuale con il DAIJA collegato (`gioca --joystick`
  e una partita vera): quale pulsante salta e quale lancia la magia.

## Lotto 52: corpi del gioco libero e ricetta 1A (brief 52)

Tradotti 13 corpi della tabella 3B11 che il gioco libero spediva e su cui si
fermava («FERMO corpo XX non tradotto»): 04 05 09 0F 17 19 1A 1B 1D 23
(`attivita_piccoli.c`, con 4C89), 12 e 13 (`attivita_12_13.c`), 18
(`attivita_18.c`, con 6735 e la voce 2E79). Banco, replay, giro e `gioca
--oracolo` invariati (le voci nuove non sono mai spedite nei frame 0-5499).

- Banco dei corpi: `SNAP_CORPO=1` fa aspettare a `snap.lua` il ritorno in
  [SP+2] (in [SP] c'è lo slot); `sh cattura_corpi.sh` rifà le catture di
  `corpi_catture.txt`; le voci dei corpi (`banco_corpi.c`, campo `solo_ram`)
  confrontano la sola RAM, con la parola dello slot in [SP] contata nella
  pila libera. Il passo 7 del 13 (670F) dura più di un frame e non si cattura:
  è verificato per composizione.
- Ricetta 1A (magia sulla chiave): `decomp/ricette/partita_1a.reg`,
  registrata dall'utente con `gioca --registra`. Da `decomp/`:
  `harness/gioca --riproduci ricette/partita_1a.reg -n 4000 --muto`.
  Prima del lotto: `FERMO corpo 1A non tradotto al frame 3320`, uscita 3.
  Dopo: `FRAME rese 4000`, `HASH ultimo 3f25ba053eeb56b9`, `AUDIO hash
  79f9a550028a41b9`, uscita 0. Durante la stessa partita il log ripete
  «1288: continuazione 12AB (raccolta) non tradotta: nessun effetto»: la
  raccolta di certi oggetti nel gioco libero non aveva ancora effetto
  (tradotta nel lotto 54, vedi sotto).
- Regressione a ingressi casuali: `python3 ricette/genera_ingressi.py SEME
  FILE FINE [PASSO]`, poi `harness/gioca -n FINE --muto --riproduci FILE`.
  Seme 10 (passo 8, 6000 frame): prima «FERMO corpo 04 ... al frame 3733»,
  ora `FRAME rese 6000`. Seme 4: prima «FERMO corpo 13 ... al frame 5475»,
  ora «FERMO routine 352E mancante al frame 5602»; dopo il lotto 54
  `FRAME rese 6000`.
- Registro R: il corpo 12 arriva a 4302 (`ld a,r`); nel gioco libero R non
  è modellato (decisione utente, lotto dedicato) e quando un oggetto
  soddisfa le condizioni del 12 il gioco si ferma su «lettura del registro R
  non modellata».
- Restavano non tradotti: 1C 22 25 26 (fatti nel lotto 54), 15 16 24 27
  (lotto 55; i lotti slittano di uno per il brief 53).

## Brief 53: stelline d'inizio quadro nel gioco libero

Il cerchio di 16 stelline che a inizio quadro si stringe sulla chiave (corpo
07 fase 13, voce 1 305F: `sub_3097`, poi 237E e due 23A1) avanza di un passo
per **chiamata** di 305F, ma dura 64 **frame** (c463 da 00 a 3F, contato
dalla NMI). In MAME (`out/sched_log.txt`) 305F gira una volta per frame
(frame 1773-1835: 1 spedizione di 07 su 2): 3097 dura circa un frame. Il
gioco libero chiudeva il frame solo a 22 corpi, quindi 11 passi per frame: in
7 frame il raggio passava lo zero (00 -> FE) e per i 57 frame restanti le
stelline giravano su un cerchio enorme lontano dalla chiave.

- Correzione: gancio `macchina_frame_pieno` / `MACCHINA_FRAME_PIENO()`
  (`ram.h`, `comune.c`, NULL altrove), chiamato in fondo a `sub_3097`; la
  macchina libera lo aggancia dopo il boot (`frame_pieno_libero`: corpi =
  `CORPI_PER_FRAME`, la NMI entra al confine seguente). Banco (1310 OK, 322
  riepiloghi, 0 FALLITO), giro, `sched_proto`, replay e `gioca --oracolo`
  identici a prima.
- Cadenza misurata con gdb su `sub_3097` (stampa frame, c463, c2c5, c2c6,
  c2c2, c2c4), partita `ricette/partita_1a.reg -n 4000`: prima 1402 chiamate
  (frame 1381 -> 8, 1382-1444 -> 11, idem 2737-2800), ultima `C 1444 3f 3c
  c9 c4 6b`; dopo 128 chiamate, una per frame (1381-1444 e 2737-2800),
  ultima del primo quadro `C 1444 3f 3f 0d 41 48` (raggio 0D sulla chiave).
- Partita 1A dopo la correzione: arriva ancora al corpo 1A al frame 3320
  (gdb su `attivita_1a_411d`); `FRAME rese 4000`, `HASH ultimo
  3f25ba053eeb56b9` (invariato), `AUDIO hash 2b2b3e7c4ca136fc` (era
  79f9a550028a41b9), nessun FERMO. Seme 10 (6000 frame, passo 8): `FRAME
  rese 6000`, `HASH ultimo 923cbefe0efb5a9f`, invariato.
- Fondale (322A, corpo 07 fase 6 e 7CD3 nell'attract): misurato, difetto di
  natura diversa, rimandato. In MAME la chiamata è una sola e copre circa sei
  frame con NMI interne (frame 1188 inizio, 1189-1193 zero spedizioni, 1194
  ripresa); nel gioco libero inizio e consegna della palette (`sub_33ba`)
  cadono nello stesso frame (gdb: 1178, 2377, 2534). Non è un passo ripetuto
  per frame: il frame pieno in coda non basta, servirebbero le NMI dentro la
  routine prima di `sub_33ba` (come `PUNTO_LARGO` in modo oracolo). Effetto:
  il libero resta avanti di ~5 frame dopo ogni fondale; da giudicare a vista.

## Lotto 54: corpi 1C 22 25 26, routine 352E e raccolta 12AB (brief 54)

Tradotti i corpi 1C (`attivita_1c.c`), 22 (`attivita_22.c`, con 12F0 e il
salto `rst 38` alla tabella 130A), 25 (`attivita_25.c`) e 26
(`attivita_26.c`, con 6502 e la voce 3D59 condivisa con il 25), gli 11
gestori di raccolta mancanti (`raccolta_13xx.c`; 139C 13AB 13B9 1413 14D4 e
13FC esistevano già in `nemici_comparsa.c`), la continuazione 12AB di 1288
(`raccolta_12ab` in `z80_1xxx.c`: classe 2 posta 1C, classi 5-7 vuotano la
cella e postano 18, classi da 8 accodano in c05b e postano 22) e i rami
352E, 353B, 351E della fase 1 del corpo 07 (`attivita_attract07a.c`).
Banco, replay, giro e `gioca --oracolo` invariati.

- Catture: 61 nuove in `corpi_catture.txt` (93 in tutto), con il modo
  `stato` di `cattura_corpi.sh` per le 11 di 1288 nello stato `gioco`. I
  passi con 75AA (1C 3F58 e 3FF7, 25 3BAC, 26 6655) non si catturano e
  sono verificati per composizione.
- Partita 1A, nuovo riferimento (da `decomp/harness`, due misure uguali):
  `./gioca --riproduci ../ricette/partita_1a.reg -n 4000 --muto` →
  `FRAME rese 4000`, `HASH ultimo 3f25ba053eeb56b9`, `AUDIO campioni
  2988510`, `AUDIO hash c097308a8678392e` (prima `2b2b3e7c4ca136fc`),
  nessuna riga 12AB né FERMO. L'hash dell'ultimo frame resta uguale: con
  `--hash-frame` ogni 100 frame la partita diverge fra i frame 1900 e 2600
  (la raccolta ora ha effetto) e poi torna sugli stessi frame.
- Semi 4 e 67 (6000 frame, passo 8): `FRAME rese 6000`, nessun FERMO
  (prima «FERMO routine 352E» ai frame 5602 e 4866); seme 10 invariato
  (`FRAME rese 6000`).
