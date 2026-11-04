/* Corpo dell'attività 22 (12D6) del gioco libero, lotto 54 (brief 54):
 * svuota la coda di raccolta c05b-c060 (3 coppie classe, cella scritte da
 * 12BD, raccolta_12ab in z80_1xxx.c) e per ogni classe diversa da zero
 * chiama 12F0, che salta con `rst 38h` (tabella 130A, 20 voci) al gestore
 * della classe (raccolta_13xx.c e nemici_comparsa.c). Tradotto dal
 * disassemblato out/main64.dasm; il corpo non comincia con `pop hl`: lo
 * slot si azzera alla fine (12EC). */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"

/* 0x12F0: B = classe, cella E. Coordinate della cella (4B0E) in c058-c059,
 * c05a = 0; con classe − 8 < 21 suono 15 e gestore della tabella 130A
 * (indice E = classe − 8, DE = 130B + 2E come `rst 38h`), altrimenti 13F7.
 * Gli indici 14-20 (classi 1C-28) leggerebbero byte di codice dopo la
 * tabella: fuori tabella. */
void sub_12f0(struct z80 *z)
{
    z->b = z->a;                               /* 12F0 */
    z->a = z->e;                               /* 12F1 */
    sub_4b0e(z);                               /* 12F2 */
    M(0xc058) = z->l;                          /* 12F5 */
    M(0xc059) = z->h;
    M(0xc05a) = 0x00;                          /* 12F8-12FA */
    z->a = (uint8_t)(z->b - 0x08);             /* 12FD-12FE */
    if (z->a >= 0x21) {                        /* 1300-1302 */
        gestore_13f7(z);
        return;
    }
    z->c = 0x15;                               /* 1305 */
    sub_0028(z);                               /* 1307 rst 28h */
    uint8_t voce = z->a;
    z->e = voce;                               /* 1308 */
    if (voce >= 0x14) {                        /* 1309 rst 38h */
        nemici_fuori_tabella(0x1309, voce);
        return;
    }
    SET_DE(z, (uint16_t)(0x130b + 2 * voce));
    switch (voce) {
    case 0x00: gestore_13cd(z); break;
    case 0x01: case 0x0d: gestore_13d5(z); break;
    case 0x02: case 0x04: gestore_13d1(z); break;
    case 0x03: sub_13b9(z); break;
    case 0x05: case 0x0e: gestore_13e6(z); break;
    case 0x06: case 0x0f: sub_1413(z); break;
    case 0x07: case 0x10: sub_139c(z); break;
    case 0x08: gestore_1424(z); break;
    case 0x09: gestore_1436(z); break;
    case 0x0a: gestore_14aa(z); break;
    case 0x0b: gestore_1491(z); break;
    case 0x0c: gestore_13c3(z); break;
    case 0x11: sub_13ab(z); break;
    case 0x12: gestore_135b(z); break;
    case 0x13: sub_14d4(z); break;
    }
}

/* 22, ingresso 12D6: le 3 coppie (classe, cella) di c05b si azzerano e per
 * ogni classe diversa da zero si chiama 12F0 (A = classe, E = cella; BC e
 * HL salvati attorno); poi lo slot a zero. */
void attivita_22_12d6(uint8_t slot)
{
    struct z80 z = {0};
    uint16_t hl = 0xc05b;                      /* 12D6 */
    SET_BC(&z, 0x0300);                        /* 12D9 */
    do {
        z.a = M(hl);                           /* 12DC */
        M(hl) = z.c;                           /* 12DD */
        hl++;
        z.e = M(hl);                           /* 12DF */
        M(hl) = z.c;                           /* 12E0 */
        hl++;
        uint8_t b = z.b, c = z.c;              /* 12E2-12E3 push bc, hl */
        if (z.a != 0)                          /* 12E4-12E5 */
            sub_12f0(&z);
        z.b = b;                               /* 12E8-12E9 */
        z.c = c;
        SET_HL(&z, hl);
    } while (--z.b != 0);                      /* 12EA djnz */
    tabella[slot] = 0x00;                      /* 12EC-12ED */
}
