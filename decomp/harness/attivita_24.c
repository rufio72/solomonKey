/* Corpo dell'attività 24 (2C72) del gioco libero, lotto 55 (brief 55): il
 * finale, macchina a passi sul contatore c19d (tabella `rst 20h` 2C77, 19
 * voci) posta da 6464 (corpo 17) e da 4032. Le voci sono quasi tutte fasi
 * del corpo 07 (attivita_attract07*.c), chiamate con gli stessi argomenti
 * dei `case` di attivita_07_2ccd; l'unica nuova è 2C9E. I punti di
 * sincronia delle fasi del 07 (NEMICI_PUNTO) fuori dal giro dell'attract
 * non fanno nulla. */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_attract07.h"
#include "attivita_libera.h"

/* Passo 7, 2C9E: le celle c248 e c247 della stanza (c0bd + cella) a 37;
 * l'oggetto c85e alle coordinate della cella c247 (4B0E) con lo stato C5
 * (4AF6, B = 7, E = A1); fondale di gioco (2F57). */
static void passo_2c9e(void)
{
    struct z80 z = {0};
    z.a = M(0xc248);                           /* 2C9E */
    SET_BC(&z, 0x0700);                        /* 2CA1 */
    z.l = z.a;                                 /* 2CA4 */
    z.h = z.c;                                 /* 2CA5 */
    SET_DE(&z, 0xc0bd);                        /* 2CA6 */
    SET_HL(&z, HL(&z) + DE(&z));               /* 2CA9 */
    M(HL(&z)) = 0x37;                          /* 2CAA */
    z.a = M(0xc247);                           /* 2CAC */
    z.l = z.a;                                 /* 2CAF */
    z.h = z.c;                                 /* 2CB0 */
    SET_HL(&z, HL(&z) + DE(&z));               /* 2CB1 */
    M(HL(&z)) = 0x37;                          /* 2CB2 */
    sub_4b0e(&z);                              /* 2CB4 */
    z.ix = 0xc85e;                             /* 2CB7 */
    M(z.ix + 0x0e) = z.h;                      /* 2CBB */
    M(z.ix + 0x15) = z.l;                      /* 2CBE */
    z.e = 0xa1;                                /* 2CC1 */
    z.a = 0xc5;                                /* 2CC3 */
    sub_4af6(&z);                              /* 2CC5 */
    sub_2f57(&z);                              /* 2CC8 */
}

/* 24, ingresso 2C72: passo da c19d (`rst 20h` 2C77). */
void attivita_24_2c72(uint8_t slot)
{
    uint8_t passo = M(0xc19d);                 /* 2C72-2C75 */
    M(0xc19d) = (uint8_t)(passo + 1);          /* 2C76 */
    switch (passo) {                           /* 2C77 rst 20h */
    case 0:  fase_07_3861(); break;
    case 1:  fase_2dee(); break;
    case 2:  fase_07_37d7(); break;
    case 3:  fase_07_358f(); break;
    case 4:  fase_3225(); break;
    case 5:  fase_07_3a7d(); break;
    case 6:  attesa_07(0xc19d, 0x60, 0x2e89); break;
    case 7:  passo_2c9e(); break;
    case 8:  attesa_07(0xc19d, 0x0d, 0x2e5d); break;
    case 9:  fase_07_2dce(); break;
    case 10: fase_07_2e9c(); break;
    case 11: fase_07_3709(); break;
    case 12: fase_07_3829(); break;
    case 13: fase_07_321e(); break;
    case 14: fase_07_31c5(); break;
    case 15: fase_07_2ef5(); break;
    case 16: attesa_07(0xc19d, 0x1e, 0x2e6d); break;
    case 17: fase_07_2f1a(); break;
    case 18: fase_07_2e1e(slot); break;
    default: nemici_fuori_tabella(0x2c77, passo); break;
    }
}
