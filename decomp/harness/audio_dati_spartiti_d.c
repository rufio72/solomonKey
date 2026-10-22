/* Generato da decomp/estrai_dati.py dalla ROM audio 1.3jk: non modificare
 * a mano. Dati musicali derivati dal programma Tecmo (NOTICE.md). */
/* Elenchi e flussi dei comandi 20-24. */
#include <stddef.h>
#include "audio_dati.h"

/* 2294-22AF comando 20: elenco dei record (blocco, flusso) */
static const uint8_t cmd_20[] = {
    REC(0, ADR_cmd_20_b0_s1), REC(1, ADR_cmd_20_b1_s1), REC(2, ADR_cmd_20_b2_s1),
    REC(3, ADR_cmd_20_b3_s1), REC(4, ADR_cmd_20_b4_s1), REC(5, ADR_cmd_20_b5_s1),
    REC(6, ADR_cmd_20_b6_s1), REC(7, ADR_cmd_20_b7_s1), REC(8, ADR_cmd_20_b8_s1), FINE_ELENCO,
};
/* 22B0-22B4 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_20_b0_s1[] = {
    STRUM(0), ATTEN(4), DUR(41),
};
/* 22B5-22C8 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_20_b0_s1_p2[] = {
    NOTA(3, DO), NOTA(3, SOL), NOTA(4, DO), NOTA(4, SOL), NOTA(2, LA), NOTA(3, MI), NOTA(3, LA),
    NOTA(4, MI), NOTA(2, FA), NOTA(3, DO), NOTA(3, FA), NOTA(4, DO), NOTA(2, SOL), NOTA(3, RE),
    NOTA(3, SOL), NOTA(4, RE), SALTA(ADR_cmd_20_b0_s1_p2),
    B(0xFF),
};
/* 22C9-22CF flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t cmd_20_b1_s1[] = {
    STRUM(0), ATTEN(4), DUR(20), PAUSA, DUR(41),
};
/* 22D0-22E3 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_20_b1_s1_p2[] = {
    NOTA(3, MI), NOTA(3, SI), NOTA(4, MI), NOTA(4, SI), NOTA(3, DO), NOTA(3, SOL), NOTA(4, DO),
    NOTA(4, SOL), NOTA(2, LA), NOTA(3, MI), NOTA(3, LA), NOTA(4, MI), NOTA(2, SI), NOTA(3, FA),
    NOTA(3, SI), NOTA(4, FA), SALTA(ADR_cmd_20_b1_s1_p2),
    B(0xFF),
};
/* 22E4-230D flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_20_b2_s1[] = {
    ATTEN(3), DUR(20), STRUM(6), RUMORE(8), NOTA(0, DO), RUMORE(8), NOTA(0, DO), DUR(41),
    STRUM(0), RUMORE(8), NOTA(0, DO), RUMORE_OFF, PAUSA, RUMORE(8), NOTA(0, DO), RUMORE_OFF,
    PAUSA, RUMORE(8), NOTA(0, DO), DUR(20), RUMORE_OFF, PAUSA, RUMORE(8), NOTA(0, DO),
    RUMORE_OFF, PAUSA, RUMORE(8), NOTA(0, DO), SALTA(ADR_cmd_20_b2_s1),
    B(0xFF),
};
/* 230E-2312 flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t cmd_20_b3_s1[] = {
    STRUM(6), ATTEN(5), DUR(20),
};
/* 2313-2319 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_20_b3_s1_p2[] = {
    RUMORE(1), NOTA(0, DO), SALTA(ADR_cmd_20_b3_s1_p2),
    B(0xFF),
};
/* 231A-231E flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t cmd_20_b4_s1[] = {
    STRUM(0), ATTEN(3), DUR(20),
};
/* 231F-2342 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_20_b4_s1_p2[] = {
    NOTA(2, DO), NOTA(2, DO), NOTA(2, DO), NOTA(2, DO), NOTA(2, DO), NOTA(2, DO), NOTA(2, DO),
    NOTA(2, DO), NOTA(1, LA), NOTA(1, LA), NOTA(1, LA), NOTA(1, LA), NOTA(1, LA), NOTA(1, LA),
    NOTA(1, LA), NOTA(1, LA), NOTA(1, FA), NOTA(1, FA), NOTA(1, FA), NOTA(1, FA), NOTA(1, FA),
    NOTA(1, FA), NOTA(1, FA), NOTA(1, FA), NOTA(1, SOL), NOTA(1, SOL), NOTA(1, SOL),
    NOTA(1, LA), NOTA(1, LA), NOTA(1, LA), NOTA(1, SI), NOTA(1, SI), SALTA(ADR_cmd_20_b4_s1_p2),
    B(0xFF),
};
/* 2343-2352 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_20_b5_s1[] = {
    STRUM(8), ATTEN(2), DUR(20), RIP_INIZIO(4), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA,
    PAUSA, RIP_FINE,
};
/* 2353-23D0 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_20_b5_s1_p2[] = {
    RIP_INIZIO(2), DUR(20), PAUSA, PAUSA, NOTA(3, SOL), NOTA(3, MI), NOTA(3, SOL), NOTA(3, MI),
    NOTA(3, RE), NOTA(3, DO), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA,
    PAUSA, NOTA(3, LA), NOTA(3, FA), NOTA(3, LA), NOTA(3, FA), NOTA(3, MI), NOTA(3, RE), PAUSA,
    NOTA(2, SI), PAUSA, PAUSA, NOTA(3, DO), PAUSA, NOTA(3, RE), PAUSA, RIP_FINE,
    PAUSA, PAUSA, NOTA(3, SOL), NOTA(3, MI), NOTA(3, SOL), NOTA(3, MI), NOTA(3, RE),
    NOTA(3, DO), NOTA(3, LA), PAUSA, NOTA(3, SI), NOTA(3, LA), PAUSA, PAUSA, PAUSA, PAUSA,
    PAUSA, PAUSA, NOTA(3, LA), NOTA(3, FA), NOTA(3, LA), NOTA(3, FA), NOTA(3, MI), NOTA(3, RE),
    DUR(41), NOTA(3, SI), NOTA(3, LA), NOTA(3, SOL), NOTA(3, FA), DUR(20), PAUSA, PAUSA,
    NOTA(3, SOL), NOTA(3, MI), NOTA(3, SOL), NOTA(3, MI), NOTA(3, RE), NOTA(3, DO), NOTA(3, LA),
    PAUSA, NOTA(3, SI), NOTA(3, LA), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, NOTA(3, LA),
    NOTA(3, FA), NOTA(3, LA), NOTA(3, FA), NOTA(3, MI), NOTA(3, RE), DUR(41), NOTA(3, SI),
    NOTA(3, LA), NOTA(3, SOL), NOTA(3, SI), RIP_INIZIO(2), DUR(62), NOTA(4, DO), NOTA(4, DO),
    DUR(41), PAUSA, DUR(62), NOTA(4, DO), NOTA(4, DO), DUR(41), PAUSA, DUR(62), NOTA(4, DO),
    NOTA(4, DO), DUR(41), PAUSA, DUR(20), NOTA(4, RE), NOTA(4, RE), NOTA(4, RE), DUR(41),
    NOTA(4, MI), NOTA(4, RE), DUR(20), PAUSA, RIP_FINE,
    SALTA(ADR_cmd_20_b5_s1_p2),
    B(0xFF),
};
/* 23D1-23E0 flusso, blocco 6, strato 1 (chip 3 voce 3) */
static const uint8_t cmd_20_b6_s1[] = {
    STRUM(8), ATTEN(4), DUR(20), RIP_INIZIO(4), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA,
    PAUSA, RIP_FINE,
};
/* 23E1-245E ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_20_b6_s1_p2[] = {
    RIP_INIZIO(2), DUR(20), PAUSA, PAUSA, NOTA(4, SOL), NOTA(4, MI), NOTA(4, SOL), NOTA(4, MI),
    NOTA(4, RE), NOTA(4, DO), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA,
    PAUSA, NOTA(4, LA), NOTA(4, FA), NOTA(4, LA), NOTA(4, FA), NOTA(4, MI), NOTA(4, RE), PAUSA,
    NOTA(3, SI), PAUSA, PAUSA, NOTA(4, DO), PAUSA, NOTA(4, RE), PAUSA, RIP_FINE,
    PAUSA, PAUSA, NOTA(4, SOL), NOTA(4, MI), NOTA(4, SOL), NOTA(4, MI), NOTA(4, RE),
    NOTA(4, DO), NOTA(4, LA), PAUSA, NOTA(4, SI), NOTA(4, LA), PAUSA, PAUSA, PAUSA, PAUSA,
    PAUSA, PAUSA, NOTA(4, LA), NOTA(4, FA), NOTA(4, LA), NOTA(4, FA), NOTA(4, MI), NOTA(4, RE),
    DUR(41), NOTA(4, SI), NOTA(4, LA), NOTA(4, SOL), NOTA(4, FA), DUR(20), PAUSA, PAUSA,
    NOTA(4, SOL), NOTA(4, MI), NOTA(4, SOL), NOTA(4, MI), NOTA(4, RE), NOTA(4, DO), NOTA(4, LA),
    PAUSA, NOTA(4, SI), NOTA(4, LA), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, NOTA(4, LA),
    NOTA(4, FA), NOTA(4, LA), NOTA(4, FA), NOTA(4, MI), NOTA(4, RE), DUR(41), NOTA(4, SI),
    NOTA(4, LA), NOTA(4, SOL), NOTA(4, SI), RIP_INIZIO(2), DUR(62), NOTA(5, DO), NOTA(5, DO),
    DUR(41), PAUSA, DUR(62), NOTA(5, DO), NOTA(5, DO), DUR(41), PAUSA, DUR(62), NOTA(5, DO),
    NOTA(5, DO), DUR(41), PAUSA, DUR(20), NOTA(5, RE), NOTA(5, RE), NOTA(5, RE), DUR(41),
    NOTA(5, MI), NOTA(5, RE), DUR(20), PAUSA, RIP_FINE,
    SALTA(ADR_cmd_20_b6_s1_p2),
    B(0xFF),
};
/* 245F-246E flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_20_b7_s1[] = {
    STRUM(8), ATTEN(4), DUR(20), RIP_INIZIO(4), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA,
    PAUSA, RIP_FINE,
};
/* 246F-2499 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_20_b7_s1_p2[] = {
    RIP_INIZIO(8), DUR(41), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, RIP_FINE,
    RIP_INIZIO(2), DUR(62), NOTA(4, SOL), NOTA(4, SOL), DUR(41), PAUSA, DUR(62), NOTA(4, LA),
    NOTA(4, LA), DUR(41), PAUSA, DUR(62), NOTA(4, FA), NOTA(4, FA), DUR(41), PAUSA, DUR(20),
    NOTA(4, SOL), NOTA(4, SOL), NOTA(4, SOL), DUR(41), NOTA(4, LA), NOTA(4, SOL), DUR(20),
    PAUSA, RIP_FINE,
    SALTA(ADR_cmd_20_b7_s1_p2),
    B(0xFF),
};
/* 249A-249E flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_20_b8_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 249F-24AB comando 21: elenco dei record (blocco, flusso) */
static const uint8_t cmd_21[] = {
    REC(2, ADR_cmd_21_b2_s1), REC(5, ADR_cmd_21_b5_s1), REC(7, ADR_cmd_21_b7_s1),
    REC(8, ADR_cmd_21_b8_s1), FINE_ELENCO,
};
/* 24AC-24C6 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_21_b2_s1[] = {
    STRUM(4), DUR(11), ATTEN(4), RUMORE(7), NOTA(0, DO), ATTEN(4), RUMORE(7), NOTA(0, DO),
    ATTEN(2), RUMORE(7), NOTA(0, DO), ATTEN(4), RUMORE(7), NOTA(0, DO), SALTA(ADR_cmd_21_b2_s1),
    B(0xFF),
};
/* 24C7-24D2 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_21_b5_s1[] = {
    STRUM(0), DUR(11), ATTEN(3), RIP_INIZIO(4), NOTA(2, DO), NOTA(1, SI), NOTA(2, DO),
    NOTA(2, RE), RIP_FINE,
};
/* 24D3-2519 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_21_b5_s1_p2[] = {
    RIP_INIZIO(16), NOTA(2, DO), RIP_FINE,
    RIP_INIZIO(16), NOTA(1, SI), RIP_FINE,
    RIP_INIZIO(16), NOTA(2, DO), RIP_FINE,
    RIP_INIZIO(16), NOTA(1, SI), RIP_FINE,
    RIP_INIZIO(16), NOTA(2, DO), RIP_FINE,
    RIP_INIZIO(16), NOTA(1, SI), RIP_FINE,
    RIP_INIZIO(16), NOTA(2, DO), RIP_FINE,
    RIP_INIZIO(16), NOTA(1, SI), RIP_FINE,
    RIP_INIZIO(2), NOTA(2, DO), NOTA(2, REd), PAUSA, NOTA(2, RE), NOTA(2, DOd), NOTA(2, DO),
    NOTA(1, SI), NOTA(1, LAd), NOTA(1, LA), NOTA(2, FA), PAUSA, NOTA(2, MI), NOTA(2, REd),
    NOTA(2, RE), NOTA(2, DOd), NOTA(2, DO), NOTA(1, SI), PAUSA, NOTA(2, SOL), NOTA(2, FAd),
    NOTA(2, FA), NOTA(2, MI), NOTA(2, REd), NOTA(2, RE), NOTA(2, DOd), NOTA(2, DO),
    NOTA(2, FAd), NOTA(2, FA), NOTA(2, SOLd), NOTA(2, SOL), NOTA(2, LAd), NOTA(2, LA), RIP_FINE,
    SALTA(ADR_cmd_21_b5_s1_p2),
    B(0xFF),
};
/* 251A-2525 flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_21_b7_s1[] = {
    STRUM(2), DUR(11), ATTEN(4), RIP_INIZIO(4), NOTA(3, DO), NOTA(2, SI), NOTA(3, DO),
    NOTA(3, RE), RIP_FINE,
};
/* 2526-252F ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_21_b7_s1_p2[] = {
    CHIAMA(ADR_cmd_21_b8_s1_p3), CHIAMA(ADR_cmd_21_b8_s1_p4), SALTA(ADR_cmd_21_b7_s1_p2),
    B(0xFF),
};
/* 2530-253D flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_21_b8_s1[] = {
    STRUM(2), DUR(11), ATTEN(5), RIP_INIZIO(4), NOTA(4, DO), NOTA(3, SI), NOTA(4, DO),
    NOTA(4, RE), RIP_FINE,
    TRASP(12),
};
/* 253E-2547 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_21_b8_s1_p2[] = {
    CHIAMA(ADR_cmd_21_b8_s1_p3), CHIAMA(ADR_cmd_21_b8_s1_p4), SALTA(ADR_cmd_21_b8_s1_p2),
    B(0xFF),
};
/* 2548-256B ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_21_b8_s1_p3[] = {
    RIP_INIZIO(4), NOTA(3, DO), NOTA(3, REd), PAUSA, NOTA(3, FAd), NOTA(3, SOL), NOTA(3, FAd),
    NOTA(3, REd), NOTA(3, DO), NOTA(3, SOL), NOTA(3, FAd), NOTA(3, REd), NOTA(3, DO),
    NOTA(3, SOL), NOTA(3, FAd), NOTA(3, REd), NOTA(3, DO), NOTA(2, SI), NOTA(3, RE), PAUSA,
    NOTA(3, FA), NOTA(3, SOL), NOTA(3, FA), NOTA(3, RE), NOTA(2, SI), NOTA(3, SOL), NOTA(3, FA),
    NOTA(3, RE), NOTA(2, SI), NOTA(3, SOL), NOTA(3, FA), NOTA(3, RE), NOTA(2, SI), RIP_FINE,
    RITORNA,
};
/* 256C-258F ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_21_b8_s1_p4[] = {
    RIP_INIZIO(2), NOTA(3, DO), NOTA(3, REd), PAUSA, NOTA(3, RE), NOTA(3, DOd), NOTA(3, DO),
    NOTA(2, SI), NOTA(2, LAd), NOTA(2, LA), NOTA(3, FA), PAUSA, NOTA(3, MI), NOTA(3, REd),
    NOTA(3, RE), NOTA(3, DOd), NOTA(3, DO), NOTA(2, SI), NOTA(3, SOL), PAUSA, NOTA(3, FAd),
    NOTA(3, FA), NOTA(3, MI), NOTA(3, REd), NOTA(3, RE), NOTA(3, DOd), NOTA(3, DO),
    NOTA(3, FAd), NOTA(3, FA), NOTA(3, SOLd), NOTA(3, SOL), NOTA(3, LAd), NOTA(3, LA), RIP_FINE,
    RITORNA,
};
/* 2590-259F comando 22: elenco dei record (blocco, flusso) */
static const uint8_t cmd_22[] = {
    REC(0, ADR_cmd_22_b0_s1), REC(1, ADR_cmd_22_b1_s1), REC(2, ADR_cmd_22_b2_s1),
    REC(3, ADR_cmd_22_b3_s1), REC(4, ADR_cmd_22_b4_s1), FINE_ELENCO,
};
/* 25A0-25EE flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_22_b0_s1[] = {
    STRUM(9), DUR(6), ATTEN(3), RUMORE(16), NOTA(0, DO), RIP_INIZIO(15), ATTEN(5), RUMORE(16),
    NOTA(0, DO), RIP_FINE,
    DUR(13), ATTEN(2), RUMORE(16), NOTA(0, DO), ATTEN(15), RUMORE(0), PAUSA, ATTEN(3),
    RUMORE(16), NOTA(0, DO), ATTEN(3), RUMORE(16), NOTA(0, DO), RIP_INIZIO(2), ATTEN(3),
    RUMORE(16), NOTA(0, DO), ATTEN(15), RUMORE(0), PAUSA, RIP_FINE,
    DUR(6), ATTEN(3), RUMORE(16), NOTA(0, DO), RIP_INIZIO(15), ATTEN(5), RUMORE(16),
    NOTA(0, DO), RIP_FINE,
    DUR(13), RIP_INIZIO(2), ATTEN(2), RUMORE(16), NOTA(0, DO), ATTEN(15), RUMORE(16),
    NOTA(0, DO), RIP_FINE,
    FINE,
};
/* 25EF-2610 flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t cmd_22_b1_s1[] = {
    STRUM(9), ATTEN(6), DUR(13), NOTA(1, LA), PAUSA, PAUSA, NOTA(1, LAd), NOTA(2, DO),
    NOTA(1, FA), PAUSA, NOTA(1, FA), PAUSA, NOTA(2, RE), PAUSA, NOTA(1, LAd), NOTA(2, DO),
    PAUSA, PAUSA, PAUSA, NOTA(1, SOL), PAUSA, NOTA(1, LA), PAUSA, NOTA(1, LAd), PAUSA, PAUSA,
    PAUSA, NOTA(1, LA), PAUSA, NOTA(1, FA), PAUSA, FINE,
};
/* 2611-2632 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_22_b2_s1[] = {
    STRUM(9), ATTEN(3), DUR(13), NOTA(2, LA), PAUSA, PAUSA, NOTA(2, LAd), NOTA(3, DO),
    NOTA(2, FA), PAUSA, NOTA(2, FA), PAUSA, NOTA(3, RE), PAUSA, NOTA(2, LAd), NOTA(3, DO),
    PAUSA, PAUSA, PAUSA, NOTA(2, SOL), PAUSA, NOTA(2, LA), PAUSA, NOTA(2, LAd), PAUSA, PAUSA,
    PAUSA, NOTA(2, LA), PAUSA, NOTA(2, FA), PAUSA, FINE,
};
/* 2633-2654 flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t cmd_22_b3_s1[] = {
    STRUM(9), ATTEN(3), DUR(13), NOTA(3, LA), PAUSA, PAUSA, NOTA(3, LAd), NOTA(4, DO),
    NOTA(3, FA), PAUSA, NOTA(3, FA), PAUSA, NOTA(4, RE), PAUSA, NOTA(3, LAd), NOTA(4, DO),
    PAUSA, PAUSA, PAUSA, NOTA(3, SOL), PAUSA, NOTA(3, LA), PAUSA, NOTA(3, LAd), PAUSA, PAUSA,
    PAUSA, NOTA(3, LA), PAUSA, NOTA(3, FA), PAUSA, FINE,
};
/* 2655-2676 flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t cmd_22_b4_s1[] = {
    STRUM(9), ATTEN(3), DUR(13), NOTA(4, LA), PAUSA, PAUSA, NOTA(4, LAd), NOTA(5, DO),
    NOTA(4, FA), PAUSA, NOTA(4, FA), PAUSA, NOTA(5, RE), PAUSA, NOTA(4, LAd), NOTA(5, DO),
    PAUSA, PAUSA, PAUSA, NOTA(4, SOL), PAUSA, NOTA(4, LA), PAUSA, NOTA(4, LAd), PAUSA, PAUSA,
    PAUSA, NOTA(4, LA), PAUSA, NOTA(4, FA), PAUSA, FINE,
};
/* 2677-2692 comando 23: elenco dei record (blocco, flusso) */
static const uint8_t cmd_23[] = {
    REC(0, ADR_cmd_23_b0_s1), REC(1, ADR_cmd_23_b1_s1), REC(2, ADR_cmd_23_b2_s1),
    REC(3, ADR_cmd_23_b3_s1), REC(4, ADR_cmd_23_b4_s1), REC(5, ADR_cmd_23_b5_s1),
    REC(6, ADR_cmd_23_b6_s1), REC(7, ADR_cmd_23_b7_s1), REC(8, ADR_cmd_23_b8_s1), FINE_ELENCO,
};
/* 2693-269B flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_23_b0_s1[] = {
    STRUM(8), ATTEN(3), DUR(63), PAUSA, PAUSA, PAUSA, NOTA(3, SOL),
};
/* 269C-26D3 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_23_b0_s1_p2[] = {
    NOTA(4, MI), DUR(31), PAUSA, NOTA(4, MI), NOTA(4, FA), NOTA(4, MI), NOTA(4, RE),
    NOTA(4, MI), DUR(63), NOTA(4, RE), NOTA(4, DO), PAUSA, NOTA(3, SOL), NOTA(4, SOL), DUR(31),
    PAUSA, NOTA(4, SOL), NOTA(4, LA), NOTA(4, SOL), NOTA(4, FA), NOTA(4, LA), DUR(63),
    NOTA(4, SOL), PAUSA, PAUSA, NOTA(4, DO), NOTA(4, LA), DUR(31), PAUSA, NOTA(4, LA), DUR(63),
    NOTA(4, SI), DUR(31), NOTA(4, LA), NOTA(4, SI), DUR(63), NOTA(5, DO), NOTA(4, SOL),
    NOTA(4, MI), NOTA(4, DO), DUR(31), NOTA(4, RE), NOTA(4, DOd), NOTA(4, RE), NOTA(4, MI),
    DUR(63), NOTA(4, FA), NOTA(3, SI), NOTA(4, RE), NOTA(4, DO), PAUSA, NOTA(3, SOL),
    SALTA(ADR_cmd_23_b0_s1_p2),
    B(0xFF),
};
/* 26D4-26DC flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t cmd_23_b1_s1[] = {
    STRUM(8), ATTEN(3), DUR(63), PAUSA, PAUSA, PAUSA, NOTA(2, SOL),
};
/* 26DD-2714 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_23_b1_s1_p2[] = {
    NOTA(3, MI), DUR(31), PAUSA, NOTA(3, MI), NOTA(3, FA), NOTA(3, MI), NOTA(3, RE),
    NOTA(3, MI), DUR(63), NOTA(3, RE), NOTA(3, DO), PAUSA, NOTA(2, SOL), NOTA(3, SOL), DUR(31),
    PAUSA, NOTA(3, SOL), NOTA(3, LA), NOTA(3, SOL), NOTA(3, FA), NOTA(3, LA), DUR(63),
    NOTA(3, SOL), PAUSA, PAUSA, NOTA(3, DO), NOTA(3, LA), DUR(31), PAUSA, NOTA(3, LA), DUR(63),
    NOTA(3, SI), DUR(31), NOTA(3, LA), NOTA(3, SI), DUR(63), NOTA(4, DO), NOTA(3, SOL),
    NOTA(3, MI), NOTA(3, DO), DUR(31), NOTA(3, RE), NOTA(3, DOd), NOTA(3, RE), NOTA(3, MI),
    DUR(63), NOTA(3, FA), NOTA(2, SI), NOTA(3, RE), NOTA(3, DO), PAUSA, NOTA(2, SOL),
    SALTA(ADR_cmd_23_b1_s1_p2),
    B(0xFF),
};
/* 2715-271D flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_23_b2_s1[] = {
    STRUM(8), ATTEN(3), DUR(63), PAUSA, PAUSA, PAUSA, NOTA(3, MI),
};
/* 271E-2755 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_23_b2_s1_p2[] = {
    NOTA(4, DO), DUR(31), PAUSA, NOTA(4, DO), NOTA(4, DO), NOTA(4, DO), NOTA(3, SI),
    NOTA(4, DO), DUR(63), NOTA(3, SI), NOTA(3, SOL), PAUSA, NOTA(3, MI), NOTA(4, MI), DUR(31),
    PAUSA, NOTA(4, MI), NOTA(4, FA), NOTA(4, MI), NOTA(4, DO), NOTA(4, FA), DUR(63),
    NOTA(4, MI), PAUSA, PAUSA, NOTA(3, SOL), NOTA(4, FA), DUR(31), PAUSA, NOTA(4, FA), DUR(63),
    NOTA(4, SOL), DUR(31), NOTA(4, FA), NOTA(4, SOL), DUR(63), NOTA(5, SOL), NOTA(4, MI),
    NOTA(4, DO), NOTA(3, SOL), DUR(31), NOTA(3, SI), NOTA(3, LA), NOTA(3, SI), NOTA(4, DO),
    DUR(63), NOTA(4, RE), NOTA(3, SOL), NOTA(3, SI), NOTA(3, SOL), PAUSA, NOTA(3, MI),
    SALTA(ADR_cmd_23_b2_s1_p2),
    B(0xFF),
};
/* 2756-275E flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t cmd_23_b3_s1[] = {
    STRUM(8), ATTEN(3), DUR(63), PAUSA, PAUSA, PAUSA, NOTA(3, DO),
};
/* 275F-2796 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_23_b3_s1_p2[] = {
    NOTA(3, SOL), DUR(31), PAUSA, NOTA(3, SOL), NOTA(3, LA), NOTA(3, SOL), NOTA(3, SOL),
    NOTA(3, SOL), DUR(63), NOTA(3, SOL), NOTA(3, MI), PAUSA, NOTA(3, DO), NOTA(4, DO), DUR(31),
    PAUSA, NOTA(4, DO), NOTA(4, DO), NOTA(4, DO), NOTA(3, LA), NOTA(4, DO), DUR(63),
    NOTA(4, DO), PAUSA, PAUSA, NOTA(3, MI), NOTA(4, DO), DUR(31), PAUSA, NOTA(4, DO), DUR(63),
    NOTA(4, RE), DUR(31), NOTA(4, DO), NOTA(4, RE), DUR(63), NOTA(4, MI), NOTA(4, DO),
    NOTA(3, SOL), NOTA(3, MI), DUR(31), NOTA(3, SOL), NOTA(3, FAd), NOTA(3, SOL), NOTA(3, SOL),
    DUR(63), NOTA(3, SI), NOTA(3, RE), NOTA(3, SOL), NOTA(3, MI), PAUSA, NOTA(3, DO),
    SALTA(ADR_cmd_23_b3_s1_p2),
    B(0xFF),
};
/* 2797-27A1 flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t cmd_23_b4_s1[] = {
    STRUM(8), ATTEN(3), DUR(63), PAUSA, PAUSA, PAUSA, PERIODO(0x00EC),
};
/* 27A2-281F ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_23_b4_s1_p2[] = {
    PERIODO(0x008B), DUR(31), PAUSA, PERIODO(0x008B), PERIODO(0x0083), PERIODO(0x008B),
    PERIODO(0x009D), PERIODO(0x008B), DUR(63), PERIODO(0x009D), PERIODO(0x00B0), PAUSA,
    PERIODO(0x00EC), PERIODO(0x0075), DUR(31), PAUSA, PERIODO(0x0075), PERIODO(0x0068),
    PERIODO(0x0075), PERIODO(0x0083), PERIODO(0x0068), DUR(63), PERIODO(0x0075), PAUSA, PAUSA,
    PERIODO(0x00B0), PERIODO(0x0068), DUR(31), PAUSA, PERIODO(0x0068), DUR(63), PERIODO(0x005D),
    DUR(31), PERIODO(0x0068), PERIODO(0x005D), DUR(63), PERIODO(0x0058), PERIODO(0x0075),
    PERIODO(0x008B), PERIODO(0x00B0), DUR(31), PERIODO(0x009D), PERIODO(0x00A6),
    PERIODO(0x009D), PERIODO(0x008B), DUR(63), PERIODO(0x0083), PERIODO(0x00BA),
    PERIODO(0x009D), PERIODO(0x00B0), PAUSA, PERIODO(0x00EC), SALTA(ADR_cmd_23_b4_s1_p2),
    B(0xFF),
};
/* 2820-2828 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_23_b5_s1[] = {
    STRUM(8), ATTEN(2), DUR(63), PAUSA, PAUSA, PAUSA, PAUSA,
};
/* 2829-284C ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_23_b5_s1_p2[] = {
    NOTA(2, DO), NOTA(1, SOL), NOTA(1, LA), NOTA(1, SI), NOTA(2, DO), NOTA(1, SOL), NOTA(1, MI),
    NOTA(1, SOL), NOTA(2, DO), NOTA(1, SOL), NOTA(1, LA), NOTA(1, FA), NOTA(1, SOL),
    NOTA(1, LA), NOTA(1, SI), NOTA(1, SOL), NOTA(2, DO), NOTA(1, LA), NOTA(2, RE), NOTA(1, SI),
    NOTA(2, DO), NOTA(1, SOL), NOTA(2, DO), NOTA(2, MI), NOTA(2, RE), NOTA(1, SI), NOTA(1, SOL),
    NOTA(1, SI), NOTA(2, DO), NOTA(1, SOL), NOTA(1, MI), NOTA(1, SOL),
    SALTA(ADR_cmd_23_b5_s1_p2),
    B(0xFF),
};
/* 284D-2873 flusso, blocco 6, strato 1 (chip 3 voce 3) */
static const uint8_t cmd_23_b6_s1[] = {
    STRUM(11), DUR(31), PAUSA, DUR(7), ATTEN(4), RIP_INIZIO(4), RUMORE(4), NOTA(0, DO),
    RIP_FINE,
    ATTEN(3), RIP_INIZIO(4), RUMORE(4), NOTA(0, DO), RIP_FINE,
    ATTEN(2), RIP_INIZIO(4), RUMORE(4), NOTA(0, DO), RIP_FINE,
    DUR(31), ATTEN(1), RUMORE(8), NOTA(0, DO), RUMORE_OFF, PAUSA, PAUSA, PAUSA,
};
/* 2874-28FD ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_23_b6_s1_p2[] = {
    STRUM(11), ATTEN(3), DUR(31), RUMORE_OFF, PAUSA, DUR(7), RIP_INIZIO(4), RUMORE(4),
    NOTA(0, DO), RIP_FINE,
    DUR(31), RUMORE(8), NOTA(0, DO), RUMORE(8), NOTA(0, DO), ATTEN(2), DUR(31), RIP_INIZIO(2),
    RUMORE_OFF, PAUSA, RUMORE(8), NOTA(0, DO), RIP_FINE,
    RIP_INIZIO(3), ATTEN(3), DUR(31), RUMORE_OFF, PAUSA, DUR(7), RUMORE(4), NOTA(0, DO),
    RUMORE(4), NOTA(0, DO), RUMORE(4), NOTA(0, DO), RUMORE(4), NOTA(0, DO), RIP_FINE,
    ATTEN(2), DUR(31), RUMORE(8), NOTA(0, DO), RUMORE(8), NOTA(0, DO), DUR(31), RUMORE_OFF,
    PAUSA, ATTEN(3), DUR(7), RIP_INIZIO(4), RUMORE(4), NOTA(0, DO), RIP_FINE,
    DUR(31), RUMORE(8), NOTA(0, DO), RUMORE(8), NOTA(0, DO), RIP_INIZIO(2), ATTEN(2),
    RUMORE_OFF, PAUSA, RUMORE(8), NOTA(0, DO), RIP_FINE,
    RUMORE_OFF, PAUSA, DUR(7), ATTEN(4), RIP_INIZIO(4), RUMORE(4), NOTA(0, DO), RIP_FINE,
    ATTEN(3), RIP_INIZIO(4), RUMORE(4), NOTA(0, DO), RIP_FINE,
    ATTEN(2), RIP_INIZIO(4), RUMORE(4), NOTA(0, DO), RIP_FINE,
    DUR(31), ATTEN(1), RUMORE(8), NOTA(0, DO), RUMORE_OFF, PAUSA, ATTEN(0), DUR(31), RUMORE(8),
    NOTA(0, DO), RUMORE_OFF, PAUSA, SALTA(ADR_cmd_23_b6_s1_p2),
    B(0xFF),
};
/* 28FE-2906 flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_23_b7_s1[] = {
    STRUM(14), ATTEN(5), DUR(63), PAUSA, PAUSA, PAUSA, PAUSA,
};
/* 2907-296E ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_23_b7_s1_p2[] = {
    DUR(15), RIP_INIZIO(2), NOTA(6, DO), NOTA(5, SOL), NOTA(5, MI), NOTA(5, SOL), NOTA(6, DO),
    NOTA(5, SOL), NOTA(5, MI), NOTA(5, SOL), NOTA(6, DO), NOTA(5, LA), NOTA(5, FA), NOTA(5, LA),
    NOTA(6, DO), NOTA(5, LA), NOTA(5, FA), NOTA(5, LA), NOTA(6, DO), NOTA(5, SOL), NOTA(5, MI),
    NOTA(5, SOL), NOTA(6, DO), NOTA(5, SOL), NOTA(5, MI), NOTA(5, SOL), NOTA(5, DO),
    NOTA(5, MI), NOTA(4, SOL), NOTA(5, DO), NOTA(5, DO), NOTA(5, MI), NOTA(4, SOL), NOTA(5, DO),
    RIP_FINE,
    NOTA(6, DO), NOTA(5, LA), NOTA(5, FA), NOTA(6, DO), NOTA(6, DO), NOTA(5, LA), NOTA(5, FA),
    NOTA(6, DO), NOTA(6, RE), NOTA(5, SI), NOTA(5, SOL), NOTA(6, RE), NOTA(6, RE), NOTA(5, SI),
    NOTA(5, SOL), NOTA(6, RE), NOTA(6, MI), NOTA(6, DO), NOTA(5, SOL), NOTA(6, DO), NOTA(6, MI),
    NOTA(6, DO), NOTA(5, SOL), NOTA(6, DO), NOTA(5, MI), NOTA(5, SOL), NOTA(5, DO), NOTA(5, MI),
    NOTA(5, MI), NOTA(5, SOL), NOTA(5, DO), NOTA(5, MI), NOTA(5, LA), NOTA(5, FA), NOTA(5, RE),
    NOTA(5, LA), NOTA(5, LA), NOTA(5, FA), NOTA(5, RE), NOTA(5, LA), NOTA(5, SI), NOTA(5, SOL),
    NOTA(5, RE), NOTA(5, SI), NOTA(5, SI), NOTA(5, SOL), NOTA(5, RE), NOTA(5, SI), NOTA(6, DO),
    NOTA(5, SOL), NOTA(5, MI), NOTA(5, SOL), NOTA(6, DO), NOTA(5, SOL), NOTA(5, MI),
    NOTA(5, SOL), NOTA(5, DO), NOTA(5, MI), NOTA(4, SOL), NOTA(5, DO), NOTA(5, DO), NOTA(5, MI),
    NOTA(4, SOL), NOTA(5, DO), SALTA(ADR_cmd_23_b7_s1_p2),
    B(0xFF),
};
/* 296F-2973 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_23_b8_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2974-298F comando 24: elenco dei record (blocco, flusso) */
static const uint8_t cmd_24[] = {
    REC(0, ADR_cmd_24_b0_s1), REC(1, ADR_cmd_24_b1_s1), REC(2, ADR_cmd_24_b2_s1),
    REC(3, ADR_cmd_24_b3_s1), REC(4, ADR_cmd_24_b4_s1), REC(5, ADR_cmd_24_b5_s1),
    REC(6, ADR_cmd_24_b6_s1), REC(7, ADR_cmd_24_b7_s1), REC(8, ADR_cmd_24_b8_s1), FINE_ELENCO,
};
/* 2990-2994 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_24_b0_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2995-2999 flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t cmd_24_b1_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 299A-29AF flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_24_b2_s1[] = {
    STRUM(2), TRASP(0), ATTEN(5), DUR(6), NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA),
    NOTA(3, LAd), NOTA(3, SI), NOTA(4, DO), NOTA(4, DOd), NOTA(4, RE), STRUM(7), TRASP(0),
    ATTEN(4), DUR(13),
};
/* 29B0-29D3 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_24_b2_s1_p2[] = {
    NOTA(4, REd), NOTA(4, DO), NOTA(4, DO), NOTA(4, FA), NOTA(4, DO), NOTA(4, DO), NOTA(4, SOL),
    NOTA(4, DO), NOTA(4, LAd), NOTA(4, REd), NOTA(4, REd), NOTA(4, LA), NOTA(4, RE),
    NOTA(4, RE), NOTA(4, SOLd), NOTA(4, DOd), NOTA(4, REd), NOTA(4, DO), NOTA(4, DO),
    NOTA(4, FA), NOTA(4, DO), NOTA(4, DO), NOTA(4, SOL), NOTA(4, DO), NOTA(3, SI), NOTA(4, DO),
    NOTA(4, RE), NOTA(3, SOL), NOTA(3, LA), NOTA(3, SI), NOTA(4, DO), NOTA(3, SI),
    SALTA(ADR_cmd_24_b2_s1_p2),
    B(0xFF),
};
/* 29D4-29D8 flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t cmd_24_b3_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 29D9-29DD flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t cmd_24_b4_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 29DE-29F3 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_24_b5_s1[] = {
    STRUM(2), TRASP(0), ATTEN(5), DUR(6), NOTA(3, MI), NOTA(3, FA), NOTA(3, FAd), NOTA(3, SOL),
    NOTA(3, SOLd), NOTA(3, LA), NOTA(3, LAd), NOTA(3, SI), STRUM(7), TRASP(0), ATTEN(4),
    DUR(13),
};
/* 29F4-2A17 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_24_b5_s1_p2[] = {
    NOTA(4, DO), NOTA(3, SOL), NOTA(3, SOL), NOTA(4, DO), NOTA(3, SOL), NOTA(3, SOL),
    NOTA(4, REd), NOTA(3, SOL), NOTA(4, FAd), NOTA(3, LAd), NOTA(3, LAd), NOTA(4, FA),
    NOTA(3, LA), NOTA(3, LA), NOTA(4, MI), NOTA(3, SOLd), NOTA(4, DO), NOTA(3, SOL),
    NOTA(3, SOL), NOTA(4, DO), NOTA(3, SOL), NOTA(3, SOL), NOTA(4, REd), NOTA(3, SOL),
    NOTA(3, SOL), NOTA(3, LA), NOTA(3, SI), NOTA(3, MI), NOTA(3, FA), NOTA(3, SOL), NOTA(3, LA),
    NOTA(3, SOL), SALTA(ADR_cmd_24_b5_s1_p2),
    B(0xFF),
};
/* 2A18-2A1C flusso, blocco 6, strato 1 (chip 3 voce 3) */
static const uint8_t cmd_24_b6_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 2A1D-2A32 flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_24_b7_s1[] = {
    STRUM(2), TRASP(0), ATTEN(5), DUR(6), NOTA(2, SI), NOTA(3, DO), NOTA(3, DOd), NOTA(3, RE),
    NOTA(3, REd), NOTA(3, MI), NOTA(3, FA), NOTA(3, FAd), STRUM(7), TRASP(0), ATTEN(4), DUR(13),
};
/* 2A33-2A56 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_24_b7_s1_p2[] = {
    NOTA(3, SOL), NOTA(3, REd), NOTA(3, REd), NOTA(3, SOLd), NOTA(3, REd), NOTA(3, REd),
    NOTA(4, DO), NOTA(3, REd), NOTA(4, REd), NOTA(3, FAd), NOTA(3, FAd), NOTA(4, RE),
    NOTA(3, FA), NOTA(3, FA), NOTA(4, DOd), NOTA(3, MI), NOTA(3, SOL), NOTA(3, REd),
    NOTA(3, REd), NOTA(3, SOLd), NOTA(3, REd), NOTA(3, REd), NOTA(4, DO), NOTA(3, REd),
    NOTA(3, MI), NOTA(3, FA), NOTA(3, SOL), NOTA(3, DO), NOTA(3, RE), NOTA(3, MI), NOTA(3, FA),
    NOTA(3, MI), SALTA(ADR_cmd_24_b7_s1_p2),
    B(0xFF),
};
/* 2A57-2A6C flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_24_b8_s1[] = {
    STRUM(2), TRASP(0), ATTEN(2), DUR(6), NOTA(1, SOL), NOTA(1, SOLd), NOTA(1, LA),
    NOTA(1, LAd), NOTA(1, SI), NOTA(2, DO), NOTA(2, DOd), NOTA(2, RE), STRUM(7), TRASP(0),
    ATTEN(2), DUR(13),
};
/* 2A6D-2A90 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_24_b8_s1_p2[] = {
    NOTA(2, REd), NOTA(2, DO), NOTA(2, DO), NOTA(2, FA), NOTA(2, DO), NOTA(2, DO), NOTA(2, SOL),
    NOTA(2, DO), NOTA(2, LAd), NOTA(2, REd), NOTA(2, REd), NOTA(2, LA), NOTA(2, RE),
    NOTA(2, RE), NOTA(2, SOLd), NOTA(2, DOd), NOTA(2, REd), NOTA(2, DO), NOTA(2, DO),
    NOTA(2, FA), NOTA(2, DO), NOTA(2, DO), NOTA(2, SOL), NOTA(2, DO), NOTA(1, SI), NOTA(2, DO),
    NOTA(2, RE), NOTA(1, SOL), NOTA(1, LA), NOTA(1, SI), NOTA(2, DO), NOTA(1, SI),
    SALTA(ADR_cmd_24_b8_s1_p2),
    B(0xFF),
};

const struct aud_pezzo aud_pezzi_spartiti_d[] = {
    PEZZO(cmd_20),
    PEZZO(cmd_20_b0_s1),
    PEZZO(cmd_20_b0_s1_p2),
    PEZZO(cmd_20_b1_s1),
    PEZZO(cmd_20_b1_s1_p2),
    PEZZO(cmd_20_b2_s1),
    PEZZO(cmd_20_b3_s1),
    PEZZO(cmd_20_b3_s1_p2),
    PEZZO(cmd_20_b4_s1),
    PEZZO(cmd_20_b4_s1_p2),
    PEZZO(cmd_20_b5_s1),
    PEZZO(cmd_20_b5_s1_p2),
    PEZZO(cmd_20_b6_s1),
    PEZZO(cmd_20_b6_s1_p2),
    PEZZO(cmd_20_b7_s1),
    PEZZO(cmd_20_b7_s1_p2),
    PEZZO(cmd_20_b8_s1),
    PEZZO(cmd_21),
    PEZZO(cmd_21_b2_s1),
    PEZZO(cmd_21_b5_s1),
    PEZZO(cmd_21_b5_s1_p2),
    PEZZO(cmd_21_b7_s1),
    PEZZO(cmd_21_b7_s1_p2),
    PEZZO(cmd_21_b8_s1),
    PEZZO(cmd_21_b8_s1_p2),
    PEZZO(cmd_21_b8_s1_p3),
    PEZZO(cmd_21_b8_s1_p4),
    PEZZO(cmd_22),
    PEZZO(cmd_22_b0_s1),
    PEZZO(cmd_22_b1_s1),
    PEZZO(cmd_22_b2_s1),
    PEZZO(cmd_22_b3_s1),
    PEZZO(cmd_22_b4_s1),
    PEZZO(cmd_23),
    PEZZO(cmd_23_b0_s1),
    PEZZO(cmd_23_b0_s1_p2),
    PEZZO(cmd_23_b1_s1),
    PEZZO(cmd_23_b1_s1_p2),
    PEZZO(cmd_23_b2_s1),
    PEZZO(cmd_23_b2_s1_p2),
    PEZZO(cmd_23_b3_s1),
    PEZZO(cmd_23_b3_s1_p2),
    PEZZO(cmd_23_b4_s1),
    PEZZO(cmd_23_b4_s1_p2),
    PEZZO(cmd_23_b5_s1),
    PEZZO(cmd_23_b5_s1_p2),
    PEZZO(cmd_23_b6_s1),
    PEZZO(cmd_23_b6_s1_p2),
    PEZZO(cmd_23_b7_s1),
    PEZZO(cmd_23_b7_s1_p2),
    PEZZO(cmd_23_b8_s1),
    PEZZO(cmd_24),
    PEZZO(cmd_24_b0_s1),
    PEZZO(cmd_24_b1_s1),
    PEZZO(cmd_24_b2_s1),
    PEZZO(cmd_24_b2_s1_p2),
    PEZZO(cmd_24_b3_s1),
    PEZZO(cmd_24_b4_s1),
    PEZZO(cmd_24_b5_s1),
    PEZZO(cmd_24_b5_s1_p2),
    PEZZO(cmd_24_b6_s1),
    PEZZO(cmd_24_b7_s1),
    PEZZO(cmd_24_b7_s1_p2),
    PEZZO(cmd_24_b8_s1),
    PEZZO(cmd_24_b8_s1_p2),
};
const unsigned aud_n_pezzi_spartiti_d =
    sizeof aud_pezzi_spartiti_d / sizeof aud_pezzi_spartiti_d[0];
