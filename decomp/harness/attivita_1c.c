/* Corpo dell'attività 1C (3F40) del gioco libero, lotto 54 (brief 54):
 * macchina a passi sul contatore c19d (tabella `rst 20h` 3F45, 9 voci),
 * postata dalla raccolta della classe 2 (1332, raccolta_12ab). Tradotto
 * dal disassemblato out/main64.dasm; regole come il 07
 * (attivita_attract07.c): HL = c19d, A = passo, c19d avanza di uno; i
 * passi finiscono con `pop hl; ret` (405F con `pop bc`), slot invariato.
 * I passi 4 (3F58) e 8 (3FF7) chiamano 75AA, che dura più di un frame:
 * non si catturano in MAME e si verificano per composizione. */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"
#include "attivita_attract07.h"

/* Passo 0, 40E1: bit 0 di c044 spento; le attività 15 e 20 della tabella
 * (6735) si azzerano; i 17 oggetti da c4fe (passo 30) spenti e il bit 6
 * di c82e spento; 33BA (C = 1) e barra 31E9 (A = c237). */
static void passo_40e1(void)
{
    struct z80 z = {0};
    M(0xc044) &= (uint8_t)~0x01;               /* 40E1-40E4 */
    SET_DE(&z, 0x0200);                        /* 40E6 */
    z.a = 0x15;                                /* 40E9 */
    do {
        sub_6735(&z);                          /* 40EB */
        if (z.f & LIBERA_ZERO)                 /* 40EE jr nz */
            M(HL(&z)) = z.e;                   /* 40F0 */
        z.a = (uint8_t)(z.a + 0x0b);           /* 40F1 */
    } while (--z.d != 0);                      /* 40F3-40F4 */
    SET_HL(&z, 0xc4fe);                        /* 40F6 */
    SET_DE(&z, 0x0030);                        /* 40F9 */
    z.b = 0x11;                                /* 40FC */
    do {
        M(HL(&z)) = z.d;                       /* 40FE */
        SET_HL(&z, HL(&z) + DE(&z));           /* 40FF */
    } while (--z.b != 0);                      /* 4100 djnz */
    M(HL(&z)) &= (uint8_t)~0x40;               /* 4102 res 6,(hl): c82e */
    z.c = 0x01;                                /* 4104 */
    sub_33ba(&z);                              /* 4106 */
    z.a = M(0xc237);                           /* 4109 */
    sub_31e9(&z);                              /* 410C */
}

/* Passo 2, 40A7: c82e = 0; dalla cella c247 i 7 byte di c2c1 (2430) e
 * i passi (2FF3) da 8880 alle coordinate della cella in c2cb e c2cd; le
 * attività 0A e 0D della tabella (6735) si azzerano (se mancano si
 * azzera c03f, come il Z80: `ld (hl),e` senza condizione); 3045, c463 = 0. */
static void passo_40a7(void)
{
    struct z80 z = {0};
    M(0xc82e) = 0x00;                          /* 40A7-40A8 */
    z.a = M(0xc247);                           /* 40AB */
    z.b = z.a;                                 /* 40AE */
    sub_4b0e(&z);                              /* 40AF */
    SET_DE(&z, 0xc2c1);                        /* 40B2 */
    sub_2430(&z);                              /* 40B5: A = B */
    sub_4b0e(&z);                              /* 40B8 */
    SET_DE(&z, HL(&z));                        /* 40BB-40BF push hl; pop de */
    SET_HL(&z, 0x8880);                        /* 40BC */
    sub_2ff3(&z);                              /* 40C0 */
    M(0xc2cb) = z.l;                           /* 40C3 */
    M(0xc2cc) = z.h;
    M(0xc2cd) = z.e;                           /* 40C6 */
    M(0xc2ce) = z.d;
    z.a = 0x0a;                                /* 40CA */
    z.e = 0x00;                                /* 40CC */
    sub_6735(&z);                              /* 40CE */
    M(HL(&z)) = z.e;                           /* 40D1 */
    z.a = 0x0d;                                /* 40D2 */
    sub_6735(&z);                              /* 40D4 */
    M(HL(&z)) = z.e;                           /* 40D7 */
    sub_3045(&z);                              /* 40D8 */
    M(0xc463) = 0x00;                          /* 40DB-40DC */
}

/* Passo 3, 405F: resta sul passo; con c467 diverso da zero lo azzera e,
 * se c463 < 40, muove il cerchio (3097), fa avanzare la fase c2c5 (modulo
 * 40) e la parola c2c6-c2c7 (byte alto prima) di 0150, e somma due volte
 * (2425) le parole di c2cb a c2c1; con c463 >= 40 spegne i 16 oggetti da
 * c4fe (passo 30) e passa al passo 4. */
static void passo_405f(void)
{
    struct z80 z = {0};
    M(0xc19d)--;                               /* 405F dec (hl) */
    if (M(0xc467) == 0)                        /* 4060-4066 ret z */
        return;
    M(0xc467) = 0x00;                          /* 4067 */
    if (M(0xc463) >= 0x40) {                   /* 4069-406E */
        uint16_t hl = 0xc4fe;                  /* 4096 */
        for (int b = 0x10; b > 0; b--) {       /* 409C-40A0 */
            M(hl) = 0x00;
            hl = (uint16_t)(hl + 0x30);
        }
        M(0xc19d)++;                           /* 40A2-40A5 */
        return;
    }
    sub_3097(&z);                              /* 4070 */
    M(0xc2c5) = (uint8_t)((M(0xc2c5) + 1) & 0x3f);   /* 4073-407A */
    uint16_t v = (uint16_t)(((M(0xc2c6) << 8) | M(0xc2c7)) + 0x0150);   /* 407B-4084 */
    M(0xc2c7) = (uint8_t)v;                    /* 4085 */
    M(0xc2c6) = (uint8_t)(v >> 8);             /* 4087 */
    SET_HL(&z, 0xc2c1);                        /* 4088 */
    SET_DE(&z, 0xc2cb);                        /* 408B */
    z.b = 0x02;                                /* 408E */
    do {
        sub_2425(&z);                          /* 4090 */
    } while (--z.b != 0);                      /* 4093 djnz */
}

/* Passo 4, 3F58: azzeramenti di cambio quadro (75AA) e le scritte 3F6F
 * a d5c7 e 3F84 a d612 (6771). */
static void passo_3f58(void)
{
    struct z80 z = {0};
    sub_75aa(&z);                              /* 3F58 */
    SET_HL(&z, 0x3f6f);                        /* 3F5B */
    SET_DE(&z, 0xd5c7);                        /* 3F5E */
    sub_6771(&z);                              /* 3F61 */
    SET_HL(&z, 0x3f84);                        /* 3F64 */
    SET_DE(&z, 0xd612);                        /* 3F67 */
    sub_6771(&z);                              /* 3F6A */
}

/* Passo 6, 3F93: palette 80B3 (3F88), suono 22, punteggio del tempo
 * (3FB2), 4C89, punteggio scomposto (4D87, HL = d610, C = 11), 4D05. */
static void passo_3f93(void)
{
    struct z80 z = {0};
    SET_HL(&z, 0x80b3);                        /* 3F93 */
    sub_3f88(&z);                              /* 3F96 */
    z.c = 0x22;                                /* 3F99 */
    sub_0028(&z);                              /* 3F9B rst 28h */
    sub_3fb2(&z);                              /* 3F9C */
    sub_4c89(&z);                              /* 3F9F */
    SET_DE(&z, 0xc061);                        /* 3FA2 */
    SET_HL(&z, 0xd610);                        /* 3FA5 */
    z.c = 0x11;                                /* 3FA8 */
    sub_4d87(&z);                              /* 3FAA */
    sub_4d05(&z);                              /* 3FAD */
}

/* Passo 8, 3FF7: palette 80D3 (3F88), tabella delle attività vuota (673F),
 * azzeramenti (75AA), posta 14. In attract (bit 1-2 di c045 spenti) posta
 * 17; altrimenti la stanza c243 va nel primo posto FF della lista c251, e
 * il byte e = (c24b + c24a) sceglie la stanza seguente: c243 = e & 7F,
 * con il bit 7 bit 1 di c24e acceso e c24f + 1 (altrimenti c246 + 1);
 * con e = 32 bit 7 di c24e acceso e posta 24, altrimenti posta 07. Poi
 * c299 = c19d = 0 e c248 = FF. */
static void passo_3ff7(void)
{
    struct z80 z = {0};
    SET_HL(&z, 0x80d3);                        /* 3FF7 */
    sub_3f88(&z);                              /* 3FFA */
    sub_673f(&z);                              /* 3FFD */
    sub_75aa(&z);                              /* 4000 */
    z.a = 0x14;                                /* 4003 */
    sub_0493(&z);                              /* 4005 */
    if (!(M(0xc045) & 0x06)) {                 /* 4008-400D */
        z.c = 0x17;                            /* 400F */
    } else {
        uint8_t e = M((uint16_t)(0xc24b + M(0xc24a)));   /* 4013-401B */
        uint16_t hl = 0xc250;                  /* 401C */
        do
            hl++;                              /* 401F */
        while (M(hl) != 0xff);                 /* 4020-4022 */
        M(hl) = M(0xc243);                     /* 4024-4027 */
        z.c = 0x07;                            /* 402E */
        if (e == 0x32) {                       /* 402C, 4030 */
            z.c = 0x24;                        /* 4032 */
            M(0xc24e) |= 0x80;                 /* 4034 */
        }
        M(0xc24e) &= (uint8_t)~0x03;           /* 4036-4038 */
        if (e & 0x80) {                        /* 403A-403B add a,a */
            M(0xc24e) |= 0x02;                 /* 403D */
            M(0xc24f)++;                       /* 403F-4042 */
        } else {
            M(0xc246)++;                       /* 4045-4048 */
        }
        M(0xc243) = (uint8_t)(e & 0x7f);       /* 4049-404B */
    }
    z.a = z.c;                                 /* 404E */
    sub_0493(&z);                              /* 404F */
    M(0xc299) = 0x00;                          /* 4052-4053 */
    M(0xc19d) = 0x00;                          /* 4056 */
    M(0xc248) = 0xff;                          /* 4059-405A */
}

/* 1C, ingresso 3F40: passo da c19d (`rst 20h` 3F45). */
void attivita_1c_3f40(uint8_t slot)
{
    uint8_t passo = M(0xc19d);                 /* 3F40-3F43 */
    M(0xc19d) = (uint8_t)(passo + 1);          /* 3F44 */
    (void)slot;
    switch (passo) {                           /* 3F45 rst 20h */
    case 0: passo_40e1(); break;
    case 1: case 5: attesa_07(0xc19d, 0x14, 0x41a8); break;
    case 2: passo_40a7(); break;
    case 3: passo_405f(); break;
    case 4: passo_3f58(); break;
    case 6: passo_3f93(); break;
    case 7: attesa_07(0xc19d, 0xc0, 0x41b2); break;
    case 8: passo_3ff7(); break;
    default: nemici_fuori_tabella(0x3f45, passo); break;
    }
}
