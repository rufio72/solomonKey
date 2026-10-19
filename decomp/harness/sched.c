/* Spedidore 3AF3 e posta 0493 tradotti uno a uno (vedi SCHEDULER.md). */
#include <stddef.h>
#include "sched.h"

void (*sched_gancio_stub)(uint8_t codice, uint8_t slot) = NULL;
void (*sched_gancio_scansione)(int lettura) = NULL;

/* Una funzione per codice di attività, con l'ingresso della tabella 3B11
 * nel nome. Le routine tradotte sostituiranno lo stub con lo stesso prototipo. */
#define STUB(cod, ingresso) \
    static void attivita_##cod##_##ingresso(uint8_t slot) \
    { if (sched_gancio_stub) sched_gancio_stub(0x##cod, slot); }

STUB(00, 3b61) STUB(01, 48d1) STUB(02, 4b84) STUB(03, 4bc6) STUB(04, 4c30)
STUB(05, 4c42) STUB(06, 75e7) STUB(07, 2ccd) STUB(08, 7c98) STUB(09, 4d29)
STUB(0a, 4da2) STUB(0b, 2c27) STUB(0c, 7bf6) STUB(0d, 5c8e) STUB(0e, 63a4)
STUB(0f, 640e) STUB(10, 457b) STUB(11, 456e) STUB(12, 426a) STUB(13, 6480)
STUB(14, 44b5) STUB(15, 6466) STUB(16, 6496) STUB(17, 6429) STUB(18, 4124)
STUB(19, 3792) STUB(1a, 411d) STUB(1b, 4111) STUB(1c, 3f40) STUB(1d, 3f39)
STUB(1e, 3b61) STUB(1f, 43c8) STUB(20, 3e90) STUB(21, 3b61) STUB(22, 12d6)
STUB(23, 3e4e) STUB(24, 2c72) STUB(25, 3b77) STUB(26, 3b63) STUB(27, 62b6)

attivita_fn corpo_attivita[SCHED_CODICI] = {
    attivita_00_3b61, attivita_01_48d1, attivita_02_4b84, attivita_03_4bc6,
    attivita_04_4c30, attivita_05_4c42, attivita_06_75e7, attivita_07_2ccd,
    attivita_08_7c98, attivita_09_4d29, attivita_0a_4da2, attivita_0b_2c27,
    attivita_0c_7bf6, attivita_0d_5c8e, attivita_0e_63a4, attivita_0f_640e,
    attivita_10_457b, attivita_11_456e, attivita_12_426a, attivita_13_6480,
    attivita_14_44b5, attivita_15_6466, attivita_16_6496, attivita_17_6429,
    attivita_18_4124, attivita_19_3792, attivita_1a_411d, attivita_1b_4111,
    attivita_1c_3f40, attivita_1d_3f39, attivita_1e_3b61, attivita_1f_43c8,
    attivita_20_3e90, attivita_21_3b61, attivita_22_12d6, attivita_23_3e4e,
    attivita_24_2c72, attivita_25_3b77, attivita_26_3b63, attivita_27_62b6,
};

uint8_t posta_attivita(uint8_t codice)
{
    /* 0493 ld hl,c04e; ld d,a */
    for (int giri = 0; giri < 256; giri++) {
        /* 0496 ld a,3f; and (hl); inc (hl): c04e cresce senza maschera */
        uint8_t i = prossimo_libero & 0x3f;
        prossimo_libero++;
        /* se lo slot è occupato si ripete */
        if (tabella[i] != 0)
            continue;
        tabella[i] = codice;          /* 04AA ld (hl),d */
        return i;
    }
    return 0xff;
}

int spedisci_una(uint8_t *slot, uint8_t *codice)
{
    uint8_t l = indice;               /* 3AF3 ld a,(c04f); ld l,a; ld h,c0 */
    uint8_t a;
    /* 3AF9 xor a; 3AFA or (hl); jr nz; inc l; res 6,l */
    for (int lettura = 0;; lettura++) {
        if (sched_gancio_scansione)
            sched_gancio_scansione(lettura);
        a = M(0xc000 + l);
        if (a != 0)
            break;
        l = (uint8_t)((l + 1) & ~0x40);
        if (lettura > 4096)
            return 0;
    }
    *slot = l;                        /* 3B07 push hl */
    *codice = a;                      /* 3B02 ld b,a */
    indice = (uint8_t)((l + 1) & ~0x40);   /* 3B08 inc l; res 6,l; ld (c04f),a */
    if (a >= SCHED_CODICI)            /* 3B10 rst 20h fuori tabella */
        return -1;
    corpo_attivita[a](l);             /* il ritorno torna a 3AF3 (push de) */
    return 1;
}
