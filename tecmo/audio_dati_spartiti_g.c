/* Generato da decomp/estrai_dati.py dalla ROM audio 1.3jk: non modificare
 * a mano. Dati musicali derivati dal programma Tecmo (NOTICE.md). */
/* Elenchi e flussi dei comandi 3F-3F. */
#include <stddef.h>
#include "audio_dati.h"

/* 375C-3792 comando 3F: elenco dei record (blocco, flusso) */
static const uint8_t cmd_3F[] = {
    REC(0, ADR_cmd_3F_b0_s1), REC(1, ADR_cmd_3F_b1_s1), REC(2, ADR_cmd_3F_b2_s1),
    REC(3, ADR_cmd_3F_b3_s1), REC(4, ADR_cmd_3F_b4_s1), REC(5, ADR_cmd_3F_b5_s1),
    REC(6, ADR_cmd_3F_b6_s1), REC(7, ADR_cmd_3F_b7_s1), REC(8, ADR_cmd_3F_b8_s1),
    REC(9, ADR_cmd_3F_b9_s2), REC(10, ADR_cmd_3F_b10_s2), REC(11, ADR_cmd_3F_b11_s2),
    REC(12, ADR_cmd_3F_b12_s2), REC(13, ADR_cmd_3F_b13_s2), REC(14, ADR_cmd_3F_b14_s2),
    REC(15, ADR_cmd_3F_b15_s2), REC(16, ADR_cmd_3F_b16_s2), REC(17, ADR_cmd_3F_b17_s2),
    FINE_ELENCO,
};
/* 3793-3799 flusso, blocco 0, strato 1 (chip 1 voce 3) */
static const uint8_t cmd_3F_b0_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 379A-37A0 flusso, blocco 1, strato 1 (chip 1 voce 2) */
static const uint8_t cmd_3F_b1_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37A1-37A7 flusso, blocco 2, strato 1 (chip 1 voce 1) */
static const uint8_t cmd_3F_b2_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37A8-37AE flusso, blocco 3, strato 1 (chip 2 voce 3) */
static const uint8_t cmd_3F_b3_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37AF-37B5 flusso, blocco 4, strato 1 (chip 2 voce 2) */
static const uint8_t cmd_3F_b4_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37B6-37BC flusso, blocco 5, strato 1 (chip 2 voce 1) */
static const uint8_t cmd_3F_b5_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37BD-37C3 flusso, blocco 6, strato 1 (chip 3 voce 3) */
static const uint8_t cmd_3F_b6_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37C4-37CA flusso, blocco 7, strato 1 (chip 3 voce 2) */
static const uint8_t cmd_3F_b7_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37CB-37D1 flusso, blocco 8, strato 1 (chip 3 voce 1) */
static const uint8_t cmd_3F_b8_s1[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37D2-37D8 flusso, blocco 9, strato 2 (chip 1 voce 3) */
static const uint8_t cmd_3F_b9_s2[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37D9-37DF flusso, blocco 10, strato 2 (chip 1 voce 2) */
static const uint8_t cmd_3F_b10_s2[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37E0-37E6 flusso, blocco 11, strato 2 (chip 1 voce 1) */
static const uint8_t cmd_3F_b11_s2[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37E7-37ED flusso, blocco 12, strato 2 (chip 2 voce 3) */
static const uint8_t cmd_3F_b12_s2[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37EE-37F4 flusso, blocco 13, strato 2 (chip 2 voce 2) */
static const uint8_t cmd_3F_b13_s2[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 37F5-37FB flusso, blocco 14, strato 2 (chip 2 voce 1) */
static const uint8_t cmd_3F_b14_s2[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, B(0x0F),
};
/* 37FC-3802 flusso, blocco 15, strato 2 (chip 3 voce 3) */
static const uint8_t cmd_3F_b15_s2[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 3803-3809 flusso, blocco 16, strato 2 (chip 3 voce 2) */
static const uint8_t cmd_3F_b16_s2[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};
/* 380A-3810 flusso, blocco 17, strato 2 (chip 3 voce 1) */
static const uint8_t cmd_3F_b17_s2[] = {
    STRUM(0), ATTEN(15), DUR(0), PAUSA, FINE,
};

const struct aud_pezzo aud_pezzi_spartiti_g[] = {
    PEZZO(cmd_3F),
    PEZZO(cmd_3F_b0_s1),
    PEZZO(cmd_3F_b1_s1),
    PEZZO(cmd_3F_b2_s1),
    PEZZO(cmd_3F_b3_s1),
    PEZZO(cmd_3F_b4_s1),
    PEZZO(cmd_3F_b5_s1),
    PEZZO(cmd_3F_b6_s1),
    PEZZO(cmd_3F_b7_s1),
    PEZZO(cmd_3F_b8_s1),
    PEZZO(cmd_3F_b9_s2),
    PEZZO(cmd_3F_b10_s2),
    PEZZO(cmd_3F_b11_s2),
    PEZZO(cmd_3F_b12_s2),
    PEZZO(cmd_3F_b13_s2),
    PEZZO(cmd_3F_b14_s2),
    PEZZO(cmd_3F_b15_s2),
    PEZZO(cmd_3F_b16_s2),
    PEZZO(cmd_3F_b17_s2),
};
const unsigned aud_n_pezzi_spartiti_g =
    sizeof aud_pezzi_spartiti_g / sizeof aud_pezzi_spartiti_g[0];
