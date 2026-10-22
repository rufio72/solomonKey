# Spartiti del Z80 audio: i comandi 01-3F

Generato da `estrai_dati.py` dalla ROM `1.3jk` (brief 21): non modificare a
mano. I dati stessi stanno in `harness/audio_dati_*.c` come macro leggibili
(`NOTA`, `DUR`, `STRUM`, `SALTA`, `CHIAMA`, ...; definite in
`harness/audio_dati.h`); la semantica dei byte è in `AUDIO-SEQUENZE.md`.

Legenda. Un comando (valore 01-3F scritto nel latch) ha un elenco di record
(blocco, flusso). Il blocco k (0-17) suona sul chip k/3 mod 3 + 1, voce 3 − k
mod 3; i blocchi 0-8 sono il primo strato, 9-17 il secondo, che copre la
stessa voce del blocco k − 9 finché non finisce. Byte e note contano ciò che
è raggiungibile dal flusso (F4 e F7 seguiti); le ottave sono quelle scritte
nei byte, prima della trasposizione F2. La durata nominale somma le durate
delle note lungo il percorso (ripetizioni F5/F6 coi loro giri, chiamate
F7/F8), d + 1 IRQ per nota, 2 IRQ per frame, secondi a 60 frame/s
(attesa, non misurata); per il comando è quella della voce più lunga, il
primo IRQ di avvio escluso. «ciclo» = un salto F4 riporta il flusso in un
punto già suonato con lo stesso stato: il comando non finisce da solo (lo
ferma un altro comando o il latch 00). La colonna «nome» resta vuota: i nomi
si scrivono in `audio_nomi.tsv` ascoltando `out/wav/codici/<cc>.wav`.

## Riassunto

| comando | nome | elenco | record | voci | strati | durata | strumenti |
|---|---|---|---|---|---|---|---|
| 01 | | 0B73 | 2 | 2 | 1 | 4,5 frame (0,07 s) | inviluppo software 2; glissando |
| 02 | | 0B92 | 2 | 2 | 1 | 15 frame (0,25 s) | inviluppo software 2; rumore |
| 03 | | 0C91 | 1 | 1 | 1 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 04 | | 0C9A | 1 | 1 | 1 | 0,5 frame (0,01 s) | inviluppo software 2 |
| 05 | | 0CA3 | 1 | 1 | 1 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 06 | | 0CAC | 1 | 1 | 1 | 2 frame (0,03 s) | inviluppo software 2; rumore |
| 07 | | 0CC3 | 1 | 1 | 1 | 8 frame (0,13 s) | inviluppo software 3; rumore |
| 08 | | 0CD0 | 2 | 2 | 1 | 8 frame (0,13 s) | inviluppo software 5 |
| 09 | | 0CE5 | 3 | 3 | 1 | 8 frame (0,13 s) | inviluppo software 2, 3; periodo diretto |
| 0A | | 0D2E | 18 | 9 | 2 | 17 frame (0,28 s) | inviluppo software 0, 2 |
| 0B | | 0E0F | 9 | 9 | 1 | 70 frame (1,17 s) | inviluppo software 0, 2, 6; glissando; periodo diretto |
| 0C | | 0ECE | 1 | 1 | 1 | 192,5 frame (3,21 s) | inviluppo software 2; glissando |
| 0D | | 0EDE | 2 | 2 | 1 | 512,5 frame (8,54 s) | inviluppo software 2; glissando |
| 0E | | 0EFD | 3 | 2 | 1 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 0F | | 0F16 | 3 | 3 | 1 | 512,5 frame (8,54 s) | inviluppo software 2; glissando |
| 10 | | 0F44 | 2 | 2 | 1 | 45,5 frame (0,76 s) | inviluppo software 7 |
| 11 | | 0F75 | 9 | 9 | 1 | 61 frame (1,02 s) | inviluppo software 0, 2, 7; trasposizione |
| 12 | | 1024 | 9 | 9 | 1 | 63 frame (1,05 s) | inviluppo software 0, 7 |
| 13 | | 10C5 | 4 | 4 | 1 | ciclo | inviluppo software 1, 2, 8; rumore; trasposizione |
| 14 | | 11BF | 4 | 4 | 1 | ciclo | inviluppo software 1, 2, 7, 8; rumore; trasposizione |
| 15 | | 1336 | 3 | 3 | 1 | 18 frame (0,30 s) | inviluppo software 2; trasposizione |
| 16 | | 1390 | 4 | 4 | 1 | ciclo | inviluppo software 2; periodo diretto; rumore; trasposizione |
| 17 | | 160E | 1 | 1 | 1 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 18 | | 1617 | 3 | 3 | 1 | 76 frame (1,27 s) | inviluppo software 0, 2 |
| 19 | | 1669 | 4 | 4 | 1 | ciclo | inviluppo software 0, 4, 7; rumore; trasposizione |
| 1A | | 179A | 4 | 4 | 1 | ciclo | inviluppo software 1, 2, 7, 8, 9; rumore; trasposizione |
| 1B | | 19AA | 4 | 4 | 1 | ciclo | inviluppo software 0, 2, 6, 9; glissando; rumore |
| 1C | | 1BCE | 4 | 4 | 1 | ciclo | inviluppo software 2, 8; rumore; trasposizione |
| 1D | | 1C8C | 9 | 9 | 1 | 150,5 frame (2,51 s) | inviluppo software 0, 12, 14 |
| 1E | | 1D31 | 4 | 4 | 1 | ciclo | inviluppo software 8; trasposizione |
| 1F | | 1F0E | 9 | 9 | 1 | ciclo | inviluppo software 0, 11, 14; periodo diretto; rumore |
| 20 | | 2294 | 9 | 9 | 1 | ciclo | inviluppo software 0, 6, 8; rumore |
| 21 | | 249F | 4 | 4 | 1 | ciclo | inviluppo software 0, 2, 4; rumore; trasposizione |
| 22 | | 2590 | 5 | 5 | 1 | 196 frame (3,27 s) | inviluppo software 9; rumore |
| 23 | | 2677 | 9 | 9 | 1 | ciclo | inviluppo software 0, 8, 11, 14; periodo diretto; rumore |
| 24 | | 2974 | 9 | 9 | 1 | ciclo | inviluppo software 0, 2, 7; trasposizione |
| 25 | | 2A91 | 18 | 9 | 2 | 99,5 frame (1,66 s) | inviluppo software 0, 2, 6, 7 |
| 26 | | 2BC0 | 18 | 9 | 2 | 85 frame (1,42 s) | inviluppo software 0, 2; rumore |
| 27 | | 2D4D | 1 | 1 | 1 | 46 frame (0,77 s) | inviluppo software 2; trasposizione |
| 28 | | 2DC4 | 4 | 4 | 1 | ciclo | inviluppo software 1, 2, 4; periodo diretto; rumore; trasposizione |
| 29 | | 2F12 | 1 | 1 | 1 | 8 frame (0,13 s) | inviluppo software 2 |
| 2A | | 2F2C | 4 | 4 | 1 | ciclo | inviluppo software 0, 1, 2, 4; rumore; trasposizione |
| 2B | | 3096 | 9 | 9 | 1 | 160 frame (2,67 s) | inviluppo software 0, 8; trasposizione |
| 2C | | 3196 | 1 | 1 | 1 | 20 frame (0,33 s) | inviluppo software 2; rumore |
| 2D | | 31BB | 2 | 2 | 1 | 196 frame (3,27 s) | inviluppo software 2 |
| 2E | | 3272 | 1 | 1 | 1 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 2F | | 327D | 2 | 2 | 1 | 132 frame (2,20 s) | inviluppo software 2; inviluppo hardware; rumore |
| 30 | | 32D8 | 2 | 2 | 1 | 20 frame (0,33 s) | inviluppo software 1, 2; periodo diretto; rumore |
| 31 | | 3319 | 3 | 3 | 1 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 32 | | 3332 | 9 | 9 | 1 | 40 frame (0,67 s) | inviluppo software 0, 2; trasposizione |
| 33 | | 3405 | 2 | 2 | 1 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 34 | | 3416 | 9 | 9 | 1 | 76 frame (1,27 s) | inviluppo software 0, 7; trasposizione |
| 35 | | 3490 | 2 | 2 | 1 | 16 frame (0,27 s) | inviluppo software 0, 2; rumore |
| 36 | | 34BF | 2 | 2 | 1 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 37 | | 34D0 | 1 | 1 | 1 | 64 frame (1,07 s) | inviluppo software 2; rumore; trasposizione |
| 38 | | 350C | 9 | 9 | 1 | 66 frame (1,10 s) | inviluppo software 0, 7, 9; trasposizione |
| 39 | | 35B4 | 9 | 9 | 1 | 5 frame (0,08 s) | inviluppo software 0, 2; rumore |
| 3A | | 3699 | 1 | 1 | 1 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 3B | | 36A4 | 1 | 1 | 1 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 3C | | 36AF | 1 | 1 | 1 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 3D | | 36BA | 1 | 1 | 1 | 1800 frame (30,00 s) | inviluppo software 0 |
| 3E | | 36EB | 14 | 9 | 2 | 0,5 frame (0,01 s) | inviluppo software 0 |
| 3F | | 375C | 18 | 9 | 2 | 1,5 frame (0,03 s) | inviluppo software 0 |

Comandi in ciclo: 13 14 16 19 1A 1B 1C 1E 1F 20 21 23 24 28 2A

Comandi: 63; record: 308; flussi distinti: 307.

### Comando 01

Elenco a 0B73, 2 record. 2 voci, 1 strato, 4,5 frame (0,07 s); inviluppo software 2; glissando.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_01_b9_s2` a 0B7A: 12 byte, 0 note, nessuna nota; inviluppo software 2; glissando; 4,5 frame (0,07 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_01_b12_s2` a 0B86: 12 byte, 0 note, nessuna nota; inviluppo software 2; glissando; 4,5 frame (0,07 s)

### Comando 02

Elenco a 0B92, 2 record. 2 voci, 1 strato, 15 frame (0,25 s); inviluppo software 2; rumore.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_02_b9_s2` a 0B99: 124 byte, 30 note, ottave 0-0; inviluppo software 2; rumore; 15 frame (0,25 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_02_b12_s2` a 0C15: 124 byte, 30 note, ottave 0-0; inviluppo software 2; rumore; 15 frame (0,25 s)

### Comando 03

Elenco a 0C91, 1 record. 1 voce, 1 strato, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_03_b0_s1` a 0C95: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 04

Elenco a 0C9A, 1 record. 1 voce, 1 strato, 0,5 frame (0,01 s); inviluppo software 2.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_04_b0_s1` a 0C9E: 5 byte, 0 note, nessuna nota; inviluppo software 2; 0,5 frame (0,01 s)

### Comando 05

Elenco a 0CA3, 1 record. 1 voce, 1 strato, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_05_b0_s1` a 0CA7: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 06

Elenco a 0CAC, 1 record. 1 voce, 1 strato, 2 frame (0,03 s); inviluppo software 2; rumore.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_06_b9_s2` a 0CB0: 19 byte, 3 note, ottave 0-0; inviluppo software 2; rumore; 2 frame (0,03 s)

### Comando 07

Elenco a 0CC3, 1 record. 1 voce, 1 strato, 8 frame (0,13 s); inviluppo software 3; rumore.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_07_b9_s2` a 0CC7: 9 byte, 1 note, ottave 0-0; inviluppo software 3; rumore; 8 frame (0,13 s)

### Comando 08

Elenco a 0CD0, 2 record. 2 voci, 1 strato, 8 frame (0,13 s); inviluppo software 5.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_08_b9_s2` a 0CD7: 7 byte, 1 note, ottave 1-1; inviluppo software 5; 8 frame (0,13 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_08_b12_s2` a 0CDE: 7 byte, 1 note, ottave 5-5; inviluppo software 5; 8 frame (0,13 s)

### Comando 09

Elenco a 0CE5, 3 record. 3 voci, 1 strato, 8 frame (0,13 s); inviluppo software 2, 3; periodo diretto.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_09_b9_s2` a 0CEF: 49 byte, 0 note, nessuna nota; inviluppo software 2; periodo diretto; 4,5 frame (0,07 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_09_b12_s2` a 0D20: 7 byte, 1 note, ottave 5-5; inviluppo software 3; 8 frame (0,13 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_09_b15_s2` a 0D27: 7 byte, 1 note, ottave 3-3; inviluppo software 3; 8 frame (0,13 s)

### Comando 0A

Elenco a 0D2E, 18 record. 9 voci, 2 strati, 17 frame (0,28 s); inviluppo software 0, 2.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_0A_b0_s1` a 0D65: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 1, strato 1, chip 1 voce 2, flusso `cmd_0A_b1_s1` a 0D6A: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_0A_b2_s1` a 0D6F: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 3, strato 1, chip 2 voce 3, flusso `cmd_0A_b3_s1` a 0D74: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 4, strato 1, chip 2 voce 2, flusso `cmd_0A_b4_s1` a 0D79: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_0A_b5_s1` a 0D7E: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 6, strato 1, chip 3 voce 3, flusso `cmd_0A_b6_s1` a 0D83: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_0A_b7_s1` a 0D88: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_0A_b8_s1` a 0D8D: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_0A_b9_s2` a 0D92: 15 byte, 4 note, ottave 3-5; inviluppo software 0, 2; 17 frame (0,28 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_0A_b10_s2` a 0DA1: 15 byte, 4 note, ottave 3-5; inviluppo software 0, 2; 17 frame (0,28 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_0A_b11_s2` a 0DB0: 15 byte, 4 note, ottave 3-5; inviluppo software 0, 2; 17 frame (0,28 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_0A_b12_s2` a 0DBF: 15 byte, 4 note, ottave 3-5; inviluppo software 0, 2; 17 frame (0,28 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_0A_b13_s2` a 0DCE: 15 byte, 4 note, ottave 3-5; inviluppo software 0, 2; 17 frame (0,28 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_0A_b14_s2` a 0DDD: 15 byte, 4 note, ottave 3-5; inviluppo software 0, 2; 17 frame (0,28 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_0A_b15_s2` a 0DEC: 15 byte, 4 note, ottave 3-5; inviluppo software 0, 2; 17 frame (0,28 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_0A_b16_s2` a 0DFB: 15 byte, 4 note, ottave 3-5; inviluppo software 0, 2; 17 frame (0,28 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_0A_b17_s2` a 0E0A: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 0B

Elenco a 0E0F, 9 record. 9 voci, 1 strato, 70 frame (1,17 s); inviluppo software 0, 2, 6; glissando; periodo diretto.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_0B_b9_s2` a 0E2B: 45 byte, 0 note, nessuna nota; inviluppo software 2, 6; glissando; periodo diretto; 70 frame (1,17 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_0B_b10_s2` a 0E58: 45 byte, 0 note, nessuna nota; inviluppo software 2, 6; glissando; periodo diretto; 70 frame (1,17 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_0B_b11_s2` a 0E85: 43 byte, 0 note, nessuna nota; inviluppo software 2; glissando; periodo diretto; 70 frame (1,17 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_0B_b12_s2` a 0EB0: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_0B_b13_s2` a 0EB5: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_0B_b14_s2` a 0EBA: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_0B_b15_s2` a 0EBF: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_0B_b16_s2` a 0EC4: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_0B_b17_s2` a 0EC9: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 0C

Elenco a 0ECE, 1 record. 1 voce, 1 strato, 192,5 frame (3,21 s); inviluppo software 2; glissando.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_0C_b9_s2` a 0ED2: 12 byte, 0 note, nessuna nota; inviluppo software 2; glissando; 192,5 frame (3,21 s)

### Comando 0D

Elenco a 0EDE, 2 record. 2 voci, 1 strato, 512,5 frame (8,54 s); inviluppo software 2; glissando.

- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_0D_b10_s2` a 0EE5: 12 byte, 0 note, nessuna nota; inviluppo software 2; glissando; 512,5 frame (8,54 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_0D_b13_s2` a 0EF1: 12 byte, 0 note, nessuna nota; inviluppo software 2; glissando; 512,5 frame (8,54 s)

### Comando 0E

Elenco a 0EFD, 3 record. 2 voci, 1 strato, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_0E_b10_s2` a 0F07: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_0E_b13_s2` a 0F0C: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_0E_b10_s2_r2` a 0F11: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 0F

Elenco a 0F16, 3 record. 3 voci, 1 strato, 512,5 frame (8,54 s); inviluppo software 2; glissando.

- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_0F_b10_s2` a 0F20: 12 byte, 0 note, nessuna nota; inviluppo software 2; glissando; 512,5 frame (8,54 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_0F_b13_s2` a 0F2C: 12 byte, 0 note, nessuna nota; inviluppo software 2; glissando; 512,5 frame (8,54 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_0F_b15_s2` a 0F38: 12 byte, 0 note, nessuna nota; inviluppo software 2; glissando; 512,5 frame (8,54 s)

### Comando 10

Elenco a 0F44, 2 record. 2 voci, 1 strato, 45,5 frame (0,76 s); inviluppo software 7.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_10_b9_s2` a 0F4B: 21 byte, 6 note, ottave 4-5; inviluppo software 7; 45,5 frame (0,76 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_10_b12_s2` a 0F60: 21 byte, 6 note, ottave 3-6; inviluppo software 7; 45,5 frame (0,76 s)

### Comando 11

Elenco a 0F75, 9 record. 9 voci, 1 strato, 61 frame (1,02 s); inviluppo software 0, 2, 7; trasposizione.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_11_b9_s2` a 0F91: 19 byte, 4 note, ottave 3-3; inviluppo software 0, 2, 7; trasposizione; 61 frame (1,02 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_11_b10_s2` a 0FA4: 19 byte, 4 note, ottave 3-3; inviluppo software 0, 2, 7; trasposizione; 61 frame (1,02 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_11_b11_s2` a 0FB7: 19 byte, 4 note, ottave 3-4; inviluppo software 0, 2, 7; trasposizione; 61 frame (1,02 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_11_b12_s2` a 0FCA: 19 byte, 4 note, ottave 4-4; inviluppo software 0, 2, 7; trasposizione; 61 frame (1,02 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_11_b13_s2` a 0FDD: 19 byte, 4 note, ottave 4-4; inviluppo software 0, 2, 7; trasposizione; 61 frame (1,02 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_11_b14_s2` a 0FF0: 19 byte, 4 note, ottave 4-5; inviluppo software 0, 2, 7; trasposizione; 61 frame (1,02 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_11_b15_s2` a 1003: 11 byte, 0 note, nessuna nota; inviluppo software 0; 61 frame (1,02 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_11_b16_s2` a 100E: 11 byte, 0 note, nessuna nota; inviluppo software 0; 61 frame (1,02 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_11_b17_s2` a 1019: 11 byte, 0 note, nessuna nota; inviluppo software 0; 61 frame (1,02 s)

### Comando 12

Elenco a 1024, 9 record. 9 voci, 1 strato, 63 frame (1,05 s); inviluppo software 0, 7.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_12_b9_s2` a 1040: 15 byte, 8 note, ottave 2-3; inviluppo software 7; 63 frame (1,05 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_12_b10_s2` a 104F: 15 byte, 8 note, ottave 3-4; inviluppo software 7; 63 frame (1,05 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_12_b11_s2` a 105E: 15 byte, 8 note, ottave 4-5; inviluppo software 7; 63 frame (1,05 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_12_b12_s2` a 106D: 15 byte, 8 note, ottave 5-6; inviluppo software 7; 63 frame (1,05 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_12_b13_s2` a 107C: 15 byte, 8 note, ottave 2-3; inviluppo software 7; 63 frame (1,05 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_12_b14_s2` a 108B: 15 byte, 8 note, ottave 3-4; inviluppo software 7; 63 frame (1,05 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_12_b15_s2` a 109A: 15 byte, 8 note, ottave 4-5; inviluppo software 7; 63 frame (1,05 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_12_b16_s2` a 10A9: 15 byte, 8 note, ottave 5-6; inviluppo software 7; 63 frame (1,05 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_12_b17_s2` a 10B8: 13 byte, 0 note, nessuna nota; inviluppo software 0; 63 frame (1,05 s)

### Comando 13

Elenco a 10C5, 4 record. 4 voci, 1 strato, ciclo; inviluppo software 1, 2, 8; rumore; trasposizione.

- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_1A_b2_s1` a 17A7: 204 byte, 28 note, ottave 0-0; inviluppo software 1, 2; rumore; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_13_b5_s1` a 10D2: 78 byte, 64 note, ottave 1-3; inviluppo software 8; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_13_b7_s1` a 1121: 127 byte, 71 note, ottave 3-4; inviluppo software 1; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_13_b8_s1` a 113D: 129 byte, 55 note, ottave 3-4; inviluppo software 8; trasposizione; ciclo

Flussi condivisi: `cmd_1A_b2_s1` anche nel comando 1A.

### Comando 14

Elenco a 11BF, 4 record. 4 voci, 1 strato, ciclo; inviluppo software 1, 2, 7, 8; rumore; trasposizione.

- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_14_b2_s1` a 11CC: 123 byte, 15 note, ottave 0-0; inviluppo software 1, 2; rumore; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_14_b5_s1` a 1248: 78 byte, 36 note, ottave 2-3; inviluppo software 2; trasposizione; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_14_b7_s1` a 1297: 144 byte, 91 note, ottave 3-5; inviluppo software 7; trasposizione; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_14_b8_s1` a 12A5: 144 byte, 91 note, ottave 3-5; inviluppo software 8; trasposizione; ciclo

### Comando 15

Elenco a 1336, 3 record. 3 voci, 1 strato, 18 frame (0,30 s); inviluppo software 2; trasposizione.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_15_b9_s2` a 1340: 26 byte, 12 note, ottave 2-5; inviluppo software 2; 18 frame (0,30 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_15_b12_s2` a 135A: 26 byte, 12 note, ottave 3-6; inviluppo software 2; 18 frame (0,30 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_15_b15_s2` a 1374: 28 byte, 12 note, ottave 2-5; inviluppo software 2; trasposizione; 18 frame (0,30 s)

### Comando 16

Elenco a 1390, 4 record. 4 voci, 1 strato, ciclo; inviluppo software 2; periodo diretto; rumore; trasposizione.

- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_16_b2_s1` a 139D: 27 byte, 3 note, ottave 0-0; inviluppo software 2; rumore; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_16_b5_s1` a 13B9: 232 byte, 146 note, ottave 3-5; inviluppo software 2; trasposizione; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_16_b7_s1` a 14A2: 198 byte, 3 note, ottave 2-2; inviluppo software 2; periodo diretto; trasposizione; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_16_b8_s1` a 1569: 164 byte, 144 note, ottave 2-6; inviluppo software 2; trasposizione; ciclo

### Comando 17

Elenco a 160E, 1 record. 1 voce, 1 strato, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_17_b0_s1` a 1612: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 18

Elenco a 1617, 3 record. 3 voci, 1 strato, 76 frame (1,27 s); inviluppo software 0, 2.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_18_b9_s2` a 1621: 24 byte, 13 note, ottave 4-5; inviluppo software 0, 2; 76 frame (1,27 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_18_b10_s2` a 1639: 24 byte, 13 note, ottave 5-6; inviluppo software 0, 2; 76 frame (1,27 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_18_b12_s2` a 1651: 24 byte, 13 note, ottave 6-7; inviluppo software 0, 2; 76 frame (1,27 s)

### Comando 19

Elenco a 1669, 4 record. 4 voci, 1 strato, ciclo; inviluppo software 0, 4, 7; rumore; trasposizione.

- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_19_b2_s1` a 1676: 82 byte, 2 note, ottave 0-0; inviluppo software 4; rumore; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_19_b5_s1` a 16C9: 157 byte, 70 note, ottave 3-4; inviluppo software 7; trasposizione; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_19_b7_s1` a 16D6: 157 byte, 70 note, ottave 3-4; inviluppo software 7; trasposizione; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_19_b8_s1` a 1774: 37 byte, 17 note, ottave 1-2; inviluppo software 0; trasposizione; ciclo

### Comando 1A

Elenco a 179A, 4 record. 4 voci, 1 strato, ciclo; inviluppo software 1, 2, 7, 8, 9; rumore; trasposizione.

- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_1A_b2_s1` a 17A7: 204 byte, 28 note, ottave 0-0; inviluppo software 1, 2; rumore; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_1A_b5_s1` a 1874: 106 byte, 80 note, ottave 1-3; inviluppo software 7; trasposizione; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_1A_b7_s1` a 18DF: 165 byte, 91 note, ottave 2-4; inviluppo software 8, 9; trasposizione; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_1A_b8_s1` a 1985: 165 byte, 91 note, ottave 3-5; inviluppo software 8, 9; trasposizione; ciclo

Flussi condivisi: `cmd_1A_b2_s1` anche nel comando 13.

### Comando 1B

Elenco a 19AA, 4 record. 4 voci, 1 strato, ciclo; inviluppo software 0, 2, 6, 9; glissando; rumore.

- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_1B_b2_s1` a 19B7: 48 byte, 0 note, nessuna nota; inviluppo software 0; glissando; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_1B_b5_s1` a 19E8: 20 byte, 5 note, ottave 3-4; inviluppo software 6; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_1B_b7_s1` a 19FD: 22 byte, 4 note, ottave 0-0; inviluppo software 9; rumore; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_1B_b8_s1` a 1A14: 441 byte, 164 note, ottave 2-5; inviluppo software 2; ciclo

### Comando 1C

Elenco a 1BCE, 4 record. 4 voci, 1 strato, ciclo; inviluppo software 2, 8; rumore; trasposizione.

- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_1C_b2_s1` a 1BDB: 52 byte, 4 note, ottave 0-0; inviluppo software 2; rumore; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_1C_b5_s1` a 1C10: 95 byte, 72 note, ottave 1-2; inviluppo software 8; trasposizione; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_1C_b7_s1` a 1C1E: 95 byte, 72 note, ottave 1-2; inviluppo software 8; trasposizione; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_1C_b8_s1` a 1C2C: 95 byte, 72 note, ottave 1-2; inviluppo software 8; trasposizione; ciclo

### Comando 1D

Elenco a 1C8C, 9 record. 9 voci, 1 strato, 150,5 frame (2,51 s); inviluppo software 0, 12, 14.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_1D_b0_s1` a 1CA8: 27 byte, 8 note, ottave 4-5; inviluppo software 12, 14; 149,5 frame (2,49 s)
- blocco 1, strato 1, chip 1 voce 2, flusso `cmd_1D_b1_s1` a 1CC3: 27 byte, 8 note, ottave 5-6; inviluppo software 12, 14; 149,5 frame (2,49 s)
- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_1D_b2_s1` a 1CDE: 29 byte, 8 note, ottave 4-5; inviluppo software 12, 14; 150,5 frame (2,51 s)
- blocco 3, strato 1, chip 2 voce 3, flusso `cmd_1D_b3_s1` a 1CFB: 29 byte, 8 note, ottave 5-6; inviluppo software 12, 14; 150,5 frame (2,51 s)
- blocco 4, strato 1, chip 2 voce 2, flusso `cmd_1D_b4_s1` a 1D18: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_1D_b5_s1` a 1D1D: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 6, strato 1, chip 3 voce 3, flusso `cmd_1D_b6_s1` a 1D22: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_1D_b7_s1` a 1D27: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_1D_b8_s1` a 1D2C: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 1E

Elenco a 1D31, 4 record. 4 voci, 1 strato, ciclo; inviluppo software 8; trasposizione.

- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_1E_b2_s1` a 1D3E: 115 byte, 96 note, ottave 3-5; inviluppo software 8; trasposizione; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_1E_b5_s1` a 1DB2: 115 byte, 96 note, ottave 3-5; inviluppo software 8; trasposizione; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_1E_b7_s1` a 1E26: 115 byte, 96 note, ottave 2-5; inviluppo software 8; trasposizione; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_1E_b8_s1` a 1E9A: 115 byte, 96 note, ottave 2-4; inviluppo software 8; trasposizione; ciclo

### Comando 1F

Elenco a 1F0E, 9 record. 9 voci, 1 strato, ciclo; inviluppo software 0, 11, 14; periodo diretto; rumore.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_1F_b0_s1` a 1F2A: 138 byte, 22 note, ottave 0-0; inviluppo software 0, 11; rumore; ciclo
- blocco 1, strato 1, chip 1 voce 2, flusso `cmd_1F_b1_s1` a 1FB5: 33 byte, 9 note, ottave 2-2; inviluppo software 11; ciclo
- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_1F_b2_s1` a 1FD7: 31 byte, 9 note, ottave 1-1; inviluppo software 11; ciclo
- blocco 3, strato 1, chip 2 voce 3, flusso `cmd_1F_b3_s1` a 1FF7: 85 byte, 36 note, ottave 3-4; inviluppo software 14; ciclo
- blocco 4, strato 1, chip 2 voce 2, flusso `cmd_1F_b4_s1` a 204D: 85 byte, 36 note, ottave 3-4; inviluppo software 14; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_1F_b5_s1` a 20A3: 85 byte, 36 note, ottave 3-4; inviluppo software 14; ciclo
- blocco 6, strato 1, chip 3 voce 3, flusso `cmd_1F_b6_s1` a 20F9: 136 byte, 0 note, nessuna nota; inviluppo software 14; periodo diretto; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_1F_b7_s1` a 2182: 136 byte, 0 note, nessuna nota; inviluppo software 14; periodo diretto; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_1F_b8_s1` a 220B: 136 byte, 0 note, nessuna nota; inviluppo software 14; periodo diretto; ciclo

### Comando 20

Elenco a 2294, 9 record. 9 voci, 1 strato, ciclo; inviluppo software 0, 6, 8; rumore.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_20_b0_s1` a 22B0: 24 byte, 16 note, ottave 2-4; inviluppo software 0; ciclo
- blocco 1, strato 1, chip 1 voce 2, flusso `cmd_20_b1_s1` a 22C9: 26 byte, 16 note, ottave 2-4; inviluppo software 0; ciclo
- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_20_b2_s1` a 22E4: 41 byte, 7 note, ottave 0-0; inviluppo software 0, 6; rumore; ciclo
- blocco 3, strato 1, chip 2 voce 3, flusso `cmd_20_b3_s1` a 230E: 11 byte, 1 note, ottave 0-0; inviluppo software 6; rumore; ciclo
- blocco 4, strato 1, chip 2 voce 2, flusso `cmd_20_b4_s1` a 231A: 40 byte, 32 note, ottave 1-2; inviluppo software 0; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_20_b5_s1` a 2343: 141 byte, 64 note, ottave 2-4; inviluppo software 8; ciclo
- blocco 6, strato 1, chip 3 voce 3, flusso `cmd_20_b6_s1` a 23D1: 141 byte, 64 note, ottave 3-5; inviluppo software 8; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_20_b7_s1` a 245F: 58 byte, 11 note, ottave 4-4; inviluppo software 8; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_20_b8_s1` a 249A: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 21

Elenco a 249F, 4 record. 4 voci, 1 strato, ciclo; inviluppo software 0, 2, 4; rumore; trasposizione.

- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_21_b2_s1` a 24AC: 26 byte, 4 note, ottave 0-0; inviluppo software 4; rumore; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_21_b5_s1` a 24C7: 82 byte, 41 note, ottave 1-2; inviluppo software 0; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_21_b7_s1` a 251A: 93 byte, 63 note, ottave 2-3; inviluppo software 2; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_21_b8_s1` a 2530: 95 byte, 63 note, ottave 2-4; inviluppo software 2; trasposizione; ciclo

### Comando 22

Elenco a 2590, 5 record. 5 voci, 1 strato, 196 frame (3,27 s); inviluppo software 9; rumore.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_22_b0_s1` a 25A0: 79 byte, 10 note, ottave 0-0; inviluppo software 9; rumore; 196 frame (3,27 s)
- blocco 1, strato 1, chip 1 voce 2, flusso `cmd_22_b1_s1` a 25EF: 34 byte, 13 note, ottave 1-2; inviluppo software 9; 196 frame (3,27 s)
- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_22_b2_s1` a 2611: 34 byte, 13 note, ottave 2-3; inviluppo software 9; 196 frame (3,27 s)
- blocco 3, strato 1, chip 2 voce 3, flusso `cmd_22_b3_s1` a 2633: 34 byte, 13 note, ottave 3-4; inviluppo software 9; 196 frame (3,27 s)
- blocco 4, strato 1, chip 2 voce 2, flusso `cmd_22_b4_s1` a 2655: 34 byte, 13 note, ottave 4-5; inviluppo software 9; 196 frame (3,27 s)

### Comando 23

Elenco a 2677, 9 record. 9 voci, 1 strato, ciclo; inviluppo software 0, 8, 11, 14; periodo diretto; rumore.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_23_b0_s1` a 2693: 64 byte, 36 note, ottave 3-5; inviluppo software 8; ciclo
- blocco 1, strato 1, chip 1 voce 2, flusso `cmd_23_b1_s1` a 26D4: 64 byte, 36 note, ottave 2-4; inviluppo software 8; ciclo
- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_23_b2_s1` a 2715: 64 byte, 36 note, ottave 3-5; inviluppo software 8; ciclo
- blocco 3, strato 1, chip 2 voce 3, flusso `cmd_23_b3_s1` a 2756: 64 byte, 36 note, ottave 3-4; inviluppo software 8; ciclo
- blocco 4, strato 1, chip 2 voce 2, flusso `cmd_23_b4_s1` a 2797: 136 byte, 0 note, nessuna nota; inviluppo software 8; periodo diretto; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_23_b5_s1` a 2820: 44 byte, 32 note, ottave 1-2; inviluppo software 8; ciclo
- blocco 6, strato 1, chip 3 voce 3, flusso `cmd_23_b6_s1` a 284D: 176 byte, 23 note, ottave 0-0; inviluppo software 11; rumore; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_23_b7_s1` a 28FE: 112 byte, 96 note, ottave 4-6; inviluppo software 14; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_23_b8_s1` a 296F: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 24

Elenco a 2974, 9 record. 9 voci, 1 strato, ciclo; inviluppo software 0, 2, 7; trasposizione.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_24_b0_s1` a 2990: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 1, strato 1, chip 1 voce 2, flusso `cmd_24_b1_s1` a 2995: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_24_b2_s1` a 299A: 57 byte, 40 note, ottave 3-4; inviluppo software 2, 7; trasposizione; ciclo
- blocco 3, strato 1, chip 2 voce 3, flusso `cmd_24_b3_s1` a 29D4: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 4, strato 1, chip 2 voce 2, flusso `cmd_24_b4_s1` a 29D9: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_24_b5_s1` a 29DE: 57 byte, 40 note, ottave 3-4; inviluppo software 2, 7; trasposizione; ciclo
- blocco 6, strato 1, chip 3 voce 3, flusso `cmd_24_b6_s1` a 2A18: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_24_b7_s1` a 2A1D: 57 byte, 40 note, ottave 2-4; inviluppo software 2, 7; trasposizione; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_24_b8_s1` a 2A57: 57 byte, 40 note, ottave 1-2; inviluppo software 2, 7; trasposizione; ciclo

### Comando 25

Elenco a 2A91, 18 record. 9 voci, 2 strati, 99,5 frame (1,66 s); inviluppo software 0, 2, 6, 7.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_25_b0_s1` a 2AC8: 41 byte, 24 note, ottave 3-4; inviluppo software 2, 6, 7; 99,5 frame (1,66 s)
- blocco 1, strato 1, chip 1 voce 2, flusso `cmd_25_b1_s1` a 2AF1: 41 byte, 24 note, ottave 2-3; inviluppo software 2, 6, 7; 99,5 frame (1,66 s)
- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_25_b2_s1` a 2B1A: 41 byte, 24 note, ottave 1-2; inviluppo software 2, 6, 7; 99,5 frame (1,66 s)
- blocco 3, strato 1, chip 2 voce 3, flusso `cmd_25_b3_s1` a 2B43: 30 byte, 8 note, ottave 3-4; inviluppo software 2, 6, 7; 99,5 frame (1,66 s)
- blocco 4, strato 1, chip 2 voce 2, flusso `cmd_25_b4_s1` a 2B61: 30 byte, 8 note, ottave 3-4; inviluppo software 2, 6, 7; 99,5 frame (1,66 s)
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_25_b5_s1` a 2B7F: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 6, strato 1, chip 3 voce 3, flusso `cmd_25_b6_s1` a 2B84: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_25_b7_s1` a 2B89: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_25_b8_s1` a 2B8E: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_25_b9_s2` a 2B93: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_25_b10_s2` a 2B98: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_25_b11_s2` a 2B9D: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_25_b12_s2` a 2BA2: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_25_b13_s2` a 2BA7: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_25_b14_s2` a 2BAC: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_25_b15_s2` a 2BB1: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_25_b16_s2` a 2BB6: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_25_b17_s2` a 2BBB: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 26

Elenco a 2BC0, 18 record. 9 voci, 2 strati, 85 frame (1,42 s); inviluppo software 0, 2; rumore.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_26_b0_s1` a 2BF7: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 1, strato 1, chip 1 voce 2, flusso `cmd_26_b1_s1` a 2BFC: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_26_b2_s1` a 2C01: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 3, strato 1, chip 2 voce 3, flusso `cmd_26_b3_s1` a 2C06: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 4, strato 1, chip 2 voce 2, flusso `cmd_26_b4_s1` a 2C0B: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_26_b5_s1` a 2C10: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 6, strato 1, chip 3 voce 3, flusso `cmd_26_b6_s1` a 2C15: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_26_b7_s1` a 2C1A: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_26_b8_s1` a 2C1F: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_26_b9_s2` a 2C24: 89 byte, 16 note, ottave 0-0; inviluppo software 2; rumore; 85 frame (1,42 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_26_b10_s2` a 2C7D: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_26_b11_s2` a 2C82: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_26_b12_s2` a 2C87: 89 byte, 16 note, ottave 0-0; inviluppo software 2; rumore; 85 frame (1,42 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_26_b13_s2` a 2CE0: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_26_b14_s2` a 2CE5: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_26_b15_s2` a 2CEA: 89 byte, 16 note, ottave 0-0; inviluppo software 2; rumore; 85 frame (1,42 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_26_b16_s2` a 2D43: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_26_b17_s2` a 2D48: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 27

Elenco a 2D4D, 1 record. 1 voce, 1 strato, 46 frame (0,77 s); inviluppo software 2; trasposizione.

- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_27_b15_s2` a 2D51: 115 byte, 66 note, ottave 2-3; inviluppo software 2; trasposizione; 46 frame (0,77 s)

### Comando 28

Elenco a 2DC4, 4 record. 4 voci, 1 strato, ciclo; inviluppo software 1, 2, 4; periodo diretto; rumore; trasposizione.

- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_28_b2_s1` a 2DD1: 83 byte, 8 note, ottave 0-0; inviluppo software 2, 4; rumore; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_28_b5_s1` a 2E25: 50 byte, 16 note, ottave 2-4; inviluppo software 2; trasposizione; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_28_b7_s1` a 2E58: 142 byte, 82 note, ottave 4-5; inviluppo software 2; periodo diretto; trasposizione; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_28_b8_s1` a 2EE7: 42 byte, 8 note, ottave 1-2; inviluppo software 1; trasposizione; ciclo

### Comando 29

Elenco a 2F12, 1 record. 1 voce, 1 strato, 8 frame (0,13 s); inviluppo software 2.

- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_29_b15_s2` a 2F16: 22 byte, 16 note, ottave 2-2; inviluppo software 2; 8 frame (0,13 s)

### Comando 2A

Elenco a 2F2C, 4 record. 4 voci, 1 strato, ciclo; inviluppo software 0, 1, 2, 4; rumore; trasposizione.

- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_2A_b2_s1` a 2F39: 27 byte, 3 note, ottave 0-0; inviluppo software 0, 1, 4; rumore; ciclo
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_2A_b5_s1` a 2F55: 106 byte, 96 note, ottave 2-5; inviluppo software 2; trasposizione; ciclo
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_2A_b7_s1` a 2FC0: 106 byte, 96 note, ottave 2-4; inviluppo software 2; trasposizione; ciclo
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_2A_b8_s1` a 302B: 106 byte, 96 note, ottave 1-4; inviluppo software 2; trasposizione; ciclo

### Comando 2B

Elenco a 3096, 9 record. 9 voci, 1 strato, 160 frame (2,67 s); inviluppo software 0, 8; trasposizione.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_2B_b9_s2` a 30B2: 12 byte, 0 note, nessuna nota; inviluppo software 0; 40 frame (0,67 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_2B_b10_s2` a 30BE: 12 byte, 0 note, nessuna nota; inviluppo software 0; 40 frame (0,67 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_2B_b11_s2` a 30CA: 35 byte, 8 note, ottave 3-4; inviluppo software 8; trasposizione; 160 frame (2,67 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_2B_b12_s2` a 30EE: 12 byte, 0 note, nessuna nota; inviluppo software 0; 40 frame (0,67 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_2B_b13_s2` a 30FA: 12 byte, 0 note, nessuna nota; inviluppo software 0; 40 frame (0,67 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_2B_b14_s2` a 3106: 35 byte, 8 note, ottave 3-4; inviluppo software 8; trasposizione; 160 frame (2,67 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_2B_b15_s2` a 312A: 35 byte, 8 note, ottave 3-4; inviluppo software 8; trasposizione; 160 frame (2,67 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_2B_b16_s2` a 314E: 35 byte, 8 note, ottave 2-3; inviluppo software 8; trasposizione; 160 frame (2,67 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_2B_b17_s2` a 3172: 35 byte, 8 note, ottave 2-3; inviluppo software 8; trasposizione; 160 frame (2,67 s)

### Comando 2C

Elenco a 3196, 1 record. 1 voce, 1 strato, 20 frame (0,33 s); inviluppo software 2; rumore.

- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_2C_b10_s2` a 319A: 33 byte, 8 note, ottave 0-0; inviluppo software 2; rumore; 20 frame (0,33 s)

### Comando 2D

Elenco a 31BB, 2 record. 2 voci, 1 strato, 196 frame (3,27 s); inviluppo software 2.

- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_2D_b10_s2` a 31C2: 88 byte, 56 note, ottave 3-3; inviluppo software 2; 196 frame (3,27 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_2D_b15_s2` a 321A: 88 byte, 56 note, ottave 5-5; inviluppo software 2; 196 frame (3,27 s)

### Comando 2E

Elenco a 3272, 1 record. 1 voce, 1 strato, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_2E_b0_s1` a 3276: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 2F

Elenco a 327D, 2 record. 2 voci, 1 strato, 132 frame (2,20 s); inviluppo software 2; inviluppo hardware; rumore.

- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_2F_b10_s2` a 3284: 42 byte, 9 note, ottave 0-0; inviluppo software 2; inviluppo hardware; rumore; 132 frame (2,20 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_2F_b13_s2` a 32AE: 42 byte, 9 note, ottave 0-0; inviluppo software 2; inviluppo hardware; rumore; 132 frame (2,20 s)

### Comando 30

Elenco a 32D8, 2 record. 2 voci, 1 strato, 20 frame (0,33 s); inviluppo software 1, 2; periodo diretto; rumore.

- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_30_b10_s2` a 32DF: 29 byte, 3 note, ottave 0-0; inviluppo software 1, 2; periodo diretto; rumore; 20 frame (0,33 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_30_b13_s2` a 32FC: 29 byte, 3 note, ottave 0-0; inviluppo software 1, 2; periodo diretto; rumore; 20 frame (0,33 s)

### Comando 31

Elenco a 3319, 3 record. 3 voci, 1 strato, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_31_b10_s2` a 3323: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_31_b13_s2` a 3328: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_31_b15_s2` a 332D: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 32

Elenco a 3332, 9 record. 9 voci, 1 strato, 40 frame (0,67 s); inviluppo software 0, 2; trasposizione.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_32_b9_s2` a 334E: 9 byte, 0 note, nessuna nota; inviluppo software 0; 40 frame (0,67 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_32_b10_s2` a 3357: 9 byte, 0 note, nessuna nota; inviluppo software 0; 40 frame (0,67 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_32_b11_s2` a 3360: 60 byte, 49 note, ottave 2-4; inviluppo software 0, 2; trasposizione; 40 frame (0,67 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_32_b12_s2` a 339C: 9 byte, 0 note, nessuna nota; inviluppo software 0; 40 frame (0,67 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_32_b13_s2` a 33A5: 9 byte, 0 note, nessuna nota; inviluppo software 0; 40 frame (0,67 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_32_b14_s2` a 33AE: 60 byte, 49 note, ottave 3-5; inviluppo software 0, 2; trasposizione; 40 frame (0,67 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_32_b15_s2` a 33EA: 9 byte, 0 note, nessuna nota; inviluppo software 0; 40 frame (0,67 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_32_b16_s2` a 33F3: 9 byte, 0 note, nessuna nota; inviluppo software 0; 40 frame (0,67 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_32_b17_s2` a 33FC: 9 byte, 0 note, nessuna nota; inviluppo software 0; 40 frame (0,67 s)

### Comando 33

Elenco a 3405, 2 record. 2 voci, 1 strato, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_33_b10_s2` a 340C: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_33_b13_s2` a 3411: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 34

Elenco a 3416, 9 record. 9 voci, 1 strato, 76 frame (1,27 s); inviluppo software 0, 7; trasposizione.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_34_b9_s2` a 3432: 12 byte, 4 note, ottave 2-5; inviluppo software 7; trasposizione; 64 frame (1,07 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_34_b10_s2` a 343E: 14 byte, 4 note, ottave 2-5; inviluppo software 7; trasposizione; 68 frame (1,13 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_34_b11_s2` a 344C: 14 byte, 4 note, ottave 2-6; inviluppo software 7; trasposizione; 72 frame (1,20 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_34_b12_s2` a 345A: 14 byte, 4 note, ottave 3-6; inviluppo software 7; trasposizione; 76 frame (1,27 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_34_b13_s2` a 3468: 8 byte, 0 note, nessuna nota; inviluppo software 0; 64 frame (1,07 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_34_b14_s2` a 3470: 8 byte, 0 note, nessuna nota; inviluppo software 0; 64 frame (1,07 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_34_b15_s2` a 3478: 8 byte, 0 note, nessuna nota; inviluppo software 0; 64 frame (1,07 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_34_b16_s2` a 3480: 8 byte, 0 note, nessuna nota; inviluppo software 0; 64 frame (1,07 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_34_b17_s2` a 3488: 8 byte, 0 note, nessuna nota; inviluppo software 0; 64 frame (1,07 s)

### Comando 35

Elenco a 3490, 2 record. 2 voci, 1 strato, 16 frame (0,27 s); inviluppo software 0, 2; rumore.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_35_b9_s2` a 3497: 9 byte, 1 note, ottave 1-1; inviluppo software 0; rumore; 16 frame (0,27 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_35_b10_s2` a 34A0: 31 byte, 9 note, ottave 0-1; inviluppo software 2; 9 frame (0,15 s)

### Comando 36

Elenco a 34BF, 2 record. 2 voci, 1 strato, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_36_b9_s2` a 34C6: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_36_b10_s2` a 34CB: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 37

Elenco a 34D0, 1 record. 1 voce, 1 strato, 64 frame (1,07 s); inviluppo software 2; rumore; trasposizione.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_37_b9_s2` a 34D4: 56 byte, 16 note, ottave 0-0; inviluppo software 2; rumore; trasposizione; 64 frame (1,07 s)

### Comando 38

Elenco a 350C, 9 record. 9 voci, 1 strato, 66 frame (1,10 s); inviluppo software 0, 7, 9; trasposizione.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_38_b9_s2` a 3528: 8 byte, 0 note, nessuna nota; inviluppo software 9; 64 frame (1,07 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_38_b10_s2` a 3530: 8 byte, 0 note, nessuna nota; inviluppo software 9; 64 frame (1,07 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_38_b11_s2` a 3538: 24 byte, 16 note, ottave 3-4; inviluppo software 7; trasposizione; 64 frame (1,07 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_38_b12_s2` a 3550: 8 byte, 0 note, nessuna nota; inviluppo software 0; 64 frame (1,07 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_38_b13_s2` a 3558: 8 byte, 0 note, nessuna nota; inviluppo software 0; 64 frame (1,07 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_38_b14_s2` a 3560: 24 byte, 16 note, ottave 4-5; inviluppo software 7; trasposizione; 64 frame (1,07 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_38_b15_s2` a 3578: 8 byte, 0 note, nessuna nota; inviluppo software 0; 64 frame (1,07 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_38_b16_s2` a 3580: 26 byte, 16 note, ottave 3-4; inviluppo software 7; trasposizione; 66 frame (1,10 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_38_b17_s2` a 359A: 26 byte, 16 note, ottave 4-5; inviluppo software 7; trasposizione; 66 frame (1,10 s)

### Comando 39

Elenco a 35B4, 9 record. 9 voci, 1 strato, 5 frame (0,08 s); inviluppo software 0, 2; rumore.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_39_b9_s2` a 35D0: 34 byte, 10 note, ottave 3-3; inviluppo software 2; 5 frame (0,08 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_39_b10_s2` a 35F2: 34 byte, 10 note, ottave 3-3; inviluppo software 2; 5 frame (0,08 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_39_b11_s2` a 3614: 54 byte, 10 note, ottave 0-0; inviluppo software 2; rumore; 5 frame (0,08 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_39_b12_s2` a 364A: 54 byte, 10 note, ottave 0-0; inviluppo software 2; rumore; 5 frame (0,08 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_39_b13_s2` a 3680: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_39_b14_s2` a 3685: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_39_b15_s2` a 368A: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_39_b16_s2` a 368F: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_39_b17_s2` a 3694: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 3A

Elenco a 3699, 1 record. 1 voce, 1 strato, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_3A_b0_s1` a 369D: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 3B

Elenco a 36A4, 1 record. 1 voce, 1 strato, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_3B_b0_s1` a 36A8: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 3C

Elenco a 36AF, 1 record. 1 voce, 1 strato, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_3C_b0_s1` a 36B3: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 3D

Elenco a 36BA, 1 record. 1 voce, 1 strato, 1800 frame (30,00 s); inviluppo software 0.

- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_3D_b9_s2` a 36BE: 45 byte, 35 note, ottave 5-5; inviluppo software 0; 1800 frame (30,00 s)

### Comando 3E

Elenco a 36EB, 14 record. 9 voci, 2 strati, 0,5 frame (0,01 s); inviluppo software 0.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_3E_b0_s1` a 3716: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 1, strato 1, chip 1 voce 2, flusso `cmd_3E_b1_s1` a 371B: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_3E_b2_s1` a 3720: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 3, strato 1, chip 2 voce 3, flusso `cmd_3E_b3_s1` a 3725: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 4, strato 1, chip 2 voce 2, flusso `cmd_3E_b4_s1` a 372A: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_3E_b5_s1` a 372F: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 6, strato 1, chip 3 voce 3, flusso `cmd_3E_b6_s1` a 3734: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_3E_b7_s1` a 3739: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_3E_b8_s1` a 373E: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_3E_b11_s2` a 3743: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_3E_b14_s2` a 3748: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_3E_b15_s2` a 374D: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_3E_b16_s2` a 3752: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_3E_b17_s2` a 3757: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

### Comando 3F

Elenco a 375C, 18 record. 9 voci, 2 strati, 1,5 frame (0,03 s); inviluppo software 0.

- blocco 0, strato 1, chip 1 voce 3, flusso `cmd_3F_b0_s1` a 3793: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 1, strato 1, chip 1 voce 2, flusso `cmd_3F_b1_s1` a 379A: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 2, strato 1, chip 1 voce 1, flusso `cmd_3F_b2_s1` a 37A1: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 3, strato 1, chip 2 voce 3, flusso `cmd_3F_b3_s1` a 37A8: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 4, strato 1, chip 2 voce 2, flusso `cmd_3F_b4_s1` a 37AF: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 5, strato 1, chip 2 voce 1, flusso `cmd_3F_b5_s1` a 37B6: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 6, strato 1, chip 3 voce 3, flusso `cmd_3F_b6_s1` a 37BD: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 7, strato 1, chip 3 voce 2, flusso `cmd_3F_b7_s1` a 37C4: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 8, strato 1, chip 3 voce 1, flusso `cmd_3F_b8_s1` a 37CB: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 9, strato 2, chip 1 voce 3, flusso `cmd_3F_b9_s2` a 37D2: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 10, strato 2, chip 1 voce 2, flusso `cmd_3F_b10_s2` a 37D9: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 11, strato 2, chip 1 voce 1, flusso `cmd_3F_b11_s2` a 37E0: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 12, strato 2, chip 2 voce 3, flusso `cmd_3F_b12_s2` a 37E7: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 13, strato 2, chip 2 voce 2, flusso `cmd_3F_b13_s2` a 37EE: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 14, strato 2, chip 2 voce 1, flusso `cmd_3F_b14_s2` a 37F5: 14 byte, 0 note, nessuna nota; inviluppo software 0; 1,5 frame (0,03 s)
- blocco 15, strato 2, chip 3 voce 3, flusso `cmd_3F_b15_s2` a 37FC: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 16, strato 2, chip 3 voce 2, flusso `cmd_3F_b16_s2` a 3803: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 17, strato 2, chip 3 voce 1, flusso `cmd_3F_b17_s2` a 380A: 7 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)

## Residuo 3811 (mai collegato)

Elenco a 3811 con 9 record, che nessuna voce della tabella dei comandi
raggiunge: resto di un comando tolto. Estratto come `residuo_3811` per
ricomporre la ROM. 9 voci, 1 strato, ciclo; inviluppo software 0, 8.

- blocco 0, strato 1, chip 1 voce 3, flusso `residuo_3811_b0_s1` a 382D: 10 byte, 2 note, ottave 3-4; inviluppo software 8; ciclo
- blocco 1, strato 1, chip 1 voce 2, flusso `residuo_3811_b1_s1` a 3838: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 2, strato 1, chip 1 voce 1, flusso `residuo_3811_b2_s1` a 383D: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 3, strato 1, chip 2 voce 3, flusso `residuo_3811_b3_s1` a 3842: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 4, strato 1, chip 2 voce 2, flusso `residuo_3811_b4_s1` a 3847: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 5, strato 1, chip 2 voce 1, flusso `residuo_3811_b5_s1` a 384C: 5 byte, 1 note, ottave 0-0; inviluppo software 0; 1 frame (0,02 s)
- blocco 6, strato 1, chip 3 voce 3, flusso `residuo_3811_b6_s1` a 3851: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 7, strato 1, chip 3 voce 2, flusso `residuo_3811_b7_s1` a 3856: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
- blocco 8, strato 1, chip 3 voce 1, flusso `residuo_3811_b8_s1` a 385B: 5 byte, 0 note, nessuna nota; inviluppo software 0; 0,5 frame (0,01 s)
