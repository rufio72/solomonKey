# Giro dal boot (lotti H4a e H4b1, brief 44 e 45)

Primo pezzo dell'ex «giro libero»: il C parte dal vettore 0000 e gira senza
che l'oracolo di MAME lo guidi. Dell'oracolo restano il controllo (impronta
della RAM a ogni NMI) e gli ingressi registrati. Il giro si ferma, atteso e
misurabile, alla prima attività non tradotta: in H4a `CORPO 1F (43C8)` al
frame 602, da H4b1 `CORPO 07 (2CCD) non tradotto al frame 1188`. Restano
H4b2 (corpo 07), H4b3 (08, 0C, 0B e confronto al 2410) e H4c (giro 0-5499).

## Uso (da `decomp/`)

```
sh replay.sh               # solo se manca out/replay_oracolo.txt
sh estrai_ingressi.sh      # out/ingressi_replay.txt: <frame> <e600> <e601> <e602>
sh giro_boot.sh            # oracolo del boot, ~45 s: out/giroboot_*.txt e dump 602
cd harness && make gioco_libero && cd ..
harness/gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt \
    out/ingressi_replay.txt [frame_fine]
```

Uscita attesa: `SANITA 602: ... coincide`, `CORPO 1F (43C8) non tradotto al
frame 602`, `NMI confrontate: 537, divergenti: 0`, codice 1.

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
