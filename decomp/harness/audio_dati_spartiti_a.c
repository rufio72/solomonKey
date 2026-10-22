/* Generato da decomp/estrai_dati.py dalla ROM audio 1.3jk: non modificare
 * a mano. Dati musicali derivati dal programma Tecmo (NOTICE.md). */
/* Elenchi e flussi dei comandi 01-12. */
#include <stddef.h>
#include "audio_dati.h"

/* 0B73-0B79 comando 01: elenco dei record (blocco, flusso) */
static const uint8_t cmd_01[] = {
    REC(9, ADR_cmd_01_b9_s2), REC(12, ADR_cmd_01_b12_s2), FINE_ELENCO,
};
/* 0B7A-0B85 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_01_b9_s2[] = {
    STRUM(2), ATTEN(1), GLISSA(0x00C0, 0x0080, 8), FINE,
};
/* 0B86-0B91 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_01_b12_s2[] = {
    STRUM(2), ATTEN(2), GLISSA(0x0030, 0x0020, 8), FINE,
};
/* 0B92-0B98 comando 02: elenco dei record (blocco, flusso) */
static const uint8_t cmd_02[] = {
    REC(9, ADR_cmd_02_b9_s2), REC(12, ADR_cmd_02_b12_s2), FINE_ELENCO,
};
/* 0B99-0C14 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_02_b9_s2[] = {
    STRUM(2), DUR(0), ATTEN(0), RUMORE(15), NOTA(0, DO), RUMORE(14), NOTA(0, DO), ATTEN(1),
    RUMORE(15), NOTA(0, DO), RUMORE(14), NOTA(0, DO), ATTEN(2), RUMORE(15), NOTA(0, DO),
    RUMORE(14), NOTA(0, DO), ATTEN(3), RUMORE(15), NOTA(0, DO), RUMORE(14), NOTA(0, DO),
    ATTEN(4), RUMORE(15), NOTA(0, DO), RUMORE(14), NOTA(0, DO), ATTEN(5), RUMORE(15),
    NOTA(0, DO), RUMORE(14), NOTA(0, DO), ATTEN(6), RUMORE(15), NOTA(0, DO), RUMORE(14),
    NOTA(0, DO), ATTEN(7), RUMORE(15), NOTA(0, DO), RUMORE(14), NOTA(0, DO), ATTEN(8),
    RUMORE(15), NOTA(0, DO), RUMORE(14), NOTA(0, DO), ATTEN(9), RUMORE(15), NOTA(0, DO),
    RUMORE(14), NOTA(0, DO), ATTEN(10), RUMORE(15), NOTA(0, DO), RUMORE(14), NOTA(0, DO),
    ATTEN(11), RUMORE(15), NOTA(0, DO), RUMORE(14), NOTA(0, DO), ATTEN(12), RUMORE(15),
    NOTA(0, DO), RUMORE(14), NOTA(0, DO), ATTEN(13), RUMORE(15), NOTA(0, DO), RUMORE(14),
    NOTA(0, DO), ATTEN(14), RUMORE(15), NOTA(0, DO), RUMORE(14), NOTA(0, DO), FINE,
};
/* 0C15-0C90 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_02_b12_s2[] = {
    STRUM(2), DUR(0), ATTEN(0), RUMORE(14), NOTA(0, DO), RUMORE(13), NOTA(0, DO), ATTEN(1),
    RUMORE(14), NOTA(0, DO), RUMORE(13), NOTA(0, DO), ATTEN(2), RUMORE(14), NOTA(0, DO),
    RUMORE(13), NOTA(0, DO), ATTEN(3), RUMORE(14), NOTA(0, DO), RUMORE(13), NOTA(0, DO),
    ATTEN(4), RUMORE(14), NOTA(0, DO), RUMORE(13), NOTA(0, DO), ATTEN(5), RUMORE(14),
    NOTA(0, DO), RUMORE(13), NOTA(0, DO), ATTEN(6), RUMORE(14), NOTA(0, DO), RUMORE(13),
    NOTA(0, DO), ATTEN(7), RUMORE(14), NOTA(0, DO), RUMORE(13), NOTA(0, DO), ATTEN(8),
    RUMORE(14), NOTA(0, DO), RUMORE(13), NOTA(0, DO), ATTEN(9), RUMORE(14), NOTA(0, DO),
    RUMORE(13), NOTA(0, DO), ATTEN(10), RUMORE(14), NOTA(0, DO), RUMORE(13), NOTA(0, DO),
    ATTEN(11), RUMORE(14), NOTA(0, DO), RUMORE(13), NOTA(0, DO), ATTEN(12), RUMORE(14),
    NOTA(0, DO), RUMORE(13), NOTA(0, DO), ATTEN(13), RUMORE(14), NOTA(0, DO), RUMORE(13),
    NOTA(0, DO), ATTEN(14), RUMORE(14), NOTA(0, DO), RUMORE(13), NOTA(0, DO), FINE,
};
/* 0C91-0C94 comando 03: elenco dei record (blocco, flusso) */
static const uint8_t cmd_03[] = {
    REC(0, ADR_cmd_03_b0_s1), FINE_ELENCO,
};
/* 0C95-0C99 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_03_b0_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0C9A-0C9D comando 04: elenco dei record (blocco, flusso) */
static const uint8_t cmd_04[] = {
    REC(0, ADR_cmd_04_b0_s1), FINE_ELENCO,
};
/* 0C9E-0CA2 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_04_b0_s1[] = {
    STRUM(2), DUR(0), PAUSA, FINE,
};
/* 0CA3-0CA6 comando 05: elenco dei record (blocco, flusso) */
static const uint8_t cmd_05[] = {
    REC(0, ADR_cmd_05_b0_s1), FINE_ELENCO,
};
/* 0CA7-0CAB flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_05_b0_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0CAC-0CAF comando 06: elenco dei record (blocco, flusso) */
static const uint8_t cmd_06[] = {
    REC(9, ADR_cmd_06_b9_s2), FINE_ELENCO,
};
/* 0CB0-0CC2 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_06_b9_s2[] = {
    STRUM(2), ATTEN(1), DUR(0), RUMORE(16), NOTA(0, DO), ATTEN(4), DUR(0), RUMORE(17),
    NOTA(0, DO), DUR(1), RUMORE(18), NOTA(0, DO), FINE,
};
/* 0CC3-0CC6 comando 07: elenco dei record (blocco, flusso) */
static const uint8_t cmd_07[] = {
    REC(9, ADR_cmd_07_b9_s2), FINE_ELENCO,
};
/* 0CC7-0CCF flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_07_b9_s2[] = {
    STRUM(3), ATTEN(0), DUR(15), RUMORE(30), NOTA(0, DO), FINE,
};
/* 0CD0-0CD6 comando 08: elenco dei record (blocco, flusso) */
static const uint8_t cmd_08[] = {
    REC(9, ADR_cmd_08_b9_s2), REC(12, ADR_cmd_08_b12_s2), FINE_ELENCO,
};
/* 0CD7-0CDD flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_08_b9_s2[] = {
    STRUM(5), ATTEN(0), DUR(15), NOTA(1, DO), FINE,
};
/* 0CDE-0CE4 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_08_b12_s2[] = {
    STRUM(5), ATTEN(0), DUR(15), NOTA(5, DOd), FINE,
};
/* 0CE5-0CEE comando 09: elenco dei record (blocco, flusso) */
static const uint8_t cmd_09[] = {
    REC(9, ADR_cmd_09_b9_s2), REC(12, ADR_cmd_09_b12_s2), REC(15, ADR_cmd_09_b15_s2),
    FINE_ELENCO,
};
/* 0CEF-0D1F flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_09_b9_s2[] = {
    STRUM(2), DUR(0), ATTEN(6), PERIODO(0x0020), ATTEN(7), PERIODO(0x0021), ATTEN(8),
    PERIODO(0x0020), ATTEN(9), PERIODO(0x0021), ATTEN(10), PERIODO(0x0020), ATTEN(11),
    PERIODO(0x0021), ATTEN(12), PERIODO(0x0020), ATTEN(13), PERIODO(0x0021), ATTEN(14),
    PERIODO(0x0020), FINE,
};
/* 0D20-0D26 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_09_b12_s2[] = {
    STRUM(3), ATTEN(1), DUR(15), NOTA(5, DO), FINE,
};
/* 0D27-0D2D flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_09_b15_s2[] = {
    STRUM(3), ATTEN(1), DUR(15), NOTA(3, DO), FINE,
};
/* 0D2E-0D64 comando 0A: elenco dei record (blocco, flusso) */
static const uint8_t cmd_0A[] = {
    REC(0, ADR_cmd_0A_b0_s1), REC(1, ADR_cmd_0A_b1_s1), REC(2, ADR_cmd_0A_b2_s1),
    REC(3, ADR_cmd_0A_b3_s1), REC(4, ADR_cmd_0A_b4_s1), REC(5, ADR_cmd_0A_b5_s1),
    REC(6, ADR_cmd_0A_b6_s1), REC(7, ADR_cmd_0A_b7_s1), REC(8, ADR_cmd_0A_b8_s1),
    REC(9, ADR_cmd_0A_b9_s2), REC(10, ADR_cmd_0A_b10_s2), REC(11, ADR_cmd_0A_b11_s2),
    REC(12, ADR_cmd_0A_b12_s2), REC(13, ADR_cmd_0A_b13_s2), REC(14, ADR_cmd_0A_b14_s2),
    REC(15, ADR_cmd_0A_b15_s2), REC(16, ADR_cmd_0A_b16_s2), REC(17, ADR_cmd_0A_b17_s2),
    FINE_ELENCO,
};
/* 0D65-0D69 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_0A_b0_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0D6A-0D6E flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t cmd_0A_b1_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0D6F-0D73 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_0A_b2_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0D74-0D78 flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t cmd_0A_b3_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0D79-0D7D flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t cmd_0A_b4_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0D7E-0D82 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_0A_b5_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0D83-0D87 flusso, blocco 6, strato 1 (chip 3 voce 3) */
static const uint8_t cmd_0A_b6_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0D88-0D8C flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_0A_b7_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0D8D-0D91 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_0A_b8_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0D92-0DA0 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_0A_b9_s2[] = {
    STRUM(2), DUR(2), STRUM(0), NOTA(4, LAd), NOTA(4, SI), DUR(13), ATTEN(1), NOTA(5, DO),
    ATTEN(2), NOTA(3, DO), FINE,
};
/* 0DA1-0DAF flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_0A_b10_s2[] = {
    STRUM(2), DUR(2), STRUM(0), NOTA(4, SI), NOTA(5, DO), DUR(13), ATTEN(1), NOTA(5, DOd),
    ATTEN(2), NOTA(3, DOd), FINE,
};
/* 0DB0-0DBE flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_0A_b11_s2[] = {
    STRUM(2), DUR(2), STRUM(0), NOTA(5, DO), NOTA(5, DOd), DUR(13), ATTEN(1), NOTA(5, RE),
    ATTEN(2), NOTA(3, RE), FINE,
};
/* 0DBF-0DCD flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_0A_b12_s2[] = {
    STRUM(2), DUR(2), STRUM(0), NOTA(5, DOd), NOTA(5, RE), DUR(13), ATTEN(1), NOTA(5, REd),
    ATTEN(2), NOTA(3, REd), FINE,
};
/* 0DCE-0DDC flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_0A_b13_s2[] = {
    STRUM(2), DUR(2), STRUM(0), NOTA(5, RE), NOTA(5, REd), DUR(13), ATTEN(1), NOTA(5, MI),
    ATTEN(2), NOTA(3, MI), FINE,
};
/* 0DDD-0DEB flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_0A_b14_s2[] = {
    STRUM(2), DUR(2), STRUM(0), NOTA(5, REd), NOTA(5, MI), DUR(13), ATTEN(1), NOTA(5, FA),
    ATTEN(2), NOTA(3, FA), FINE,
};
/* 0DEC-0DFA flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_0A_b15_s2[] = {
    STRUM(2), DUR(2), STRUM(0), NOTA(5, MI), NOTA(5, FA), DUR(13), ATTEN(1), NOTA(5, FAd),
    ATTEN(2), NOTA(3, FAd), FINE,
};
/* 0DFB-0E09 flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_0A_b16_s2[] = {
    STRUM(2), DUR(2), STRUM(0), NOTA(5, FA), NOTA(5, FAd), DUR(13), ATTEN(1), NOTA(5, SOL),
    ATTEN(2), NOTA(3, SOL), FINE,
};
/* 0E0A-0E0E flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_0A_b17_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0E0F-0E2A comando 0B: elenco dei record (blocco, flusso) */
static const uint8_t cmd_0B[] = {
    REC(9, ADR_cmd_0B_b9_s2), REC(10, ADR_cmd_0B_b10_s2), REC(11, ADR_cmd_0B_b11_s2),
    REC(12, ADR_cmd_0B_b12_s2), REC(13, ADR_cmd_0B_b13_s2), REC(14, ADR_cmd_0B_b14_s2),
    REC(15, ADR_cmd_0B_b15_s2), REC(16, ADR_cmd_0B_b16_s2), REC(17, ADR_cmd_0B_b17_s2),
    FINE_ELENCO,
};
/* 0E2B-0E57 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_0B_b9_s2[] = {
    STRUM(2), ATTEN(2), GLISSA(0x0140, 0x0150, 16), ATTEN(2), DUR(7), PERIODO(0x00C0), DUR(9),
    PERIODO(0x00A0), ATTEN(2), GLISSA(0x00E0, 0x0100, 32), STRUM(6), ATTEN(2), DUR(8),
    RIP_INIZIO(4), PERIODO(0x0060), PERIODO(0x0080), RIP_FINE,
    FINE,
};
/* 0E58-0E84 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_0B_b10_s2[] = {
    STRUM(2), ATTEN(2), GLISSA(0x0142, 0x0152, 16), ATTEN(3), DUR(7), PERIODO(0x00C2), DUR(9),
    PERIODO(0x00A2), ATTEN(4), GLISSA(0x00E2, 0x0102, 32), STRUM(6), ATTEN(2), DUR(8),
    RIP_INIZIO(4), PERIODO(0x0062), PERIODO(0x0082), RIP_FINE,
    FINE,
};
/* 0E85-0EAF flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_0B_b11_s2[] = {
    STRUM(2), ATTEN(4), GLISSA(0x0144, 0x0154, 16), ATTEN(5), DUR(7), PERIODO(0x00C4), DUR(9),
    PERIODO(0x00A4), ATTEN(5), GLISSA(0x00E4, 0x0104, 32), ATTEN(4), DUR(8), RIP_INIZIO(4),
    PERIODO(0x0064), PERIODO(0x0084), RIP_FINE,
    FINE,
};
/* 0EB0-0EB4 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_0B_b12_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0EB5-0EB9 flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_0B_b13_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0EBA-0EBE flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_0B_b14_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0EBF-0EC3 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_0B_b15_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0EC4-0EC8 flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_0B_b16_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0EC9-0ECD flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_0B_b17_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0ECE-0ED1 comando 0C: elenco dei record (blocco, flusso) */
static const uint8_t cmd_0C[] = {
    REC(9, ADR_cmd_0C_b9_s2), FINE_ELENCO,
};
/* 0ED2-0EDD flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_0C_b9_s2[] = {
    STRUM(2), ATTEN(5), GLISSA(0x0010, 0x0190, 384), FINE,
};
/* 0EDE-0EE4 comando 0D: elenco dei record (blocco, flusso) */
static const uint8_t cmd_0D[] = {
    REC(10, ADR_cmd_0D_b10_s2), REC(13, ADR_cmd_0D_b13_s2), FINE_ELENCO,
};
/* 0EE5-0EF0 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_0D_b10_s2[] = {
    STRUM(2), ATTEN(2), GLISSA(0x0030, 0x0431, 1024), FINE,
};
/* 0EF1-0EFC flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_0D_b13_s2[] = {
    STRUM(2), ATTEN(2), GLISSA(0x0070, 0x0471, 1024), FINE,
};
/* 0EFD-0F06 comando 0E: elenco dei record (blocco, flusso) */
static const uint8_t cmd_0E[] = {
    REC(10, ADR_cmd_0E_b10_s2), REC(13, ADR_cmd_0E_b13_s2), REC(10, ADR_cmd_0E_b10_s2_r2),
    FINE_ELENCO,
};
/* 0F07-0F0B flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_0E_b10_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0F0C-0F10 flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_0E_b13_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0F11-0F15 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_0E_b10_s2_r2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 0F16-0F1F comando 0F: elenco dei record (blocco, flusso) */
static const uint8_t cmd_0F[] = {
    REC(10, ADR_cmd_0F_b10_s2), REC(13, ADR_cmd_0F_b13_s2), REC(15, ADR_cmd_0F_b15_s2),
    FINE_ELENCO,
};
/* 0F20-0F2B flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_0F_b10_s2[] = {
    STRUM(2), ATTEN(2), GLISSA(0x0060, 0x0461, 1024), FINE,
};
/* 0F2C-0F37 flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_0F_b13_s2[] = {
    STRUM(2), ATTEN(2), GLISSA(0x00A0, 0x04A1, 1024), FINE,
};
/* 0F38-0F43 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_0F_b15_s2[] = {
    STRUM(2), ATTEN(2), GLISSA(0x00E0, 0x04E0, 1024), FINE,
};
/* 0F44-0F4A comando 10: elenco dei record (blocco, flusso) */
static const uint8_t cmd_10[] = {
    REC(9, ADR_cmd_10_b9_s2), REC(12, ADR_cmd_10_b12_s2), FINE_ELENCO,
};
/* 0F4B-0F5F flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_10_b9_s2[] = {
    STRUM(7), DUR(6), ATTEN(1), NOTA(4, FA), ATTEN(2), NOTA(4, SOL), ATTEN(3), NOTA(4, LA),
    ATTEN(4), NOTA(4, SI), ATTEN(5), NOTA(5, DOd), DUR(55), NOTA(5, REd), FINE,
};
/* 0F60-0F74 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_10_b12_s2[] = {
    STRUM(7), DUR(6), ATTEN(3), NOTA(3, FA), ATTEN(4), NOTA(3, SOL), ATTEN(5), NOTA(3, LA),
    ATTEN(6), NOTA(3, SI), ATTEN(7), NOTA(4, DOd), DUR(55), NOTA(6, REd), FINE,
};
/* 0F75-0F90 comando 11: elenco dei record (blocco, flusso) */
static const uint8_t cmd_11[] = {
    REC(9, ADR_cmd_11_b9_s2), REC(10, ADR_cmd_11_b10_s2), REC(11, ADR_cmd_11_b11_s2),
    REC(12, ADR_cmd_11_b12_s2), REC(13, ADR_cmd_11_b13_s2), REC(14, ADR_cmd_11_b14_s2),
    REC(15, ADR_cmd_11_b15_s2), REC(16, ADR_cmd_11_b16_s2), REC(17, ADR_cmd_11_b17_s2),
    FINE_ELENCO,
};
/* 0F91-0FA3 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_11_b9_s2[] = {
    STRUM(7), ATTEN(4), TRASP(0), DUR(27), NOTA(3, RE), DUR(13), NOTA(3, REd), STRUM(2),
    DUR(31), NOTA(3, FA), STRUM(0), DUR(47), NOTA(3, FA), FINE,
};
/* 0FA4-0FB6 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_11_b10_s2[] = {
    STRUM(7), ATTEN(4), TRASP(0), DUR(27), NOTA(3, FAd), DUR(13), NOTA(3, SOL), STRUM(2),
    DUR(31), NOTA(3, LA), STRUM(0), DUR(47), NOTA(3, LA), FINE,
};
/* 0FB7-0FC9 flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_11_b11_s2[] = {
    STRUM(7), ATTEN(3), TRASP(0), DUR(27), NOTA(3, LA), DUR(13), NOTA(3, LAd), STRUM(2),
    DUR(31), NOTA(4, DO), STRUM(0), DUR(47), NOTA(4, DO), FINE,
};
/* 0FCA-0FDC flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_11_b12_s2[] = {
    STRUM(7), ATTEN(4), TRASP(0), DUR(27), NOTA(4, RE), DUR(13), NOTA(4, REd), STRUM(2),
    DUR(31), NOTA(4, FA), STRUM(0), DUR(47), NOTA(4, FA), FINE,
};
/* 0FDD-0FEF flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_11_b13_s2[] = {
    STRUM(7), ATTEN(4), TRASP(0), DUR(27), NOTA(4, FAd), DUR(13), NOTA(4, SOL), STRUM(2),
    DUR(31), NOTA(4, LA), STRUM(0), DUR(47), NOTA(4, LA), FINE,
};
/* 0FF0-1002 flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_11_b14_s2[] = {
    STRUM(7), ATTEN(3), TRASP(0), DUR(27), NOTA(4, LA), DUR(13), NOTA(4, LAd), STRUM(2),
    DUR(31), NOTA(5, DO), STRUM(0), DUR(47), NOTA(5, DO), FINE,
};
/* 1003-100D flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_11_b15_s2[] = {
    STRUM(0), DUR(27), PAUSA, DUR(13), PAUSA, DUR(31), PAUSA, DUR(47), PAUSA, FINE,
};
/* 100E-1018 flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_11_b16_s2[] = {
    STRUM(0), DUR(27), PAUSA, DUR(13), PAUSA, DUR(31), PAUSA, DUR(47), PAUSA, FINE,
};
/* 1019-1023 flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_11_b17_s2[] = {
    STRUM(0), DUR(27), PAUSA, DUR(13), PAUSA, DUR(31), PAUSA, DUR(47), PAUSA, FINE,
};
/* 1024-103F comando 12: elenco dei record (blocco, flusso) */
static const uint8_t cmd_12[] = {
    REC(9, ADR_cmd_12_b9_s2), REC(10, ADR_cmd_12_b10_s2), REC(11, ADR_cmd_12_b11_s2),
    REC(12, ADR_cmd_12_b12_s2), REC(13, ADR_cmd_12_b13_s2), REC(14, ADR_cmd_12_b14_s2),
    REC(15, ADR_cmd_12_b15_s2), REC(16, ADR_cmd_12_b16_s2), REC(17, ADR_cmd_12_b17_s2),
    FINE_ELENCO,
};
/* 1040-104E flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_12_b9_s2[] = {
    STRUM(7), ATTEN(5), DUR(9), NOTA(2, SOL), NOTA(2, SOLd), NOTA(3, DO), NOTA(3, DOd),
    NOTA(3, REd), NOTA(3, MI), NOTA(3, SOL), DUR(55), NOTA(3, SOLd), FINE,
};
/* 104F-105D flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_12_b10_s2[] = {
    STRUM(7), ATTEN(4), DUR(9), NOTA(3, SOL), NOTA(3, SOLd), NOTA(4, DO), NOTA(4, DOd),
    NOTA(4, REd), NOTA(4, MI), NOTA(4, SOL), DUR(55), NOTA(4, SOLd), FINE,
};
/* 105E-106C flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_12_b11_s2[] = {
    STRUM(7), ATTEN(3), DUR(9), NOTA(4, SOL), NOTA(4, SOLd), NOTA(5, DO), NOTA(5, DOd),
    NOTA(5, REd), NOTA(5, MI), NOTA(5, SOL), DUR(55), NOTA(5, SOLd), FINE,
};
/* 106D-107B flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_12_b12_s2[] = {
    STRUM(7), ATTEN(3), DUR(9), NOTA(5, SOL), NOTA(5, SOLd), NOTA(6, DO), NOTA(6, DOd),
    NOTA(6, REd), NOTA(6, MI), NOTA(6, SOL), DUR(55), NOTA(6, SOLd), FINE,
};
/* 107C-108A flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_12_b13_s2[] = {
    STRUM(7), ATTEN(5), DUR(9), NOTA(2, DO), NOTA(2, DOd), NOTA(2, REd), NOTA(2, MI),
    NOTA(2, SOL), NOTA(2, SOLd), NOTA(3, DO), DUR(55), NOTA(3, DOd), FINE,
};
/* 108B-1099 flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_12_b14_s2[] = {
    STRUM(7), ATTEN(4), DUR(9), NOTA(3, DO), NOTA(3, DOd), NOTA(3, REd), NOTA(3, MI),
    NOTA(3, SOL), NOTA(3, SOLd), NOTA(4, DO), DUR(55), NOTA(4, DOd), FINE,
};
/* 109A-10A8 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_12_b15_s2[] = {
    STRUM(7), ATTEN(3), DUR(9), NOTA(4, DO), NOTA(4, DOd), NOTA(4, REd), NOTA(4, MI),
    NOTA(4, SOL), NOTA(4, SOLd), NOTA(5, DO), DUR(55), NOTA(5, DOd), FINE,
};
/* 10A9-10B7 flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_12_b16_s2[] = {
    STRUM(7), ATTEN(3), DUR(9), NOTA(5, DO), NOTA(5, DOd), NOTA(5, REd), NOTA(5, MI),
    NOTA(5, SOL), NOTA(5, SOLd), NOTA(6, DO), DUR(55), NOTA(6, DOd), FINE,
};
/* 10B8-10C4 flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_12_b17_s2[] = {
    STRUM(0), DUR(9), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, DUR(55), PAUSA, FINE,
};

const struct aud_pezzo aud_pezzi_spartiti_a[] = {
    PEZZO(cmd_01),
    PEZZO(cmd_01_b9_s2),
    PEZZO(cmd_01_b12_s2),
    PEZZO(cmd_02),
    PEZZO(cmd_02_b9_s2),
    PEZZO(cmd_02_b12_s2),
    PEZZO(cmd_03),
    PEZZO(cmd_03_b0_s1),
    PEZZO(cmd_04),
    PEZZO(cmd_04_b0_s1),
    PEZZO(cmd_05),
    PEZZO(cmd_05_b0_s1),
    PEZZO(cmd_06),
    PEZZO(cmd_06_b9_s2),
    PEZZO(cmd_07),
    PEZZO(cmd_07_b9_s2),
    PEZZO(cmd_08),
    PEZZO(cmd_08_b9_s2),
    PEZZO(cmd_08_b12_s2),
    PEZZO(cmd_09),
    PEZZO(cmd_09_b9_s2),
    PEZZO(cmd_09_b12_s2),
    PEZZO(cmd_09_b15_s2),
    PEZZO(cmd_0A),
    PEZZO(cmd_0A_b0_s1),
    PEZZO(cmd_0A_b1_s1),
    PEZZO(cmd_0A_b2_s1),
    PEZZO(cmd_0A_b3_s1),
    PEZZO(cmd_0A_b4_s1),
    PEZZO(cmd_0A_b5_s1),
    PEZZO(cmd_0A_b6_s1),
    PEZZO(cmd_0A_b7_s1),
    PEZZO(cmd_0A_b8_s1),
    PEZZO(cmd_0A_b9_s2),
    PEZZO(cmd_0A_b10_s2),
    PEZZO(cmd_0A_b11_s2),
    PEZZO(cmd_0A_b12_s2),
    PEZZO(cmd_0A_b13_s2),
    PEZZO(cmd_0A_b14_s2),
    PEZZO(cmd_0A_b15_s2),
    PEZZO(cmd_0A_b16_s2),
    PEZZO(cmd_0A_b17_s2),
    PEZZO(cmd_0B),
    PEZZO(cmd_0B_b9_s2),
    PEZZO(cmd_0B_b10_s2),
    PEZZO(cmd_0B_b11_s2),
    PEZZO(cmd_0B_b12_s2),
    PEZZO(cmd_0B_b13_s2),
    PEZZO(cmd_0B_b14_s2),
    PEZZO(cmd_0B_b15_s2),
    PEZZO(cmd_0B_b16_s2),
    PEZZO(cmd_0B_b17_s2),
    PEZZO(cmd_0C),
    PEZZO(cmd_0C_b9_s2),
    PEZZO(cmd_0D),
    PEZZO(cmd_0D_b10_s2),
    PEZZO(cmd_0D_b13_s2),
    PEZZO(cmd_0E),
    PEZZO(cmd_0E_b10_s2),
    PEZZO(cmd_0E_b13_s2),
    PEZZO(cmd_0E_b10_s2_r2),
    PEZZO(cmd_0F),
    PEZZO(cmd_0F_b10_s2),
    PEZZO(cmd_0F_b13_s2),
    PEZZO(cmd_0F_b15_s2),
    PEZZO(cmd_10),
    PEZZO(cmd_10_b9_s2),
    PEZZO(cmd_10_b12_s2),
    PEZZO(cmd_11),
    PEZZO(cmd_11_b9_s2),
    PEZZO(cmd_11_b10_s2),
    PEZZO(cmd_11_b11_s2),
    PEZZO(cmd_11_b12_s2),
    PEZZO(cmd_11_b13_s2),
    PEZZO(cmd_11_b14_s2),
    PEZZO(cmd_11_b15_s2),
    PEZZO(cmd_11_b16_s2),
    PEZZO(cmd_11_b17_s2),
    PEZZO(cmd_12),
    PEZZO(cmd_12_b9_s2),
    PEZZO(cmd_12_b10_s2),
    PEZZO(cmd_12_b11_s2),
    PEZZO(cmd_12_b12_s2),
    PEZZO(cmd_12_b13_s2),
    PEZZO(cmd_12_b14_s2),
    PEZZO(cmd_12_b15_s2),
    PEZZO(cmd_12_b16_s2),
    PEZZO(cmd_12_b17_s2),
};
const unsigned aud_n_pezzi_spartiti_a =
    sizeof aud_pezzi_spartiti_a / sizeof aud_pezzi_spartiti_a[0];
