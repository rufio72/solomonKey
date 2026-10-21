/* Routine del Z80 principale fra 0x1000 e 0x1fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include <stdio.h>
#include "routine.h"

/* 0x1288 (dalla NMI, 0117): raccolta sotto Dana. IX = c82e; esce se Dana
 * è spento o sulla scala (posa 7). Calcola la cella della griglia al
 * centro di Dana ((ix+0e)+8, (ix+15)+8) con sub_36f0 e ne legge la
 * classe: se è almeno 0x37 esce (sempre, nelle tracce). La continuazione
 * 12AB (raccolta vera, zona mai eseguita fino a 2349) non è tradotta:
 * avviso su stderr, stato invariato. Uscita del ramo eseguito:
 * A = classe − 0x37, HL = cella (c0bd + spiazzo), DE = spiazzo, IX = c82e. */
void sub_1288(struct z80 *z)
{
    z->ix = 0xc82e;
    z->a = M(0xc82e);
    if (z->a < 0xc0)
        return;                            /* ret c: Dana spento */
    z->a = 0x07;
    if (M(0xc82e + 0x1d) == 0x07)
        return;                            /* ret z: sulla scala */
    z->h = (uint8_t)(M(0xc82e + 0x0e) + 8);
    z->l = (uint8_t)(M(0xc82e + 0x15) + 8);
    z->a = z->l;
    sub_36f0(z);                           /* cella della griglia c0bd */
    z->a = (uint8_t)(M(HL(z)) - 0x37);
    if (M(HL(z)) >= 0x37)
        return;                            /* ret nc: niente da raccogliere */
    fprintf(stderr,
            "1288: continuazione 12AB (raccolta) non tradotta: nessun effetto\n");
}
