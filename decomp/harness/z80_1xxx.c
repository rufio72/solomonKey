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

/* ------------------------------------------------------------------ */
/* Lotto 38 (compito 38): contatori BCD e bitmap dei quadri bonus. */

/* 0x1474: raddoppio BCD di 3 byte a HL (cifre basse prima). Con
 * `or a` il riporto parte da zero; per ogni byte `adc a,a; daa`, e il
 * riporto passa al byte seguente. Uscita: A = ultimo byte, B = 0,
 * HL = HL + 3, riporto nel bit 0 di F. */
void sub_1474(struct z80 *z)
{
    uint16_t hl = HL(z);
    unsigned cy = 0;                           /* 1474 or a */
    uint8_t a = z->a;
    for (int i = 0; i < 3; i++) {              /* 1475 ld b,3 ... 147C djnz */
        a = M(hl);                             /* 1477 */
        unsigned somma = a + a + cy;           /* 1478 adc a,a */
        unsigned mezzo = ((a & 0x0f) + (a & 0x0f) + cy) > 0x0f;
        cy = somma > 0xff;
        a = (uint8_t)somma;
        unsigned corr = 0;                     /* 1479 daa (dopo una somma) */
        if (mezzo || (a & 0x0f) > 9)
            corr |= 0x06;
        if (cy || a > 0x99) {
            corr |= 0x60;
            cy = 1;
        }
        a = (uint8_t)(a + corr);
        M(hl) = a;                             /* 147A */
        hl++;                                  /* 147B */
    }
    z->a = a;
    z->b = 0;
    SET_HL(z, hl);
    z->f = (uint8_t)((z->f & ~1) | cy);
}

/* 0x1471: raddoppio BCD del tempo c093-c095 (HL = c093, poi 1474). */
void sub_1471(struct z80 *z)
{
    SET_HL(z, 0xc093);                         /* 1471 */
    sub_1474(z);                               /* cade in 1474 */
}

/* 0x147F: indirizzo di una parola della tabella 8EA4 (indice 2·(c243),
 * a 9 bit), letta con rst 10 (ROM), più 9. Uscita: A = byte basso
 * dell'indirizzo nella tabella, DE = parola letta, HL = DE + 9,
 * riporto di `add hl,de` nel bit 0 di F. */
void sub_147f(struct z80 *z)
{
    uint8_t a = (uint8_t)(M(0xc243) * 2);      /* 147F-1482 */
    uint16_t hl = 0x8ea4;                      /* 1483 */
    unsigned s = a + (hl & 0xff);              /* 1486 add a,l */
    z->a = (uint8_t)s;
    hl = (uint16_t)((hl & 0xff00) | (uint8_t)s);   /* 1487 */
    if (s > 0xff)
        hl += 0x0100;                          /* 148A inc h */
    SET_HL(z, hl);
    sub_0010(z);                               /* 148B rst 10 */
    unsigned t = 0x0009 + DE(z);               /* 148C-148F */
    SET_HL(z, t);
    z->f = (uint8_t)((z->f & ~1) | (t > 0xffff));
}

/* 0x1533: cerca una coppia di bit libera in una bitmap che scende da
 * c25d, per (c259) coppie. Maschera A = c0 che ruota di 2 a ogni passo
 * (si scende di un byte a ogni riporto). Se `and (hl)` dà zero esce con
 * Z acceso (bit 6 di F); se la ricerca si esaurisce esce con il riporto
 * acceso (bit 0) e Z spento. Uscita: A = risultato dell'ultimo `and`
 * (zero) oppure la maschera ruotata, C = maschera del passo, B = passi
 * mancanti, HL = byte raggiunto. */
void sub_1533(struct z80 *z)
{
    uint8_t b = M(0xc259);                     /* 1533 */
    uint16_t hl = 0xc25d;                      /* 1536 */
    uint8_t a = 0xc0, c = 0;                   /* 153A */
    do {
        c = a;                                 /* 153C */
        a = (uint8_t)(a & M(hl));              /* 153D and (hl) */
        if (a == 0) {                          /* 153E ret z */
            z->a = a;
            z->b = b;
            z->c = c;
            SET_HL(z, hl);
            z->f = (uint8_t)((z->f & ~0x41) | 0x40);
            return;
        }
        a = c;                                 /* 153F */
        a = (uint8_t)((a >> 1) | (a << 7));    /* 1540 rrca */
        unsigned cy = a >> 7;
        a = (uint8_t)((a >> 1) | (a << 7));    /* 1541 rrca */
        cy = a >> 7;
        if (cy)
            hl--;                              /* 1544 dec hl */
        b--;                                   /* 1545 djnz */
    } while (b != 0);
    z->a = a;                                  /* 1547 scf */
    z->b = 0;
    z->c = c;
    SET_HL(z, hl);
    z->f = (uint8_t)((z->f & ~0x40) | 0x01);
}
