/* Gestori di raccolta della tabella rst 38 di 12F0 (130A, 20 voci), lotto
 * 54 (brief 54): quelli non ancora tradotti, come gestore_XXXX. I gestori
 * 139C, 13AB, 13B9, 1413, 14D4 e la coda comune 13FC (con 14FC) esistono
 * già in nemici_comparsa.c (li usa anche il motore dei nemici) e non si
 * riscrivono. Tradotti dal disassemblato out/main64.dasm; ogni gestore
 * finisce in 13FC con A = 29..2D (punteggio dalla tabella 150C e posta
 * dell'attività 05). Registri all'uscita come l'originale, salvo F. */
#include "routine.h"
#include "nemici.h"
#include "attivita_libera.h"

/* Somma BCD di un byte con riporto (`adc a,n; daa`). */
static uint8_t bcd_adc(uint8_t a, uint8_t n, unsigned *cy)
{
    unsigned somma = a + n + *cy;
    unsigned mezzo = ((a & 0x0f) + (n & 0x0f) + *cy) > 0x0f;
    unsigned riporto = somma > 0xff;
    uint8_t r = (uint8_t)somma;
    unsigned corr = 0;
    if (mezzo || (r & 0x0f) > 9)
        corr |= 0x06;
    if (riporto || r > 0x99) {
        corr |= 0x60;
        riporto = 1;
    }
    *cy = riporto;
    return (uint8_t)(r + corr);
}

/* 0x135B: sui 17 oggetti da c4fe (passo 30) accesi (bit 7-6 di +0 = 11),
 * con (ix+4) − 40 in 00-0F o 28-37, accende il bit 1 di +0; se almeno uno
 * è stato segnato suono 33 e posta l'attività 12. Poi A = 2b, 13FC. */
void gestore_135b(struct z80 *z)
{
    z->ix = 0xc4fe;                            /* 135B */
    SET_DE(z, 0x0030);                         /* 135F */
    z->l = z->d;                               /* 1362 */
    z->b = 0x11;                               /* 1363 */
    do {
        z->a = M(z->ix);                       /* 1365 */
        if (z->a >= 0xc0) {                    /* 1368-136A */
            uint8_t y = M((uint16_t)(z->ix + 4));   /* 136C */
            int segna = 0;
            if (y >= 0x40) {                   /* 136F-1371 */
                z->a = (uint8_t)(y - 0x40);
                if (z->a < 0x10)               /* 1373-1375 */
                    segna = 1;
                else if (z->a >= 0x28) {       /* 1377-1379 */
                    z->a = (uint8_t)(z->a - 0x28);
                    if (z->a < 0x10)           /* 137B-137D */
                        segna = 1;
                } else
                    z->a = (uint8_t)(z->a - 0x28);
            } else
                z->a = (uint8_t)(y - 0x40);
            if (segna) {
                M(z->ix) |= 0x02;              /* 1394 */
                z->l |= 0x01;                  /* 1398 */
            }
        }
        z->ix = (uint16_t)(z->ix + DE(z));     /* 137F */
    } while (--z->b != 0);                     /* 1381 */
    if (z->l & 0x01) {                         /* 1383-1385 */
        z->c = 0x33;                           /* 1387 */
        sub_0028(z);                           /* 1389 rst 28h */
        z->a = 0x12;                           /* 138A */
        sub_0493(z);                           /* 138C */
    }
    z->a = 0x2b;                               /* 138F */
    sub_13fc(z);                               /* 1391 */
}

/* 0x13C3: DE = 4, 134B, A = 29, 13FC. */
void gestore_13c3(struct z80 *z)
{
    SET_DE(z, 0x0004);                         /* 13C3 */
    sub_134b(z);                               /* 13C6 */
    z->a = 0x29;                               /* 13C9 */
    sub_13fc(z);                               /* 13CB */
}

/* 0x13CD: A = 2b, 13FC. */
void gestore_13cd(struct z80 *z)
{
    z->a = 0x2b;                               /* 13CD */
    sub_13fc(z);                               /* 13CF */
}

/* 0x13D1: A = 2d, 13FC. */
void gestore_13d1(struct z80 *z)
{
    z->a = 0x2d;                               /* 13D1 */
    sub_13fc(z);                               /* 13D3 */
}

/* Coda di 13D5 e 13E6: se 1533 trova una coppia di bit libera (senza
 * riporto) vi accende i bit di `maschera & C`; poi il tabellone (349F),
 * A = punti, 13FC. */
static void coppia_libera(struct z80 *z, uint8_t maschera, uint8_t punti)
{
    sub_1533(z);                               /* 13D5 / 13E6 */
    if (!NEMICI_RIPORTO_ACCESO(z)) {           /* jr c */
        z->a = (uint8_t)((maschera & z->c) | M(HL(z)));
        M(HL(z)) = z->a;                       /* 13DA-13DE / 13EB-13EF */
    }
    sub_349f(z);                               /* 13DF / 13F0 */
    z->a = punti;                              /* 13E2 / 13F3 */
    sub_13fc(z);
}

/* 0x13D5: coppia libera con il bit basso (55), A = 2a. */
void gestore_13d5(struct z80 *z)
{
    coppia_libera(z, 0x55, 0x2a);
}

/* 0x13E6: coppia libera con il bit alto (AA), A = 2b. */
void gestore_13e6(struct z80 *z)
{
    coppia_libera(z, 0xaa, 0x2b);
}

/* 0x13F7 (da 1302, classi 29-36): A = classe, suono 10, 13FC. */
void gestore_13f7(struct z80 *z)
{
    z->a = (uint8_t)(z->a + 0x08);             /* 13F7 */
    z->c = 0x10;                               /* 13F9 */
    sub_0028(z);                               /* 13FB rst 28h */
    sub_13fc(z);                               /* cade in 13FC */
}

/* 0x1424: c092 = 2, raddoppio BCD di c093-c095 (1471) e di c096-c098
 * (1474), A = 29, 13FC. */
void gestore_1424(struct z80 *z)
{
    SET_HL(z, 0xc092);                         /* 1424 */
    M(0xc092) = 0x02;                          /* 1427 */
    sub_1471(z);                               /* 1429 */
    SET_HL(z, 0xc096);                         /* 142C */
    sub_1474(z);                               /* 142F */
    z->a = 0x29;                               /* 1432 */
    sub_13fc(z);                               /* 1434 */
}

/* 0x1436: c092 = 5; copia c093-c098 in c061-c066; quadruplica in BCD
 * c093-c095 e c096-c098 e vi somma la copia (×5), A = 2b, 13FC. */
void gestore_1436(struct z80 *z)
{
    M(0xc092) = 0x05;                          /* 1436-1439 */
    for (int i = 0; i < 6; i++)                /* 143B-1442 ldir */
        M(0xc061 + i) = M(0xc093 + i);
    sub_1471(z);                               /* 1444 */
    sub_1471(z);                               /* 1447 */
    SET_HL(z, 0xc096);                         /* 144A */
    sub_1474(z);                               /* 144E */
    SET_HL(z, 0xc096);                         /* 1451 pop hl */
    sub_1474(z);                               /* 1452 */
    uint16_t hl = 0xc093, de = 0xc061;         /* 1456-1459 */
    unsigned cy = 0;                           /* 1455 or a */
    for (int c = 2; c > 0; c--) {              /* 145C, 1469-146A */
        for (int b = 3; b > 0; b--) {          /* 145E, 1466 djnz */
            z->a = bcd_adc(M(de), M(hl), &cy); /* 1460-1462 */
            M(hl) = z->a;                      /* 1463 */
            hl++;
            de++;
        }
        cy = 0;                                /* 1468 or a */
    }
    SET_HL(z, hl);
    SET_DE(z, de);
    z->b = 0;
    z->c = 0;
    z->a = 0x2b;                               /* 146C */
    sub_13fc(z);                               /* 146E */
}

/* 0x1491: byte del quadro (147F, letto in HL) × 16 in c094-c095, c093 =
 * 0, A = 2b, 13FC. */
void gestore_1491(struct z80 *z)
{
    sub_147f(z);                               /* 1491 */
    uint16_t v = (uint16_t)(leggi(HL(z)) << 4);   /* 1494-149A */
    SET_HL(z, DE(z));                          /* 149B ex de,hl */
    SET_DE(z, v);
    SET_HL(z, 0xc093);                         /* 149C */
    M(0xc093) = 0x00;                          /* 149F */
    M(0xc094) = z->e;                          /* 14A2 */
    M(0xc095) = z->d;                          /* 14A4 */
    SET_HL(z, 0xc095);
    z->a = 0x2b;                               /* 14A5 */
    sub_13fc(z);                               /* 14A7 */
}

/* 0x14AA: byte del quadro (147F) × 5 in BCD (due raddoppi e una somma,
 * con il riporto nel byte alto) in c094-c095, c093 = 0, A = 2a, 13FC. */
void gestore_14aa(struct z80 *z)
{
    sub_147f(z);                               /* 14AA */
    uint8_t l = leggi(HL(z));                  /* 14AD */
    uint8_t e = l;                             /* 14AE */
    uint8_t h = 0;                             /* 14AF */
    unsigned cy;
    for (int b = 2; b > 0; b--) {              /* 14B1, 14BB djnz */
        cy = 0;
        l = bcd_adc(l, l, &cy);                /* 14B3-14B6 add a,a; daa */
        h = (uint8_t)(h + cy);                 /* 14B7-14BA adc a,0 */
    }
    cy = 0;
    l = bcd_adc(l, e, &cy);                    /* 14BD-14C0 add a,e; daa */
    h = (uint8_t)(h + cy);                     /* 14C1-14C4 */
    SET_DE(z, (uint16_t)((h << 8) | l));       /* 14C5 ex de,hl */
    z->b = 0;
    M(0xc093) = 0x00;                          /* 14C6-14C9 */
    M(0xc094) = z->e;                          /* 14CC */
    M(0xc095) = z->d;                          /* 14CE */
    SET_HL(z, 0xc095);
    z->a = 0x2a;                               /* 14CF */
    sub_13fc(z);                               /* 14D1 */
}
