# Z80 audio: censimento dell'interprete delle sequenze

Fonte: `out/audio.dasm` (z80dasm di `1.3jk`) e i conteggi di esecuzione
sommati da `out/audio/*_istruzioni.txt` (44 corse, avvio compreso). Solo
indirizzi e conteggi, nessun byte della ROM. Le letture del dasm sono
verificate e tradotte in `harness/audio_interprete.c` (brief 20); F3, FD
e FE sono tradotti dal dasm ma non osservati in nessuna corsa.

## Ciclo di 0147 (chiamato dall'IRQ, 71497 esecuzioni)

Per ognuno dei 18 blocchi (4000-423F, passo 0x20):

1. flag +00 senza bit 0, o con bit 1 → blocco saltato. Il flag ruotato
   resta in `af'`: glissando e inviluppo usano il flag **di partenza** del
   blocco.
2. Durata a 16 bit +03/+04: `+03 -= 1`; col prestito, se +04 = 0 → lettura
   della sequenza (`01E8`, `ix` = blocco) e fine del blocco per questo IRQ
   (niente glissando né inviluppo); altrimenti `+04 -= 1`. Una durata d
   dura quindi d + 1 IRQ (per questo `01FA` scrive parola − 1 e `00FA`
   scrive +03 = 1).
3. Glissando (flag bit 4, `01B2`, 8861 volte): +0B/+0C += +0D/+0E (16 bit,
   si avvolge). **Nessuna condizione di fine**: lo ferma solo una nota
   nuova (`flag &= E7`). Col bit 6 anche +10/+11 = periodo >> 3.
4. Inviluppo software solo con i bit 5 e 6 a zero (il bit 3 non conta):
   `+05 -= 1`; a zero, p = +06/+07: se (p+1) non è FF, +08 = (p+1),
   puntatore = p+2, +05 = (p+2); se è FF riparte da s = +14/+15: +08 = (s),
   puntatore = s+1, +05 = (s+1).

## Classi dei byte di sequenza (`01EF`: `rst 18h`, 31145 letture)

| byte | dove | esecuzioni | lettura |
|---|---|---|---|
| 00-7F | `0217` | 15038 | nota: nibble basso ≥ 0C → pausa (`027F`, `set 3` senza toccare il glissando, 2234); altrimenti flag &= E7, indice = 12·ottava + nota + (+17) a 8 bit; < 30h → parola da `0700`, altrimenti byte da `0760` (`0240`, 3262; byte alto 0) → +0B/+0C; poi `0250`: +03/+04 = +12/+13; con flag & 60 = 0 inviluppo software da s = +14/+15 (+08 = (s), +05 = (s+1), +06/+07 = s+1); col bit 6 +10/+11 = `de` >> 3 (`0286`, 54); fine del blocco |
| 80-EF | `01FA` | 2106 | durata: parola di `0790` all'indice ((byte & 7F) + (+16)) a 8 bit, meno 1 → +12/+13; prosegue con il byte seguente |
| F0-FF | `020E` | 14001 | gestore: parola di `03CE` all'indice byte & 0F, `jp (hl)` a `0216` |

Somma: 15038 + 2106 + 14001 = 31145, uguale alle esecuzioni di `01EF`.

### Il registro `de` in `0286`

Con una pausa e il bit 6 del flag, +10/+11 riceve `de` >> 3 dove `de` non
è il periodo ma l'ultimo valore lasciato dal percorso: all'ingresso del
blocco k ≥ 1 `de = 4240 − (4000 + 20·k)` (da `019F-01A7`); al blocco 0 è
quello del codice interrotto dall'IRQ (non modellabile, il C usa 0); dopo
`rst 8` `de = 2a` (8 bit); dopo `rst 10h` la parola letta; all'ingresso di
ogni gestore `de = 03CF + 2·(byte & 0F)`; i byte 80-EF lasciano la durata
− 1; la nota il periodo. Gestori che cambiano `de`: F0 (inizio
dell'inviluppo o periodo), F4 e F7 (parola − 1), F6 se ripete, F8, FB
(periodo iniziale), FC (periodo).

## Gestori del `jp (hl)` a 0216 (tabella a 03CE)

Esecuzioni dalla riga `gestori 0216` di `audio_numeri.py`, somma 14001.
Tutti tornano a `01EF` (byte seguente) salvo dove indicato. Il C confronta
una volta la tabella con le parole della ROM a 03CE.

| codice | indirizzo | esecuzioni | campi | lettura |
|---|---|---|---|---|
| F0 | 03EE | 1165 | +00, +01/+02, +0F, +10/+11, +14/+15 | n = byte. n = FF (`041C`, 4): forma +0F = byte, periodo +10/+11 = parola, puntatore sull'ultimo byte, flag = (flag & 9F) \| 20. n = FE: +0F = byte; se il byte dopo è FF il puntatore resta su quell'FF e flag = (flag & 9F) \| 40, altrimenti puntatore sulla forma e flag = (flag & 9F) \| 60. Altrimenti (`0400`, 1161): puntatore su n, +14/+15 = parola di `0850` all'indice n, flag &= 9F |
| F1 | 0470 | 4767 | +09 | attenuazione = byte |
| F2 | 0477 | 121 | +17 | trasposizione = byte |
| F3 | 047E | 0 | +16 | scostamento della tabella durate = byte; tradotto dal dasm, non osservato |
| F4 | 0485 | 466 | +01/+02 | salto: puntatore = parola − 1 |
| F5 | 0499 | 302 | +1A, +18/+19 | inizio ripetizione: +1A = byte, +18/+19 = indirizzo del contatore |
| F6 | 04A6 | 1394 | +1A, +18/+19, +01/+02 | fine ripetizione: `+1A -= 1`, se ≠ 0 puntatore = +18/+19 (contatore 0 = 256 giri) |
| F7 | 04BB | 326 | +01/+02, +1B/+1C | chiamata: puntatore = parola − 1, +1B/+1C = indirizzo del byte alto della parola (un solo livello) |
| F8 | 04D5 | 286 | +1B/+1C, +01/+02 | ritorno: puntatore = +1B/+1C |
| F9 | 04E4 | 4264 | +0A | rumore acceso: +0A = byte + 1 (029E scrive il registro 6 = +0A − 1) |
| FA | 04EC | 52 | +0A | rumore spento: +0A = 0 |
| FB | 04F3 | 126 | +00, +0B/+0C, +0D/+0E, +12/+13 | glissando: flag &= E7; p0 → +0B/+0C; diff = p1 − p0 (se negativo `set 4` e cambio di segno); durata (senza − 1) → +12/+13; passo = (diff << 4, 16 bit) / durata con la divisione a **12 giri** di `0547-0559`, cioè (diff & 0FFF) / durata; col bit 4 cambiato di segno → +0D/+0E; `set 4`; `de` = p0; `0250`; fine del blocco |
| FC | 0578 | 367 | +00, +01/+02, +0B/+0C | nota a periodo diretto: flag &= E7, parola → +0B/+0C, puntatore sul byte alto, `024A`/`0250`; fine del blocco |
| FD | 0591 | 0 | 4243 | comando a catena: (4243) = byte, 00E4 lo avvia nello stesso IRQ; tradotto dal dasm, non osservato |
| FE | 0598 | 0 | +09 | attenuazione + 1; arrivata a 0F prosegue in FF senza aggiornare +09; tradotto dal dasm, non osservato |
| FF | 05A6 | 365 | +00, flag del blocco −9 | fine: flag = (flag & 4) \| 3 (029E spegne la voce); nel secondo strato (blocco oltre 4100) azzera il bit 2 del blocco −9; fine del blocco |

## Campi del blocco da 32 byte

| campo | uso |
|---|---|
| +00 | flag: bit 0 attivo, 1 da spegnere, 2 coperto dal secondo strato, 3 muto (pausa), 4 glissando, 5-6 inviluppo hardware (20 forma e periodo, 40 solo forma: 029E azzera il volume, 60 forma con periodo dalla nota) |
| +01/+02 | puntatore di sequenza (preincrementato da `rst 18h`); 00FA lo pone a parola − 1 |
| +03/+04 | durata residua della nota (00FA: 1) |
| +05 | contatore del passo dell'inviluppo software |
| +06/+07 | puntatore dell'inviluppo software |
| +08 | volume (029E: volume = +08 − +09, 0 se ≥ 10h) |
| +09 | attenuazione (F1, FE) |
| +0A | rumore: 0 = tono, altrimenti periodo + 1 (F9, FA) |
| +0B/+0C | periodo del tono (registri 2(b−1), 2(b−1)+1) |
| +0D/+0E | passo del glissando (FB) |
| +0F | forma dell'inviluppo hardware (registro 0D), azzerata da 029E dopo l'uso |
| +10/+11 | periodo dell'inviluppo hardware (registri 0B/0C) |
| +12/+13 | durata della nota (80-EF, FB) |
| +14/+15 | inizio dell'inviluppo software (F0, tabella 0850) |
| +16 | scostamento nella tabella durate 0790 (F3) |
| +17 | trasposizione in semitoni (F2) |
| +18/+19, +1A | ritorno e contatore della ripetizione (F5/F6) |
| +1B/+1C | ritorno della chiamata (F7/F8) |
| +1D-+1F | mai scritti in nessuna corsa |

## Blocchi e voci

- Byte di RAM scritti in tutte le corse: 458. I 18 blocchi sono tutti
  usati; +1D-+1F mai. Globali 4240-4242 (ombre del mixer) e 4243
  (comando). Pila 34 byte da 47DE a 47FF (sp iniziale 4800). Unico altro
  byte: 46E0, scritto dal comando 40.
- Un blocco non è una voce: da `029E` il blocco k va al chip `k/3 % 3 + 1`,
  voce `3 − k%3`. I blocchi 9-17 sono un **secondo strato** sulle stesse
  9 voci dei blocchi 0-8: 18 blocchi = 9 voci × 2 strati. `00FA` pone il
  bit 2 «coperto» sul blocco k − 9 (029E lo salta), FF lo toglie.

## Comando 40

La voce 40 della tabella 0AF3 punta a 7A09, fuori dalla ROM: `00FA` scorre
la memoria non mappata (letta 00 → blocco 0) fino a FFFF, legge il latch a
8000 (40 → blocco 40: `set 2` a 46E0) e riparte dalla ROM a 0000. In MAME
dura ~86 frame dentro il primo IRQ (4000 scritto 11436 volte fra il frame
120 e il 209); l'IRQ fa `ei` a 003E, quindi gli IRQ successivi si annidano
(0147 + 00E4) mentre `029E` non gira. Il gioco non lo invia mai. Il banco
ferma il confronto al primo latch 40 (scelta del brief 20): è un effetto di
tempismo a livello di cicli che il modello a fine frame non simula.

## Tabelle dati usate

| indirizzo | contenuto | usata da |
|---|---|---|
| 0AF3 | parola per comando, indice = codice (01-40) | `00E4` con `rst 8`/`rst 10h`; la voce del 40 cade a 0B73 e punta a 7A09 |
| 03CE | 16 parole: gestori F0-FF | `020E` |
| 0700 | parole: periodi del tono, indice < 30h | `0217` |
| 0760 | byte: periodi del tono, indice ≥ 30h (byte alto 0) | `0240` |
| 0790 | parole: durate, indice (byte & 7F) + (+16) | `01FA` |
| 0850 | parole: inizi degli inviluppi software | F0 (`0400`) |

## Dati estratti (brief 21)

Le tabelle e i flussi qui descritti sono estratti come macro C in
`harness/audio_dati_*.c` (`estrai_dati.py`); i comandi 01-3F sono descritti
in `SPARTITI.md`.
