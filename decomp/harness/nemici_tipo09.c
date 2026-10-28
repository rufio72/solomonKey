/* Motore dei nemici, lotto N1 (brief 25): parti del tipo 09 usate dal
 * tipo 12 (5693): la cella davanti all'oggetto (5928), la presa del blocco
 * da parte dell'oggetto portato (58CF) e la sua chiusura (5917).
 * Ingressi: 58CF, 5917, 5928; dal lotto N3, anticipate dal lotto N2
 * (brief 27) per il tipo 14: 6EB7, 6EC1.
 *
 * IX = slot (c2c1 + i·0x18), IY = oggetto (c4fe + i·0x30); (IX+0E) è
 * l'indice dell'oggetto portato nella tabella ROM 48AF. Tradotto da
 * out/main64.dasm. */
#include "routine.h"
#include "nemici.h"

#define PORTATI_48AF   0x48af   /* ROM: indirizzo dell'oggetto per indice */
#define POSA_C08A      0xc08a
#define OGGETTI_488D   0x488d   /* ROM: indirizzo per indice (IX+0E) */

/* IX = indirizzo dell'oggetto portato (tabella 48AF all'indice (IX+0E)),
 * come `ld a,(ix+0e); ld hl,48af; rst 8; rst 10h; push de; ex (sp),ix`.
 * Restituisce l'IX di prima (quello che resta in pila). */
static uint16_t prendi_portato(struct z80 *z)
{
    z->a = M(z->ix + 0x0e);
    SET_HL(z, PORTATI_48AF);
    sub_0008(z);                               /* rst 8 */
    sub_0010(z);                               /* rst 10 */
    uint16_t ix0 = z->ix;
    z->ix = DE(z);                             /* push de; ex (sp),ix: DE invariato */
    return ix0;
}

/* ------------------------------------------------------------------ */
/* 0x5928: cella davanti all'oggetto IY secondo la direzione (IY+1E):
 * Y = (IY+0E) + 8 (f8 col bit 1), X = (IY+15) + 1a (f6 col bit 0), poi
 * sub_36f0. B = bit 1 e bit 0 della direzione; HL = cella, E = indice. */
void sub_5928(struct z80 *z)
{
    NEMICI_PUNTO(0x5928, 0x594e);              /* NMI del replay: 5928 non scrive */
    uint8_t dir = M(z->iy + 0x1e);             /* 5928 */
    z->a = 0x08;                               /* 592B */
    z->b = 0x00;                               /* 592F */
    if (dir & 0x02) {                          /* 592D, 5931 */
        z->a = 0xf8;                           /* 5933 */
        z->b |= 0x02;                          /* 5935 */
    }
    z->h = (uint8_t)(z->a + M(z->iy + 0x0e));  /* 5937-593A */
    z->a = 0x1a;                               /* 593F */
    if (M(z->iy + 0x1e) & 0x01) {              /* 593B, 5941 */
        z->a = 0xf6;                           /* 5943 */
        z->b |= 0x01;                          /* 5945 */
    }
    z->l = (uint8_t)(z->a + M(z->iy + 0x15));  /* 5947-594A */
    sub_36f0(z);                               /* 594B */
}

/* 0x58CF: l'oggetto IY porta l'oggetto della tabella 48AF sul blocco
 * davanti. Bit 6 di (IY+0) acceso, bit 5 spento, (IX+0D) = 0, bit 3 di
 * (IX+1). Se la cella davanti vale meno di 8 esce col riporto acceso.
 * Altrimenti c08a = 0 e l'oggetto portato si posa: sub_4938 per le celle
 * >= 80, sub_49ae (bit 0 di (IY+1) acceso durante la chiamata) per le
 * altre; c835 = ff, riporto spento. */
void sub_58cf(struct z80 *z)
{
    M(z->iy) |= 0x40;                          /* 58CF */
    M(z->iy) &= ~0x20;                         /* 58D3 */
    M(z->ix + 0x0d) = 0x00;                    /* 58D7 */
    M(z->ix + 0x01) |= 0x08;                   /* 58DB */
    NEMICI_PUNTO(0x58df, 0x58ea);              /* NMI del replay */
    uint16_t ix0 = prendi_portato(z);          /* 58DF-58E8 */
    sub_5928(z);                               /* 58EA */
    z->a = M(HL(z));                           /* 58ED */
    if (z->a < 0x08) {                         /* 58EE-58F0 */
        z->ix = ix0;                           /* 58F2 pop ix */
        NEMICI_PONI_RIPORTO(z, 1);
        return;
    }
    SET_HL(z, POSA_C08A);                      /* 58F7 */
    M(POSA_C08A) = 0x00;                       /* 58FA */
    if (z->a >= 0x80) {                        /* 58F5, 58FC */
        sub_4938(z);                           /* 590B */
    } else {
        M(z->iy + 0x01) |= 0x01;               /* 58FE */
        sub_49ae(z);                           /* 5902 */
        M(z->iy + 0x01) &= ~0x01;              /* 5905 */
    }
    z->a = 0xff;                               /* 590E */
    M(0xc835) = z->a;                          /* 5910 */
    z->ix = ix0;                               /* 5913 pop ix */
    NEMICI_PONI_RIPORTO(z, 0);                 /* 5915 or a */
}

/* 0x5917: chiude l'oggetto portato (tabella 48AF all'indice (IX+0E)) con
 * sub_4b98. */
void sub_5917(struct z80 *z)
{
    uint16_t ix0 = prendi_portato(z);          /* 5917-5920 */
    sub_4b98(z->ix);                           /* 5922 */
    z->ix = ix0;                               /* 5925 pop ix */
}

/* ------------------------------------------------------------------ */
/* Lotto N3, anticipate dal lotto N2 (brief 27). */

/* Scrittura a un indirizzo del Z80: sotto c000 è ROM e non cambia. */
static void scrivi_ram(uint16_t a, uint8_t v)
{
    if (a >= 0xc000)
        M(a) = v;
}

/* 0x6EB7: azzera i due byte puntati dalla tabella 488D all'indice A,
 * poi cade in 6EC1. */
void sub_6eb7(struct z80 *z)
{
    SET_HL(z, OGGETTI_488D);                   /* 6EB7 */
    z->c = 0x00;                               /* 6EBA */
    sub_0008(z);                               /* 6EBC rst 8 */
    sub_0018(z);                               /* 6EBD rst 18 */
    scrivi_ram(HL(z), z->c);                   /* 6EBE */
    SET_HL(z, (uint16_t)(HL(z) + 1));          /* 6EBF */
    scrivi_ram(HL(z), z->c);                   /* 6EC0 */
    sub_6ec1(z);
}

/* 0x6EC1: azzera il byte puntato dalla tabella 48AF all'indice A. */
void sub_6ec1(struct z80 *z)
{
    z->c = 0x00;                               /* 6EC1 */
    SET_HL(z, PORTATI_48AF);                   /* 6EC3 */
    sub_0008(z);                               /* 6EC6 rst 8 */
    sub_0018(z);                               /* 6EC7 rst 18 */
    scrivi_ram(HL(z), z->c);                   /* 6EC8 */
}
