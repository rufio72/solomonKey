/* Generato da decomp/estrai_dati.py dalla ROM audio 1.3jk: non modificare
 * a mano. Dati musicali derivati dal programma Tecmo (NOTICE.md). */
/* Tabelle, inviluppi, tabella dei comandi, residuo 3811 e riempimenti. */
#include <stddef.h>
#include "audio_dati.h"

/* 0700-075F periodi del tono per indice 00-2F (nota + 12 * ottava + trasposizione) */
static const uint8_t tab_periodi_toni[] = {
    W(0x0B33), W(0x0A92), W(0x09FA), W(0x096B), W(0x08E3), W(0x0864), W(0x07EB), W(0x0779),
    W(0x070D), W(0x06A9), W(0x0649), W(0x05EF), W(0x0599), W(0x0549), W(0x04FD), W(0x04B5),
    W(0x0472), W(0x0432), W(0x03F6), W(0x03BD), W(0x0387), W(0x0354), W(0x0324), W(0x02F7),
    W(0x02CC), W(0x02A4), W(0x027E), W(0x025B), W(0x0239), W(0x0219), W(0x01FB), W(0x01DE),
    W(0x01C3), W(0x01AA), W(0x0192), W(0x017C), W(0x0166), W(0x0152), W(0x013F), W(0x012D),
    W(0x011C), W(0x010C), W(0x00FD), W(0x00EF), W(0x00E2), W(0x00D5), W(0x00C9), W(0x00BE),
};
/* 0760-078F periodi del tono (byte alto 0) per indice 30-5F */
static const uint8_t tab_periodi_toni_b[] = {
    B(0xB3), B(0xA9), B(0xA0), B(0x97), B(0x8E), B(0x86), B(0x7F), B(0x78), B(0x71), B(0x6B),
    B(0x65), B(0x5F), B(0x5A), B(0x55), B(0x50), B(0x4B), B(0x47), B(0x43), B(0x3F), B(0x3C),
    B(0x38), B(0x35), B(0x32), B(0x2F), B(0x2D), B(0x2A), B(0x28), B(0x25), B(0x24), B(0x22),
    B(0x20), B(0x1E), B(0x1C), B(0x1B), B(0x19), B(0x18), B(0x16), B(0x15), B(0x14), B(0x13),
    B(0x12), B(0x11), B(0x10), B(0x0F), B(0x0E), B(0x0D), B(0x0C), B(0x0C),
};
/* 0790-084F durate in IRQ (2 per frame) dei byte DUR(0)-DUR(95) */
static const uint8_t tab_durate[] = {
    W(0x0001), W(0x0002), W(0x0003), W(0x0004), W(0x0005), W(0x0006), W(0x0007), W(0x0008),
    W(0x0009), W(0x000A), W(0x000B), W(0x000C), W(0x000D), W(0x000E), W(0x000F), W(0x0010),
    W(0x0011), W(0x0012), W(0x0013), W(0x0014), W(0x0015), W(0x0016), W(0x0017), W(0x0018),
    W(0x0019), W(0x001A), W(0x001B), W(0x001C), W(0x001D), W(0x001E), W(0x001F), W(0x0020),
    W(0x0021), W(0x0022), W(0x0023), W(0x0024), W(0x0025), W(0x0026), W(0x0027), W(0x0028),
    W(0x0029), W(0x002A), W(0x002B), W(0x002C), W(0x002D), W(0x002E), W(0x002F), W(0x0030),
    W(0x0031), W(0x0032), W(0x0033), W(0x0034), W(0x0035), W(0x0036), W(0x0037), W(0x0038),
    W(0x0039), W(0x003A), W(0x003B), W(0x003C), W(0x003D), W(0x003E), W(0x003F), W(0x0040),
    W(0x0041), W(0x0042), W(0x0043), W(0x0044), W(0x0045), W(0x0046), W(0x0047), W(0x0048),
    W(0x0049), W(0x004A), W(0x004B), W(0x004C), W(0x004D), W(0x004E), W(0x004F), W(0x0050),
    W(0x0010), W(0x0020), W(0x0030), W(0x0040), W(0x0050), W(0x0060), W(0x0070), W(0x0080),
    W(0x0090), W(0x00A0), W(0x00B0), W(0x00C0), W(0x00D0), W(0x00E0), W(0x00F0), W(0x0100),
};
/* 0850-086F inizi degli inviluppi software STRUM(0)-STRUM(15) */
static const uint8_t tab_inviluppi[] = {
    W(ADR_inviluppo_00), W(ADR_inviluppo_01), W(ADR_inviluppo_02), W(ADR_inviluppo_03),
    W(ADR_inviluppo_04), W(ADR_inviluppo_05), W(ADR_inviluppo_06), W(ADR_inviluppo_07),
    W(ADR_inviluppo_08), W(ADR_inviluppo_09), W(ADR_inviluppo_10), W(ADR_inviluppo_11),
    W(ADR_inviluppo_12), W(ADR_inviluppo_13), W(ADR_inviluppo_14), W(ADR_inviluppo_15),
};
/* 0870-0897 inviluppo software 0: coppie (volume, IRQ) */
static const uint8_t inviluppo_00[] = {
    PASSO(15, 4), PASSO(14, 4), PASSO(13, 4), PASSO(12, 4), PASSO(11, 4), PASSO(10, 4),
    PASSO(9, 4), PASSO(8, 4), PASSO(7, 4), PASSO(6, 4), PASSO(5, 4), PASSO(4, 4), PASSO(3, 4),
    PASSO(2, 4), PASSO(1, 4), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255),
    PASSO(0, 255),
};
/* 0898-08BF inviluppo software 1: coppie (volume, IRQ) */
static const uint8_t inviluppo_01[] = {
    PASSO(15, 1), PASSO(14, 2), PASSO(13, 3), PASSO(12, 4), PASSO(11, 5), PASSO(10, 6),
    PASSO(9, 7), PASSO(8, 8), PASSO(7, 9), PASSO(6, 10), PASSO(5, 11), PASSO(4, 12),
    PASSO(3, 13), PASSO(2, 14), PASSO(1, 15), PASSO(0, 16), PASSO(0, 255), PASSO(0, 255),
    PASSO(0, 255), PASSO(0, 255),
};
/* 08C0-08C2 inviluppo software 2: coppie (volume, IRQ) */
static const uint8_t inviluppo_02[] = {
    PASSO(15, 255), RIPARTI,
};
/* 08C3-08EA inviluppo software 3: coppie (volume, IRQ) */
static const uint8_t inviluppo_03[] = {
    PASSO(15, 2), PASSO(14, 2), PASSO(13, 2), PASSO(12, 2), PASSO(11, 1), PASSO(10, 1),
    PASSO(9, 1), PASSO(8, 1), PASSO(7, 1), PASSO(6, 1), PASSO(5, 1), PASSO(4, 1), PASSO(3, 1),
    PASSO(2, 1), PASSO(1, 1), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255),
    PASSO(0, 255),
};
/* 08EB-0912 inviluppo software 4: coppie (volume, IRQ) */
static const uint8_t inviluppo_04[] = {
    PASSO(15, 4), PASSO(14, 4), PASSO(13, 5), PASSO(12, 5), PASSO(11, 6), PASSO(10, 6),
    PASSO(9, 7), PASSO(8, 7), PASSO(7, 8), PASSO(6, 8), PASSO(5, 9), PASSO(4, 9), PASSO(3, 10),
    PASSO(2, 10), PASSO(1, 11), PASSO(0, 11), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255),
    PASSO(0, 255),
};
/* 0913-0932 inviluppo software 5: coppie (volume, IRQ) */
static const uint8_t inviluppo_05[] = {
    PASSO(15, 1), PASSO(14, 1), PASSO(13, 1), PASSO(12, 1), PASSO(11, 1), PASSO(10, 1),
    PASSO(9, 1), PASSO(8, 1), PASSO(7, 1), PASSO(6, 1), PASSO(5, 1), PASSO(4, 1), PASSO(3, 1),
    PASSO(2, 1), PASSO(1, 1), PASSO(0, 255),
};
/* 0933-094A inviluppo software 6: coppie (volume, IRQ) */
static const uint8_t inviluppo_06[] = {
    PASSO(15, 1), PASSO(13, 1), PASSO(11, 1), PASSO(9, 1), PASSO(7, 1), PASSO(5, 1),
    PASSO(3, 1), PASSO(1, 1), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255),
};
/* 094B-0972 inviluppo software 7: coppie (volume, IRQ) */
static const uint8_t inviluppo_07[] = {
    PASSO(13, 1), PASSO(14, 1), PASSO(15, 4), PASSO(14, 4), PASSO(13, 4), PASSO(12, 4),
    PASSO(11, 4), PASSO(10, 4), PASSO(9, 4), PASSO(8, 4), PASSO(7, 4), PASSO(6, 4), PASSO(5, 4),
    PASSO(4, 4), PASSO(3, 4), PASSO(2, 4), PASSO(1, 4), PASSO(0, 255), PASSO(0, 255),
    PASSO(0, 255),
};
/* 0973-099A inviluppo software 8: coppie (volume, IRQ) */
static const uint8_t inviluppo_08[] = {
    PASSO(10, 1), PASSO(11, 1), PASSO(12, 1), PASSO(13, 1), PASSO(14, 5), PASSO(13, 5),
    PASSO(12, 5), PASSO(11, 5), PASSO(10, 5), PASSO(9, 5), PASSO(8, 5), PASSO(7, 5),
    PASSO(6, 5), PASSO(5, 5), PASSO(4, 5), PASSO(3, 5), PASSO(2, 5), PASSO(1, 5), PASSO(0, 255),
    PASSO(0, 255),
};
/* 099B-09BA inviluppo software 9: coppie (volume, IRQ) */
static const uint8_t inviluppo_09[] = {
    PASSO(15, 1), PASSO(14, 1), PASSO(13, 2), PASSO(12, 2), PASSO(11, 3), PASSO(10, 3),
    PASSO(9, 4), PASSO(8, 4), PASSO(7, 5), PASSO(6, 5), PASSO(5, 6), PASSO(4, 6), PASSO(3, 7),
    PASSO(2, 7), PASSO(1, 8), PASSO(0, 255),
};
/* 09BB-09DA inviluppo software 10: coppie (volume, IRQ) */
static const uint8_t inviluppo_10[] = {
    PASSO(15, 3), PASSO(14, 4), PASSO(13, 5), PASSO(12, 6), PASSO(11, 7), PASSO(10, 8),
    PASSO(9, 9), PASSO(8, 10), PASSO(7, 11), PASSO(6, 12), PASSO(5, 13), PASSO(4, 14),
    PASSO(3, 15), PASSO(2, 16), PASSO(1, 17), PASSO(0, 255),
};
/* 09DB-0A02 inviluppo software 11: coppie (volume, IRQ) */
static const uint8_t inviluppo_11[] = {
    PASSO(15, 2), PASSO(14, 2), PASSO(13, 2), PASSO(12, 2), PASSO(11, 2), PASSO(10, 2),
    PASSO(9, 2), PASSO(8, 2), PASSO(7, 2), PASSO(6, 2), PASSO(5, 2), PASSO(4, 2), PASSO(3, 2),
    PASSO(2, 2), PASSO(1, 2), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255),
    PASSO(0, 255),
};
/* 0A03-0A2A inviluppo software 12: coppie (volume, IRQ) */
static const uint8_t inviluppo_12[] = {
    PASSO(15, 8), PASSO(14, 8), PASSO(13, 8), PASSO(12, 8), PASSO(11, 8), PASSO(10, 8),
    PASSO(9, 8), PASSO(8, 8), PASSO(7, 8), PASSO(6, 8), PASSO(5, 8), PASSO(4, 8), PASSO(3, 8),
    PASSO(2, 8), PASSO(1, 8), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255),
    PASSO(0, 255),
};
/* 0A2B-0A6A inviluppo software 13: coppie (volume, IRQ) */
static const uint8_t inviluppo_13[] = {
    PASSO(1, 1), PASSO(2, 2), PASSO(3, 3), PASSO(4, 4), PASSO(5, 5), PASSO(6, 6), PASSO(7, 7),
    PASSO(8, 8), PASSO(9, 9), PASSO(10, 10), PASSO(11, 11), PASSO(12, 12), PASSO(13, 13),
    PASSO(14, 14), PASSO(15, 1), PASSO(14, 1), PASSO(13, 1), PASSO(12, 1), PASSO(11, 1),
    PASSO(10, 1), PASSO(9, 1), PASSO(8, 1), PASSO(7, 1), PASSO(6, 1), PASSO(5, 1), PASSO(4, 1),
    PASSO(3, 1), PASSO(2, 1), PASSO(1, 1), PASSO(0, 255), PASSO(0, 255), PASSO(0, 255),
};
/* 0A6B-0A92 inviluppo software 14: coppie (volume, IRQ) */
static const uint8_t inviluppo_14[] = {
    PASSO(10, 1), PASSO(11, 1), PASSO(12, 1), PASSO(13, 1), PASSO(14, 9), PASSO(13, 9),
    PASSO(12, 9), PASSO(11, 9), PASSO(10, 9), PASSO(9, 9), PASSO(8, 9), PASSO(7, 9),
    PASSO(6, 9), PASSO(5, 9), PASSO(4, 9), PASSO(3, 9), PASSO(2, 9), PASSO(1, 9), PASSO(0, 255),
    PASSO(0, 255),
};
/* 0A93-0AF2 inviluppo software 15: coppie (volume, IRQ) */
static const uint8_t inviluppo_15[] = {
    PASSO(14, 8), PASSO(13, 8), PASSO(12, 8), PASSO(11, 8), PASSO(13, 8), PASSO(12, 8),
    PASSO(11, 8), PASSO(10, 8), PASSO(12, 8), PASSO(11, 8), PASSO(10, 8), PASSO(9, 8),
    PASSO(11, 8), PASSO(10, 8), PASSO(9, 8), PASSO(8, 8), PASSO(10, 8), PASSO(9, 8),
    PASSO(8, 8), PASSO(7, 8), PASSO(9, 8), PASSO(8, 8), PASSO(7, 8), PASSO(6, 8), PASSO(8, 8),
    PASSO(7, 8), PASSO(6, 8), PASSO(5, 8), PASSO(7, 8), PASSO(6, 8), PASSO(5, 8), PASSO(4, 8),
    PASSO(6, 8), PASSO(5, 8), PASSO(4, 8), PASSO(3, 8), PASSO(5, 8), PASSO(4, 8), PASSO(3, 8),
    PASSO(2, 8), PASSO(4, 8), PASSO(3, 8), PASSO(2, 8), PASSO(1, 8), PASSO(0, 255),
    PASSO(0, 255), PASSO(0, 255), PASSO(0, 255),
};
/* 0AF3-0B72 tabella dei comandi: elenco per latch 01-3F (00 = arresto) */
static const uint8_t tab_comandi[] = {
    W(0x0000),
    W(ADR_cmd_01), W(ADR_cmd_02), W(ADR_cmd_03), W(ADR_cmd_04), W(ADR_cmd_05), W(ADR_cmd_06),
    W(ADR_cmd_07), W(ADR_cmd_08), W(ADR_cmd_09), W(ADR_cmd_0A), W(ADR_cmd_0B), W(ADR_cmd_0C),
    W(ADR_cmd_0D), W(ADR_cmd_0E), W(ADR_cmd_0F), W(ADR_cmd_10), W(ADR_cmd_11), W(ADR_cmd_12),
    W(ADR_cmd_13), W(ADR_cmd_14), W(ADR_cmd_15), W(ADR_cmd_16), W(ADR_cmd_17), W(ADR_cmd_18),
    W(ADR_cmd_19), W(ADR_cmd_1A), W(ADR_cmd_1B), W(ADR_cmd_1C), W(ADR_cmd_1D), W(ADR_cmd_1E),
    W(ADR_cmd_1F), W(ADR_cmd_20), W(ADR_cmd_21), W(ADR_cmd_22), W(ADR_cmd_23), W(ADR_cmd_24),
    W(ADR_cmd_25), W(ADR_cmd_26), W(ADR_cmd_27), W(ADR_cmd_28), W(ADR_cmd_29), W(ADR_cmd_2A),
    W(ADR_cmd_2B), W(ADR_cmd_2C), W(ADR_cmd_2D), W(ADR_cmd_2E), W(ADR_cmd_2F), W(ADR_cmd_30),
    W(ADR_cmd_31), W(ADR_cmd_32), W(ADR_cmd_33), W(ADR_cmd_34), W(ADR_cmd_35), W(ADR_cmd_36),
    W(ADR_cmd_37), W(ADR_cmd_38), W(ADR_cmd_39), W(ADR_cmd_3A), W(ADR_cmd_3B), W(ADR_cmd_3C),
    W(ADR_cmd_3D), W(ADR_cmd_3E), W(ADR_cmd_3F),
};
/* 3811-382C residuo mai collegato: elenco dei record */
static const uint8_t residuo_3811[] = {
    REC(0, ADR_residuo_3811_b0_s1), REC(1, ADR_residuo_3811_b1_s1),
    REC(2, ADR_residuo_3811_b2_s1), REC(3, ADR_residuo_3811_b3_s1),
    REC(4, ADR_residuo_3811_b4_s1), REC(5, ADR_residuo_3811_b5_s1),
    REC(6, ADR_residuo_3811_b6_s1), REC(7, ADR_residuo_3811_b7_s1),
    REC(8, ADR_residuo_3811_b8_s1), FINE_ELENCO,
};
/* 382D-3837 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t residuo_3811_b0_s1[] = {
    STRUM(8), ATTEN(2), DUR(47), NOTA(4, LA), NOTA(3, LA), SALTA(ADR_residuo_3811_b0_s1),
    B(0xFF),
};
/* 3838-383C flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t residuo_3811_b1_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 383D-3841 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t residuo_3811_b2_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3842-3846 flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t residuo_3811_b3_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3847-384B flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t residuo_3811_b4_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 384C-3850 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t residuo_3811_b5_s1[] = {
    STRUM(0), NOTA(0, SOLd), PAUSA, FINE,
};
/* 3851-3855 flusso, blocco 6, strato 1 (chip 3 voce 3) */
static const uint8_t residuo_3811_b6_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3856-385A flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t residuo_3811_b7_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 385B-385E flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t residuo_3811_b8_s1[] = {
    STRUM(0), DUR(0), PAUSA,
};

const struct aud_pezzo aud_pezzi_tabelle[] = {
    RIEMPIMENTO(riempimento_05C6, 0x013A),
    PEZZO(tab_periodi_toni),
    PEZZO(tab_periodi_toni_b),
    PEZZO(tab_durate),
    PEZZO(tab_inviluppi),
    PEZZO(inviluppo_00),
    PEZZO(inviluppo_01),
    PEZZO(inviluppo_02),
    PEZZO(inviluppo_03),
    PEZZO(inviluppo_04),
    PEZZO(inviluppo_05),
    PEZZO(inviluppo_06),
    PEZZO(inviluppo_07),
    PEZZO(inviluppo_08),
    PEZZO(inviluppo_09),
    PEZZO(inviluppo_10),
    PEZZO(inviluppo_11),
    PEZZO(inviluppo_12),
    PEZZO(inviluppo_13),
    PEZZO(inviluppo_14),
    PEZZO(inviluppo_15),
    PEZZO(tab_comandi),
    PEZZO(residuo_3811),
    PEZZO(residuo_3811_b0_s1),
    PEZZO(residuo_3811_b1_s1),
    PEZZO(residuo_3811_b2_s1),
    PEZZO(residuo_3811_b3_s1),
    PEZZO(residuo_3811_b4_s1),
    PEZZO(residuo_3811_b5_s1),
    PEZZO(residuo_3811_b6_s1),
    PEZZO(residuo_3811_b7_s1),
    PEZZO(residuo_3811_b8_s1),
    RIEMPIMENTO(riempimento_385F, 0x07A1),
};
const unsigned aud_n_pezzi_tabelle =
    sizeof aud_pezzi_tabelle / sizeof aud_pezzi_tabelle[0];
