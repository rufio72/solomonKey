# Giro dal boot (lotto H4a, brief 44)

Primo pezzo dell'ex «giro libero»: il C parte dal vettore 0000 e gira senza
che l'oracolo di MAME lo guidi. Dell'oracolo restano il controllo (impronta
della RAM a ogni NMI) e gli ingressi registrati. Il giro si ferma, atteso e
misurabile, alla prima attività non tradotta: `CORPO 1F (43C8) non tradotto
al frame 602`. Restano H4b (corpi attract/titolo 1F, 06, 07, 08+0C, 0B e
regione 6253-673F, circa 950 istruzioni, più l'oracolo del registro R) e H4c
(giro 0-5499).

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

## Limiti

- La NMI nella scansione dello spedidore e a metà attività (come nel
  replay) non è ancora gestita: serve da H4b, quando i corpi dopo il 1F
  saranno tradotti.
- `sub_2a03` non traduce il ramo di guasto 2AC8 (IX & 7F ≠ 0): esce con
  un avviso. `sub_29cf` e `sub_2a03` non sono catturabili dal banco (durano
  224 frame): la verifica è questo giro.
