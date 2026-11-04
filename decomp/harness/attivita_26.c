/* Corpo dell'attività 26 (3B63) del gioco libero, lotto 54 (brief 54):
 * macchina a passi sul contatore c19d (tabella `rst 20h` 3B68, 7 voci),
 * con la routine 6502 e la voce 3D59 condivisa con il corpo 25 (3D4C vi
 * cade dentro). Tradotto dal disassemblato out/main64.dasm; regole come il
 * 07 (attivita_attract07.c): HL = c19d, A = passo, c19d avanza di uno; i
 * passi finiscono con `pop hl; ret` (slot invariato). Il passo 6 (6655)
 * chiama 75AA, che dura più di un frame: non si cattura in MAME e si
 * verifica per composizione (75AA, 673F, 0493 nel banco). */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"

/* 0x6502: c46e = 0, tabella delle attività vuota (673F), posta 14, bit 0
 * di c044 spento, i 15 oggetti da c55e (passo 30) a zero; poi da c46e
 * cinque oggetti agganciati con 2F74 da HL = 7858 (C = 0-3, B = 5, poi
 * C = 3, B = 4). */
void sub_6502(struct z80 *z)
{
    z->a = 0x00;                               /* 6502 xor a */
    M(0xc46e) = z->a;                          /* 6503 */
    sub_673f(z);                               /* 6506 */
    z->a = 0x14;                               /* 6509 */
    sub_0493(z);                               /* 650B */
    M(0xc044) &= (uint8_t)~0x01;               /* 650E-6511 */
    SET_HL(z, 0xc55e);                         /* 6513 */
    SET_DE(z, 0x0030);                         /* 6516 */
    z->b = 0x0f;                               /* 6519 */
    do {
        M(HL(z)) = z->d;                       /* 651B */
        SET_HL(z, HL(z) + DE(z));              /* 651C */
    } while (--z->b != 0);                     /* 651D djnz */
    z->ix = 0xc46e;                            /* 651F */
    SET_BC(z, 0x0500);                         /* 6523 */
    SET_HL(z, 0x7858);                         /* 6526 */
    do {
        sub_2f74(z);                           /* 6529 */
        z->c++;                                /* 652C */
        z->a = z->c;                           /* 652D */
    } while (z->a != 0x04);                    /* 652E-6530 */
    SET_BC(z, 0x0403);                         /* 6532 */
    sub_2f74(z);                               /* 6535 */
}

/* Passo 0, 3E34: 6502, oggetto c88e (3DEA), c82e = 80, c93e = 1B, suono
 * 3A, posta 26. */
static void passo_3e34(void)
{
    struct z80 z = {0};
    sub_6502(&z);                              /* 3E34 */
    sub_3dea(&z);                              /* 3E37 */
    M(0xc82e) = 0x80;                          /* 3E3A-3E3C */
    M(0xc93e) = 0x1b;                          /* 3E3F-3E41 */
    z.c = 0x3a;                                /* 3E44 */
    sub_0028(&z);                              /* 3E46 rst 28h */
    z.a = 0x26;                                /* 3E47 */
    sub_0493(&z);                              /* 3E49 */
}

/* Passo 2, 3E0E: Dana (c82e, bit 6 spento) verso la cella c247: passi
 * (2FF3) dalle coordinate di Dana a quelle della cella (4B0E), 3DA6, stato
 * C0 (4AF6, BC = 0401, E = 40). */
static void passo_3e0e(void)
{
    struct z80 z = {0};
    z.a = M(0xc247);                           /* 3E0E */
    sub_4b0e(&z);                              /* 3E11 */
    z.ix = 0xc82e;                             /* 3E14 */
    M(z.ix) &= (uint8_t)~0x40;                 /* 3E18 res 6,(ix+0) */
    z.d = M(z.ix + 0x0e);                      /* 3E1C */
    z.e = M(z.ix + 0x15);                      /* 3E1F */
    sub_2ff3(&z);                              /* 3E22 */
    sub_3da6(&z);                              /* 3E25 */
    SET_BC(&z, 0x0401);                        /* 3E28 */
    z.e = 0x40;                                /* 3E2B */
    z.a = 0xc0;                                /* 3E2D */
    sub_4af6(&z);                              /* 3E2F */
}

/* Voce 3D59 (passo 4 del 26; coda del passo 5 del 25, 3D4C): c82e = 0,
 * bit 1 acceso in c87c e c8ac. */
void passo_3d59(void)
{
    M(0xc82e) = 0x00;                          /* 3D59-3D5A */
    M(0xc87c) |= 0x02;                         /* 3D60-3D63 */
    M(0xc87c + 0x30) |= 0x02;                  /* 3D65-3D66 */
}

/* Passo 6, 6655: azzeramenti di cambio quadro (75AA), tabella delle
 * attività vuota (673F), posta 14, suono 3F; c237 scende di uno e si posta
 * 17 (16 se arriva a zero); c19d = 0. */
void passo_6655(void)
{
    struct z80 z = {0};
    sub_75aa(&z);                              /* 6655 */
    sub_673f(&z);                              /* 6658 */
    z.a = 0x14;                                /* 665B */
    sub_0493(&z);                              /* 665D */
    z.c = 0x3f;                                /* 6660 */
    sub_0028(&z);                              /* 6662 rst 28h */
    M(0xc237)--;                               /* 6663-6666 */
    z.a = M(0xc237) != 0 ? 0x17 : 0x16;        /* 6668-666C */
    sub_0493(&z);                              /* 666E */
    M(0xc19d) = 0x00;                          /* 6671-6672 */
}

/* 26, ingresso 3B63: passo da c19d (`rst 20h` 3B68). */
void attivita_26_3b63(uint8_t slot)
{
    uint8_t passo = M(0xc19d);                 /* 3B63-3B66 */
    M(0xc19d) = (uint8_t)(passo + 1);          /* 3B67 */
    (void)slot;
    switch (passo) {                           /* 3B68 rst 20h */
    case 0: passo_3e34(); break;
    case 1: case 3: case 5: passo_2e79(0xc19d); break;
    case 2: passo_3e0e(); break;
    case 4: passo_3d59(); break;
    case 6: passo_6655(); break;
    default: nemici_fuori_tabella(0x3b68, passo); break;
    }
}
