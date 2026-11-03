/* Routine del Z80 principale fra 0x2d00 e 0x2dff, tradotte in C leggibile
 * (z80_2xxx.c ha raggiunto le 796 righe). Si riproducono l'effetto sulla
 * RAM e i registri all'uscita (le bandiere F no). */
#include "routine.h"

/* 0x2D8A (2C34, 6455, 6A15): scambia i 49 byte di c237-c267 con quelli di
 * c268-c298 (due blocchi consecutivi di 0x31 byte).
 * Uscita: B = 0, C = ultimo byte di c237-c267 prima dello scambio, A = ultimo
 * di c268-c298, HL = c268, DE = c299. */
void sub_2d8a(struct z80 *z)
{
    uint16_t hl = 0xc237, de = 0xc268;         /* 2D8A-2D8F */
    for (int i = 0; i < 0x31; i++) {           /* 2D92-2D9A */
        z->c = M(hl);                          /* 2D92 */
        z->a = M(de);                          /* 2D93 */
        M(hl) = z->a;                          /* 2D94-2D95 ex de,hl; ld (hl),c */
        M(de) = z->c;                          /* 2D96 */
        hl++;
        de++;
    }
    z->b = 0;
    SET_HL(z, hl);
    SET_DE(z, de);
}
