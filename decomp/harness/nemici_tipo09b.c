/* Motore dei nemici, lotto N3 (brief 29): tipo 09, scelta della mossa
 * (5995, voce [5] della tabella 57D3) e le sue code. Ingressi: 5995, 5B12,
 * 5C19, 5C1E, 5C23, 5C29; interni 5B09 (mai eseguita nelle sessioni
 * storiche, quindi senza cattura) e 5C2D.
 *
 * IX = slot (c2c1 + i·0x18), IY = oggetto (c4fe + i·0x30). (IX+11) e
 * (IX+12) = maschere C e B di 5345, (IY+1E) = direzione (bit 0 destra,
 * bit 1 giù), (IY+1D) = fase. 5995 è un groviglio di salti: resta con le
 * etichette dell'originale (goto), così A, BC, DE e HL all'uscita
 * coincidono con quelli misurati. `cy` è il riporto dove un `rra` lo
 * eredita. Tradotto da out/main64.dasm. */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"

#define IXB(o) M(z->ix + (o))
#define IYB(o) M(z->iy + (o))
#define CLASSI_C07E 0xc07e   /* sub_5c32: classi delle celle davanti */
#define PORTATO_C2BA 0xc2ba  /* bit 0: un portato è in arrivo */
#define CELLA_C2BB  0xc2bb

/* rra: A scorre a destra col riporto in bit 7, cy = bit 0 uscito. */
#define RRA(z, cy) \
    do { int cin_ = (cy); (cy) = (z)->a & 1; \
         (z)->a = (uint8_t)(((z)->a >> 1) | (cin_ << 7)); } while (0)

/* 0x5B09 (mai eseguita): fase 0, bit 1 della direzione. */
static void sub_5b09(struct z80 *z)
{
    IYB(0x1d) = 0x00;                          /* 5B09 */
    IYB(0x1e) |= 0x02;                         /* 5B0D */
}

/* 0x5B12: A = ((IY+15) - 6) & 0f; riporto acceso se A < 5. */
void sub_5b12(struct z80 *z)
{
    z->a = (uint8_t)((IYB(0x15) - 0x06) & 0x0f);   /* 5B12-5B17 */
    NEMICI_PONI_RIPORTO(z, z->a < 0x05);       /* 5B19 cp 5 */
}

/* 0x5C19 / 0x5C1E: direzione 0 / 1 (destinazioni 0, 2 e 7, 9 di 5BFC). */
void sub_5c19(struct z80 *z)
{
    (void)z;
    IYB(0x1e) = 0x00;                          /* 5C19 */
}

void sub_5c1e(struct z80 *z)
{
    (void)z;
    IYB(0x1e) = 0x01;                          /* 5C1E */
}

/* 0x5C2D (ingresso interno): fase 0. */
static void sub_5c2d(struct z80 *z)
{
    (void)z;
    IYB(0x1d) = 0x00;                          /* 5C2D */
}

/* 0x5C23 / 0x5C29: direzione 0 / 1, poi fase 0 (5C2D). */
void sub_5c23(struct z80 *z)
{
    IYB(0x1e) = 0x00;                          /* 5C23 */
    sub_5c2d(z);                               /* 5C27 */
}

void sub_5c29(struct z80 *z)
{
    IYB(0x1e) = 0x01;                          /* 5C29 */
    sub_5c2d(z);
}

/* Riporto lasciato da 5B12, 7379 (z->f) o, per 5C32, dall'ultimo confronto
 * (best effort: serve solo come riporto d'ingresso di un rra). */
static int riporto(struct z80 *z)
{
    return NEMICI_RIPORTO_ACCESO(z);
}

/* 0x5995: sceglie la mossa dell'oggetto IY secondo le maschere di 5345
 * (BC), la direzione, le celle davanti (5C32, 5928) e il caso (R), poi
 * 5BF2: l'indice scelto da 51D7 smista con `rst 38h` al sito 5BFC
 * (14 voci). */
void sub_5995(struct z80 *z)
{
    int cy = 0;
    uint16_t dest;

    sub_5345(z);                               /* 5995 */
    IXB(0x11) = z->c;                          /* 5998 */
    IXB(0x12) = z->b;                          /* 599B */
    if (!(IXB(0x00) & 0x20))                   /* 599E */
        goto l59e5;
    IXB(0x01) &= ~0x04;                        /* 59A4 */
    {
        uint8_t ra = IXB(0x05);                /* 59A8 */
        uint8_t sb = IYB(0x1e);                /* 59AB */
        int addc = ra >> 7;
        z->a = (uint8_t)(ra << 1);             /* 59B0 add a,a */
        if (addc) {                            /* 59B1 jr nc */
            cy = z->a != 0;                    /* 59B3 neg */
            z->a = (uint8_t)-z->a;
        } else {
            cy = 0;
        }
        z->e = z->a;                           /* 59B5 */
        cy = !cy;                              /* 59B6 ccf */
        z->b = (uint8_t)((sb & 0xfe) | cy);    /* 59AE srl b, 59B7 rl b */
        cy = 0;
    }
    cy = z->a < 0x20;                          /* 59B9 */
    if (!cy)
        IXB(0x01) &= ~0x01;                    /* 59BD */
    if (!(IXB(0x01) & 0x01)) {                 /* 59C1 */
        cy = z->a < 0x20;                      /* 59C7 */
        if (!cy)
            IYB(0x1e) = z->b;                  /* 59CB */
    }
    z->a = IXB(0x04);                          /* 59CE */
    cy = z->a >> 7;                            /* 59D1 add a,a */
    z->a = (uint8_t)(z->a << 1);
    if (z->a == 0 || cy) {                     /* 59D2 jr z, 59D4 jr nc */
        cy = z->a != 0;                        /* 59D6 neg */
        z->a = (uint8_t)-z->a;
        cy = z->a < 0x20;                      /* 59D8 */
        if (!cy)
            goto l5a66;                        /* 59DA */
        goto l5b4b;                            /* 59DD-59E2 */
    }
    cy = z->a < z->e;                          /* 59DF */
    if (!cy)
        goto l5a2d;                            /* 59E0 */
    goto l5b4b;                                /* 59E2 */

l59e5:
    z->a = (uint8_t)(nemici_r_somma() & 0x0f); /* 59E5-59EB */
    cy = z->a < 0x02;                          /* 59ED */
    if (z->a == 0x02)                          /* 59EF */
        goto l5a2d;
    SET_BC(z, 0x0000);                         /* 59F1 */
    SET_HL(z, 0x3f0f);                         /* 59F4 */
    z->a = IXB(0x11) & z->h;                   /* 59F7 */
    z->e = z->a;                               /* 59FB */
    z->a = IXB(0x12) & z->h;                   /* 59FC */
    z->d = z->a;                               /* 5A00 */
    if (IYB(0x1e) & 0x01) {                    /* 5A01 */
        z->a = z->d & z->l;                    /* 5A07 */
        z->b = z->a;
        if (z->a) goto l5a2a;
        z->a = z->d;                           /* 5A0C */
        z->b = z->a;
        if (z->a) goto l5a2a;
        z->a = z->e;                           /* 5A11 */
        z->c = z->a;
        if (z->a) goto l5a2a;
        goto l5a27;                            /* 5A16 */
    }
    z->a = z->e & z->l;                        /* 5A18 */
    z->c = z->a;
    if (z->a) goto l5a2a;
    z->a = z->e;                               /* 5A1D */
    z->c = z->a;
    if (z->a) goto l5a2a;
    z->a = z->d;                               /* 5A22 */
    z->b = z->a;
    if (z->a) goto l5a2a;
l5a27:
    cy = 0;
    goto l5b4b;
l5a2a:
    cy = 0;
    goto l5bf2;

l5a2d:
    z->c = IXB(0x11);                          /* 5A2D */
    z->b = IXB(0x12);                          /* 5A30 */
    z->a = 0x0c;                               /* 5A37 */
    if (!(IYB(0x1e) & 0x01)) {                 /* 5A33 */
        z->a &= z->c;                          /* 5A44 */
        cy = 0;
        if (z->a == 0) goto l5a4c;
        z->c = z->a;
        z->b = 0x00;
        goto l5bf2;                            /* 5A4A */
    }
    z->a &= z->b;                              /* 5A3B */
    cy = 0;
    if (z->a == 0) goto l5a4c;
    z->b = z->a;                               /* 5A3E */
    z->c = 0x00;
    goto l5bf2;                                /* 5A41 */
l5a4c:
    sub_5c32(z);                               /* 5A4C */
    cy = riporto(z);
    SET_BC(z, CLASSI_C07E + 2);                /* 5A4F */
    z->a = M(BC(z));                           /* 5A52 */
    if (z->a & 0x02) {                         /* 5A53 */
        RRA(z, cy);                            /* 5A57 */
        if (!cy) goto l5add;                   /* 5A58 */
        goto l5b4b;                            /* 5A5B */
    }
    SET_BC(z, (uint16_t)(BC(z) + 1));          /* 5A5E */
    z->a = M(BC(z));                           /* 5A5F */
    RRA(z, cy);                                /* 5A60 */
    if (cy) goto l5b4b;                        /* 5A61 */
    goto l5aab;                                /* 5A64 */

l5a66:
    z->c = IXB(0x11);                          /* 5A66 */
    z->b = IXB(0x12);                          /* 5A69 */
    z->a = 0x22;                               /* 5A70 */
    if (!(IYB(0x1e) & 0x01)) {                 /* 5A6C */
        z->a &= z->c;                          /* 5A7D */
        cy = 0;
        if (z->a == 0) goto l5a85;
        z->c = z->a;
        z->b = 0x00;
        goto l5bf2;                            /* 5A83 */
    }
    z->a &= z->b;                              /* 5A74 */
    cy = 0;
    if (z->a == 0) goto l5a85;
    z->b = z->a;                               /* 5A77 */
    z->c = 0x00;
    goto l5bf2;                                /* 5A7A */
l5a85:
    sub_5c32(z);                               /* 5A85 */
    cy = riporto(z);
    SET_BC(z, CLASSI_C07E);                    /* 5A88 */
    z->a = M(BC(z));                           /* 5A8B */
    if (z->a & 0x02) {                         /* 5A8C */
        RRA(z, cy);                            /* 5A90 */
        if (cy) goto l5b4b;                    /* 5A91 */
        goto l5b09;                            /* 5A94 */
    }
    SET_BC(z, (uint16_t)(BC(z) + 1));          /* 5A96 */
    z->a = M(BC(z));                           /* 5A97 */
    if (z->a & 0x02) {                         /* 5A98 */
        RRA(z, cy);                            /* 5A9C */
        if (!cy) goto l5b2b;                   /* 5A9D */
        goto l5b4b;                            /* 5AA0 */
    }
    SET_BC(z, (uint16_t)(BC(z) + 1));          /* 5AA3 */
    z->a = M(BC(z));                           /* 5AA4 */
    RRA(z, cy);                                /* 5AA5 */
    if (cy) goto l5b4b;                        /* 5AA6 */
    goto l5add;                                /* 5AA9 */

l5aab:
    sub_5b12(z);                               /* 5AAB */
    if (!riporto(z))                           /* 5AAE ret nc */
        return;
    IYB(0x1e) |= 0x02;                         /* 5AAF */
    sub_5928(z);                               /* 5AB3 */
    IYB(0x1e) &= ~0x02;                        /* 5AB6 */
    if (z->a != IXB(0x14)) {                   /* 5ABA */
        z->a = M(CELLA_C2BB);                  /* 5ABF */
        if (z->a == z->e)                      /* 5AC2 cp e; ret z */
            return;
    }
    sub_7379(z);                               /* 5AC4 */
    if (!riporto(z))                           /* 5AC7 ret nc */
        return;
    IXB(0x0e) = z->a;                          /* 5AC8 */
    M(HL(z)) = 0x80;                           /* 5ACB */
    z->a = z->e;                               /* 5ACD */
    IXB(0x14) = z->a;                          /* 5ACE */
    M(CELLA_C2BB) = z->a;                      /* 5AD1 */
    IYB(0x1d) = 0x07;                          /* 5AD4 */
    IYB(0x1e) |= 0x02;                         /* 5AD8 */
    return;

l5add:
    sub_5b12(z);                               /* 5ADD */
    if (!riporto(z))                           /* 5AE0 jr nc */
        goto l5b04;
    sub_5928(z);                               /* 5AE2 */
    if (z->a != IXB(0x14)) {                   /* 5AE5 */
        z->a = M(CELLA_C2BB);                  /* 5AEA */
        if (z->a == z->e)                      /* 5AED cp e; ret z */
            return;
    }
    sub_7379(z);                               /* 5AEF */
    if (!riporto(z))                           /* 5AF2 jr nc */
        goto l5b04;
    M(HL(z)) = 0x80;                           /* 5AF4 */
    IXB(0x0e) = z->a;                          /* 5AF6 */
    z->a = z->e;                               /* 5AF9 */
    IXB(0x14) = z->a;                          /* 5AFA */
    M(CELLA_C2BB) = z->a;                      /* 5AFD */
    IYB(0x1d) = 0x07;                          /* 5B00 */
l5b04:
    IYB(0x1e) &= ~0x02;                        /* 5B04 */
    return;

l5b09:
    sub_5b09(z);
    return;

l5b1c:
    IYB(0x1d) = 0x04;                          /* 5B1C */
    IXB(0x0d) = 0x00;                          /* 5B20 */
    return;

l5b25:
    IYB(0x01) &= ~0x02;                        /* 5B25 */
    goto l5b2f;
l5b2b:
    IYB(0x01) |= 0x02;                         /* 5B2B */
l5b2f:
    z->a = M(PORTATO_C2BA);                    /* 5B2F */
    RRA(z, cy);                                /* 5B32 */
    if (cy)                                    /* 5B33 ret c */
        return;
    sub_7379(z);                               /* 5B34 */
    if (!riporto(z))                           /* 5B37 ret nc */
        return;
    IXB(0x0e) = z->a;                          /* 5B38 */
    M(HL(z)) = 0x80;                           /* 5B3B */
    SET_HL(z, PORTATO_C2BA);                   /* 5B3D */
    M(PORTATO_C2BA) |= 0x01;                   /* 5B40 */
    IXB(0x01) |= 0x02;                         /* 5B42 */
    IYB(0x1d) = 0x00;                          /* 5B46 */
    return;

l5b4b:
    z->c = IXB(0x11);                          /* 5B4B */
    z->b = IXB(0x12);                          /* 5B4E */
    z->a = 0x33;                               /* 5B55 */
    if (IYB(0x1e) & 0x01) {                    /* 5B51 */
        SET_DE(z, 0x0200);                     /* 5B59 */
        z->a &= z->b;                          /* 5B5C */
        cy = 0;
        if (z->a == 0) goto l5b7c;
        if (z->a & 0x01)                       /* 5B5F */
            z->a &= 0xcf;                      /* 5B63 */
        z->b = z->a;                           /* 5B65 */
        z->c = 0x00;
        goto l5bf2;                            /* 5B68 */
    }
    SET_DE(z, 0x0004);                         /* 5B6B */
    z->a &= z->c;                              /* 5B6E */
    cy = 0;
    if (z->a == 0) goto l5b7c;
    if (z->a & 0x01)                           /* 5B71 */
        z->a &= 0xcf;                          /* 5B75 */
    z->c = z->a;                               /* 5B77 */
    z->b = 0x00;
    goto l5bf2;                                /* 5B7A */

l5b7c:
    {
        uint16_t de = DE(z);                   /* 5B7C push de */
        sub_5c32(z);                           /* 5B7D */
        SET_DE(z, de);                         /* 5B80 pop de */
    }
    cy = riporto(z);
    SET_BC(z, CLASSI_C07E + 3);                /* 5B81 */
    z->a = M(BC(z));                           /* 5B84 */
    if (z->a & 0x02) {                         /* 5B85 */
        SET_BC(z, (uint16_t)(BC(z) - 1));      /* 5B95 */
        z->a = M(BC(z));                       /* 5B96 */
        if (!(z->a & 0x02))                    /* 5B97 ret z */
            return;
        RRA(z, cy);                            /* 5B9A */
        if (cy) goto l5ba0;                    /* 5B9B */
        goto l5add;                            /* 5B9D */
    }
    RRA(z, cy);                                /* 5B89 */
    if (cy) goto l5bbe;                        /* 5B8A */
    SET_BC(z, (uint16_t)(BC(z) - 1));          /* 5B8C */
    z->a = M(BC(z));                           /* 5B8D */
    cy = z->a < 0x03;                          /* 5B8E */
    if (z->a == 0x03) goto l5ba0;              /* 5B90 */
    goto l5aab;                                /* 5B92 */

l5ba0:
    SET_BC(z, (uint16_t)(BC(z) - 2));          /* 5BA0 */
    z->a = M(BC(z));                           /* 5BA2 */
    if (z->a & 0x02) {                         /* 5BA3 */
        RRA(z, cy);                            /* 5BA7 */
        if (cy) goto l5b1c;                    /* 5BA8 */
        SET_BC(z, (uint16_t)(BC(z) + 1));      /* 5BAB */
        z->a = M(BC(z));                       /* 5BAC */
        cy = z->a < 0x03;                      /* 5BAD */
        if (z->a == 0x03) goto l5b1c;          /* 5BAF */
        goto l5b09;                            /* 5BB2 */
    }
    SET_BC(z, (uint16_t)(BC(z) + 1));          /* 5BB5 */
    z->a = M(BC(z));                           /* 5BB6 */
    RRA(z, cy);                                /* 5BB7 */
    if (cy) goto l5b1c;                        /* 5BB8 */
    goto l5b25;                                /* 5BBB */

l5bbe:
    SET_BC(z, CLASSI_C07E);                    /* 5BBE */
    z->a = M(BC(z));                           /* 5BC1 */
    cy = z->a < 0x03;                          /* 5BC2 */
    if (z->a == 0x03) goto l5be4;              /* 5BC4 */
    SET_BC(z, (uint16_t)(BC(z) + 1));          /* 5BC6 */
    z->a = M(BC(z));                           /* 5BC7 */
    SET_BC(z, (uint16_t)(BC(z) - 1));          /* 5BC8 */
    cy = z->a < 0x03;                          /* 5BC9 */
    if (z->a == 0x03) goto l5be4;              /* 5BCB */
    z->a = M(BC(z));                           /* 5BCD */
    if (z->a & 0x02) goto l5b09;               /* 5BCE-5BD0 */
    SET_BC(z, (uint16_t)(BC(z) + 1));          /* 5BD3 */
    z->a = M(BC(z));                           /* 5BD4 */
    if (z->a & 0x02) goto l5b2b;               /* 5BD5-5BD7 */
    SET_BC(z, (uint16_t)(BC(z) + 1));          /* 5BDA */
    z->a = M(BC(z));                           /* 5BDB */
    RRA(z, cy);                                /* 5BDC */
    if (!cy) goto l5add;                       /* 5BDD */
l5be0:
    z->c = z->e;                               /* 5BE0 */
    z->b = z->d;                               /* 5BE1 */
    goto l5bf2;                                /* 5BE2 */

l5be4:
    SET_BC(z, (uint16_t)(BC(z) + 2));          /* 5BE4 */
    z->a = M(BC(z));                           /* 5BE6 */
    if (!(z->a & 0x02)) goto l5be0;            /* 5BE7 */
    RRA(z, cy);                                /* 5BEB */
    if (!cy) goto l5add;                       /* 5BEC */
    goto l5b1c;                                /* 5BEF */

l5bf2:
    /* rl c; srl b; rr c: C = (C & 7f) | (B << 7), B >>= 1 */
    z->c = (uint8_t)((z->c & 0x7f) | ((z->b & 0x01) << 7));
    z->b >>= 1;
    sub_51d7(z);                               /* 5BF8 */
    z->e = z->a;                               /* 5BFB */
    dest = nemici_rst38(z, 0x5bfc, rst38_5BFC,
                        sizeof rst38_5BFC / sizeof rst38_5BFC[0]);
    NEMICI_PONI_RIPORTO(z, 0);                 /* rst 38: add hl,de spegne il riporto */
    switch (dest) {
    case 0x5c19: sub_5c19(z); break;
    case 0x5c23: sub_5c23(z); break;
    case 0x3b62: sub_3b62(z); break;
    case 0x5c1e: sub_5c1e(z); break;
    case 0x5c29: sub_5c29(z); break;
    case 0x5b09: sub_5b09(z); break;
    default: break;
    }
}
