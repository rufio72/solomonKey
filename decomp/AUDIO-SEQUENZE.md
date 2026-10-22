# Z80 audio: censimento dell'interprete delle sequenze

Ingresso del lotto interprete (dopo il brief 19). Fonte: `out/audio.dasm`
(z80dasm di `1.3jk`) e i conteggi di esecuzione sommati da
`out/audio/*_istruzioni.txt` (44 corse, avvio compreso). Solo indirizzi e
conteggi, nessun byte della ROM. Tutte le letture del dasm sono **da
confermare nel lotto interprete**; i conteggi sono misurati.

## Ciclo di 0147 (chiamato dall'IRQ, 71497 esecuzioni)

Per ognuno dei 18 blocchi (4000-423F, passo 0x20), se il flag ha il bit 0
e non il bit 1: decrementa la durata +03/+04; a zero va a `01E8` (lettura
della sequenza, `ix` = blocco). Se il flag ha il bit 4 va a `01B2`
(glissando, 8861 volte): +0B/+0C += +0D/+0E, e con il bit 6 anche
+10/+11 = periodo >> 3. Senza bit 5-6: inviluppo di volume software
(+05 contatore, +06/+07 puntatore, +08 volume; a FF riparte da +14/+15).

## Classi dei byte di sequenza (`01EF`: `rst 18h`, 31145 letture)

| byte | dove | esecuzioni | lettura del dasm (da confermare) |
|---|---|---|---|
| 00-7F | `0217` | 15038 | nota: nibble basso ≥ 0C → pausa (`027F`, flag bit 3, 2234); altrimenti flag &= E7, indice = ottava·12 + nota + (+17); < 30h → parola da `0700`, altrimenti byte da `0760` (`0240`, 3262; byte alto 0) → +0B/+0C; poi `0250`: durata +03/+04 = +12/+13, inviluppo software da +14/+15 o, con flag bit 6, +10/+11 = periodo >> 3 (`0286`, 54); fine del blocco per questo IRQ |
| 80-EF | `01FA` | 2106 | durata: parola di `0790` all'indice (byte & 7F) + (+16), meno 1 → +12/+13; prosegue con il byte seguente |
| F0-FF | `020E` | 14001 | gestore: parola di `03CE` all'indice byte & 0F, `jp (hl)` a `0216` |

Somma: 15038 + 2106 + 14001 = 31145, uguale alle esecuzioni di `01EF`.

## Gestori del `jp (hl)` a 0216 (tabella a 03CE)

Esecuzioni dalla riga `gestori 0216` di `audio_numeri.py`, somma 14001.
Tutti tornano a `01EF` (byte seguente) salvo dove indicato.

| codice | indirizzo | esecuzioni | campi letti / scritti | lettura (da confermare nel lotto interprete) |
|---|---|---|---|---|
| F0 | 03EE | 1165 | +00, +01/+02, +0F, +10/+11, +14/+15 | inviluppo: byte n; n < FE → +14/+15 = parola di `0850`[n], flag bit 5-6 = 0 (`0400`, 1161); FF → forma +0F e periodo +10/+11, bit 5-6 = 01 (`041C`, 4); FE → forma +0F, bit 5-6 = 11, o 10 se segue FF |
| F1 | 0470 | 4767 | +09 | attenuazione = byte |
| F2 | 0477 | 121 | +17 | trasposizione = byte |
| F3 | 047E | 0 | +16 | scostamento della tabella durate = byte; mai eseguito |
| F4 | 0485 | 466 | +01/+02 | salto: puntatore = parola − 1 |
| F5 | 0499 | 302 | +1A, +18/+19 | inizio ripetizione: contatore = byte, ritorno = puntatore |
| F6 | 04A6 | 1394 | +1A, +18/+19, +01/+02 | fine ripetizione: decrementa +1A, se ≠ 0 torna a +18/+19 |
| F7 | 04BB | 326 | +01/+02, +1B/+1C | chiamata: puntatore = parola − 1, ritorno in +1B/+1C |
| F8 | 04D5 | 286 | +1B/+1C, +01/+02 | ritorno dalla chiamata |
| F9 | 04E4 | 4264 | +0A | rumore acceso: +0A = byte + 1 (029E scrive il registro 6 = +0A − 1) |
| FA | 04EC | 52 | +0A | rumore spento: +0A = 0 |
| FB | 04F3 | 126 | +00, +0B/+0C, +0D/+0E, +12/+13 | glissando: periodo iniziale, finale e durata; passo +0D/+0E = differenza·16 / durata (divisione a 12 giri), segno nel flag bit 4; poi `0250` come una nota |
| FC | 0578 | 367 | +00, +01/+02, +0B/+0C | nota a periodo diretto: parola → +0B/+0C, poi `024A`/`0250` |
| FD | 0591 | 0 | 4243 | comando a catena: (4243) = byte; mai eseguito |
| FE | 0598 | 0 | +09 | attenuazione + 1; da 0F in su prosegue in FF; mai eseguito |
| FF | 05A6 | 365 | +00, flag del blocco −9 | fine: flag = (flag & 4) \| 3 (029E spegne la voce); nel secondo strato (blocco ≥ 4120) azzera il bit 2 del blocco −9; fine del blocco per questo IRQ |

## Campi del blocco da 32 byte

Da `00FA` (avvio del comando), `029E` (registri, tradotto nel brief 19) e
`0147` con i gestori (da confermare).

| campo | uso |
|---|---|
| +00 | flag: bit 0 attivo, 1 da spegnere, 2 coperto dal secondo strato, 3 muto (pausa), 4 glissando, 5-6 inviluppo hardware (029E → `0381`; con il bit 5 anche mixer e tono) |
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

## Tabelle dati usate

| indirizzo | contenuto (lettura del dasm) | usata da |
|---|---|---|
| 0AF3 | parola per comando, indice = codice (01-40) | `00E4` con `rst 8`/`rst 10h`; la voce del 40 cade a 0B73, già nelle sequenze, e punta a 7A09 (memoria non mappata, letta come 00) |
| 03CE | 16 parole: gestori F0-FF | `020E` |
| 0700 | parole: periodi del tono, indice < 30h | `0217` |
| 0760 | byte: periodi del tono, indice ≥ 30h (byte alto 0) | `0240` |
| 0790 | parole: durate, indice (byte & 7F) + (+16) | `01FA` |
| 0850 | parole: inizi degli inviluppi software | F0 (`0400`) |

## Note per il lotto interprete

- I flag dei blocchi a 00 o 03 fanno saltare il blocco a 0147: per questo
  lo stub vuoto di `aud_0147` coincide con MAME su avvio, attesa, codice
  00 e codici 41-FF.
- Il comando 40 scorre la memoria non mappata da 7A09 a FFFF (11436
  scritture al blocco 0 in `cod_40_ram.txt`) e passa per il latch a 8000:
  `aud_leggi` restituisce 00 fuori da ROM, RAM e latch.
- Il banco confronta lo stato dei registri a fine frame; l'interprete andrà
  verificato sui `cod_*` e `lungo_*` con `./harness/audio_banco <nome>`.
