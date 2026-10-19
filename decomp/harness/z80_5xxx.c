/* Routine del Z80 principale fra 0x5000 e 0x5fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* 0x5C78: classifica il byte con segno a DE.
 * Uscita: A = byte letto; C = 1 se 0..0x36, 0 se 0x37..0x7f,
 * 2 se 0x80..0xf7, 3 se 0xf8..0xff. */
void sub_5c78(struct z80 *z)
{
    uint8_t v = leggi(DE(z));
    uint8_t c = 0;
    if (v < 0x80) {
        if (v < 0x37)
            c = 1;
    } else {
        c = 2;
        if (v >= 0xf8)
            c = 3;
    }
    z->a = v;
    z->c = c;
}
