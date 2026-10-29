/* Motore dei nemici, lotto N3 (brief 29): due dipendenze del tipo 09 che il
 * censimento assegna al lotto N10 (tipo 06, parte A): 51D7, che sceglie a
 * caso uno dei bit accesi di una maschera, e 24DF, un piccolo calcolo su
 * nibble con una coppia di byte della tabella puntata da HL.
 * Tradotto da out/main64.dasm. */
#include "routine.h"
#include "nemici.h"

#define TAB_C06A 0xc06a   /* RAM: posizioni dei bit accesi della maschera */

/* Scambio dei nibble: quattro `rrca`. */
static uint8_t scambia(uint8_t x)
{
    return (uint8_t)((x >> 4) | (x << 4));
}

/* 0x24DF: A, (HL) e (HL+1) sono tre indici a nibble. E = (8 + A + (HL)),
 * D = nibble alto di E scambiato (& 0f), poi (HL+1) + E scambiato di nuovo
 * dà E (& 0f). C = E + B, A = B = 0, HL avanzato di uno. La tabella può
 * stare in ROM: si legge con leggi(). */
void sub_24df(struct z80 *z)
{
    uint16_t hl = HL(z);
    uint8_t e = (uint8_t)(0x08 + z->a + leggi(hl));       /* 24DF-24E7 */
    hl++;                                                  /* 24E8 */
    z->d = scambia(e) & 0x0f;                              /* 24E9-24F7 */
    uint8_t a = scambia((uint8_t)(leggi(hl) + e)) & 0x0f;  /* 24F8-2507 */
    z->e = a;                                              /* 2508 */
    z->c = (uint8_t)(a + z->b);                            /* 2509 */
    z->a = 0x00;                                           /* 250B */
    z->b = 0x00;                                           /* 250C */
    SET_HL(z, hl);
}

/* 0x51D7: BC = maschera a 14 bit. Da c06a si scrivono le posizioni dei bit
 * accesi (b - 1, con b da 14 a 1) e C ne conta il numero; 54C5 sceglie a
 * caso un indice C minore del numero. Uscita: A = 0d - (c06a + C), B = 0,
 * C = E = indice, D = 0, HL = c06a + indice. */
void sub_51d7(struct z80 *z)
{
    uint16_t de = BC(z);                       /* 51D7-51D8 */
    uint16_t hl = TAB_C06A;                    /* 51DC */
    z->c = 0x00;                               /* 51D9 ld bc,0e00 */
    for (z->b = 0x0e; z->b; z->b--) {          /* 51DF-51EA */
        int bit = de & 1;                      /* rr d; rr e: il riporto d'ingresso */
        de >>= 1;                              /* non arriva a un bit esaminato */
        if (bit) {
            M(hl) = (uint8_t)(z->b - 1);       /* 51E5-51E6 */
            z->c++;                            /* 51E8 */
            hl++;                              /* 51E9 */
        }
    }
    sub_54c5(z);                               /* 51EC */
    z->e = z->c;                               /* 51EF */
    z->d = 0x00;                               /* 51F0 */
    hl = (uint16_t)(TAB_C06A + z->e);          /* 51F2-51F5 */
    SET_HL(z, hl);
    z->a = (uint8_t)(0x0d - M(hl));            /* 51F6-51F8 */
}
