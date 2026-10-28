/* Motore dei nemici, lotto N2 (brief 27): gestori per tipo 14 (6CE8) e
 * 8 (6ECA) chiamati da 4F0F.
 *
 * Tipo 14: 6CE8 confronta (IX+0B..0C) con la parola c1a6/c1a8 (sub_57aa,
 * sub_57b3) e smista con `rst 38h` a 6D03 sulla fase (IY+1D): 571D (in
 * nemici_tipi_10_13.c), 6D1B, 6D37, 6D12, 6DCE, 6E55, 6EA1.
 *
 * Tipo 8: 6ECA muove l'oggetto IY con la velocità di (IX+0E..11), guarda
 * le quattro celle attorno (70FE) e smista con `rst 20h` a 6F1C sulle 16
 * combinazioni di celle piene; ogni destinazione può cambiare la
 * direzione in C; la coda 7056 (raggiunta dal `ret` della destinazione,
 * perché 6ECA spinge 7056 prima del `rst 20h`) scrive velocità e
 * posizione. Le destinazioni di 6F1C e 6F91 sono esportate per il banco.
 *
 * Ingressi: 6CE8, 6D12, 6D1B, 6D37, 6DCE, 6E55 (con 6E83 e 6E8B interni),
 * 6E90, 6EA1, 6ECA, 6F49, 6F4B, 6F5E, 6F6E, 6F81, 6F91, 6FB3, 6FC3, 6FD1,
 * 6FDF, 6FED, 6FF7, 7001, 700B, 7015, 7025, 7036, 7045, 70A8, 70B0, 70CE,
 * 70FE.
 *
 * IX = slot (c2c1 + i·0x18), IY = oggetto (c4fe + i·0x30). Il riporto,
 * dove il codice originale lo legge, sta nel bit 0 di z->f. Tradotto da
 * out/main64.dasm; tabelle da rst38_tables.h e rst20_tables.h. */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"
#include "rst20_tables.h"

#define PORTATI_48AF   0x48af   /* ROM: indirizzo dell'oggetto per indice */
#define VELOCITA_6F3D  0x6f3d   /* ROM: velocità per (IY+4) - 0x16 */
#define DIREZIONI_70EE 0x70ee   /* ROM: 16 byte, direzioni ammesse per combinazione */
#define VELOCITA_C1A0  0xc1a0   /* parola: velocità del tipo 8 */
#define CLASSI_C080    0xc080   /* classe della cella davanti (sub_5c32) */
#define POSA_C08A      0xc08a
#define CODA_7056      0x7056

/* Scrittura a un indirizzo del Z80: sotto c000 è ROM e non cambia. */
static void scrivi(uint16_t a, uint8_t v)
{
    if (a >= 0xc000)
        M(a) = v;
}

/* `ld hl,48af; rst 8; rst 18h`: HL = indirizzo dell'oggetto di indice A,
 * DE = voce successiva della tabella. */
static void portato(struct z80 *z)
{
    SET_HL(z, PORTATI_48AF);
    sub_0008(z);                               /* rst 8 */
    sub_0018(z);                               /* rst 18 */
}

/* ------------------------------------------------------------------ */
/* Tipo 14. */

/* 0x6CE8: BC = c1a6 col bit 3 di (IY+4), c1a8 altrimenti; sub_57aa; senza
 * riporto sub_57b3 con A = 1; poi `rst 38h` a 6D03 sulla fase (IY+1D). */
void sub_6ce8(struct z80 *z)
{
    uint16_t sorgente = (M(z->iy + 0x04) & 0x08) ? 0xc1a6 : 0xc1a8;  /* 6CE8-6CEC */
    z->c = M(sorgente);                        /* 6CEE / 6CF4 ld bc,(nn) */
    z->b = M(sorgente + 1);
    sub_57aa(z);                               /* 6CF8 */
    z->a = 0x01;                               /* 6CFB */
    if (!NEMICI_RIPORTO_ACCESO(z))
        sub_57b3(z);                           /* 6CFD call nc */
    z->e = M(z->iy + 0x1d);                    /* 6D00 */
    uint16_t dest = nemici_rst38(z, 0x6D03, rst38_6D03,
                                 sizeof rst38_6D03 / sizeof rst38_6D03[0]);
    NEMICI_PONI_RIPORTO(z, 0);                 /* rst 38: add hl,de spegne il riporto */
    switch (dest) {                            /* 6D03 rst 38 */
    case 0x571D: sub_571d(z); break;
    case 0x6D1B: sub_6d1b(z); break;
    case 0x6D37: sub_6d37(z); break;
    case 0x6D12: sub_6d12(z); break;
    case 0x6DCE: sub_6dce(z); break;
    case 0x6E55: sub_6e55(z); break;
    case 0x6EA1: sub_6ea1(z); break;
    case 0:      break;                        /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}

/* 0x6D12, fase 3: (IY+12) = 5 e fase 5. */
void sub_6d12(struct z80 *z)
{
    M(z->iy + 0x12) = 0x05;                    /* 6D12 */
    M(z->iy + 0x1d) = 0x05;                    /* 6D16 */
}

/* 0x6D1B, fase 1: col bit 3 di (IY+4) azzera (IX+0B..0C); direzione
 * (IY+1E) = 1 se (IX+5) è positivo, 0 altrimenti; fase 3. */
void sub_6d1b(struct z80 *z)
{
    z->a = 0;                                  /* 6D1B */
    if (M(z->iy + 0x04) & 0x08) {              /* 6D1C-6D20 */
        M(z->ix + 0x0b) = z->a;                /* 6D22 */
        M(z->ix + 0x0c) = z->a;                /* 6D25 */
    }
    if (!(M(z->ix + 0x05) & 0x80))             /* 6D28-6D2C */
        z->a++;                                /* 6D2E */
    M(z->iy + 0x1e) = z->a;                    /* 6D2F */
    M(z->iy + 0x1d) = 0x03;                    /* 6D32 */
}

/* 0x6D37, fase 2. Senza il bit 3 di (IX+1) prende due oggetti liberi
 * (sub_7379 due volte, indici in (IX+0E) e (IX+0F)) e fa comparire il
 * secondo accanto all'oggetto IY (sub_4af6, A = 45). Col bit 3 attende
 * (IX+0D) >= 24, accende il bit 2 di (IX+1) e il bit 7 del primo oggetto
 * (suono 2C); poi, a (IX+0D) >= 30, spegne il bit 0 del primo oggetto e
 * passa alla fase 4. */
void sub_6d37(struct z80 *z)
{
    if (M(z->ix + 0x01) & 0x08) {              /* 6D37-6D3B */
        z->a = M(z->ix + 0x0d);                /* 6D3D */
        if (z->a < 0x24)                       /* 6D40-6D42 */
            return;
        if (!(M(z->ix + 0x01) & 0x04)) {       /* 6D43-6D47 */
            M(z->ix + 0x01) |= 0x04;           /* 6D49 */
            z->a = M(z->ix + 0x0e);            /* 6D4D */
            portato(z);                        /* 6D50-6D54 */
            scrivi(HL(z), leggi(HL(z)) | 0x80);    /* 6D55 */
            z->c = 0x2c;                       /* 6D57 */
            sub_0028(z);                       /* 6D59 rst 28 */
            return;
        }
        if (z->a < 0x30)                       /* 6D5B-6D5D */
            return;
        z->a = M(z->ix + 0x0e);                /* 6D5E */
        portato(z);                            /* 6D61-6D65 */
        scrivi(HL(z), leggi(HL(z)) & ~0x01);   /* 6D66 */
        M(z->iy + 0x1d) = 0x04;                /* 6D68 */
        M(z->iy + 0x1e) &= ~0x02;              /* 6D6C */
        M(z->ix + 0x01) &= ~0x04;              /* 6D70 */
        M(z->ix + 0x0d) = 0x00;                /* 6D74 */
        return;
    }
    sub_7379(z);                               /* 6D79 */
    if (!NEMICI_RIPORTO_ACCESO(z))             /* 6D7C ret nc */
        return;
    M(z->ix + 0x0e) = z->a;                    /* 6D7D */
    M(HL(z)) = 0x80;                           /* 6D80 */
    uint16_t primo = HL(z);                    /* 6D82 ex de,hl */
    SET_HL(z, DE(z));
    SET_DE(z, primo);
    sub_7379(z);                               /* 6D83 */
    if (!NEMICI_RIPORTO_ACCESO(z)) {           /* 6D86 jr c */
        z->a = 0;                              /* 6D88 */
        M(DE(z)) = z->a;                       /* 6D89 */
        return;
    }
    M(z->ix + 0x01) |= 0x08;                   /* 6D8B */
    M(HL(z)) = 0x80;                           /* 6D8F */
    M(z->ix + 0x0f) = z->a;                    /* 6D91 */
    M(z->ix + 0x0d) = 0x00;                    /* 6D94 */
    z->a = (uint8_t)((0x08 + M(z->iy + 0x0e)) & 0xf0);  /* 6D98-6D9D */
    z->b = z->a;                               /* 6D9F */
    z->a = (M(z->iy + 0x1e) & 0x01) ? 0xf0 : 0x10;      /* 6DA0-6DA8 */
    z->a = (uint8_t)(z->a + M(z->iy + 0x15));  /* 6DAA */
    z->c = z->a;                               /* 6DAD */
    z->a = M(z->ix + 0x0e);                    /* 6DAE */
    SET_HL(z, PORTATI_48AF);                   /* 6DB1 */
    sub_0008(z);                               /* 6DB4 rst 8 */
    sub_0010(z);                               /* 6DB5 rst 10 */
    uint16_t ix0 = z->ix;                      /* 6DB6 push de; ex (sp),ix */
    z->ix = DE(z);
    M(z->ix + 0x0e) = z->b;                    /* 6DB9 */
    M(z->ix + 0x15) = z->c;                    /* 6DBC */
    z->c = M(z->iy + 0x1e);                    /* 6DBF */
    z->b = 0x05;                               /* 6DC2 */
    z->e = 0x01;                               /* 6DC4 */
    z->a = 0x45;                               /* 6DC6 */
    sub_4af6(z);                               /* 6DC8 */
    z->ix = ix0;                               /* 6DCB pop ix */
}

/* 0x6DCE, fase 4. Senza il bit 1 di (IX+1) lo accende e, se la cella
 * davanti al primo oggetto ((IX+0F) nella tabella 48AF) ha un blocco
 * (80-f7), ve lo posa (sub_4938). Col bit 1 attende (IX+0D) >= 15: accende
 * il bit 2 di (IX+1), il bit 0 dell'oggetto (IX+0E) e libera (IX+0F)
 * (sub_6ec1); a (IX+0D) >= 29 inverte i bit 1-3 di (IX+1), libera i due
 * oggetti (sub_6eb7) e passa alla fase 3. */
void sub_6dce(struct z80 *z)
{
    if (M(z->ix + 0x01) & 0x02) {              /* 6DCE-6DD2 */
        z->a = M(z->ix + 0x0d);                /* 6DD4 */
        if (z->a < 0x15)                       /* 6DD7-6DD9 */
            return;
        if (!(M(z->ix + 0x01) & 0x04)) {       /* 6DDA-6DDE */
            M(z->ix + 0x01) |= 0x04;           /* 6DE0 */
            z->a = M(z->ix + 0x0e);            /* 6DE4 */
            portato(z);                        /* 6DE7-6DEB */
            scrivi(HL(z), leggi(HL(z)) | 0x01);    /* 6DEC */
            z->a = M(z->ix + 0x0f);            /* 6DEE */
            sub_6ec1(z);                       /* 6DF1 */
            return;
        }
        if (z->a < 0x29)                       /* 6DF5-6DF7 */
            return;
        z->a = (uint8_t)(0x0e ^ M(z->ix + 0x01));  /* 6DF8-6DFA */
        M(z->ix + 0x01) = z->a;                /* 6DFD */
        z->a = M(z->ix + 0x0e);                /* 6E00 */
        sub_6eb7(z);                           /* 6E03 */
        z->a = M(z->ix + 0x0f);                /* 6E06 */
        sub_6eb7(z);                           /* 6E09 */
        M(z->iy + 0x1d) = 0x03;                /* 6E0C */
        return;
    }
    M(z->ix + 0x01) |= 0x02;                   /* 6E11 */
    z->a = M(z->ix + 0x0f);                    /* 6E15 */
    SET_HL(z, PORTATI_48AF);                   /* 6E18 */
    sub_0008(z);                               /* 6E1B rst 8 */
    sub_0010(z);                               /* 6E1C rst 10 */
    uint16_t oggetto = DE(z);                  /* 6E1D push de */
    z->a = 0x18;                               /* 6E1E */
    z->b = M(z->iy + 0x1e);                    /* 6E20 */
    if (z->b & 0x01)                           /* 6E23-6E25 */
        z->a = 0xf8;                           /* 6E27 */
    z->l = (uint8_t)(z->a + M(z->iy + 0x15));  /* 6E29-6E2C */
    z->h = (uint8_t)((0x08 + M(z->iy + 0x0e)) & 0xf0);  /* 6E2D-6E34 */
    sub_36f0(z);                               /* 6E35 */
    z->a = M(HL(z));                           /* 6E38 */
    if (!(z->a & 0x80) || z->a >= 0xf8) {      /* 6E39-6E41 */
        SET_DE(z, oggetto);                    /* 6E3D pop de */
        return;
    }
    uint16_t ix0 = z->ix;                      /* 6E43 ex (sp),ix */
    z->ix = oggetto;
    SET_HL(z, POSA_C08A);                      /* 6E45 */
    M(POSA_C08A) = 0x00;                       /* 6E48 */
    sub_4938(z);                               /* 6E4A */
    z->a = 0xff;                               /* 6E4D */
    M(0xc835) = z->a;                          /* 6E4F */
    z->ix = ix0;                               /* 6E52 pop ix */
}

/* 0x6E55, fase 5. Vicino a Dana (sub_6e90 con HL = 0816, riporto): la
 * direzione segue il segno di (IX+5) e fase 2 (6E83, 6E8B). Altrimenti,
 * a (IY+12) nullo, guarda la cella davanti (sub_5c32): libera, (IY+7) =
 * ff; piena col bit 0, inverte la direzione e fase 3; senza, fase 2. */
void sub_6e55(struct z80 *z)
{
    SET_HL(z, 0x0816);                         /* 6E55 */
    sub_6e90(z);                               /* 6E58 */
    if (NEMICI_RIPORTO_ACCESO(z)) {            /* 6E5B jr c,6e83 */
        z->a = (uint8_t)((z->c << 1) | (z->c >> 7));   /* 6E83-6E84 rlca */
        z->a = (uint8_t)(~z->a & 0x01);        /* 6E85-6E86 */
        M(z->iy + 0x1e) = z->a;                /* 6E88 */
        M(z->iy + 0x1d) = 0x02;                /* 6E8B */
        return;
    }
    z->a = M(z->iy + 0x12);                    /* 6E5D */
    if (z->a)                                  /* 6E60-6E61 */
        return;
    sub_5c32(z);                               /* 6E62 */
    SET_HL(z, CLASSI_C080);                    /* 6E65 */
    z->a = M(CLASSI_C080);                     /* 6E68 */
    if (!(z->a & 0x02)) {                      /* 6E69-6E6B */
        M(z->iy + 0x07) = 0xff;                /* 6E6D */
        return;
    }
    if (!(z->a & 0x01)) {                      /* 6E72-6E74 */
        M(z->iy + 0x1d) = 0x02;                /* 6E8B */
        return;
    }
    z->a = (uint8_t)(0x01 ^ M(z->iy + 0x1e));  /* 6E76-6E78 */
    M(z->iy + 0x1e) = z->a;                    /* 6E7B */
    M(z->iy + 0x1d) = 0x03;                    /* 6E7E */
}

/* 0x6E90: riporto se |2·(IX+5)| < L e (IX+4) < H (Dana vicina). C =
 * (IX+5); A = il doppio col segno tolto oppure (IX+4). */
void sub_6e90(struct z80 *z)
{
    z->a = M(z->ix + 0x05);                    /* 6E90 */
    z->c = z->a;                               /* 6E93 */
    int negativo = z->a & 0x80;                /* 6E94-6E96 or a; rla */
    z->a = (uint8_t)(z->a << 1);
    if (negativo)
        z->a = (uint8_t)(0 - z->a);            /* 6E98 neg */
    if (z->a >= z->l) {                        /* 6E9A-6E9B cp l; ret nc */
        NEMICI_PONI_RIPORTO(z, 0);
        return;
    }
    z->a = M(z->ix + 0x04);                    /* 6E9C */
    NEMICI_PONI_RIPORTO(z, z->a < z->h);       /* 6E9F cp h */
}

/* 0x6EA1, fase 6: col bit 3 di (IX+1) libera i due oggetti (IX+0E) e
 * (IX+0F) (sub_6eb7) e spegne il bit. */
void sub_6ea1(struct z80 *z)
{
    if (!(M(z->ix + 0x01) & 0x08))             /* 6EA1-6EA5 */
        return;
    z->a = M(z->ix + 0x0e);                    /* 6EA6 */
    sub_6eb7(z);                               /* 6EA9 */
    z->a = M(z->ix + 0x0f);                    /* 6EAC */
    sub_6eb7(z);                               /* 6EAF */
    M(z->ix + 0x01) &= ~0x08;                  /* 6EB2 */
}

/* ------------------------------------------------------------------ */
/* Tipo 8: destinazioni del `rst 20h` a 6F1C. All'ingresso A = indice
 * delle celle piene (70FE), B = direzioni ammesse (70EE), C = direzione
 * (IX+12), D = y, E = x (nuove), HL = 7056. Escono con la direzione in C
 * e il riporto (letto da `rl c` a 7056): acceso = direzione cambiata. */

/* 0x70B0: confronta il nibble basso di D con B >> 4. Uguali, o diversi
 * dal lato giusto per il bit 0 di C: allinea D (nibble basso = B >> 4) e
 * riporto acceso. Altrimenti bit 7 di C e riporto spento. */
void sub_70b0(struct z80 *z)
{
    z->a = z->d & 0x0f;                        /* 70B0-70B1 */
    z->h = z->a;                               /* 70B3 */
    z->a = (uint8_t)((z->b & 0xf0) >> 4);      /* 70B4-70BA */
    if (z->a != z->h) {                        /* 70BB-70BC */
        int riporto = z->a < z->h;
        if (z->c & 0x01)                       /* 70BE-70C2 ccf */
            riporto = !riporto;
        z->c |= 0x80;                          /* 70C3 */
        if (!riporto) {                        /* 70C5 ret nc */
            NEMICI_PONI_RIPORTO(z, 0);
            return;
        }
    }
    z->h = z->a;                               /* 70C6 */
    z->a = (uint8_t)((z->d & 0xf0) | z->h);    /* 70C7-70CA */
    z->d = z->a;                               /* 70CB */
    NEMICI_PONI_RIPORTO(z, 1);                 /* 70CC scf */
}

/* 0x70CE: come 70B0 sull'asse x: (E + 8) & 0f contro B & 0f; allinea E
 * con il nibble basso (B + 8) & 0f. */
void sub_70ce(struct z80 *z)
{
    z->a = (uint8_t)((z->e + 0x08) & 0x0f);    /* 70CE-70D1 */
    z->l = z->a;                               /* 70D3 */
    z->a = z->b & 0x0f;                        /* 70D4-70D5 */
    if (z->a != z->l) {                        /* 70D7-70D8 */
        int riporto = z->a < z->l;
        if (z->c & 0x01)                       /* 70DA-70DE ccf */
            riporto = !riporto;
        z->c |= 0x80;                          /* 70DF */
        if (!riporto) {                        /* 70E1 ret nc */
            NEMICI_PONI_RIPORTO(z, 0);
            return;
        }
    }
    z->a = (uint8_t)((z->a + 0x08) & 0x0f);    /* 70E2-70E4 */
    z->l = z->a;                               /* 70E6 */
    z->a = (uint8_t)((z->e & 0xf0) | z->l);    /* 70E7-70EA */
    z->e = z->a;                               /* 70EB */
    NEMICI_PONI_RIPORTO(z, 1);                 /* 70EC scf */
}

/* 0x70A8: sub_70b0 e, se dà riporto, sub_70ce. */
void sub_70a8(struct z80 *z)
{
    sub_70b0(z);                               /* 70A8 */
    if (!NEMICI_RIPORTO_ACCESO(z))             /* 70AB ret nc */
        return;
    sub_70ce(z);                               /* 70AC */
}

/* `call 70a8; ret nc; ld a,c; add a,passo; and 3; ld c,a; scf; ret`. */
static void gira(struct z80 *z, uint8_t passo)
{
    sub_70a8(z);
    if (!NEMICI_RIPORTO_ACCESO(z))
        return;
    z->a = (uint8_t)((z->c + passo) & 0x03);
    z->c = z->a;
    NEMICI_PONI_RIPORTO(z, 1);
}

/* 0x6F49 (6F1C[0], [15]): `or a; ret`, direzione invariata. */
void sub_6f49(struct z80 *z)
{
    NEMICI_PONI_RIPORTO(z, 0);                 /* 6F49 or a */
}

/* 0x6F4B (6F1C[1]): con (C + 1) & 2 nullo sub_6f91, altrimenti gira di 2. */
void sub_6f4b(struct z80 *z)
{
    z->a = (uint8_t)((z->c + 1) & 0x02);       /* 6F4B-6F4D */
    if (!z->a) sub_6f91(z);                    /* 6F4F jp z */
    else       gira(z, 2);                     /* 6F52-6F5D */
}

/* 0x6F5E (6F1C[2]): col bit 0 di C spento sub_6f91, altrimenti gira di 1. */
void sub_6f5e(struct z80 *z)
{
    if (!(z->c & 0x01)) sub_6f91(z);           /* 6F5E-6F60 jp z */
    else                gira(z, 1);            /* 6F63-6F6D */
}

/* 0x6F6E (6F1C[4]): con (C + 1) & 2 acceso sub_6f91, altrimenti gira di 2. */
void sub_6f6e(struct z80 *z)
{
    z->a = (uint8_t)((z->c + 1) & 0x02);       /* 6F6E-6F70 */
    if (z->a) sub_6f91(z);                     /* 6F72 jp nz */
    else      gira(z, 2);                      /* 6F75-6F80 */
}

/* 0x6F81 (6F1C[8]): col bit 0 di C acceso sub_6f91, altrimenti gira di -1. */
void sub_6f81(struct z80 *z)
{
    if (z->c & 0x01) sub_6f91(z);              /* 6F81-6F83 jp nz */
    else             gira(z, 0xff);            /* 6F86-6F90 dec a */
}

/* 0x6FB3: colonna di D (nibble basso) contro B >> 4; diverse: bit 7 di C. */
void sub_6fb3(struct z80 *z)
{
    z->a = z->d & 0x0f;                        /* 6FB3-6FB4 */
    z->h = z->a;                               /* 6FB6 */
    z->a = (uint8_t)((z->b & 0xf0) >> 4);      /* 6FB7-6FBD */
    NEMICI_PONI_RIPORTO(z, z->a < z->h);       /* 6FBE cp h */
    if (z->a != z->h)                          /* 6FBF ret z */
        z->c |= 0x80;                          /* 6FC0 */
}

/* 0x6FC3: allinea il nibble basso di D a B >> 4. */
static void sub_6fc3(struct z80 *z)
{
    z->a = (uint8_t)((z->b & 0xf0) >> 4);      /* 6FC3-6FC9 */
    z->h = z->a;                               /* 6FCA */
    z->a = (uint8_t)((z->d & 0xf0) | z->h);    /* 6FCB-6FCE */
    z->d = z->a;                               /* 6FCF */
}

/* 0x6FD1: (E + 8) & 0f contro B & 0f; diversi: bit 7 di C. */
void sub_6fd1(struct z80 *z)
{
    z->a = (uint8_t)((z->e + 0x08) & 0x0f);    /* 6FD1-6FD4 */
    z->l = z->a;                               /* 6FD6 */
    z->a = z->b & 0x0f;                        /* 6FD7-6FD8 */
    NEMICI_PONI_RIPORTO(z, z->a < z->l);       /* 6FDA cp l */
    if (z->a != z->l)                          /* 6FDB ret z */
        z->c |= 0x80;                          /* 6FDC */
}

/* 0x6FDF: allinea il nibble basso di E a (B & 0f) + 8. */
static void sub_6fdf(struct z80 *z)
{
    z->a = (uint8_t)(((z->b & 0x0f) + 0x08) & 0x0f);  /* 6FDF-6FE4 */
    z->l = z->a;                               /* 6FE6 */
    z->a = (uint8_t)((z->e & 0xf0) | z->l);    /* 6FE7-6FEA */
    z->e = z->a;                               /* 6FEB */
}

/* 0x6F91: col bit 1 di C (asse y) sub_6fb3, altrimenti sub_6fd1; se hanno
 * acceso il bit 7 di C allinea l'altro asse (6FDF, 6FC3) e la direzione
 * diventa 0 o 2 (riporto acceso); altrimenti riporto spento. */
void sub_6f91(struct z80 *z)
{
    if (z->c & 0x02) {                         /* 6F91-6F93 */
        sub_6fb3(z);                           /* 6F95 */
        if (z->c & 0x80) {                     /* 6F98-6F9A */
            sub_6fdf(z);                       /* 6F9C */
            z->c = 0x00;                       /* 6F9F */
            NEMICI_PONI_RIPORTO(z, 1);         /* 6FA1 scf */
            return;
        }
    } else {
        sub_6fd1(z);                           /* 6FA3 */
        if (z->c & 0x80) {                     /* 6FA6-6FA8 */
            sub_6fc3(z);                       /* 6FAA */
            z->c = 0x02;                       /* 6FAD */
            NEMICI_PONI_RIPORTO(z, 1);         /* 6FAF scf */
            return;
        }
    }
    NEMICI_PONI_RIPORTO(z, 0);                 /* 6FB1 or a */
}

/* 0x6FED (6F1C[3], [12]): col bit 1 di C spento, se sub_70b0 dà riporto
 * la direzione diventa 2. */
void sub_6fed(struct z80 *z)
{
    if (z->c & 0x02)                           /* 6FED-6FEF ret nz */
        return;
    sub_70b0(z);                               /* 6FF0 */
    if (!NEMICI_RIPORTO_ACCESO(z))             /* 6FF3 ret nc */
        return;
    z->c = 0x02;                               /* 6FF4 */
}

/* 0x6FF7 (6F1C[6], [9]): col bit 1 di C acceso, se sub_70ce dà riporto
 * la direzione diventa 0. */
void sub_6ff7(struct z80 *z)
{
    if (!(z->c & 0x02))                        /* 6FF7-6FF9 ret z */
        return;
    sub_70ce(z);                               /* 6FFA */
    if (!NEMICI_RIPORTO_ACCESO(z))             /* 6FFD ret nc */
        return;
    z->c = 0x00;                               /* 6FFE */
}

/* 7015, 7025, 7036, 7045: A = condizione; nulla (7015, 7025) o accesa
 * (7036, 7045) esce col riporto spento (`ret nz`/`ret z` dopo `and`),
 * altrimenti gira di passo. */
static void gira_se(struct z80 *z, int esce, uint8_t passo)
{
    if (esce) NEMICI_PONI_RIPORTO(z, 0);
    else      gira(z, passo);
}

/* 0x7015 (e da 7001): col bit 0 di C spento gira di 3. */
void sub_7015(struct z80 *z)
{
    z->a = 0x01 & z->c;                        /* 7015-7017 */
    gira_se(z, z->a != 0, 3);                  /* 7018 ret nz; 7019-7024 */
}

/* 0x7025 (e da 700B): con (C + 1) & 2 nullo gira di 2. */
void sub_7025(struct z80 *z)
{
    z->a = (uint8_t)((z->c + 1) & 0x02);       /* 7025-7027 */
    gira_se(z, z->a != 0, 2);                  /* 7029 ret nz; 702A-7035 */
}

/* 0x7036 (e da 7001): col bit 0 di C acceso gira di 1. */
void sub_7036(struct z80 *z)
{
    z->a = 0x01 & z->c;                        /* 7036-7038 */
    gira_se(z, z->a == 0, 1);                  /* 7039 ret z; 703A-7044 */
}

/* 0x7045 (e da 700B): con (C + 1) & 2 acceso gira di 2. */
void sub_7045(struct z80 *z)
{
    z->a = (uint8_t)((z->c + 1) & 0x02);       /* 7045-7047 */
    gira_se(z, z->a == 0, 2);                  /* 7049 ret z; 704A-7055 */
}

/* 0x7001 (6F1C[5]): B = 44 e 7015 col bit 3 di D spento, B = cc e 7036
 * altrimenti. */
void sub_7001(struct z80 *z)
{
    if (!(z->d & 0x08)) { z->b = 0x44; sub_7015(z); }  /* 7001-7005 */
    else                { z->b = 0xcc; sub_7036(z); }  /* 7007-7009 */
}

/* 0x700B (6F1C[10]): B = 4c e 7025 col bit 3 di D spento, B = c4 e 7045
 * altrimenti. */
void sub_700b(struct z80 *z)
{
    if (!(z->d & 0x08)) { z->b = 0x4c; sub_7025(z); }  /* 700B-700F */
    else                { z->b = 0xc4; sub_7045(z); }  /* 7011-7013 */
}

/* 0x70FE: le quattro celle della griglia attorno al punto (D, E): B =
 * bit 1 se y + 2 e y + 0d cadono in righe diverse, bit 0 se x - 6 e
 * x + 5 cadono in colonne diverse; poi, dalla cella di (D + 2, E + 2)
 * (sub_36f0), il bit 7 di ogni cella letta entra in B: la cella, la
 * vicina a destra (se le colonne sono due), quella sopra (se le righe sono
 * due) e la sua vicina. Uscita A = B & 0f. */
void sub_70fe(struct z80 *z)
{
    z->b = 0x00;                               /* 70FE */
    z->a = (uint8_t)(z->d + 0x02);             /* 7100-7101 */
    z->h = z->a;                               /* 7103 */
    z->a = (uint8_t)(((uint8_t)(z->a + 0x0b) ^ z->h) & 0xf0);  /* 7104-7107 */
    if (z->a)                                  /* 7109 */
        z->b |= 0x02;                          /* 710B */
    z->a = (uint8_t)(z->e - 0x08 + 0x02);      /* 710D-7110 */
    z->l = z->a;                               /* 7112 */
    z->a = (uint8_t)(((uint8_t)(z->a + 0x0b) ^ z->l) & 0xf0);  /* 7113-7116 */
    if (z->a)                                  /* 7118 */
        z->b++;                                /* 711A */
    z->l = (uint8_t)(0x08 + z->l);             /* 711B-711E */
    sub_36f0(z);                               /* 711F */
    uint16_t hl = HL(z);
    z->b = (uint8_t)((z->b << 1) | (M(hl) >> 7));      /* 7122-7124 */
    if (z->b & 0x02)                           /* 7126-7128 */
        hl++;                                  /* 712A */
    z->b = (uint8_t)((z->b << 1) | (M(hl) >> 7));      /* 712B-712D */
    if (z->b & 0x08)                           /* 712F-7131 */
        hl -= 0x10;                            /* 7133-7139 */
    z->b = (uint8_t)((z->b << 1) | (M(hl) >> 7));      /* 713A-713C */
    if (z->b & 0x08)                           /* 713E-7140 */
        hl--;                                  /* 7142 */
    z->b = (uint8_t)((z->b << 1) | (M(hl) >> 7));      /* 7143-7145 */
    SET_HL(z, hl);
    z->a = 0x0f & z->b;                        /* 7147-7149 */
}

/* Coda 7056-70A7, raggiunta dal `ret` della destinazione di 6F1C: la
 * direzione (C << 1 col riporto) dà la velocità (c1a0, negata col bit 1)
 * su (IX+0E..0F) o (IX+10..11) secondo il bit 2; posizione D, E in
 * (IY+0E), (IY+15); (IX+12) = nuova direzione; la frazione B (vecchio
 * (IX+0E)) va in (IY+0D) o (IY+14), o entrambe a zero se è cambiata. */
static void coda_7056(struct z80 *z)
{
    z->c = (uint8_t)((z->c << 1) | (NEMICI_RIPORTO_ACCESO(z) ? 1 : 0));  /* 7056 rl c */
    z->b = M(z->ix + 0x0e);                    /* 7058 */
    uint16_t v = (uint16_t)(M(VELOCITA_C1A0) | (M(VELOCITA_C1A0 + 1) << 8));  /* 705B */
    if (z->c & 0x02)                           /* 705E-7060 */
        v = (uint16_t)(0 - v);                 /* 7062-7068 */
    SET_HL(z, v);
    z->a = 0;                                  /* 7069 */
    if (z->c & 0x04) {                         /* 706A-706C */
        M(z->ix + 0x0e) = z->a;                /* 706E */
        M(z->ix + 0x0f) = z->a;                /* 7071 */
        M(z->ix + 0x10) = z->l;                /* 7074 */
        M(z->ix + 0x11) = z->h;                /* 7077 */
    } else {
        M(z->ix + 0x0e) = z->l;                /* 707C */
        M(z->ix + 0x0f) = z->h;                /* 707F */
        M(z->ix + 0x10) = z->a;                /* 7082 */
        M(z->ix + 0x11) = z->a;                /* 7085 */
    }
    M(z->iy + 0x0e) = z->d;                    /* 7088 */
    M(z->iy + 0x15) = z->e;                    /* 708B */
    int uscito = z->c & 0x01;                  /* 708E srl c */
    z->c >>= 1;
    M(z->ix + 0x12) = z->c;                    /* 7090 */
    NEMICI_PONI_RIPORTO(z, uscito);
    if (uscito) {                              /* 7093 jr c */
        M(z->iy + 0x14) = z->a;                /* 70A1 */
        M(z->iy + 0x0d) = z->a;                /* 70A4 */
    } else if (z->c & 0x02) {                  /* 7095-7097 */
        M(z->iy + 0x14) = z->b;                /* 7099 */
    } else {
        M(z->iy + 0x0d) = z->b;                /* 709D */
    }
}

static void salta_6f1c(struct z80 *z, uint16_t dest)
{
    switch (dest) {
    case 0x6F49: sub_6f49(z); break;
    case 0x6F4B: sub_6f4b(z); break;
    case 0x6F5E: sub_6f5e(z); break;
    case 0x6FED: sub_6fed(z); break;
    case 0x6F6E: sub_6f6e(z); break;
    case 0x7001: sub_7001(z); break;
    case 0x6FF7: sub_6ff7(z); break;
    case 0x7015: sub_7015(z); break;
    case 0x6F81: sub_6f81(z); break;
    case 0x700B: sub_700b(z); break;
    case 0x7025: sub_7025(z); break;
    case 0x7036: sub_7036(z); break;
    case 0x7045: sub_7045(z); break;
    default:     nemici_manca(dest); break;
    }
}

/* 0x6ECA: tipo 8. Velocità da 6F3D[(IY+4) - 16] in c1a0; nuova posizione
 * = (IY+0D..0E) + (IX+0E..0F) oppure, col bit 1 della direzione (IX+12),
 * (IY+14..15) + (IX+10..11); la frazione va in (IX+0E). Celle attorno
 * (sub_70fe), direzioni ammesse da 70EE e smistamento con `rst 20h` a
 * 6F1C (16 voci); poi la coda 7056. */
void sub_6eca(struct z80 *z)
{
    z->a = (uint8_t)(M(z->iy + 0x04) - 0x16);  /* 6ECA-6ECD */
    SET_HL(z, VELOCITA_6F3D);                  /* 6ECF */
    sub_0008(z);                               /* 6ED2 rst 8 */
    sub_0010(z);                               /* 6ED3 rst 10 */
    M(VELOCITA_C1A0) = z->e;                   /* 6ED4 ld (c1a0),de */
    M(VELOCITA_C1A0 + 1) = z->d;
    z->a = M(z->ix + 0x12);                    /* 6ED8 */
    uint16_t yy = (uint16_t)(M(z->iy + 0x0d) | (M(z->iy + 0x0e) << 8));  /* 6EDB-6EDE */
    uint16_t xx = (uint16_t)(M(z->iy + 0x14) | (M(z->iy + 0x15) << 8));  /* 6EE1-6EE4 */
    uint8_t frazione;
    if (z->a & 0x02) {                         /* 6EE7-6EE9 */
        z->c = M(z->ix + 0x10);                /* 6EEB */
        z->b = M(z->ix + 0x11);                /* 6EEE */
        xx = (uint16_t)(xx + BC(z));           /* 6EF1-6EF3 */
        frazione = (uint8_t)xx;                /* 6F01 ld l,e */
    } else {
        z->c = M(z->ix + 0x0e);                /* 6EF6 */
        z->b = M(z->ix + 0x0f);                /* 6EF9 */
        yy = (uint16_t)(yy + BC(z));           /* 6EFC */
        frazione = (uint8_t)yy;
    }
    M(z->ix + 0x0e) = frazione;                /* 6F02 */
    /* 6F05 ld l,d; 6F07 ex de,hl: DE = (y, x), HL = parola x */
    SET_DE(z, (uint16_t)((yy & 0xff00) | (xx >> 8)));
    SET_HL(z, xx);
    z->c = z->a;                               /* 6F06 */
    uint16_t de = DE(z);                       /* 6F08 push de */
    sub_70fe(z);                               /* 6F09 */
    SET_DE(z, de);                             /* 6F0C pop de */
    z->b = z->a;                               /* 6F0D */
    SET_HL(z, (uint16_t)(DIREZIONI_70EE + z->a));  /* 6F0E-6F15 */
    z->a = z->b;                               /* 6F16 */
    z->b = leggi(HL(z));                       /* 6F17 */
    SET_HL(z, CODA_7056);                      /* 6F18 ld hl,7056; push hl */
    uint16_t dest = nemici_rst20(z, 0x6F1C, rst20_6F1C, 16);
    NEMICI_PONI_RIPORTO(z, 0);                 /* rst 20: add hl,de spegne il riporto */
    if (!dest)                                 /* fuori tabella, già registrato */
        return;
    salta_6f1c(z, dest);                       /* 6F1C rst 20 */
    coda_7056(z);                              /* ret della destinazione a 7056 */
}
