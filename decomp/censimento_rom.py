#!/usr/bin/env python3
"""Censimento delle letture della ROM a runtime (brief 61). Sola lettura.

Uso (da decomp/):
  python3 -I censimento_rom.py                 sintesi (voci, copertura, zone)
  python3 -I censimento_rom.py --verifica      controlli; ultima riga
                                               «CENSIMENTO OK: N occorrenze, 0 non classificate»
  python3 -I censimento_rom.py --tabella       tabella delle voci in markdown
  python3 -I censimento_rom.py --siti          ripartizione per file delle occorrenze di leggi(
  python3 -I censimento_rom.py --zone          ripartizione di main64.bin e zone mai lette
  python3 -I censimento_rom.py --estrazione    via d'estrazione proposta per voce e per file
  python3 -I censimento_rom.py --unisci F...   unisce i file della copia strumentata
                                               in out/censimento_rom_letti.txt

Legge out/main64.bin, le ROM in ../roms/ext/solomon/, harness/*.c e
harness/rst20_tables.h, rst38_tables.h; non scrive nulla, tranne
out/censimento_rom_letti.txt con --unisci.

Spazi: P = Z80 principale (main64.bin), A = Z80 audio (1.3jk).
La misura dinamica viene da una copia strumentata di harness/ (fuori dal
repository): leggi(), M() sopra f000 e aud_leggi() registrano per sito
(file, riga) gli indirizzi ROM letti; il formato del file unito è
  S <tipo> <file> <riga> <letture rom> <letture ram>   tipo L/M/A
  I <inizio> <fine>                                     intervalli del sito
"""
import glob
import os
import re
import sys

QUI = os.path.dirname(os.path.abspath(__file__))
HARNESS = os.path.join(QUI, 'harness')
MAIN64 = os.path.join(QUI, 'out', 'main64.bin')
LETTI = os.path.join(QUI, 'out', 'censimento_rom_letti.txt')
CHIP = os.path.join(QUI, '..', 'roms', 'ext', 'solomon')
OCC = re.compile(r'(^|[^_a-z0-9])leggi\(')

# Voci: (spazio, nome, inizio, fine, stato, lettori, formato, estensione)
# stato: «estratto» (già in C generato) o «letto dalla ROM».
# estensione: statica (dal codice: base e indice limitato), puntatori
# (calcolata seguendo le parole della ROM), zona (intervallo di dati fra
# codice eseguito, out/copertura.txt), misurata (dai byte letti; il lotto
# di estrazione la conferma), stimata (fino alla voce seguente).
E, L = 'estratto', 'letto dalla ROM'
VOCI = [
    ('P', 'tab_parole_08ae', 0x08AE, 0x08CD, L, '08A2', '16 parole', 'zona'),
    ('P', 'tab_gestori_chiave_0d18', 0x0D18, 0x0D37, L, '0CFD', '16 parole (gestori della chiave)', 'statica'),
    ('P', 'tab_chiave_0d6e', 0x0D6E, 0x0D8D, L, '0D3C', '16 coppie, indice <= 1E (brief 62: 0D7E-0D8D dal banco)', 'statica'),
    ('P', 'tab_chiave_123c', 0x123C, 0x1287, L, '0D8E 0DD1 0E1F 0E64 0EDF 0F2E 100E 105D',
     '8 tabelle di byte (123C 1246 124F 1258 1261 126A 1274 127E)', 'zona'),
    ('P', 'tab_punti_150c', 0x150C, 0x1532, L, '13FC, rst 10', '13 voci da 3 byte', 'statica'),
    ('P', 'tab_mappe_blocchi_1557', 0x1557, 0x1F16, L, '3812', '52 stanze x 48 byte', 'statica'),
    ('P', 'tab_boot_295c', 0x295C, 0x297F, L, '2833 (boot)', '36 byte copiati', 'statica'),
    ('P', 'tab_monete_29c7', 0x29C7, 0x29CE, L, '2980', 'monete 29C7, vite 29CB', 'zona'),
    ('P', 'tab_testi_2b29', 0x2B29, 0x2BC2, L, '2B08, rst 10', 'parole 2B29 + testi 2B61', 'zona'),
    ('P', 'tab_attract_2d4a', 0x2D4A, 0x2D71, L, '2D07', '8 byte + 32 posizioni (R & 1F)', 'statica'),
    ('P', 'tab_303e', 0x303E, 0x3044, L, '3009', '7 byte copiati', 'statica'),
    ('P', 'tab_3111', 0x3111, 0x3130, L, '30EF', '32 byte', 'zona'),
    ('P', 'testo_31e1', 0x31E1, 0x31E8, L, '6771', 'stringa (attributo, caratteri, FF)', 'zona'),
    ('P', 'tab_3427', 0x3427, 0x342C, L, '33EB', '6 byte copiati', 'statica'),
    ('P', 'tab_bonus_343f', 0x343F, 0x349E, L, 'rst 10, 4C89', 'parole e BCD', 'zona'),
    ('P', 'tab_bonus_355d', 0x355D, 0x356C, L, '354B via 3569', '4 record da 4 byte (c24a); il quarto sono i primi byte del codice di 3569 (brief 62)', 'statica'),
    ('P', 'tab_copie_3574', 0x3574, 0x358E, L, '23F8', 'record da 3 byte', 'zona'),
    ('P', 'tab_36b3', 0x36B3, 0x36BC, L, '3678', '10 byte copiati', 'statica'),
    ('P', 'testi_39c0', 0x39C0, 0x3A0D, L, '6771', 'stringhe', 'zona'),
    ('P', 'tab_quadri_3a0e', 0x3A0E, 0x3A1B, L, '3861', '7 parole confrontate', 'zona'),
    ('P', 'tab_corpi_3b11', 0x3B11, 0x3B60, L, 'rst 20 a 3B10 (rst 10; banco 0010/0018)', '40 parole, indirizzi dei corpi (brief 62)', 'zona'),
    ('P', 'testi_bonus_3d0a', 0x3D0A, 0x3D1F, L, '3BAC (att. 25)', '2 stringhe + 3 byte BCD', 'statica'),
    ('P', 'tab_bonus_3e72', 0x3E72, 0x3E8F, L, '3569', '30 byte copiati', 'misurata'),
    ('P', 'testi_3f6f', 0x3F6F, 0x3F87, L, '6771', 'stringa', 'misurata'),
    ('P', 'tab_bcd_3fe8', 0x3FE8, 0x3FF6, L, '3FCE', '5 valori BCD da 3 byte', 'statica'),
    ('P', 'tab_43b9', 0x43B9, 0x43C7, L, '4398', '5 record da 3 byte', 'zona'),
    ('P', 'tab_attract_43e5', 0x43E5, 0x43FC, L, '43C8 (att. 1F)', '24 byte copiati', 'statica'),
    ('P', 'tab_attract_43fd', 0x43FD, 0x4402, L, '43C8 (att. 1F)', '6 byte copiati', 'statica'),
    ('P', 'tab_registri_4403', 0x4403, 0x451B, L, '452B', 'coppie (registro, valore)', 'zona'),
    ('P', 'tab_record_451c', 0x451C, 0x452A, L, '452B', '5 record (coppie, destinazione)', 'zona'),
    ('P', 'tab_porta_47c7', 0x47C7, 0x47EC, L, '47B0 (4767) via 3569', '4 record da 10 byte, 8 copiati; il quarto legge il codice di 47E5 (brief 62)', 'statica'),
    ('P', 'tab_record_488d', 0x488D, 0x48AE, L, 'rst 10 (nemici)', 'parole per indice', 'misurata'),
    ('P', 'tab_oggetti_48af', 0x48AF, 0x48D0, L, 'rst 10 (nemici)', 'parole per indice', 'misurata'),
    ('P', 'tab_4ff9', 0x4FF9, 0x5004, L, '4FEF', '6 parole', 'statica'),
    ('P', 'tab_nemici_608c', 0x608C, 0x610B, L, '5F4B 5F82 5FDC 6046', '4 tabelle da 16 parole', 'statica'),
    ('P', 'tab_6653', 0x6653, 0x6654, L, '3569', '2 byte copiati', 'misurata'),
    ('P', 'testo_6766', 0x6766, 0x6770, L, '6771', 'stringa', 'statica'),
    ('P', 'tab_segni_6837', 0x6837, 0x6841, L, '6785 (att. 16)', '11 caratteri', 'statica'),
    ('P', 'tab_6970', 0x6970, 0x6975, L, '6842 (att. 16)', '6 byte copiati', 'statica'),
    ('P', 'testi_6a9e', 0x6A9E, 0x6ADD, L, '6771', 'stringhe', 'misurata'),
    ('P', 'tab_6bc5', 0x6BC5, 0x6BC6, L, 'rst 10 (6BA8)', 'parole', 'misurata'),
    ('P', 'tab_velocita_6f3d', 0x6F3D, 0x6F44, L, 'rst 10 (nemici 08-14)', 'parole', 'misurata'),
    ('P', 'tab_8089', 0x8089, 0x808C, E, '8042 (copia a mano tab8089[4])', '4 byte', 'statica'),
    ('P', 'tab_70ee', 0x70EE, 0x70FC, L, '6ECA', '15 byte', 'misurata'),
    ('P', 'tab_direzioni_7196', 0x7196, 0x719D, L, 'rst 10 (nemici 10-13)', '4 parole', 'statica'),
    ('P', 'tab_velocita_7468', 0x7468, 0x746B, L, 'rst 10 (comparsa)', 'coppie di byte', 'misurata'),
    ('P', 'tab_urto_754c', 0x754C, 0x7567, L, 'rst 10 (comparsa)', 'parole urto X (754C) e Y (755C)', 'misurata'),
    ('P', 'tab_demo_765e', 0x765E, 0x767B, L, '760B 2551', 'record da 3 byte', 'misurata'),
    ('P', 'copione_demo_767c', 0x767C, 0x770E, L, '01C5 (NMI)', 'coppie (chiave, codice)', 'misurata'),
    ('P', 'testi_79a4', 0x79A4, 0x79DE, L, '6771', 'stringhe', 'misurata'),
    ('P', 'testi_79fa', 0x79FA, 0x7A0A, L, '6771', 'stringhe', 'misurata'),
    ('P', 'testi_7a3a', 0x7A3A, 0x7A55, L, '6771', 'stringhe', 'misurata'),
    ('P', 'tab_7aef', 0x7AEF, 0x7AF2, L, '7A56', '4 byte copiati', 'statica'),
    ('P', 'testi_7af3', 0x7AF3, 0x7B3A, L, '6771', 'stringhe', 'misurata'),
    ('P', 'tab_coppie_7bb1', 0x7BB1, 0x7BF5, L, '7B9E', 'liste di coppie (B, C) chiuse da FF', 'misurata'),
    ('P', 'testi_7c58', 0x7C58, 0x7C83, L, '6771', 'stringhe', 'zona'),
    ('P', 'tab_esiti_7e3a', 0x7E3A, 0x7E45, L, 'rst 10 (7E..)', 'parole', 'misurata'),
    ('P', 'testi_7e46', 0x7E46, 0x7EC9, L, '6771', 'stringhe', 'zona'),
    ('P', 'tab_bonus_80b3', 0x80B3, 0x80D2, L, '3569', 'byte copiati', 'misurata'),
    ('P', 'tab_palette_80d3', 0x80D3, 0x82D2, L, '36C3 3569', '512 byte di palette', 'statica'),
    ('P', 'tab_82d3', 0x82D3, 0x82D9, L, '3861', '7 byte', 'misurata'),
    ('P', 'tab_casuale_82da', 0x82DA, 0x83F9, L, '4302 426A', '18 righe da 16 byte', 'statica'),
    ('P', 'tab_sequenze_83fa', 0x83FA, 0x8417, L, 'rst 10 (4DEE)', '15 parole', 'statica'),
    ('P', 'seq_8418', 0x8418, 0x84C7, L, '4DEE', 'sequenze a righe', 'stimata'),
    ('P', 'tab_percorsi_84c8', 0x84C8, 0x8577, L, '5D29', 'parole + record', 'stimata'),
    ('P', 'tab_tile_8578', 0x8578, 0x858D, L, '3A2E', '11 coppie (codice, colore)', 'statica'),
    ('P', 'tab_tile_858e', 0x858E, 0x85EB, L, '3A2E', '47 coppie (valori 08-36)', 'statica'),
    ('P', 'tab_quadri_85ec', 0x85EC, 0x85F8, L, '3861', '13 soglie', 'misurata'),
    ('P', 'tab_85f9', 0x85F9, 0x8660, L, 'rst 10 (3709)', 'parole', 'stimata'),
    ('P', 'tab_3709_8661', 0x8661, 0x8692, L, '3709', 'record', 'misurata'),
    ('P', 'tab_89cd', 0x89CD, 0x8A00, L, '2368 (2E9C)', '52 byte, indice di 8A01 per quadro (brief 62)', 'statica'),
    ('P', 'tab_8a01', 0x8A01, 0x8B2C, L, '4E87', '25 record da 12 byte, indici 00-18 di 89CD (brief 62)', 'statica'),
    ('P', 'tab_parole_8b2d', 0x8B2D, 0x8D10, L, 'rst 10, 2E9C', '8 parole + 52 record da 9 da 8B3D (brief 62)', 'statica'),
    ('P', 'tab_8d11', 0x8D11, 0x8D58, L, '2E9C', '36 parole, liste di 8D59 (brief 62)', 'statica'),
    ('P', 'tab_8d59', 0x8D59, 0x8DD3, L, '470A 46B4', 'liste chiuse da FF + passo indietro (brief 62)', 'puntatori'),
    ('P', 'tab_avanzamento_8dd4', 0x8DD4, 0x8EA3, L, '34FA', '52 record da 4 byte', 'statica'),
    ('P', 'tab_stanze_8ea4', 0x8EA4, 0x9483, L, '358F 147F', '52 parole + record (10 byte + coppie fino a FF)', 'puntatori'),
    ('P', 'tab_soglie_9484', 0x9484, 0x9494, L, '4722', '17 soglie', 'misurata'),
    ('P', 'tab_oggetti_9495', 0x9495, 0x9554, L, '027C 0602 4592 4B20 2474 24DF', '24 record da 8 byte', 'stimata'),
    ('P', 'tab_anim_9555', 0x9555, 0x969C, L, '07DE', '164 parole', 'statica'),
    ('P', 'tab_anim_rec_969d', 0x969D, 0x9DCC, L, '07DE', 'record da 3 byte e parole', 'stimata'),
    ('P', 'tab_anim2_9dcd', 0x9DCD, 0xA745, L, '07DE', 'record', 'stimata'),
    ('P', 'tab_comparse_a746', 0xA746, 0xA88D, L, '0602', '164 parole', 'statica'),
    ('P', 'tab_comparse_rec_a88e', 0xA88E, 0xAEE1, L, '0602', 'record da 3 byte + parole', 'stimata'),
    ('P', 'tab_campi_aee2', 0xAEE2, 0xB125, L, '0602', 'campi copiati', 'misurata'),
    ('P', 'tab_palette_f000', 0xF000, 0xF067, L, '33BA (via ram[])', '52 parole', 'statica'),
    ('P', 'tab_fondali_f068', 0xF068, 0xF137, L, '3231 (via ram[])', '52 voci da 4 byte', 'statica'),
    ('P', 'tab_sovrapposizioni_f138', 0xF138, 0xF1E7, L, '325E (via ram[])', '44 voci da 4 byte', 'puntatori'),
    ('P', 'rec_palette_f1e8', 0xF1E8, 0xF414, L, '33BA (via ram[])', '13 record (h, 2*(h&7F) byte)', 'puntatori'),
    ('P', 'flussi_fondale_f415', 0xF415, 0xF996, L, '326C (via ram[])', 'flussi chiusi da tipo 3', 'puntatori'),
    ('A', 'aud_gestori_03ce', 0x03CE, 0x03ED, L, 'controlla_tabella (audio_interprete.c)',
     '16 parole, già copiate a mano in indirizzi[16]', 'statica'),
    ('A', 'aud_dati_05c6', 0x05C6, 0x3FFF, E, 'interprete, ossatura (estrai_dati.py)', 'tabelle e spartiti', 'statica'),
]

# Siti di leggi( per riga: categoria e voci. V = tabelle fisse; G = lettore
# generico (indirizzo dal chiamante, ROM o RAM); R = solo RAM (c000-ffff);
# C = occorrenza in un commento.
SITI = {}
def _s(file, righe, cat, *voci):
    for r in righe:
        SITI['%s:%d' % (file, r)] = (cat, voci)
_s('attivita_06b.c', (59, 64), 'V', 'tab_coppie_7bb1')
_s('attivita_12_13.c', (53,), 'V', 'tab_casuale_82da')
_s('attivita_16.c', (100,), 'V', 'tab_segni_6837')
_s('attivita_16.c', (136,), 'R')
_s('attivita_16b.c', (119,), 'V', 'tab_6970')
_s('attivita_16b.c', (189,), 'V', 'tab_7aef')
_s('attivita_16b.c', (221, 239, 251), 'R')
_s('attivita_25.c', (186,), 'V', 'testi_bonus_3d0a')
_s('attivita_attract.c', (43,), 'V', 'tab_attract_43e5')
_s('attivita_attract.c', (45,), 'V', 'tab_attract_43fd')
_s('attivita_attract.c', (75, 78, 79), 'V', 'tab_demo_765e')
_s('attivita_attract07.c', (71, 72), 'V', 'tab_attract_2d4a')
_s('attivita_attract07a.c', (34,), 'V', 'tab_quadri_3a0e')
_s('attivita_attract07a.c', (94,), 'V', 'tab_82d3')
_s('attivita_attract07a.c', (121, 126), 'V', 'tab_quadri_85ec')
_s('attivita_attract07a.c', (223,), 'V', 'tab_avanzamento_8dd4')
_s('attivita_attract07b.c', (46, 50, 59, 69, 70, 80, 96, 121, 125), 'V', 'tab_stanze_8ea4')
_s('attivita_attract07c.c', (41,), 'V', 'tab_303e')
_s('attivita_attract07c.c', (269,), 'V', 'tab_parole_8b2d')
_s('attivita_attract07c.c', (282, 290), 'V', 'tab_8d11')
_s('attivita_attract07d.c', (45, 64, 68), 'V', 'tab_3709_8661')
_s('attivita_piccoli.c', (108,), 'V', 'tab_bonus_343f')
_s('macchina.c', (368, 369, 370), 'V', 'tab_boot_295c')
_s('macchina.c', (611,), 'C')
_s('macchina_libera.c', (137,), 'C')
_s('nemici.c', (281, 286, 435), 'V', 'tab_8d59')
_s('nemici_comparsa.c', (105,), 'V', 'tab_punti_150c')
_s('nemici_comparsab.c', (60,), 'R')
_s('nemici_comuni.c', (99, 101, 258, 268, 285, 441, 443), 'V', 'tab_oggetti_9495')
_s('nemici_tipi_08_14.c', (121, 130, 192), 'R')
_s('nemici_tipi_08_14.c', (689,), 'V', 'tab_70ee')
_s('nemici_tipi_10_13.c', (405,), 'R')
_s('nemici_tipi_10_13b.c', (264,), 'R')
_s('nemici_tipi_15_17.c', (386,), 'V', 'tab_8089')
_s('nemici_tipo06a.c', (33,), 'C')
_s('nemici_tipo06a.c', (37, 40), 'V', 'tab_oggetti_9495')
_s('nemici_tipo06a.c', (173, 175), 'V', 'tab_4ff9')
_s('nemici_tipo07a.c', (127,), 'V', 'tab_percorsi_84c8')
_s('nemici_tipo07a.c', (421, 423, 439, 441), 'V', 'tab_nemici_608c')
_s('nemici_tipo07b.c', (280, 282, 336, 338), 'V', 'tab_nemici_608c')
_s('nmi_oggetti.c', (130, 134, 141, 144, 193, 197, 205, 209), 'V', 'tab_oggetti_9495')
_s('raccolta_13xx.c', (182, 199), 'V', 'tab_stanze_8ea4')
_s('z80_08xx.c', (443,), 'V', 'tab_parole_08ae')
_s('z80_0xxx.c', (19, 20), 'G')
_s('z80_0xxx.c', (69, 71), 'V', 'copione_demo_767c')
_s('z80_0xxx.c', (90, 95, 97, 98, 99, 106, 107, 133, 135, 145, 146, 164), 'R')
_s('z80_0xxx.c', (115,), 'V', 'tab_anim_9555')
_s('z80_0xxx.c', (117, 122, 125, 126), 'V', 'tab_anim_rec_969d')
_s('z80_0xxx.c', (171, 172, 173, 180, 184, 192), 'V', 'tab_anim2_9dcd')
_s('z80_0xxx.c', (259, 260), 'V', 'tab_comparse_a746')
_s('z80_0xxx.c', (261, 262, 265), 'V', 'tab_comparse_rec_a88e')
_s('z80_0xxx.c', (276,), 'V', 'tab_campi_aee2')
_s('z80_0xxx.c', (285,), 'V', 'tab_oggetti_9495')
_s('z80_2xxx.c', (31, 32, 536, 538), 'R')
_s('z80_2xxx.c', (87,), 'G')
_s('z80_2xxx.c', (112, 113, 114), 'V', 'tab_copie_3574')
_s('z80_2xxx.c', (166,), 'V', 'tab_testi_2b29')
_s('z80_2xxx.c', (289, 292, 297), 'V', 'tab_monete_29c7')
_s('z80_2xxx.c', (520,), 'V', 'tab_demo_765e')
_s('z80_38xx.c', (89, 90), 'V', 'tab_tile_8578', 'tab_tile_858e')
_s('z80_38xx.c', (105,), 'V', 'tab_mappe_blocchi_1557')
_s('z80_38xx.c', (244,), 'V', 'tab_bcd_3fe8')
_s('z80_3xxx.c', (20,), 'V', 'tab_3111')
_s('z80_3xxx.c', (106, 108), 'V', 'tab_sovrapposizioni_f138')
_s('z80_3xxx.c', (152, 156), 'V', 'rec_palette_f1e8')
_s('z80_3xxx.c', (359,), 'V', 'tab_36b3')
_s('z80_3xxx.c', (371, 373, 375, 377), 'V', 'tab_palette_80d3')
_s('z80_3xxx.c', (426,), 'V', 'tab_3427')
_s('z80_3xxx.c', (500,), 'G')
_s('z80_4xxx.c', (45,), 'V', 'tab_soglie_9484')
_s('z80_4xxx.c', (90, 97, 100, 110, 112), 'V', 'seq_8418')
_s('z80_4xxx.c', (252, 254), 'V', 'tab_8a01')
_s('z80_4xxx.c', (297, 298), 'V', 'tab_record_451c')
_s('z80_4xxx.c', (311, 312), 'V', 'tab_registri_4403')
_s('z80_4xxx.c', (442, 542, 574), 'R')
_s('z80_4xxx.c', (476, 477), 'V', 'tab_43b9')
_s('z80_4xxx.c', (525,), 'V', 'tab_casuale_82da')
_s('z80_5xxx.c', (11,), 'R')
_s('z80_6xxx.c', (31, 33), 'G')
_s('z80_7xxx.c', (119, 126), 'R')
_s('z80_7xxx.c', (254, 260), 'V', 'tab_coppie_7bb1')
_s('z80_chiave_a.c', (7,), 'C')
_s('z80_chiave_a.c', (42,), 'V', 'tab_gestori_chiave_0d18')
_s('z80_chiave_a.c', (77, 81), 'V', 'tab_chiave_0d6e')
_s('z80_chiave_a.c', (101, 142, 186, 224, 293, 331), 'V', 'tab_chiave_123c')
_s('z80_chiave_b.c', (4,), 'C')
_s('z80_chiave_b.c', (133, 173), 'V', 'tab_chiave_123c')

# Siti fuori da leggi(: M() sopra f000 (copia della finestra in ram[]) e aud_leggi.
ALTRI = {
    'M fondale.c:113': ('V', ('flussi_fondale_f415',)),
    # res 6 sui 18 slot da c4ce con passo D:30: non è un dato, l'indirizzo
    # scorre oltre c000 e tocca la copia di f000 (vedi CENSIMENTO-ROM.md)
    'M nmi_oggetti.c:236': ('X', ()),
}

# Siti di aud_leggi( (Z80 audio): V = tabelle, R = RAM 4000-47ff, D = definizione.
AUD_SITI = {
    'audio_piattaforma.c:104': ('D', ()),
    'audio_interprete.c:37': ('V', ('aud_gestori_03ce', 'aud_dati_05c6')),
    'audio_ossatura.c:74': ('R', ()),
    'audio_ossatura.c:76': ('R', ()),
}
for _k in ('audio_interprete.c:%d' % r for r in (52, 54, 90, 108, 111, 124, 126, 399, 402, 407)):
    AUD_SITI[_k] = ('V', ('aud_dati_05c6',))
for _k in ('audio_ossatura.c:%d' % r for r in (69, 77, 137, 139, 151)):
    AUD_SITI[_k] = ('V', ('aud_dati_05c6',))
for _k, _v in AUD_SITI.items():
    if _v[0] != 'D':
        ALTRI['A ' + _k] = _v
OCC_AUD = re.compile(r'(^|[^_a-z0-9])aud_leggi\(')


def esa(a):
    return '%04X' % a


def leggi_bin(p):
    with open(p, 'rb') as f:
        return f.read()


def origine(spazio, a):
    """File ROM e posizione del byte a (build_rom.sh)."""
    if spazio == 'A':
        return '1.3jk', a
    if a < 0x4000:
        return '6.3f', a
    if a < 0x8000:
        return '7.3h', a            # seconda metà di 7.3h
    if a < 0xC000:
        return '7.3h', a - 0x8000   # prima metà di 7.3h
    if a >= 0xF000:
        return '8.3jk', a - 0xF000
    return None, None


def occorrenze(espr=OCC):
    """[(file, riga, n)] delle righe di harness/*.c con leggi( (o espr)."""
    out = []
    for p in sorted(glob.glob(os.path.join(HARNESS, '*.c'))):
        with open(p, encoding='utf-8') as f:
            for i, riga in enumerate(f, 1):
                n = len(espr.findall(riga))
                if n:
                    out.append((os.path.basename(p), i, n))
    return out


def carica_letti():
    """{chiave sito: [rom, ram, set di indirizzi]} da out/censimento_rom_letti.txt."""
    siti = {}
    if not os.path.exists(LETTI):
        return None
    cur = None
    with open(LETTI) as f:
        for riga in f:
            p = riga.split()
            if not p or p[0].startswith('#'):
                continue
            if p[0] == 'S':
                k = '%s %s:%s' % (p[1], p[2], p[3])
                cur = siti.setdefault(k, [0, 0, set()])
                cur[0] += int(p[4])
                cur[1] += int(p[5])
            elif p[0] == 'I':
                cur[2].update(range(int(p[1], 16), int(p[2], 16) + 1))
    return siti


def unisci(file):
    siti = {}
    percorsi = []
    for fn in file:
        percorsi.append(os.path.basename(fn))
        cur = None
        with open(fn) as f:
            for riga in f:
                p = riga.split()
                if p[0] == 'S':
                    k = (p[1], p[2], int(p[3]))
                    cur = siti.setdefault(k, [0, 0, set()])
                    cur[0] += int(p[4])
                    cur[1] += int(p[5])
                else:
                    cur[2].update(range(int(p[1], 16), int(p[2], 16) + 1))
    righe = ['# censimento_rom_letti (brief 61): unione di %d percorsi: %s'
             % (len(percorsi), ' '.join(percorsi))]
    for k in sorted(siti):
        r, m, s = siti[k]
        righe.append('S %s %s %d %d %d' % (k[0], k[1], k[2], r, m))
        righe += ['I %s %s' % (esa(a), esa(b)) for a, b in intervalli(s)]
    for sp, tipi in (('P', 'LM'), ('A', 'A')):
        u = set().union(*[v[2] for k, v in siti.items() if k[0] in tipi] or [set()])
        righe += ['U %s %s %s' % (sp, esa(a), esa(b)) for a, b in intervalli(u)]
    os.makedirs(os.path.dirname(LETTI), exist_ok=True)
    with open(LETTI, 'w') as f:
        f.write('\n'.join(righe) + '\n')
    print('scritto %s: %d siti' % (os.path.relpath(LETTI, QUI), len(siti)))


def intervalli(s):
    out = []
    for a in sorted(s):
        if out and out[-1][1] == a - 1:
            out[-1][1] = a
        else:
            out.append([a, a])
    return out


def unione(letti, spazio):
    tipi = 'LM' if spazio == 'P' else 'A'
    u = set()
    for k, v in letti.items():
        if k[0] in tipi and not (k in ALTRI and ALTRI[k][0] == 'X'):
            u |= v[2]
    return u


def voce_di(spazio, a):
    for v in VOCI:
        if v[0] == spazio and v[2] <= a <= v[3]:
            return v
    return None


def controlla_rst(rom):
    """Le tabelle a mano rst20/rst38 contro la ROM: [(nome, voci, base, esito)]."""
    out = []
    for h, pref in (('rst20_tables.h', 'rst20'), ('rst38_tables.h', 'rst38')):
        testo = open(os.path.join(HARNESS, h), encoding='utf-8').read()
        for m in re.finditer(r'static const uint16_t %s_([0-9A-F]{4})\[\] = \{([^}]*)\}' % pref, testo):
            sito = int(m.group(1), 16)
            parole = [int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]{4})', m.group(2))]
            esito = None
            for base in (sito + 1, sito, sito + 2):
                if all(rom[base + 2 * i] | rom[base + 2 * i + 1] << 8 == w for i, w in enumerate(parole)):
                    esito = base
                    break
            out.append(('%s_%s' % (pref, m.group(1)), len(parole), esito))
    return out


def voce_stats(v, u):
    letti = sum(1 for a in range(v[2], v[3] + 1) if a in u)
    return letti, v[3] - v[2] + 1


def tabella(letti):
    up, ua = unione(letti, 'P'), unione(letti, 'A')
    print('| voce | inizio-fine | byte | stato | letta da | formato | estensione | letti | origine |')
    print('|---|---|---|---|---|---|---|---|---|')
    for v in VOCI:
        n, tot = voce_stats(v, up if v[0] == 'P' else ua)
        f, off = origine(v[0], v[2])
        print('| `%s` | %s-%s | %d | %s | %s | %s | %s | %d/%d | %s+%s |'
              % (v[1], esa(v[2]), esa(v[3]), tot, v[4], v[5], v[6], v[7], n, tot, f, esa(off)))


def siti_per_file(occ):
    per = {}
    for f, r, n in occ:
        cat = SITI.get('%s:%d' % (f, r), ('?',))[0]
        d = per.setdefault(f, {})
        d[cat] = d.get(cat, 0) + n
    print('| file | V | G | R | C | totale |')
    print('|---|---|---|---|---|---|')
    tot = {}
    for f in sorted(per):
        d = per[f]
        for c in 'VGRC':
            tot[c] = tot.get(c, 0) + d.get(c, 0)
        print('| %s | %s | %d |' % (f, ' | '.join(str(d.get(c, 0)) for c in 'VGRC'), sum(d.values())))
    print('| **totale** | %s | %d |' % (' | '.join(str(tot.get(c, 0)) for c in 'VGRC'), sum(tot.values())))


def zone(letti, rom):
    """Ripartizione di main64.bin: codice eseguito in MAME (out/copertura.txt),
    voci, dati fuori voce (ripiego), riempimento; zone mai lette."""
    up = unione(letti, 'P')
    cop = []
    with open(os.path.join(QUI, 'out', 'copertura.txt')) as f:
        for riga in f:
            if riga.startswith('#'):
                continue
            r, k = riga.split()
            a, b = r.split('-')
            cop.append((int(a, 16), int(b, 16), k))
    codice = set()
    for a, b, k in cop:
        if k == 'codice':
            codice.update(range(a, b + 1))
    in_voce = set()
    for v in VOCI:
        if v[0] == 'P':
            in_voce.update(range(v[2], v[3] + 1))
    print('letti in codice eseguito da MAME: %d' % len(up & codice))
    for nome, lo, hi in (('0000-BFFF', 0, 0xBFFF), ('F000-FFFF', 0xF000, 0xFFFF)):
        tutto = set(range(lo, hi + 1))
        c = tutto & codice
        v = tutto & in_voce
        resto = tutto - codice - in_voce
        riemp = {a for a in resto if rom[a] in (0x00, 0xFF)}
        print('%s: codice %d, voci %d (letti %d), dati fuori voce %d (di cui 00/FF %d)'
              % (nome, len(c), len(v), len(v & up), len(resto), len(riemp)))
        grandi = [(a, b) for a, b in intervalli(resto) if b - a + 1 >= 64]
        print('  fuori voce >= 64 byte: ' + ' '.join('%s-%s(%d)' % (esa(a), esa(b), b - a + 1) for a, b in grandi))
    print('C000-EFFF: 12288 byte a zero, mai indirizzati come ROM (leggi() li serve da ram[])')


# Rapporto misurato di video_dati_*.c: 16384 byte di ROM in 689 righe e
# 85587 byte di sorgente per file.
RIGHE_PER_PEZZO, BYTE_C_PER_PEZZO, PEZZO = 689, 85587, 16384


def file_generato(v):
    """File C proposto per la voce (lotti in CENSIMENTO-ROM.md)."""
    if v[0] == 'A':
        return 'audio_driver.c' if v[2] < 0x05C6 else 'audio_dati_*.c (esiste)'
    if v[2] >= 0xF000:
        return 'rom_dati_fondali.c'
    if v[1].startswith('testi') or v[1].startswith('testo'):
        return 'rom_dati_testi.c'
    if v[2] >= 0x9484:
        return 'rom_dati_oggetti.c'
    if v[2] >= 0x80A5:
        return 'rom_dati_stanze.c'
    return 'rom_dati_varie.c'


def estrazione():
    print('| voce | byte | file generato | simbolo | righe C | byte C | controllo |')
    print('|---|---|---|---|---|---|---|')
    gruppi = {}
    for v in VOCI:
        if v[4] != L:
            continue
        n = v[3] - v[2] + 1
        righe = -(-n // 24) + 2
        bc = round(n * BYTE_C_PER_PEZZO / PEZZO)
        f = file_generato(v)
        g = gruppi.setdefault(f, [0, 0, 0, 0])
        g[0] += 1
        g[1] += n
        g[2] += righe
        g[3] += bc
        print('| `%s` | %d | %s | `const uint8_t %s[%d]` | %d | %d | round-trip byte per byte, uscita 2 |'
              % (v[1], n, f, v[1], n, righe, bc))
    print()
    print('| file generato | voci | byte ROM | righe C stimate | byte C stimati |')
    print('|---|---|---|---|---|')
    for f in sorted(gruppi):
        g = gruppi[f]
        print('| %s | %d | %d | %d | %d |' % (f, g[0], g[1], g[2] + 6, g[3]))


def main():
    arg = sys.argv[1:]
    if arg and arg[0] == '--unisci':
        unisci(arg[1:])
        return 0
    rom = leggi_bin(MAIN64)
    occ = occorrenze()
    letti = carica_letti()
    if '--siti' in arg:
        siti_per_file(occ)
        return 0
    if letti is None:
        print('manca %s: rifare la misura (vedi CENSIMENTO-ROM.md) e --unisci' % os.path.relpath(LETTI, QUI))
        return 2
    if '--estrazione' in arg:
        estrazione()
        return 0
    if '--zone' in arg:
        zone(letti, rom)
        return 0
    if '--tabella' in arg:
        tabella(letti)
        return 0

    errori = []
    n_occ = sum(n for _, _, n in occ)
    non_class = [(f, r, n) for f, r, n in occ if '%s:%d' % (f, r) not in SITI]
    vecchi = set(SITI) - {'%s:%d' % (f, r) for f, r, _ in occ}
    for k in sorted(vecchi):
        errori.append('sito classificato ma senza leggi( in harness: %s' % k)
    occ_a = occorrenze(OCC_AUD)
    n_aud = sum(n for _, _, n in occ_a)
    non_class += [(f, r, n) for f, r, n in occ_a if '%s:%d' % (f, r) not in AUD_SITI]
    for k in sorted(set(AUD_SITI) - {'%s:%d' % (f, r) for f, r, _ in occ_a}):
        errori.append('sito audio classificato ma senza aud_leggi(: %s' % k)
    nomi = {v[1] for v in VOCI}
    for k, (cat, voci) in list(SITI.items()) + list(ALTRI.items()):
        for v in voci:
            if v not in nomi:
                errori.append('%s: voce sconosciuta %s' % (k, v))

    # voci: non sovrapposte, dentro la ROM, byte d'origine uguali a main64
    for sp in 'PA':
        vv = sorted((v for v in VOCI if v[0] == sp), key=lambda v: v[2])
        for a, b in zip(vv, vv[1:]):
            if b[2] <= a[3]:
                errori.append('voci sovrapposte: %s e %s' % (a[1], b[1]))
    chip = {}
    for v in VOCI:
        f, off = origine(v[0], v[2])
        if f is None:
            errori.append('%s fuori dalla ROM' % v[1])
            continue
        if f not in chip:
            chip[f] = leggi_bin(os.path.join(CHIP, f))
        img = chip[f] if v[0] == 'A' else rom
        base = off if v[0] == 'A' else v[2]
        if chip[f][off:off + v[3] - v[2] + 1] != img[base:base + v[3] - v[2] + 1]:
            errori.append('%s: byte diversi fra main64.bin e %s' % (v[1], f))

    # dinamica contro statica
    up, ua = unione(letti, 'P'), unione(letti, 'A')
    fuori = {}
    for k, (r, m, s) in letti.items():
        cat, voci = (ALTRI.get(k) or SITI.get(k.split(' ', 1)[1], ('?', ())))
        if cat == '?':
            errori.append('sito misurato senza classificazione: %s' % k)
            continue
        sp = 'A' if k[0] == 'A' else 'P'
        for a in s:
            v = voce_di(sp, a)
            if cat == 'X':
                if v is None:
                    fuori.setdefault((k, 'X'), set()).add(a)
                continue
            if v is None:
                fuori.setdefault((k, cat), set()).add(a)
            elif cat == 'V' and v[1] not in voci:
                errori.append('%s legge %s in %s, non fra le sue voci' % (k, esa(a), v[1]))
            elif cat == 'R':
                errori.append('%s classificato RAM ma legge la ROM a %s' % (k, esa(a)))
    c000 = sum(1 for a in up if 0xC000 <= a <= 0xEFFF)
    n_fuori = sum(len(s) for (k, c), s in fuori.items() if c != 'X')
    n_x = sum(len(s) for (k, c), s in fuori.items() if c == 'X')
    rst = controlla_rst(rom)
    for nome, n, base in rst:
        if base is None:
            errori.append('%s: le parole non coincidono con la ROM' % nome)

    stati = {}
    for v in VOCI:
        n, tot = voce_stats(v, up if v[0] == 'P' else ua)
        if n > tot:
            errori.append('%s: letti %d > %d' % (v[1], n, tot))
        s = stati.setdefault((v[0], v[4]), [0, 0, 0])
        s[0] += 1
        s[1] += tot
        s[2] += n
    print('occorrenze di leggi( in harness/*.c: %d su %d righe' % (n_occ, len(occ)))
    cats = {}
    for f, r, n in occ:
        c = SITI.get('%s:%d' % (f, r), ('?',))[0]
        cats[c] = cats.get(c, 0) + n
    print('  per categoria: ' + ', '.join('%s %d' % (c, cats[c]) for c in sorted(cats)))
    misurati = {k.split(' ', 1)[1] for k in letti if k[0] == 'L'}
    mai = sum(n for f, r, n in occ if '%s:%d' % (f, r) not in misurati and SITI.get('%s:%d' % (f, r), ('?',))[0] != 'C')
    print('  occorrenze di codice mai eseguite nei percorsi misurati: %d' % mai)
    for (sp, st), (nv, tot, n) in sorted(stati.items()):
        print('voci %s «%s»: %d, %d byte, %d letti' % (sp, st, nv, tot, n))
    print('byte ROM letti: principale %d, audio %d' % (len(up), len(ua)))
    print('letture ROM in C000-EFFF: %d' % c000)
    print('occorrenze di aud_leggi(: %d su %d righe (definizione compresa)' % (n_aud, len(occ_a)))
    jk = leggi_bin(os.path.join(CHIP, '8.3jk'))
    print('8.3jk: %d byte, oltre 0x1000 valori distinti %s (mai caricati: build_rom.sh ne prende 4096)'
          % (len(jk), ' '.join('%02X' % x for x in sorted(set(jk[0x1000:])))))
    print('copia di F000-FFFF in ram[]: 4096 byte, letti %d' % sum(1 for a in up if a >= 0xF000))
    for (k, c), s in sorted(fuori.items()):
        print('  fuori voce %s [%s]: %s' % (k, c, ' '.join('%s-%s' % (esa(a), esa(b)) for a, b in intervalli(s))))
    print('tabelle a mano rst20/rst38: %d, %d coincidono con la ROM' % (len(rst), sum(1 for x in rst if x[2] is not None)))
    for e in errori:
        print('ERRORE: ' + e)
    print('LETTI FUORI VOCE: %d (più %d tocchi non di dato del ciclo res 6 di 027C)' % (n_fuori, n_x))
    if '--verifica' in arg:
        if errori or non_class or n_fuori:
            for f, r, n in non_class:
                print('NON CLASSIFICATA: %s:%d (%d)' % (f, r, n))
            print('CENSIMENTO KO: %d occorrenze, %d non classificate, %d errori'
                  % (n_occ, sum(n for _, _, n in non_class), len(errori)))
            return 1
        print('CENSIMENTO OK: %d occorrenze, 0 non classificate' % n_occ)
    return 0


if __name__ == '__main__':
    sys.exit(main())
