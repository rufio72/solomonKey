/* Motore dei nemici, lotto N1 (brief 25): gestori per tipo 10 (714B) e
 * 12 (55B2) chiamati da 4F0F, con le fasi smistate da (IY+1D): per 55B2
 * 5693 (presa di un blocco), 561B (inversione), 55C9 e 5640 (cammino);
 * per 714B 7155 (partenza dalla direzione di (IY+4)) e 7235 (cammino col
 * controllo della cella davanti). Ingressi: 55B2, 55C9, 561B, 5640, 5693,
 * 714B, 7155, 7235. Dal lotto N5 (brief 31) anche 55C4 e 6C1D (tipo 12)
 * e 719E (tipo 10); il resto del lotto (tipi 13 e 11: 54D8, 56E9 e le
 * loro fasi) sta in nemici_tipi_10_13b.c. Anticipate dal lotto N2
 * (brief 27) per il tipo 14: 571D, 57AA, 57B3.
 *
 * IX = slot (c2c1 + i·0x18), IY = oggetto (c4fe + i·0x30). Tradotto da
 * out/main64.dasm; tabelle da rst38_tables.h. */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"

#define CLASSI_C080    0xc080   /* classi delle celle davanti (sub_5c32: c07e-c081) */
#define CLASSI_C081    0xc081
#define OGGETTI_488D   0x488d   /* ROM: indirizzo per indice (IX+0E) */
#define DIREZIONI_7196 0x7196   /* ROM: quattro velocità a 16 bit */
#define PORTATI_48AF   0x48af   /* ROM: indirizzo dell'oggetto per indice */

/* ------------------------------------------------------------------ */
/* 0x55B2: tipo 12, smista con `rst 38h` a 55B5 sulla fase (IY+1D). */
void sub_55b2(struct z80 *z)
{
    z->e = M(z->iy + 0x1d);                    /* 55B2 */
    uint16_t dest = nemici_rst38(z, 0x55B5, rst38_55B5,
                                 sizeof rst38_55B5 / sizeof rst38_55B5[0]);
    switch (dest) {                            /* 55B5 rst 38 */
    case 0x5693: sub_5693(z); break;
    case 0x6C1D: sub_6c1d(z); break;
    case 0x561B: sub_561b(z); break;
    case 0x55C4: sub_55c4(z); break;
    case 0x55C9: sub_55c9(z); break;
    case 0x5640: sub_5640(z); break;
    case 0x3B62: sub_3b62(z); break;
    case 0:      break;                        /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}

/* 0x55C9, fase 4: attende (IX+0D) >= 1a, poi cammina con (IY+12) = 2
 * finché (IY+12) non si azzera (fase 0). Con le classi davanti (sub_5c32)
 * libere e il bit di direzione di (IY+1A) spento, accende il bit 4 di
 * (IX+1) e passa alla fase 2. */
void sub_55c9(struct z80 *z)
{
    if (!(M(z->iy + 0x1e) & 0x02)) {           /* 55C9-55CD */
        z->a = M(z->ix + 0x0d);                /* 55CF */
        if (z->a < 0x1a)                       /* 55D2-55D4 */
            return;
        M(z->iy + 0x1e) |= 0x02;               /* 55D5 */
        M(z->iy + 0x12) = 0x02;                /* 55D9 */
    }
    z->a = M(z->iy + 0x12);                    /* 55DD */
    if (z->a == 0) {                           /* 55E0-55E1 */
        M(z->iy + 0x1e) &= ~0x02;              /* 560F */
        z->a = 0;                              /* 5613 */
        M(z->iy + 0x1d) = z->a;                /* 5614 */
        M(z->ix + 0x0d) = z->a;                /* 5617 */
        return;
    }
    sub_5c32(z);                               /* 55E3 */
    SET_HL(z, CLASSI_C081);                    /* 55E6 */
    z->a = (M(z->iy + 0x1e) & 0x01) ? 0x20 : 0x10;  /* 55E9-55F1 */
    z->a &= M(z->iy + 0x1a);                   /* 55F3 */
    if (z->a)                                  /* 55F6 ret nz */
        return;
    if (M(CLASSI_C081) & 0x02)                 /* 55F7-55F9 */
        return;
    SET_HL(z, CLASSI_C080);                    /* 55FA */
    if (M(CLASSI_C080) & 0x02)                 /* 55FB-55FD */
        return;
    M(z->ix + 0x01) |= 0x10;                   /* 55FE */
    M(z->iy + 0x1e) &= ~0x02;                  /* 5602 */
    M(z->ix + 0x0d) = 0x00;                    /* 5606 */
    M(z->iy + 0x1d) = 0x02;                    /* 560A */
}

/* 0x561B, fase 2: se (IX+1) ha il bit 4 attende (IX+0D) >= 18, lo spegne
 * e inverte la direzione (bit 0 di (IY+1E)); poi bit 1 di (IY+1E),
 * (IY+12) = 2 e fase 5. */
void sub_561b(struct z80 *z)
{
    z->a = M(z->iy + 0x1e);                    /* 561F */
    if (M(z->ix + 0x01) & 0x10) {              /* 561B, 5622 */
        z->c = z->a;                           /* 5624 */
        z->a = M(z->ix + 0x0d);                /* 5625 */
        if (z->a < 0x18)                       /* 5628-562A */
            return;
        z->a = z->c;                           /* 562B */
        M(z->ix + 0x01) &= ~0x10;              /* 562C */
        z->a ^= 0x01;                          /* 5630 */
    }
    z->a |= 0x02;                              /* 5632 */
    M(z->iy + 0x1e) = z->a;                    /* 5634 */
    M(z->iy + 0x12) = 0x02;                    /* 5637 */
    M(z->iy + 0x1d) = 0x05;                    /* 563B */
}

/* 0x5640, fase 5: cammino. (IY+12) nullo: fase 0. Vicino a Dana
 * (sub_5507, C = 8, riporto): fase 4. Altrimenti, come 55C9, con le
 * classi davanti libere passa alla fase 2; con c080 occupata, fase 0. */
void sub_5640(struct z80 *z)
{
    z->a = M(z->iy + 0x12);                    /* 5640 */
    if (z->a) {                                /* 5643-5644 */
        z->c = 0x08;                           /* 5646 */
        sub_5507(z);                           /* 5648 */
        if (NEMICI_RIPORTO_ACCESO(z)) {        /* 564B jr nc */
            M(z->iy + 0x1e) &= ~0x02;          /* 564D */
            M(z->iy + 0x1d) = 0x04;            /* 5651 */
            M(z->ix + 0x0d) = 0x00;            /* 5655 */
            return;
        }
        sub_5c32(z);                           /* 565A */
        SET_HL(z, CLASSI_C081);                /* 565D */
        z->a = (M(z->iy + 0x1e) & 0x01) ? 0x20 : 0x10;  /* 5660-5668 */
        z->a &= M(z->iy + 0x1a);               /* 566A */
        if (z->a)                              /* 566D ret nz */
            return;
        if (M(CLASSI_C081) & 0x02)             /* 566E-5670 */
            return;
        SET_HL(z, CLASSI_C080);                /* 5671 */
        if (!(M(CLASSI_C080) & 0x02)) {        /* 5672-5674 */
            M(z->ix + 0x01) |= 0x10;           /* 5676 */
            M(z->iy + 0x1e) &= ~0x02;          /* 567A */
            M(z->iy + 0x1d) = 0x02;            /* 567E */
            M(z->ix + 0x0d) = 0x00;            /* 5682 */
            return;
        }
    }
    M(z->iy + 0x1e) &= ~0x02;                  /* 5687 */
    z->a = 0;                                  /* 568B */
    M(z->iy + 0x1d) = z->a;                    /* 568C */
    M(z->ix + 0x0d) = z->a;                    /* 568F */
}

/* Scrittura a un indirizzo del Z80: sotto c000 è ROM e non cambia. */
static void scrivi_ram(uint16_t a, uint8_t v)
{
    if (a >= 0xc000)                           /* scritture in ROM: nessun effetto */
        M(a) = v;
}

/* 0x5693, fase 0: dopo 20 passi ((IX+0D) >= 20) guarda la cella davanti
 * (sub_5928). Senza blocco (bit 7 spento): fase 2. Con un blocco prende
 * un oggetto libero (sub_7379, (HL) = 80, indice in (IX+0E)) e lo porta
 * sul blocco (sub_58cf). Col bit 3 di (IX+1) lo chiude invece (sub_5917).
 * In chiusura: bit 4 di (IX+1) se c'è ancora un blocco, spegne il bit 3,
 * bit 5 di (IY+0), fase 2 e azzera il byte puntato dalla tabella 488D. */
void sub_5693(struct z80 *z)
{
    if (!(M(z->ix + 0x01) & 0x08)) {           /* 5693-5697 */
        z->a = M(z->ix + 0x0d);                /* 5699 */
        if (z->a < 0x20)                       /* 569C-569E */
            return;
        sub_5928(z);                           /* 569F */
        if (!(M(HL(z)) & 0x80)) {              /* 56A2-56A4 */
            M(z->iy + 0x1d) = 0x02;            /* 56A6 */
            return;
        }
        sub_7379(z);                           /* 56AB */
        if (!NEMICI_RIPORTO_ACCESO(z))         /* 56AE ret nc */
            return;
        M(HL(z)) = 0x80;                       /* 56AF */
        M(z->ix + 0x0e) = z->a;                /* 56B1 */
        sub_58cf(z);                           /* 56B4 */
        M(z->ix + 0x0d) = 0x00;                /* 56B7 */
        if (!NEMICI_RIPORTO_ACCESO(z))         /* 56BB jr c,56c7 */
            return;
    } else {
        z->a = M(z->ix + 0x0d);                /* 56BE */
        if (z->a < 0x20)                       /* 56C1-56C3 */
            return;
        sub_5917(z);                           /* 56C4 */
    }
    sub_5928(z);                               /* 56C7 */
    if (M(HL(z)) & 0x80)                       /* 56CA-56CC */
        M(z->ix + 0x01) |= 0x10;               /* 56CE */
    M(z->ix + 0x01) &= ~0x08;                  /* 56D2 */
    M(z->iy) |= 0x20;                          /* 56D6 */
    M(z->iy + 0x1d) = 0x02;                    /* 56DA */
    z->a = M(z->ix + 0x0e);                    /* 56DE */
    SET_HL(z, OGGETTI_488D);                   /* 56E1 */
    sub_0008(z);                               /* 56E4 rst 8 */
    sub_0010(z);                               /* 56E5 rst 10 */
    z->a = 0;                                  /* 56E6 */
    scrivi_ram(DE(z), z->a);                   /* 56E7 */
}

/* ------------------------------------------------------------------ */
/* 0x714B: tipo 10, smista con `rst 38h` a 714E sulla fase (IY+1D). */
void sub_714b(struct z80 *z)
{
    z->e = M(z->iy + 0x1d);                    /* 714B */
    uint16_t dest = nemici_rst38(z, 0x714E, rst38_714E,
                                 sizeof rst38_714E / sizeof rst38_714E[0]);
    switch (dest) {                            /* 714E rst 38 */
    case 0x7155: sub_7155(z); break;
    case 0x719E: sub_719e(z); break;
    case 0x7235: sub_7235(z); break;
    case 0:      break;                        /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}

/* 0x7155, fase 0: fase 2; velocità dalla tabella 7196 sui bit 0-1 di
 * (IY+4), negata se il bit 2 è acceso; bit 3 = asse: spento (IY+12/13),
 * acceso (IY+0B/0C). (IY+1E) = direzione dai bit 2-3, col bit 0 di
 * (IY+1E) conservato sull'asse verticale. */
void sub_7155(struct z80 *z)
{
    z->a = M(z->iy + 0x04);                    /* 7155 */
    M(z->iy + 0x1d) = 0x02;                    /* 7158 */
    z->b = z->a;                               /* 715C */
    z->a &= 0x03;                              /* 715D */
    SET_HL(z, DIREZIONI_7196);                 /* 715F */
    sub_0008(z);                               /* 7162 rst 8 */
    sub_0010(z);                               /* 7163 rst 10 */
    z->a = (uint8_t)((z->b & 0x0c) >> 2);      /* 7164-7168 */
    z->b = z->a;                               /* 7169 */
    if (z->a & 0x01) {                         /* 716A-716B rrca; jr nc */
        uint16_t de = DE(z);
        SET_HL(z, de);                         /* 716D-7173: DE = -DE, HL = DE */
        SET_DE(z, (uint16_t)(0 - de));
    }
    z->a = z->b;                               /* 7174 */
    if (z->a >= 0x02)                          /* 7175-7177 */
        z->a = (uint8_t)(((z->a & 0x01) << 1) | (M(z->iy + 0x1e) & 0x01));  /* 7179-7180 */
    M(z->iy + 0x1e) = z->a;                    /* 7181 */
    if (z->b & 0x02) {                         /* 7184-7186 */
        M(z->iy + 0x0b) = z->e;                /* 718F */
        M(z->iy + 0x0c) = z->d;                /* 7192 */
    } else {
        M(z->iy + 0x12) = z->e;                /* 7188 */
        M(z->iy + 0x13) = z->d;                /* 718B */
    }
}

/* 0x7235, fase 2: col bit 3 di (IY+4) il bit 0 di (IY+1E) diventa il bit
 * 7 negato di (IX+5). Con (IY+1A) & 0f nullo spegne il bit 1 di (IX+1);
 * altrimenti, se quel bit è spento, lo accende, va in fase 1 e azzera
 * (IX+8); col bit 4 di (IY+4) guarda prima la cella davanti (sub_734d,
 * BC = 0e01): se vale >= f8 inverte il bit 2 di (IY+4) e (IY+1B) e va in
 * fase 0. */
void sub_7235(struct z80 *z)
{
    if (M(z->iy + 0x04) & 0x08) {              /* 7235-7239 */
        z->c = M(z->iy + 0x1e);                /* 723B */
        z->a = M(z->ix + 0x05);                /* 7240 */
        z->a = (uint8_t)((z->a << 1) | (z->a >> 7));   /* 7243 rlca */
        z->c = (uint8_t)((z->c & 0xfe) | ((z->a & 0x01) ^ 0x01));  /* 723E rr c, 7244-7245 */
        M(z->iy + 0x1e) = z->c;                /* 7247 */
    }
    z->a = M(z->iy + 0x1a) & 0x0f;             /* 724A-724D */
    if (z->a == 0) {                           /* 724F */
        M(z->ix + 0x01) &= ~0x02;              /* 7251 */
        return;
    }
    if (M(z->ix + 0x01) & 0x02)                /* 7256-725A ret nz */
        return;
    z->b = 0x01;                               /* 725F */
    if (M(z->iy + 0x04) & 0x10) {              /* 725B, 7261 */
        SET_BC(z, 0x0e01);                     /* 7263 */
        sub_734d(z);                           /* 7266 */
        z->a = M(HL(z));                       /* 7269 */
        z->b = 0x01;                           /* 726C */
        if (z->a >= 0xf8) {                    /* 726A, 726E */
            z->b = 0x00;                       /* 7270 */
            z->a = M(z->iy + 0x04) ^ 0x04;     /* 7271-7274 */
            M(z->iy + 0x04) = z->a;            /* 7276 */
            M(z->iy + 0x1b) = z->a;            /* 7279 */
        }
    }
    M(z->ix + 0x01) |= 0x02;                   /* 727C */
    M(z->iy + 0x1d) = z->b;                    /* 7280 */
    M(z->ix + 0x08) = 0x00;                    /* 7283 */
}

/* ------------------------------------------------------------------ */
/* Lotto N5, anticipate dal lotto N2 (brief 27): usate dal tipo 14 (6CE8). */

/* 0x571D (6D03[0], fase 0): a (IX+0D) >= 40 chiude slot e oggetto (80A5). */
void sub_571d(struct z80 *z)
{
    z->a = M(z->ix + 0x0d);                    /* 571D */
    if (z->a < 0x40)                           /* 5720-5722 ret c */
        return;
    sub_80a5(z);                               /* 5723 jp */
}

/* 0x57AA: HL = (IX+0B..0C) - BC - riporto, col riporto del prestito. */
void sub_57aa(struct z80 *z)
{
    unsigned parola = M(z->ix + 0x0b) | (M(z->ix + 0x0c) << 8);  /* 57AA-57AD */
    unsigned sottratto = BC(z) + (NEMICI_RIPORTO_ACCESO(z) ? 1u : 0u);
    SET_HL(z, (uint16_t)(parola - sottratto)); /* 57B0 sbc hl,bc */
    NEMICI_PONI_RIPORTO(z, parola < sottratto);
}

/* 0x57B3: senza il bit 3 di (IX+1): col bit 0 di A accende il bit 0 di
 * (IY+0); poi fase 0 e azzera (IX+0D) e (IX+0B..0C). A = 0. */
void sub_57b3(struct z80 *z)
{
    if (M(z->ix + 0x01) & 0x08)                /* 57B3-57B7 ret nz */
        return;
    int basso = z->a & 0x01;                   /* 57B8 rrca */
    z->a = (uint8_t)((z->a >> 1) | (basso << 7));
    if (basso)                                 /* 57B9 jr nc */
        M(z->iy) |= 0x01;                      /* 57BB */
    z->a = 0;                                  /* 57BF */
    M(z->iy + 0x1d) = z->a;                    /* 57C0 */
    M(z->ix + 0x0d) = z->a;                    /* 57C3 */
    M(z->ix + 0x0b) = z->a;                    /* 57C6 */
    M(z->ix + 0x0c) = z->a;                    /* 57C9 */
}

/* ------------------------------------------------------------------ */
/* Lotto N5 (brief 31): tipo 12 (55C4, 6C1D) e tipo 10 (719E). */

/* 0x55C4, fase 3 di 55B5: passa alla fase 2. */
void sub_55c4(struct z80 *z)
{
    M(z->iy + 0x1d) = 0x02;                    /* 55C4 */
}

/* 0x6C1D, fase 1 di 55B5: 6C2C con A = 3; senza riporto esce; con
 * (IX+0D) >= 0d chiude slot e oggetto (80A5). */
void sub_6c1d(struct z80 *z)
{
    z->a = 0x03;                               /* 6C1D */
    sub_6c2c(z);                               /* 6C1F */
    if (!NEMICI_RIPORTO_ACCESO(z))             /* 6C22 ret nc */
        return;
    z->a = M(z->ix + 0x0d);                    /* 6C23 */
    if (z->a < 0x0d)                           /* 6C26-6C28 ret c */
        return;
    sub_80a5(z);                               /* 6C29 jp */
}

/* 0x719E, fase 1 di 714E. Col bit 0 di (IX+1) spento attende (IX+8) >= 0b
 * e lo accende. Con il bit 0 acceso e uno dei bit 2-3 acceso attende
 * (IX+8) >= 16, chiama 6EB7 con (IX+0E) se il bit 2 è spento, spegne i
 * bit 2-3 e (con E = (IY+4) ^ 4) riaggancia l'oggetto con 4AF6 (71D4,
 * con IX = IY). Senza bit 2-3: se (IY+1A) & 0f è nullo riaggancia con
 * E = (IY+4); altrimenti guarda la cella davanti (734D): >= f8 accende il
 * bit 2, altrimenti prende uno slot (7379), accende il bit 3, crea
 * l'oggetto portato con 4938 e pone (c835) = ff. */
void sub_719e(struct z80 *z)
{
    z->a = M(z->ix + 0x01);                    /* 719E */
    if (!(z->a & 0x01)) {                      /* 71A1-71A3 */
        z->a = M(z->ix + 0x08);                /* 71A5 */
        if (z->a < 0x0b)                       /* 71A8-71AA ret c */
            return;
        M(z->ix + 0x01) |= 0x01;               /* 71AB */
        return;
    }
    z->a &= 0x0c;                              /* 71B0 */
    if (z->a != 0) {                           /* 71B2 jr z 71E9 */
        z->a = M(z->ix + 0x08);                /* 71B4 */
        if (z->a < 0x16)                       /* 71B7-71B9 ret c */
            return;
        if (!(M(z->ix + 0x01) & 0x04)) {       /* 71BA-71BE */
            z->a = M(z->ix + 0x0e);            /* 71C0 */
            sub_6eb7(z);                       /* 71C3 */
        }
        M(z->ix + 0x01) &= ~0x04;              /* 71C6 */
        M(z->ix + 0x01) &= ~0x08;              /* 71CA */
        z->a = (uint8_t)(M(z->iy + 0x04) ^ 0x04);  /* 71CE-71D1 */
        z->e = z->a;                           /* 71D3 */
        goto l71d4;
    }
    z->a = M(z->iy + 0x1a) & 0x0f;             /* 71E9-71EC */
    if (z->a == 0) {                           /* 71EE jr nz */
        z->e = M(z->iy + 0x04);                /* 71F0 */
        goto l71d4;
    }
    M(z->ix + 0x08) = 0x00;                    /* 71F5 */
    z->b = 0x0e;                               /* 71F9 */
    z->c = 0x01;
    sub_734d(z);                               /* 71FC */
    z->a = leggi(HL(z));                       /* 71FF */
    if (z->a >= 0xf8) {                        /* 7200-7202 */
        M(z->ix + 0x01) |= 0x04;               /* 7204 */
        return;
    }
    z->d = z->a;                               /* 7209 */
    sub_7379(z);                               /* 720A */
    if (!NEMICI_RIPORTO_ACCESO(z))             /* 720D ret nc */
        return;
    M(z->ix + 0x01) |= 0x08;                   /* 720E */
    M(z->ix + 0x0e) = z->a;                    /* 7212 */
    z->c = z->e;                               /* 7215 */
    z->b = z->d;                               /* 7216 */
    M(HL(z)) = 0x80;                           /* 7217 */
    M(0xc08a) = 0x00;                          /* 7219-721C */
    SET_HL(z, PORTATI_48AF);                   /* 721E */
    sub_0008(z);                               /* 7221 rst 8 */
    sub_0010(z);                               /* 7222 rst 10 */
    uint16_t ix0 = z->ix;
    z->ix = DE(z);                             /* 7223-7224 push de; ex (sp),ix */
    z->a = z->b;                               /* 7226 */
    z->e = z->c;                               /* 7227 */
    z->b = 0x00;                               /* 7228 */
    sub_4938(z);                               /* 722A */
    z->ix = ix0;                               /* 722D pop ix */
    z->a = 0xff;                               /* 722F */
    M(0xc835) = z->a;                          /* 7231 */
    return;
l71d4:
    M(z->ix + 0x01) &= ~0x01;                  /* 71D4 */
    {
        uint16_t ix1 = z->ix;
        z->ix = z->iy;                         /* 71D8-71DA push iy; ex (sp),ix */
        z->a = 0xc0;                           /* 71DC */
        z->b = 0x00;                           /* 71DE */
        z->c = M(z->ix + 0x1e);                /* 71E0 */
        sub_4af6(z);                           /* 71E3 */
        z->ix = ix1;                           /* 71E6 pop ix */
    }
}
