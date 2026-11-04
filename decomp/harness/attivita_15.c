/* Corpo dell'attività 15 (6466) del gioco libero, lotto 55 (brief 55):
 * macchina a passi sul contatore c19d (tabella `rst 20h` 646B, 10 voci),
 * tradotta dal disassemblato out/main64.dasm. Regole come il 13
 * (attivita_12_13.c): HL = c19d all'ingresso, A = passo, c19d avanza di
 * uno; `dec (hl)` resta sullo stesso passo. Le attese sono la voce 64CE
 * (attesa_64ce); il passo 9 è 6655 (passo_6655 di attivita_26.c), che
 * chiama 75AA e dura più di un frame: non si cattura e si verifica per
 * composizione. I registri del Z80 (struct z80) seguono il disassemblato
 * per passare gli argomenti alle routine tradotte. */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"

/* Passo 0, 64D3: azzeramenti di 6502, Dana (c84b = 9, bit 1 di c84c
 * spento), l'oggetto c85e alle coordinate della cella c247 (4B0E) con lo
 * stato C4 (4AF6, BC = 0502, E = 1), suono 26 e posta 15. */
static void passo_64d3(void)
{
    struct z80 z = {0};
    sub_6502(&z);                              /* 64D4 */
    SET_HL(&z, 0xc84b);                        /* 64D7 */
    M(0xc84b) = 0x09;                          /* 64DA */
    SET_HL(&z, 0xc84c);                        /* 64DC */
    M(0xc84c) &= (uint8_t)~0x02;               /* 64DD */
    z.a = M(0xc247);                           /* 64DF */
    sub_4b0e(&z);                              /* 64E2 */
    z.ix = 0xc85e;                             /* 64E5 */
    M(z.ix + 0x0e) = z.h;                      /* 64E9 */
    M(z.ix + 0x15) = z.l;                      /* 64EC */
    z.e = 0x01;                                /* 64EF */
    SET_BC(&z, 0x0502);                        /* 64F1 */
    z.a = 0xc4;                                /* 64F4 */
    sub_4af6(&z);                              /* 64F6 */
    z.c = 0x26;                                /* 64F9 */
    sub_0028(&z);                              /* 64FB rst 28h */
    z.a = 0x15;                                /* 64FC */
    sub_0493(&z);                              /* 64FE */
}

/* Passo 2, 6539: c85e a zero; quattro oggetti da c55e (passo 30) ai
 * quattro angoli (+0 o +10) del riquadro che parte 8 sopra e a sinistra
 * delle coordinate di c85e, ognuno con lo stato C4 (4AF6, B = 6, E = 1).
 * C scende da 3 a 0 e sceglie l'angolo (bit 0: colonna, bit 1: riga). */
static void passo_6539(void)
{
    struct z80 z = {0};
    z.ix = 0xc85e;                             /* 653A */
    z.e = 0xf8;                                /* 653E */
    z.h = (uint8_t)(z.e + M(z.ix + 0x0e));     /* 6540-6544 */
    z.l = (uint8_t)(z.e + M(z.ix + 0x15));     /* 6545-6549 */
    z.a = 0x00;                                /* 654A */
    M(z.ix) = z.a;                             /* 654B */
    z.ix = 0xc55e;                             /* 654E */
    SET_BC(&z, 0x0603);                        /* 6552 */
    for (;;) {
        z.d = 0x10;                            /* 6555 */
        z.a = z.l;                             /* 6557 */
        if (!(z.c & 0x01))                     /* 6558-655A */
            z.a = (uint8_t)(z.a + z.d);        /* 655C */
        M(z.ix + 0x15) = z.a;                  /* 655D */
        z.a = z.h;                             /* 6560 */
        if (!(z.c & 0x02))                     /* 6561-6563 */
            z.a = (uint8_t)(z.a + z.d);        /* 6565 */
        M(z.ix + 0x0e) = z.a;                  /* 6566 */
        z.e = 0x01;                            /* 6569 */
        z.a = 0xc4;                            /* 656B */
        sub_4af6(&z);                          /* 656D */
        SET_DE(&z, 0x0030);                    /* 6570 */
        z.ix = (uint16_t)(z.ix + DE(&z));      /* 6573 */
        z.a = z.c;                             /* 6575 */
        z.c--;                                 /* 6576 */
        if (z.a == 0)                          /* 6577-6578 or a; jr nz */
            break;
    }
}

/* 65BF: HL = d460 + 32 · L (2529): la riga L del piano d400 con base d460. */
static void blocco_65bf(struct z80 *z)
{
    sub_2529(z);                               /* 65BF */
    SET_BC(z, 0xd460);                         /* 65C2 */
    SET_HL(z, HL(z) + BC(z));                  /* 65C5 */
}

/* 65C7: tile 01 in (HL) e attributo 70 nel piano basso (HL - 400). */
static void blocco_65c7(struct z80 *z)
{
    scrivi_mem(HL(z), 0x01);                   /* 65C7 */
    z->h &= (uint8_t)~0x04;                    /* 65C9 res 2,h */
    scrivi_mem(HL(z), 0x70);                   /* 65CB */
    z->h |= 0x04;                              /* 65CD set 2,h */
}

/* Passo 4, 657B: riquadro della cella c247 (2457: D = riga, E = colonna);
 * c2c1-c2c4 = D - 3, E, E + 1, D - 2 (251D); i quattro tile del riquadro
 * di 2×2 a d460 + 32 · (D - 3) + E - 1 (65BF, 65C7); i quattro oggetti da
 * c55e (passo 30) a zero, gestore 2 (4398), c467 = 0. */
static void passo_657b(void)
{
    struct z80 z = {0};
    z.a = M(0xc247);                           /* 657B */
    z.e = z.a;                                 /* 657E */
    sub_2457(&z);                              /* 657F */
    z.a = (uint8_t)(z.d - 0x03);               /* 6582-6583 */
    z.d = z.a;                                 /* 6585 */
    SET_HL(&z, 0xc2c1);                        /* 6586 */
    sub_251d(&z);                              /* 6589 */
    blocco_65bf(&z);                           /* 658C */
    z.e--;                                     /* 658F */
    z.d = 0x00;                                /* 6590 */
    z.b = 0x02;                                /* 6592 */
    SET_HL(&z, HL(&z) + DE(&z));               /* 6594 */
    blocco_65c7(&z);                           /* 6595 */
    SET_HL(&z, HL(&z) + 1);                    /* 6598 */
    blocco_65c7(&z);                           /* 6599 */
    SET_DE(&z, 0x0020);                        /* 659C */
    SET_HL(&z, HL(&z) + DE(&z));               /* 659F */
    blocco_65c7(&z);                           /* 65A0 */
    SET_HL(&z, HL(&z) - 1);                    /* 65A3 */
    blocco_65c7(&z);                           /* 65A4 */
    SET_BC(&z, 0x0400);                        /* 65A7 */
    SET_HL(&z, 0xc55e);                        /* 65AA */
    SET_DE(&z, 0x0030);                        /* 65AD */
    do {
        M(HL(&z)) = z.c;                       /* 65B0 */
        SET_HL(&z, HL(&z) + DE(&z));           /* 65B1 */
    } while (--z.b != 0);                      /* 65B2 djnz */
    z.a = 0x02;                                /* 65B4 */
    sub_4398(&z);                              /* 65B6 */
    z.a = 0x00;                                /* 65B9 */
    M(0xc467) = z.a;                           /* 65BA */
}

/* Passo 5, 65D0: resta sul passo finché c467 < 2; poi c467 = 0 e il
 * riquadro c2c1-c2c4 (riga alta, colonna sinistra, colonna destra, riga
 * bassa) si allarga di uno per lato (riga alta e colonna sinistra non
 * sotto 1, colonna destra fino a 1E, riga bassa fino a 18); ne disegna il
 * bordo con i tile di 65C7 (in alto verso destra, a destra verso il basso,
 * in basso verso sinistra, a sinistra verso l'alto). Con la larghezza
 * finale 1D e l'altezza 17 passa al passo seguente. */
static void passo_65d0(void)
{
    struct z80 z = {0};
    M(0xc19d)--;                               /* 65D0 dec (hl) */
    SET_DE(&z, 0xc467);                        /* 65D1 */
    z.a = M(0xc467);                           /* 65D4 */
    if (z.a < 0x02)                            /* 65D5-65D8 cp 2; pop hl; ret c */
        return;
    z.a = 0x00;                                /* 65D9 */
    M(0xc467) = z.a;                           /* 65DA */
    SET_HL(&z, 0xc2c1);                        /* 65DB */
    z.b = 0x02;                                /* 65DE */
    do {
        z.a = (uint8_t)(M(HL(&z)) - 1);        /* 65E0-65E1 */
        if (z.a != 0)                          /* 65E2 */
            M(HL(&z)) = z.a;                   /* 65E4 */
        SET_HL(&z, HL(&z) + 1);                /* 65E5 */
    } while (--z.b != 0);                      /* 65E6 djnz */
    SET_BC(&z, 0x021f);                        /* 65E8 */
    do {
        z.a = (uint8_t)(M(HL(&z)) + 1);        /* 65EB-65EC */
        if (z.a != z.c)                        /* 65ED-65EE */
            M(HL(&z)) = z.a;                   /* 65F0 */
        SET_HL(&z, HL(&z) + 1);                /* 65F1 */
        z.c = 0x19;                            /* 65F2 */
    } while (--z.b != 0);                      /* 65F4 djnz */
    z.ix = 0xc2c1;                             /* 65F6 */
    z.l = M(z.ix + 0x00);                      /* 65FA */
    z.d = z.l;                                 /* 65FD */
    blocco_65bf(&z);                           /* 65FE */
    z.a = M(z.ix + 0x01);                      /* 6601 */
    z.e = z.a;                                 /* 6604 */
    z.a = (uint8_t)(z.a + z.l);                /* 6605 */
    z.l = z.a;                                 /* 6606 */
    z.a = (uint8_t)(M(z.ix + 0x02) - z.e);     /* 6607-660A */
    z.e = z.a;                                 /* 660B */
    z.b = z.a;                                 /* 660C */
    do {
        blocco_65c7(&z);                       /* 660D */
        SET_HL(&z, HL(&z) + 1);                /* 6610 */
    } while (--z.b != 0);                      /* 6611 djnz */
    z.a = (uint8_t)(M(z.ix + 0x03) - z.d);     /* 6613-6616 */
    z.c = z.a;                                 /* 6617 */
    z.b = z.a;                                 /* 6618 */
    do {
        blocco_65c7(&z);                       /* 6619 */
        SET_HL(&z, HL(&z) + 0x20);             /* 661C-6622 */
    } while (--z.b != 0);                      /* 6623 djnz */
    z.b = z.e;                                 /* 6625 */
    z.a = z.e;                                 /* 6626 */
    do {
        blocco_65c7(&z);                       /* 6627 */
        SET_HL(&z, HL(&z) - 1);                /* 662A */
    } while (--z.b != 0);                      /* 662B djnz */
    z.b = z.c;                                 /* 662D */
    SET_DE(&z, 0xffe0);                        /* 662E */
    do {
        blocco_65c7(&z);                       /* 6631 */
        SET_HL(&z, HL(&z) + DE(&z));           /* 6634 */
    } while (--z.b != 0);                      /* 6635 djnz */
    if (z.a != 0x1d)                           /* 6637-6639 */
        return;
    if (z.c != 0x17)                           /* 663B-663E */
        return;
    M(0xc19d)++;                               /* 663F-6642 */
}

/* Passo 7, 6644: bit 2 di c09b spento, record 3 da 6653 (3569). */
static void passo_6644(void)
{
    struct z80 z = {0};
    SET_HL(&z, 0xc09b);                        /* 6644 */
    M(0xc09b) &= (uint8_t)~0x04;               /* 6647 */
    z.a = 0x03;                                /* 6649 */
    SET_HL(&z, 0x6653);                        /* 664B */
    sub_3569(&z);                              /* 664E */
}

/* 15, ingresso 6466: passo da c19d (`rst 20h` 646B). */
void attivita_15_6466(uint8_t slot)
{
    (void)slot;                                /* ogni passo: pop hl, slot invariato */
    uint8_t passo = M(0xc19d);                 /* 6466-6469 */
    M(0xc19d) = (uint8_t)(passo + 1);          /* 646A */
    switch (passo) {                           /* 646B rst 20h */
    case 0: passo_64d3(); break;
    case 1: attesa_64ce(0x06); break;          /* 64B4 */
    case 2: passo_6539(); break;
    case 3: attesa_64ce(0x14); break;          /* 64C4 */
    case 4: passo_657b(); break;
    case 5: passo_65d0(); break;
    case 6: attesa_64ce(0x48); break;          /* 64C8 */
    case 7: passo_6644(); break;
    case 8: attesa_64ce(0x78); break;          /* 64CC */
    case 9: passo_6655(); break;
    default: nemici_fuori_tabella(0x646b, passo); break;
    }
}
