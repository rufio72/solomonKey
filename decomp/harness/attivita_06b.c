/* Fasi 5 (7A0B), 8 (7B3B) e 11 (7604) del corpo 06 dell'attract (75E7, tabella 75EC
 * sul contatore c1ae), lotto 55 (brief 55, domanda 1): erano rami
 * nemici_manca, mai eseguiti nei frame 0-2410 del giro dal boot; il gioco
 * libero ci arriva dopo il fine partita (attract con la classifica). Si
 * traducono dal disassemblato out/main64.dasm senza punti di sincronia
 * (NEMICI_PUNTO): il giro dal boot non li raggiunge. Entrambe chiamano
 * 75AA e durano più di un frame: verificate per composizione (75AA, 322A,
 * 33BA, 6771, 7A56, 3A54, 0493 nel banco o già nel giro). La tabella 75EC
 * ha 12 voci, non 10 come dichiara rst20_tables.h: la 10 è 760B e la 11 è
 * 7604, il codice subito dopo la tabella, che chiude il ciclo. */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_attract07.h"
#include "attivita_libera.h"

/* Fase 5, 7A0B: c1ae + 1, 75AA, fondale della stanza 32 (322A, c243 = 32),
 * 33BA con C = 1, scritte 7A3A a d4aa e d4ca (6771), classifica (7A56),
 * attributo 50 su d102-d11d. */
void fase_06_7a0b(void)
{
    struct z80 z = {0};
    M(0xc1ae)++;                               /* 7A0B inc (hl) */
    sub_75aa(&z);                              /* 7A0C */
    z.a = 0x32;                                /* 7A0F */
    M(0xc243) = z.a;                           /* 7A11 */
    fondale_322a(&z);                          /* 7A14 */
    z.c = 0x01;                                /* 7A17 */
    sub_33ba(&z);                              /* 7A19 */
    SET_HL(&z, 0x7a3a);                        /* 7A1C */
    SET_DE(&z, 0xd4aa);                        /* 7A1F */
    sub_6771(&z);                              /* 7A22 */
    SET_DE(&z, 0xd4ca);                        /* 7A25 */
    sub_6771(&z);                              /* 7A28 */
    sub_7a56(&z);                              /* 7A2B */
    SET_HL(&z, 0xd102);                        /* 7A2E */
    z.b = 0x1c;                                /* 7A31 */
    do {
        M(HL(&z)) = 0x50;                      /* 7A33 */
        SET_HL(&z, HL(&z) + 1);                /* 7A35 */
    } while (--z.b != 0);                      /* 7A36 djnz */
}

/* 7B95: tile C in (HL), attributo 04 nel piano basso, HL - 1. */
static void blocco_7b95(struct z80 *z)
{
    M(HL(z)) = z->c;                           /* 7B95 */
    z->h &= (uint8_t)~0x04;                    /* 7B96 res 2,h */
    M(HL(z)) = 0x04;                           /* 7B98 */
    z->h |= 0x04;                              /* 7B9A set 2,h */
    SET_HL(z, HL(z) - 1);                      /* 7B9C */
}

/* 7B9E: riquadri 2×2 (3A54) da una lista di coppie (tile, attributo) in
 * HL fino a FF, alla cella DE e poi due righe più in basso ciascuno. */
static void lista_7b9e(struct z80 *z)
{
    for (;;) {
        z->a = leggi(HL(z));                   /* 7B9E */
        SET_HL(z, HL(z) + 1);                  /* 7B9F */
        if (z->a == 0xff)                      /* 7BA0-7BA2 */
            return;
        z->b = z->a;                           /* 7BA3 */
        z->c = leggi(HL(z));                   /* 7BA4 */
        SET_HL(z, HL(z) + 1);                  /* 7BA5 */
        uint16_t hl = HL(z), de = DE(z);       /* 7BA6-7BA7 push hl, de */
        sub_3a54(z);                           /* 7BA8 */
        SET_DE(z, de);                         /* 7BAB-7BAC pop de, hl */
        SET_HL(z, hl);
        z->d = (uint8_t)(z->d + 2);            /* 7BAD-7BAE */
    }
}

/* Fase 8, 7B3B: c1ae + 1, 75AA, posta 23, fondale della stanza 32 (322A),
 * 33BA con C = 1; da d75b all'indietro 242 tile a serpentina (F2 in giù,
 * righe di 11 con salti FFFC e FFDA, 7B95), poi cinque liste di riquadri
 * da 7BB1 (7B9E) alle celle 050B 050D 051C 0D1A 191A. */
void fase_06_7b3b(void)
{
    struct z80 z = {0};
    M(0xc1ae)++;                               /* 7B3B inc (hl) */
    sub_75aa(&z);                              /* 7B3C */
    z.a = 0x23;                                /* 7B3F */
    sub_0493(&z);                              /* 7B41 */
    z.a = 0x32;                                /* 7B44 */
    M(0xc243) = z.a;                           /* 7B46 */
    fondale_322a(&z);                          /* 7B49 */
    z.c = 0x01;                                /* 7B4C */
    sub_33ba(&z);                              /* 7B4E */
    SET_HL(&z, 0xd75b);                        /* 7B51 */
    z.c = 0xf2;                                /* 7B54 */
    for (;;) {
        z.b = 0x0b;                            /* 7B56 */
        do {
            blocco_7b95(&z);                   /* 7B58 */
            z.c--;                             /* 7B5B */
        } while (--z.b != 0);                  /* 7B5C djnz */
        SET_HL(&z, HL(&z) + 0xfffc);           /* 7B5E-7B61 */
        z.b = 0x0b;                            /* 7B62 */
        int finito = 0;
        do {
            blocco_7b95(&z);                   /* 7B64 */
            if (--z.c == 0) {                  /* 7B67-7B68 jr z */
                finito = 1;
                break;
            }
        } while (--z.b != 0);                  /* 7B6A djnz */
        if (finito)
            break;
        SET_HL(&z, HL(&z) + 0xffda);           /* 7B6C-7B6F */
    }
    static const uint16_t celle[5] = { 0x050b, 0x050d, 0x051c, 0x0d1a, 0x191a };
    SET_HL(&z, 0x7bb1);                        /* 7B72 */
    for (int i = 0; i < 5; i++) {              /* 7B75-7B90 */
        SET_DE(&z, celle[i]);
        lista_7b9e(&z);
    }
}

/* Fase 11, 7604: c467 = 0 e c1ae = 0 (HL = c1ae): l'attract ricomincia. */
void fase_06_7604(void)
{
    M(0xc467) = 0x00;                          /* 7604-7605 xor a; ld (c467),a */
    M(0xc1ae) = 0x00;                          /* 7608 ld (hl),a */
}
