/* Routine del Z80 principale fra 0x4000 e 0x4fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* 0x4AF6: inizializza i campi di un oggetto a IX.
 * IX+04 = E, IX+05 = ff, IX+1B = E, IX+1C = ff, IX+1D = B, IX+1E = C,
 * IX+00 = A (stato). Registri invariati. */
void sub_4af6(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + 0x04) = z->e;
    M(ix + 0x05) = 0xff;
    M(ix + 0x1b) = z->e;
    M(ix + 0x1c) = 0xff;
    M(ix + 0x1d) = z->b;
    M(ix + 0x1e) = z->c;
    M(ix + 0x00) = z->a;
}

/* 0x4B0E: da un indice di cella (riga nel nibble alto, colonna nel basso)
 * alle coordinate in pixel.
 * Uscita: H = ((~A) - 0x10) & 0xf0 (y), L = (A & 0x0f)·16 - 8 (x), A = L. */
void sub_4b0e(struct z80 *z)
{
    uint8_t cella = z->a;
    z->h = (uint8_t)(((uint8_t)~cella - 0x10) & 0xf0);
    z->l = (uint8_t)(((cella & 0x0f) << 4) - 8);
    z->a = z->l;
}
