/* Routine del Z80 principale fra 0x6000 e 0x6fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* Tabella delle attività dello scheduler (64 byte). */
#define TABELLA_ATTIVITA 0xc000
#define LUNG_ATTIVITA    0x40

/* 0x673F: azzera la tabella delle attività c000-c03f.
 * Uscita: BC = 0, HL = c040; A invariato. */
void sub_673f(struct z80 *z)
{
    for (int i = 0; i < LUNG_ATTIVITA; i++)
        M(TABELLA_ATTIVITA + i) = 0;
    SET_BC(z, 0);
    SET_HL(z, TABELLA_ATTIVITA + LUNG_ATTIVITA);
}
