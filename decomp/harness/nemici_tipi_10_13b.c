/* Motore dei nemici, lotto N5 (brief 31), seconda parte di
 * nemici_tipi_10_13.c: tipo 13 di 4F1B (ingresso 54D8, con le fasi 5584,
 * 551A, 54EA, 5529 e 6BCD, che crea l'oggetto portato) e tipo 11 (ingresso
 * 56E9, con le fasi 5713, 5726, 573C e 5749). Il tipo 10 (719E) e le due
 * fasi del tipo 12 (55C4, 6C1D) stanno in nemici_tipi_10_13.c.
 *
 * IX = slot (c2c1 + i·0x18), IY = oggetto (c4fe + i·0x30). Tradotto da
 * out/main64.dasm (out/nemici_tipi_10_13.dasm); tabelle da rst38_tables.h. */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"

#define PORTATI_48AF   0x48af   /* ROM: indirizzo dell'oggetto per indice */

/* ------------------------------------------------------------------ */
/* Tipo 13 */

/* 0x54D8: smista con `rst 38h` a 54DB sulla fase (IY+1D). */
void sub_54d8(struct z80 *z)
{
    z->e = M(z->iy + 0x1d);                    /* 54D8 */
    uint16_t dest = nemici_rst38(z, 0x54DB, rst38_54DB,
                                 sizeof rst38_54DB / sizeof rst38_54DB[0]);
    switch (dest) {                            /* 54DB rst 38 */
    case 0x5584: sub_5584(z); break;
    case 0x551A: sub_551a(z); break;
    case 0x54EA: sub_54ea(z); break;
    case 0x5529: sub_5529(z); break;
    case 0x3B62: sub_3b62(z); break;
    case 0:      break;                        /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}

/* 0x54EA, fase 2: 5507 con soglia 18; senza riporto va a 561B; altrimenti
 * fase 5 e il bit 1 di (IY+1E) acceso solo se (IY+1A) & 30 == 30. A = quel
 * valore all'uscita. */
void sub_54ea(struct z80 *z)
{
    z->c = 0x18;                               /* 54EA */
    sub_5507(z);                               /* 54EC */
    if (!NEMICI_RIPORTO_ACCESO(z)) {           /* 54EF jp nc */
        sub_561b(z);
        return;
    }
    M(z->iy + 0x1d) = 0x05;                    /* 54F2 */
    z->a = M(z->iy + 0x1a) & 0x30;             /* 54F6-54F9 */
    M(z->iy + 0x1e) |= 0x02;                   /* 54FD */
    if (z->a == 0x30)                          /* 54FB cp, 5501 ret z */
        return;
    M(z->iy + 0x1e) &= ~0x02;                  /* 5502 */
}

/* 0x551A, fase 1: 6C2C con A = 2; senza riporto esce; con (IX+0D) >= 18
 * chiude slot e oggetto (80A5). */
void sub_551a(struct z80 *z)
{
    z->a = 0x02;                               /* 551A */
    sub_6c2c(z);                               /* 551C */
    if (!NEMICI_RIPORTO_ACCESO(z))             /* 551F ret nc */
        return;
    z->a = M(z->ix + 0x0d);                    /* 5520 */
    if (z->a < 0x18)                           /* 5523-5525 ret c */
        return;
    sub_80a5(z);                               /* 5526 jp */
}

/* 0x5529, fase 5. Il bit 0 di (IY+1E) sceglie la maschera 10 o 20 su
 * (IY+1A): con bit acceso va a 5567 (secondo controllo con 5507, poi 5549
 * o, senza riporto, esce se (IY+12) != 0 o va a 5573); altrimenti 5507 e,
 * senza riporto, 5573. 5549: attende (IX+0D) >= 32 (bit 4 di (IY+4)
 * acceso) o 64, poi azzera fase e (IX+0D) con il suono 27. 5573: fase 2. */
void sub_5529(struct z80 *z)
{
    z->a = M(z->iy + 0x1e);                    /* 5529 */
    int c0 = z->a & 0x01;                      /* 552C rrca */
    z->a = (uint8_t)((z->a >> 1) | (c0 << 7));
    z->a = c0 ? 0x20 : 0x10;                   /* 552D-5531 */
    z->a &= M(z->iy + 0x1a);                   /* 5533 */
    if (z->a != 0) {                           /* 5536 jr nz 5567 */
        z->c = 0x18;                           /* 5567 */
        sub_5507(z);                           /* 5569 */
        if (!NEMICI_RIPORTO_ACCESO(z)) {       /* 556C jr c 5549 */
            z->a = M(z->iy + 0x12);            /* 556E */
            if (z->a != 0)                     /* 5571-5572 ret nz */
                return;
            goto l5573;
        }
        goto l5549;
    }
    z->c = 0x18;                               /* 5538 */
    sub_5507(z);                               /* 553A */
    if (!NEMICI_RIPORTO_ACCESO(z))             /* 553D jr nc */
        goto l5573;
    if (M(z->iy + 0x1e) & 0x02)                /* 553F-5543 */
        M(z->iy + 0x1e) &= ~0x02;              /* 5545 */
l5549:
    z->a = M(z->ix + 0x0d);                    /* 5549 */
    z->b = (M(z->iy + 0x04) & 0x10) ? 0x32 : 0x64;  /* 554C-5554 */
    if (z->a < z->b)                           /* 5556-5557 ret c */
        return;
    z->a = 0;                                  /* 5558 */
    M(z->iy + 0x1e) &= ~0x02;                  /* 5559 */
    M(z->iy + 0x1d) = z->a;                    /* 555D */
    M(z->ix + 0x0d) = z->a;                    /* 5560 */
    z->c = 0x27;                               /* 5563 */
    sub_0028(z);                               /* 5565 rst 28 */
    return;
l5573:
    M(z->iy + 0x1e) &= ~0x02;                  /* 5573 */
    M(z->iy + 0x1d) = 0x02;                    /* 5577 */
    M(z->ix + 0x0d) = 0x00;                    /* 557B */
    M(z->ix + 0x01) |= 0x10;                   /* 557F */
}

/* 0x5584, fase 0: con il bit 1 di (IY+1E) spento attende (IX+0D) >= 1c,
 * prende uno slot libero (7379), vi mette 80 e crea con 6BCD l'oggetto
 * portato (B = 0e, C = (IY+1E)); poi accende il bit 1. Con il bit 1
 * acceso attende (IX+0D) >= 48, lo azzera e passa alla fase 2. */
void sub_5584(struct z80 *z)
{
    if (!(M(z->iy + 0x1e) & 0x02)) {           /* 5584-5588 */
        z->a = M(z->ix + 0x0d);                /* 558A */
        if (z->a < 0x1c)                       /* 558D-558F ret c */
            return;
        sub_7379(z);                           /* 5590 */
        if (!NEMICI_RIPORTO_ACCESO(z))         /* 5593 ret nc */
            return;
        M(HL(z)) = 0x80;                       /* 5594 */
        z->c = M(z->iy + 0x1e);                /* 5596 */
        z->b = 0x0e;                           /* 5599 */
        sub_6bcd(z);                           /* 559B */
        M(z->iy + 0x1e) |= 0x02;               /* 559E */
        return;
    }
    z->a = M(z->ix + 0x0d);                    /* 55A3 */
    if (z->a < 0x48)                           /* 55A6-55A8 ret c */
        return;
    M(z->ix + 0x0d) = 0x00;                    /* 55A9 */
    M(z->iy + 0x1d) = 0x02;                    /* 55AD */
}

/* 0x6BCD: l'oggetto di indice A (tabella 48AF) prende IX; posizione di
 * partenza = (IY+15), (IY+0E) spostata di 0b sull'asse X (bit 1 di C
 * spento) o Y (acceso), in negativo se il bit 0 di C è acceso; ritorna
 * nell'oggetto IX con 4AF6 (A = c4, B = 0, E = B d'ingresso). */
void sub_6bcd(struct z80 *z)
{
    SET_HL(z, PORTATI_48AF);                   /* 6BCD */
    sub_0008(z);                               /* 6BD0 rst 8 */
    sub_0010(z);                               /* 6BD1 rst 10 */
    uint16_t ix0 = z->ix;
    z->ix = DE(z);                             /* 6BD2-6BD3 push de; ex (sp),ix */
    z->e = M(z->iy + 0x15);                    /* 6BD5 */
    z->d = M(z->iy + 0x0e);                    /* 6BD8 */
    z->a = 0x0b;                               /* 6BDB */
    if (z->c & 0x02) {                         /* 6BDD */
        if (z->c & 0x01)                       /* 6BEB */
            z->a = (uint8_t)-z->a;             /* 6BEF neg */
        z->a = (uint8_t)(z->a + z->d);         /* 6BF1 */
        z->d = z->a;                           /* 6BF2 */
    } else {
        if (z->c & 0x01)                       /* 6BE1 */
            z->a = (uint8_t)-z->a;             /* 6BE5 */
        z->a = (uint8_t)(z->a + z->e);         /* 6BE7 */
        z->e = z->a;                           /* 6BE8 */
    }
    z->a = 0;                                  /* 6BF3 */
    M(z->ix + 0x0e) = z->d;                    /* 6BF4 */
    M(z->ix + 0x15) = z->e;                    /* 6BF7 */
    M(z->ix + 0x1a) = z->a;                    /* 6BFA */
    z->e = z->b;                               /* 6BFD */
    z->b = z->a;                               /* 6BFE */
    M(z->ix + 0x03) = 0x0b;                    /* 6BFF */
    z->a = 0xc4;                               /* 6C03 */
    sub_4af6(z);                               /* 6C05 */
    z->ix = ix0;                               /* 6C08 pop ix */
}

/* ------------------------------------------------------------------ */
/* Tipo 11 */

/* 0x56E9: BC = (C1A2), o (C1A4) col bit 3 di (IY+4) acceso; 57AA, poi
 * senza prestito 57B3 con A = 1; smista con `rst 38h` a 5704 sulla fase
 * (IY+1D), riletta dopo 57B3. */
void sub_56e9(struct z80 *z)
{
    uint16_t base = (M(z->iy + 0x04) & 0x08) ? 0xc1a4 : 0xc1a2;  /* 56E9-56F5 */
    SET_BC(z, (uint16_t)(M(base) | (M(base + 1) << 8)));         /* 56EF/56F5 */
    sub_57aa(z);                               /* 56F9 */
    z->a = 0x01;                               /* 56FC */
    if (!NEMICI_RIPORTO_ACCESO(z))             /* 56FE call nc */
        sub_57b3(z);
    z->e = M(z->iy + 0x1d);                    /* 5701 */
    uint16_t dest = nemici_rst38(z, 0x5704, rst38_5704,
                                 sizeof rst38_5704 / sizeof rst38_5704[0]);
    switch (dest) {                            /* 5704 rst 38 */
    case 0x571D: sub_571d(z); break;
    case 0x5726: sub_5726(z); break;
    case 0x573C: sub_573c(z); break;
    case 0x3B62: sub_3b62(z); break;
    case 0x5749: sub_5749(z); break;
    case 0x5713: sub_5713(z); break;
    case 0:      break;                        /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}

/* 0x5713, fase 6: con il bit 5 di (IY+0) spento passa alla fase 5. */
void sub_5713(struct z80 *z)
{
    if (M(z->iy) & 0x20)                       /* 5713-5717 ret nz */
        return;
    M(z->iy + 0x1d) = 0x05;                    /* 5718 */
}

/* 0x5726, fase 1: col bit 3 di (IY+4) spento azzera (IX+0B..0C); poi
 * accende il bit 6 di (IX+1) e passa alla fase 2. */
void sub_5726(struct z80 *z)
{
    if (!(M(z->iy + 0x04) & 0x08)) {           /* 5726-572A */
        z->a = 0;                              /* 572C */
        M(z->ix + 0x0b) = z->a;                /* 572D */
        M(z->ix + 0x0c) = z->a;                /* 5730 */
    }
    M(z->ix + 0x01) |= 0x40;                   /* 5733 */
    M(z->iy + 0x1d) = 0x02;                    /* 5737 */
}

/* 0x573C, fasi 2 e 3: (IY+12) = 5, fase 5, bit 1 di (IY+1E) acceso. */
void sub_573c(struct z80 *z)
{
    M(z->iy + 0x12) = 0x05;                    /* 573C */
    M(z->iy + 0x1d) = 0x05;                    /* 5740 */
    M(z->iy + 0x1e) |= 0x02;                   /* 5744 */
}

/* 0x5749, fase 5: con (IY+12) != 0 spegne il bit 6 di (IX+1). Altrimenti,
 * col bit 3 di (IX+1) spento: se il bit 6 è spento guarda la cella davanti
 * (5928): libera (bit 7 spento) torna alla fase 2, altrimenti prende uno
 * slot libero (7379) e vi mette 80, accende il bit 6, memorizza l'indice in
 * (IX+0E) e con 58CF (riporto) va a 5797. Col bit 3 acceso attende
 * (IX+0D) >= 16, inverte (IY+1E) bit 0 con 5917 e va a 5797: spegne il
 * bit 3, accende il bit 5 di (IY+0), fase 2 e 6EB7 con A = (IX+0E). */
void sub_5749(struct z80 *z)
{
    z->a = M(z->iy + 0x12);                    /* 5749 */
    if (z->a != 0) {                           /* 574C-574D */
        M(z->ix + 0x01) &= ~0x40;              /* 574F */
        return;
    }
    if (M(z->ix + 0x01) & 0x08) {              /* 5754-5758 jr nz 5786 */
        z->a = M(z->ix + 0x0d);                /* 5786 */
        if (z->a < 0x16)                       /* 5789-578B ret c */
            return;
        sub_5917(z);                           /* 578C */
        z->a = 0x01 ^ M(z->iy + 0x1e);         /* 578F-5791 */
        M(z->iy + 0x1e) = z->a;                /* 5794 */
    } else {
        if (M(z->ix + 0x01) & 0x40)            /* 575A-575E ret nz */
            return;
        M(z->iy + 0x1e) &= ~0x02;              /* 575F */
        sub_5928(z);                           /* 5763 */
        if (!(leggi(HL(z)) & 0x80)) {          /* 5766-5768 */
            M(z->iy + 0x1d) = 0x02;            /* 576A */
            return;
        }
        sub_7379(z);                           /* 576F */
        if (!NEMICI_RIPORTO_ACCESO(z))         /* 5772 ret nc */
            return;
        M(z->ix + 0x01) |= 0x40;               /* 5773 */
        M(HL(z)) = 0x80;                       /* 5777 */
        M(z->iy + 0x1e) &= ~0x02;              /* 5779 */
        M(z->ix + 0x0e) = z->a;                /* 577D */
        sub_58cf(z);                           /* 5780 */
        if (!NEMICI_RIPORTO_ACCESO(z))         /* 5783-5785 ret */
            return;
    }
    M(z->ix + 0x01) &= ~0x08;                  /* 5797 */
    M(z->iy) |= 0x20;                          /* 579B */
    M(z->iy + 0x1d) = 0x02;                    /* 579F */
    z->a = M(z->ix + 0x0e);                    /* 57A3 */
    sub_6eb7(z);                               /* 57A6 */
}
