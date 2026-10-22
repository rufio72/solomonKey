/* Generato da decomp/estrai_dati.py dalla ROM audio 1.3jk: non modificare
 * a mano. Dati musicali derivati dal programma Tecmo (NOTICE.md). */
/* Elenchi e flussi dei comandi 13-1A. */
#include <stddef.h>
#include "audio_dati.h"

/* 10C5-10D1 comando 13: elenco dei record (blocco, flusso) */
static const uint8_t cmd_13[] = {
    REC(2, ADR_cmd_1A_b2_s1), REC(5, ADR_cmd_13_b5_s1), REC(7, ADR_cmd_13_b7_s1),
    REC(8, ADR_cmd_13_b8_s1), FINE_ELENCO,
};
/* 10D2-10E6 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_13_b5_s1[] = {
    STRUM(8), ATTEN(3), DUR(21), NOTA(2, REd), NOTA(2, MI), NOTA(2, FA), NOTA(2, FAd),
    NOTA(2, SOL), NOTA(2, SOLd), NOTA(2, LA), NOTA(2, LAd), NOTA(2, SI), NOTA(3, DO),
    NOTA(3, DOd), NOTA(3, RE), NOTA(3, REd), NOTA(3, MI), NOTA(3, FA), NOTA(3, FAd),
};
/* 10E7-1120 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_13_b5_s1_p2[] = {
    RIP_INIZIO(4), NOTA(2, DO), NOTA(3, DO), NOTA(2, DO), NOTA(3, DO), NOTA(1, SOL),
    NOTA(2, SOL), NOTA(1, SOL), NOTA(2, SOL), NOTA(1, SI), NOTA(2, SI), NOTA(1, SI),
    NOTA(2, SI), NOTA(1, LA), NOTA(2, LA), NOTA(1, LA), NOTA(2, LA), RIP_FINE,
    RIP_INIZIO(2), NOTA(2, DO), NOTA(3, DO), NOTA(2, DO), NOTA(3, DO), NOTA(1, SOL),
    NOTA(2, SOL), NOTA(1, SOL), NOTA(2, SOL), NOTA(2, DO), NOTA(3, DO), NOTA(2, DO),
    NOTA(3, DO), NOTA(1, SOL), NOTA(2, SOL), NOTA(1, SOL), NOTA(2, SOL), NOTA(2, DO),
    NOTA(3, DO), NOTA(2, DO), NOTA(3, DO), NOTA(1, SOL), NOTA(2, SOL), NOTA(1, SOL),
    NOTA(2, SOL), NOTA(1, FA), NOTA(2, FA), NOTA(1, FA), NOTA(2, FA), NOTA(1, SOL),
    NOTA(2, SOL), NOTA(1, SOL), NOTA(2, SOL), RIP_FINE,
    SALTA(ADR_cmd_13_b5_s1_p2),
    B(0xFF),
};
/* 1121-1135 flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_13_b7_s1[] = {
    STRUM(1), ATTEN(3), DUR(21), NOTA(3, REd), NOTA(3, MI), NOTA(3, FA), NOTA(3, FAd),
    NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA), NOTA(3, LAd), NOTA(3, SI), NOTA(4, DO),
    NOTA(4, DOd), NOTA(4, RE), NOTA(4, REd), NOTA(4, MI), NOTA(4, FA), NOTA(4, FAd),
};
/* 1136-113C ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_13_b7_s1_p2[] = {
    CHIAMA(ADR_cmd_13_b8_s1_p3), SALTA(ADR_cmd_13_b7_s1_p2),
    B(0xFF),
};
/* 113D-1153 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_13_b8_s1[] = {
    STRUM(8), ATTEN(3), DUR(21), TRASP(12), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA,
    PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA,
};
/* 1154-115A ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_13_b8_s1_p2[] = {
    CHIAMA(ADR_cmd_13_b8_s1_p3), SALTA(ADR_cmd_13_b8_s1_p2),
    B(0xFF),
};
/* 115B-11BE ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_13_b8_s1_p3[] = {
    NOTA(3, SOL), PAUSA, PAUSA, NOTA(4, DO), NOTA(4, DO), PAUSA, NOTA(4, DO), PAUSA,
    NOTA(4, RE), PAUSA, NOTA(4, DO), NOTA(4, RE), NOTA(4, MI), PAUSA, NOTA(4, RE), NOTA(4, DO),
    NOTA(3, SOL), PAUSA, PAUSA, NOTA(4, DO), NOTA(4, DO), PAUSA, NOTA(4, DO), PAUSA,
    NOTA(4, RE), PAUSA, NOTA(4, DO), NOTA(4, RE), NOTA(4, MI), PAUSA, PAUSA, PAUSA,
    NOTA(3, SOL), PAUSA, PAUSA, NOTA(4, DO), NOTA(4, DO), PAUSA, NOTA(4, DO), PAUSA,
    NOTA(4, RE), PAUSA, NOTA(4, DO), NOTA(4, RE), NOTA(4, MI), PAUSA, NOTA(4, RE), NOTA(4, DO),
    NOTA(3, SOL), PAUSA, PAUSA, NOTA(4, DO), NOTA(4, DO), PAUSA, NOTA(4, DO), PAUSA,
    NOTA(4, RE), PAUSA, NOTA(4, DO), NOTA(4, RE), NOTA(4, DO), PAUSA, PAUSA, PAUSA,
    RIP_INIZIO(2), NOTA(4, SOL), PAUSA, NOTA(4, SOL), PAUSA, NOTA(4, MI), NOTA(4, RE),
    NOTA(4, DO), PAUSA, NOTA(4, SOL), PAUSA, NOTA(4, SOL), PAUSA, NOTA(4, MI), NOTA(4, RE),
    NOTA(4, DO), PAUSA, NOTA(4, SOL), PAUSA, NOTA(4, SOL), PAUSA, NOTA(4, MI), NOTA(4, RE),
    NOTA(4, DO), PAUSA, NOTA(3, SI), PAUSA, NOTA(4, DO), PAUSA, NOTA(4, RE), PAUSA, NOTA(3, SI),
    PAUSA, RIP_FINE,
    RITORNA,
};
/* 11BF-11CB comando 14: elenco dei record (blocco, flusso) */
static const uint8_t cmd_14[] = {
    REC(2, ADR_cmd_14_b2_s1), REC(5, ADR_cmd_14_b5_s1), REC(7, ADR_cmd_14_b7_s1),
    REC(8, ADR_cmd_14_b8_s1), FINE_ELENCO,
};
/* 11CC-1247 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_14_b2_s1[] = {
    STRUM(2), RIP_INIZIO(3), DUR(0), ATTEN(3), RUMORE(7), NOTA(0, DO), ATTEN(6), RUMORE(6),
    NOTA(0, DO), ATTEN(9), RUMORE(5), NOTA(0, DO), ATTEN(12), RUMORE(4), NOTA(0, DO), DUR(11),
    ATTEN(15), RUMORE(0), PAUSA, RIP_FINE,
    STRUM(1), DUR(15), ATTEN(2), RUMORE(7), NOTA(0, DO), STRUM(2), RIP_INIZIO(4), DUR(0),
    ATTEN(3), RUMORE(7), NOTA(0, DO), ATTEN(6), RUMORE(6), NOTA(0, DO), ATTEN(9), RUMORE(5),
    NOTA(0, DO), ATTEN(12), RUMORE(4), NOTA(0, DO), DUR(11), ATTEN(15), RUMORE(0), PAUSA,
    RIP_FINE,
    STRUM(1), DUR(15), ATTEN(2), RUMORE(7), NOTA(0, DO), STRUM(2), RIP_INIZIO(2), DUR(0),
    ATTEN(3), RUMORE(7), NOTA(0, DO), ATTEN(6), RUMORE(6), NOTA(0, DO), ATTEN(9), RUMORE(5),
    NOTA(0, DO), ATTEN(12), RUMORE(4), NOTA(0, DO), DUR(11), ATTEN(15), RUMORE(0), PAUSA,
    RIP_FINE,
    STRUM(1), DUR(15), ATTEN(2), RUMORE(7), NOTA(0, DO), SALTA(ADR_cmd_14_b2_s1),
    B(0xFF),
};
/* 1248-125B flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_14_b5_s1[] = {
    STRUM(2), ATTEN(4), TRASP(-3), DUR(15), CHIAMA(ADR_cmd_14_b5_s1_p2),
    CHIAMA(ADR_cmd_14_b5_s1_p2), CHIAMA(ADR_cmd_14_b5_s1_p3), SALTA(ADR_cmd_14_b5_s1),
    B(0xFF),
};
/* 125C-1286 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_14_b5_s1_p2[] = {
    RIP_INIZIO(3), NOTA(2, SOLd), NOTA(3, DO), NOTA(3, REd), RIP_FINE,
    NOTA(2, LA), NOTA(3, DO), NOTA(3, FA), RIP_INIZIO(3), NOTA(2, SOLd), NOTA(3, DO),
    NOTA(3, REd), RIP_FINE,
    NOTA(2, LA), NOTA(3, DO), NOTA(3, FA), RIP_INIZIO(2), NOTA(2, LAd), NOTA(3, RE),
    NOTA(3, FA), RIP_FINE,
    RIP_INIZIO(2), NOTA(3, DO), NOTA(3, REd), NOTA(3, SOL), RIP_FINE,
    RIP_INIZIO(2), NOTA(2, SOLd), NOTA(3, DO), NOTA(3, REd), RIP_FINE,
    RIP_INIZIO(2), NOTA(2, SOLd), NOTA(3, DO), NOTA(3, FA), RIP_FINE,
    RITORNA,
};
/* 1287-1296 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_14_b5_s1_p3[] = {
    RIP_INIZIO(8), NOTA(3, DO), NOTA(3, MI), NOTA(3, SOL), NOTA(3, DO), NOTA(3, MI),
    NOTA(3, SOL), NOTA(3, DO), NOTA(3, FA), NOTA(3, LA), NOTA(3, DO), NOTA(3, FA), NOTA(3, LA),
    RIP_FINE,
    RITORNA,
};
/* 1297-129D flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_14_b7_s1[] = {
    STRUM(7), ATTEN(3), DUR(15), TRASP(-3),
};
/* 129E-12A4 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_14_b7_s1_p2[] = {
    CHIAMA(ADR_cmd_14_b8_s1_p3), SALTA(ADR_cmd_14_b7_s1_p2),
    B(0xFF),
};
/* 12A5-12AB flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_14_b8_s1[] = {
    STRUM(8), ATTEN(4), DUR(15), TRASP(9),
};
/* 12AC-12B2 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_14_b8_s1_p2[] = {
    CHIAMA(ADR_cmd_14_b8_s1_p3), SALTA(ADR_cmd_14_b8_s1_p2),
    B(0xFF),
};
/* 12B3-1335 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_14_b8_s1_p3[] = {
    DUR(15), RIP_INIZIO(2), NOTA(4, MI), NOTA(4, REd), NOTA(4, DO), NOTA(3, SOLd), NOTA(3, FA),
    NOTA(3, SOLd), NOTA(4, DO), NOTA(4, REd), NOTA(4, MI), NOTA(4, FA), PAUSA, PAUSA, RIP_FINE,
    NOTA(4, RE), NOTA(4, LAd), NOTA(4, RE), NOTA(4, LAd), NOTA(4, RE), NOTA(4, LAd),
    NOTA(4, SOLd), NOTA(4, SOL), PAUSA, PAUSA, PAUSA, PAUSA, NOTA(4, DO), NOTA(4, SOLd),
    NOTA(4, DO), NOTA(4, SOLd), NOTA(4, DO), NOTA(4, SOLd), NOTA(4, SOL), NOTA(4, FA), PAUSA,
    PAUSA, PAUSA, PAUSA, RIP_INIZIO(2), NOTA(4, MI), NOTA(4, REd), NOTA(4, DO), NOTA(3, SOLd),
    NOTA(3, FA), NOTA(3, SOLd), NOTA(4, DO), NOTA(4, REd), NOTA(4, MI), NOTA(4, FA), PAUSA,
    PAUSA, RIP_FINE,
    NOTA(4, RE), NOTA(4, LAd), NOTA(4, RE), NOTA(4, LAd), NOTA(4, RE), NOTA(4, LAd),
    NOTA(4, SOLd), NOTA(4, SOL), PAUSA, PAUSA, NOTA(4, FA), NOTA(4, SOL), NOTA(4, SOLd),
    NOTA(4, DO), NOTA(4, SOLd), NOTA(4, DO), NOTA(4, SOLd), NOTA(4, DO), PAUSA, PAUSA, PAUSA,
    PAUSA, PAUSA, PAUSA, RIP_INIZIO(2), NOTA(4, LAd), NOTA(4, SOL), NOTA(4, MI), NOTA(4, DO),
    NOTA(4, MI), NOTA(4, SOL), NOTA(5, DO), NOTA(4, LA), NOTA(4, FA), NOTA(5, DO), NOTA(4, LA),
    PAUSA, NOTA(4, LAd), NOTA(4, SOL), NOTA(4, MI), NOTA(4, DO), NOTA(4, MI), NOTA(4, SOL),
    NOTA(4, LA), NOTA(4, FA), NOTA(5, DO), PAUSA, PAUSA, PAUSA, NOTA(4, LAd), NOTA(4, SOL),
    NOTA(4, MI), NOTA(4, DO), NOTA(4, MI), NOTA(4, SOL), NOTA(5, DO), NOTA(4, LA), NOTA(5, DO),
    NOTA(4, LA), NOTA(5, DO), NOTA(4, LA), NOTA(4, LAd), NOTA(4, SOL), NOTA(4, MI), NOTA(4, DO),
    NOTA(4, MI), NOTA(4, SOL), NOTA(4, FA), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, RIP_FINE,
    RITORNA,
};
/* 1336-133F comando 15: elenco dei record (blocco, flusso) */
static const uint8_t cmd_15[] = {
    REC(9, ADR_cmd_15_b9_s2), REC(12, ADR_cmd_15_b12_s2), REC(15, ADR_cmd_15_b15_s2),
    FINE_ELENCO,
};
/* 1340-1359 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_15_b9_s2[] = {
    STRUM(2), ATTEN(2), DUR(2), NOTA(2, DO), NOTA(2, MI), NOTA(2, SOL), NOTA(2, SI),
    NOTA(3, RE), NOTA(3, FA), NOTA(3, LA), NOTA(4, DO), ATTEN(3), NOTA(4, MI), ATTEN(4),
    NOTA(4, SOL), ATTEN(5), NOTA(4, SI), ATTEN(6), NOTA(5, RE), FINE,
};
/* 135A-1373 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_15_b12_s2[] = {
    STRUM(2), ATTEN(2), DUR(2), NOTA(3, DO), NOTA(3, MI), NOTA(3, SOL), NOTA(3, SI),
    NOTA(4, RE), NOTA(4, FA), NOTA(4, LA), NOTA(5, DO), ATTEN(3), NOTA(5, MI), ATTEN(4),
    NOTA(5, SOL), ATTEN(5), NOTA(5, SI), ATTEN(6), NOTA(6, RE), FINE,
};
/* 1374-138F flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_15_b15_s2[] = {
    STRUM(2), ATTEN(4), TRASP(0), DUR(2), NOTA(2, DO), NOTA(2, MI), NOTA(2, SOL), NOTA(2, SI),
    NOTA(3, RE), NOTA(3, FA), NOTA(3, LA), NOTA(4, DO), ATTEN(5), NOTA(4, MI), ATTEN(6),
    NOTA(4, SOL), ATTEN(7), NOTA(4, SI), ATTEN(8), NOTA(5, RE), FINE,
};
/* 1390-139C comando 16: elenco dei record (blocco, flusso) */
static const uint8_t cmd_16[] = {
    REC(2, ADR_cmd_16_b2_s1), REC(5, ADR_cmd_16_b5_s1), REC(7, ADR_cmd_16_b7_s1),
    REC(8, ADR_cmd_16_b8_s1), FINE_ELENCO,
};
/* 139D-13B8 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_16_b2_s1[] = {
    STRUM(2), DUR(0), ATTEN(4), RUMORE(7), NOTA(0, DO), ATTEN(8), RUMORE(6), NOTA(0, DO),
    ATTEN(12), RUMORE(5), NOTA(0, DO), DUR(12), ATTEN(15), RUMORE(0), PAUSA,
    SALTA(ADR_cmd_16_b2_s1),
    B(0xFF),
};
/* 13B9-13C3 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_16_b5_s1[] = {
    STRUM(2), TRASP(0), DUR(31), ATTEN(6), NOTA(3, FA), ATTEN(5), NOTA(3, FA),
};
/* 13C4-14A1 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_16_b5_s1_p2[] = {
    ATTEN(4), NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), NOTA(4, DOd),
    NOTA(4, DOd), NOTA(4, FA), NOTA(4, FA), NOTA(4, FA), NOTA(4, FA), NOTA(4, LAd),
    NOTA(4, LAd), NOTA(4, SOLd), NOTA(4, SOLd), NOTA(4, SOLd), NOTA(4, FAd), NOTA(4, SOLd),
    NOTA(4, FAd), ATTEN(5), NOTA(4, FA), ATTEN(6), NOTA(4, FA), ATTEN(7), NOTA(4, FA), ATTEN(8),
    NOTA(4, FA), ATTEN(5), NOTA(4, FA), ATTEN(4), NOTA(4, FA), NOTA(4, MI), NOTA(4, MI),
    NOTA(4, MI), NOTA(4, DOd), NOTA(3, LA), NOTA(3, LA), NOTA(4, REd), NOTA(4, REd),
    NOTA(4, REd), NOTA(4, DO), NOTA(3, SOLd), NOTA(3, SOLd), NOTA(4, DOd), NOTA(4, DOd),
    NOTA(4, DOd), NOTA(4, DOd), NOTA(4, DOd), NOTA(4, DOd), ATTEN(5), NOTA(4, DOd), ATTEN(6),
    NOTA(4, DOd), ATTEN(7), NOTA(4, DOd), ATTEN(8), NOTA(4, DOd), ATTEN(5), NOTA(3, FA),
    ATTEN(4), NOTA(3, FA), NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), NOTA(4, DOd),
    NOTA(4, DOd), NOTA(4, FA), NOTA(4, FA), NOTA(4, FA), NOTA(4, FA), NOTA(4, LAd),
    NOTA(4, LAd), NOTA(5, DO), NOTA(5, DO), NOTA(5, DO), NOTA(4, LAd), NOTA(5, DO),
    NOTA(4, LAd), ATTEN(5), NOTA(4, SOLd), ATTEN(6), NOTA(4, SOLd), ATTEN(7), NOTA(4, SOLd),
    ATTEN(8), NOTA(4, SOLd), ATTEN(5), NOTA(4, SOLd), ATTEN(4), NOTA(4, SOLd), NOTA(4, FAd),
    NOTA(4, FAd), NOTA(4, FAd), NOTA(4, REd), NOTA(3, SI), NOTA(3, SI), NOTA(4, FA),
    NOTA(4, FA), NOTA(4, FA), NOTA(4, RE), NOTA(3, LAd), NOTA(3, LAd), NOTA(4, REd),
    NOTA(4, REd), NOTA(4, REd), NOTA(4, REd), NOTA(4, REd), NOTA(4, REd), ATTEN(5),
    NOTA(4, REd), ATTEN(6), NOTA(4, REd), ATTEN(7), NOTA(4, REd), ATTEN(8), NOTA(4, REd),
    ATTEN(5), NOTA(3, SOL), ATTEN(4), NOTA(3, SOL), NOTA(4, DO), NOTA(4, DO), NOTA(4, DO),
    NOTA(4, DO), NOTA(4, REd), NOTA(4, REd), NOTA(4, FA), NOTA(4, FA), NOTA(4, FA),
    NOTA(4, REd), NOTA(4, RE), NOTA(4, RE), NOTA(4, REd), NOTA(4, REd), NOTA(4, REd),
    NOTA(4, RE), NOTA(4, DO), NOTA(4, DO), ATTEN(5), NOTA(3, SI), ATTEN(6), NOTA(3, SI),
    ATTEN(7), NOTA(3, SI), ATTEN(8), NOTA(3, SI), ATTEN(5), NOTA(3, SOL), ATTEN(4),
    NOTA(3, SOL), NOTA(4, DO), NOTA(4, DO), NOTA(4, DO), NOTA(4, DO), NOTA(4, REd),
    NOTA(4, REd), NOTA(4, FA), NOTA(4, FA), NOTA(4, FA), NOTA(4, REd), NOTA(4, RE), NOTA(4, RE),
    NOTA(4, REd), NOTA(4, REd), NOTA(4, REd), NOTA(4, RE), NOTA(4, DO), NOTA(4, DO), ATTEN(5),
    NOTA(3, SI), ATTEN(6), NOTA(3, SI), ATTEN(7), NOTA(4, FAd), ATTEN(8), NOTA(4, FAd),
    ATTEN(9), NOTA(4, FA), ATTEN(10), NOTA(4, FA), SALTA(ADR_cmd_16_b5_s1_p2),
    B(0xFF),
};
/* 14A2-14A9 flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_16_b7_s1[] = {
    STRUM(2), TRASP(0), DUR(15), PAUSA, PAUSA, PAUSA,
};
/* 14AA-1568 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_16_b7_s1_p2[] = {
    ATTEN(5), DUR(15), RIP_INIZIO(6), PERIODO(0x0152), PERIODO(0x0157), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x010C), PERIODO(0x0111), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x00E2), PERIODO(0x00E7), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x010C), PERIODO(0x0111), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x0152), PERIODO(0x0157), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x0166), PERIODO(0x016B), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x0152), PERIODO(0x0157), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x01C3), PERIODO(0x01C9), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x0152), PERIODO(0x0157), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x010C), PERIODO(0x0111), RIP_FINE,
    RIP_INIZIO(12), PERIODO(0x00E2), PERIODO(0x00E7), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x012D), PERIODO(0x0132), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x013F), PERIODO(0x0144), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x012D), PERIODO(0x0132), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x0192), PERIODO(0x0198), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x012D), PERIODO(0x0132), RIP_FINE,
    RIP_INIZIO(12), PERIODO(0x0166), PERIODO(0x016B), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x017C), PERIODO(0x0182), RIP_FINE,
    RIP_INIZIO(6), PERIODO(0x012D), PERIODO(0x0132), RIP_FINE,
    RIP_INIZIO(12), PERIODO(0x0166), PERIODO(0x016B), RIP_FINE,
    DUR(63), NOTA(2, SI), NOTA(2, LAd), NOTA(2, LA), SALTA(ADR_cmd_16_b7_s1_p2),
    B(0xFF),
};
/* 1569-1571 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_16_b8_s1[] = {
    STRUM(2), ATTEN(5), TRASP(0), DUR(31), PAUSA, PAUSA,
};
/* 1572-160D ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_16_b8_s1_p2[] = {
    ATTEN(5), NOTA(2, LAd), NOTA(3, LAd), NOTA(2, LAd), NOTA(4, LAd), NOTA(2, LAd),
    NOTA(5, LAd), NOTA(3, DOd), NOTA(4, DOd), NOTA(3, DOd), NOTA(5, DOd), NOTA(3, DOd),
    NOTA(6, DOd), NOTA(3, MI), NOTA(4, MI), NOTA(3, MI), NOTA(5, MI), NOTA(3, MI), NOTA(6, MI),
    NOTA(3, DOd), NOTA(4, DOd), NOTA(3, DOd), NOTA(5, DOd), NOTA(3, DOd), NOTA(6, DOd),
    NOTA(2, LA), NOTA(3, LA), NOTA(2, LA), NOTA(4, LA), NOTA(2, LA), NOTA(5, LA), NOTA(2, SOLd),
    NOTA(3, SOLd), NOTA(2, SOLd), NOTA(4, SOLd), NOTA(2, SOLd), NOTA(5, SOLd), NOTA(2, LA),
    NOTA(3, LA), NOTA(2, LA), NOTA(4, LA), NOTA(2, LA), NOTA(5, LA), NOTA(2, FA), NOTA(3, FA),
    NOTA(2, FA), NOTA(4, FA), NOTA(2, FA), NOTA(5, FA), NOTA(2, LAd), NOTA(3, LAd),
    NOTA(2, LAd), NOTA(4, LAd), NOTA(2, LAd), NOTA(5, LAd), NOTA(3, DOd), NOTA(4, DOd),
    NOTA(3, DOd), NOTA(5, DOd), NOTA(3, DOd), NOTA(6, DOd), NOTA(3, FA), NOTA(4, FA),
    NOTA(3, FA), NOTA(5, FA), NOTA(3, FA), NOTA(6, FA), NOTA(3, MI), NOTA(4, MI), NOTA(3, MI),
    NOTA(5, MI), NOTA(3, MI), NOTA(6, MI), NOTA(2, SI), NOTA(3, SI), NOTA(2, SI), NOTA(4, SI),
    NOTA(2, SI), NOTA(5, SI), NOTA(2, LAd), NOTA(3, LAd), NOTA(2, LAd), NOTA(4, LAd),
    NOTA(2, LAd), NOTA(5, LAd), NOTA(2, SI), NOTA(3, SI), NOTA(2, SI), NOTA(4, SI), NOTA(2, SI),
    NOTA(5, SI), NOTA(2, SOL), NOTA(3, SOL), NOTA(2, SOL), NOTA(4, SOL), NOTA(2, SOL),
    NOTA(5, SOL), NOTA(3, DO), NOTA(4, DO), NOTA(3, DO), NOTA(5, DO), NOTA(3, DO), NOTA(6, DO),
    NOTA(2, LA), NOTA(3, LA), NOTA(2, LA), NOTA(4, LA), NOTA(2, LA), NOTA(5, LA), NOTA(2, SOLd),
    NOTA(3, SOLd), NOTA(2, SOLd), NOTA(4, SOLd), NOTA(2, SOLd), NOTA(5, SOLd), NOTA(2, SOL),
    NOTA(3, SOL), NOTA(2, SOL), NOTA(4, SOL), NOTA(2, SOL), NOTA(5, SOL), NOTA(3, DO),
    NOTA(4, DO), NOTA(3, DO), NOTA(5, DO), NOTA(3, DO), NOTA(6, DO), NOTA(2, LA), NOTA(3, LA),
    NOTA(2, LA), NOTA(4, LA), NOTA(2, LA), NOTA(5, LA), NOTA(2, SOLd), NOTA(3, SOLd),
    NOTA(2, SOLd), NOTA(4, SOLd), NOTA(2, SOLd), NOTA(5, SOLd), ATTEN(6), NOTA(2, SOL),
    NOTA(3, SOL), ATTEN(7), NOTA(2, FAd), NOTA(3, FAd), ATTEN(8), NOTA(2, FA), NOTA(3, FA),
    SALTA(ADR_cmd_16_b8_s1_p2),
    B(0xFF),
};
/* 160E-1611 comando 17: elenco dei record (blocco, flusso) */
static const uint8_t cmd_17[] = {
    REC(0, ADR_cmd_17_b0_s1), FINE_ELENCO,
};
/* 1612-1616 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_17_b0_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 1617-1620 comando 18: elenco dei record (blocco, flusso) */
static const uint8_t cmd_18[] = {
    REC(9, ADR_cmd_18_b9_s2), REC(10, ADR_cmd_18_b10_s2), REC(12, ADR_cmd_18_b12_s2),
    FINE_ELENCO,
};
/* 1621-1638 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_18_b9_s2[] = {
    STRUM(2), ATTEN(1), DUR(1), NOTA(4, DO), NOTA(4, DOd), NOTA(4, RE), NOTA(4, REd),
    NOTA(4, MI), NOTA(4, FA), NOTA(4, FAd), NOTA(4, SOL), NOTA(4, SOLd), NOTA(4, LA),
    NOTA(4, LAd), NOTA(4, SI), STRUM(0), ATTEN(0), DUR(87), NOTA(5, DO), FINE,
};
/* 1639-1650 flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_18_b10_s2[] = {
    STRUM(2), ATTEN(1), DUR(1), NOTA(5, DO), NOTA(5, DOd), NOTA(5, RE), NOTA(5, REd),
    NOTA(5, MI), NOTA(5, FA), NOTA(5, FAd), NOTA(5, SOL), NOTA(5, SOLd), NOTA(5, LA),
    NOTA(5, LAd), NOTA(5, SI), STRUM(0), ATTEN(0), DUR(87), NOTA(6, DO), FINE,
};
/* 1651-1668 flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_18_b12_s2[] = {
    STRUM(2), ATTEN(1), DUR(1), NOTA(6, DO), NOTA(6, DOd), NOTA(6, RE), NOTA(6, REd),
    NOTA(6, MI), NOTA(6, FA), NOTA(6, FAd), NOTA(6, SOL), NOTA(6, SOLd), NOTA(6, LA),
    NOTA(6, LAd), NOTA(6, SI), STRUM(0), ATTEN(0), DUR(87), NOTA(7, DO), FINE,
};
/* 1669-1675 comando 19: elenco dei record (blocco, flusso) */
static const uint8_t cmd_19[] = {
    REC(2, ADR_cmd_19_b2_s1), REC(5, ADR_cmd_19_b5_s1), REC(7, ADR_cmd_19_b7_s1),
    REC(8, ADR_cmd_19_b8_s1), FINE_ELENCO,
};
/* 1676-16B3 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_19_b2_s1[] = {
    STRUM(4), DUR(11), RIP_INIZIO(4), CHIAMA(ADR_cmd_19_b2_s1_p2), RIP_FINE,
    DUR(15), RIP_INIZIO(4), CHIAMA(ADR_cmd_19_b2_s1_p2), RIP_FINE,
    DUR(11), RIP_INIZIO(4), CHIAMA(ADR_cmd_19_b2_s1_p2), RIP_FINE,
    DUR(15), RIP_INIZIO(4), CHIAMA(ADR_cmd_19_b2_s1_p2), RIP_FINE,
    DUR(11), RIP_INIZIO(5), CHIAMA(ADR_cmd_19_b2_s1_p2), RIP_FINE,
    DUR(15), RIP_INIZIO(4), CHIAMA(ADR_cmd_19_b2_s1_p2), RIP_FINE,
    DUR(11), RIP_INIZIO(5), CHIAMA(ADR_cmd_19_b2_s1_p2), RIP_FINE,
    DUR(15), RIP_INIZIO(4), CHIAMA(ADR_cmd_19_b2_s1_p2), RIP_FINE,
    SALTA(ADR_cmd_19_b2_s1),
    B(0xFF),
};
/* 16B4-16C8 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_19_b2_s1_p2[] = {
    ATTEN(4), RUMORE(7), NOTA(0, DO), ATTEN(15), RUMORE(0), PAUSA, ATTEN(15), RUMORE(0), PAUSA,
    ATTEN(8), RUMORE(7), NOTA(0, DO), RITORNA,
};
/* 16C9-16D5 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_19_b5_s1[] = {
    STRUM(7), ATTEN(3), TRASP(2), CHIAMA(ADR_cmd_19_b7_s1_p2), SALTA(ADR_cmd_19_b5_s1),
    B(0xFF),
};
/* 16D6-16E2 flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_19_b7_s1[] = {
    STRUM(7), ATTEN(4), TRASP(14), CHIAMA(ADR_cmd_19_b7_s1_p2), SALTA(ADR_cmd_19_b7_s1),
    B(0xFF),
};
/* 16E3-1773 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_19_b7_s1_p2[] = {
    DUR(11), NOTA(3, FA), PAUSA, NOTA(3, LAd), NOTA(3, LA), NOTA(3, FA), PAUSA, PAUSA,
    NOTA(3, REd), NOTA(3, FA), PAUSA, PAUSA, PAUSA, NOTA(3, LA), PAUSA, PAUSA, NOTA(3, LAd),
    DUR(15), NOTA(3, SOL), PAUSA, NOTA(4, REd), NOTA(4, REd), NOTA(4, REd), PAUSA, NOTA(4, REd),
    PAUSA, NOTA(4, FA), PAUSA, NOTA(4, REd), PAUSA, NOTA(4, FA), PAUSA, NOTA(4, REd), PAUSA,
    DUR(11), NOTA(3, FA), PAUSA, NOTA(3, LAd), NOTA(3, LA), NOTA(3, FA), PAUSA, PAUSA,
    NOTA(3, REd), NOTA(3, FA), PAUSA, PAUSA, PAUSA, NOTA(3, LA), PAUSA, PAUSA, NOTA(3, LAd),
    DUR(15), NOTA(3, SOL), PAUSA, NOTA(4, MI), NOTA(4, MI), NOTA(4, MI), PAUSA, NOTA(4, MI),
    PAUSA, NOTA(4, FA), PAUSA, NOTA(4, MI), PAUSA, NOTA(4, FA), PAUSA, NOTA(4, MI), PAUSA,
    DUR(11), NOTA(4, RE), PAUSA, PAUSA, NOTA(4, FAd), NOTA(4, RE), PAUSA, PAUSA, NOTA(4, REd),
    NOTA(3, SI), PAUSA, PAUSA, PAUSA, NOTA(4, RE), PAUSA, NOTA(4, FAd), PAUSA, NOTA(4, RE),
    PAUSA, PAUSA, NOTA(4, REd), DUR(15), NOTA(3, SI), PAUSA, NOTA(4, SOL), NOTA(4, SOL),
    NOTA(4, SOL), PAUSA, NOTA(4, SOL), PAUSA, NOTA(4, LA), PAUSA, NOTA(4, SOL), PAUSA,
    NOTA(4, LA), PAUSA, NOTA(4, SOL), PAUSA, DUR(11), NOTA(4, RE), PAUSA, PAUSA, NOTA(4, FAd),
    NOTA(4, RE), PAUSA, PAUSA, NOTA(4, REd), NOTA(3, SI), PAUSA, PAUSA, PAUSA, NOTA(4, RE),
    PAUSA, PAUSA, NOTA(4, FAd), NOTA(4, RE), PAUSA, PAUSA, NOTA(4, REd), DUR(15), NOTA(3, SI),
    PAUSA, NOTA(4, SOLd), NOTA(4, SOLd), NOTA(4, SOLd), PAUSA, NOTA(4, SOLd), PAUSA,
    NOTA(4, LA), PAUSA, NOTA(4, SOLd), PAUSA, NOTA(4, LA), PAUSA, NOTA(4, SOLd), PAUSA, RITORNA,
};
/* 1774-1799 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_19_b8_s1[] = {
    STRUM(0), ATTEN(3), TRASP(2), DUR(47), RIP_INIZIO(2), DUR(47), NOTA(1, LAd), NOTA(1, SOLd),
    NOTA(1, LAd), NOTA(1, SOLd), DUR(63), NOTA(1, SOL), NOTA(1, LAd), NOTA(1, SOL),
    NOTA(1, LAd), RIP_FINE,
    RIP_INIZIO(2), DUR(47), NOTA(1, SI), NOTA(1, SOLd), NOTA(1, FA), NOTA(1, SI), NOTA(1, SOLd),
    DUR(63), NOTA(1, FA), NOTA(2, RE), NOTA(1, FA), NOTA(2, RE), RIP_FINE,
    SALTA(ADR_cmd_19_b8_s1),
    B(0xFF),
};
/* 179A-17A6 comando 1A: elenco dei record (blocco, flusso) */
static const uint8_t cmd_1A[] = {
    REC(2, ADR_cmd_1A_b2_s1), REC(5, ADR_cmd_1A_b5_s1), REC(7, ADR_cmd_1A_b7_s1),
    REC(8, ADR_cmd_1A_b8_s1), FINE_ELENCO,
};
/* 17A7-17A9 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_1A_b2_s1[] = {
    CHIAMA(ADR_cmd_1A_b2_s1_p5),
};
/* 17AA-17BC ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1A_b2_s1_p2[] = {
    CHIAMA(ADR_cmd_1A_b2_s1_p3), CHIAMA(ADR_cmd_1A_b2_s1_p3), CHIAMA(ADR_cmd_1A_b2_s1_p3),
    CHIAMA(ADR_cmd_1A_b2_s1_p4), CHIAMA(ADR_cmd_1A_b2_s1_p4), SALTA(ADR_cmd_1A_b2_s1_p2),
    B(0xFF),
};
/* 17BD-1804 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1A_b2_s1_p3[] = {
    STRUM(2), DUR(3), ATTEN(11), RUMORE(10), NOTA(0, DO), ATTEN(10), RUMORE(10), NOTA(0, DO),
    ATTEN(9), RUMORE(10), NOTA(0, DO), ATTEN(8), RUMORE(10), NOTA(0, DO), ATTEN(7), RUMORE(10),
    NOTA(0, DO), ATTEN(6), RUMORE(10), NOTA(0, DO), ATTEN(7), RUMORE(10), NOTA(0, DO), ATTEN(8),
    RUMORE(10), NOTA(0, DO), ATTEN(9), RUMORE(10), NOTA(0, DO), ATTEN(10), RUMORE(10),
    NOTA(0, DO), ATTEN(11), RUMORE(10), NOTA(0, DO), STRUM(1), DUR(21), ATTEN(2), RUMORE(12),
    NOTA(0, DO), ATTEN(3), RUMORE(12), NOTA(0, DO), RITORNA,
};
/* 1805-1847 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1A_b2_s1_p4[] = {
    STRUM(2), DUR(1), ATTEN(11), RUMORE(12), NOTA(0, DO), ATTEN(10), RUMORE(12), NOTA(0, DO),
    ATTEN(9), RUMORE(12), NOTA(0, DO), ATTEN(8), RUMORE(12), NOTA(0, DO), ATTEN(7), RUMORE(12),
    NOTA(0, DO), ATTEN(6), RUMORE(12), NOTA(0, DO), ATTEN(7), RUMORE(12), NOTA(0, DO), ATTEN(8),
    RUMORE(12), NOTA(0, DO), ATTEN(9), RUMORE(12), NOTA(0, DO), ATTEN(10), RUMORE(12),
    NOTA(0, DO), ATTEN(11), RUMORE(12), NOTA(0, DO), STRUM(1), DUR(21), ATTEN(2), RUMORE(14),
    NOTA(0, DO), RITORNA,
};
/* 1848-1873 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1A_b2_s1_p5[] = {
    STRUM(1), DUR(21), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA,
    ATTEN(2), RUMORE(10), NOTA(0, DO), ATTEN(2), RUMORE(10), NOTA(0, DO), ATTEN(15), RUMORE(0),
    PAUSA, ATTEN(15), RUMORE(0), PAUSA, ATTEN(1), RUMORE(10), NOTA(0, DO), ATTEN(15), RUMORE(0),
    PAUSA, RITORNA,
};
/* 1874-188C flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_1A_b5_s1[] = {
    STRUM(7), ATTEN(4), TRASP(0), DUR(21), NOTA(1, SI), NOTA(2, DO), NOTA(2, RE), NOTA(2, REd),
    NOTA(2, FA), NOTA(2, REd), NOTA(2, RE), NOTA(2, DO), NOTA(1, SI), NOTA(1, SOL),
    NOTA(1, SOL), NOTA(1, SI), NOTA(1, SOL), NOTA(1, SOL), NOTA(1, SI), NOTA(1, SOL), ATTEN(3),
};
/* 188D-189C ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1A_b5_s1_p2[] = {
    CHIAMA(ADR_cmd_1A_b5_s1_p3), CHIAMA(ADR_cmd_1A_b5_s1_p3), CHIAMA(ADR_cmd_1A_b5_s1_p4),
    CHIAMA(ADR_cmd_1A_b5_s1_p4), SALTA(ADR_cmd_1A_b5_s1_p2),
    B(0xFF),
};
/* 189D-18BD ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1A_b5_s1_p3[] = {
    NOTA(2, DO), NOTA(2, REd), NOTA(2, SOL), NOTA(2, REd), NOTA(2, RE), NOTA(2, FA),
    NOTA(2, LAd), NOTA(2, RE), NOTA(2, REd), NOTA(2, SOL), NOTA(3, DO), NOTA(2, REd),
    NOTA(2, FA), NOTA(2, SOLd), NOTA(3, RE), NOTA(2, FA), NOTA(2, REd), NOTA(2, SOL),
    NOTA(3, DO), NOTA(2, REd), NOTA(2, RE), NOTA(2, FA), NOTA(2, LAd), NOTA(2, RE), NOTA(2, DO),
    NOTA(2, REd), NOTA(2, SOL), NOTA(2, DO), NOTA(1, SI), NOTA(2, RE), NOTA(2, SOL),
    NOTA(1, SI), RITORNA,
};
/* 18BE-18DE ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1A_b5_s1_p4[] = {
    NOTA(1, LA), NOTA(2, DO), NOTA(2, MI), NOTA(1, LA), NOTA(1, SI), NOTA(2, RE), NOTA(2, FA),
    NOTA(1, SI), NOTA(2, DO), NOTA(2, MI), NOTA(2, SOL), NOTA(2, DO), NOTA(2, RE), NOTA(2, FA),
    NOTA(2, LA), NOTA(2, RE), NOTA(2, MI), NOTA(2, SOL), NOTA(3, DO), NOTA(2, MI), NOTA(2, RE),
    NOTA(2, FA), NOTA(2, SI), NOTA(2, RE), NOTA(2, DO), NOTA(2, MI), NOTA(2, LA), NOTA(2, DO),
    NOTA(1, SI), NOTA(2, RE), NOTA(2, SOL), NOTA(1, SI), RITORNA,
};
/* 18DF-18FB flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_1A_b7_s1[] = {
    STRUM(8), ATTEN(3), TRASP(0), DUR(21), NOTA(3, SI), NOTA(4, DO), NOTA(4, RE), NOTA(4, REd),
    NOTA(4, FA), NOTA(4, REd), NOTA(4, RE), NOTA(2, DO), NOTA(3, SI), NOTA(3, SOL),
    NOTA(3, SOL), NOTA(3, SI), NOTA(3, SOL), NOTA(3, SOL), NOTA(3, SI), NOTA(3, SOL), STRUM(9),
    ATTEN(3), TRASP(0),
};
/* 18FC-1903 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1A_b7_s1_p2[] = {
    DUR(21), CHIAMA(ADR_cmd_1A_b7_s1_p3), SALTA(ADR_cmd_1A_b7_s1_p2),
    B(0xFF),
};
/* 1904-1984 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1A_b7_s1_p3[] = {
    PAUSA, PAUSA, NOTA(4, DO), NOTA(4, DO), NOTA(3, SI), PAUSA, NOTA(4, DO), NOTA(4, DO),
    NOTA(4, RE), PAUSA, NOTA(4, DO), NOTA(3, SI), NOTA(4, DO), PAUSA, PAUSA, PAUSA, PAUSA,
    PAUSA, NOTA(4, DO), NOTA(4, DO), NOTA(3, SI), PAUSA, NOTA(4, DO), NOTA(4, DO), NOTA(3, SOL),
    PAUSA, NOTA(3, REd), PAUSA, NOTA(3, RE), PAUSA, NOTA(3, SOL), PAUSA, PAUSA, PAUSA,
    NOTA(4, DO), NOTA(4, DO), NOTA(3, SI), PAUSA, NOTA(4, DO), NOTA(4, DO), NOTA(4, RE), PAUSA,
    NOTA(4, DO), NOTA(3, SI), NOTA(4, DO), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, NOTA(4, DO),
    NOTA(4, DO), NOTA(3, SI), PAUSA, NOTA(4, DO), NOTA(4, DO), NOTA(3, SOL), PAUSA,
    NOTA(3, REd), PAUSA, NOTA(3, RE), PAUSA, NOTA(3, DO), PAUSA, PAUSA, PAUSA, NOTA(3, LA),
    NOTA(3, LA), NOTA(3, SOLd), PAUSA, NOTA(3, LA), NOTA(3, LA), NOTA(3, SI), PAUSA,
    NOTA(3, LA), NOTA(3, SOLd), NOTA(3, LA), PAUSA, NOTA(3, SI), NOTA(4, DO), NOTA(4, RE),
    PAUSA, NOTA(4, DO), NOTA(3, SI), NOTA(4, DO), PAUSA, NOTA(4, RE), NOTA(4, MI), NOTA(4, FA),
    PAUSA, NOTA(4, MI), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, NOTA(3, LA),
    NOTA(3, LA), NOTA(3, SOLd), PAUSA, NOTA(3, LA), NOTA(3, LA), NOTA(3, SI), PAUSA,
    NOTA(3, LA), NOTA(3, SOLd), NOTA(3, LA), PAUSA, NOTA(3, SI), NOTA(4, DO), NOTA(4, RE),
    NOTA(4, DO), NOTA(3, SI), NOTA(3, LA), NOTA(4, DO), NOTA(3, SI), NOTA(3, LA), NOTA(3, SOLd),
    NOTA(3, LA), PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, PAUSA, RITORNA,
};
/* 1985-19A1 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_1A_b8_s1[] = {
    STRUM(8), ATTEN(3), TRASP(0), DUR(21), NOTA(4, SI), NOTA(5, DO), NOTA(5, RE), NOTA(5, REd),
    NOTA(5, FA), NOTA(5, REd), NOTA(5, RE), NOTA(5, DO), NOTA(4, SI), NOTA(4, SOL),
    NOTA(4, SOL), NOTA(4, SI), NOTA(4, SOL), NOTA(4, SOL), NOTA(4, SI), NOTA(4, SOL), STRUM(9),
    ATTEN(4), TRASP(12),
};
/* 19A2-19A9 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1A_b8_s1_p2[] = {
    DUR(21), CHIAMA(ADR_cmd_1A_b7_s1_p3), SALTA(ADR_cmd_1A_b8_s1_p2),
    B(0xFF),
};

const struct aud_pezzo aud_pezzi_spartiti_b[] = {
    PEZZO(cmd_13),
    PEZZO(cmd_13_b5_s1),
    PEZZO(cmd_13_b5_s1_p2),
    PEZZO(cmd_13_b7_s1),
    PEZZO(cmd_13_b7_s1_p2),
    PEZZO(cmd_13_b8_s1),
    PEZZO(cmd_13_b8_s1_p2),
    PEZZO(cmd_13_b8_s1_p3),
    PEZZO(cmd_14),
    PEZZO(cmd_14_b2_s1),
    PEZZO(cmd_14_b5_s1),
    PEZZO(cmd_14_b5_s1_p2),
    PEZZO(cmd_14_b5_s1_p3),
    PEZZO(cmd_14_b7_s1),
    PEZZO(cmd_14_b7_s1_p2),
    PEZZO(cmd_14_b8_s1),
    PEZZO(cmd_14_b8_s1_p2),
    PEZZO(cmd_14_b8_s1_p3),
    PEZZO(cmd_15),
    PEZZO(cmd_15_b9_s2),
    PEZZO(cmd_15_b12_s2),
    PEZZO(cmd_15_b15_s2),
    PEZZO(cmd_16),
    PEZZO(cmd_16_b2_s1),
    PEZZO(cmd_16_b5_s1),
    PEZZO(cmd_16_b5_s1_p2),
    PEZZO(cmd_16_b7_s1),
    PEZZO(cmd_16_b7_s1_p2),
    PEZZO(cmd_16_b8_s1),
    PEZZO(cmd_16_b8_s1_p2),
    PEZZO(cmd_17),
    PEZZO(cmd_17_b0_s1),
    PEZZO(cmd_18),
    PEZZO(cmd_18_b9_s2),
    PEZZO(cmd_18_b10_s2),
    PEZZO(cmd_18_b12_s2),
    PEZZO(cmd_19),
    PEZZO(cmd_19_b2_s1),
    PEZZO(cmd_19_b2_s1_p2),
    PEZZO(cmd_19_b5_s1),
    PEZZO(cmd_19_b7_s1),
    PEZZO(cmd_19_b7_s1_p2),
    PEZZO(cmd_19_b8_s1),
    PEZZO(cmd_1A),
    PEZZO(cmd_1A_b2_s1),
    PEZZO(cmd_1A_b2_s1_p2),
    PEZZO(cmd_1A_b2_s1_p3),
    PEZZO(cmd_1A_b2_s1_p4),
    PEZZO(cmd_1A_b2_s1_p5),
    PEZZO(cmd_1A_b5_s1),
    PEZZO(cmd_1A_b5_s1_p2),
    PEZZO(cmd_1A_b5_s1_p3),
    PEZZO(cmd_1A_b5_s1_p4),
    PEZZO(cmd_1A_b7_s1),
    PEZZO(cmd_1A_b7_s1_p2),
    PEZZO(cmd_1A_b7_s1_p3),
    PEZZO(cmd_1A_b8_s1),
    PEZZO(cmd_1A_b8_s1_p2),
};
const unsigned aud_n_pezzi_spartiti_b =
    sizeof aud_pezzi_spartiti_b / sizeof aud_pezzi_spartiti_b[0];
