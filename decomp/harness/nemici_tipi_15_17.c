/* Motore dei nemici, lotto N1 (brief 25): gestore del tipo 17 (7F68),
 * con le due tabelle di fasi (7F39 per (IY+4) = a1, 7F72 per gli altri),
 * e la fase 4 della tabella 7F39 (7F51). Ingressi: 7F51, 7F68.
 *
 * 7F39 non appartiene a sub_7f2d (z80_7xxx.c), che finisce col `ret` a
 * 7F38: ci si arriva solo dal `jr z` di 7F68, e lo smistamento sta qui.
 * IX = slot, IY = oggetto. Tradotto da out/main64.dasm; tabelle da
 * rst38_tables.h. */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"

/* Destinazioni delle tabelle 7F39 e 7F72. */
static void salta_7f68(struct z80 *z, uint16_t dest)
{
    switch (dest) {
    case 0x3B62: sub_3b62(z); break;
    case 0x7FA6: sub_7fa6(z); break;
    case 0x7F48: sub_7f48(z); break;
    case 0x7FCF: sub_7fcf(z); break;
    case 0x7F51: sub_7f51(z); break;
    case 0x7F81: sub_7f81(z); break;
    case 0x7F8A: sub_7f8a(z); break;
    case 0x801D: sub_801d(z); break;
    case 0x8042: sub_8042(z); break;
    case 0:      break;                        /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}

/* 0x7F51, fase 4 (con (IY+4) = a1): col bit 1 di (IY+1) (IX+0E) = 4 e
 * fase 3; se (IY+1A) ha i bit 4-5 spenti prosegue a 809C. */
void sub_7f51(struct z80 *z)
{
    if (M(z->iy + 0x01) & 0x02) {              /* 7F51-7F55 */
        M(z->ix + 0x0e) = 0x04;                /* 7F57 */
        M(z->iy + 0x1d) = 0x03;                /* 7F5B */
    }
    z->a = M(z->iy + 0x1a) & 0x30;             /* 7F5F-7F62 */
    if (z->a)                                  /* 7F64 ret nz */
        return;
    sub_809c(z);                               /* 7F65 jp 809c */
}

/* 0x7F68: tipo 17, smista sulla fase (IY+1D) con `rst 38h` a 7F39 se
 * (IY+4) = a1, altrimenti a 7F72. */
void sub_7f68(struct z80 *z)
{
    z->e = M(z->iy + 0x1d);                    /* 7F68 */
    z->a = M(z->iy + 0x04);                    /* 7F6B */
    uint16_t dest;
    if (z->a == 0xa1)                          /* 7F6E-7F70 jr z,7f39 */
        dest = nemici_rst38(z, 0x7F39, rst38_7F39,
                            sizeof rst38_7F39 / sizeof rst38_7F39[0]);
    else
        dest = nemici_rst38(z, 0x7F72, rst38_7F72,
                            sizeof rst38_7F72 / sizeof rst38_7F72[0]);
    salta_7f68(z, dest);
}
