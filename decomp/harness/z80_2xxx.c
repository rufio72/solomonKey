/* Routine del Z80 principale fra 0x2000 e 0x2fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* 0x237E: aggiorna un piccolo record a HL.
 * (HL) diventa ((HL) + 1) & 0x7f; la parola big-endian in (HL+1),(HL+2)
 * (byte alto prima) diminuisce di 0x0150.
 * Uscita: A = byte alto nuovo, BC = 0150, DE = parola nuova, HL = HL+1. */
void sub_237e(struct z80 *z)
{
    uint16_t hl = HL(z);
    M(hl) = (uint8_t)((M(hl) + 1) & 0x7f);
    uint16_t parola = (uint16_t)((M(hl + 1) << 8) | M(hl + 2));
    parola -= 0x0150;
    M(hl + 2) = parola & 0xff;
    M(hl + 1) = parola >> 8;
    z->a = parola >> 8;
    SET_BC(z, 0x0150);
    SET_DE(z, parola);
    SET_HL(z, hl + 1);
}

/* 0x23A1: somma a 16 bit della parola in DE alla parola in HL (byte basso
 * prima). Uscita: A = byte alto della somma, DE e HL avanzati di 2. */
void sub_23a1(struct z80 *z)
{
    uint16_t hl = HL(z), de = DE(z);
    uint16_t somma = (uint16_t)(leggi(hl) | (leggi(hl + 1) << 8)) +
                     (uint16_t)(leggi(de) | (leggi(de + 1) << 8));
    M(hl) = somma & 0xff;
    M(hl + 1) = somma >> 8;
    z->a = somma >> 8;
    SET_HL(z, hl + 2);
    SET_DE(z, de + 2);
}

/* 0x2418: indirizzo di una cella in una griglia di 32 colonne.
 * Ingresso: D riga, E colonna. Uscita: HL = 32·D + E, D = 0; A invariato. */
void sub_2418(struct z80 *z)
{
    SET_HL(z, 32 * z->d + z->e);
    z->d = 0;
}

/* 0x249E: spostamento di un record a HL.
 * (HL) += C; parola a HL+3 (byte basso prima) += D:C; (HL+5) += C.
 * Uscita: A = nuovo (HL+5), HL = HL + 5 + DE. */
void sub_249e(struct z80 *z)
{
    uint16_t hl = HL(z);
    M(hl) += z->c;
    uint16_t parola = (uint16_t)(M(hl + 3) | (M(hl + 4) << 8));
    parola += (uint16_t)((z->d << 8) | z->c);
    M(hl + 3) = parola & 0xff;
    M(hl + 4) = parola >> 8;
    z->a = M(hl + 5) = (uint8_t)(M(hl + 5) + z->c);
    SET_HL(z, hl + 5 + DE(z));
}

/* 0x24B1: riempie tre byte di un record a HL.
 * Il riporto in ingresso entra nel bit 7 di A (rra): serve davvero, quindi
 * si legge dal bit 0 di F.
 * (HL+1) = E, (HL+2) = A ruotato, (HL+3) = C.
 * Uscita: HL = HL+3, E = B, C = B = C+1, A = 0. */
void sub_24b1(struct z80 *z)
{
    uint16_t hl = HL(z);
    uint8_t a = (uint8_t)((z->a >> 1) | ((z->f & 1) << 7));
    M(hl + 1) = z->e;
    M(hl + 2) = a;
    M(hl + 3) = z->c;
    SET_HL(z, hl + 3);
    z->e = z->b;
    z->c++;
    z->b = z->c;
    z->a = 0;
}
