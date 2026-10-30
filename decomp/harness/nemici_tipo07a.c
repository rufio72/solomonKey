/* Motore dei nemici, lotto N8 (brief 34): tipo 07 di 4F0F, prima metà.
 * 5CFB e le routine che chiama: 5D29 (consuma la tabella 84C8 dei passi),
 * 5D76, 5DDF e 5E09 (scelta a caso della direzione verso il giocatore),
 * 5E1A (smista con `rst 38` a 5E40 sullo stato (IY+1D/1E)), 5E4D, 5E60,
 * 5E8F, 5E9D, 5EDD, 5F0C, 5F33, 5F4B, 5F82; 5FAC è anticipata da N11.
 * 4FEF (anticipata da N10) sta in nemici_tipo06a.c.
 * 610C, 5FC8 e 627F restano stub (nemici_stub.c).
 * 5DDF legge il registro R (due `ld a,r`): nel banco serve banco_r_5ddf
 * (e banco_r_5e1a per chi arriva da 5E1A).
 * IX = slot (c2c1 + i·0x18), IY = oggetto (c4fe + i·0x30). Tradotto da
 * out/main64.dasm (out/nemici_n8.dasm). */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"

#define IXB(o) M(z->ix + (o))
#define IYB(o) M(z->iy + (o))

/* 0x5CFB: tipo 07. Con il bit 2 di (IX+0) acceso prosegue il passo
 * (5D29); altrimenti 610C sceglie lo stato, (IY+1B) = (IY+4), 4FEF dà BC
 * dalla tabella e, se (IX+0B/0C) >= BC, il bit 4 di (IY+0) si accende e
 * 5E1A sceglie la mossa. */
void sub_5cfb(struct z80 *z)
{
    sub_52fd(z);                                          /* 5CFB */
    if (IXB(0x00) & 0x04) {                               /* 5CFE */
        sub_5d29(z);                                      /* 5D04 */
        return;                                           /* 5D07 */
    }
    IYB(0x00) &= ~0x10;                                   /* 5D08 */
    sub_610c(z);                                          /* 5D0C */
    z->a = IYB(0x04);                                     /* 5D0F */
    IYB(0x1b) = z->a;                                     /* 5D12 */
    sub_4fef(z);                                          /* 5D15 */
    uint16_t hl = (uint16_t)(IXB(0x0b) | (IXB(0x0c) << 8));  /* 5D18-5D1B */
    uint16_t bc = BC(z);
    SET_HL(z, (uint16_t)(hl - bc));                       /* 5D1E sbc hl,bc */
    if (hl < bc)                                          /* 5D20 ret c */
        return;
    IYB(0x00) |= 0x10;                                    /* 5D21 */
    sub_5e1a(z);                                          /* 5D25 */
}

/* 0x5D76: A = nibble basso di A con segno (bit 3 acceso: estende a 0xF_). */
void sub_5d76(struct z80 *z)
{
    if (z->a & 0x08)                                      /* 5D76 */
        z->a |= 0xf0;                                     /* 5D7A */
    else
        z->a &= 0x0f;                                     /* 5D7D */
}

/* 0x5D29: passo del tipo 07 con il bit 2 di (IX+0) acceso. Con
 * (IX+0D) < 3 esce subito. Altrimenti azzera (IX+0D) e, dallo stato
 * (IY+1D/1E), o chiude il movimento (5DC3), o legge la tabella 84C8
 * (4 byte per voce) e applica il passo a (IY+0E) e (IY+15), o a fine
 * tabella (5D80) lancia un oggetto con 4AF6. */
void sub_5d29(struct z80 *z)
{
    z->a = IXB(0x0d);                                     /* 5D29 */
    if (z->a < 0x03)                                      /* 5D2C-5D2E */
        return;
    IXB(0x0d) = 0x00;                                     /* 5D2F */
    z->a = (uint8_t)(IYB(0x1d) * 4 + IYB(0x1e));          /* 5D33-5D38 */
    int minore = z->a < 0x02;                             /* 5D3B sub 2 */
    z->a = (uint8_t)(z->a - 0x02);
    if (minore) {                                         /* 5D3D */
        IXB(0x00) &= ~0x04;                               /* 5DC3 */
        IXB(0x01) |= 0x01;                                /* 5DC7 */
        z->a = IYB(0x1e);                                 /* 5DCB */
        if (z->a & 0x01)                                  /* 5DCE-5DD5 */
            SET_BC(z, 0x0207);
        else
            SET_BC(z, 0x0100);
        IYB(0x1d) = z->b;                                 /* 5DD8 */
        IXB(0x10) = z->c;                                 /* 5DDB */
        return;
    }
    SET_HL(z, 0x84c8);                                    /* 5D40 */
    sub_0008(z);                                          /* 5D43 */
    uint16_t de = (uint16_t)(HL(z) + DE(z) + DE(z));      /* 5D44-5D45 */
    uint16_t hl = DE(z);                                  /* 5D46 ex de,hl */
    SET_DE(z, de);
    SET_HL(z, hl);
    IYB(0x22) = 0x80;                                     /* 5D47 */
    z->a = IYB(0x24);                                     /* 5D4B */
    unsigned c = z->a & 0x01;                             /* 5D4E rra */
    z->a >>= 1;
    z->a = (uint8_t)(z->a - 1);                           /* 5D4F dec a */
    if (z->a == 0) {                                      /* 5D50 */
        /* 5D80: fine della tabella */
        IXB(0x00) &= ~0x04;
        IXB(0x0e) = 0xff;                                 /* 5D84 */
        z->a = IYB(0x1d);                                 /* 5D88 */
        if (z->a < 0x0a)                                  /* 5D8B-5D8D */
            return;
        int uguale = (z->a == 0x0a);
        z->a = IYB(0x1e);                                 /* 5D8E */
        if (uguale && !(z->a & 0x02))                     /* 5D91-5D95 */
            return;
        z->d = z->a;                                      /* 5D96 */
        z->a = IYB(0x04);                                 /* 5D97 */
        z->a = (uint8_t)(z->a - 0x03);
        z->e = z->a;                                      /* 5D9C */
        IYB(0x03) = 0x0c;                                 /* 5D9D */
        SET_BC(z, 0x0502);                                /* 5DA1 */
        z->a = (uint8_t)((IYB(0x00) | 0x20) & ~0x10);     /* 5DA4-5DA9 */
        uint16_t ix = z->ix;                              /* 5DAB-5DAD */
        z->ix = z->iy;
        sub_4af6(z);                                      /* 5DAF */
        z->ix = ix;                                       /* 5DB2 */
        z->a = IXB(0x01);                                 /* 5DB6 */
        if (z->d & 0x01)                                  /* 5DB4, 5DB9 */
            z->a |= 0x80;                                 /* 5DBB */
        z->a &= 0x80;                                     /* 5DBD */
        IXB(0x01) = z->a;                                 /* 5DBF */
        return;
    }
    z->l = z->a;                                          /* 5D53 */
    z->a = (uint8_t)((z->a << 1) | c);                    /* 5D54 rla */
    IYB(0x24) = z->a;                                     /* 5D55 */
    z->l = (uint8_t)(z->l - 1);                           /* 5D58 */
    z->h = 0x00;                                          /* 5D59 */
    SET_HL(z, (uint16_t)(HL(z) + DE(z)));                 /* 5D5B */
    z->b = leggi(HL(z));                                  /* 5D5C */
    z->a = z->b;                                          /* 5D5D */
    sub_5d76(z);                                          /* 5D5E */
    z->a = (uint8_t)(z->a + IYB(0x0e));                   /* 5D61 */
    IYB(0x0e) = z->a;                                     /* 5D64 */
    z->a = z->b;                                          /* 5D67 */
    z->a = (uint8_t)((z->a >> 4) | (z->a << 4));          /* 5D68-5D6B rrca x4 */
    sub_5d76(z);                                          /* 5D6C */
    z->a = (uint8_t)(z->a + IYB(0x15));                   /* 5D6F */
    IYB(0x15) = z->a;                                     /* 5D72 */
}

/* 0x5E09: A = valore di riferimento, C = coordinata. Esce con
 * B = (C - (-A - 0x10)) / 2 (con segno), A = |B·2'| e il riporto
 * (nel bit 0 di z->f) acceso se A < 0x18: «abbastanza vicino». */
void sub_5e09(struct z80 *z)
{
    uint8_t a = (uint8_t)(-z->a);                         /* 5E09 neg */
    a = (uint8_t)(a - 0x10);                              /* 5E0B */
    uint8_t b = a;                                        /* 5E0D */
    a = z->c;                                             /* 5E0E */
    unsigned c = a < b;                                   /* 5E0F sub b */
    a = (uint8_t)(a - b);
    unsigned c2 = a & 0x01;                               /* 5E10 rra */
    a = (uint8_t)((c << 7) | (a >> 1));
    c = c2;
    b = a;                                                /* 5E11 */
    unsigned c3 = a >> 7;                                 /* 5E12 rla */
    a = (uint8_t)((a << 1) | c);
    if (c3)                                               /* 5E13 */
        a = (uint8_t)(-a);                                /* 5E15 */
    z->b = b;
    z->a = a;
    NEMICI_PONI_RIPORTO(z, a < 0x18);                     /* 5E17 cp 18h */
}

/* 0x5DDF: due `ld a,r` ravvicinati (D = somma); con il bit 0 di D prova a
 * orientare (IX+5) secondo (c843) e (IY+15), con il bit 1 spento
 * (IX+4) secondo (c83c) e (IY+0E), solo se 5E09 le trova vicine. C resta
 * R1 sui percorsi che non ricaricano C. Nel banco: banco_r_5ddf. */
void sub_5ddf(struct z80 *z)
{
    uint8_t r1;
    z->a = nemici_r_somma_r1(&r1);                        /* 5DDF-5DE4 */
    z->c = r1;
    z->d = z->a;                                          /* 5DE5 */
    if (z->d & 0x01) {                                    /* 5DE6 */
        z->a = M(0xc843);                                 /* 5DEA */
        z->c = IYB(0x15);                                 /* 5DED */
        sub_5e09(z);                                      /* 5DF0 */
        if (!NEMICI_RIPORTO_ACCESO(z))                    /* 5DF3 */
            IXB(0x05) = z->b;                             /* 5DF5 */
    }
    if (z->d & 0x02)                                      /* 5DF8-5DFA */
        return;
    z->a = M(0xc83c);                                     /* 5DFB */
    z->c = IYB(0x0e);                                     /* 5DFE */
    sub_5e09(z);                                          /* 5E01 */
    if (NEMICI_RIPORTO_ACCESO(z))                         /* 5E04 */
        return;
    IXB(0x04) = z->b;                                     /* 5E05 */
}

/* 0x5E4D: (IY+1D) = B, (IY+1E) = C, con il bit 6 di (IY+0) spento
 * durante la scrittura e poi acceso. */
void sub_5e4d(struct z80 *z)
{
    IYB(0x00) &= ~0x40;                                   /* 5E4D */
    IYB(0x1d) = z->b;                                     /* 5E51 */
    IYB(0x1e) = z->c;                                     /* 5E54 */
    IYB(0x00) |= 0x40;                                    /* 5E57 */
}

/* 0x5E1A: sceglie la mossa. Se il bit 5 di (IX+0) è spento chiama 5DDF
 * (nel banco: banco_r_5e1a). Poi bit 2 acceso, (IX+0D/0B/0C) = 0,
 * (IY+1F) = ff; con (IY+1D) < 0C esce (nell'originale scrive in ROM). Altrimenti
 * E = 4·((IY+1D) - 0C) + (IY+1E) e `rst 38` al sito 5E40 (6 voci). */
void sub_5e1a(struct z80 *z)
{
    if (!(IXB(0x00) & 0x20))                              /* 5E1A */
        sub_5ddf(z);                                      /* 5E1E */
    IXB(0x00) |= 0x04;                                    /* 5E21 */
    IXB(0x0d) = 0x00;                                     /* 5E25-5E26 */
    IXB(0x0b) = 0x00;                                     /* 5E29 */
    IXB(0x0c) = 0x00;                                     /* 5E2C */
    IYB(0x1f) = 0xff;                                     /* 5E2F */
    uint8_t a = IYB(0x1d);                                /* 5E33 */
    int minore = a < 0x0c;
    a = (uint8_t)(a - 0x0c);                              /* 5E36 */
    z->a = a;
    if (minore)                                           /* 5E38 */
        return;                                           /* 5E5C-5E5F */
    a = (uint8_t)(a * 4 + IYB(0x1e));                     /* 5E3A-5E3C */
    z->a = a;
    z->e = a;                                             /* 5E3F */
    uint16_t dest = nemici_rst38(z, 0x5E40, rst38_5E40,
                                 sizeof rst38_5E40 / sizeof rst38_5E40[0]);
    switch (dest) {                                       /* 5E40 rst 38 */
    case 0x5E60: sub_5e60(z); break;
    case 0x5E9D: sub_5e9d(z); break;
    case 0x5FC8: sub_5fc8(z); break;
    case 0x5F33: sub_5f33(z); break;
    case 0x5EDD: sub_5edd(z); break;
    case 0:      break;                                   /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}

/* 0x5E60: con il bit 7 di (IX+4) spento (B, C) = (0, 3) e 5E4D; altrimenti
 * lancia con 4AF6 (stato = (IY+0) con bit 5 e senza bit 4, E = (IY+4) - 3,
 * BC = 0502) l'oggetto IY, e azzera (IX+1). */
void sub_5e60(struct z80 *z)
{
    if (!(IXB(0x04) & 0x80)) {                            /* 5E60 */
        SET_BC(z, 0x0003);                                /* 5E66 */
        sub_5e4d(z);                                      /* 5E69 */
        return;
    }
    uint16_t ix = z->ix;                                  /* 5E6D-5E6F */
    z->ix = z->iy;
    IXB(0x03) = 0x0c;                                     /* 5E71 */
    z->a = IYB(0x04);                                     /* 5E75 */
    z->e = (uint8_t)(z->a - 0x03);                        /* 5E78-5E7A */
    z->a = (uint8_t)((IYB(0x00) & ~0x10) | 0x20);         /* 5E7B-5E80 */
    SET_BC(z, 0x0502);                                    /* 5E82 */
    sub_4af6(z);                                          /* 5E85 */
    z->ix = ix;                                           /* 5E88 */
    IXB(0x01) = 0x00;                                     /* 5E8A */
}

/* 0x5E8F: |2B| in E e |2C| in A (bit 7 prima del raddoppio = negativo). */
void sub_5e8f(struct z80 *z)
{
    uint8_t a = z->b;                                     /* 5E8F */
    unsigned neg = a >> 7;
    a = (uint8_t)(a << 1);                                /* 5E90 */
    if (neg)
        a = (uint8_t)(-a);                                /* 5E93 */
    z->e = a;                                             /* 5E95 */
    a = z->c;                                             /* 5E96 */
    neg = a >> 7;
    a = (uint8_t)(a << 1);                                /* 5E97 */
    if (neg)
        a = (uint8_t)(-a);                                /* 5E9A */
    z->a = a;                                             /* 5E9C */
}

/* 0x5E9D: mossa verso il giocatore. B, C = (IX+4), (IX+5); se |2C| >= |2B|
 * guarda la cella davanti (36F0) e in certi casi (B, C) = (3, C); altrimenti
 * (B, C) = (0, 2) se B è negativo; altrimenti accende il bit 7 di (IX+1)
 * e salta a 627F (stub). */
void sub_5e9d(struct z80 *z)
{
    z->b = IXB(0x04);                                     /* 5E9D */
    z->c = IXB(0x05);                                     /* 5EA0 */
    sub_5e8f(z);                                          /* 5EA3 */
    unsigned minore = z->a < z->e;                        /* 5EA6 sub e */
    z->a = (uint8_t)(z->a - z->e);
    if (!minore) {                                        /* 5EA7 */
        z->a = (uint8_t)(0x10 + IYB(0x0e));               /* 5EA9-5EAB */
        z->h = z->a;                                      /* 5EAE */
        z->l = IYB(0x15);                                 /* 5EAF */
        sub_36f0(z);                                      /* 5EB2 */
        unsigned neg = z->c & 0x80;                       /* 5EB5 */
        z->c = 0x00;                                      /* 5EB7 */
        int vai = 1;
        if (!neg) {                                       /* 5EB9 */
            if (M(HL(z)) & 0x80) {                        /* 5EC2 */
                z->c++;                                   /* 5EC6 */
            } else {
                vai = 0;                                  /* 5EC4 */
            }
        } else {
            SET_HL(z, (uint16_t)(HL(z) + 1));             /* 5EBB */
            if (!(M(HL(z)) & 0x80))                       /* 5EBC-5EBE */
                vai = 0;
        }
        if (vai) {
            z->b = 0x03;                                  /* 5EC7 */
            sub_5e4d(z);                                  /* 5ED2 */
            return;
        }
    }
    if (!(z->b & 0x80)) {                                 /* 5ECB */
        IXB(0x01) |= 0x80;                                /* 5ED6 */
        sub_627f(z);                                      /* 5EDA jp 627F */
        return;
    }
    SET_BC(z, 0x0002);                                    /* 5ECF */
    sub_5e4d(z);                                          /* 5ED2 */
}

/* 0x5EDD: come 5E9D ma per le destinazioni 4 e 5: azzera il bit 1 di
 * (IX+1), (B, C) da (IX+4/5), e sceglie (B, C) da 5E8F, (IY+1E) e dal
 * segno di C o di E; poi 5E4D. */
void sub_5edd(struct z80 *z)
{
    IXB(0x01) &= ~0x02;                                   /* 5EDD */
    z->b = IXB(0x04);                                     /* 5EE1 */
    z->c = IXB(0x05);                                     /* 5EE4 */
    sub_5e8f(z);                                          /* 5EE7 */
    unsigned minore = z->a < z->e;                        /* 5EEA sub e */
    z->a = (uint8_t)(z->a - z->e);
    z->e = z->b;                                          /* 5EEB */
    z->a = IYB(0x1e);                                     /* 5EEC */
    z->b = z->a;                                          /* 5EEF */
    z->b++;                                               /* 5EF0 */
    if (!minore) {                                        /* 5EF1 */
        unsigned neg = z->c & 0x80;                       /* 5EF3 */
        z->c = 0x00;                                      /* 5EF5 */
        if (!neg)
            z->c++;                                       /* 5EF9 */
        if (z->a == z->c)                                 /* 5EFA cp c */
            z->b = 0x00;                                  /* 5EFD */
    } else {
        z->c = 0x02;                                      /* 5F01 */
        if (!(z->e & 0x80))                               /* 5F03 */
            z->c++;                                       /* 5F07 */
    }
    sub_5e4d(z);                                          /* 5F08 */
}

/* 0x5F0C: legge le quattro celle 2x2 della griglia a partire da (y, x) =
 * (D + (IY+0E), E + (IY+15)) e ne mette il bit 7 in C (da sinistra:
 * alto-sinistra, alto-destra, basso-sinistra, basso-destra). Uscita
 * A = C (4 bit), HL sull'ultima cella, DE = 000F. */
void sub_5f0c(struct z80 *z)
{
    z->a = (uint8_t)(z->d + IYB(0x0e));                   /* 5F0C-5F0D */
    z->h = z->a;                                          /* 5F10 */
    z->a = (uint8_t)(z->e + IYB(0x15));                   /* 5F11-5F12 */
    z->l = z->a;                                          /* 5F15 */
    z->c = 0x00;                                          /* 5F16 */
    sub_36f0(z);                                          /* 5F18 */
    z->c = (uint8_t)((z->c << 1) | (M(HL(z)) >> 7));      /* 5F1B-5F1D */
    SET_HL(z, (uint16_t)(HL(z) + 1));                     /* 5F1F */
    z->c = (uint8_t)((z->c << 1) | (M(HL(z)) >> 7));      /* 5F20-5F22 */
    SET_DE(z, 0x000f);                                    /* 5F24 */
    SET_HL(z, (uint16_t)(HL(z) + 0x000f));                /* 5F27 */
    z->c = (uint8_t)((z->c << 1) | (M(HL(z)) >> 7));      /* 5F28-5F2A */
    SET_HL(z, (uint16_t)(HL(z) + 1));                     /* 5F2C */
    z->c = (uint8_t)((z->c << 1) | (M(HL(z)) >> 7));      /* 5F2D-5F2F */
    z->a = z->c;                                          /* 5F31 */
}

/* Parte comune di 5F4B e 5F82: dopo la lettura di (B, C) dalla tabella
 * (HL sul secondo byte) decide il risultato; `bit7` della voce spento =
 * esce con (B, C) della tabella; bit 2 spento = coda 5FAC; altrimenti
 * (B, C) = (diff, direzione) con la costante base `base` (0400 / 0500). */
static void scegli_5f4b_5f82(struct z80 *z, uint8_t voce, uint8_t base)
{
    z->b = IXB(0x04);                                     /* 5F5B / 5F92 */
    z->c = IXB(0x05);
    if (voce & 0x02) {                                    /* 5F59 / 5F90 */
        sub_5e8f(z);                                      /* 5F63 */
        unsigned minore = z->a < z->e;                    /* 5F66 sub e */
        z->a = (uint8_t)(z->a - z->e);
        if (!minore) {                                    /* 5F67 */
            SET_BC(z, (uint16_t)(base << 8));             /* 5F69 */
            return;
        }
    }
    uint8_t bb = z->b;                                    /* 5F6D */
    SET_BC(z, (uint16_t)((base << 8) | 0x02));            /* 5F6F */
    if (!(bb & 0x80))
        z->c++;                                           /* 5F73 */
}

/* 0x5F33: azzera il bit 1 di (IX+1); secondo il bit 7 di (IX+5) legge le
 * celle con DE = 1008 e sceglie con 5F4B, oppure con DE = 10F8 e 5F82;
 * poi 5E4D. */
void sub_5f33(struct z80 *z)
{
    IXB(0x01) &= ~0x02;                                   /* 5F33 */
    if (IXB(0x05) & 0x80) {                               /* 5F37 */
        SET_DE(z, 0x1008);                                /* 5F3E */
        sub_5f0c(z);                                      /* 5F41 */
        sub_5f4b(z);                                      /* 5F44 */
    } else {
        SET_DE(z, 0x10f8);                                /* 5F75 */
        sub_5f0c(z);                                      /* 5F78 */
        sub_5f82(z);                                      /* 5F7B */
    }
    sub_5e4d(z);                                          /* 5F47 / 5F7E */
}

/* 0x5F4B: A = indice nella tabella 608C (parole B, C). Bit 7 di C spento:
 * esce con (B, C) della tabella; bit 2 spento: coda 5FAC; altrimenti
 * (B, C) = (04, 00) se bit 1 e 5E8F danno un percorso libero, (04, 02)
 * se (IX+4) è negativo, (04, 03) altrimenti. */
void sub_5f4b(struct z80 *z)
{
    SET_HL(z, 0x608c);                                    /* 5F4B */
    sub_0008(z);                                          /* 5F4E */
    z->b = leggi(HL(z));                                  /* 5F4F */
    SET_HL(z, (uint16_t)(HL(z) + 1));                     /* 5F50 */
    z->c = leggi(HL(z));                                  /* 5F51 */
    uint8_t voce = z->c;
    if (!(voce & 0x80))                                   /* 5F52 */
        return;
    if (!(voce & 0x04)) {                                 /* 5F55 */
        sub_5fac(z);                                      /* 5F57 */
        return;
    }
    scegli_5f4b_5f82(z, voce, 0x04);                      /* 5F59-5F74 */
}

/* 0x5F82: come 5F4B con la tabella 60AC e base 05. */
void sub_5f82(struct z80 *z)
{
    SET_HL(z, 0x60ac);                                    /* 5F82 */
    sub_0008(z);                                          /* 5F85 */
    z->b = leggi(HL(z));                                  /* 5F86 */
    SET_HL(z, (uint16_t)(HL(z) + 1));                     /* 5F87 */
    z->c = leggi(HL(z));                                  /* 5F88 */
    uint8_t voce = z->c;
    if (!(voce & 0x80))                                   /* 5F89 */
        return;
    if (!(voce & 0x04)) {                                 /* 5F8C */
        sub_5fac(z);                                      /* 5F8E */
        return;
    }
    scegli_5f4b_5f82(z, voce, 0x05);                      /* 5F90-5FAB */
}

/* 0x5FAC: coda di 5F4B e 5F82 (anticipata da N11): spegne il bit 2 di
 * (IX+0), 4FEF con A = (IY+4) - 3, (IX+0B/0C) = FFF8, (B, C) =
 * ((IY+1D), (IY+1E)). */
void sub_5fac(struct z80 *z)
{
    IXB(0x00) &= ~0x04;                                   /* 5FAC */
    z->a = (uint8_t)(IYB(0x04) - 0x03);                   /* 5FB0-5FB3 */
    sub_4fef(z);                                          /* 5FB5 */
    SET_HL(z, 0xfff8);                                    /* 5FB8 */
    IXB(0x0b) = z->l;                                     /* 5FBB */
    IXB(0x0c) = z->h;                                     /* 5FBE */
    z->b = IYB(0x1d);                                     /* 5FC1 */
    z->c = IYB(0x1e);                                     /* 5FC4 */
}
