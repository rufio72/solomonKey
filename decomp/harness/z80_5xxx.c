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

/* Classi delle 4 celle attorno all'oggetto (riempite da 0x5C32). */
#define CLASSI_C07E 0xc07e

/* 0x5C32: classifica le celle della griglia FG attorno all'oggetto IY:
 * cella di partenza da x = (IY+15) + 0x0a (o + 6 col bit 0 di IY+1E) e
 * y = 0x18 + (IY+0E) via sub_36f0; scrive in c07e-c081 le classi
 * (sub_5c78) della cella, della vicina (±1 secondo il bit 3 di B, cioè
 * del passo 0x0a/6) e delle due righe sotto (+0x10 e +0x10).
 * Uscita: HL = c081, B = 0x10, C e A dall'ultima sub_5c78, DE ultima cella. */
void sub_5c32(struct z80 *z)
{
    uint8_t b = 0x0a;
    if (M(z->iy + 0x1e) & 1)
        b = 6;
    z->l = (uint8_t)(M(z->iy + 0x15) + b);
    z->h = (uint8_t)(0x18 + M(z->iy + 0x0e));
    sub_36f0(z);
    SET_DE(z, HL(z));              /* ex de,hl: DE = cella */
    uint16_t hl = CLASSI_C07E;
    sub_5c78(z);
    M(hl) = z->c; hl++;
    if (b & 0x08)                  /* bit 3,b: passo 0x0a */
        SET_DE(z, DE(z) + 1);
    else
        SET_DE(z, DE(z) - 1);
    sub_5c78(z);
    M(hl) = z->c; hl++;
    z->b = 0x10;
    SET_DE(z, DE(z) + 0x10);
    sub_5c78(z);
    M(hl) = z->c; hl++;
    SET_DE(z, DE(z) + 0x10);
    sub_5c78(z);
    M(hl) = z->c;
    SET_HL(z, hl);
}
