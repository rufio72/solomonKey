/* Generato da decomp/estrai_dati.py dalla ROM audio 1.3jk: non modificare
 * a mano. Dati musicali derivati dal programma Tecmo (NOTICE.md). */
/* Elenchi e flussi dei comandi 25-2C. */
#include <stddef.h>
#include "audio_dati.h"

/* 2A91-2AC7 comando 25: elenco dei record (blocco, flusso) */
static const uint8_t cmd_25[] = {
    REC(0, ADR_cmd_25_b0_s1), REC(1, ADR_cmd_25_b1_s1), REC(2, ADR_cmd_25_b2_s1),
    REC(3, ADR_cmd_25_b3_s1), REC(4, ADR_cmd_25_b4_s1), REC(5, ADR_cmd_25_b5_s1),
    REC(6, ADR_cmd_25_b6_s1), REC(7, ADR_cmd_25_b7_s1), REC(8, ADR_cmd_25_b8_s1),
    REC(9, ADR_cmd_25_b9_s2), REC(10, ADR_cmd_25_b10_s2), REC(11, ADR_cmd_25_b11_s2),
    REC(12, ADR_cmd_25_b12_s2), REC(13, ADR_cmd_25_b13_s2), REC(14, ADR_cmd_25_b14_s2),
    REC(15, ADR_cmd_25_b15_s2), REC(16, ADR_cmd_25_b16_s2), REC(17, ADR_cmd_25_b17_s2),
    FINE_ELENCO,
};
/* 2AC8-2AF0 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_25_b0_s1[] = {
    STRUM(2), ATTEN(3), DUR(3), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd),
    NOTA(3, MI), NOTA(3, FA), NOTA(3, FAd), NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA),
    NOTA(3, LAd), NOTA(3, SI), NOTA(4, DO), NOTA(4, DOd), NOTA(4, RE), NOTA(4, REd), STRUM(7),
    ATTEN(2), DUR(12), NOTA(4, MI), NOTA(4, MI), NOTA(4, MI), NOTA(4, RE), NOTA(4, MI),
    NOTA(4, FA), STRUM(2), DUR(48), NOTA(4, SOL), STRUM(6), DUR(7), NOTA(4, SOL), FINE,
};
/* 2AF1-2B19 flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t cmd_25_b1_s1[] = {
    STRUM(2), ATTEN(3), DUR(3), NOTA(2, DO), NOTA(2, DOd), NOTA(2, RE), NOTA(2, REd),
    NOTA(2, MI), NOTA(2, FA), NOTA(2, FAd), NOTA(2, SOL), NOTA(2, SOLd), NOTA(2, LA),
    NOTA(2, LAd), NOTA(2, SI), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), STRUM(7),
    ATTEN(2), DUR(12), NOTA(3, MI), NOTA(3, MI), NOTA(3, MI), NOTA(3, RE), NOTA(3, MI),
    NOTA(3, FA), STRUM(2), DUR(48), NOTA(3, SOL), STRUM(6), DUR(7), NOTA(3, SOL), FINE,
};
/* 2B1A-2B42 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_25_b2_s1[] = {
    STRUM(2), ATTEN(6), DUR(3), NOTA(1, DO), NOTA(1, DOd), NOTA(1, RE), NOTA(1, REd),
    NOTA(1, MI), NOTA(1, FA), NOTA(1, FAd), NOTA(1, SOL), NOTA(1, SOLd), NOTA(1, LA),
    NOTA(1, LAd), NOTA(1, SI), NOTA(2, DO), NOTA(2, DOd), NOTA(2, RE), NOTA(2, REd), STRUM(7),
    ATTEN(5), DUR(12), NOTA(2, MI), NOTA(2, MI), NOTA(2, MI), NOTA(2, RE), NOTA(2, MI),
    NOTA(2, FA), STRUM(2), DUR(48), NOTA(2, SOL), STRUM(6), DUR(7), NOTA(2, SOL), FINE,
};
/* 2B43-2B60 flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t cmd_25_b3_s1[] = {
    STRUM(2), ATTEN(15), DUR(3), RIP_INIZIO(8), PAUSA, PAUSA, RIP_FINE,
    STRUM(7), ATTEN(2), DUR(12), NOTA(4, DO), NOTA(4, DO), NOTA(4, DO), NOTA(3, SI),
    NOTA(4, DO), NOTA(4, DO), STRUM(2), DUR(48), NOTA(4, MI), STRUM(6), DUR(7), NOTA(4, MI),
    FINE,
};
/* 2B61-2B7E flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t cmd_25_b4_s1[] = {
    STRUM(2), ATTEN(3), DUR(3), RIP_INIZIO(8), PAUSA, PAUSA, RIP_FINE,
    STRUM(7), ATTEN(2), DUR(12), NOTA(3, SOL), NOTA(3, SOL), NOTA(3, SOL), NOTA(3, SOL),
    NOTA(3, SOL), NOTA(3, LA), STRUM(2), DUR(48), NOTA(4, DO), STRUM(6), DUR(7), NOTA(4, DO),
    FINE,
};
/* 2B7F-2B83 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_25_b5_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2B84-2B88 flusso, blocco 6, strato 1 (chip 3 voce 3) */
static const uint8_t cmd_25_b6_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2B89-2B8D flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_25_b7_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2B8E-2B92 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_25_b8_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2B93-2B97 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_25_b9_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2B98-2B9C flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_25_b10_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2B9D-2BA1 flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_25_b11_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2BA2-2BA6 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_25_b12_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2BA7-2BAB flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_25_b13_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2BAC-2BB0 flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_25_b14_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2BB1-2BB5 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_25_b15_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2BB6-2BBA flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_25_b16_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2BBB-2BBF flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_25_b17_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2BC0-2BF6 comando 26: elenco dei record (blocco, flusso) */
static const uint8_t cmd_26[] = {
    REC(0, ADR_cmd_26_b0_s1), REC(1, ADR_cmd_26_b1_s1), REC(2, ADR_cmd_26_b2_s1),
    REC(3, ADR_cmd_26_b3_s1), REC(4, ADR_cmd_26_b4_s1), REC(5, ADR_cmd_26_b5_s1),
    REC(6, ADR_cmd_26_b6_s1), REC(7, ADR_cmd_26_b7_s1), REC(8, ADR_cmd_26_b8_s1),
    REC(9, ADR_cmd_26_b9_s2), REC(10, ADR_cmd_26_b10_s2), REC(11, ADR_cmd_26_b11_s2),
    REC(12, ADR_cmd_26_b12_s2), REC(13, ADR_cmd_26_b13_s2), REC(14, ADR_cmd_26_b14_s2),
    REC(15, ADR_cmd_26_b15_s2), REC(16, ADR_cmd_26_b16_s2), REC(17, ADR_cmd_26_b17_s2),
    FINE_ELENCO,
};
/* 2BF7-2BFB flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_26_b0_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2BFC-2C00 flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t cmd_26_b1_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2C01-2C05 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_26_b2_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2C06-2C0A flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t cmd_26_b3_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2C0B-2C0F flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t cmd_26_b4_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2C10-2C14 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_26_b5_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2C15-2C19 flusso, blocco 6, strato 1 (chip 3 voce 3) */
static const uint8_t cmd_26_b6_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2C1A-2C1E flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_26_b7_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2C1F-2C23 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_26_b8_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2C24-2C7C flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_26_b9_s2[] = {
    STRUM(2), DUR(9), ATTEN(0), RUMORE(16), NOTA(0, DO), ATTEN(15), RUMORE(0), PAUSA, ATTEN(0),
    RUMORE(31), NOTA(0, DO), ATTEN(1), RUMORE(30), NOTA(0, DO), ATTEN(2), RUMORE(29),
    NOTA(0, DO), ATTEN(3), RUMORE(28), NOTA(0, DO), ATTEN(4), RUMORE(27), NOTA(0, DO), ATTEN(5),
    RUMORE(26), NOTA(0, DO), ATTEN(6), RUMORE(25), NOTA(0, DO), ATTEN(7), RUMORE(24),
    NOTA(0, DO), ATTEN(8), RUMORE(23), NOTA(0, DO), ATTEN(9), RUMORE(22), NOTA(0, DO),
    ATTEN(10), RUMORE(21), NOTA(0, DO), ATTEN(11), RUMORE(20), NOTA(0, DO), ATTEN(12),
    RUMORE(19), NOTA(0, DO), ATTEN(13), RUMORE(18), NOTA(0, DO), ATTEN(14), RUMORE(17),
    NOTA(0, DO), FINE,
};
/* 2C7D-2C81 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_26_b10_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2C82-2C86 flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_26_b11_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2C87-2CDF flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_26_b12_s2[] = {
    STRUM(2), DUR(9), ATTEN(0), RUMORE(15), NOTA(0, DO), ATTEN(15), RUMORE(0), PAUSA, ATTEN(0),
    RUMORE(30), NOTA(0, DO), ATTEN(1), RUMORE(29), NOTA(0, DO), ATTEN(2), RUMORE(28),
    NOTA(0, DO), ATTEN(3), RUMORE(27), NOTA(0, DO), ATTEN(4), RUMORE(26), NOTA(0, DO), ATTEN(5),
    RUMORE(25), NOTA(0, DO), ATTEN(6), RUMORE(24), NOTA(0, DO), ATTEN(7), RUMORE(23),
    NOTA(0, DO), ATTEN(8), RUMORE(22), NOTA(0, DO), ATTEN(9), RUMORE(21), NOTA(0, DO),
    ATTEN(10), RUMORE(20), NOTA(0, DO), ATTEN(11), RUMORE(19), NOTA(0, DO), ATTEN(12),
    RUMORE(18), NOTA(0, DO), ATTEN(13), RUMORE(17), NOTA(0, DO), ATTEN(14), RUMORE(16),
    NOTA(0, DO), FINE,
};
/* 2CE0-2CE4 flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_26_b13_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2CE5-2CE9 flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_26_b14_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2CEA-2D42 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_26_b15_s2[] = {
    STRUM(2), DUR(9), ATTEN(0), RUMORE(14), NOTA(0, DO), ATTEN(15), RUMORE(0), PAUSA, ATTEN(0),
    RUMORE(29), NOTA(0, DO), ATTEN(1), RUMORE(28), NOTA(0, DO), ATTEN(2), RUMORE(27),
    NOTA(0, DO), ATTEN(3), RUMORE(26), NOTA(0, DO), ATTEN(4), RUMORE(25), NOTA(0, DO), ATTEN(5),
    RUMORE(24), NOTA(0, DO), ATTEN(6), RUMORE(23), NOTA(0, DO), ATTEN(7), RUMORE(22),
    NOTA(0, DO), ATTEN(8), RUMORE(21), NOTA(0, DO), ATTEN(9), RUMORE(20), NOTA(0, DO),
    ATTEN(10), RUMORE(19), NOTA(0, DO), ATTEN(11), RUMORE(18), NOTA(0, DO), ATTEN(12),
    RUMORE(17), NOTA(0, DO), ATTEN(13), RUMORE(16), NOTA(0, DO), ATTEN(14), RUMORE(15),
    NOTA(0, DO), FINE,
};
/* 2D43-2D47 flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_26_b16_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2D48-2D4C flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_26_b17_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2D4D-2D50 comando 27: elenco dei record (blocco, flusso) */
static const uint8_t cmd_27[] = {
    REC(15, ADR_cmd_27_b15_s2), FINE_ELENCO,
};
/* 2D51-2DC3 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_27_b15_s2[] = {
    STRUM(2), DUR(1), ATTEN(1), NOTA(2, DO), NOTA(2, SOL), NOTA(3, DO), NOTA(3, SOL), ATTEN(2),
    NOTA(2, DOd), NOTA(2, SOLd), NOTA(3, DOd), NOTA(3, SOLd), ATTEN(3), NOTA(2, RE),
    NOTA(2, LA), NOTA(3, RE), NOTA(3, LA), ATTEN(4), NOTA(2, REd), NOTA(2, LAd), NOTA(3, REd),
    NOTA(3, LAd), DUR(0), TRASP(0), ATTEN(250), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE),
    NOTA(3, REd), NOTA(3, MI), ATTEN(2), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), NOTA(3, MI),
    NOTA(3, FA), ATTEN(3), NOTA(3, RE), NOTA(3, REd), NOTA(3, MI), NOTA(3, FA), NOTA(3, FAd),
    ATTEN(4), NOTA(3, REd), NOTA(3, MI), NOTA(3, FA), NOTA(3, FAd), NOTA(3, SOL), ATTEN(5),
    NOTA(3, MI), NOTA(3, FA), NOTA(3, FAd), NOTA(3, SOL), NOTA(3, SOLd), ATTEN(6), NOTA(3, FA),
    NOTA(3, FAd), NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA), ATTEN(7), NOTA(3, FAd),
    NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA), NOTA(3, LAd), ATTEN(8), NOTA(3, SOL),
    NOTA(3, SOLd), NOTA(3, LA), NOTA(3, LAd), NOTA(3, SI), ATTEN(9), NOTA(3, SOLd), NOTA(3, LA),
    NOTA(3, LAd), NOTA(3, SI), B(0x3C), ATTEN(10), NOTA(3, LA), NOTA(3, LAd), NOTA(3, SI),
    B(0x3C), B(0x3D), ATTEN(11), NOTA(3, LAd), NOTA(3, SI), B(0x3C), B(0x3D), B(0x3E),
    ATTEN(12), NOTA(3, SI), B(0x3C), B(0x3D), B(0x3E), B(0x3F), FINE,
};
/* 2DC4-2DD0 comando 28: elenco dei record (blocco, flusso) */
static const uint8_t cmd_28[] = {
    REC(2, ADR_cmd_28_b2_s1), REC(5, ADR_cmd_28_b5_s1), REC(7, ADR_cmd_28_b7_s1),
    REC(8, ADR_cmd_28_b8_s1), FINE_ELENCO,
};
/* 2DD1-2DE2 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_28_b2_s1[] = {
    STRUM(2), CHIAMA(ADR_cmd_28_b2_s1_p2), CHIAMA(ADR_cmd_28_b2_s1_p2),
    CHIAMA(ADR_cmd_28_b2_s1_p2), CHIAMA(ADR_cmd_28_b2_s1_p3), SALTA(ADR_cmd_28_b2_s1),
    B(0xFF),
};
/* 2DE3-2E0E ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_28_b2_s1_p2[] = {
    RIP_INIZIO(2), DUR(0), ATTEN(3), RUMORE(7), NOTA(0, DO), ATTEN(6), RUMORE(6), NOTA(0, DO),
    ATTEN(9), RUMORE(5), NOTA(0, DO), ATTEN(12), RUMORE(4), NOTA(0, DO), DUR(11), ATTEN(15),
    RUMORE(0), PAUSA, RIP_FINE,
    STRUM(4), DUR(15), ATTEN(2), RUMORE(7), NOTA(0, DO), ATTEN(15), RUMORE(0), PAUSA, RITORNA,
};
/* 2E0F-2E24 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_28_b2_s1_p3[] = {
    DUR(15), ATTEN(15), RUMORE(0), PAUSA, ATTEN(2), RUMORE(9), NOTA(0, DO), ATTEN(2), RUMORE(8),
    NOTA(0, DO), ATTEN(2), RUMORE(7), NOTA(0, DO), RITORNA,
};
/* 2E25-2E57 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_28_b5_s1[] = {
    STRUM(2), ATTEN(4), TRASP(0), DUR(15), RIP_INIZIO(32), NOTA(3, RE), NOTA(4, RE), RIP_FINE,
    RIP_INIZIO(16), NOTA(2, LA), NOTA(3, LA), RIP_FINE,
    RIP_INIZIO(32), NOTA(3, RE), NOTA(4, RE), RIP_FINE,
    RIP_INIZIO(16), NOTA(2, LA), NOTA(3, LA), RIP_FINE,
    RIP_INIZIO(8), NOTA(2, SOL), NOTA(3, SOL), RIP_FINE,
    RIP_INIZIO(8), NOTA(2, FAd), NOTA(3, FAd), RIP_FINE,
    RIP_INIZIO(8), NOTA(2, SOL), NOTA(3, SOL), RIP_FINE,
    RIP_INIZIO(8), NOTA(2, FAd), NOTA(3, FAd), RIP_FINE,
    SALTA(ADR_cmd_28_b5_s1),
    B(0xFF),
};
/* 2E58-2E6B flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_28_b7_s1[] = {
    STRUM(2), ATTEN(4), TRASP(0), DUR(15), CHIAMA(ADR_cmd_28_b7_s1_p2),
    CHIAMA(ADR_cmd_28_b7_s1_p2), CHIAMA(ADR_cmd_28_b7_s1_p3), SALTA(ADR_cmd_28_b7_s1),
    B(0xFF),
};
/* 2E6C-2EC2 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_28_b7_s1_p2[] = {
    RIP_INIZIO(5), NOTA(5, RE), PERIODO(0x0053), RIP_FINE,
    NOTA(5, DOd), NOTA(5, DO), NOTA(4, LA), NOTA(4, LA), NOTA(5, DO), NOTA(5, DOd),
    RIP_INIZIO(5), NOTA(5, RE), PERIODO(0x0053), RIP_FINE,
    NOTA(5, DOd), NOTA(5, DO), NOTA(4, LA), NOTA(4, LA), NOTA(5, DO), NOTA(5, DOd),
    RIP_INIZIO(5), NOTA(5, RE), PERIODO(0x0053), RIP_FINE,
    NOTA(5, DOd), NOTA(5, DO), NOTA(4, LA), NOTA(4, LA), NOTA(5, DO), NOTA(5, DOd),
    RIP_INIZIO(5), NOTA(5, RE), PERIODO(0x0053), RIP_FINE,
    NOTA(5, DOd), NOTA(5, DO), NOTA(4, LA), NOTA(4, LA), NOTA(4, SOLd), NOTA(4, FA),
    RIP_INIZIO(3), NOTA(4, MI), PERIODO(0x0092), RIP_FINE,
    NOTA(4, FA), NOTA(4, MI), NOTA(4, SOLd), NOTA(4, FA), NOTA(4, MI), NOTA(4, RE), NOTA(4, LA),
    NOTA(4, SOLd), NOTA(4, FA), NOTA(4, MI), RIP_INIZIO(3), NOTA(4, MI), PERIODO(0x0092),
    RIP_FINE,
    NOTA(4, FA), NOTA(4, MI), NOTA(4, SOLd), NOTA(4, FA), NOTA(4, MI), NOTA(4, RE), NOTA(4, LA),
    NOTA(4, SOLd), NOTA(4, FA), NOTA(4, MI), RITORNA,
};
/* 2EC3-2EE6 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_28_b7_s1_p3[] = {
    RIP_INIZIO(2), NOTA(4, RE), NOTA(4, MI), NOTA(4, FA), NOTA(4, SOLd), NOTA(4, SOLd),
    NOTA(4, FA), NOTA(4, MI), NOTA(4, RE), NOTA(4, SOLd), NOTA(4, FA), NOTA(4, MI), NOTA(4, RE),
    NOTA(4, SOLd), NOTA(4, FA), NOTA(4, MI), NOTA(4, RE), NOTA(4, DOd), NOTA(4, RE),
    NOTA(4, DOd), NOTA(4, RE), NOTA(4, RE), NOTA(4, DOd), NOTA(4, RE), NOTA(4, DOd),
    NOTA(4, FAd), NOTA(4, FA), NOTA(4, RE), NOTA(4, DOd), NOTA(4, FAd), NOTA(4, FA),
    NOTA(4, RE), NOTA(4, DOd), RIP_FINE,
    RITORNA,
};
/* 2EE7-2F11 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_28_b8_s1[] = {
    STRUM(1), ATTEN(3), TRASP(0), DUR(15), RIP_INIZIO(64), NOTA(2, RE), RIP_FINE,
    RIP_INIZIO(32), NOTA(1, LA), RIP_FINE,
    RIP_INIZIO(64), NOTA(2, RE), RIP_FINE,
    RIP_INIZIO(32), NOTA(1, LA), RIP_FINE,
    RIP_INIZIO(16), NOTA(1, SOL), RIP_FINE,
    RIP_INIZIO(16), NOTA(1, FAd), RIP_FINE,
    RIP_INIZIO(16), NOTA(1, SOL), RIP_FINE,
    RIP_INIZIO(16), NOTA(1, FAd), RIP_FINE,
    SALTA(ADR_cmd_28_b8_s1),
    B(0xFF),
};
/* 2F12-2F15 comando 29: elenco dei record (blocco, flusso) */
static const uint8_t cmd_29[] = {
    REC(15, ADR_cmd_29_b15_s2), FINE_ELENCO,
};
/* 2F16-2F2B flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_29_b15_s2[] = {
    STRUM(2), DUR(0), ATTEN(1), NOTA(2, DOd), NOTA(2, RE), NOTA(2, REd), NOTA(2, MI),
    NOTA(2, FA), NOTA(2, FAd), NOTA(2, SOL), NOTA(2, SOLd), NOTA(2, SOL), NOTA(2, FAd),
    NOTA(2, FA), NOTA(2, MI), NOTA(2, REd), NOTA(2, RE), NOTA(2, DOd), NOTA(2, DO), FINE,
};
/* 2F2C-2F38 comando 2A: elenco dei record (blocco, flusso) */
static const uint8_t cmd_2A[] = {
    REC(2, ADR_cmd_2A_b2_s1), REC(5, ADR_cmd_2A_b5_s1), REC(7, ADR_cmd_2A_b7_s1),
    REC(8, ADR_cmd_2A_b8_s1), FINE_ELENCO,
};
/* 2F39-2F54 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_2A_b2_s1[] = {
    STRUM(0), DUR(47), ATTEN(4), RUMORE(5), NOTA(0, DO), STRUM(1), DUR(31), ATTEN(2), RUMORE(6),
    NOTA(0, DO), STRUM(4), DUR(15), ATTEN(4), RUMORE(7), NOTA(0, DO), SALTA(ADR_cmd_2A_b2_s1),
    B(0xFF),
};
/* 2F55-2FBF flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_2A_b5_s1[] = {
    STRUM(2), ATTEN(4), TRASP(0), DUR(15), NOTA(4, DO), NOTA(4, DO), NOTA(3, DO), NOTA(4, REd),
    NOTA(4, REd), NOTA(3, REd), NOTA(4, FA), NOTA(4, FA), NOTA(3, FA), NOTA(4, FAd),
    NOTA(4, FAd), NOTA(3, FAd), NOTA(4, SOL), NOTA(4, SOL), NOTA(3, SOL), NOTA(5, DO),
    NOTA(5, DO), NOTA(4, DO), NOTA(4, SI), NOTA(4, SI), NOTA(3, SI), NOTA(4, SOL), NOTA(4, SOL),
    NOTA(3, SOL), NOTA(4, LAd), NOTA(4, LAd), NOTA(3, LAd), NOTA(4, LA), NOTA(4, LA),
    NOTA(3, LA), NOTA(4, FA), NOTA(4, FA), NOTA(3, FA), NOTA(4, SOLd), NOTA(4, SOLd),
    NOTA(3, SOLd), NOTA(4, SOL), NOTA(4, SOL), NOTA(3, SOL), NOTA(4, MI), NOTA(4, MI),
    NOTA(3, MI), NOTA(4, FA), NOTA(4, FA), NOTA(3, FA), NOTA(4, DOd), NOTA(4, DOd),
    NOTA(3, DOd), NOTA(4, DO), NOTA(4, DO), NOTA(3, DO), NOTA(3, SI), NOTA(3, SI), NOTA(2, SI),
    NOTA(3, SOL), NOTA(3, SOL), NOTA(2, SOL), NOTA(3, LAd), NOTA(3, LAd), NOTA(2, LAd),
    NOTA(3, LA), NOTA(3, LA), NOTA(2, LA), NOTA(3, FA), NOTA(3, FA), NOTA(2, FA), NOTA(3, SOLd),
    NOTA(3, SOLd), NOTA(2, SOLd), NOTA(3, SOL), NOTA(3, SOL), NOTA(2, SOL), NOTA(4, REd),
    NOTA(4, REd), NOTA(3, REd), NOTA(4, RE), NOTA(4, RE), NOTA(3, RE), NOTA(3, LA), NOTA(3, LA),
    NOTA(2, LA), NOTA(4, DOd), NOTA(4, DOd), NOTA(3, DOd), NOTA(4, DO), NOTA(4, DO),
    NOTA(3, DO), NOTA(3, SOLd), NOTA(3, SOLd), NOTA(2, SOLd), NOTA(3, SI), NOTA(3, SI),
    NOTA(2, SI), NOTA(3, SOL), NOTA(3, SOL), NOTA(2, SOL), SALTA(ADR_cmd_2A_b5_s1),
    B(0xFF),
};
/* 2FC0-302A flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_2A_b7_s1[] = {
    STRUM(2), ATTEN(4), TRASP(0), DUR(15), NOTA(3, SOL), NOTA(3, SOL), NOTA(2, SOL),
    NOTA(3, SI), NOTA(3, SI), NOTA(2, SI), NOTA(4, DOd), NOTA(4, DOd), NOTA(3, DOd),
    NOTA(4, RE), NOTA(4, RE), NOTA(3, RE), NOTA(4, REd), NOTA(4, REd), NOTA(3, REd),
    NOTA(4, SOLd), NOTA(4, SOLd), NOTA(3, SOLd), NOTA(4, SOL), NOTA(4, SOL), NOTA(3, SOL),
    NOTA(4, REd), NOTA(4, REd), NOTA(3, REd), NOTA(4, FAd), NOTA(4, FAd), NOTA(3, FAd),
    NOTA(4, FA), NOTA(4, FA), NOTA(3, FA), NOTA(4, DO), NOTA(4, DO), NOTA(3, DO), NOTA(4, DOd),
    NOTA(4, DOd), NOTA(3, DOd), NOTA(4, RE), NOTA(4, RE), NOTA(3, RE), NOTA(3, SI), NOTA(3, SI),
    NOTA(2, SI), NOTA(4, DO), NOTA(4, DO), NOTA(3, DO), NOTA(3, LA), NOTA(3, LA), NOTA(2, LA),
    NOTA(3, SOLd), NOTA(3, SOLd), NOTA(2, SOLd), NOTA(3, SOL), NOTA(3, SOL), NOTA(2, SOL),
    NOTA(3, REd), NOTA(3, REd), NOTA(2, REd), NOTA(3, FAd), NOTA(3, FAd), NOTA(2, FAd),
    NOTA(3, FA), NOTA(3, FA), NOTA(2, FA), NOTA(3, DOd), NOTA(3, DOd), NOTA(2, DOd),
    NOTA(3, REd), NOTA(3, REd), NOTA(2, REd), NOTA(3, REd), NOTA(3, REd), NOTA(2, REd),
    NOTA(3, SI), NOTA(3, SI), NOTA(2, SI), NOTA(3, LA), NOTA(3, LA), NOTA(2, LA), NOTA(3, MI),
    NOTA(3, MI), NOTA(2, MI), NOTA(3, LA), NOTA(3, LA), NOTA(2, LA), NOTA(3, SOLd),
    NOTA(3, SOLd), NOTA(2, SOLd), NOTA(3, MI), NOTA(3, MI), NOTA(2, MI), NOTA(3, FA),
    NOTA(3, FA), NOTA(2, FA), NOTA(3, RE), NOTA(3, RE), NOTA(2, RE), SALTA(ADR_cmd_2A_b7_s1),
    B(0xFF),
};
/* 302B-3095 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_2A_b8_s1[] = {
    STRUM(2), ATTEN(4), TRASP(0), DUR(15), NOTA(3, REd), NOTA(3, REd), NOTA(2, REd),
    NOTA(3, SOLd), NOTA(3, SOLd), NOTA(2, SOLd), NOTA(3, SOLd), NOTA(3, SOLd), NOTA(2, SOLd),
    NOTA(3, LA), NOTA(3, LA), NOTA(2, LA), NOTA(4, DO), NOTA(4, DO), NOTA(3, DO), NOTA(4, REd),
    NOTA(4, REd), NOTA(3, REd), NOTA(4, RE), NOTA(4, RE), NOTA(3, RE), NOTA(3, LAd),
    NOTA(3, LAd), NOTA(2, LAd), NOTA(4, DOd), NOTA(4, DOd), NOTA(3, DOd), NOTA(4, DO),
    NOTA(4, DO), NOTA(3, DO), NOTA(3, SOLd), NOTA(3, SOLd), NOTA(2, SOLd), NOTA(3, LAd),
    NOTA(3, LAd), NOTA(2, LAd), NOTA(3, SI), NOTA(3, SI), NOTA(2, SI), NOTA(3, SOL),
    NOTA(3, SOL), NOTA(2, SOL), NOTA(3, LA), NOTA(3, LA), NOTA(2, LA), NOTA(3, MI), NOTA(3, MI),
    NOTA(2, MI), NOTA(3, REd), NOTA(3, REd), NOTA(2, REd), NOTA(3, RE), NOTA(3, RE),
    NOTA(2, RE), NOTA(2, SI), NOTA(2, SI), NOTA(1, SI), NOTA(3, REd), NOTA(3, REd),
    NOTA(2, REd), NOTA(3, RE), NOTA(3, RE), NOTA(2, RE), NOTA(2, LA), NOTA(2, LA), NOTA(1, LA),
    NOTA(2, LAd), NOTA(2, LAd), NOTA(1, LAd), NOTA(2, LAd), NOTA(2, LAd), NOTA(1, LAd),
    NOTA(3, FAd), NOTA(3, FAd), NOTA(2, FAd), NOTA(3, FA), NOTA(3, FA), NOTA(2, FA),
    NOTA(2, SI), NOTA(2, SI), NOTA(1, SI), NOTA(3, MI), NOTA(3, MI), NOTA(2, MI), NOTA(3, REd),
    NOTA(3, REd), NOTA(2, REd), NOTA(2, SI), NOTA(2, SI), NOTA(1, SI), NOTA(3, RE), NOTA(3, RE),
    NOTA(2, RE), NOTA(2, SI), NOTA(2, SI), NOTA(1, SI), SALTA(ADR_cmd_2A_b8_s1),
    B(0xFF),
};
/* 3096-30B1 comando 2B: elenco dei record (blocco, flusso) */
static const uint8_t cmd_2B[] = {
    REC(9, ADR_cmd_2B_b9_s2), REC(10, ADR_cmd_2B_b10_s2), REC(11, ADR_cmd_2B_b11_s2),
    REC(12, ADR_cmd_2B_b12_s2), REC(13, ADR_cmd_2B_b13_s2), REC(14, ADR_cmd_2B_b14_s2),
    REC(15, ADR_cmd_2B_b15_s2), REC(16, ADR_cmd_2B_b16_s2), REC(17, ADR_cmd_2B_b17_s2),
    FINE_ELENCO,
};
/* 30B2-30BD flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_2B_b9_s2[] = {
    STRUM(0), DUR(9), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 30BE-30C9 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_2B_b10_s2[] = {
    STRUM(0), DUR(9), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 30CA-30E3 flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_2B_b11_s2[] = {
    STRUM(8), TRASP(0), DUR(9), ATTEN(2), CHIAMA(ADR_cmd_2B_b11_s2_p2), ATTEN(3),
    CHIAMA(ADR_cmd_2B_b11_s2_p2), ATTEN(4), CHIAMA(ADR_cmd_2B_b11_s2_p2), ATTEN(5),
    CHIAMA(ADR_cmd_2B_b11_s2_p2), FINE,
};
/* 30E4-30ED ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_2B_b11_s2_p2[] = {
    NOTA(4, MI), NOTA(4, SOL), NOTA(4, MI), NOTA(4, DO), NOTA(3, LA), NOTA(4, DO), NOTA(4, MI),
    NOTA(4, DO), RITORNA,
    B(0xFF),
};
/* 30EE-30F9 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_2B_b12_s2[] = {
    STRUM(0), DUR(9), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 30FA-3105 flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_2B_b13_s2[] = {
    STRUM(0), DUR(9), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 3106-311F flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_2B_b14_s2[] = {
    STRUM(8), TRASP(0), DUR(9), ATTEN(2), CHIAMA(ADR_cmd_2B_b14_s2_p2), ATTEN(3),
    CHIAMA(ADR_cmd_2B_b14_s2_p2), ATTEN(4), CHIAMA(ADR_cmd_2B_b14_s2_p2), ATTEN(5),
    CHIAMA(ADR_cmd_2B_b14_s2_p2), FINE,
};
/* 3120-3129 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_2B_b14_s2_p2[] = {
    NOTA(4, DO), NOTA(4, MI), NOTA(4, DO), NOTA(3, LA), NOTA(3, FA), NOTA(3, LA), NOTA(4, DO),
    NOTA(3, LA), RITORNA,
    B(0xFF),
};
/* 312A-3143 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_2B_b15_s2[] = {
    STRUM(8), TRASP(0), DUR(9), ATTEN(2), CHIAMA(ADR_cmd_2B_b15_s2_p2), ATTEN(3),
    CHIAMA(ADR_cmd_2B_b15_s2_p2), ATTEN(4), CHIAMA(ADR_cmd_2B_b15_s2_p2), ATTEN(5),
    CHIAMA(ADR_cmd_2B_b15_s2_p2), FINE,
};
/* 3144-314D ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_2B_b15_s2_p2[] = {
    NOTA(3, LA), NOTA(4, DO), NOTA(3, LA), NOTA(3, FA), NOTA(3, RE), NOTA(3, FA), NOTA(3, LA),
    NOTA(3, FA), RITORNA,
    B(0xFF),
};
/* 314E-3167 flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_2B_b16_s2[] = {
    STRUM(8), TRASP(0), DUR(9), ATTEN(2), CHIAMA(ADR_cmd_2B_b16_s2_p2), ATTEN(3),
    CHIAMA(ADR_cmd_2B_b16_s2_p2), ATTEN(4), CHIAMA(ADR_cmd_2B_b16_s2_p2), ATTEN(5),
    CHIAMA(ADR_cmd_2B_b16_s2_p2), FINE,
};
/* 3168-3171 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_2B_b16_s2_p2[] = {
    NOTA(3, FA), NOTA(3, LA), NOTA(3, FA), NOTA(3, RE), NOTA(2, SI), NOTA(3, RE), NOTA(3, FA),
    NOTA(3, RE), RITORNA,
    B(0xFF),
};
/* 3172-318B flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_2B_b17_s2[] = {
    STRUM(8), TRASP(0), DUR(9), ATTEN(2), CHIAMA(ADR_cmd_2B_b17_s2_p2), ATTEN(3),
    CHIAMA(ADR_cmd_2B_b17_s2_p2), ATTEN(4), CHIAMA(ADR_cmd_2B_b17_s2_p2), ATTEN(5),
    CHIAMA(ADR_cmd_2B_b17_s2_p2), FINE,
};
/* 318C-3195 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_2B_b17_s2_p2[] = {
    NOTA(3, RE), NOTA(3, FA), NOTA(3, RE), NOTA(2, SI), NOTA(2, SOL), NOTA(2, SI), NOTA(3, RE),
    NOTA(2, SI), RITORNA,
    B(0xFF),
};
/* 3196-3199 comando 2C: elenco dei record (blocco, flusso) */
static const uint8_t cmd_2C[] = {
    REC(10, ADR_cmd_2C_b10_s2), FINE_ELENCO,
};
/* 319A-31BA flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_2C_b10_s2[] = {
    STRUM(2), ATTEN(1), DUR(0), RIP_INIZIO(5), RUMORE(31), NOTA(0, DO), RUMORE(30), NOTA(0, DO),
    RUMORE(29), NOTA(0, DO), RUMORE(28), NOTA(0, DO), RUMORE(27), NOTA(0, DO), RUMORE(26),
    NOTA(0, DO), RUMORE(25), NOTA(0, DO), RUMORE(24), NOTA(0, DO), RIP_FINE,
    FINE,
};

const struct aud_pezzo aud_pezzi_spartiti_e[] = {
    PEZZO(cmd_25),
    PEZZO(cmd_25_b0_s1),
    PEZZO(cmd_25_b1_s1),
    PEZZO(cmd_25_b2_s1),
    PEZZO(cmd_25_b3_s1),
    PEZZO(cmd_25_b4_s1),
    PEZZO(cmd_25_b5_s1),
    PEZZO(cmd_25_b6_s1),
    PEZZO(cmd_25_b7_s1),
    PEZZO(cmd_25_b8_s1),
    PEZZO(cmd_25_b9_s2),
    PEZZO(cmd_25_b10_s2),
    PEZZO(cmd_25_b11_s2),
    PEZZO(cmd_25_b12_s2),
    PEZZO(cmd_25_b13_s2),
    PEZZO(cmd_25_b14_s2),
    PEZZO(cmd_25_b15_s2),
    PEZZO(cmd_25_b16_s2),
    PEZZO(cmd_25_b17_s2),
    PEZZO(cmd_26),
    PEZZO(cmd_26_b0_s1),
    PEZZO(cmd_26_b1_s1),
    PEZZO(cmd_26_b2_s1),
    PEZZO(cmd_26_b3_s1),
    PEZZO(cmd_26_b4_s1),
    PEZZO(cmd_26_b5_s1),
    PEZZO(cmd_26_b6_s1),
    PEZZO(cmd_26_b7_s1),
    PEZZO(cmd_26_b8_s1),
    PEZZO(cmd_26_b9_s2),
    PEZZO(cmd_26_b10_s2),
    PEZZO(cmd_26_b11_s2),
    PEZZO(cmd_26_b12_s2),
    PEZZO(cmd_26_b13_s2),
    PEZZO(cmd_26_b14_s2),
    PEZZO(cmd_26_b15_s2),
    PEZZO(cmd_26_b16_s2),
    PEZZO(cmd_26_b17_s2),
    PEZZO(cmd_27),
    PEZZO(cmd_27_b15_s2),
    PEZZO(cmd_28),
    PEZZO(cmd_28_b2_s1),
    PEZZO(cmd_28_b2_s1_p2),
    PEZZO(cmd_28_b2_s1_p3),
    PEZZO(cmd_28_b5_s1),
    PEZZO(cmd_28_b7_s1),
    PEZZO(cmd_28_b7_s1_p2),
    PEZZO(cmd_28_b7_s1_p3),
    PEZZO(cmd_28_b8_s1),
    PEZZO(cmd_29),
    PEZZO(cmd_29_b15_s2),
    PEZZO(cmd_2A),
    PEZZO(cmd_2A_b2_s1),
    PEZZO(cmd_2A_b5_s1),
    PEZZO(cmd_2A_b7_s1),
    PEZZO(cmd_2A_b8_s1),
    PEZZO(cmd_2B),
    PEZZO(cmd_2B_b9_s2),
    PEZZO(cmd_2B_b10_s2),
    PEZZO(cmd_2B_b11_s2),
    PEZZO(cmd_2B_b11_s2_p2),
    PEZZO(cmd_2B_b12_s2),
    PEZZO(cmd_2B_b13_s2),
    PEZZO(cmd_2B_b14_s2),
    PEZZO(cmd_2B_b14_s2_p2),
    PEZZO(cmd_2B_b15_s2),
    PEZZO(cmd_2B_b15_s2_p2),
    PEZZO(cmd_2B_b16_s2),
    PEZZO(cmd_2B_b16_s2_p2),
    PEZZO(cmd_2B_b17_s2),
    PEZZO(cmd_2B_b17_s2_p2),
    PEZZO(cmd_2C),
    PEZZO(cmd_2C_b10_s2),
};
const unsigned aud_n_pezzi_spartiti_e =
    sizeof aud_pezzi_spartiti_e / sizeof aud_pezzi_spartiti_e[0];
