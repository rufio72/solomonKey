/* Generato da decomp/estrai_dati.py dalla ROM audio 1.3jk: non modificare
 * a mano. Dati musicali derivati dal programma Tecmo (NOTICE.md). */
/* Elenchi e flussi dei comandi 1B-1F. */
#include <stddef.h>
#include "audio_dati.h"

/* 19AA-19B6 comando 1B: elenco dei record (blocco, flusso) */
static const uint8_t cmd_1B[] = {
    REC(2, ADR_cmd_1B_b2_s1), REC(5, ADR_cmd_1B_b5_s1), REC(7, ADR_cmd_1B_b7_s1),
    REC(8, ADR_cmd_1B_b8_s1), FINE_ELENCO,
};
/* 19B7-19BA flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_1B_b2_s1[] = {
    STRUM(0), ATTEN(3),
};
/* 19BB-19E7 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1B_b2_s1_p2[] = {
    RIP_INIZIO(3), GLISSA(0x0432, 0x03BC, 31), GLISSA(0x0218, 0x025A, 15),
    GLISSA(0x02CC, 0x0224, 15), RIP_FINE,
    RIP_INIZIO(2), GLISSA(0x0432, 0x03BC, 15), GLISSA(0x0218, 0x025A, 15), RIP_FINE,
    SALTA(ADR_cmd_1B_b2_s1_p2),
    B(0xFF),
};
/* 19E8-19EC flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_1B_b5_s1[] = {
    STRUM(6), ATTEN(3), DUR(15),
};
/* 19ED-19FC ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1B_b5_s1_p2[] = {
    RIP_INIZIO(3), NOTA(3, FA), PAUSA, NOTA(4, FA), NOTA(4, DO), RIP_FINE,
    RIP_INIZIO(2), NOTA(3, FA), NOTA(4, FA), RIP_FINE,
    SALTA(ADR_cmd_1B_b5_s1_p2),
    B(0xFF),
};
/* 19FD-19FF flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_1B_b7_s1[] = {
    STRUM(9), DUR(15),
};
/* 1A00-1A13 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1B_b7_s1_p2[] = {
    ATTEN(4), RUMORE(12), NOTA(0, DO), RUMORE(12), NOTA(0, DO), ATTEN(3), RUMORE(12),
    NOTA(0, DO), RUMORE(12), NOTA(0, DO), SALTA(ADR_cmd_1B_b7_s1_p2),
    B(0xFF),
};
/* 1A14-1A55 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_1B_b8_s1[] = {
    STRUM(2), DUR(3), RIP_INIZIO(16), ATTEN(8), NOTA(3, REd), ATTEN(10), NOTA(4, REd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(7), NOTA(3, REd), ATTEN(9), NOTA(4, REd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(6), NOTA(3, REd), ATTEN(8), NOTA(4, REd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(5), NOTA(3, REd), ATTEN(7), NOTA(4, REd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(4), NOTA(3, REd), ATTEN(6), NOTA(4, REd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(5), NOTA(3, REd), ATTEN(7), NOTA(4, REd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(6), NOTA(3, REd), ATTEN(8), NOTA(4, REd), RIP_FINE,
};
/* 1A56-1BCD ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1B_b8_s1_p2[] = {
    ATTEN(8), DUR(5), NOTA(3, RE), DUR(4), NOTA(4, RE), ATTEN(7), DUR(5), NOTA(3, REd), DUR(4),
    NOTA(4, REd), ATTEN(6), DUR(4), NOTA(3, FA), DUR(4), NOTA(4, FA), DUR(5), NOTA(3, REd),
    DUR(4), NOTA(4, REd), DUR(5), NOTA(3, FA), DUR(4), NOTA(4, FA), DUR(4), NOTA(3, FAd),
    DUR(4), NOTA(4, FAd), DUR(5), NOTA(3, FA), DUR(4), NOTA(4, FA), DUR(5), NOTA(3, FAd),
    DUR(4), NOTA(4, FAd), DUR(4), NOTA(3, SOLd), DUR(4), NOTA(4, SOLd), DUR(5), NOTA(3, FAd),
    DUR(4), NOTA(4, FAd), DUR(5), NOTA(3, SOLd), DUR(4), NOTA(4, SOLd), DUR(4), NOTA(3, LAd),
    DUR(4), NOTA(4, LAd), DUR(3), ATTEN(8), NOTA(3, MI), NOTA(3, FA), NOTA(3, FAd),
    NOTA(3, SOL), RIP_INIZIO(14), ATTEN(6), NOTA(3, SOLd), ATTEN(8), NOTA(4, SOLd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(5), NOTA(3, SOLd), ATTEN(7), NOTA(4, SOLd), RIP_FINE,
    NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA), NOTA(3, LAd), RIP_INIZIO(14), ATTEN(4),
    NOTA(3, SI), ATTEN(6), NOTA(4, SI), RIP_FINE,
    NOTA(3, LAd), NOTA(3, SI), NOTA(4, DO), NOTA(4, DOd), RIP_INIZIO(14), ATTEN(4), NOTA(4, RE),
    ATTEN(6), NOTA(5, RE), RIP_FINE,
    NOTA(3, SI), NOTA(4, DO), NOTA(4, DOd), NOTA(4, RE), RIP_INIZIO(30), ATTEN(4), NOTA(4, REd),
    ATTEN(6), NOTA(5, REd), RIP_FINE,
    NOTA(3, FAd), NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, SOLd), RIP_INIZIO(14), ATTEN(5),
    NOTA(3, LAd), ATTEN(7), NOTA(4, LAd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(6), NOTA(3, LAd), ATTEN(8), NOTA(4, LAd), RIP_FINE,
    NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA), NOTA(3, LAd), RIP_INIZIO(14), ATTEN(4),
    NOTA(3, SI), ATTEN(6), NOTA(4, SI), RIP_FINE,
    NOTA(3, FAd), NOTA(3, SOL), NOTA(3, SOLd), NOTA(3, LA), RIP_INIZIO(14), ATTEN(4),
    NOTA(3, LAd), ATTEN(6), NOTA(4, LAd), RIP_FINE,
    NOTA(3, RE), NOTA(3, REd), NOTA(3, MI), NOTA(3, FA), RIP_INIZIO(14), ATTEN(4), NOTA(3, FAd),
    ATTEN(6), NOTA(4, FAd), RIP_FINE,
    NOTA(2, LAd), NOTA(2, SI), NOTA(3, DO), NOTA(3, DOd), RIP_INIZIO(14), ATTEN(4), NOTA(3, RE),
    ATTEN(6), NOTA(4, RE), RIP_FINE,
    RIP_INIZIO(16), ATTEN(4), NOTA(3, REd), ATTEN(6), NOTA(4, REd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(5), NOTA(3, FA), ATTEN(7), NOTA(4, FA), RIP_FINE,
    RIP_INIZIO(16), ATTEN(6), NOTA(3, FAd), ATTEN(8), NOTA(4, FAd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(7), NOTA(3, LAd), ATTEN(9), NOTA(4, LAd), RIP_FINE,
    NOTA(4, MI), NOTA(4, FA), NOTA(4, FAd), NOTA(4, SOL), RIP_INIZIO(30), ATTEN(4),
    NOTA(3, SOLd), ATTEN(6), NOTA(4, SOLd), RIP_FINE,
    NOTA(4, SOL), NOTA(4, SOLd), NOTA(4, LA), NOTA(4, LAd), RIP_INIZIO(14), ATTEN(4),
    NOTA(3, SI), ATTEN(6), NOTA(4, SI), RIP_FINE,
    NOTA(4, LAd), NOTA(4, SI), NOTA(5, DO), NOTA(5, DOd), RIP_INIZIO(14), ATTEN(4), NOTA(4, RE),
    ATTEN(6), NOTA(5, RE), RIP_FINE,
    NOTA(4, SI), NOTA(5, DO), NOTA(5, DOd), NOTA(5, RE), RIP_INIZIO(30), ATTEN(4), NOTA(4, REd),
    ATTEN(6), NOTA(5, REd), RIP_FINE,
    NOTA(4, FAd), NOTA(4, SOL), NOTA(4, SOLd), NOTA(4, LA), RIP_INIZIO(30), ATTEN(5),
    NOTA(3, LAd), ATTEN(7), NOTA(4, LAd), RIP_FINE,
    NOTA(4, SOL), NOTA(4, SOLd), NOTA(4, LA), NOTA(4, LAd), RIP_INIZIO(14), ATTEN(4),
    NOTA(3, SI), ATTEN(6), NOTA(4, SI), RIP_FINE,
    NOTA(4, FAd), NOTA(4, SOL), NOTA(4, SOLd), NOTA(4, LA), RIP_INIZIO(14), ATTEN(4),
    NOTA(3, LAd), ATTEN(6), NOTA(4, LAd), RIP_FINE,
    NOTA(4, RE), NOTA(4, REd), NOTA(4, MI), NOTA(4, FA), RIP_INIZIO(14), ATTEN(4), NOTA(3, FAd),
    ATTEN(6), NOTA(4, FAd), RIP_FINE,
    NOTA(3, LAd), NOTA(3, SI), NOTA(4, DO), NOTA(4, DOd), RIP_INIZIO(14), ATTEN(4), NOTA(3, RE),
    ATTEN(6), NOTA(4, RE), RIP_FINE,
    RIP_INIZIO(16), ATTEN(5), NOTA(3, REd), ATTEN(7), NOTA(4, REd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(6), NOTA(3, REd), ATTEN(8), NOTA(4, REd), RIP_FINE,
    RIP_INIZIO(16), ATTEN(7), NOTA(3, REd), ATTEN(9), NOTA(4, REd), RIP_FINE,
    SALTA(ADR_cmd_1B_b8_s1_p2),
    B(0xFF),
};
/* 1BCE-1BDA comando 1C: elenco dei record (blocco, flusso) */
static const uint8_t cmd_1C[] = {
    REC(2, ADR_cmd_1C_b2_s1), REC(5, ADR_cmd_1C_b5_s1), REC(7, ADR_cmd_1C_b7_s1),
    REC(8, ADR_cmd_1C_b8_s1), FINE_ELENCO,
};
/* 1BDB-1BEF flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_1C_b2_s1[] = {
    STRUM(2), CHIAMA(ADR_cmd_1C_b2_s1_p2), DUR(11), ATTEN(15), RUMORE(0), PAUSA,
    CHIAMA(ADR_cmd_1C_b2_s1_p2), CHIAMA(ADR_cmd_1C_b2_s1_p2), SALTA(ADR_cmd_1C_b2_s1),
    B(0xFF),
};
/* 1BF0-1C0F ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1C_b2_s1_p2[] = {
    DUR(1), ATTEN(3), RUMORE(7), NOTA(0, DO), ATTEN(6), RUMORE(6), NOTA(0, DO), ATTEN(9),
    RUMORE(5), NOTA(0, DO), ATTEN(12), RUMORE(4), NOTA(0, DO), ATTEN(15), RUMORE(0), PAUSA,
    ATTEN(15), RUMORE(0), PAUSA, RITORNA,
};
/* 1C10-1C1D flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_1C_b5_s1[] = {
    STRUM(8), ATTEN(2), TRASP(0), DUR(11), CHIAMA(ADR_cmd_1C_b8_s1_p2), SALTA(ADR_cmd_1C_b5_s1),
    B(0xFF),
};
/* 1C1E-1C2B flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_1C_b7_s1[] = {
    STRUM(8), ATTEN(2), TRASP(8), DUR(11), CHIAMA(ADR_cmd_1C_b8_s1_p2), SALTA(ADR_cmd_1C_b7_s1),
    B(0xFF),
};
/* 1C2C-1C39 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_1C_b8_s1[] = {
    STRUM(8), ATTEN(2), TRASP(16), DUR(11), CHIAMA(ADR_cmd_1C_b8_s1_p2),
    SALTA(ADR_cmd_1C_b8_s1),
    B(0xFF),
};
/* 1C3A-1C8B ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1C_b8_s1_p2[] = {
    RIP_INIZIO(2), NOTA(1, FAd), NOTA(1, FA), NOTA(1, MI), NOTA(1, REd), NOTA(1, RE),
    NOTA(1, REd), NOTA(1, MI), NOTA(1, FA), NOTA(1, FAd), NOTA(1, FA), NOTA(1, MI),
    NOTA(1, REd), NOTA(1, RE), NOTA(1, REd), NOTA(1, MI), NOTA(1, FA), NOTA(1, FAd),
    NOTA(1, FA), NOTA(1, MI), NOTA(1, REd), NOTA(1, RE), NOTA(1, REd), NOTA(1, MI), NOTA(1, FA),
    NOTA(1, FAd), NOTA(2, FAd), NOTA(1, FAd), NOTA(2, REd), NOTA(1, FAd), NOTA(2, DOd),
    NOTA(1, FAd), NOTA(1, SOLd), RIP_FINE,
    RIP_INIZIO(4), NOTA(2, DOd), NOTA(2, DO), NOTA(1, SI), NOTA(2, DO), RIP_FINE,
    RIP_INIZIO(4), NOTA(1, SI), NOTA(1, LAd), NOTA(1, LA), NOTA(1, LAd), RIP_FINE,
    NOTA(1, LA), NOTA(1, SOLd), NOTA(1, SOL), NOTA(1, FAd), NOTA(1, SOLd), NOTA(1, SOL),
    NOTA(1, FAd), NOTA(1, FA), NOTA(1, SOL), NOTA(1, FAd), NOTA(1, FA), NOTA(1, MI),
    NOTA(1, FAd), NOTA(1, FA), NOTA(1, MI), NOTA(1, REd), NOTA(1, FA), NOTA(1, MI),
    NOTA(1, REd), NOTA(1, RE), NOTA(1, MI), NOTA(1, REd), NOTA(1, RE), NOTA(1, DOd),
    NOTA(1, DO), NOTA(1, DOd), NOTA(1, RE), NOTA(1, REd), NOTA(1, MI), NOTA(1, FA),
    NOTA(1, FAd), NOTA(1, SOL), RITORNA,
};
/* 1C8C-1CA7 comando 1D: elenco dei record (blocco, flusso) */
static const uint8_t cmd_1D[] = {
    REC(0, ADR_cmd_1D_b0_s1), REC(1, ADR_cmd_1D_b1_s1), REC(2, ADR_cmd_1D_b2_s1),
    REC(3, ADR_cmd_1D_b3_s1), REC(4, ADR_cmd_1D_b4_s1), REC(5, ADR_cmd_1D_b5_s1),
    REC(6, ADR_cmd_1D_b6_s1), REC(7, ADR_cmd_1D_b7_s1), REC(8, ADR_cmd_1D_b8_s1), FINE_ELENCO,
};
/* 1CA8-1CC2 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_1D_b0_s1[] = {
    STRUM(12), ATTEN(4), DUR(29), NOTA(5, DO), DUR(23), NOTA(4, LAd), DUR(19), NOTA(4, SOLd),
    DUR(17), NOTA(4, FAd), DUR(21), NOTA(4, MI), DUR(27), NOTA(4, RE), STRUM(14), DUR(72),
    NOTA(4, DO), DUR(1), PAUSA, DUR(84), NOTA(4, SOL), DUR(1), PAUSA, FINE,
};
/* 1CC3-1CDD flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t cmd_1D_b1_s1[] = {
    STRUM(12), ATTEN(5), DUR(29), NOTA(6, DO), DUR(23), NOTA(5, LAd), DUR(19), NOTA(5, SOLd),
    DUR(17), NOTA(5, FAd), DUR(21), NOTA(5, MI), DUR(27), NOTA(5, RE), STRUM(14), DUR(72),
    NOTA(5, DO), DUR(1), PAUSA, DUR(84), NOTA(5, SOL), DUR(1), PAUSA, FINE,
};
/* 1CDE-1CFA flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_1D_b2_s1[] = {
    STRUM(12), ATTEN(4), DUR(15), PAUSA, DUR(26), NOTA(4, SI), DUR(21), NOTA(4, LA), DUR(18),
    NOTA(4, SOL), DUR(18), NOTA(4, FA), DUR(24), NOTA(4, REd), DUR(82), NOTA(4, DOd), DUR(0),
    PAUSA, STRUM(14), DUR(84), NOTA(4, MI), DUR(5), PAUSA, DUR(37), NOTA(5, DO), FINE,
};
/* 1CFB-1D17 flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t cmd_1D_b3_s1[] = {
    STRUM(12), ATTEN(5), DUR(15), PAUSA, DUR(26), NOTA(5, SI), DUR(21), NOTA(5, LA), DUR(18),
    NOTA(5, SOL), DUR(18), NOTA(5, FA), DUR(24), NOTA(5, REd), DUR(82), NOTA(5, DOd), DUR(0),
    PAUSA, STRUM(14), DUR(84), NOTA(5, MI), DUR(5), PAUSA, DUR(37), NOTA(6, DO), FINE,
};
/* 1D18-1D1C flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t cmd_1D_b4_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 1D1D-1D21 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_1D_b5_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 1D22-1D26 flusso, blocco 6, strato 1 (chip 3 voce 3) */
static const uint8_t cmd_1D_b6_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 1D27-1D2B flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_1D_b7_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 1D2C-1D30 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_1D_b8_s1[] = {
    STRUM(0), DUR(0), PAUSA, FINE,
};
/* 1D31-1D3D comando 1E: elenco dei record (blocco, flusso) */
static const uint8_t cmd_1E[] = {
    REC(2, ADR_cmd_1E_b2_s1), REC(5, ADR_cmd_1E_b5_s1), REC(7, ADR_cmd_1E_b7_s1),
    REC(8, ADR_cmd_1E_b8_s1), FINE_ELENCO,
};
/* 1D3E-1D62 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_1E_b2_s1[] = {
    STRUM(8), TRASP(-2), DUR(10), ATTEN(6), NOTA(4, FA), NOTA(4, SOL), NOTA(4, FA),
    NOTA(4, SOL), NOTA(4, FA), NOTA(4, SOL), ATTEN(5), NOTA(4, FA), NOTA(4, SOL), NOTA(4, FA),
    NOTA(4, SOL), NOTA(4, FA), NOTA(4, SOL), ATTEN(4), NOTA(4, FA), NOTA(4, SOL), NOTA(4, FA),
    NOTA(4, SOL), NOTA(4, FA), NOTA(4, SOL), ATTEN(3), NOTA(4, FA), NOTA(4, SOL), NOTA(4, FA),
    NOTA(4, SOL), NOTA(4, FA), NOTA(4, SOL),
};
/* 1D63-1DB1 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1E_b2_s1_p2[] = {
    RIP_INIZIO(3), NOTA(5, RE), NOTA(4, RE), NOTA(5, SOL), NOTA(4, SOL), NOTA(5, FA),
    NOTA(4, FA), NOTA(5, RE), NOTA(4, DOd), NOTA(4, LAd), NOTA(3, LAd), NOTA(5, DO),
    NOTA(4, DO), RIP_FINE,
    NOTA(5, DOd), NOTA(4, DOd), NOTA(5, FA), NOTA(4, FA), NOTA(5, SOLd), NOTA(4, SOLd),
    NOTA(5, SOL), NOTA(4, SOL), NOTA(5, REd), NOTA(4, REd), NOTA(5, DO), NOTA(4, DO),
    NOTA(5, DOd), NOTA(4, DOd), NOTA(4, LAd), NOTA(3, LAd), NOTA(4, FAd), NOTA(3, FAd),
    NOTA(4, FA), NOTA(3, FA), NOTA(4, LA), NOTA(3, LA), NOTA(5, DO), NOTA(4, DO), NOTA(4, LAd),
    NOTA(3, LAd), NOTA(5, RE), NOTA(4, RE), NOTA(5, FA), NOTA(4, FA), NOTA(5, SOLd),
    NOTA(4, SOLd), NOTA(5, FA), NOTA(4, FA), NOTA(5, RE), NOTA(4, RE), NOTA(5, SOL),
    NOTA(4, SOL), NOTA(5, REd), NOTA(4, REd), NOTA(5, DO), NOTA(4, DO), NOTA(4, LA),
    NOTA(3, LA), NOTA(4, SOL), NOTA(3, SOL), NOTA(4, LA), NOTA(3, LA), NOTA(4, LAd),
    NOTA(3, LAd), NOTA(5, RE), NOTA(4, RE), NOTA(5, FA), NOTA(4, FA), NOTA(5, LAd),
    NOTA(4, LAd), NOTA(4, LAd), NOTA(3, LAd), NOTA(5, LAd), NOTA(4, LAd),
    SALTA(ADR_cmd_1E_b2_s1_p2),
    B(0xFF),
};
/* 1DB2-1DD6 flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_1E_b5_s1[] = {
    STRUM(8), TRASP(-2), DUR(10), ATTEN(6), NOTA(4, RE), NOTA(4, REd), NOTA(4, RE),
    NOTA(4, REd), NOTA(4, RE), NOTA(4, REd), ATTEN(5), NOTA(4, RE), NOTA(4, REd), NOTA(4, RE),
    NOTA(4, REd), NOTA(4, RE), NOTA(4, REd), ATTEN(4), NOTA(4, RE), NOTA(4, REd), NOTA(4, RE),
    NOTA(4, REd), NOTA(4, RE), NOTA(4, REd), ATTEN(3), NOTA(4, RE), NOTA(4, REd), NOTA(4, RE),
    NOTA(4, REd), NOTA(4, RE), NOTA(4, REd),
};
/* 1DD7-1E25 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1E_b5_s1_p2[] = {
    RIP_INIZIO(3), NOTA(4, LAd), NOTA(3, LAd), NOTA(5, RE), NOTA(4, RE), NOTA(5, RE),
    NOTA(4, RE), NOTA(4, LA), NOTA(3, LA), NOTA(4, SOL), NOTA(3, SOL), NOTA(4, LA), NOTA(3, LA),
    RIP_FINE,
    NOTA(4, LAd), NOTA(3, LAd), NOTA(5, DOd), NOTA(4, DOd), NOTA(5, FA), NOTA(4, FA),
    NOTA(5, MI), NOTA(4, MI), NOTA(5, DO), NOTA(4, DO), NOTA(4, SOL), NOTA(3, SOL),
    NOTA(4, LAd), NOTA(3, LAd), NOTA(4, FAd), NOTA(3, FAd), NOTA(4, DOd), NOTA(3, DOd),
    NOTA(4, DO), NOTA(3, DO), NOTA(4, FA), NOTA(3, FA), NOTA(4, LA), NOTA(3, LA), NOTA(4, FA),
    NOTA(3, FA), NOTA(4, LAd), NOTA(3, LAd), NOTA(5, RE), NOTA(4, RE), NOTA(5, FA), NOTA(4, FA),
    NOTA(5, RE), NOTA(4, RE), NOTA(4, LAd), NOTA(3, LAd), NOTA(5, REd), NOTA(4, REd),
    NOTA(5, DO), NOTA(4, DO), NOTA(4, SOL), NOTA(3, SOL), NOTA(4, FA), NOTA(3, FA),
    NOTA(4, REd), NOTA(3, REd), NOTA(4, FA), NOTA(3, FA), NOTA(4, FA), NOTA(3, FA),
    NOTA(4, LAd), NOTA(3, LAd), NOTA(5, RE), NOTA(4, RE), NOTA(5, FA), NOTA(4, FA), NOTA(4, FA),
    NOTA(3, FA), NOTA(5, FA), NOTA(4, FA), SALTA(ADR_cmd_1E_b5_s1_p2),
    B(0xFF),
};
/* 1E26-1E4A flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_1E_b7_s1[] = {
    STRUM(8), TRASP(-2), DUR(10), ATTEN(6), NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd),
    NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), ATTEN(5), NOTA(3, LAd), NOTA(3, LAd),
    NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), ATTEN(4), NOTA(3, LAd),
    NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), ATTEN(3),
    NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd), NOTA(3, LAd),
};
/* 1E4B-1E99 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1E_b7_s1_p2[] = {
    RIP_INIZIO(3), NOTA(4, FA), NOTA(3, FA), NOTA(4, LAd), NOTA(3, LAd), NOTA(4, LAd),
    NOTA(3, LAd), NOTA(4, FA), NOTA(3, FA), NOTA(4, REd), NOTA(3, REd), NOTA(4, FA),
    NOTA(3, FA), RIP_FINE,
    NOTA(4, FA), NOTA(3, FA), NOTA(4, SOLd), NOTA(3, SOLd), NOTA(5, DOd), NOTA(4, DOd),
    NOTA(5, DO), NOTA(4, DO), NOTA(4, SOL), NOTA(3, SOL), NOTA(4, REd), NOTA(3, REd),
    NOTA(4, FAd), NOTA(3, FAd), NOTA(4, DOd), NOTA(3, DOd), NOTA(3, LAd), NOTA(2, LAd),
    NOTA(3, LA), NOTA(2, LA), NOTA(4, DO), NOTA(3, DO), NOTA(4, FA), NOTA(3, FA), NOTA(4, RE),
    NOTA(3, RE), NOTA(4, SOL), NOTA(3, SOL), NOTA(4, LAd), NOTA(3, LAd), NOTA(5, RE),
    NOTA(4, RE), NOTA(4, LAd), NOTA(3, LAd), NOTA(4, FA), NOTA(3, FA), NOTA(5, DO), NOTA(4, DO),
    NOTA(4, SOLd), NOTA(3, SOLd), NOTA(4, REd), NOTA(3, REd), NOTA(4, DO), NOTA(3, DO),
    NOTA(4, DO), NOTA(3, DO), NOTA(4, DO), NOTA(3, DO), NOTA(4, RE), NOTA(3, RE), NOTA(4, SOL),
    NOTA(3, SOL), NOTA(4, LAd), NOTA(3, LAd), NOTA(5, RE), NOTA(4, RE), NOTA(4, RE),
    NOTA(3, RE), NOTA(5, RE), NOTA(4, RE), SALTA(ADR_cmd_1E_b7_s1_p2),
    B(0xFF),
};
/* 1E9A-1EBE flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_1E_b8_s1[] = {
    STRUM(8), TRASP(-2), DUR(10), ATTEN(6), NOTA(3, FA), NOTA(3, SOL), NOTA(3, FA),
    NOTA(3, SOL), NOTA(3, FA), NOTA(3, SOL), ATTEN(5), NOTA(3, FA), NOTA(3, SOL), NOTA(3, FA),
    NOTA(3, SOL), NOTA(3, FA), NOTA(3, SOL), ATTEN(4), NOTA(3, FA), NOTA(3, SOL), NOTA(3, FA),
    NOTA(3, SOL), NOTA(3, FA), NOTA(3, SOL), ATTEN(3), NOTA(3, FA), NOTA(3, SOL), NOTA(3, FA),
    NOTA(3, SOL), NOTA(3, FA), NOTA(3, SOL),
};
/* 1EBF-1F0D ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1E_b8_s1_p2[] = {
    RIP_INIZIO(3), NOTA(4, RE), NOTA(3, RE), NOTA(4, SOL), NOTA(3, SOL), NOTA(4, FA),
    NOTA(3, FA), NOTA(4, RE), NOTA(3, RE), NOTA(4, DO), NOTA(3, DO), NOTA(4, RE), NOTA(3, RE),
    RIP_FINE,
    NOTA(4, DOd), NOTA(3, DOd), NOTA(4, FA), NOTA(3, FA), NOTA(4, SOLd), NOTA(3, SOLd),
    NOTA(4, SOL), NOTA(3, SOL), NOTA(4, REd), NOTA(3, REd), NOTA(4, DO), NOTA(3, DO),
    NOTA(4, DOd), NOTA(3, DOd), NOTA(3, LAd), NOTA(2, LAd), NOTA(3, FAd), NOTA(2, FAd),
    NOTA(3, FA), NOTA(2, FA), NOTA(3, LA), NOTA(2, LA), NOTA(4, DO), NOTA(3, DO), NOTA(3, LAd),
    NOTA(2, LAd), NOTA(4, RE), NOTA(3, RE), NOTA(4, SOL), NOTA(3, SOL), NOTA(4, LAd),
    NOTA(3, LAd), NOTA(4, SOL), NOTA(3, SOL), NOTA(4, RE), NOTA(3, RE), NOTA(4, SOLd),
    NOTA(3, SOLd), NOTA(4, FA), NOTA(3, FA), NOTA(4, DO), NOTA(3, DO), NOTA(3, LA), NOTA(2, LA),
    NOTA(3, LA), NOTA(2, LA), NOTA(3, LA), NOTA(2, LA), NOTA(3, LAd), NOTA(2, LAd), NOTA(4, MI),
    NOTA(3, MI), NOTA(4, SOL), NOTA(3, SOL), NOTA(4, LAd), NOTA(3, LAd), NOTA(3, LAd),
    NOTA(2, LAd), NOTA(4, LAd), NOTA(3, LAd), SALTA(ADR_cmd_1E_b8_s1_p2),
    B(0xFF),
};
/* 1F0E-1F29 comando 1F: elenco dei record (blocco, flusso) */
static const uint8_t cmd_1F[] = {
    REC(0, ADR_cmd_1F_b0_s1), REC(1, ADR_cmd_1F_b1_s1), REC(2, ADR_cmd_1F_b2_s1),
    REC(3, ADR_cmd_1F_b3_s1), REC(4, ADR_cmd_1F_b4_s1), REC(5, ADR_cmd_1F_b5_s1),
    REC(6, ADR_cmd_1F_b6_s1), REC(7, ADR_cmd_1F_b7_s1), REC(8, ADR_cmd_1F_b8_s1), FINE_ELENCO,
};
/* 1F2A-1F4F flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_1F_b0_s1[] = {
    RIP_INIZIO(2), STRUM(11), DUR(15), ATTEN(4), RUMORE(8), NOTA(0, DO), ATTEN(10), RUMORE(8),
    NOTA(0, DO), ATTEN(4), RUMORE(8), NOTA(0, DO), STRUM(0), ATTEN(4), RUMORE(8), NOTA(0, DO),
    ATTEN(10), RUMORE(8), NOTA(0, DO), ATTEN(4), RUMORE(8), NOTA(0, DO), RIP_FINE,
};
/* 1F50-1FB4 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1F_b0_s1_p2[] = {
    RIP_INIZIO(2), STRUM(11), DUR(15), ATTEN(4), RUMORE(8), NOTA(0, DO), ATTEN(10), RUMORE(8),
    NOTA(0, DO), ATTEN(5), RUMORE(8), NOTA(0, DO), STRUM(0), ATTEN(4), RUMORE(8), NOTA(0, DO),
    ATTEN(10), RUMORE(8), NOTA(0, DO), ATTEN(4), RUMORE(8), NOTA(0, DO), RIP_FINE,
    STRUM(11), ATTEN(4), RUMORE(8), NOTA(0, DO), ATTEN(10), RUMORE(8), NOTA(0, DO), ATTEN(4),
    RUMORE(8), NOTA(0, DO), STRUM(0), ATTEN(4), RUMORE(8), NOTA(0, DO), ATTEN(4), RUMORE(8),
    NOTA(0, DO), ATTEN(4), RUMORE(8), NOTA(0, DO), STRUM(11), ATTEN(4), RUMORE(8), NOTA(0, DO),
    ATTEN(10), RUMORE(8), NOTA(0, DO), ATTEN(5), RUMORE(8), NOTA(0, DO), STRUM(0), DUR(47),
    ATTEN(2), RUMORE(14), NOTA(0, DO), SALTA(ADR_cmd_1F_b0_s1_p2),
    B(0xFF),
};
/* 1FB5-1FBF flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t cmd_1F_b1_s1[] = {
    STRUM(11), DUR(15), ATTEN(2), RIP_INIZIO(4), NOTA(2, DO), PAUSA, NOTA(2, DO), RIP_FINE,
};
/* 1FC0-1FD6 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1F_b1_s1_p2[] = {
    RIP_INIZIO(4), ATTEN(2), NOTA(2, DO), PAUSA, NOTA(2, DO), RIP_FINE,
    RIP_INIZIO(2), ATTEN(3), NOTA(2, DO), PAUSA, NOTA(2, DO), NOTA(2, DO), NOTA(2, DO),
    NOTA(2, DO), RIP_FINE,
    SALTA(ADR_cmd_1F_b1_s1_p2),
    B(0xFF),
};
/* 1FD7-1FF6 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_1F_b2_s1[] = {
    STRUM(11), DUR(15), ATTEN(4), RIP_INIZIO(4), NOTA(1, DO), PAUSA, NOTA(1, DO), RIP_FINE,
    RIP_INIZIO(4), ATTEN(4), NOTA(1, DO), PAUSA, NOTA(1, DO), RIP_FINE,
    RIP_INIZIO(2), NOTA(1, DO), PAUSA, NOTA(1, DO), NOTA(1, DO), NOTA(1, DO), NOTA(1, DO),
    RIP_FINE,
    SALTA(ADR_cmd_1F_b2_s1),
    B(0xFF),
};
/* 1FF7-1FFF flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t cmd_1F_b3_s1[] = {
    STRUM(14), ATTEN(3), DUR(47), PAUSA, PAUSA, PAUSA, NOTA(3, SOL),
};
/* 2000-204C ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1F_b3_s1_p2[] = {
    DUR(79), NOTA(4, DO), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, NOTA(4, RE), DUR(31),
    NOTA(4, MI), DUR(15), NOTA(4, FA), DUR(47), NOTA(4, SOL), NOTA(4, DO), NOTA(3, SI),
    NOTA(4, DO), DUR(79), NOTA(4, LA), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, NOTA(4, SOL),
    DUR(31), NOTA(4, FA), DUR(15), NOTA(4, LA), DUR(79), NOTA(4, SOL), DUR(15), PAUSA, DUR(47),
    PAUSA, NOTA(3, SOL), DUR(79), NOTA(4, FA), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA,
    NOTA(4, MI), DUR(31), NOTA(4, RE), DUR(15), NOTA(4, FA), DUR(47), NOTA(4, MI), NOTA(4, DO),
    PAUSA, NOTA(3, SOL), DUR(15), NOTA(3, LAd), NOTA(3, LA), NOTA(3, LAd), NOTA(3, LA),
    NOTA(3, LAd), NOTA(3, LA), NOTA(3, LAd), NOTA(3, LA), NOTA(3, LAd), NOTA(3, LA),
    NOTA(3, LAd), NOTA(3, SI), DUR(79), NOTA(4, DO), DUR(15), PAUSA, DUR(47), PAUSA,
    NOTA(3, SOL), SALTA(ADR_cmd_1F_b3_s1_p2),
    B(0xFF),
};
/* 204D-2055 flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t cmd_1F_b4_s1[] = {
    STRUM(14), ATTEN(3), DUR(47), PAUSA, PAUSA, PAUSA, NOTA(3, MI),
};
/* 2056-20A2 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1F_b4_s1_p2[] = {
    DUR(79), NOTA(3, SOL), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, NOTA(3, LA), DUR(31),
    NOTA(3, SI), DUR(15), NOTA(4, DO), DUR(47), NOTA(4, MI), NOTA(3, SOL), NOTA(3, SOL),
    NOTA(3, SOL), DUR(79), NOTA(4, FA), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, NOTA(4, MI),
    DUR(31), NOTA(4, DO), DUR(15), NOTA(4, FA), DUR(79), NOTA(4, MI), DUR(15), PAUSA, DUR(47),
    PAUSA, NOTA(3, MI), DUR(79), NOTA(4, DO), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA,
    NOTA(3, SI), DUR(31), NOTA(3, LA), DUR(15), NOTA(4, DO), DUR(47), NOTA(4, DO), NOTA(3, SOL),
    PAUSA, NOTA(3, MI), DUR(15), NOTA(3, SOL), NOTA(3, FAd), NOTA(3, SOL), NOTA(3, FAd),
    NOTA(3, SOL), NOTA(3, FAd), NOTA(3, SOL), NOTA(3, FAd), NOTA(3, SOL), NOTA(3, FAd),
    NOTA(3, SOL), NOTA(3, SOL), DUR(79), NOTA(3, SOL), DUR(15), PAUSA, DUR(47), PAUSA,
    NOTA(3, MI), SALTA(ADR_cmd_1F_b4_s1_p2),
    B(0xFF),
};
/* 20A3-20AB flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_1F_b5_s1[] = {
    STRUM(14), ATTEN(3), DUR(47), PAUSA, PAUSA, PAUSA, NOTA(3, DO),
};
/* 20AC-20F8 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1F_b5_s1_p2[] = {
    DUR(79), NOTA(3, MI), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, NOTA(3, FA), DUR(31),
    NOTA(3, SOL), DUR(15), NOTA(3, LA), DUR(47), NOTA(4, DO), NOTA(3, MI), NOTA(3, RE),
    NOTA(3, MI), DUR(79), NOTA(4, DO), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, NOTA(4, DO),
    DUR(31), NOTA(3, LA), DUR(15), NOTA(4, DO), DUR(79), NOTA(4, DO), DUR(15), PAUSA, DUR(47),
    PAUSA, NOTA(3, DO), DUR(79), NOTA(3, LA), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA,
    NOTA(3, SOL), DUR(31), NOTA(3, FA), DUR(15), NOTA(3, LA), DUR(47), NOTA(3, SOL),
    NOTA(3, MI), PAUSA, NOTA(3, DO), DUR(15), NOTA(3, REd), NOTA(3, RE), NOTA(3, REd),
    NOTA(3, RE), NOTA(3, REd), NOTA(3, RE), NOTA(3, REd), NOTA(3, RE), NOTA(3, REd),
    NOTA(3, RE), NOTA(3, REd), NOTA(3, RE), DUR(79), NOTA(3, MI), DUR(15), PAUSA, DUR(47),
    PAUSA, NOTA(3, DO), SALTA(ADR_cmd_1F_b5_s1_p2),
    B(0xFF),
};
/* 20F9-2103 flusso, blocco 6, strato 1 (chip 3 voce 3) */
static const uint8_t cmd_1F_b6_s1[] = {
    STRUM(14), ATTEN(2), DUR(47), PAUSA, PAUSA, PAUSA, PERIODO(0x00EC),
};
/* 2104-2181 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1F_b6_s1_p2[] = {
    DUR(79), PERIODO(0x00B0), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, PERIODO(0x009D), DUR(31),
    PERIODO(0x008B), DUR(15), PERIODO(0x0083), DUR(47), PERIODO(0x0075), PERIODO(0x00B0),
    PERIODO(0x00BB), PERIODO(0x00B0), DUR(79), PERIODO(0x0068), DUR(15), PAUSA, DUR(15), PAUSA,
    PAUSA, PERIODO(0x0075), DUR(31), PERIODO(0x0083), DUR(15), PERIODO(0x0068), DUR(79),
    PERIODO(0x0075), DUR(15), PAUSA, DUR(47), PAUSA, PERIODO(0x00EC), DUR(79), PERIODO(0x0083),
    DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, PERIODO(0x008B), DUR(31), PERIODO(0x009D), DUR(15),
    PERIODO(0x0083), DUR(47), PERIODO(0x008B), PERIODO(0x00B0), PAUSA, PERIODO(0x00EC), DUR(15),
    RIP_INIZIO(5), PERIODO(0x00C6), PERIODO(0x00D2), RIP_FINE,
    PERIODO(0x00C6), PERIODO(0x00BB), DUR(79), PERIODO(0x00B0), DUR(15), PAUSA, DUR(47), PAUSA,
    PERIODO(0x00EC), SALTA(ADR_cmd_1F_b6_s1_p2),
    B(0xFF),
};
/* 2182-218C flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_1F_b7_s1[] = {
    STRUM(14), ATTEN(3), DUR(47), PAUSA, PAUSA, PAUSA, PERIODO(0x0118),
};
/* 218D-220A ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1F_b7_s1_p2[] = {
    DUR(79), PERIODO(0x00EC), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, PERIODO(0x00D2), DUR(31),
    PERIODO(0x00BB), DUR(15), PERIODO(0x00B0), DUR(47), PERIODO(0x008B), PERIODO(0x00EC),
    PERIODO(0x00EC), PERIODO(0x00EC), DUR(79), PERIODO(0x0083), DUR(15), PAUSA, DUR(15), PAUSA,
    PAUSA, PERIODO(0x008B), DUR(31), PERIODO(0x00B0), DUR(15), PERIODO(0x0083), DUR(79),
    PERIODO(0x008B), DUR(15), PAUSA, DUR(47), PAUSA, PERIODO(0x0118), DUR(79), PERIODO(0x00B0),
    DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, PERIODO(0x00BB), DUR(31), PERIODO(0x00D2), DUR(15),
    PERIODO(0x00B0), DUR(47), PERIODO(0x00B0), PERIODO(0x00EC), PAUSA, PERIODO(0x0118), DUR(15),
    RIP_INIZIO(5), PERIODO(0x00EC), PERIODO(0x00FA), RIP_FINE,
    PERIODO(0x00EC), PERIODO(0x00EC), DUR(79), PERIODO(0x00EC), DUR(15), PAUSA, DUR(47), PAUSA,
    PERIODO(0x0118), SALTA(ADR_cmd_1F_b7_s1_p2),
    B(0xFF),
};
/* 220B-2215 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_1F_b8_s1[] = {
    STRUM(14), ATTEN(3), DUR(47), PAUSA, PAUSA, PAUSA, PERIODO(0x0162),
};
/* 2216-2293 ingresso interno (bersaglio di F4/F7) */
static const uint8_t cmd_1F_b8_s1_p2[] = {
    DUR(79), PERIODO(0x0118), DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, PERIODO(0x0108), DUR(31),
    PERIODO(0x00EC), DUR(15), PERIODO(0x00D2), DUR(47), PERIODO(0x00B0), PERIODO(0x0118),
    PERIODO(0x013B), PERIODO(0x0118), DUR(79), PERIODO(0x00B0), DUR(15), PAUSA, DUR(15), PAUSA,
    PAUSA, PERIODO(0x00B0), DUR(31), PERIODO(0x00D2), DUR(15), PERIODO(0x00B0), DUR(79),
    PERIODO(0x00B0), DUR(15), PAUSA, DUR(47), PAUSA, PERIODO(0x0162), DUR(79), PERIODO(0x00D2),
    DUR(15), PAUSA, DUR(15), PAUSA, PAUSA, PERIODO(0x00EC), DUR(31), PERIODO(0x0108), DUR(15),
    PERIODO(0x00D2), DUR(47), PERIODO(0x00EC), PERIODO(0x0118), PAUSA, PERIODO(0x0161), DUR(15),
    RIP_INIZIO(5), PERIODO(0x012A), PERIODO(0x013B), RIP_FINE,
    PERIODO(0x012A), PERIODO(0x013B), DUR(79), PERIODO(0x0118), DUR(15), PAUSA, DUR(47), PAUSA,
    PERIODO(0x0161), SALTA(ADR_cmd_1F_b8_s1_p2),
    B(0xFF),
};

const struct aud_pezzo aud_pezzi_spartiti_c[] = {
    PEZZO(cmd_1B),
    PEZZO(cmd_1B_b2_s1),
    PEZZO(cmd_1B_b2_s1_p2),
    PEZZO(cmd_1B_b5_s1),
    PEZZO(cmd_1B_b5_s1_p2),
    PEZZO(cmd_1B_b7_s1),
    PEZZO(cmd_1B_b7_s1_p2),
    PEZZO(cmd_1B_b8_s1),
    PEZZO(cmd_1B_b8_s1_p2),
    PEZZO(cmd_1C),
    PEZZO(cmd_1C_b2_s1),
    PEZZO(cmd_1C_b2_s1_p2),
    PEZZO(cmd_1C_b5_s1),
    PEZZO(cmd_1C_b7_s1),
    PEZZO(cmd_1C_b8_s1),
    PEZZO(cmd_1C_b8_s1_p2),
    PEZZO(cmd_1D),
    PEZZO(cmd_1D_b0_s1),
    PEZZO(cmd_1D_b1_s1),
    PEZZO(cmd_1D_b2_s1),
    PEZZO(cmd_1D_b3_s1),
    PEZZO(cmd_1D_b4_s1),
    PEZZO(cmd_1D_b5_s1),
    PEZZO(cmd_1D_b6_s1),
    PEZZO(cmd_1D_b7_s1),
    PEZZO(cmd_1D_b8_s1),
    PEZZO(cmd_1E),
    PEZZO(cmd_1E_b2_s1),
    PEZZO(cmd_1E_b2_s1_p2),
    PEZZO(cmd_1E_b5_s1),
    PEZZO(cmd_1E_b5_s1_p2),
    PEZZO(cmd_1E_b7_s1),
    PEZZO(cmd_1E_b7_s1_p2),
    PEZZO(cmd_1E_b8_s1),
    PEZZO(cmd_1E_b8_s1_p2),
    PEZZO(cmd_1F),
    PEZZO(cmd_1F_b0_s1),
    PEZZO(cmd_1F_b0_s1_p2),
    PEZZO(cmd_1F_b1_s1),
    PEZZO(cmd_1F_b1_s1_p2),
    PEZZO(cmd_1F_b2_s1),
    PEZZO(cmd_1F_b3_s1),
    PEZZO(cmd_1F_b3_s1_p2),
    PEZZO(cmd_1F_b4_s1),
    PEZZO(cmd_1F_b4_s1_p2),
    PEZZO(cmd_1F_b5_s1),
    PEZZO(cmd_1F_b5_s1_p2),
    PEZZO(cmd_1F_b6_s1),
    PEZZO(cmd_1F_b6_s1_p2),
    PEZZO(cmd_1F_b7_s1),
    PEZZO(cmd_1F_b7_s1_p2),
    PEZZO(cmd_1F_b8_s1),
    PEZZO(cmd_1F_b8_s1_p2),
};
const unsigned aud_n_pezzi_spartiti_c =
    sizeof aud_pezzi_spartiti_c / sizeof aud_pezzi_spartiti_c[0];
