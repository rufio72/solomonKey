/* Motore dei nemici, lotto N11 (brief 37): tipo 07 di 4F0F, seconda metà.
 * 5FC8 (destinazione 2 di 5E1A) con 5FDC e 6046 (movimento in orizzontale,
 * con la tabella ROM 60CC / 60EC letta con `rst 8` = sub_0008), 610C (sceglie
 * lo stato dell'oggetto e smista con `rst 38` a 6130) e le sue destinazioni
 * 6151-6270, 627F (crea un oggetto con 4AF6).
 *
 * Riporto: 6248 lascia in z->f il riporto (A < 4 dopo `cp 4`), che 6179 e
 * 6183 leggono. 627F ha un'uscita anomala: `pop hl; ret` (62B4) scarta
 * l'indirizzo di ritorno e torna al chiamante del chiamante; in C corpo_627f
 * lascia HL = indirizzo scartato (5D0F da 610C, 5D28 da 5E9D) e sub_610c
 * segnala l'uscita anomala con il riporto acceso (1 solo dal caso 627F),
 * così sub_5cfb non prosegue (stessa convenzione di 531C in 50BA e 50DF).
 *
 * 6151, 6165 ecc. sono salti `jp`: in C sono chiamate seguite da return.
 * IX = slot, IY = oggetto. Tradotto da out/main64.dasm (out/nemici_n11.dasm). */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"

#define IXB(o) M(z->ix + (o))
#define IYB(o) M(z->iy + (o))

/* 0x6248: A = ((IY+0E) + 2) & 0F; riporto = A < 4 (`cp 4`). */
void sub_6248(struct z80 *z)
{
    z->a = (uint8_t)((IYB(0x0e) + 0x02) & 0x0f);          /* 6248-624D */
    NEMICI_PONI_RIPORTO(z, z->a < 0x04);                  /* 624F */
}

/* 0x6252: (IY+15) arrotondato a multiplo di 16 (più 8 se il bit 3 è acceso). */
void sub_6252(struct z80 *z)
{
    z->a = IYB(0x15);                                     /* 6252 */
    if (z->a & 0x08)                                      /* 6255 */
        z->a = (uint8_t)(z->a + 0x08);                    /* 6259 */
    z->a &= 0xf0;                                         /* 625B */
    IYB(0x15) = z->a;                                     /* 625D */
}

/* 0x6196: (IY+0E) e (IY+15) arrotondati a multipli di 16 (+8). */
void sub_6196(struct z80 *z)
{
    z->a = (uint8_t)((IYB(0x0e) + 0x08) & 0xf0);          /* 6196-619B */
    IYB(0x0e) = z->a;                                     /* 619D */
    z->a = (uint8_t)((IYB(0x15) + 0x08) & 0xf0);          /* 61A0-61A5 */
    IYB(0x15) = z->a;                                     /* 61A7 */
    SET_BC(z, 0x0c02);                                    /* 61AA */
    sub_5e4d(z);                                          /* 61AD */
}

/* 0x6151: come 6165 con (IY+0E) = nibble alto + 0A e BC = 0C00. */
void sub_6151(struct z80 *z)
{
    sub_6252(z);                                          /* 6151 */
    z->a = (uint8_t)((IYB(0x0e) & 0xf0) + 0x0a);          /* 6154-6159 */
    IYB(0x0e) = z->a;                                     /* 615B */
    SET_BC(z, 0x0c00);                                    /* 615E */
    sub_5e4d(z);                                          /* 6161 */
}

/* 0x6165: (IY+0E) = nibble alto + 06, BC = 0C01. */
void sub_6165(struct z80 *z)
{
    sub_6252(z);                                          /* 6165 */
    z->a = (uint8_t)((IYB(0x0e) & 0xf0) + 0x06);          /* 6168-616D */
    IYB(0x0e) = z->a;                                     /* 616F */
    SET_BC(z, 0x0c01);                                    /* 6172 */
    sub_5e4d(z);                                          /* 6175 */
}

/* 0x618B (ingresso interno, coda di 6179 e 6183): C = A, B = 3, 5E4D. */
void sub_618b(struct z80 *z)
{
    z->c = z->a;                                          /* 618B */
    z->b = 0x03;                                          /* 618C */
    sub_5e4d(z);                                          /* 618E */
    IXB(0x0e) = 0xff;                                     /* 6191 */
}

/* 0x6179: se 6248 lascia il riporto va a 6196, altrimenti A = 2 e 618B. */
void sub_6179(struct z80 *z)
{
    sub_6248(z);                                          /* 6179 */
    if (NEMICI_RIPORTO_ACCESO(z)) {                       /* 617C */
        sub_6196(z);
        return;
    }
    z->a = 0x02;                                          /* 617F */
    sub_618b(z);                                          /* 6181 */
}

/* 0x6183: come 6179 con A = 3. */
void sub_6183(struct z80 *z)
{
    sub_6248(z);                                          /* 6183 */
    if (NEMICI_RIPORTO_ACCESO(z)) {                       /* 6186 */
        sub_6196(z);
        return;
    }
    z->a = 0x03;                                          /* 6189 */
    sub_618b(z);                                          /* 618B */
}

/* 0x61B1: A (nibble o valore di 6261/6270) ruotata a destra in E; tre
 * rami su (IY+0E) e (IY+15). */
void sub_61b1(struct z80 *z)
{
    z->a = (uint8_t)((z->a >> 1) | (z->a << 7));          /* 61B1 rrca */
    z->e = z->a;                                          /* 61B2 */
    z->a = IYB(0x0e);                                     /* 61B3 */
    z->b = z->a;                                          /* 61B6 */
    z->a = (uint8_t)((z->a + 0x03) & 0x0f);               /* 61B7-61B9 */
    if (z->a < 0x06) {                                    /* 61BB-61BD */
        sub_6196(z);
        return;
    }
    z->c = IYB(0x15);                                     /* 61C0 */
    z->a = (uint8_t)((z->c + 0x02) & 0x0f);               /* 61C3-61C6 */
    if (z->a < 0x04) {                                    /* 61C8-61CA */
        z->a = (uint8_t)(0x0c & z->e);                    /* 61CC-61CE */
        if (z->a == 0) {
            sub_6165(z);                                  /* 61CF */
            return;
        }
        z->a = (uint8_t)(0x03 & z->e);                    /* 61D2-61D4 */
        if (z->a == 0) {
            sub_6151(z);                                  /* 61D5 */
            return;
        }
        sub_6196(z);                                      /* 61D8 */
        return;
    }
    z->a = z->b;                                          /* 61DB */
    if (!(z->a & 0x08))                                   /* 61DC */
        z->a = (uint8_t)(z->a + 0x08);                    /* 61E0 */
    z->a &= 0xf8;                                         /* 61E2 */
    IYB(0x0e) = z->a;                                     /* 61E4 */
    z->a = z->c;                                          /* 61E7 */
    if (!(z->a & 0x08))                                   /* 61E8 */
        z->a = (uint8_t)(z->a + 0x08);                    /* 61EC */
    z->a &= 0xf8;                                         /* 61EE */
    IYB(0x15) = z->a;                                     /* 61F0 */
    SET_BC(z, 0x0c03);                                    /* 61F3 */
    sub_5e4d(z);                                          /* 61F6 */
}

/* 0x61FA: (IY+15) = multiplo di 8 + 1; (IY+0E) = ((IY+0E)-2) nibble alto + 0A;
 * BC = 0D00. */
void sub_61fa(struct z80 *z)
{
    z->a = IYB(0x15);                                     /* 61FA */
    if (z->a & 0x08)                                      /* 61FD */
        z->a = (uint8_t)(z->a + 0x08);                    /* 6201 */
    z->a = (uint8_t)((z->a & 0xf8) + 1);                  /* 6203-6205 */
    z->b = z->a;                                          /* 6206 */
    z->a = (uint8_t)(((IYB(0x0e) - 0x02) & 0xf0) + 0x0a); /* 6207-620E */
    IYB(0x00) &= ~0x40;                                   /* 6210 */
    IYB(0x15) = z->b;                                     /* 6214 */
    IYB(0x0e) = z->a;                                     /* 6217 */
    SET_BC(z, 0x0d00);                                    /* 621A */
    sub_5e4d(z);                                          /* 621D */
}

/* 0x6221: come 61FA con (IY+15) = multiplo di 8 - 1 e BC = 0D01. */
void sub_6221(struct z80 *z)
{
    z->a = IYB(0x15);                                     /* 6221 */
    if (z->a & 0x08)                                      /* 6224 */
        z->a = (uint8_t)(z->a + 0x08);                    /* 6228 */
    z->a = (uint8_t)((z->a & 0xf8) - 1);                  /* 622A-622C */
    z->b = z->a;                                          /* 622D */
    z->a = (uint8_t)(((IYB(0x0e) - 0x02) & 0xf0) + 0x0a); /* 622E-6235 */
    IYB(0x00) &= ~0x40;                                   /* 6237 */
    IYB(0x0e) = z->a;                                     /* 623B */
    IYB(0x15) = z->b;                                     /* 623E */
    SET_BC(z, 0x0d01);                                    /* 6241 */
    sub_5e4d(z);                                          /* 6244 */
}

/* 0x6261: A = ((IY+0E)-6) & 0F; A < 7 -> 61FA, altrimenti 61B1. */
void sub_6261(struct z80 *z)
{
    z->a = (uint8_t)((IYB(0x0e) - 0x06) & 0x0f);          /* 6261-6266 */
    if (z->a < 0x07)                                      /* 6268 */
        sub_61fa(z);                                      /* 626A */
    else
        sub_61b1(z);                                      /* 626D */
}

/* 0x6270: come 6261 con 6221 al posto di 61FA. */
void sub_6270(struct z80 *z)
{
    z->a = (uint8_t)((IYB(0x0e) - 0x06) & 0x0f);          /* 6270-6275 */
    if (z->a < 0x07)                                      /* 6277 */
        sub_6221(z);                                      /* 6279 */
    else
        sub_61b1(z);                                      /* 627C */
}

/* 0x627F-62B5: crea l'oggetto con 4AF6 e scarta l'indirizzo di ritorno
 * (`pop hl; ret`): HL = `scartato`, il chiamante del chiamante prosegue. */
static void corpo_627f(struct z80 *z, uint16_t scartato)
{
    uint8_t d = IYB(0x00);                                /* 627F */
    IYB(0x00) &= ~0x40;                                   /* 6282 */
    d = (uint8_t)((d | 0x20) & ~0x10);                    /* 6286-6288 */
    z->d = d;
    z->a = (uint8_t)(IYB(0x12) | IYB(0x13));              /* 628A-628D */
    z->c = z->a ? 0x00 : 0x02;                            /* 6290-6294 */
    z->a = IYB(0x04);                                     /* 6296 */
    z->e = (uint8_t)(z->a - 0x03);                        /* 6299-629B */
    IYB(0x03) = 0x0c;                                     /* 629C */
    z->b = 0x06;                                          /* 62A0 */
    z->a = z->d;                                          /* 62A2 */
    uint16_t ix = z->ix;                                  /* 62A3-62A5 push iy; ex (sp),ix */
    z->ix = z->iy;
    sub_4af6(z);                                          /* 62A7 */
    z->ix = ix;                                           /* 62AA */
    z->a = (uint8_t)(IXB(0x01) & 0x81);                   /* 62AC-62AF */
    IXB(0x01) = z->a;                                     /* 62B1 */
    SET_HL(z, scartato);                                  /* 62B4 pop hl */
}

/* 0x627F da 5E9D (`jp 627F`): l'indirizzo scartato è il ritorno di 5E1A a 5D28. */
void sub_627f(struct z80 *z)
{
    corpo_627f(z, 0x5d28);
}

/* 0x610C: A = (IY+1A); se uguale a (IX+0E) esce; filtro su (IY+1D) e
 * (IY+1E); (IX+0E) = A, E = nibble basso e `rst 38` a 6130. Tutte le
 * uscite lasciano il riporto spento tranne il caso 627F (anomalo). */
void sub_610c(struct z80 *z)
{
    z->a = IYB(0x1a);                                     /* 610C */
    if (z->a == IXB(0x0e)) {                              /* 610F-6112 ret z */
        NEMICI_PONI_RIPORTO(z, 0);
        return;
    }
    z->b = z->a;                                          /* 6113 */
    z->a = (uint8_t)(IYB(0x1d) - 1);                      /* 6114-6117 */
    if (z->a < 0x02 && z->a == IYB(0x1e)) {               /* 6118-611F */
        z->a = z->b & 0x0f;                               /* 6121-6122 */
        if (z->a != 0x00 && z->a != 0x0f) {               /* 6124-6128 ret nz */
            NEMICI_PONI_RIPORTO(z, 0);
            return;
        }
    }
    z->a = z->b;                                          /* 6129 */
    IXB(0x0e) = z->a;                                     /* 612A */
    z->a &= 0x0f;                                         /* 612D */
    z->e = z->a;                                          /* 612F */
    uint16_t dest = nemici_rst38(z, 0x6130, rst38_6130,
                                 sizeof rst38_6130 / sizeof rst38_6130[0]);
    int anomala = 0;
    switch (dest) {                                       /* 6130 rst 38 */
    case 0x627F: corpo_627f(z, 0x5d0f); anomala = 1; break;
    case 0x6165: sub_6165(z); break;
    case 0x61B1: sub_61b1(z); break;
    case 0x6151: sub_6151(z); break;
    case 0x6261: sub_6261(z); break;
    case 0x6183: sub_6183(z); break;
    case 0x6196: sub_6196(z); break;
    case 0x6179: sub_6179(z); break;
    case 0x6270: sub_6270(z); break;
    case 0x61FA: sub_61fa(z); break;
    case 0x6221: sub_6221(z); break;
    case 0:      break;                                   /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
    NEMICI_PONI_RIPORTO(z, anomala);
}

/* 0x5FDC: `rst 8` sulla tabella ROM 60CC (B, C); ret z, coda 5FAC, o rami
 * 6018 / 5FF5; BC finale 0602, 0600, 0601. */
void sub_5fdc(struct z80 *z)
{
    SET_HL(z, 0x60cc);                                    /* 5FDC */
    sub_0008(z);                                          /* 5FDF */
    z->b = leggi(HL(z));                                  /* 5FE0 */
    SET_HL(z, (uint16_t)(HL(z) + 1));                     /* 5FE1 */
    z->c = leggi(HL(z));                                  /* 5FE2 */
    if (!(z->c & 0x80))                                   /* 5FE3 ret z */
        return;
    if (!(z->c & 0x04)) {                                 /* 5FE6 */
        sub_5fac(z);                                      /* 5FE8 jr z,5FAC */
        return;
    }
    z->d = z->c;                                          /* 5FEA */
    int via_6018 = !(z->c & 0x02);                        /* 5FEB */
    z->b = IXB(0x04);                                     /* 5FED */
    z->c = IXB(0x05);                                     /* 5FF0 */
    if (via_6018) {                                       /* 5FF3 */
        sub_5e8f(z);                                      /* 6018 */
        int c_ = z->a < z->e;                             /* 601B sub e */
        z->a = (uint8_t)(z->a - z->e);
        if (c_)                                           /* 601C */
            goto l6008;
        z->b = 0x06;                                      /* 6020 */
        if (!(z->d & 0x01)) {                             /* 601E-6022 */
            if (!(z->c & 0x80))                           /* 6024 */
                goto l6008;
            z->c = 0x00;                                  /* 6028 */
            return;
        }
        if (z->c & 0x80)                                  /* 602B */
            goto l6008;
        z->c = 0x01;                                      /* 602F */
        return;
    }
    sub_5e8f(z);                                          /* 5FF5 */
    int c_ = z->a < z->e;                                 /* 5FF8 sub e */
    z->a = (uint8_t)(z->a - z->e);
    if (c_) {                                             /* 5FF9 */
        z->a = IYB(0x0e);                                 /* 5FFB */
        if (z->a < 0xc8)                                  /* 5FFE cp c8; 6000 */
            goto l6008;
        IXB(0x01) &= ~0x02;                               /* 6002 */
    }
    int neg = z->c & 0x80;                                /* 6010 */
    SET_BC(z, 0x0600);                                    /* 6012 */
    if (neg)                                              /* 6015 */
        return;
    z->c++;                                               /* 6016 */
    return;
l6008:
    SET_BC(z, 0x0602);                                    /* 6008 */
    IXB(0x01) |= 0x02;                                    /* 600B */
}

/* 0x6046: come 5FDC con la tabella ROM 60EC; BC finale 0702, 0700, 0701. */
void sub_6046(struct z80 *z)
{
    SET_HL(z, 0x60ec);                                    /* 6046 */
    sub_0008(z);                                          /* 6049 */
    z->b = leggi(HL(z));                                  /* 604A */
    SET_HL(z, (uint16_t)(HL(z) + 1));                     /* 604B */
    z->c = leggi(HL(z));                                  /* 604C */
    if (!(z->c & 0x80))                                   /* 604D ret z */
        return;
    if (!(z->c & 0x04)) {                                 /* 6050 */
        sub_5fac(z);                                      /* 6052 jp z,5FAC */
        return;
    }
    z->d = z->c;                                          /* 6055 */
    int via_6072 = !(z->c & 0x02);                        /* 6056 */
    z->b = IXB(0x04);                                     /* 6058 */
    z->c = IXB(0x05);                                     /* 605B */
    if (via_6072) {                                       /* 605E */
        sub_5e8f(z);                                      /* 6072 */
        int c_ = z->a < z->e;                             /* 6075 sub e */
        z->a = (uint8_t)(z->a - z->e);
        if (c_)                                           /* 6076 */
            goto l6066;
        z->b = 0x07;                                      /* 607A */
        if (!(z->d & 0x01)) {                             /* 6078-607C */
            if (!(z->c & 0x80))                           /* 607E */
                goto l6066;
            z->c = 0x00;                                  /* 6082 */
            return;
        }
        if (z->c & 0x80)                                  /* 6085 */
            goto l6066;
        z->c = 0x01;                                      /* 6089 */
        return;
    }
    sub_5e8f(z);                                          /* 6060 */
    int c_ = z->a < z->e;                                 /* 6063 sub e */
    z->a = (uint8_t)(z->a - z->e);
    if (c_)                                               /* 6064 */
        goto l6066;
    int neg = z->c & 0x80;                                /* 606A */
    SET_BC(z, 0x0700);                                    /* 606C */
    if (neg)                                              /* 606F */
        return;
    z->c++;                                               /* 6070 */
    return;
l6066:
    SET_BC(z, 0x0702);                                    /* 6066 */
}

/* 0x5FC8: bit 7 de (IX+4) e bit 1 de (IX+1) spenti: DE = 0800, 5F0C, 6046,
 * 5E4D; altrimenti DE = 1800, 5F0C, 5FDC, 5E4D. */
void sub_5fc8(struct z80 *z)
{
    int basso = !(IXB(0x04) & 0x80) && !(IXB(0x01) & 0x02);  /* 5FC8, 6032 */
    SET_DE(z, basso ? 0x0800 : 0x1800);                   /* 6039 / 5FCF */
    sub_5f0c(z);                                          /* 603C / 5FD2 */
    if (basso)
        sub_6046(z);                                      /* 603F */
    else
        sub_5fdc(z);                                      /* 5FD5 */
    sub_5e4d(z);                                          /* 6042 / 5FD8 */
}
