/* Generato da decomp/estrai_dati.py dalla ROM audio 1.3jk: non modificare
 * a mano. Dati musicali derivati dal programma Tecmo (NOTICE.md). */
/* Elenchi e flussi dei comandi 2D-3E. */
#include <stddef.h>
#include "audio_dati.h"

/* 31BB-31C1 comando 2D: elenco dei record (blocco, flusso) */
static const uint8_t cmd_2D[] = {
    REC(10, ADR_cmd_2D_b10_s2), REC(15, ADR_cmd_2D_b15_s2), FINE_ELENCO,
};
/* 31C2-3219 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_2D_b10_s2[] = {
    STRUM(2), DUR(6), ATTEN(1), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), ATTEN(2),
    NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), ATTEN(3), NOTA(3, DO), NOTA(3, DOd),
    NOTA(3, RE), NOTA(3, REd), ATTEN(4), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd),
    ATTEN(5), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), ATTEN(6), NOTA(3, DO),
    NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), ATTEN(7), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE),
    NOTA(3, REd), ATTEN(8), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), ATTEN(9),
    NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), ATTEN(10), NOTA(3, DO), NOTA(3, DOd),
    NOTA(3, RE), NOTA(3, REd), ATTEN(11), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd),
    ATTEN(12), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), ATTEN(13), NOTA(3, DO),
    NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), ATTEN(14), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE),
    NOTA(3, REd), FINE,
};
/* 321A-3271 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_2D_b15_s2[] = {
    STRUM(2), DUR(6), ATTEN(1), NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd), NOTA(5, MI), ATTEN(2),
    NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd), NOTA(5, MI), ATTEN(3), NOTA(5, DOd), NOTA(5, RE),
    NOTA(5, REd), NOTA(5, MI), ATTEN(4), NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd), NOTA(5, MI),
    ATTEN(5), NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd), NOTA(5, MI), ATTEN(6), NOTA(5, DOd),
    NOTA(5, RE), NOTA(5, REd), NOTA(5, MI), ATTEN(7), NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd),
    NOTA(5, MI), ATTEN(8), NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd), NOTA(5, MI), ATTEN(9),
    NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd), NOTA(5, MI), ATTEN(10), NOTA(5, DOd), NOTA(5, RE),
    NOTA(5, REd), NOTA(5, MI), ATTEN(11), NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd), NOTA(5, MI),
    ATTEN(12), NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd), NOTA(5, MI), ATTEN(13), NOTA(5, DOd),
    NOTA(5, RE), NOTA(5, REd), NOTA(5, MI), ATTEN(14), NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd),
    NOTA(5, MI), FINE,
};
/* 3272-3275 comando 2E: elenco dei record (blocco, flusso) */
static const uint8_t cmd_2E[] = {
    REC(0, ADR_cmd_2E_b0_s1), FINE_ELENCO,
};
/* 3276-327C flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_2E_b0_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 327D-3283 comando 2F: elenco dei record (blocco, flusso) */
static const uint8_t cmd_2F[] = {
    REC(10, ADR_cmd_2F_b10_s2), REC(13, ADR_cmd_2F_b13_s2), FINE_ELENCO,
};
/* 3284-32AD flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_2F_b10_s2[] = {
    STRUM(2), ATTEN(0), DUR(0), RUMORE(29), NOTA(0, DO), RUMORE(27), NOTA(0, DO), RUMORE(25),
    NOTA(0, DO), RUMORE(23), NOTA(0, DO), RUMORE(21), NOTA(0, DO), RUMORE(19), NOTA(0, DO),
    RUMORE(17), NOTA(0, DO), RUMORE(15), NOTA(0, DO), INV_HW(0x09, 0x1200), DUR(15),
    RIP_INIZIO(16), RUMORE(16), NOTA(0, DO), RIP_FINE,
    FINE,
};
/* 32AE-32D7 flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_2F_b13_s2[] = {
    STRUM(2), ATTEN(0), DUR(0), RUMORE(30), NOTA(0, DO), RUMORE(28), NOTA(0, DO), RUMORE(26),
    NOTA(0, DO), RUMORE(24), NOTA(0, DO), RUMORE(22), NOTA(0, DO), RUMORE(20), NOTA(0, DO),
    RUMORE(18), NOTA(0, DO), RUMORE(16), NOTA(0, DO), INV_HW(0x09, 0x1200), DUR(15),
    RIP_INIZIO(16), RUMORE(17), NOTA(0, DO), RIP_FINE,
    FINE,
};
/* 32D8-32DE comando 30: elenco dei record (blocco, flusso) */
static const uint8_t cmd_30[] = {
    REC(10, ADR_cmd_30_b10_s2), REC(13, ADR_cmd_30_b13_s2), FINE_ELENCO,
};
/* 32DF-32FB flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_30_b10_s2[] = {
    STRUM(2), ATTEN(0), DUR(0), RIP_INIZIO(2), RUMORE(16), NOTA(0, DO), RUMORE_OFF,
    PERIODO(0x0070), RUMORE(17), NOTA(0, DO), RUMORE_OFF, PERIODO(0x0080), RIP_FINE,
    STRUM(1), DUR(81), RUMORE(16), NOTA(0, DO), FINE,
};
/* 32FC-3318 flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_30_b13_s2[] = {
    STRUM(2), ATTEN(0), DUR(0), RIP_INIZIO(2), RUMORE(17), NOTA(0, DO), RUMORE_OFF,
    PERIODO(0x0071), RUMORE(18), NOTA(0, DO), RUMORE_OFF, PERIODO(0x0081), RIP_FINE,
    STRUM(1), DUR(81), RUMORE(17), NOTA(0, DO), FINE,
};
/* 3319-3322 comando 31: elenco dei record (blocco, flusso) */
static const uint8_t cmd_31[] = {
    REC(10, ADR_cmd_31_b10_s2), REC(13, ADR_cmd_31_b13_s2), REC(15, ADR_cmd_31_b15_s2),
    FINE_ELENCO,
};
/* 3323-3327 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_31_b10_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3328-332C flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_31_b13_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 332D-3331 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_31_b15_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3332-334D comando 32: elenco dei record (blocco, flusso) */
static const uint8_t cmd_32[] = {
    REC(9, ADR_cmd_32_b9_s2), REC(10, ADR_cmd_32_b10_s2), REC(11, ADR_cmd_32_b11_s2),
    REC(12, ADR_cmd_32_b12_s2), REC(13, ADR_cmd_32_b13_s2), REC(14, ADR_cmd_32_b14_s2),
    REC(15, ADR_cmd_32_b15_s2), REC(16, ADR_cmd_32_b16_s2), REC(17, ADR_cmd_32_b17_s2),
    FINE_ELENCO,
};
/* 334E-3356 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_32_b9_s2[] = {
    STRUM(0), DUR(15), PAUSA, PAUSA, PAUSA, DUR(31), PAUSA, FINE,
};
/* 3357-335F flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_32_b10_s2[] = {
    STRUM(0), DUR(15), PAUSA, PAUSA, PAUSA, DUR(31), PAUSA, FINE,
};
/* 3360-339B flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_32_b11_s2[] = {
    STRUM(2), ATTEN(1), TRASP(0), DUR(0), NOTA(3, LA), NOTA(3, SOLd), NOTA(3, SOL),
    NOTA(3, FAd), NOTA(3, FA), NOTA(3, MI), NOTA(3, REd), NOTA(3, RE), NOTA(3, DOd),
    NOTA(3, DO), NOTA(2, SI), NOTA(2, LAd), NOTA(2, LA), NOTA(2, SOLd), NOTA(2, SOL),
    NOTA(2, FAd), NOTA(2, FA), NOTA(2, MI), NOTA(2, REd), NOTA(2, RE), NOTA(2, DOd),
    NOTA(2, DO), NOTA(2, DOd), NOTA(2, RE), NOTA(2, REd), NOTA(2, MI), NOTA(2, FA),
    NOTA(2, FAd), NOTA(2, SOL), NOTA(2, SOLd), NOTA(2, LA), NOTA(2, LAd), NOTA(2, SI),
    NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), NOTA(3, MI), NOTA(3, FA),
    NOTA(3, FAd), NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA), NOTA(3, LAd), NOTA(3, SI),
    NOTA(4, DO), NOTA(4, DOd), NOTA(4, RE), STRUM(0), DUR(31), NOTA(4, REd), FINE,
};
/* 339C-33A4 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_32_b12_s2[] = {
    STRUM(0), DUR(15), PAUSA, PAUSA, PAUSA, DUR(31), PAUSA, FINE,
};
/* 33A5-33AD flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_32_b13_s2[] = {
    STRUM(0), DUR(15), PAUSA, PAUSA, PAUSA, DUR(31), PAUSA, FINE,
};
/* 33AE-33E9 flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_32_b14_s2[] = {
    STRUM(2), ATTEN(5), TRASP(0), DUR(0), NOTA(4, LA), NOTA(4, SOLd), NOTA(4, SOL),
    NOTA(4, FAd), NOTA(4, FA), NOTA(4, MI), NOTA(4, REd), NOTA(4, RE), NOTA(4, DOd),
    NOTA(4, DO), NOTA(3, SI), NOTA(3, LAd), NOTA(3, LA), NOTA(3, SOLd), NOTA(3, SOL),
    NOTA(3, FAd), NOTA(3, FA), NOTA(3, MI), NOTA(3, REd), NOTA(3, RE), NOTA(3, DOd),
    NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), NOTA(3, MI), NOTA(3, FA),
    NOTA(3, FAd), NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA), NOTA(3, LAd), NOTA(3, SI),
    NOTA(4, DO), NOTA(4, DOd), NOTA(4, RE), NOTA(4, REd), NOTA(4, MI), NOTA(4, FA),
    NOTA(4, FAd), NOTA(4, SOL), NOTA(4, SOLd), NOTA(4, LA), NOTA(4, LAd), NOTA(4, SI),
    NOTA(5, DO), NOTA(5, DOd), NOTA(5, RE), STRUM(0), DUR(31), NOTA(5, REd), FINE,
};
/* 33EA-33F2 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_32_b15_s2[] = {
    STRUM(0), DUR(15), PAUSA, PAUSA, PAUSA, DUR(31), PAUSA, FINE,
};
/* 33F3-33FB flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_32_b16_s2[] = {
    STRUM(0), DUR(15), PAUSA, PAUSA, PAUSA, DUR(31), PAUSA, FINE,
};
/* 33FC-3404 flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_32_b17_s2[] = {
    STRUM(0), DUR(15), PAUSA, PAUSA, PAUSA, DUR(31), PAUSA, FINE,
};
/* 3405-340B comando 33: elenco dei record (blocco, flusso) */
static const uint8_t cmd_33[] = {
    REC(10, ADR_cmd_33_b10_s2), REC(13, ADR_cmd_33_b13_s2), FINE_ELENCO,
};
/* 340C-3410 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_33_b10_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3411-3415 flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_33_b13_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3416-3431 comando 34: elenco dei record (blocco, flusso) */
static const uint8_t cmd_34[] = {
    REC(9, ADR_cmd_34_b9_s2), REC(10, ADR_cmd_34_b10_s2), REC(11, ADR_cmd_34_b11_s2),
    REC(12, ADR_cmd_34_b12_s2), REC(13, ADR_cmd_34_b13_s2), REC(14, ADR_cmd_34_b14_s2),
    REC(15, ADR_cmd_34_b15_s2), REC(16, ADR_cmd_34_b16_s2), REC(17, ADR_cmd_34_b17_s2),
    FINE_ELENCO,
};
/* 3432-343D flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_34_b9_s2[] = {
    STRUM(7), ATTEN(3), TRASP(0), DUR(31), NOTA(2, RE), NOTA(3, MI), NOTA(4, FAd),
    NOTA(5, SOLd), FINE,
};
/* 343E-344B flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_34_b10_s2[] = {
    STRUM(7), ATTEN(3), TRASP(0), DUR(7), PAUSA, DUR(31), NOTA(2, FAd), NOTA(3, SOL),
    NOTA(4, LA), NOTA(5, SI), FINE,
};
/* 344C-3459 flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_34_b11_s2[] = {
    STRUM(7), ATTEN(3), TRASP(0), DUR(15), PAUSA, DUR(31), NOTA(2, LA), NOTA(3, SI),
    NOTA(5, DOd), NOTA(6, RE), FINE,
};
/* 345A-3467 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_34_b12_s2[] = {
    STRUM(7), ATTEN(3), TRASP(0), DUR(23), PAUSA, DUR(31), NOTA(3, DOd), NOTA(4, RE),
    NOTA(5, MI), NOTA(6, FAd), FINE,
};
/* 3468-346F flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_34_b13_s2[] = {
    STRUM(0), DUR(31), PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 3470-3477 flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_34_b14_s2[] = {
    STRUM(0), DUR(31), PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 3478-347F flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_34_b15_s2[] = {
    STRUM(0), DUR(31), PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 3480-3487 flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_34_b16_s2[] = {
    STRUM(0), DUR(31), PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 3488-348F flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_34_b17_s2[] = {
    STRUM(0), DUR(31), PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 3490-3496 comando 35: elenco dei record (blocco, flusso) */
static const uint8_t cmd_35[] = {
    REC(9, ADR_cmd_35_b9_s2), REC(10, ADR_cmd_35_b10_s2), FINE_ELENCO,
};
/* 3497-349F flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_35_b9_s2[] = {
    STRUM(0), ATTEN(1), DUR(31), RUMORE(7), NOTA(1, DO), FINE,
};
/* 34A0-34BE flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_35_b10_s2[] = {
    STRUM(2), DUR(1), ATTEN(1), NOTA(0, LAd), ATTEN(2), NOTA(0, SI), ATTEN(3), NOTA(1, DO),
    ATTEN(4), NOTA(1, DOd), ATTEN(6), NOTA(1, RE), ATTEN(8), NOTA(1, REd), ATTEN(10),
    NOTA(1, MI), ATTEN(12), NOTA(1, FA), ATTEN(14), NOTA(1, FAd), FINE,
};
/* 34BF-34C5 comando 36: elenco dei record (blocco, flusso) */
static const uint8_t cmd_36[] = {
    REC(9, ADR_cmd_36_b9_s2), REC(10, ADR_cmd_36_b10_s2), FINE_ELENCO,
};
/* 34C6-34CA flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_36_b9_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 34CB-34CF flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_36_b10_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 34D0-34D3 comando 37: elenco dei record (blocco, flusso) */
static const uint8_t cmd_37[] = {
    REC(9, ADR_cmd_37_b9_s2), FINE_ELENCO,
};
/* 34D4-350B flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_37_b9_s2[] = {
    STRUM(2), ATTEN(2), TRASP(0), DUR(7), RUMORE(0), NOTA(0, DO), RUMORE(2), NOTA(0, DO),
    RUMORE(2), NOTA(0, DO), RUMORE(4), NOTA(0, DO), RUMORE(4), NOTA(0, DO), RUMORE(6),
    NOTA(0, DO), RUMORE(6), NOTA(0, DO), RUMORE(8), NOTA(0, DO), RUMORE(8), NOTA(0, DO),
    RUMORE(10), NOTA(0, DO), RUMORE(10), NOTA(0, DO), RUMORE(12), NOTA(0, DO), RUMORE(12),
    NOTA(0, DO), RUMORE(14), NOTA(0, DO), RUMORE(14), NOTA(0, DO), RUMORE(16), NOTA(0, DO),
    FINE,
};
/* 350C-3527 comando 38: elenco dei record (blocco, flusso) */
static const uint8_t cmd_38[] = {
    REC(9, ADR_cmd_38_b9_s2), REC(10, ADR_cmd_38_b10_s2), REC(11, ADR_cmd_38_b11_s2),
    REC(12, ADR_cmd_38_b12_s2), REC(13, ADR_cmd_38_b13_s2), REC(14, ADR_cmd_38_b14_s2),
    REC(15, ADR_cmd_38_b15_s2), REC(16, ADR_cmd_38_b16_s2), REC(17, ADR_cmd_38_b17_s2),
    FINE_ELENCO,
};
/* 3528-352F flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_38_b9_s2[] = {
    STRUM(9), DUR(31), PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 3530-3537 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_38_b10_s2[] = {
    STRUM(9), DUR(31), PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 3538-354F flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_38_b11_s2[] = {
    STRUM(7), ATTEN(5), TRASP(0), DUR(7), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd),
    NOTA(3, MI), NOTA(3, FA), NOTA(3, FAd), NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA),
    NOTA(3, LAd), NOTA(3, SI), NOTA(4, DO), NOTA(4, DOd), NOTA(4, RE), NOTA(4, REd), FINE,
};
/* 3550-3557 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_38_b12_s2[] = {
    STRUM(0), DUR(31), PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 3558-355F flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_38_b13_s2[] = {
    STRUM(0), DUR(31), PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 3560-3577 flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_38_b14_s2[] = {
    STRUM(7), ATTEN(5), TRASP(0), DUR(7), NOTA(4, DO), NOTA(4, DOd), NOTA(4, RE), NOTA(4, REd),
    NOTA(4, MI), NOTA(4, FA), NOTA(4, FAd), NOTA(4, SOL), NOTA(4, SOLd), NOTA(4, LA),
    NOTA(4, LAd), NOTA(4, SI), NOTA(5, DO), NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd), FINE,
};
/* 3578-357F flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_38_b15_s2[] = {
    STRUM(0), DUR(31), PAUSA, PAUSA, PAUSA, PAUSA, FINE,
};
/* 3580-3599 flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_38_b16_s2[] = {
    STRUM(7), ATTEN(5), TRASP(0), DUR(3), PAUSA, DUR(7), NOTA(3, MI), NOTA(3, FA), NOTA(3, FAd),
    NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA), NOTA(3, LAd), NOTA(3, SI), NOTA(4, DO),
    NOTA(4, DOd), NOTA(4, RE), NOTA(4, REd), NOTA(4, MI), NOTA(4, FA), NOTA(4, FAd),
    NOTA(4, SOL), FINE,
};
/* 359A-35B3 flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_38_b17_s2[] = {
    STRUM(7), ATTEN(5), TRASP(0), DUR(3), PAUSA, DUR(7), NOTA(4, MI), NOTA(4, FA), NOTA(4, FAd),
    NOTA(4, SOL), NOTA(4, SOLd), NOTA(4, LA), NOTA(4, LAd), NOTA(4, SI), NOTA(5, DO),
    NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd), NOTA(5, MI), NOTA(5, FA), NOTA(5, FAd),
    NOTA(5, SOL), FINE,
};
/* 35B4-35CF comando 39: elenco dei record (blocco, flusso) */
static const uint8_t cmd_39[] = {
    REC(9, ADR_cmd_39_b9_s2), REC(10, ADR_cmd_39_b10_s2), REC(11, ADR_cmd_39_b11_s2),
    REC(12, ADR_cmd_39_b12_s2), REC(13, ADR_cmd_39_b13_s2), REC(14, ADR_cmd_39_b14_s2),
    REC(15, ADR_cmd_39_b15_s2), REC(16, ADR_cmd_39_b16_s2), REC(17, ADR_cmd_39_b17_s2),
    FINE_ELENCO,
};
/* 35D0-35F1 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_39_b9_s2[] = {
    STRUM(2), DUR(0), ATTEN(0), NOTA(3, LA), ATTEN(1), NOTA(3, SOLd), ATTEN(2), NOTA(3, SOL),
    ATTEN(3), NOTA(3, FAd), ATTEN(4), NOTA(3, FA), ATTEN(5), NOTA(3, MI), ATTEN(6),
    NOTA(3, REd), ATTEN(7), NOTA(3, RE), ATTEN(8), NOTA(3, DOd), ATTEN(9), NOTA(3, DO), FINE,
};
/* 35F2-3613 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_39_b10_s2[] = {
    STRUM(2), DUR(0), ATTEN(0), NOTA(3, LAd), ATTEN(1), NOTA(3, LA), ATTEN(2), NOTA(3, SOLd),
    ATTEN(3), NOTA(3, SOL), ATTEN(4), NOTA(3, FAd), ATTEN(5), NOTA(3, FA), ATTEN(6),
    NOTA(3, MI), ATTEN(7), NOTA(3, REd), ATTEN(8), NOTA(3, RE), ATTEN(9), NOTA(3, DOd), FINE,
};
/* 3614-3649 flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_39_b11_s2[] = {
    STRUM(2), DUR(0), ATTEN(1), RUMORE(15), NOTA(0, DO), ATTEN(2), RUMORE(14), NOTA(0, DO),
    ATTEN(3), RUMORE(13), NOTA(0, DO), ATTEN(4), RUMORE(12), NOTA(0, DO), ATTEN(5), RUMORE(11),
    NOTA(0, DO), ATTEN(6), RUMORE(10), NOTA(0, DO), ATTEN(7), RUMORE(9), NOTA(0, DO), ATTEN(8),
    RUMORE(8), NOTA(0, DO), ATTEN(9), RUMORE(7), NOTA(0, DO), ATTEN(10), RUMORE(6), NOTA(0, DO),
    FINE,
};
/* 364A-367F flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_39_b12_s2[] = {
    STRUM(2), DUR(0), ATTEN(1), RUMORE(14), NOTA(0, DO), ATTEN(2), RUMORE(13), NOTA(0, DO),
    ATTEN(3), RUMORE(12), NOTA(0, DO), ATTEN(4), RUMORE(11), NOTA(0, DO), ATTEN(5), RUMORE(10),
    NOTA(0, DO), ATTEN(6), RUMORE(9), NOTA(0, DO), ATTEN(7), RUMORE(8), NOTA(0, DO), ATTEN(8),
    RUMORE(7), NOTA(0, DO), ATTEN(9), RUMORE(6), NOTA(0, DO), ATTEN(10), RUMORE(5), NOTA(0, DO),
    FINE,
};
/* 3680-3684 flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_39_b13_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3685-3689 flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_39_b14_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 368A-368E flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_39_b15_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 368F-3693 flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_39_b16_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3694-3698 flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_39_b17_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3699-369C comando 3A: elenco dei record (blocco, flusso) */
static const uint8_t cmd_3A[] = {
    REC(0, ADR_cmd_3A_b0_s1), FINE_ELENCO,
};
/* 369D-36A3 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_3A_b0_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 36A4-36A7 comando 3B: elenco dei record (blocco, flusso) */
static const uint8_t cmd_3B[] = {
    REC(0, ADR_cmd_3B_b0_s1), FINE_ELENCO,
};
/* 36A8-36AE flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_3B_b0_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 36AF-36B2 comando 3C: elenco dei record (blocco, flusso) */
static const uint8_t cmd_3C[] = {
    REC(0, ADR_cmd_3C_b0_s1), FINE_ELENCO,
};
/* 36B3-36B9 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_3C_b0_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 36BA-36BD comando 3D: elenco dei record (blocco, flusso) */
static const uint8_t cmd_3D[] = {
    REC(9, ADR_cmd_3D_b9_s2), FINE_ELENCO,
};
/* 36BE-36EA flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_3D_b9_s2[] = {
    STRUM(0), ATTEN(0), RIP_INIZIO(18), DUR(3), NOTA(5, FA), NOTA(5, MI), NOTA(5, FA),
    NOTA(5, MI), NOTA(5, FA), NOTA(5, MI), NOTA(5, FA), NOTA(5, MI), NOTA(5, FA), NOTA(5, MI),
    NOTA(5, FA), NOTA(5, MI), NOTA(5, FA), NOTA(5, MI), NOTA(5, FA), NOTA(5, MI), NOTA(5, FA),
    NOTA(5, MI), NOTA(5, FA), NOTA(5, MI), NOTA(5, FA), NOTA(5, MI), NOTA(5, FA), NOTA(5, MI),
    NOTA(5, FA), NOTA(5, MI), NOTA(5, FA), NOTA(5, MI), NOTA(5, FA), NOTA(5, MI), NOTA(5, FA),
    NOTA(5, MI), NOTA(5, FA), NOTA(5, MI), DUR(63), NOTA(5, FA), RIP_FINE,
    FINE,
};
/* 36EB-3715 comando 3E: elenco dei record (blocco, flusso) */
static const uint8_t cmd_3E[] = {
    REC(0, ADR_cmd_3E_b0_s1), REC(1, ADR_cmd_3E_b1_s1), REC(2, ADR_cmd_3E_b2_s1),
    REC(3, ADR_cmd_3E_b3_s1), REC(4, ADR_cmd_3E_b4_s1), REC(5, ADR_cmd_3E_b5_s1),
    REC(6, ADR_cmd_3E_b6_s1), REC(7, ADR_cmd_3E_b7_s1), REC(8, ADR_cmd_3E_b8_s1),
    REC(11, ADR_cmd_3E_b11_s2), REC(14, ADR_cmd_3E_b14_s2), REC(15, ADR_cmd_3E_b15_s2),
    REC(16, ADR_cmd_3E_b16_s2), REC(17, ADR_cmd_3E_b17_s2), FINE_ELENCO,
};
/* 3716-371A flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_3E_b0_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 371B-371F flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t cmd_3E_b1_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3720-3724 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_3E_b2_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3725-3729 flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t cmd_3E_b3_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 372A-372E flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t cmd_3E_b4_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 372F-3733 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_3E_b5_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3734-3738 flusso, blocco 6, strato 1 (chip 3 voce 3) */
static const uint8_t cmd_3E_b6_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3739-373D flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_3E_b7_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 373E-3742 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_3E_b8_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3743-3747 flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_3E_b11_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3748-374C flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_3E_b14_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 374D-3751 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_3E_b15_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3752-3756 flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_3E_b16_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 3757-375B flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_3E_b17_s2[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};

const struct aud_pezzo aud_pezzi_spartiti_f[] = {
    PEZZO(cmd_2D),
    PEZZO(cmd_2D_b10_s2),
    PEZZO(cmd_2D_b15_s2),
    PEZZO(cmd_2E),
    PEZZO(cmd_2E_b0_s1),
    PEZZO(cmd_2F),
    PEZZO(cmd_2F_b10_s2),
    PEZZO(cmd_2F_b13_s2),
    PEZZO(cmd_30),
    PEZZO(cmd_30_b10_s2),
    PEZZO(cmd_30_b13_s2),
    PEZZO(cmd_31),
    PEZZO(cmd_31_b10_s2),
    PEZZO(cmd_31_b13_s2),
    PEZZO(cmd_31_b15_s2),
    PEZZO(cmd_32),
    PEZZO(cmd_32_b9_s2),
    PEZZO(cmd_32_b10_s2),
    PEZZO(cmd_32_b11_s2),
    PEZZO(cmd_32_b12_s2),
    PEZZO(cmd_32_b13_s2),
    PEZZO(cmd_32_b14_s2),
    PEZZO(cmd_32_b15_s2),
    PEZZO(cmd_32_b16_s2),
    PEZZO(cmd_32_b17_s2),
    PEZZO(cmd_33),
    PEZZO(cmd_33_b10_s2),
    PEZZO(cmd_33_b13_s2),
    PEZZO(cmd_34),
    PEZZO(cmd_34_b9_s2),
    PEZZO(cmd_34_b10_s2),
    PEZZO(cmd_34_b11_s2),
    PEZZO(cmd_34_b12_s2),
    PEZZO(cmd_34_b13_s2),
    PEZZO(cmd_34_b14_s2),
    PEZZO(cmd_34_b15_s2),
    PEZZO(cmd_34_b16_s2),
    PEZZO(cmd_34_b17_s2),
    PEZZO(cmd_35),
    PEZZO(cmd_35_b9_s2),
    PEZZO(cmd_35_b10_s2),
    PEZZO(cmd_36),
    PEZZO(cmd_36_b9_s2),
    PEZZO(cmd_36_b10_s2),
    PEZZO(cmd_37),
    PEZZO(cmd_37_b9_s2),
    PEZZO(cmd_38),
    PEZZO(cmd_38_b9_s2),
    PEZZO(cmd_38_b10_s2),
    PEZZO(cmd_38_b11_s2),
    PEZZO(cmd_38_b12_s2),
    PEZZO(cmd_38_b13_s2),
    PEZZO(cmd_38_b14_s2),
    PEZZO(cmd_38_b15_s2),
    PEZZO(cmd_38_b16_s2),
    PEZZO(cmd_38_b17_s2),
    PEZZO(cmd_39),
    PEZZO(cmd_39_b9_s2),
    PEZZO(cmd_39_b10_s2),
    PEZZO(cmd_39_b11_s2),
    PEZZO(cmd_39_b12_s2),
    PEZZO(cmd_39_b13_s2),
    PEZZO(cmd_39_b14_s2),
    PEZZO(cmd_39_b15_s2),
    PEZZO(cmd_39_b16_s2),
    PEZZO(cmd_39_b17_s2),
    PEZZO(cmd_3A),
    PEZZO(cmd_3A_b0_s1),
    PEZZO(cmd_3B),
    PEZZO(cmd_3B_b0_s1),
    PEZZO(cmd_3C),
    PEZZO(cmd_3C_b0_s1),
    PEZZO(cmd_3D),
    PEZZO(cmd_3D_b9_s2),
    PEZZO(cmd_3E),
    PEZZO(cmd_3E_b0_s1),
    PEZZO(cmd_3E_b1_s1),
    PEZZO(cmd_3E_b2_s1),
    PEZZO(cmd_3E_b3_s1),
    PEZZO(cmd_3E_b4_s1),
    PEZZO(cmd_3E_b5_s1),
    PEZZO(cmd_3E_b6_s1),
    PEZZO(cmd_3E_b7_s1),
    PEZZO(cmd_3E_b8_s1),
    PEZZO(cmd_3E_b11_s2),
    PEZZO(cmd_3E_b14_s2),
    PEZZO(cmd_3E_b15_s2),
    PEZZO(cmd_3E_b16_s2),
    PEZZO(cmd_3E_b17_s2),
};
const unsigned aud_n_pezzi_spartiti_f =
    sizeof aud_pezzi_spartiti_f / sizeof aud_pezzi_spartiti_f[0];
