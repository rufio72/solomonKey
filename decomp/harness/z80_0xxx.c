/* Routine del Z80 principale fra 0x0000 e 0x0fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* 0x0008 (rst 08): indice di una tabella di parole.
 * Ingresso: A indice, HL base. Uscita: DE = A, HL = HL + 2·A. */
void sub_0008(struct z80 *z)
{
    SET_DE(z, z->a);
    SET_HL(z, HL(z) + 2 * DE(z));
}

/* 0x0010 (rst 10): legge la parola a HL (anche in ROM).
 * Uscita: DE = parola (byte basso a HL), HL avanzato di 2. */
void sub_0010(struct z80 *z)
{
    uint16_t hl = HL(z);
    z->e = leggi(hl);
    z->d = leggi(hl + 1);
    SET_HL(z, hl + 2);
}

/* Contatore a 16 bit c090-c091 e suo incremento c08f, attivo col bit 7 di c08d. */
#define TEMPO_ATTIVO   0xc08d   /* bit 7: conteggio attivo */
#define TEMPO_PASSO    0xc08f   /* valore scelto dal record della stanza */
#define TEMPO_CONTO    0xc090   /* parola: byte basso c090, alto c091 */

/* 0x026C (chiamata dalla NMI): se il bit 7 di c08d è acceso somma c08f alla
 * parola c090-c091. Uscita: HL sull'ultimo byte toccato, A = byte basso nuovo. */
void sub_026c(struct z80 *z)
{
    SET_HL(z, TEMPO_ATTIVO);
    if (!(M(TEMPO_ATTIVO) & 0x80))
        return;
    unsigned somma = M(TEMPO_PASSO) + M(TEMPO_CONTO);
    z->a = M(TEMPO_CONTO) = (uint8_t)somma;
    SET_HL(z, TEMPO_CONTO);
    if (somma <= 0xff)
        return;
    /* riporto: si incrementa il byte alto */
    M(TEMPO_CONTO + 1)++;
    SET_HL(z, TEMPO_CONTO + 1);
}

/* 0x0018 (rst 18): come rst 10 ma con DE e HL scambiati all'uscita.
 * Uscita: HL = parola letta a HL (anche ROM), DE = HL d'ingresso + 2. */
void sub_0018(struct z80 *z)
{
    sub_0010(z);
    uint16_t de = DE(z);
    SET_DE(z, HL(z));
    SET_HL(z, de);
}
