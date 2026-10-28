/* Motore dei nemici, lotto N1 (brief 25): cella vicina all'oggetto IY in
 * una delle quattro direzioni (734D), usata da 7235 per guardare davanti.
 * Il modulo accoglierà la comparsa delle entità (lotto N4, 4FC5 e
 * seguenti). Ingresso: 734D; dal lotto N4, anticipata dal lotto N2 (brief
 * 27): 80A5.
 *
 * Tradotto da out/main64.dasm. */
#include "routine.h"
#include "nemici.h"

/* 0x734D: sceglie gli scostamenti (H, L) fra B e C dal primo bit acceso
 * di A (dal bit 0: (B, C), (B, B), (C, B), (C, C)), li somma a (IY+0E) e
 * (IY+15) e chiama sub_36f0 (HL = cella, E = indice). Senza bit accesi
 * nei bit 0-3 l'originale fa `ld (0000h),a` (scrittura in ROM, nessun
 * effetto) e ritorna con H = C. */
void sub_734d(struct z80 *z)
{
    uint8_t a = z->a;
    z->h = z->b;                               /* 734D */
    if (a & 0x01) {                            /* 734E-734F rra */
        z->l = z->c;                           /* 7351 */
    } else if (a & 0x02) {                     /* 7354-7355 */
        z->l = z->b;                           /* 7357 */
    } else {
        z->h = z->c;                           /* 735A */
        if (a & 0x04) {                        /* 735B-735C */
            z->l = z->b;                       /* 735E */
        } else if (a & 0x08) {                 /* 7361-7362 */
            z->l = z->c;                       /* 7364 */
        } else {
            /* 7367 ld (0000h),a: A ruotato quattro volte attraverso il riporto */
            z->a = (uint8_t)(a >> 4 | (a << 5) | ((z->f & 1) << 4));
            return;                            /* 736A */
        }
    }
    z->h = (uint8_t)(z->h + M(z->iy + 0x0e));  /* 736B-736F */
    z->l = (uint8_t)(z->l + M(z->iy + 0x15));  /* 7370-7374 */
    sub_36f0(z);                               /* 7375 */
}

/* 0x80A5 (lotto N4, anticipata dal lotto N2): azzera (IX+0), (IX+1),
 * (IY+0), (IY+1): slot e oggetto liberi. A = 0. */
void sub_80a5(struct z80 *z)
{
    z->a = 0;                                  /* 80A5 */
    M(z->ix + 0x00) = z->a;                    /* 80A6 */
    M(z->ix + 0x01) = z->a;                    /* 80A9 */
    M(z->iy + 0x00) = z->a;                    /* 80AC */
    M(z->iy + 0x01) = z->a;                    /* 80AF */
}
