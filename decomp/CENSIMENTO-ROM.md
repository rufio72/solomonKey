# Censimento delle letture della ROM a runtime

Data: 2026-11-06 (brief 61). Compito di sola analisi: nessun file del gioco è
cambiato. Strumento: `decomp/censimento_rom.py` (sola lettura, scrive solo
`out/censimento_rom_letti.txt` con `--unisci`). Traguardo a cui serve (fuori
perimetro qui): un binario `gioca` che non legge nessuna ROM, con i dati Tecmo
raccolti in una cartella `tecmo/`.

## Sintesi

| misura | valore |
|---|---|
| occorrenze di `leggi(` in `harness/*.c` | **201** su 190 righe (155 tabelle fisse, 6 lettori generici, 35 solo RAM, 5 in commenti) |
| occorrenze di `aud_leggi(` | 21 su 19 righe (1 definizione, 2 solo RAM, 18 dati) |
| occorrenze mai eseguite nei percorsi misurati | 32 di `leggi(`, 2 di `aud_leggi(` |
| voci del Z80 principale | 88: 87 «letto dalla ROM» (17739 byte), 1 «estratto» (4 byte) |
| voci del Z80 audio | 2: 1 «letto dalla ROM» (32 byte), 1 «estratto» (14906 byte) |
| byte ROM letti davvero (unione dei percorsi) | principale 5888, audio 6180 (6148 dai dati estratti, 32 dal driver) |
| letture fuori da ogni voce | **0** (più 1 tocco non di dato a FDEE, vedi «Scoperte») |
| letture ROM in C000-EFFF | **0** |
| letture di codice eseguito (out/copertura.txt) | 0 |
| tabelle a mano `rst20_tables.h` / `rst38_tables.h` | 42, tutte uguali alla ROM |
| `8.3jk` oltre i primi 4096 byte | solo FF, mai caricato |

Dopo il lotto di estrazione resterebbero da togliere: `out/main64.bin`
(65536 byte, di cui 17739 in voci da estrarre) e `1.3jk` (16384 byte, di cui
oggi servono 32 byte di dato più il codice del driver per decisione del brief).

## Come si rifà la misura

1. Copia di `decomp/harness/` nella scratchpad (mai nel repository). Nella
   copia: in `ram.h` `leggi(a)` diventa una macro che chiama
   `cens_leggi(a, __FILE__, __LINE__)` prima della lettura vera, e `M(a)`
   passa da `cens_m()` che registra gli indirizzi sopra F000 (la copia della
   finestra F000-FFFF in `ram[]`); in `audio.h` `aud_leggi(a)` diventa una
   macro con `cens_aud()` (la definizione in `audio_piattaforma.c` la esclude
   con `#define CENS_NO_AUD`). Un `cens.c` aggiunto a `SORGENTI_BASE` tiene
   per sito una bitmap di 64 KB e all'uscita scrive, se c'è `CENS_OUT`, righe
   `S <L|M|A> <file> <riga> <letture rom> <letture ram>` e `I <inizio> <fine>`.
2. Con la copia (stessi risultati dell'originale: partita 1A a 4000 frame dà
   `HASH ultimo 474cc37bfa97faeb`, `AUDIO hash b6758278078e86fe` sia con la
   copia sia con `harness/gioca`), da `decomp/`, `SDL_VIDEODRIVER=dummy`:
   - giro dal boot: `gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt
     out/ingressi_replay.txt` → `GIRO COMPLETO 0-5499: saldatura 0 byte,
     spedidore 0 divergenze, 30 a metà attività (attese), 13 frame resi con
     hash coincidenti`, 30 righe MEZZA;
   - le quattro ricette versionate, ciascuna fino all'ultimo frame + 600:
     `bottiglia_rossa.reg` (-n 11167), `partita_1a.reg` (-n 3919),
     `riferimento_nuova.reg` (-n 14525), `stanze_bonus.reg` (-n 12387), tutte
     senza FERMO; `riferimento_nuova2.reg` nominato dal brief non esiste;
   - dieci semi: `ricette/genera_ingressi.py S F 20000` per S = 1..10, poi
     `gioca --riproduci F -n 20000 --muto`: tutti `rc=0`;
   - in più, senza ingressi: `gioca -n 30000 --muto` si ferma con `FERMO
     routine 39B9 mancante al frame 3385` (coda nota della demo).
3. `python3 -I censimento_rom.py --unisci <file della copia>` scrive
   `out/censimento_rom_letti.txt` (non versionato: 171 siti, 342 intervalli
   uniti del principale). La copia si cancella a fine lavoro.

Una zona mai letta è «non raggiunta nei percorsi misurati», non inutile: il
lotto di estrazione la estrae comunque se sta in una voce.

## Occorrenze di `leggi(` (201)

Il conteggio è quello del brief (`grep -hoE '(^|[^_a-z0-9])leggi\(' harness/*.c`).
Categorie: **V** = la riga legge una o più tabelle fisse (voci sotto); **G** =
lettore generico, l'indirizzo arriva dal chiamante e può essere ROM o RAM
(`sub_0010` = `rst 10`, `sub_6771` = stampa di stringhe, `sub_3569` = copia
con sorgente da tabella, `sub_2368`); **R** = l'indirizzo è sempre in RAM
(c000-ffff, `leggi` usato per uniformità); **C** = la parola compare in un
commento. La classificazione riga per riga è nel dizionario `SITI` dello
script; `--verifica` controlla che ogni riga con `leggi(` sia classificata,
che ogni byte letto da un sito V stia in una delle sue voci e che nessun sito
R legga la ROM.

| file | V | G | R | C | totale |
|---|---|---|---|---|---|
| attivita_06b.c | 2 | 0 | 0 | 0 | 2 |
| attivita_12_13.c | 1 | 0 | 0 | 0 | 1 |
| attivita_16.c | 1 | 0 | 1 | 0 | 2 |
| attivita_16b.c | 2 | 0 | 3 | 0 | 5 |
| attivita_25.c | 1 | 0 | 0 | 0 | 1 |
| attivita_attract.c | 5 | 0 | 0 | 0 | 5 |
| attivita_attract07.c | 2 | 0 | 0 | 0 | 2 |
| attivita_attract07a.c | 5 | 0 | 0 | 0 | 5 |
| attivita_attract07b.c | 9 | 0 | 0 | 0 | 9 |
| attivita_attract07c.c | 4 | 0 | 0 | 0 | 4 |
| attivita_attract07d.c | 3 | 0 | 0 | 0 | 3 |
| attivita_piccoli.c | 1 | 0 | 0 | 0 | 1 |
| macchina.c | 3 | 0 | 0 | 1 | 4 |
| macchina_libera.c | 0 | 0 | 0 | 1 | 1 |
| nemici.c | 3 | 0 | 0 | 0 | 3 |
| nemici_comparsa.c | 1 | 0 | 0 | 0 | 1 |
| nemici_comparsab.c | 0 | 0 | 1 | 0 | 1 |
| nemici_comuni.c | 7 | 0 | 0 | 0 | 7 |
| nemici_tipi_08_14.c | 1 | 0 | 3 | 0 | 4 |
| nemici_tipi_10_13.c | 0 | 0 | 1 | 0 | 1 |
| nemici_tipi_10_13b.c | 0 | 0 | 1 | 0 | 1 |
| nemici_tipi_15_17.c | 1 | 0 | 0 | 0 | 1 |
| nemici_tipo06a.c | 4 | 0 | 0 | 1 | 5 |
| nemici_tipo07a.c | 5 | 0 | 0 | 0 | 5 |
| nemici_tipo07b.c | 4 | 0 | 0 | 0 | 4 |
| nmi_oggetti.c | 8 | 0 | 0 | 0 | 8 |
| raccolta_13xx.c | 2 | 0 | 0 | 0 | 2 |
| z80_08xx.c | 2 | 0 | 0 | 0 | 2 |
| z80_0xxx.c | 24 | 2 | 13 | 0 | 39 |
| z80_2xxx.c | 8 | 1 | 6 | 0 | 15 |
| z80_38xx.c | 4 | 0 | 0 | 0 | 4 |
| z80_3xxx.c | 11 | 1 | 0 | 0 | 12 |
| z80_4xxx.c | 17 | 0 | 3 | 0 | 20 |
| z80_5xxx.c | 0 | 0 | 1 | 0 | 1 |
| z80_6xxx.c | 0 | 2 | 0 | 0 | 2 |
| z80_7xxx.c | 2 | 0 | 2 | 0 | 4 |
| z80_chiave_a.c | 10 | 0 | 0 | 1 | 11 |
| z80_chiave_b.c | 2 | 0 | 0 | 1 | 3 |
| **totale** | 155 | 6 | 35 | 5 | 201 |

Occorrenze mai eseguite nei percorsi misurati (32), classificate dal codice:
`attivita_12_13.c` 53 (riga di 82DA), `attivita_16.c` 100 (6837) e 136 (RAM),
`attivita_16b.c` 119 (6970), `attivita_25.c` 186 (3D12-3D14 o 3D1D-3D1F), `nemici_tipi_15_17.c`
386 (8089), `nemici_tipo06a.c` 37 40 (9495) 173 175 (4FF9), `nemici_tipo07a.c`
127 (84C8) 421 423 439 441 e `nemici_tipo07b.c` 280 282 336 338 (608C-610B),
`raccolta_13xx.c` 199 (record di 8EA4 via 147F), `z80_4xxx.c` 574 (RAM),
`z80_7xxx.c` 254 260 (doppione di `lista_7b9e`), `z80_chiave_a.c` 77 81
(0D6E) 142 186 224 293 331 e `z80_chiave_b.c` 133 173 (123C-1287).

## Voci

Tabella delle voci (`python3 -I censimento_rom.py --tabella`). «letta da» =
routine Z80 d'origine (il C ha lo stesso nome, `sub_XXXX` o il passo
dell'attività); «letti» = byte letti nei percorsi misurati sui byte della voce;
«origine» = file ROM e posizione (`build_rom.sh`: 4000-7FFF = seconda metà di
`7.3h`, 8000-BFFF = prima metà, F000-FFFF = `8.3jk`). Estensione: *statica* =
base e indice limitato dal codice; *puntatori* = calcolata seguendo le parole
della ROM (8EA4: record fino a 9483; F000: record palette F1E8-F414; F068 e
F138: flussi F415-F996, ognuno chiuso dal primo byte di tipo 3); *zona* =
intervallo di dati fra codice eseguito in `out/copertura.txt`; *misurata* = dai
byte letti, da confermare nel lotto di estrazione; *stimata* = fino alla voce
seguente. Nessuna voce si sovrappone a un'altra e in ognuna i byte di
`main64.bin` coincidono con il file ROM d'origine (controllato da `--verifica`).

| voce | inizio-fine | byte | stato | letta da | formato | estensione | letti | origine |
|---|---|---|---|---|---|---|---|---|
| `tab_parole_08ae` | 08AE-08CD | 32 | letto dalla ROM | 08A2 | 16 parole | zona | 32/32 | 6.3f+08AE |
| `tab_gestori_chiave_0d18` | 0D18-0D37 | 32 | letto dalla ROM | 0CFD | 16 parole (gestori della chiave) | statica | 4/32 | 6.3f+0D18 |
| `tab_chiave_0d6e` | 0D6E-0D8D | 32 | letto dalla ROM | 0D3C | 16 coppie, indice <= 1E (brief 62: 0D7E-0D8D dal banco) | statica | 0/32 | 6.3f+0D6E |
| `tab_chiave_123c` | 123C-1287 | 76 | letto dalla ROM | 0D8E 0DD1 0E1F 0E64 0EDF 0F2E 100E 105D | 8 tabelle di byte (123C 1246 124F 1258 1261 126A 1274 127E) | zona | 3/76 | 6.3f+123C |
| `tab_punti_150c` | 150C-1532 | 39 | letto dalla ROM | 13FC, rst 10 | 13 voci da 3 byte | statica | 18/39 | 6.3f+150C |
| `tab_mappe_blocchi_1557` | 1557-1F16 | 2496 | letto dalla ROM | 3812 | 52 stanze x 48 byte | statica | 192/2496 | 6.3f+1557 |
| `tab_boot_295c` | 295C-297F | 36 | letto dalla ROM | 2833 (boot) | 36 byte copiati | statica | 36/36 | 6.3f+295C |
| `tab_monete_29c7` | 29C7-29CE | 8 | letto dalla ROM | 2980 | monete 29C7, vite 29CB | zona | 2/8 | 6.3f+29C7 |
| `tab_testi_2b29` | 2B29-2BC2 | 154 | letto dalla ROM | 2B08, rst 10 | parole 2B29 + testi 2B61 | zona | 146/154 | 6.3f+2B29 |
| `tab_attract_2d4a` | 2D4A-2D71 | 40 | letto dalla ROM | 2D07 | 8 byte + 32 posizioni (R & 1F) | statica | 24/40 | 6.3f+2D4A |
| `tab_303e` | 303E-3044 | 7 | letto dalla ROM | 3009 | 7 byte copiati | statica | 7/7 | 6.3f+303E |
| `tab_3111` | 3111-3130 | 32 | letto dalla ROM | 30EF | 32 byte | zona | 32/32 | 6.3f+3111 |
| `testo_31e1` | 31E1-31E8 | 8 | letto dalla ROM | 6771 | stringa (attributo, caratteri, FF) | zona | 8/8 | 6.3f+31E1 |
| `tab_3427` | 3427-342C | 6 | letto dalla ROM | 33EB | 6 byte copiati | statica | 6/6 | 6.3f+3427 |
| `tab_bonus_343f` | 343F-349E | 96 | letto dalla ROM | rst 10, 4C89 | parole e BCD | zona | 10/96 | 6.3f+343F |
| `tab_bonus_355d` | 355D-356C | 16 | letto dalla ROM | 354B via 3569 | 4 record da 4 byte (c24a); il quarto sono i primi byte del codice di 3569 (brief 62) | statica | 8/16 | 6.3f+355D |
| `tab_copie_3574` | 3574-358E | 27 | letto dalla ROM | 23F8 | record da 3 byte | zona | 15/27 | 6.3f+3574 |
| `tab_36b3` | 36B3-36BC | 10 | letto dalla ROM | 3678 | 10 byte copiati | statica | 10/10 | 6.3f+36B3 |
| `testi_39c0` | 39C0-3A0D | 78 | letto dalla ROM | 6771 | stringhe | zona | 57/78 | 6.3f+39C0 |
| `tab_quadri_3a0e` | 3A0E-3A1B | 14 | letto dalla ROM | 3861 | 7 parole confrontate | zona | 7/14 | 6.3f+3A0E |
| `tab_corpi_3b11` | 3B11-3B60 | 80 | letto dalla ROM | rst 20 a 3B10 (rst 10; banco 0010/0018) | 40 parole, indirizzi dei corpi (brief 62) | zona | 0/80 | 6.3f+3B11 |
| `testi_bonus_3d0a` | 3D0A-3D1F | 22 | letto dalla ROM | 3BAC (att. 25) | 2 stringhe + 3 byte BCD | statica | 0/22 | 6.3f+3D0A |
| `tab_bonus_3e72` | 3E72-3E8F | 30 | letto dalla ROM | 3569 | 30 byte copiati | misurata | 30/30 | 6.3f+3E72 |
| `testi_3f6f` | 3F6F-3F87 | 25 | letto dalla ROM | 6771 | stringa | misurata | 25/25 | 6.3f+3F6F |
| `tab_bcd_3fe8` | 3FE8-3FF6 | 15 | letto dalla ROM | 3FCE | 5 valori BCD da 3 byte | statica | 9/15 | 6.3f+3FE8 |
| `tab_43b9` | 43B9-43C7 | 15 | letto dalla ROM | 4398 | 5 record da 3 byte | zona | 12/15 | 7.3h+43B9 |
| `tab_attract_43e5` | 43E5-43FC | 24 | letto dalla ROM | 43C8 (att. 1F) | 24 byte copiati | statica | 24/24 | 7.3h+43E5 |
| `tab_attract_43fd` | 43FD-4402 | 6 | letto dalla ROM | 43C8 (att. 1F) | 6 byte copiati | statica | 6/6 | 7.3h+43FD |
| `tab_registri_4403` | 4403-451B | 281 | letto dalla ROM | 452B | coppie (registro, valore) | zona | 178/281 | 7.3h+4403 |
| `tab_record_451c` | 451C-452A | 15 | letto dalla ROM | 452B | 5 record (coppie, destinazione) | zona | 12/15 | 7.3h+451C |
| `tab_porta_47c7` | 47C7-47EC | 38 | letto dalla ROM | 47B0 (4767) via 3569 | 4 record da 10 byte, 8 copiati; il quarto legge il codice di 47E5 (brief 62) | statica | 0/38 | 7.3h+47C7 |
| `tab_record_488d` | 488D-48AE | 34 | letto dalla ROM | rst 10 (nemici) | parole per indice | misurata | 26/34 | 7.3h+488D |
| `tab_oggetti_48af` | 48AF-48D0 | 34 | letto dalla ROM | rst 10 (nemici) | parole per indice | misurata | 28/34 | 7.3h+48AF |
| `tab_4ff9` | 4FF9-5004 | 12 | letto dalla ROM | 4FEF | 6 parole | statica | 0/12 | 7.3h+4FF9 |
| `tab_nemici_608c` | 608C-610B | 128 | letto dalla ROM | 5F4B 5F82 5FDC 6046 | 4 tabelle da 16 parole | statica | 0/128 | 7.3h+608C |
| `tab_6653` | 6653-6654 | 2 | letto dalla ROM | 3569 | 2 byte copiati | misurata | 2/2 | 7.3h+6653 |
| `testo_6766` | 6766-6770 | 11 | letto dalla ROM | 6771 | stringa | statica | 11/11 | 7.3h+6766 |
| `tab_segni_6837` | 6837-6841 | 11 | letto dalla ROM | 6785 (att. 16) | 11 caratteri | statica | 0/11 | 7.3h+6837 |
| `tab_6970` | 6970-6975 | 6 | letto dalla ROM | 6842 (att. 16) | 6 byte copiati | statica | 0/6 | 7.3h+6970 |
| `testi_6a9e` | 6A9E-6ADD | 64 | letto dalla ROM | 6771 | stringhe | misurata | 64/64 | 7.3h+6A9E |
| `tab_6bc5` | 6BC5-6BC6 | 2 | letto dalla ROM | rst 10 (6BA8) | parole | misurata | 2/2 | 7.3h+6BC5 |
| `tab_velocita_6f3d` | 6F3D-6F44 | 8 | letto dalla ROM | rst 10 (nemici 08-14) | parole | misurata | 4/8 | 7.3h+6F3D |
| `tab_8089` | 8089-808C | 4 | estratto | 8042 (copia a mano tab8089[4]) | 4 byte | statica | 0/4 | 7.3h+0089 |
| `tab_70ee` | 70EE-70FC | 15 | letto dalla ROM | 6ECA | 15 byte | misurata | 15/15 | 7.3h+70EE |
| `tab_direzioni_7196` | 7196-719D | 8 | letto dalla ROM | rst 10 (nemici 10-13) | 4 parole | statica | 2/8 | 7.3h+7196 |
| `tab_velocita_7468` | 7468-746B | 4 | letto dalla ROM | rst 10 (comparsa) | coppie di byte | misurata | 4/4 | 7.3h+7468 |
| `tab_urto_754c` | 754C-7567 | 28 | letto dalla ROM | rst 10 (comparsa) | parole urto X (754C) e Y (755C) | misurata | 16/28 | 7.3h+754C |
| `tab_demo_765e` | 765E-767B | 30 | letto dalla ROM | 760B 2551 | record da 3 byte | misurata | 15/30 | 7.3h+765E |
| `copione_demo_767c` | 767C-770E | 147 | letto dalla ROM | 01C5 (NMI) | coppie (chiave, codice) | misurata | 147/147 | 7.3h+767C |
| `testi_79a4` | 79A4-79DE | 59 | letto dalla ROM | 6771 | stringhe | misurata | 59/59 | 7.3h+79A4 |
| `testi_79fa` | 79FA-7A0A | 17 | letto dalla ROM | 6771 | stringhe | misurata | 17/17 | 7.3h+79FA |
| `testi_7a3a` | 7A3A-7A55 | 28 | letto dalla ROM | 6771 | stringhe | misurata | 28/28 | 7.3h+7A3A |
| `tab_7aef` | 7AEF-7AF2 | 4 | letto dalla ROM | 7A56 | 4 byte copiati | statica | 4/4 | 7.3h+7AEF |
| `testi_7af3` | 7AF3-7B3A | 72 | letto dalla ROM | 6771 | stringhe | misurata | 72/72 | 7.3h+7AF3 |
| `tab_coppie_7bb1` | 7BB1-7BF5 | 69 | letto dalla ROM | 7B9E | liste di coppie (B, C) chiuse da FF | misurata | 69/69 | 7.3h+7BB1 |
| `testi_7c58` | 7C58-7C83 | 44 | letto dalla ROM | 6771 | stringhe | zona | 44/44 | 7.3h+7C58 |
| `tab_esiti_7e3a` | 7E3A-7E45 | 12 | letto dalla ROM | rst 10 (7E..) | parole | misurata | 12/12 | 7.3h+7E3A |
| `testi_7e46` | 7E46-7EC9 | 132 | letto dalla ROM | 6771 | stringhe | zona | 108/132 | 7.3h+7E46 |
| `tab_bonus_80b3` | 80B3-80D2 | 32 | letto dalla ROM | 3569 | byte copiati | misurata | 32/32 | 7.3h+00B3 |
| `tab_palette_80d3` | 80D3-82D2 | 512 | letto dalla ROM | 36C3 3569 | 512 byte di palette | statica | 512/512 | 7.3h+00D3 |
| `tab_82d3` | 82D3-82D9 | 7 | letto dalla ROM | 3861 | 7 byte | misurata | 7/7 | 7.3h+02D3 |
| `tab_casuale_82da` | 82DA-83F9 | 288 | letto dalla ROM | 4302 426A | 18 righe da 16 byte | statica | 7/288 | 7.3h+02DA |
| `tab_sequenze_83fa` | 83FA-8417 | 30 | letto dalla ROM | rst 10 (4DEE) | 15 parole | statica | 30/30 | 7.3h+03FA |
| `seq_8418` | 8418-84C7 | 176 | letto dalla ROM | 4DEE | sequenze a righe | stimata | 176/176 | 7.3h+0418 |
| `tab_percorsi_84c8` | 84C8-8577 | 176 | letto dalla ROM | 5D29 | parole + record | stimata | 0/176 | 7.3h+04C8 |
| `tab_tile_8578` | 8578-858D | 22 | letto dalla ROM | 3A2E | 11 coppie (codice, colore) | statica | 12/22 | 7.3h+0578 |
| `tab_tile_858e` | 858E-85EB | 94 | letto dalla ROM | 3A2E | 47 coppie (valori 08-36) | statica | 16/94 | 7.3h+058E |
| `tab_quadri_85ec` | 85EC-85F8 | 13 | letto dalla ROM | 3861 | 13 soglie | misurata | 13/13 | 7.3h+05EC |
| `tab_85f9` | 85F9-8660 | 104 | letto dalla ROM | rst 10 (3709) | parole | stimata | 8/104 | 7.3h+05F9 |
| `tab_3709_8661` | 8661-89CC | 876 | letto dalla ROM | 3709 | 52 record per stanza puntati da 85F9, chiusi da FF (collaudo 2026-11-06) | puntatori | 50/876 | 7.3h+0661 |
| `tab_89cd` | 89CD-8A00 | 52 | letto dalla ROM | 2368 (2E9C) | 52 byte, indice di 8A01 per quadro (brief 62) | statica | 4/52 | 7.3h+09CD |
| `tab_8a01` | 8A01-8B2C | 300 | letto dalla ROM | 4E87 | 25 record da 12 byte, indici 00-18 di 89CD (brief 62) | statica | 22/300 | 7.3h+0A01 |
| `tab_parole_8b2d` | 8B2D-8D10 | 484 | letto dalla ROM | rst 10, 2E9C | 8 parole + 52 record da 9 da 8B3D (brief 62) | statica | 14/484 | 7.3h+0B2D |
| `tab_8d11` | 8D11-8D58 | 72 | letto dalla ROM | 2E9C | 36 parole, liste di 8D59 (brief 62) | statica | 4/72 | 7.3h+0D11 |
| `tab_8d59` | 8D59-8DD3 | 123 | letto dalla ROM | 470A 46B4 | liste chiuse da FF + passo indietro (brief 62) | puntatori | 3/123 | 7.3h+0D59 |
| `tab_avanzamento_8dd4` | 8DD4-8EA3 | 208 | letto dalla ROM | 34FA | 52 record da 4 byte | statica | 16/208 | 7.3h+0DD4 |
| `tab_stanze_8ea4` | 8EA4-9483 | 1504 | letto dalla ROM | 358F 147F | 52 parole + record (10 byte + coppie fino a FF) | puntatori | 78/1504 | 7.3h+0EA4 |
| `tab_soglie_9484` | 9484-9494 | 17 | letto dalla ROM | 4722 | 17 soglie | misurata | 17/17 | 7.3h+1484 |
| `tab_oggetti_9495` | 9495-9554 | 192 | letto dalla ROM | 027C 0602 4592 4B20 2474 24DF | 24 record da 8 byte | stimata | 103/192 | 7.3h+1495 |
| `tab_anim_9555` | 9555-969C | 328 | letto dalla ROM | 07DE | 164 parole | statica | 50/328 | 7.3h+1555 |
| `tab_anim_rec_969d` | 969D-9DCC | 1840 | letto dalla ROM | 07DE | record da 3 byte e parole | stimata | 575/1840 | 7.3h+169D |
| `tab_anim2_9dcd` | 9DCD-A745 | 2425 | letto dalla ROM | 07DE | record | stimata | 990/2425 | 7.3h+1DCD |
| `tab_comparse_a746` | A746-A88D | 328 | letto dalla ROM | 0602 | 164 parole | statica | 50/328 | 7.3h+2746 |
| `tab_comparse_rec_a88e` | A88E-AEE1 | 1620 | letto dalla ROM | 0602 | record da 3 byte + parole | stimata | 550/1620 | 7.3h+288E |
| `tab_campi_aee2` | AEE2-B125 | 580 | letto dalla ROM | 0602 | campi copiati | misurata | 221/580 | 7.3h+2EE2 |
| `tab_palette_f000` | F000-F067 | 104 | letto dalla ROM | 33BA (via ram[]) | 52 parole | statica | 10/104 | 8.3jk+0000 |
| `tab_fondali_f068` | F068-F137 | 208 | letto dalla ROM | 3231 (via ram[]) | 52 voci da 4 byte | statica | 20/208 | 8.3jk+0068 |
| `tab_sovrapposizioni_f138` | F138-F1E7 | 176 | letto dalla ROM | 325E (via ram[]) | 44 voci da 4 byte | puntatori | 24/176 | 8.3jk+0138 |
| `rec_palette_f1e8` | F1E8-F414 | 557 | letto dalla ROM | 33BA (via ram[]) | 13 record (h, 2*(h&7F) byte) | puntatori | 229/557 | 8.3jk+01E8 |
| `flussi_fondale_f415` | F415-F996 | 1410 | letto dalla ROM | 326C (via ram[]) | flussi chiusi da tipo 3 | puntatori | 383/1410 | 8.3jk+0415 |
| `aud_gestori_03ce` | 03CE-03ED | 32 | letto dalla ROM | controlla_tabella (audio_interprete.c) | 16 parole, già copiate a mano in indirizzi[16] | statica | 32/32 | 1.3jk+03CE |
| `aud_dati_05c6` | 05C6-3FFF | 14906 | estratto | interprete, ossatura (estrai_dati.py) | tabelle e spartiti | statica | 6148/14906 | 1.3jk+05C6 |

Già estratto, fuori dalla tabella perché non è una lettura di `rom[]`:

- grafica: `video_dati_{car,fon,spr}_{a,b,c,d}.c` da `estrai_grafica.py`
  (otto ROM grafiche, 196608 byte), letta solo dal C;
- `rst20_tables.h` (21 tabelle) e `rst38_tables.h` (21): 42 tabelle di
  destinazioni, ognuna uguale parola per parola alla ROM a partire dal byte
  dopo `rst` (controllo in `--verifica`); i byte di quelle zone non sono mai
  letti dal C. Due sono falsi positivi in zona dati già noti in `DATI.md`:
  `rst20_1833` cade dentro `tab_mappe_blocchi_1557` e `rst20_8C93` nella zona
  8693-89CC mai letta; non danno fastidio ma non vanno presi come tabelle;
- copie a mano nel C: `tab8089[4]` (`nemici_tipi_15_17.c`, voce `tab_8089`),
  `indirizzi[16]` (`audio_interprete.c`, voce `aud_gestori_03ce`), la tabella di
  funzioni `gestori_0d18` (stessi gestori delle parole di 0D18, che però si
  rileggono per i registri HL e DE);
- `e603.h` **non** è ROM: è il valore della protezione restituito da MAME per
  PC (una voce, 015E → 00); resta com'è.

## Via d'estrazione proposta

Un generatore nuovo `decomp/estrai_rom.py`, sul modello di `estrai_grafica.py`:

- legge i file ROM, non `main64.bin` (`6.3f`, `7.3h` nelle due metà come
  `build_rom.sh`, i primi 4096 byte di `8.3jk`, `1.3jk` per il driver), con la
  stessa ricerca delle cartelle;
- l'elenco delle voci (nome, spazio, inizio, fine) è quello di `VOCI` in
  `censimento_rom.py`: il generatore lo importa o ne tiene una copia
  controllata (stesso elenco, altrimenti uscita 2);
- scrive `harness/rom_dati_*.c` (righe da 24 byte, intestazione «Generato da
  decomp/estrai_rom.py … non modificare» e rimando a `NOTICE.md`) e
  `harness/rom_dati.h` con `extern const uint8_t <voce>[N];` e un elenco
  `rom_voci[] = { inizio, lunghezza, puntatore }`;
- **round-trip byte per byte**: prima di scrivere rilegge i valori dal testo
  prodotto e li confronta con la ROM; se differiscono esce con 2; riscrive un
  file solo se il contenuto cambia (rieseguirlo non tocca nulla, come la
  verifica 9 di questo brief);
- un `harness/rom_immagine.c` (come `video_immagine.c` e `audio_immagine.c`)
  ricompone l'immagine dalle voci e la confronta con `out/main64.bin` voce per
  voce: secondo controllo di round-trip, nel `Makefile` accanto agli altri;
- a runtime `carica_rom()` è sostituita da una `rom_componi()` che riempie
  `rom[0x10000]` dalle voci (come `aud_usa_dati_estratti(1)` fa per l'audio),
  così `leggi()` e le 196 chiamate non cambiano. Fuori dalle voci `rom[]` vale
  una sentinella: un byte letto lì (con il gancio di prova, non in partita)
  stampa «lettura ROM fuori voce» con l'indirizzo, così una tabella sfuggita al
  censimento si vede alla prima prova invece di dare dati sbagliati.

Per ogni voce «letto dalla ROM» (`--estrazione`): file generato, simbolo,
stima del sorgente con il rapporto misurato di `video_dati_*.c` (16384 byte di
ROM in 689 righe e 85587 byte di sorgente per file: righe = ⌈n/24⌉ + 2 per
voce, più 6 di intestazione per file; byte = n × 85587 / 16384) e controllo.

| voce | byte | file generato | simbolo | righe C | byte C | controllo |
|---|---|---|---|---|---|---|
| `tab_parole_08ae` | 32 | rom_dati_varie.c | `const uint8_t tab_parole_08ae[32]` | 4 | 167 | round-trip byte per byte, uscita 2 |
| `tab_gestori_chiave_0d18` | 32 | rom_dati_varie.c | `const uint8_t tab_gestori_chiave_0d18[32]` | 4 | 167 | round-trip byte per byte, uscita 2 |
| `tab_chiave_0d6e` | 16 | rom_dati_varie.c | `const uint8_t tab_chiave_0d6e[16]` | 3 | 84 | round-trip byte per byte, uscita 2 |
| `tab_chiave_123c` | 76 | rom_dati_varie.c | `const uint8_t tab_chiave_123c[76]` | 6 | 397 | round-trip byte per byte, uscita 2 |
| `tab_punti_150c` | 39 | rom_dati_varie.c | `const uint8_t tab_punti_150c[39]` | 4 | 204 | round-trip byte per byte, uscita 2 |
| `tab_mappe_blocchi_1557` | 2496 | rom_dati_varie.c | `const uint8_t tab_mappe_blocchi_1557[2496]` | 106 | 13039 | round-trip byte per byte, uscita 2 |
| `tab_boot_295c` | 36 | rom_dati_varie.c | `const uint8_t tab_boot_295c[36]` | 4 | 188 | round-trip byte per byte, uscita 2 |
| `tab_monete_29c7` | 8 | rom_dati_varie.c | `const uint8_t tab_monete_29c7[8]` | 3 | 42 | round-trip byte per byte, uscita 2 |
| `tab_testi_2b29` | 154 | rom_dati_varie.c | `const uint8_t tab_testi_2b29[154]` | 9 | 804 | round-trip byte per byte, uscita 2 |
| `tab_attract_2d4a` | 40 | rom_dati_varie.c | `const uint8_t tab_attract_2d4a[40]` | 4 | 209 | round-trip byte per byte, uscita 2 |
| `tab_303e` | 7 | rom_dati_varie.c | `const uint8_t tab_303e[7]` | 3 | 37 | round-trip byte per byte, uscita 2 |
| `tab_3111` | 32 | rom_dati_varie.c | `const uint8_t tab_3111[32]` | 4 | 167 | round-trip byte per byte, uscita 2 |
| `testo_31e1` | 8 | rom_dati_testi.c | `const uint8_t testo_31e1[8]` | 3 | 42 | round-trip byte per byte, uscita 2 |
| `tab_3427` | 6 | rom_dati_varie.c | `const uint8_t tab_3427[6]` | 3 | 31 | round-trip byte per byte, uscita 2 |
| `tab_bonus_343f` | 96 | rom_dati_varie.c | `const uint8_t tab_bonus_343f[96]` | 6 | 501 | round-trip byte per byte, uscita 2 |
| `tab_bonus_355d` | 12 | rom_dati_varie.c | `const uint8_t tab_bonus_355d[12]` | 3 | 63 | round-trip byte per byte, uscita 2 |
| `tab_copie_3574` | 27 | rom_dati_varie.c | `const uint8_t tab_copie_3574[27]` | 4 | 141 | round-trip byte per byte, uscita 2 |
| `tab_36b3` | 10 | rom_dati_varie.c | `const uint8_t tab_36b3[10]` | 3 | 52 | round-trip byte per byte, uscita 2 |
| `testi_39c0` | 78 | rom_dati_testi.c | `const uint8_t testi_39c0[78]` | 6 | 407 | round-trip byte per byte, uscita 2 |
| `tab_quadri_3a0e` | 14 | rom_dati_varie.c | `const uint8_t tab_quadri_3a0e[14]` | 3 | 73 | round-trip byte per byte, uscita 2 |
| `testi_bonus_3d0a` | 22 | rom_dati_testi.c | `const uint8_t testi_bonus_3d0a[22]` | 3 | 115 | round-trip byte per byte, uscita 2 |
| `tab_bonus_3e72` | 30 | rom_dati_varie.c | `const uint8_t tab_bonus_3e72[30]` | 4 | 157 | round-trip byte per byte, uscita 2 |
| `testi_3f6f` | 25 | rom_dati_testi.c | `const uint8_t testi_3f6f[25]` | 4 | 131 | round-trip byte per byte, uscita 2 |
| `tab_bcd_3fe8` | 15 | rom_dati_varie.c | `const uint8_t tab_bcd_3fe8[15]` | 3 | 78 | round-trip byte per byte, uscita 2 |
| `tab_43b9` | 15 | rom_dati_varie.c | `const uint8_t tab_43b9[15]` | 3 | 78 | round-trip byte per byte, uscita 2 |
| `tab_attract_43e5` | 24 | rom_dati_varie.c | `const uint8_t tab_attract_43e5[24]` | 3 | 125 | round-trip byte per byte, uscita 2 |
| `tab_attract_43fd` | 6 | rom_dati_varie.c | `const uint8_t tab_attract_43fd[6]` | 3 | 31 | round-trip byte per byte, uscita 2 |
| `tab_registri_4403` | 281 | rom_dati_varie.c | `const uint8_t tab_registri_4403[281]` | 14 | 1468 | round-trip byte per byte, uscita 2 |
| `tab_record_451c` | 15 | rom_dati_varie.c | `const uint8_t tab_record_451c[15]` | 3 | 78 | round-trip byte per byte, uscita 2 |
| `tab_record_488d` | 34 | rom_dati_varie.c | `const uint8_t tab_record_488d[34]` | 4 | 178 | round-trip byte per byte, uscita 2 |
| `tab_oggetti_48af` | 34 | rom_dati_varie.c | `const uint8_t tab_oggetti_48af[34]` | 4 | 178 | round-trip byte per byte, uscita 2 |
| `tab_4ff9` | 12 | rom_dati_varie.c | `const uint8_t tab_4ff9[12]` | 3 | 63 | round-trip byte per byte, uscita 2 |
| `tab_nemici_608c` | 128 | rom_dati_varie.c | `const uint8_t tab_nemici_608c[128]` | 8 | 669 | round-trip byte per byte, uscita 2 |
| `tab_6653` | 2 | rom_dati_varie.c | `const uint8_t tab_6653[2]` | 3 | 10 | round-trip byte per byte, uscita 2 |
| `testo_6766` | 11 | rom_dati_testi.c | `const uint8_t testo_6766[11]` | 3 | 57 | round-trip byte per byte, uscita 2 |
| `tab_segni_6837` | 11 | rom_dati_varie.c | `const uint8_t tab_segni_6837[11]` | 3 | 57 | round-trip byte per byte, uscita 2 |
| `tab_6970` | 6 | rom_dati_varie.c | `const uint8_t tab_6970[6]` | 3 | 31 | round-trip byte per byte, uscita 2 |
| `testi_6a9e` | 64 | rom_dati_testi.c | `const uint8_t testi_6a9e[64]` | 5 | 334 | round-trip byte per byte, uscita 2 |
| `tab_6bc5` | 2 | rom_dati_varie.c | `const uint8_t tab_6bc5[2]` | 3 | 10 | round-trip byte per byte, uscita 2 |
| `tab_velocita_6f3d` | 8 | rom_dati_varie.c | `const uint8_t tab_velocita_6f3d[8]` | 3 | 42 | round-trip byte per byte, uscita 2 |
| `tab_70ee` | 15 | rom_dati_varie.c | `const uint8_t tab_70ee[15]` | 3 | 78 | round-trip byte per byte, uscita 2 |
| `tab_direzioni_7196` | 8 | rom_dati_varie.c | `const uint8_t tab_direzioni_7196[8]` | 3 | 42 | round-trip byte per byte, uscita 2 |
| `tab_velocita_7468` | 4 | rom_dati_varie.c | `const uint8_t tab_velocita_7468[4]` | 3 | 21 | round-trip byte per byte, uscita 2 |
| `tab_urto_754c` | 28 | rom_dati_varie.c | `const uint8_t tab_urto_754c[28]` | 4 | 146 | round-trip byte per byte, uscita 2 |
| `tab_demo_765e` | 30 | rom_dati_varie.c | `const uint8_t tab_demo_765e[30]` | 4 | 157 | round-trip byte per byte, uscita 2 |
| `copione_demo_767c` | 147 | rom_dati_varie.c | `const uint8_t copione_demo_767c[147]` | 9 | 768 | round-trip byte per byte, uscita 2 |
| `testi_79a4` | 59 | rom_dati_testi.c | `const uint8_t testi_79a4[59]` | 5 | 308 | round-trip byte per byte, uscita 2 |
| `testi_79fa` | 17 | rom_dati_testi.c | `const uint8_t testi_79fa[17]` | 3 | 89 | round-trip byte per byte, uscita 2 |
| `testi_7a3a` | 28 | rom_dati_testi.c | `const uint8_t testi_7a3a[28]` | 4 | 146 | round-trip byte per byte, uscita 2 |
| `tab_7aef` | 4 | rom_dati_varie.c | `const uint8_t tab_7aef[4]` | 3 | 21 | round-trip byte per byte, uscita 2 |
| `testi_7af3` | 72 | rom_dati_testi.c | `const uint8_t testi_7af3[72]` | 5 | 376 | round-trip byte per byte, uscita 2 |
| `tab_coppie_7bb1` | 69 | rom_dati_varie.c | `const uint8_t tab_coppie_7bb1[69]` | 5 | 360 | round-trip byte per byte, uscita 2 |
| `testi_7c58` | 44 | rom_dati_testi.c | `const uint8_t testi_7c58[44]` | 4 | 230 | round-trip byte per byte, uscita 2 |
| `tab_esiti_7e3a` | 12 | rom_dati_varie.c | `const uint8_t tab_esiti_7e3a[12]` | 3 | 63 | round-trip byte per byte, uscita 2 |
| `testi_7e46` | 132 | rom_dati_testi.c | `const uint8_t testi_7e46[132]` | 8 | 690 | round-trip byte per byte, uscita 2 |
| `tab_bonus_80b3` | 32 | rom_dati_stanze.c | `const uint8_t tab_bonus_80b3[32]` | 4 | 167 | round-trip byte per byte, uscita 2 |
| `tab_palette_80d3` | 512 | rom_dati_stanze.c | `const uint8_t tab_palette_80d3[512]` | 24 | 2675 | round-trip byte per byte, uscita 2 |
| `tab_82d3` | 7 | rom_dati_stanze.c | `const uint8_t tab_82d3[7]` | 3 | 37 | round-trip byte per byte, uscita 2 |
| `tab_casuale_82da` | 288 | rom_dati_stanze.c | `const uint8_t tab_casuale_82da[288]` | 14 | 1504 | round-trip byte per byte, uscita 2 |
| `tab_sequenze_83fa` | 30 | rom_dati_stanze.c | `const uint8_t tab_sequenze_83fa[30]` | 4 | 157 | round-trip byte per byte, uscita 2 |
| `seq_8418` | 176 | rom_dati_stanze.c | `const uint8_t seq_8418[176]` | 10 | 919 | round-trip byte per byte, uscita 2 |
| `tab_percorsi_84c8` | 176 | rom_dati_stanze.c | `const uint8_t tab_percorsi_84c8[176]` | 10 | 919 | round-trip byte per byte, uscita 2 |
| `tab_tile_8578` | 22 | rom_dati_stanze.c | `const uint8_t tab_tile_8578[22]` | 3 | 115 | round-trip byte per byte, uscita 2 |
| `tab_tile_858e` | 94 | rom_dati_stanze.c | `const uint8_t tab_tile_858e[94]` | 6 | 491 | round-trip byte per byte, uscita 2 |
| `tab_quadri_85ec` | 13 | rom_dati_stanze.c | `const uint8_t tab_quadri_85ec[13]` | 3 | 68 | round-trip byte per byte, uscita 2 |
| `tab_85f9` | 104 | rom_dati_stanze.c | `const uint8_t tab_85f9[104]` | 7 | 543 | round-trip byte per byte, uscita 2 |
| `tab_3709_8661` | 50 | rom_dati_stanze.c | `const uint8_t tab_3709_8661[50]` | 5 | 261 | round-trip byte per byte, uscita 2 |
| `tab_89cd` | 4 | rom_dati_stanze.c | `const uint8_t tab_89cd[4]` | 3 | 21 | round-trip byte per byte, uscita 2 |
| `tab_8a01` | 48 | rom_dati_stanze.c | `const uint8_t tab_8a01[48]` | 4 | 251 | round-trip byte per byte, uscita 2 |
| `tab_parole_8b2d` | 46 | rom_dati_stanze.c | `const uint8_t tab_parole_8b2d[46]` | 4 | 240 | round-trip byte per byte, uscita 2 |
| `tab_8d11` | 4 | rom_dati_stanze.c | `const uint8_t tab_8d11[4]` | 3 | 21 | round-trip byte per byte, uscita 2 |
| `tab_8d59` | 3 | rom_dati_stanze.c | `const uint8_t tab_8d59[3]` | 3 | 16 | round-trip byte per byte, uscita 2 |
| `tab_avanzamento_8dd4` | 208 | rom_dati_stanze.c | `const uint8_t tab_avanzamento_8dd4[208]` | 11 | 1087 | round-trip byte per byte, uscita 2 |
| `tab_stanze_8ea4` | 1504 | rom_dati_stanze.c | `const uint8_t tab_stanze_8ea4[1504]` | 65 | 7857 | round-trip byte per byte, uscita 2 |
| `tab_soglie_9484` | 17 | rom_dati_oggetti.c | `const uint8_t tab_soglie_9484[17]` | 3 | 89 | round-trip byte per byte, uscita 2 |
| `tab_oggetti_9495` | 192 | rom_dati_oggetti.c | `const uint8_t tab_oggetti_9495[192]` | 10 | 1003 | round-trip byte per byte, uscita 2 |
| `tab_anim_9555` | 328 | rom_dati_oggetti.c | `const uint8_t tab_anim_9555[328]` | 16 | 1713 | round-trip byte per byte, uscita 2 |
| `tab_anim_rec_969d` | 1840 | rom_dati_oggetti.c | `const uint8_t tab_anim_rec_969d[1840]` | 79 | 9612 | round-trip byte per byte, uscita 2 |
| `tab_anim2_9dcd` | 2425 | rom_dati_oggetti.c | `const uint8_t tab_anim2_9dcd[2425]` | 104 | 12668 | round-trip byte per byte, uscita 2 |
| `tab_comparse_a746` | 328 | rom_dati_oggetti.c | `const uint8_t tab_comparse_a746[328]` | 16 | 1713 | round-trip byte per byte, uscita 2 |
| `tab_comparse_rec_a88e` | 1620 | rom_dati_oggetti.c | `const uint8_t tab_comparse_rec_a88e[1620]` | 70 | 8463 | round-trip byte per byte, uscita 2 |
| `tab_campi_aee2` | 580 | rom_dati_oggetti.c | `const uint8_t tab_campi_aee2[580]` | 27 | 3030 | round-trip byte per byte, uscita 2 |
| `tab_palette_f000` | 104 | rom_dati_fondali.c | `const uint8_t tab_palette_f000[104]` | 7 | 543 | round-trip byte per byte, uscita 2 |
| `tab_fondali_f068` | 208 | rom_dati_fondali.c | `const uint8_t tab_fondali_f068[208]` | 11 | 1087 | round-trip byte per byte, uscita 2 |
| `tab_sovrapposizioni_f138` | 176 | rom_dati_fondali.c | `const uint8_t tab_sovrapposizioni_f138[176]` | 10 | 919 | round-trip byte per byte, uscita 2 |
| `rec_palette_f1e8` | 557 | rom_dati_fondali.c | `const uint8_t rec_palette_f1e8[557]` | 26 | 2910 | round-trip byte per byte, uscita 2 |
| `flussi_fondale_f415` | 1410 | rom_dati_fondali.c | `const uint8_t flussi_fondale_f415[1410]` | 61 | 7366 | round-trip byte per byte, uscita 2 |
| `aud_gestori_03ce` | 32 | audio_driver.c | `const uint8_t aud_gestori_03ce[32]` | 4 | 167 | round-trip byte per byte, uscita 2 |

| file generato | voci | byte ROM | righe C stimate | byte C stimati |
|---|---|---|---|---|
| audio_driver.c | 1 | 32 | 10 | 167 |
| rom_dati_fondali.c | 5 | 2455 | 121 | 12825 |
| rom_dati_oggetti.c | 8 | 7330 | 331 | 38291 |
| rom_dati_stanze.c | 19 | 3321 | 192 | 17349 |
| rom_dati_testi.c | 12 | 560 | 59 | 2925 |
| rom_dati_varie.c | 43 | 4073 | 286 | 21275 |

Immagine di ripiego (decisione 4: solo per le zone non classificate):
`rom_ripiego.c` con gli intervalli «dati» di `out/copertura.txt` fuori voce
che il lotto non riesce a dare a una voce. Limite superiore misurato: 5568
byte in 80A5-BFFF (di cui 4115 a 00/FF) più, in 0000-80A4, una parte dei
20565 byte non eseguiti in MAME e fuori voce, che però sono quasi tutti codice
tradotto (nemici, attività 16/25/26, chiave) e non vanno estratti; il lotto
decide intervallo per intervallo con il disassemblato, la sentinella sopra fa
da rete.

## Zone di `main64.bin` mai lette

`python3 -I censimento_rom.py --zone`:

| zona | byte | mai letti | natura |
|---|---|---|---|
| 0000-BFFF, codice eseguito in MAME (`out/copertura.txt`) | 7815 | 7815 | codice tradotto in C: non serve più come byte |
| 0000-80A4, non eseguito e fuori voce | 20565 | 20565 | quasi tutto codice tradotto mai eseguito nelle tracce MAME (p.es. 5005-5506, 566E-5C31, 610C-6652); più dati di percorsi non misurati |
| 0000-80A4, voci | 4637 | 4637 - letti | tabelle inline, vedi tabella |
| 80A5-BFFF, voci | 10651 | 10651 - letti | dati |
| 80A5-BFFF, fuori voce | 5568 | 5568 | dati mai letti: B126-BFFF (3802, coda della ROM), 8693-89CC (826), 8B5B-8D10 (438), 8A31-8B2C (252), 8D5C-8DD3 (120), 8D15-8D58 (68) |
| C000-EFFF | 12288 | 12288 | zeri di `build_rom.sh`: `leggi()` serve tutto ≥ C000 da `ram[]`, `rom[]` non vi è mai indicizzato |
| F000-F996, voci | 2455 | 1789 | tabelle e flussi dei fondali e delle palette, letti solo in `ram[]` dopo la copia |
| F997-FFFF | 1641 | 1641 | riempimento 00/FF (un solo byte diverso) |

Voci con 0 byte letti (codice mai eseguito nei percorsi misurati):
`tab_chiave_0d6e`, `testi_bonus_3d0a`, `tab_4ff9`, `tab_nemici_608c`,
`tab_segni_6837`, `tab_6970`, `tab_percorsi_84c8`, e `tab_8089` (servita dalla
copia a mano).

Zone lette ma non dichiarate in `DATI.md` (che documenta F000, F068, F138,
1557, 8EA4, 8578, 858E, 8DD4 e i punti a 150C): tutte le altre voci, cioè
`tab_parole_08ae`, le tre tabelle della chiave, `tab_boot_295c`,
`tab_monete_29c7`, `tab_testi_2b29`, `tab_attract_2d4a`, `tab_303e`,
`tab_3111`, `tab_3427`, `tab_bonus_343f`, `tab_bonus_355d`, `tab_copie_3574`,
`tab_36b3`, `tab_quadri_3a0e`, `tab_bonus_3e72`, `tab_bcd_3fe8`, `tab_43b9`,
le due `tab_attract_43e5/43fd`, `tab_registri_4403`, `tab_record_451c`,
`tab_record_488d`, `tab_oggetti_48af`, `tab_4ff9`, `tab_nemici_608c`,
`tab_6653`, `tab_segni_6837`, `tab_6970`, `tab_6bc5`, `tab_velocita_6f3d`,
`tab_70ee`, `tab_direzioni_7196`, `tab_velocita_7468`, `tab_urto_754c`,
`tab_demo_765e`, `copione_demo_767c`, `tab_7aef`, `tab_coppie_7bb1`,
`tab_esiti_7e3a`, tutte le `testi_*`, `tab_bonus_80b3`, `tab_palette_80d3`,
`tab_82d3`, `tab_casuale_82da`, `tab_sequenze_83fa`, `seq_8418`,
`tab_percorsi_84c8`, `tab_quadri_85ec`, `tab_85f9`, `tab_3709_8661`,
`tab_89cd`, `tab_8a01`, `tab_parole_8b2d`, `tab_8d11`, `tab_8d59`,
`tab_soglie_9484`, `tab_oggetti_9495`, le tre di 07DE (9555, 969D, 9DCD) e le
tre di 0602 (A746, A88E, AEE2). Sono documentate dai commenti del C che le
legge; `DATI.md` va aggiornato nel lotto di estrazione, non qui.

## Audio (`1.3jk`)

- 05C6-3FFF (14906 byte): **estratto** da `estrai_dati.py`; `scheda_audio.c`
  fa `aud_carica_rom(); aud_usa_dati_estratti(1);`, quindi ogni lettura della
  regione passa già dalle tabelle (6148 byte letti nei percorsi misurati).
- 0000-05C5 (1478 byte): letture misurate solo a **03CE-03ED** (32 byte, la
  tabella dei 16 gestori F0-FF), tutte da `leggi16()` di `audio_interprete.c:37`
  dentro `controlla_tabella()`, che confronta una volta la ROM con la copia a
  mano `indirizzi[16]`: è un **dato** usato solo come controllo. Il resto
  (0000-03CD e 03EE-05C5, 1446 byte) è **codice** del driver tradotto in C
  (`audio_ossatura.c`, `audio_interprete.c`, `audio_registri.c`): 0 letture.
  Le due righe mai eseguite (`audio_interprete.c` 124 e 126, forme d'onda)
  leggono puntatori di sequenza, quindi la regione 05C6-3FFF.
- Proposta (decisione 5): blob C generato `harness/audio_driver.c` con
  `const uint8_t aud_driver[0x05C6]` (1478 byte, ~68 righe, ~7720 byte di
  sorgente col rapporto misurato), prodotto da `estrai_rom.py` o da
  un'estensione di `estrai_dati.py`, con **round-trip byte per byte** e uscita
  2; `aud_carica_rom()` diventa la copia di `aud_driver` in `aud_rom[0..05C5]`
  seguita da `aud_dati_serializza()`. Con questo `1.3jk` sparisce. Variante
  più piccola, se l'utente la preferisse: estrarre solo i 32 byte di 03CE (il
  resto non è mai letto).

## `8.3jk`

32768 byte; `build_rom.sh` ne prende i primi 4096 (F000-FFFF). Oltre 0x1000
il file vale tutto FF (sette blocchi da 4096 identici): nessun byte è usato né
caricato. Dei 4096 caricati servono F000-F996 (2455 byte, cinque voci);
F997-FFFF è riempimento.

## Scoperte da portare al lotto di estrazione

- **Ciclo `res 6` di 027C** (`nmi_oggetti.c:236`, morte di Dana): passa sui 18
  slot da c4ce con passo `D:30` e, con D ≠ 0, l'indirizzo esce da c000-cfff e
  arriva nella copia di F000-FFFF in `ram[]` (misurati F7CE F7DE F7EE F7FE F8FE
  FDEE). Sulla scheda F000-FFFF è ROM e la scrittura si perde; nel C scrive
  nella copia, che è la tabella dei fondali. Nei byte misurati il bit 6 era già
  0, quindi nessun effetto visto; con le voci in `const` il ciclo va limitato a
  `p < 0xf000` (o scrivere nella sola RAM vera), altrimenti scrive su memoria
  di sola lettura. È l'unico tocco fuori voce (FDEE, riempimento).
- **`tab8089` e `leggi(HL)`** (`nemici_tipi_15_17.c:386`): la copia a mano
  copre E < 4; per E ≥ 4 il C legge la ROM a 808D+, che è codice (808D è
  `sub_808d`). Mai misurato; va deciso se E ≥ 4 è possibile o è una sentinella.
- **Doppione 7B9E**: `lista_7b9e` (`attivita_06b.c`) e `sub_7b9e`
  (`z80_7xxx.c`) leggono la stessa tabella; solo il primo è eseguito.
- **Copia di F000 in `ram[]`** (`macchina.c:613`, `macchina_libera.c:138`):
  con `rom_componi()` resta uguale ma la sorgente diventa la voce; il banco e
  il replay ricevono F000-FFFF dal dump di MAME e non cambiano.

## Ordine consigliato dei lotti

| lotto | contenuto | byte ROM | sorgente C stimato | rischio |
|---|---|---|---|---|
| 1 | audio: `audio_driver.c` (blob 0000-05C5) e caricamento senza `1.3jk` | 1478 | ~68 righe, ~7720 byte | basso: 32 byte letti, controllo già presente (`controlla_tabella`), `audio_banco` e hash audio delle ricette come regressione |
| 2 | fondali e palette F000-F996 (`rom_dati_fondali.c`, 5 voci) + limite del ciclo `res 6` | 2455 | 121 righe, ~12800 byte | basso: estensioni calcolate dai puntatori; regressione `giro_completo.sh` e `DATI.md` (52 stanze) |
| 3 | stanze, palette 80D3, tile, sequenze (`rom_dati_stanze.c`, 19 voci) | 3321 | 192 righe, ~17300 byte | medio: tre voci stimate (`seq_8418`, `tab_percorsi_84c8`, `tab_85f9`) |
| 4 | oggetti e animazioni 9484-B125 (`rom_dati_oggetti.c`, 8 voci) | 7330 | 331 righe, ~38300 byte | medio: quattro voci stimate fino alla voce seguente; la più letta (sub_07de, sub_0602) |
| 5 | tabelle inline 0000-80A4 e testi (`rom_dati_varie.c` 43 voci, `rom_dati_testi.c` 12) | 4633 | 345 righe, ~24200 byte | medio: molte voci piccole misurate; i lettori generici (rst 10, 6771, 3569) con chiamanti mai eseguiti possono puntare a tabelle non censite (sentinella) |
| 6 | ripiego, sentinella, rimozione di `carica_rom`/`main64.bin`, dati in `tecmo/` | ≤ 5568 + quota di 0000-80A4 | ≤ ~250 righe | medio: tocca il caricamento di tutti i bersagli (`harness`, `replay`, `gioco_libero`, `gioca`) |

Ogni lotto: `estrai_rom.py` rieseguito non cambia nulla, `rom_immagine`
coincide, `giro_completo.sh` con l'uscita attesa, le ricette `.reg` con gli
stessi hash di prima (video e audio), banco invariato. Nessun file generato
sopra le 800 righe: `rom_dati_oggetti.c` sta sotto (331).

## File che nominano `main64.bin` / `1.3jk` da toccare nel lotto di rimozione

Non modificati qui. Caricamento a runtime:

- `decomp/harness/comune.c`: `carica_rom()` (apre `out/main64.bin`), chiamata
  da `harness.c:186`, `replay.c:353`, `macchina.c:610`, `macchina_libera.c:136`;
- `decomp/harness/ram.h`: `rom[0x10000]`, `leggi()`, commento su `main64.bin`;
- `decomp/harness/macchina.c:613`, `macchina_libera.c:138`: `memcpy` di F000;
- `decomp/harness/audio_piattaforma.c`: `aud_carica_rom()` (apre `1.3jk`) e
  `aud_usa_dati_estratti()`, chiamate da `scheda_audio.c:74-75`,
  `audio_banco.c:150-156`, `audio_suona.c:37-43`;
- `decomp/harness/audio.h`: `aud_rom[0x4000]`.

Costruzione e documenti:

- `decomp/harness/Makefile`: bersaglio `rom` (estrae lo zip, chiama
  `build_rom.sh`) e il commento «A runtime servono solo 6.3f, 7.3h, 8.3jk
  (-> out/main64.bin) e 1.3jk»; `gioco: rom gioca`;
- `decomp/build_rom.sh`: costruisce `main48.bin` e `main64.bin`;
- `README.md` (righe 146, 175-176, 182): «the game uses only four ROM files»;
- `NOTICE.md`: elenca i dati derivati in `decomp/harness/` (`video_dati_*.c`):
  va esteso ai `rom_dati_*.c` e ad `audio_driver.c` (o alla cartella `tecmo/`).

Nominano `main64.bin` solo come strumento d'analisi (non a runtime): `disasm.sh`,
`coverage.py`, `callgraph.py`, `stanze.py`, `mappe_special.py`, `rst20.py`,
`censimento_nemici.py`, `audio_numeri.py`, `estrai_dati.py` (per `1.3jk`) e i
documenti `DATI.md`, `GIRO.md`, `AUDIO.md`, `RAPPORTO.md`: possono continuare a
usare la ROM vera, non entrano nel binario.

## Esito del lotto 62

Data: 2026-11-06 (brief 62). Le voci del Z80 principale sono estratte da
`decomp/estrai_rom.py` in `tecmo/rom_dati_{varie,testi,stanze,oggetti,fondali}.c`
(una tabella `const uint8_t` per voce), con `tecmo/rom_dati.h` e la tabella
`rom_voci[]` in `tecmo/rom_voci.c`; si aggiunge la voce locale
`riempimento_f997` (F997-FFFF) perché la finestra F000-FFFF copiata in `ram[]`
sia intera. `carica_rom()` ricompone `rom[]` dalle tabelle (zero fuori voce) e
`gioca` non apre più `out/main64.bin`; `leggi()` avvisa una volta per indirizzo
con «AVVISO: lettura ROM fuori voce» se legge sotto C000 fuori da ogni voce.
`harness/rom_immagine` confronta le tabelle con `main64.bin`.

Il banco (`harness --tutte`, non misurato nel brief 61) leggeva fuori voce in
13 catture (anche forzate): voci chiuse con il disassemblato e la struttura dei
byte. Due voci nuove e sette allargate (`VOCI` di `censimento_rom.py`), più una ottava dal collaudo (sotto):

| voce | prima | ora | motivo |
|---|---|---|---|
| `tab_chiave_0d6e` | 0D6E-0D7D | 0D6E-0D8D | 16 coppie fino al codice di 0D8E; catture `gioco_oggetti_forza02/03` leggono 0D80 e 0D8A |
| `tab_bonus_355d` | 355D-3568 | 355D-356C | `sub_354b` con c24a = 3 copia i primi 4 byte del codice di 3569 (cattura `att07_f1f`) |
| `tab_corpi_3b11` | — | 3B11-3B60 | tabella in linea di `rst 20` a 3B10 (40 corpi), letta da `rst 10` nei test 0010/0018 del banco |
| `tab_porta_47c7` | — | 47C7-47EC | 47B0: 4 record da 10 byte, 8 copiati da 3569; il quarto legge il codice di 47E5 (`porta_forza10-13`) |
| `tab_89cd` | 89CD-89D0 | 89CD-8A00 | un byte per ognuno dei 52 quadri (`titolo` legge 8A00) |
| `tab_8a01` | 8A01-8A30 | 8A01-8B2C | 25 record da 12: i valori di 89CD vanno da 00 a 18 |
| `tab_parole_8b2d` | 8B2D-8B5A | 8B2D-8D10 | 8 parole + 52 record da 9 da 8B3D (8B3D + 9·52 = 8D11) |
| `tab_8d11` | 8D11-8D14 | 8D11-8D58 | 36 parole, puntatori alle liste di 8D59 |
| `tab_8d59` | 8D59-8D5B | 8D59-8DD3 | liste chiuse da FF più il passo indietro, fino a 8DD4 (`stanza06/07/13/14`) |

Collaudo del 2026-11-06 (partita vera dell'utente, 23118 frame): la sentinella
ha segnalato 826 letture fuori voce, tutte in 8693-89CC. È il seguito di
`tab_3709_8661`: le 52 parole di `tab_85f9` (85F9-8660, una per stanza)
puntano a record da 8661 a 89C2, ognuno chiuso da FF; l'ultimo finisce a 89CC,
subito prima di `tab_89cd`. La voce passa da 8661-8692 (50 byte, *misurata*) a
8661-89CC (876 byte, *puntatori*).

Totale: 90 voci del principale nel censimento (89 «letto dalla ROM», 19629
byte, più `tab_8089`), 91 tabelle in `tecmo/` con il riempimento, 21274 byte.
Le altre voci di estensione *misurata* o *stimata* restano come nel brief 61:
una lettura fuori voce nel gioco vero si vede dall'avviso e si chiude con la
stessa procedura (disassemblato, `VOCI`, `estrai_rom.py`). La tabella
«Estrazione» sopra è la stima del brief 61 e non è aggiornata.

## Esito del lotto 63

- Il codice del driver del Z80 audio (0000-05C5 di `1.3jk`, 1478 byte) è un
  blob C generato da `estrai_rom.py` in `tecmo/audio_driver.c`
  (`aud_driver[1478]`, dichiarato in `tecmo/audio_driver.h`); il generatore
  controlla che le voci `A` del censimento (`aud_gestori_03ce`,
  `aud_dati_05c6`) cadano dentro e a ridosso del blob.
- I dati audio (`audio_dati_tabelle.c`, `audio_dati_spartiti_a.c`-`_g.c`,
  `audio_dati_indirizzi.h`, `audio_dati_gruppi.h`) e la grafica (12
  `video_dati_*.c`) sono passati da `decomp/harness/` a `tecmo/` con rinomine
  pure; `audio_dati.h`/`audio_dati.c` (macro e serializzatore) restano in
  `harness/`. Gli estrattori scrivono ora in `tecmo/`.
- `aud_carica_rom()` ricompone `aud_rom[]` da tabelle e blob: `1.3jk` non
  viene più aperto, `gioca` non legge nessun file ROM.
  `harness/audio_immagine` scrive i 16384 byte, uguali a `1.3jk` (`cmp`).
- `make gioco` non dipende più dal bersaglio `rom`, che resta come strumento
  di sviluppo (estrae `roms/solomon.zip`, costruisce `out/main64.bin` per
  `rom_immagine` e gli estrattori).
- Promemoria per il lotto 64: restano disallineati `README.md` r. 173-179
  («four ROM files», `solomon.zip` richiesto per giocare) e `NOTICE.md` r.
  9-11 (`decomp/harness/audio_dati*.c`, `decomp/harness/video_dati_*.c`).
