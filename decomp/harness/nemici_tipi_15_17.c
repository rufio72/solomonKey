/* Motore dei nemici, lotto N1 (brief 25): gestore del tipo 17 (7F68),
 * con le due tabelle di fasi (7F39 per (IY+4) = a1, 7F72 per gli altri),
 * e la fase 4 della tabella 7F39 (7F51). Ingressi: 7F51, 7F68.
 * Lotto N9 (brief 35): 6B43, 6B5F, 6B8F, 6B9B, 6C0B, 6C5F, 6C8E, 7F48,
 * 7F81, 7F8A, 7FA6, 7FCF, 801D, 8042, 808D, 809C; tipo 15 = 6C0B, tipo
 * 16 = 6B43, tipo 17 = 7F68.
 *
 * 7F39 non appartiene a sub_7f2d (z80_7xxx.c), che finisce col `ret` a
 * 7F38: ci si arriva solo dal `jr z` di 7F68, e lo smistamento sta qui.
 * IX = slot, IY = oggetto. Tradotto da out/main64.dasm; tabelle da
 * rst38_tables.h. */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"

#define IXB(o) M(z->ix + (o))
#define IYB(o) M(z->iy + (o))
#define CLASSI_C080    0xc080   /* classe della cella davanti (sub_5c32) */

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

/* ---------------- Lotto N9 (brief 35) ---------------- */

/* Destinazioni della tabella 6B58. */
static void salta_6b43(struct z80 *z, uint16_t dest)
{
    switch (dest) {
    case 0x6B5F: sub_6b5f(z); break;
    case 0x6B8F: sub_6b8f(z); break;
    case 0x6B9B: sub_6b9b(z); break;
    case 0:      break;                        /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}

/* Destinazioni della tabella 6C0E. */
static void salta_6c0b(struct z80 *z, uint16_t dest)
{
    switch (dest) {
    case 0x3B62: sub_3b62(z); break;
    case 0x6C1D: sub_6c1d(z); break;
    case 0x6D37: sub_6d37(z); break;
    case 0x6C5F: sub_6c5f(z); break;
    case 0x6DCE: sub_6dce(z); break;
    case 0x6C8E: sub_6c8e(z); break;
    case 0x6EA1: sub_6ea1(z); break;
    case 0:      break;                        /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}

/* 0x6B43, tipo 16: A = ((IY+4) & 0C) >> 2 (C), B = (IX+4) se il bit 1 di A
 * e acceso, altrimenti (IX+5); fase (IY+1D) con `rst 38h` a 6B58. */
void sub_6b43(struct z80 *z)
{
    z->a = (uint8_t)((IYB(0x04) & 0x0c) >> 2); /* 6B43-6B49 */
    z->c = z->a;                               /* 6B4A */
    z->b = (z->a & 0x02) ? IXB(0x04) : IXB(0x05);   /* 6B4B-6B52 */
    z->e = IYB(0x1d);                          /* 6B55 */
    salta_6b43(z, nemici_rst38(z, 0x6B58, rst38_6B58,
                               sizeof rst38_6B58 / sizeof rst38_6B58[0]));
}

/* 0x6B5F, fase 0: la prima volta accende il bit 0 di (IX+1) e chiama 6BCD
 * con B = 0E (0F se il bit 4 di (IY+4)); poi, a (IX+0D) >= 23, suono 2D,
 * spegne il bit e fase 2. */
void sub_6b5f(struct z80 *z)
{
    if (!(IXB(0x01) & 0x01)) {                 /* 6B5F-6B63 */
        IXB(0x01) |= 0x01;                     /* 6B65 */
        z->b = 0x0e;                           /* 6B69 */
        if (IYB(0x04) & 0x10)                  /* 6B6B-6B6F */
            z->b++;                            /* 6B71 */
        z->a = IXB(0x0f);                      /* 6B72 */
        sub_6bcd(z);                           /* 6B75 */
        IXB(0x0d) = 0x00;                      /* 6B78 */
        return;
    }
    z->a = IXB(0x0d);                          /* 6B7D */
    if (z->a < 0x23)                           /* 6B80-6B82 ret c */
        return;
    z->c = 0x2d;                               /* 6B83 */
    sub_0028(z);                               /* 6B85 rst 28 */
    IXB(0x01) &= (uint8_t)~0x01;               /* 6B86 */
    IYB(0x1d) = 0x02;                          /* 6B8A */
}

/* 0x6B8F, fase 1: a (IX+8) >= 2E passa alla fase 2. */
void sub_6b8f(struct z80 *z)
{
    z->a = IXB(0x08);                          /* 6B8F */
    if (z->a < 0x2e)                           /* 6B92-6B94 */
        return;
    IYB(0x1d) = 0x02;                          /* 6B95 */
}

/* 0x6B9B, fase 2: (IY+1E) = direzione dal bit 7 di B (opposta); sceglie la
 * parola di 6BC5 (0080, 00C0, 0100, 0140) con (IY+4) & 3 e prova 57AA e
 * 7379; se passano, scrive 80 e 00 a HL, (IX+0F) = A e chiama 57B3. */
void sub_6b9b(struct z80 *z)
{
    z->a = (uint8_t)(~z->b);                   /* 6B9B-6B9C */
    z->a = (uint8_t)((z->a << 1) | (z->a >> 7));   /* 6B9D rlca */
    z->a &= 0x01;                              /* 6B9E */
    IYB(0x1e) = z->a;                          /* 6BA0 */
    z->a = IYB(0x04) & 0x03;                   /* 6BA3 */
    SET_HL(z, 0x6BC5);                         /* 6BA8 */
    sub_0008(z);                               /* 6BAB rst 8 */
    sub_0010(z);                               /* 6BAC rst 10 */
    z->c = z->e;                               /* 6BAD */
    z->b = z->d;                               /* 6BAE */
    NEMICI_PONI_RIPORTO(z, 0);                 /* riporto entrante di 57AA */
    sub_57aa(z);                               /* 6BAF */
    if (NEMICI_RIPORTO_ACCESO(z))              /* 6BB2 ret c */
        return;
    sub_7379(z);                               /* 6BB3 */
    if (!NEMICI_RIPORTO_ACCESO(z))             /* 6BB6 ret nc */
        return;
    M(HL(z)) = 0x80;                           /* 6BB7 */
    SET_HL(z, HL(z) + 1);                      /* 6BB9 */
    M(HL(z)) = 0x00;                           /* 6BBA */
    IXB(0x0f) = z->a;                          /* 6BBC */
    z->a = 0x00;                               /* 6BBF */
    sub_57b3(z);                               /* 6BC1 */
}

/* 0x6C0B, tipo 15: fase (IY+1D) con `rst 38h` a 6C0E. */
void sub_6c0b(struct z80 *z)
{
    z->e = IYB(0x1d);                          /* 6C0B */
    salta_6c0b(z, nemici_rst38(z, 0x6C0E, rst38_6C0E,
                               sizeof rst38_6C0E / sizeof rst38_6C0E[0]));
}

/* 0x6C5F, fase 3: col bit 1 di (IY+1E) attende (IX+0D) >= 6; se il bit 4 di
 * (IX+1) e acceso lo spegne e inverte il bit 0 di (IY+1E); altrimenti (bit 1
 * spento) (IY+12) = 5, fase 5 e spegne il bit 1. */
void sub_6c5f(struct z80 *z)
{
    if (IYB(0x1e) & 0x02) {                    /* 6C5F-6C63 */
        z->a = IXB(0x0d);                      /* 6C65 */
        if (z->a < 0x06)                       /* 6C68-6C6A ret c */
            return;
        if (IXB(0x01) & 0x10) {                /* 6C6B-6C6F */
            IXB(0x01) &= (uint8_t)~0x10;       /* 6C71 */
            z->a = IYB(0x1e);                  /* 6C75 */
            /* rra (riporto entrante 0); ccf; rla: bit 0 invertito */
            uint8_t c = z->a & 0x01;
            z->a = (uint8_t)(z->a >> 1);       /* 6C78 */
            c = !c;                            /* 6C79 */
            z->a = (uint8_t)((z->a << 1) | c); /* 6C7A */
        }
        z->a &= 0x01;                          /* 6C7B */
        IYB(0x1e) = z->a;                      /* 6C7D */
        return;
    }
    IYB(0x12) = 0x05;                          /* 6C81 */
    IYB(0x1d) = 0x05;                          /* 6C85 */
    IYB(0x1e) &= (uint8_t)~0x02;               /* 6C89 */
}

/* 0x6C8E, fase 5: come 6E55 se Dana e vicina (6E90 con HL = 0816); altrimenti
 * guarda la cella davanti (5C32, C080/C081) secondo la direzione. */
void sub_6c8e(struct z80 *z)
{
    SET_HL(z, 0x0816);                         /* 6C8E */
    sub_6e90(z);                               /* 6C91 */
    if (NEMICI_RIPORTO_ACCESO(z)) {            /* 6C94 jp c,6e83 */
        z->a = (uint8_t)((z->c << 1) | (z->c >> 7));   /* 6E83 rlca */
        z->a = (uint8_t)(~z->a & 0x01);        /* 6E85-6E86 */
        IYB(0x1e) = z->a;                      /* 6E88 */
        IYB(0x1d) = 0x02;                      /* 6E8B */
        return;
    }
    sub_5c32(z);                               /* 6C97 */
    z->a = IYB(0x1e);                          /* 6C9A */
    if (z->a & 0x02) {                         /* 6C9D-6C9F jr nz,6cd9 */
        z->a = IXB(0x0d);                      /* 6CD9 */
        if (z->a < 0x30)                       /* 6CDC-6CDE ret c */
            return;
        IYB(0x1d) = 0x03;                      /* 6CDF */
        IXB(0x0d) = 0x00;                      /* 6CE3 */
        return;
    }
    z->a = (z->a & 0x01) ? 0x20 : 0x10;       /* 6CA1-6CA6 rrca */
    z->a &= IYB(0x1a);                         /* 6CA8 */
    int zero;                                  /* Z con cui si arriva a 6CC9 */
    SET_HL(z, CLASSI_C080 + 1);                /* 6CAB */
    if (z->a == 0) {                           /* 6CAE jr nz non preso */
        SET_HL(z, CLASSI_C080);                /* 6CB0 */
        z->a = M(CLASSI_C080);                 /* 6CB1 */
        zero = (z->a == 0x02);                 /* 6CB2 */
    } else {
        z->a = IYB(0x12) | IYB(0x13);          /* 6CB6-6CB9 */
        if (z->a)                              /* 6CBC ret nz */
            return;
        SET_HL(z, CLASSI_C080);                /* 6CBD */
        z->a = M(CLASSI_C080);                 /* 6CBE */
        if (!(z->a & 0x02))                    /* 6CBF-6CC1; 6CC3 scrive in ROM */
            return;
        zero = !(z->a & 0x01);                 /* 6CC7 */
    }
    IXB(0x0d) = 0x00;                          /* 6CC9 */
    IXB(0x01) |= 0x10;                         /* 6CCD */
    if (zero)                                  /* 6CD1 jp z,6e8b */
        IYB(0x1d) = 0x02;                      /* 6E8B */
    else
        IYB(0x1e) |= 0x02;                     /* 6CD4 */
}

/* 0x7F48, fase 4 (tipo 17): fase 4 e spegne il bit 5 di (IY+0). */
void sub_7f48(struct z80 *z)
{
    IYB(0x1d) = 0x04;                          /* 7F48 */
    IYB(0x00) &= (uint8_t)~0x20;               /* 7F4C */
}

/* 0x7F81: a (IX+8) >= 78 prosegue a 80A5. */
void sub_7f81(struct z80 *z)
{
    z->a = IXB(0x08);                          /* 7F81 */
    if (z->a < 0x78)                           /* 7F84-7F86 ret c */
        return;
    sub_80a5(z);                               /* 7F87 jp 80a5 */
}

/* 0x7F8A: 24BE con D = (IY+0E), E = (IY+15), 3A54 con BC = 2011, fase 5,
 * spegne il bit 5 di (IY+0), (IY+1A) = 30. */
void sub_7f8a(struct z80 *z)
{
    z->d = IYB(0x0e);                          /* 7F8A */
    z->e = IYB(0x15);                          /* 7F8D */
    sub_24be(z);                               /* 7F90 */
    SET_BC(z, 0x2011);                         /* 7F93 */
    sub_3a54(z);                               /* 7F96 */
    IYB(0x1d) = 0x05;                          /* 7F99 */
    IYB(0x00) &= (uint8_t)~0x20;               /* 7F9D */
    IYB(0x1a) = 0x30;                          /* 7FA1 */
}

/* 0x7FA6: la prima volta accende il bit 0 di (IX+1) e azzera (IX+8); a
 * (IX+8) >= 10 spegne i bit 0 e 2, il bit 5 di (IY+0), azzera (IX+8) e fase 4. */
void sub_7fa6(struct z80 *z)
{
    if (!(IXB(0x01) & 0x01)) {                 /* 7FA6-7FAA */
        IXB(0x01) |= 0x01;                     /* 7FAC */
        IXB(0x08) = 0x00;                      /* 7FB0 */
    }
    z->a = IXB(0x08);                          /* 7FB4 */
    if (z->a < 0x10)                           /* 7FB7-7FB9 ret c */
        return;
    z->a = 0;                                  /* 7FBA xor a */
    IXB(0x01) &= (uint8_t)~0x01;               /* 7FBB */
    IXB(0x01) &= (uint8_t)~0x04;               /* 7FBF */
    IYB(0x00) &= (uint8_t)~0x20;               /* 7FC3 */
    IXB(0x08) = z->a;                          /* 7FC7 */
    IYB(0x1d) = 0x04;                          /* 7FCA */
}

/* 0x7FCF: la prima volta (bit 2 di (IX+1)) prepara (IY+0E) e (IY+1A) = 30;
 * poi con (IY+1A) nullo va a 808D se (IX+0E) = 5, altrimenti a 809C; con
 * (IY+1A) non nullo attende (IX+8) >= D2, sposta (IY+0E) di 6 e passa alla
 * fase (IX+0E). */
void sub_7fcf(struct z80 *z)
{
    if (!(IXB(0x01) & 0x04)) {                 /* 7FCF-7FD3 */
        IXB(0x01) |= 0x04;                     /* 7FD5 */
        IYB(0x01) &= (uint8_t)~0x02;           /* 7FD9 */
        z->a = 0xf0 & IYB(0x0e);               /* 7FDD-7FDF */
        z->a = (uint8_t)(z->a - 0x06);         /* 7FE2 */
        IYB(0x1a) = 0x30;                      /* 7FE4 */
        IYB(0x0e) = z->a;                      /* 7FE8 */
        IXB(0x08) = 0x00;                      /* 7FEB */
    }
    z->a = IYB(0x1a);                          /* 7FEF */
    if (z->a == 0) {                           /* 7FF2-7FF3 */
        z->a = IXB(0x0e);                      /* 7FF5 */
        if (z->a == 0x05)                      /* 7FF8 cp 5 */
            sub_808d(z);                       /* 7FFA jp z,808d */
        else
            sub_809c(z);                       /* 7FFD jp 809c */
        return;
    }
    z->a = IXB(0x08);                          /* 8000 */
    if (z->a < 0xd2)                           /* 8003-8005 ret c */
        return;
    z->a = 0x06;                               /* 8006 */
    z->a = (uint8_t)(z->a + IYB(0x0e));        /* 8008 */
    IYB(0x0e) = z->a;                          /* 800B */
    z->a = IXB(0x0e);                          /* 800E */
    IYB(0x1d) = z->a;                          /* 8011 */
    IYB(0x1e) = 0x00;                          /* 8014 */
    IXB(0x01) &= (uint8_t)~0x04;               /* 8018 */
}

/* 0x801D: col bit 1 di (IY+1) (IX+0E) = 4 e fase 3; a (IX+8) >= 78 azzera
 * (IX+8) e la fase; altrimenti con (IY+1A) & 30 nullo prosegue a 809C. */
void sub_801d(struct z80 *z)
{
    if (IYB(0x01) & 0x02) {                    /* 801D-8021 */
        IXB(0x0e) = 0x04;                      /* 8023 */
        IYB(0x1d) = 0x03;                      /* 8027 */
    }
    z->a = IXB(0x08);                          /* 802B */
    if (z->a >= 0x78) {                        /* 802E-8030 */
        z->a = 0;                              /* 8032 */
        IXB(0x08) = 0;                         /* 8033 */
        IYB(0x1d) = 0;                         /* 8036 */
        return;
    }
    z->a = IYB(0x1a) & 0x30;                   /* 803A-803D */
    if (z->a)                                  /* 803F ret nz */
        return;
    sub_809c(z);                               /* 8040 jr 809c */
}

/* 0x8042: col bit 1 di (IY+1) (IX+0E) = 5 e fase 3; con (IY+1E) pari e
 * (IY+1A) & 30 nullo va a 808D; altrimenti a (IX+8) >= tab8089[(IY+1E)]
 * (04 08 08 0C) azzera (IX+8), avanza (IY+1E) (mod 4) e sposta (IY+0E) di
 * +7 o -7, con (IY+1A) = 30. */
void sub_8042(struct z80 *z)
{
    static const uint8_t tab8089[4] = { 0x04, 0x08, 0x08, 0x0c };
    if (IYB(0x01) & 0x02) {                    /* 8042-8046 */
        IXB(0x0e) = 0x05;                      /* 8048 */
        IYB(0x1d) = 0x03;                      /* 804C */
    }
    z->c = IYB(0x1e);                          /* 8050 */
    if (!(z->c & 0x01)) {                      /* 8053-8055 */
        z->a = IYB(0x1a) & 0x30;               /* 8057-805A */
        if (z->a == 0) {                       /* 805C jr z,808d */
            sub_808d(z);
            return;
        }
    }
    z->e = z->c;                               /* 805E */
    z->d = 0x00;                               /* 805F */
    SET_HL(z, (uint16_t)(0x8089 + DE(z)));     /* 8061-8064 */
    z->a = IXB(0x08);                          /* 8065 */
    uint8_t lim = (DE(z) < 4) ? tab8089[DE(z)] : leggi(HL(z));   /* 8068 */
    if (z->a < lim)                            /* 8069 ret c */
        return;
    IXB(0x08) = 0x00;                          /* 806A */
    z->a = (uint8_t)((z->c + 1) & 0x03);       /* 806E-8070 */
    z->c = z->a;                               /* 8072 */
    z->a = (z->a & 0x01) ? 0x07 : 0xf9;       /* 8073-8079 */
    z->a = (uint8_t)(z->a + IYB(0x0e));        /* 807B */
    IYB(0x1e) = z->c;                          /* 807E */
    IYB(0x0e) = z->a;                          /* 8081 */
    IYB(0x1a) = 0x30;                          /* 8084 */
}

/* 0x808D: 24BE con D = (IY+0E), E = (IY+15), 4C11 con BC = 0 e prosegue
 * in 809C. */
void sub_808d(struct z80 *z)
{
    z->d = IYB(0x0e);                          /* 808D */
    z->e = IYB(0x15);                          /* 8090 */
    sub_24be(z);                               /* 8093 */
    SET_BC(z, 0x0000);                         /* 8096 */
    sub_4c11(z);                               /* 8099 */
    sub_809c(z);                               /* cade in 809C */
}

/* 0x809C: accende il bit 5 di (IY+0) e fase 6. */
void sub_809c(struct z80 *z)
{
    IYB(0x00) |= 0x20;                         /* 809C */
    IYB(0x1d) = 0x06;                          /* 80A0 */
}
