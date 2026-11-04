/* Routine del Z80 principale a 251D-253F, lotto 55 (brief 55): piccoli
 * helper dei corpi 15 (251D, 2529) e 16 (2533), tradotti dal disassemblato
 * out/main64.dasm. Si riproducono l'effetto sulla RAM e i registri A, BC,
 * DE, HL, IX, IY all'uscita (le bandiere F no). */
#include "routine.h"
#include "attivita_libera.h"

/* 0x251D (2523 è l'etichetta intermedia): quattro byte da HL in avanti,
 * D, E, E + 1, D + 1. Uscita: E += 1, HL avanzato di 3, poi L = D (H resta
 * quello dell'ultimo byte scritto). */
void sub_251d(struct z80 *z)
{
    M(HL(z)) = z->d;                           /* 251D */
    SET_HL(z, HL(z) + 1);                      /* 251E */
    M(HL(z)) = z->e;                           /* 251F */
    SET_HL(z, HL(z) + 1);                      /* 2520 */
    z->e++;                                    /* 2521 */
    M(HL(z)) = z->e;                           /* 2522 */
    SET_HL(z, HL(z) + 1);                      /* 2523 */
    M(HL(z)) = (uint8_t)(z->d + 1);            /* 2524-2526 inc d; ld (hl),d; dec d */
    z->l = z->d;                               /* 2527 */
}

/* 0x2529: HL = 32 · L (riga di 32 celle). Uscita: C = A, A invariato. */
void sub_2529(struct z80 *z)
{
    z->c = z->a;                               /* 2529 */
    SET_HL(z, (uint16_t)(z->l << 5));          /* 252A-2531 h = 0; 5 × add hl,hl */
}

/* 0x2533: BC = HL = 9 · C (record da 9 byte). Uscita: A invariato. */
void sub_2533(struct z80 *z)
{
    uint16_t nove = (uint16_t)(9 * z->c);      /* 2533-253C */
    SET_HL(z, nove);
    SET_BC(z, nove);                           /* 253D-253E */
}
