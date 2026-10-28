/* Corpi delle attività 01 (48D1) e 10 (457B), tradotti dal disassemblato
 * out/main64.dasm (brief 26). Stesse regole di attivita.c: `pop hl; ld
 * (hl),n; ret` diventa `tabella[slot] = n; return`; i registri del Z80
 * servono solo a passare gli argomenti alle routine tradotte.
 *
 * Routine chiamate: 36F0 e 0028 (rst 28) del banco (routine.h); 4592,
 * 4938 e 49AE del lotto N1 (nemici_comuni.c, prototipi in nemici.h). */
#include "routine.h"
#include "sched.h"
#include "nemici.h"

/* 01, ingresso 48D1: cella della griglia davanti all'oggetto c82e (Dana).
 * Lo scarto orizzontale dipende dal bit 0 di (IY+2C) (16 o FA, bit 0 di
 * B), quello verticale da (IY+29) == 4 (08 o F8, bit 1 di B). Con la
 * cella < 80: c08a = 0, suono 1, 49AE con il bit 0 di (IY+1) acceso; se
 * 49AE dà A = 4 e B = 0 lo slot prende 25, altrimenti A. Con la cella
 * >= 80: suono 2 (cella < F8) o 9, c08a = 0, 4938, slot 02. */
void attivita_01_48d1(uint8_t slot)
{
    struct z80 z = {0};
    z.iy = 0xc82e;                             /* 48D1 */
    z.b = 0x00;                                /* 48D8 */
    uint8_t a = 0x16;                          /* 48DC */
    if (M(z.iy + 0x2c) & 0x01) {               /* 48D5-48DE */
        a = 0xfa;                              /* 48E0 */
        z.b |= 0x01;                           /* 48E2 */
    }
    z.l = (uint8_t)(a + M(z.iy + 0x15));       /* 48E4-48E7 */
    a = 0x08;                                  /* 48ED */
    if (M(z.iy + 0x29) == 0x04) {              /* 48E8-48EF */
        a = 0xf8;                              /* 48F1 */
        z.b |= 0x02;                           /* 48F3 */
    }
    z.h = (uint8_t)(a + M(z.iy + 0x0e));       /* 48F5-48F8 */
    sub_36f0(&z);                              /* 48F9 */
    z.a = M(HL(&z));                           /* 48FC */
    z.ix = 0xc46e;                             /* 48FD */
    SET_HL(&z, 0xc08a);                        /* 4903 */
    if (z.a < 0x80) {                          /* 4901, 4906 jr nc */
        M(z.iy + 0x01) |= 0x01;                /* 4908 */
        M(0xc08a) = 0x00;                      /* 490C */
        z.c = 0x01;                            /* 490E */
        sub_0028(&z);                          /* 4910 rst 28 */
        sub_49ae(&z);                          /* 4911 */
        M(z.iy + 0x01) &= (uint8_t)~0x01;      /* 4914 */
        if (z.a == 0x04 && z.b == 0x00) {      /* 4919-491F rrc b */
            tabella[slot] = 0x25;              /* 4921 */
            return;
        }
        tabella[slot] = z.a;                   /* 4924 */
        return;
    }
    z.c = (z.a < 0xf8) ? 0x02 : 0x09;          /* 4926-492C */
    sub_0028(&z);                              /* 492E rst 28 */
    M(0xc08a) = 0x00;                          /* 492F */
    sub_4938(&z);                              /* 4931 */
    tabella[slot] = 0x02;                      /* 4934-4935 */
}

/* 10, ingresso 457B: l'oggetto c82e tenta di prendere il blocco sotto di
 * sé con l'oggetto c49e (4592). Riporto acceso: slot 0; spento: c466 = 0
 * e slot 11. */
void attivita_10_457b(uint8_t slot)
{
    struct z80 z = {0};
    z.iy = 0xc82e;                             /* 457B */
    z.ix = 0xc49e;                             /* 457F */
    sub_4592(&z);                              /* 4583 */
    if (NEMICI_RIPORTO_ACCESO(&z)) {           /* 4588 jr c */
        tabella[slot] = 0x00;                  /* 458F-4590 */
        return;
    }
    M(0xc466) = 0x00;                          /* 458A */
    tabella[slot] = 0x11;                      /* 458D-4590 */
}
