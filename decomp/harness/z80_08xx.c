/* Routine del Z80 principale fra 0x0800 e 0x0fff: lo smistamento d'urto
 * 08A2 e i gestori della tabella inline 08AE (lotto 7, compito 11).
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). IX = oggetto (slot da 0x30 byte a c46e). */
#include <stddef.h>
#include <stdio.h>

#include "routine.h"

/* Campi dell'oggetto usati dai gestori d'urto (offsetti da IX).
 * +0D/+0E e +14/+15 sono le posizioni (sottopixel, pixel) Y e X;
 * +0B/+0C e +12/+13 le velocità; +16..+19 i margini del riquadro
 * copiati da 0602; +1D lo stato, +1E i bit di direzione. */
#define OGG_SOTTO_Y   0x0d
#define OGG_Y         0x0e
#define OGG_SOTTO_X   0x14
#define OGG_X         0x15
#define OGG_VEL_Y     0x0b   /* parola +0B/+0C */
#define OGG_VEL_X     0x12   /* parola +12/+13 */
#define OGG_MARG_ALTO 0x16
#define OGG_MARG_SIN  0x17
#define OGG_MARG_DES  0x18
#define OGG_MARG_BAS  0x19
#define OGG_STATO     0x1d
#define OGG_DIR       0x1e

/* 0x08D8 (tabella 08AE, indici 0 e F): se lo stato è fra 3 e 6 (6 escluso)
 * e i bit 4-5 delle celle d'urto (C) sono spenti, lo stato diventa 6.
 * Uscita: A = stato, oppure C & 30 se si arriva al confronto. */
void sub_08d8(struct z80 *z)
{
    uint8_t a = M(z->ix + OGG_STATO);
    z->a = a;
    if (a >= 7 || a < 3 || a == 6)
        return;
    z->a = (uint8_t)(z->c & 0x30);
    if (z->a)
        return;
    M(z->ix + OGG_STATO) = 6;
}

/* 0x094D (tabella 08AE, indice 2): spinge l'oggetto fuori dalla cella
 * urtata sul lato di minor penetrazione: corregge X (IX+15) o Y (IX+0E)
 * dei margini residui, azzera i sottopixel (e la velocità X nel ramo X),
 * stato 2 (ramo X) o 1/2 dal bit 7 di (IX+0C) (ramo Y), direzione aggiornata.
 * Uscita: A = correzione o stato; D/E = X e penetrazione X; H/L = Y e
 * penetrazione Y. */
void sub_094d(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + 0x02) = 0;
    uint8_t h = M(ix + OGG_Y);
    uint8_t l = (uint8_t)((h & 0x0f) + M(ix + OGG_MARG_ALTO)
                          + M(ix + OGG_MARG_BAS) - 0x0f);
    uint8_t d = M(ix + OGG_X);
    uint8_t e = (uint8_t)(d - (uint8_t)(((uint8_t)(d - 8) & 0xf0) + 8));
    e = (uint8_t)(e + M(ix + OGG_MARG_SIN) + M(ix + OGG_MARG_DES) - 0x0f);
    z->h = h; z->l = l;
    z->d = d; z->e = e;
    if (e < l) {                       /* penetrazione X minore: corregge X */
        z->a = (uint8_t)(d - e);
        M(ix + OGG_X) = z->a;
        M(ix + OGG_SOTTO_X) = 0;
        M(ix + OGG_VEL_X) = 0;
        M(ix + OGG_VEL_X + 1) = 0;
        M(ix + OGG_STATO) = 2;
        M(ix + OGG_DIR) = 0;
    } else {                           /* corregge Y */
        M(ix + OGG_Y) = (uint8_t)(h - l);
        M(ix + OGG_SOTTO_Y) = 0;
        z->a = (M(ix + 0x0c) & 0x80) ? 2 : 1;
        M(ix + OGG_STATO) = z->a;
        M(ix + OGG_DIR) &= (uint8_t)~0x02;
    }
}

/* 0x08ED (tabella 08AE, indice 1): come 094D con la cella urtata a
 * destra: la penetrazione X è la distanza dal bordo sinistro della
 * colonna seguente meno il margine sinistro, e X si corregge in avanti
 * (stato 2, direzione 1); il ramo Y è quello di 094D. Prima esecuzione
 * nel replay fra i frame 2894 e 3027 (brief 26).
 * Uscita: A = X corretta o stato; D/E = X e penetrazione X; H/L = Y e
 * penetrazione Y. */
void sub_08ed(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + 0x02) = 0;                          /* 08ED */
    uint8_t h = M(ix + OGG_Y);                 /* 08F1 */
    uint8_t l = (uint8_t)((h & 0x0f) + M(ix + OGG_MARG_ALTO)
                          + M(ix + OGG_MARG_BAS) - 0x0f);   /* 08F4-0903 */
    uint8_t d = M(ix + OGG_X);                 /* 0904 */
    uint8_t e = (uint8_t)((((uint8_t)(d - 8) & 0xf0) + 8 + 0x10) - d
                          - M(ix + OGG_MARG_SIN));          /* 0907-0914 */
    z->h = h; z->l = l;
    z->d = d; z->e = e;
    if (e < l) {                               /* 0915-0916: corregge X */
        z->a = (uint8_t)(d + e);               /* 0918-0919 */
        M(ix + OGG_X) = z->a;
        M(ix + OGG_SOTTO_X) = 0;
        M(ix + OGG_VEL_X + 1) = 0;
        M(ix + OGG_VEL_X) = 0;
        M(ix + OGG_STATO) = 2;                 /* 0929 */
        M(ix + OGG_DIR) = 1;                   /* 092D */
    } else {                                   /* 0932: corregge Y */
        M(ix + OGG_Y) = (uint8_t)(h - l);
        M(ix + OGG_SOTTO_Y) = 0;
        z->a = (M(ix + 0x0c) & 0x80) ? 2 : 1;  /* 093B-0943 */
        M(ix + OGG_STATO) = z->a;
        M(ix + OGG_DIR) &= (uint8_t)~0x02;     /* 0948 */
    }
}

/* 0x0A98 (tabella 08AE, indice 3): appoggio sul pavimento: Y alla base
 * della riga meno i margini verticali, sottopixel azzerato; se lo stato
 * non è 2: stato 1, (IX+2) = 0, bit 1 della direzione spento.
 * Uscita: A = stato precedente, H = Y d'ingresso & f0. */
void sub_0a98(struct z80 *z)
{
    uint16_t ix = z->ix;
    uint8_t h = (uint8_t)(M(ix + OGG_Y) & 0xf0);
    z->h = h;
    M(ix + OGG_Y) = (uint8_t)(h + 0x10 - M(ix + OGG_MARG_ALTO)
                              - M(ix + OGG_MARG_BAS) - 1);
    M(ix + OGG_SOTTO_Y) = 0;
    z->a = M(ix + OGG_STATO);
    if (z->a == 2)
        return;
    M(ix + OGG_STATO) = 1;
    M(ix + 0x02) = 0;
    M(ix + OGG_DIR) &= (uint8_t)~0x02;
}

/* 0x0AC3 (tabella 08AE, indice C): urto col soffitto: Y alla riga sotto
 * meno il margine alto, sottopixel = f0; se stato - 4 non è 0, 1 o 3:
 * stato 1, bit 1 della direzione acceso, (IX+2) = 0; altrimenti azzera
 * (IX+0B)/(IX+0C). Uscita: A = stato - 4, H = Y d'ingresso. */
void sub_0ac3(struct z80 *z)
{
    uint16_t ix = z->ix;
    uint8_t h = M(ix + OGG_Y);
    z->h = h;
    M(ix + OGG_Y) = (uint8_t)((h & 0xf0) + 0x10 - M(ix + OGG_MARG_ALTO));
    M(ix + OGG_SOTTO_Y) = 0xf0;
    uint8_t a = (uint8_t)(M(ix + OGG_STATO) - 4);
    z->a = a;
    if (a < 2 || a == 3) {
        M(ix + 0x0c) = 0;
        M(ix + 0x0b) = 0;
    } else {
        M(ix + OGG_STATO) = 1;
        M(ix + OGG_DIR) |= 0x02;
        M(ix + 0x02) = 0;
    }
}

/* 0x0AF9 (tabella 08AE, indice 9): urto sul lato sinistro: X alla colonna
 * accanto meno il margine sinistro, sottopixel azzerato; se lo stato - 4 è
 * almeno 2: stato 2, direzione 1/3 dal bit 7 di (IX+0C), velocità X
 * azzerata; altrimenti (IX+7) = ff se il bit 0 della direzione è spento,
 * poi azzera velocità X e (IX+0B)/(IX+0C). Uscita: A = stato (o stato - 4),
 * C = direzione se scritta, D = X d'ingresso. */
void sub_0af9(struct z80 *z)
{
    uint16_t ix = z->ix;
    uint8_t d = M(ix + OGG_X);
    z->d = d;
    M(ix + OGG_X) = (uint8_t)((((uint8_t)(d - 8)) & 0xf0) + 8 + 0x10
                              - M(ix + OGG_MARG_SIN));
    M(ix + OGG_SOTTO_X) = 0;
    z->a = M(ix + OGG_STATO);
    if (z->a != 0) {
        z->a = (uint8_t)(z->a - 4);
        if (z->a >= 2) {
            M(ix + OGG_STATO) = 2;
            M(ix + 0x02) = 0;
            z->c = (M(ix + 0x0c) & 0x80) ? 1 : 3;
            M(ix + OGG_DIR) = z->c;
            M(ix + OGG_VEL_X) = 0;
            M(ix + OGG_VEL_X + 1) = 0;
            return;
        }
        if (!(M(ix + OGG_DIR) & 0x01))
            M(ix + 0x07) = 0xff;
    }
    M(ix + OGG_VEL_X) = 0;
    M(ix + OGG_VEL_X + 1) = 0;
    M(ix + 0x0b) = 0;
    M(ix + 0x0c) = 0;
}

/* 0x0B57 (tabella 08AE, indice 6): come 0AF9 sul lato destro: X alla
 * colonna meno i margini orizzontali, direzione 0/2 (bit 7 di (IX+0C)
 * acceso → 0), (IX+7) = ff se il bit 0 della direzione è acceso.
 * Uscita: A = stato (o stato - 4), C = direzione se scritta,
 * D = X allineato alla colonna. */
void sub_0b57(struct z80 *z)
{
    uint16_t ix = z->ix;
    uint8_t d = (uint8_t)((((uint8_t)(M(ix + OGG_X) - 8)) & 0xf0) + 8);
    z->d = d;
    M(ix + OGG_X) = (uint8_t)(0x10 - M(ix + OGG_MARG_SIN)
                              - M(ix + OGG_MARG_DES) - 1 + d);
    M(ix + OGG_SOTTO_X) = 0;
    z->a = M(ix + OGG_STATO);
    if (z->a != 0) {
        z->a = (uint8_t)(z->a - 4);
        if (z->a >= 2) {
            M(ix + OGG_STATO) = 2;
            M(ix + 0x02) = 0;
            z->c = (M(ix + 0x0c) & 0x80) ? 0 : 2;
            M(ix + OGG_DIR) = z->c;
            M(ix + OGG_VEL_X) = 0;
            M(ix + OGG_VEL_X + 1) = 0;
            return;
        }
        if (M(ix + OGG_DIR) & 0x01)
            M(ix + 0x07) = 0xff;
    }
    M(ix + 0x0b) = 0;
    M(ix + 0x0c) = 0;
    M(ix + OGG_VEL_X) = 0;
    M(ix + OGG_VEL_X + 1) = 0;
}

/* Colonna allineata dei gestori d'urto: ((X - 8) & f0) + 8. */
static uint8_t colonna(uint16_t ix)
{
    return (uint8_t)((((uint8_t)(M(ix + OGG_X) - 8)) & 0xf0) + 8);
}

/* Ramo Y comune a 09B3 e 0A24 (0A00/0A74): Y = H + L, sottopixel f0;
 * se stato - 4 è almeno 2 stato 1 e bit 1 della direzione acceso,
 * altrimenti azzera (IX+0B)/(IX+0C). Uscita: A = stato - 4. */
static void urto_y_sopra(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + OGG_Y) = (uint8_t)(z->h + z->l);
    M(ix + OGG_SOTTO_Y) = 0xf0;
    z->a = (uint8_t)(M(ix + OGG_STATO) - 4);
    if (z->a >= 2) {
        M(ix + OGG_STATO) = 1;
        M(ix + OGG_DIR) |= 0x02;
    } else {
        M(ix + 0x0b) = 0;
        M(ix + 0x0c) = 0;
    }
}

/* 0x09B3 (tabella 08AE, indice 8): cella urtata in alto a destra:
 * penetrazione Y dal bordo inferiore della riga (L), penetrazione X dal
 * bordo sinistro della colonna seguente (E). Con E < L corregge X in
 * avanti (stato 2, direzione 1 o 3 dal bit 7 di (IX+0C), velocità X
 * azzerata, HL = 8000), altrimenti il ramo Y di urto_y_sopra. Brief 26.
 * Uscita: A, D/E = X e penetrazione X, H/L (o 8000), C nel ramo X. */
void sub_09b3(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + 0x02) = 0;                          /* 09B3 */
    z->h = M(ix + OGG_Y);                      /* 09B7 */
    z->l = (uint8_t)((z->h & 0xf0) + 0x10 - z->h - M(ix + OGG_MARG_ALTO));  /* 09BA-09C3 */
    z->d = M(ix + OGG_X);                      /* 09C4 */
    z->e = (uint8_t)(colonna(ix) + 0x10 - z->d - M(ix + OGG_MARG_SIN));    /* 09C7-09D4 */
    if (z->e >= z->l) {                        /* 09D5-09D6 */
        urto_y_sopra(z);                       /* 09FF */
        return;
    }
    z->a = (uint8_t)(z->d + z->e);             /* 09D8-09D9 */
    M(ix + OGG_X) = z->a;
    M(ix + OGG_SOTTO_X) = 0;
    SET_HL(z, 0x8000);                         /* 09E1 */
    M(ix + OGG_STATO) = 2;                     /* 09E4 */
    z->c = (M(ix + 0x0c) & 0x80) ? 1 : 3;      /* 09E8-09F1 */
    M(ix + OGG_DIR) = z->c;
    M(ix + OGG_VEL_X) = 0;                     /* 09F6-09FA */
    M(ix + OGG_VEL_X + 1) = 0;
}

/* 0x0A24 (tabella 08AE, indice 4): come 09B3 con la cella a sinistra:
 * penetrazione X come in 094D, X corretta indietro, direzione 0 o 2.
 * Brief 26. */
void sub_0a24(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + 0x02) = 0;                          /* 0A24 */
    z->h = M(ix + OGG_Y);                      /* 0A28 */
    z->l = (uint8_t)((z->h & 0xf0) + 0x10 - z->h - M(ix + OGG_MARG_ALTO));  /* 0A2B-0A34 */
    z->d = M(ix + OGG_X);                      /* 0A35 */
    z->e = (uint8_t)(z->d - colonna(ix) + M(ix + OGG_MARG_SIN)
                     + M(ix + OGG_MARG_DES) - 0x0f);                       /* 0A38-0A4B */
    if (z->e >= z->l) {                        /* 0A4C-0A4D */
        urto_y_sopra(z);                       /* 0A73 */
        return;
    }
    z->a = (uint8_t)(z->d - z->e);             /* 0A4F-0A50 */
    M(ix + OGG_X) = z->a;
    M(ix + OGG_SOTTO_X) = 0;
    M(ix + OGG_STATO) = 2;                     /* 0A58 */
    z->c = (M(ix + 0x0c) & 0x80) ? 0 : 2;      /* 0A5C-0A65 */
    M(ix + OGG_DIR) = z->c;
    M(ix + OGG_VEL_X) = 0;                     /* 0A6A-0A6E */
    M(ix + OGG_VEL_X + 1) = 0;
}

/* 0x0BBB (tabella 08AE, indice 7; anche da 0CDA): angolo in basso a
 * destra: Y sulla riga meno i margini verticali, X sulla colonna meno i
 * margini orizzontali, sottopixel azzerati, stato 2, direzione 0,
 * (IX+2) = 0. Brief 26. Uscita: A = X corretta. */
void sub_0bbb(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + OGG_Y) = (uint8_t)((M(ix + OGG_Y) & 0xf0) + 0x10 - M(ix + OGG_MARG_ALTO)
                              - M(ix + OGG_MARG_BAS) - 1);                 /* 0BBB-0BC9 */
    M(ix + OGG_SOTTO_Y) = 0;                   /* 0BCC */
    z->a = (uint8_t)(colonna(ix) + 0x10 - M(ix + OGG_MARG_SIN)
                     - M(ix + OGG_MARG_DES) - 1);                          /* 0BD0-0BE1 */
    M(ix + OGG_X) = z->a;                      /* 0BE2 */
    M(ix + OGG_SOTTO_X) = 0;
    M(ix + OGG_STATO) = 2;                     /* 0BE9 */
    M(ix + OGG_DIR) = 0;                       /* 0BED */
    M(ix + 0x02) = 0;                          /* 0BF1 */
}

/* 0x0BF6 (tabella 08AE, indice B; anche da 0CCE): angolo in basso a
 * sinistra: Y come 0BBB, X sulla colonna seguente meno il margine
 * sinistro, stato 2, direzione 1, (IX+2) = 0. Brief 26.
 * Uscita: A = X corretta. */
void sub_0bf6(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + OGG_Y) = (uint8_t)((M(ix + OGG_Y) & 0xf0) + 0x10 - M(ix + OGG_MARG_ALTO)
                              - M(ix + OGG_MARG_BAS) - 1);                 /* 0BF6-0C04 */
    M(ix + OGG_SOTTO_Y) = 0;                   /* 0C07 */
    z->a = (uint8_t)(colonna(ix) + 0x10 - M(ix + OGG_MARG_SIN));           /* 0C0B-0C16 */
    M(ix + OGG_X) = z->a;                      /* 0C19 */
    M(ix + OGG_SOTTO_X) = 0;
    M(ix + OGG_STATO) = 2;                     /* 0C20 */
    M(ix + OGG_DIR) = 1;                       /* 0C24 */
    M(ix + 0x02) = 0;                          /* 0C28 */
}

/* 0x0C2D (tabella 08AE, indice D; anche da 0CDA): angolo in alto a
 * sinistra: X come 0BF6, Y alla riga sotto meno il margine alto,
 * sottopixel Y = f0; se stato - 4 è almeno 2 stato 1, direzione 3,
 * (IX+2) = 0, altrimenti azzera le velocità. Brief 26.
 * Uscita: A = stato - 4, B = Y d'ingresso. */
void sub_0c2d(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + OGG_X) = (uint8_t)(colonna(ix) + 0x10 - M(ix + OGG_MARG_SIN));  /* 0C2D-0C3B */
    M(ix + OGG_SOTTO_X) = 0;                   /* 0C3E */
    z->b = M(ix + OGG_Y);                      /* 0C42 */
    M(ix + OGG_Y) = (uint8_t)((z->b & 0xf0) + 0x10 - M(ix + OGG_MARG_ALTO));  /* 0C45-0C4D */
    M(ix + OGG_SOTTO_Y) = 0xf0;                /* 0C50 */
    z->a = (uint8_t)(M(ix + OGG_STATO) - 4);   /* 0C54-0C57 */
    if (z->a >= 2) {                           /* 0C59-0C5B */
        M(ix + OGG_STATO) = 1;                 /* 0C5E */
        M(ix + OGG_DIR) = 3;                   /* 0C62 */
        M(ix + 0x02) = 0;                      /* 0C66 */
        return;
    }
    M(ix + OGG_VEL_X) = 0;                     /* 0C6B-0C77 */
    M(ix + OGG_VEL_X + 1) = 0;
    M(ix + 0x0b) = 0;
    M(ix + 0x0c) = 0;
}

/* 0x0C7C (tabella 08AE, indice E; anche da 0CCE): urto nell'angolo in
 * alto a destra: X alla colonna meno i margini orizzontali (come 0B57),
 * sottopixel X azzerato; Y alla riga sotto meno il margine alto,
 * sottopixel Y = f0 (come 0AC3). Se stato - 4 è almeno 2: stato 1,
 * direzione 2, (IX+2) = 0; altrimenti azzera le velocità X e Y. Prima
 * esecuzione nel replay al frame 2894 (brief 26). Uscita: A = stato - 4. */
void sub_0c7c(struct z80 *z)
{
    uint16_t ix = z->ix;
    uint8_t x = (uint8_t)((((uint8_t)(M(ix + OGG_X) - 8)) & 0xf0) + 8 + 0x10);  /* 0C7C-0C85 */
    M(ix + OGG_X) = (uint8_t)(x - M(ix + OGG_MARG_SIN) - M(ix + OGG_MARG_DES) - 1);  /* 0C87-0C8E */
    M(ix + OGG_SOTTO_X) = 0;                   /* 0C91 */
    M(ix + OGG_Y) = (uint8_t)((M(ix + OGG_Y) & 0xf0) + 0x10 - M(ix + OGG_MARG_ALTO));  /* 0C95-0C9F */
    M(ix + OGG_SOTTO_Y) = 0xf0;                /* 0CA2 */
    z->a = (uint8_t)(M(ix + OGG_STATO) - 4);   /* 0CA6-0CA9 */
    if (z->a >= 2) {                           /* 0CAB-0CAD */
        M(ix + OGG_STATO) = 1;                 /* 0CB0 */
        M(ix + OGG_DIR) = 2;                   /* 0CB4 */
        M(ix + 0x02) = 0;                      /* 0CB8 */
        return;
    }
    M(ix + OGG_VEL_X) = 0;                     /* 0CBD-0CC9 */
    M(ix + OGG_VEL_X + 1) = 0;
    M(ix + 0x0b) = 0;
    M(ix + 0x0c) = 0;
}

/* 0x0CCE (tabella 08AE, indice A): 0C7C se il bit 7 di (IX+0C) è acceso
 * (in salita), altrimenti 0BF6. Brief 26. */
void sub_0cce(struct z80 *z)
{
    if (M(z->ix + 0x0c) & 0x80)                /* 0CCE-0CD2 */
        sub_0c7c(z);                           /* 0CD7 */
    else
        sub_0bf6(z);                           /* 0CD4 */
}

/* 0x0CDA (tabella 08AE, indice 5): 0C2D se il bit 7 di (IX+0C) è acceso,
 * altrimenti 0BBB. Brief 26. */
void sub_0cda(struct z80 *z)
{
    if (M(z->ix + 0x0c) & 0x80)                /* 0CDA-0CDE */
        sub_0c2d(z);                           /* 0CE3 */
    else
        sub_0bbb(z);                           /* 0CE0 */
}

/* La tabella inline a 08AE (16 parole dopo la `call 08CE` di 08A2):
 * 08D8 08ED 094D 0A98 0A24 0CDA 0B57 0BBB 09B3 0AF9 0CCE 0BF6 0AC3 0C2D
 * 0C7C 08D8. Il lotto 7 traduceva solo i gestori visti nelle tracce; gli
 * altri (08ED, 09B3, 0A24, 0BBB, 0BF6, 0C2D, 0C7C, 0CCE, 0CDA) sono stati
 * aggiunti nel brief 26, perché il replay li esegue (0C7C dal frame 2894,
 * 08ED e 0CCE più avanti). */
static void (*const gestori_08ae[16])(struct z80 *) = {
    sub_08d8, sub_08ed, sub_094d, sub_0a98,
    sub_0a24, sub_0cda, sub_0b57, sub_0bbb,
    sub_09b3, sub_0af9, sub_0cce, sub_0bf6,
    sub_0ac3, sub_0c2d, sub_0c7c, sub_08d8,
};

/* 0x08A2: smistamento d'urto dell'oggetto IX. Se il bit 5 di (IX+0) è
 * spento esce subito; altrimenti E = C & 0F (le celle d'urto raccolte da
 * 0602) sceglie il gestore nella tabella inline a 08AE tramite 08CE.
 * 08CE (pop hl / push de / ret) usa l'indirizzo di rientro come base
 * della tabella: qui la tabella si legge dalla ROM e il salto diventa la
 * chiamata del gestore tradotto; 08CE lascia DE = indirizzo del gestore
 * e HL = voce della tabella + 1, riprodotti prima della chiamata.
 * Uscita: registri del gestore (o intatti sul ret z). */
void sub_08a2(struct z80 *z)
{
    if (!(M(z->ix) & 0x20))
        return;
    uint8_t e = (uint8_t)(z->c & 0x0f);
    z->a = e;
    z->e = e;
    uint16_t voce = (uint16_t)(0x08ae + 2 * e);
    SET_HL(z, voce + 1);
    SET_DE(z, (uint16_t)(leggi(voce) | (leggi(voce + 1) << 8)));
    gestori_08ae[e](z);
}

/* 0x0CE6 (dalla NMI, 0114): stati della chiave. IX = c4ce, IY = c299;
 * con il bit 7 di c4ce spento esce subito (sempre, nelle tracce). La
 * continuazione 0CF3 (smistamento `jp (hl)` sulla tabella 0D18) non è
 * traducibile staticamente: avviso su stderr, stato invariato. */
void sub_0ce6(struct z80 *z)
{
    z->ix = 0xc4ce;
    z->iy = 0xc299;
    if (!(M(0xc4ce) & 0x80))
        return;                            /* ret z: sempre nelle tracce */
    fprintf(stderr,
            "0ce6: continuazione 0CF3 (tabella 0D18) non tradotta: nessun effetto\n");
}
