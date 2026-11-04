/* Corpo 07 (2CCD), fasi 0 (3861), 1 (34FA) e 3 (37D7): lotto H4b2 (brief
 * 46), tradotte dalle istruzioni davvero eseguite nel giro dal boot
 * (out/titolo_istruzioni.txt). Ogni fase inizia con `pop hl` (puntatore allo
 * slot, inutile qui) e termina con `ret`: il rientro è il `return`.
 *
 * Punti di sincronia (NEMICI_PUNTO, nemici.h): l'intervallo [da, a] va dalla
 * prima istruzione dopo l'ultima scrittura in RAM fino all'istruzione che
 * scrive inclusa (o alla chiamata che scrive); nei cicli il punto sta
 * dentro il ciclo. Le scritture alle porte (e604) non contano. I rami mai
 * eseguiti nei frame 0-2410 chiamano nemici_manca(indirizzo). */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_attract07.h"

/* Fase 0, 3861: quadro di presentazione. c09a = 1A (le sette coppie di
 * 3A0E non scattano mai), barra (31E9), tabellone (349F), scritta (31CE),
 * suono 3F, palette (3F88 con HL = 80B3), spegne il bit 0 di c044, riempie
 * d840-d85f con 50, due stringhe a dc40 e dc4d (6771 con HL = 39C0), spegne
 * i bit 4-5 di c045, due tile 60 a d840, poi dalla tabella ROM 85EC il
 * quadro b e il resto c di c243: cifre a d611, d613 (e c245 = cifra + 1),
 * dfbf, c244 = b e, se b è cambiato, c251-c257 = FF; con i bit 1-2 di c045 spenti scrive una terza stringa
 * a dfb8 (6771 con HL = 39FA). */
void fase_07_3861(void)
{
    struct z80 z = {0};
    uint8_t a = M(0xc243);                     /* 3861 */
    if (a >= 0x24) {                           /* 3866-3868 */
        nemici_manca(0x386a);
        return;
    }
    for (int i = 0; i < 7; i++)                /* 386D-3876 */
        if (a == leggi((uint16_t)(0x3a0e + 2 * i))) {
            nemici_manca(0x3874);
            return;
        }
    NEMICI_PUNTO(0x3861, 0x3879);
    M(0xc09a) = 0x1a;                          /* 3879 */
    NEMICI_PUNTO(0x387c, 0x387f);
    z.a = M(0xc237);
    sub_31e9(&z);                              /* 387F */
    NEMICI_PUNTO(0x3882, 0x3882);
    sub_349f(&z);                              /* 3882 */
    NEMICI_PUNTO(0x3885, 0x3885);
    sub_31ce(&z);                              /* 3885 */
    NEMICI_PUNTO(0x3888, 0x388a);
    z.c = 0x3f;
    sub_0028(&z);                              /* 388A rst 28h */
    NEMICI_PUNTO(0x388b, 0x388e);
    SET_HL(&z, 0x80b3);
    sub_3f88(&z);                              /* 388E */
    NEMICI_PUNTO(0x3891, 0x3894);
    M(0xc044) &= 0xfe;                         /* 3894 res 0,(hl) */
    for (int i = 0; i < 0x20; i++) {           /* 3899-389E */
        if (i == 0)
            NEMICI_PUNTO(0x3896, 0x389c);
        else
            NEMICI_PUNTO(0x389c, 0x389e);
        M(0xd840 + i) = 0x50;                  /* 389C */
    }
    NEMICI_PUNTO(0x389d, 0x38a6);
    SET_HL(&z, 0x39c0);
    SET_DE(&z, 0xdc40);
    sub_6771(&z);                              /* 38A6 */
    NEMICI_PUNTO(0x38a9, 0x38ac);
    SET_DE(&z, 0xdc4d);
    sub_6771(&z);                              /* 38AC, HL prosegue */

    uint8_t b = M(0xc045);                     /* 38AF */
    if ((b & 0x06) == 0x06) {                  /* 38B3-38B7 */
        nemici_manca(0x38ba);
        return;
    }
    if (b & 0x01) {                            /* 38C3-38C8 */
        nemici_manca(0x38ca);
        return;
    }
    if (!(leggi_porta(0xe604) & 0x02)) {       /* 38D0-38D7 */
        nemici_manca(0x38d9);
        return;
    }
    scrivi_porta(0xe604, 0x00);                /* 38DC: c = 0, a = c & b */
    NEMICI_PUNTO(0x38af, 0x38ec);
    M(0xc045) = (uint8_t)(M(0xc045) & 0xcf);   /* 38EC (rr c: riporto 0) */
    NEMICI_PUNTO(0x38ed, 0x38ef);
    M(0xd840) = 0x60;                          /* 38EF */
    NEMICI_PUNTO(0x38f0, 0x38f2);
    M(0xd841) = 0x60;                          /* 38F2 */
    uint8_t q = M(0xc243);                     /* 38F4-38FB */
    if (q == leggi(0x82d3)) {                  /* 38FD */
        nemici_manca(0x3903);
        return;
    }
    if (q > leggi(0x82d3)) {                   /* 38FF */
        nemici_manca(0x3901);
        return;
    }
    uint16_t hl = 0x85ec;                      /* 3931-393B */
    uint8_t quadro = 0;
    while (q >= leggi(hl) && quadro < 0x20) {
        quadro++;
        hl++;
    }
    hl--;                                      /* 393D */
    uint8_t resto = (uint8_t)(q - leggi(hl));  /* 393E */
    NEMICI_PUNTO(0x38f3, 0x3947);
    SET_HL(&z, 0x39d9);
    SET_DE(&z, 0xd60c);
    sub_6771(&z);                              /* 3947 */
    NEMICI_PUNTO(0x394a, 0x3950);
    SET_HL(&z, 0x39cf);
    SET_DE(&z, 0xdfb8);
    sub_6771(&z);                              /* 3950 */
    z.a = quadro;
    sub_3a1c(&z);                              /* 3955 */
    if (z.d != 0x30) {                         /* 395C-395E */
        nemici_manca(0x3960);
        return;
    }
    NEMICI_PUNTO(0x3953, 0x3962);
    M(0xd611) = z.e;                           /* 3962 */
    z.a = resto;
    sub_3a1c(&z);                              /* 3964 */
    uint8_t cifra = (uint8_t)(z.e + 1);        /* 3967 inc e */
    NEMICI_PUNTO(0x3963, 0x396b);
    M(0xd613) = cifra;                         /* 396B */
    NEMICI_PUNTO(0x396c, 0x396d);
    M(0xc245) = cifra;                         /* 396D */
    if (M(0xc24e) & 0x02) {                    /* 3970-3975 */
        nemici_manca(0x398b);
        return;
    }
    z.a = (uint8_t)(M(0xc246) + 1);            /* 3977-397A */
    sub_3a1c(&z);                              /* 397B */
    if (z.d != 0x30) {                         /* 3981-3984 */
        nemici_manca(0x3986);
        return;
    }
    NEMICI_PUNTO(0x396e, 0x3988);
    M(0xdfbf) = z.e;                           /* 3988 */
    uint8_t prima = M(0xc244);                 /* 3992-3995 */
    NEMICI_PUNTO(0x3989, 0x3996);
    M(0xc244) = quadro;                        /* 3996 */
    if (prima != quadro) {                     /* 3997 jr z,39A3 */
        for (int i = 0; i < 7; i++) {          /* 3999-39A1 */
            if (i == 0)
                NEMICI_PUNTO(0x3997, 0x399e);
            else
                NEMICI_PUNTO(0x399e, 0x39a1);
            M(0xc251 + i) = 0xff;              /* 399E */
        }
    }
    NEMICI_PUNTO(0x39a0, 0x39b0);
    if (M(0xc045) & 0x06)                      /* 39A4-39A9 ret nz */
        return;
    SET_HL(&z, 0x39fa);
    SET_DE(&z, 0xdfb8);
    sub_6771(&z);                              /* 39B0 */
    NEMICI_PUNTO(0x39b3, 0x39b8);
    if (M(0xc243) == 0x33)                     /* 39B3-39B8 ret nz */
        nemici_manca(0x39b9);
}

/* Rami 352E-3547 della fase 1 (lotto 54, mai eseguiti nel giro, quindi
 * senza punti): se il byte c24b + c24a è FF, c24a diventa l'indice del
 * primo byte diverso da FF da c24b (353B); poi il record della stanza
 * (354B). */
static void fase_07_352e(struct z80 *z)
{
    uint16_t de = 0xc24b;                      /* 352E-3532 */
    if (M((uint16_t)(de + M(0xc24a))) == 0xff) {   /* 3533-3539 */
        uint8_t b = 0xff;                      /* 353B */
        uint8_t a;
        do {                                   /* 353D-3541 */
            a = M(de);
            de++;
            b++;
        } while (a == 0xff);
        M(0xc24a) = b;                         /* 3543-3544 */
    }
    sub_354b(z);                               /* 3547 */
}

/* Fase 1, 34FA: se c24e ha il bit 0 spento copia i 4 byte della tabella ROM
 * 8DD4 (indice c243) in c24a-c24d e cancella (FF) in c24b-c24d ogni byte
 * della lista c251 (fino a FF) che vi compare (351E, cpir su 3 byte); con
 * un byte diverso da FF alla posizione c24b + c24a carica il record della
 * stanza-bonus (354B), altrimenti prima sposta c24a (353B). Con c24e bit 0
 * acceso salta direttamente a 352E. I rami 352E-con-salto, 351E e 353B
 * sono del lotto 54 (mai eseguiti nel giro). */
void fase_07_34fa(void)
{
    struct z80 z = {0};
    if (M(0xc24e) & 0x01) {                    /* 34FB-3500 */
        fase_07_352e(&z);                      /* jr nz,352e */
        return;
    }
    uint16_t src = (uint16_t)(0x8dd4 + 4 * M(0xc243));   /* 3502-350D */
    for (int i = 0; i < 4; i++) {              /* 3514 ldir */
        if (i == 0)
            NEMICI_PUNTO(0x34fa, 0x3514);
        else
            NEMICI_PUNTO(0x3514, 0x3514);
        M(0xc24a + i) = leggi((uint16_t)(src + i));
    }
    NEMICI_PUNTO(0x3516, 0x3547);
    if (M(0xc251) != 0xff) {                   /* 3516-351C */
        uint16_t de = 0xc251;
        uint8_t a;
        while ((a = M(de++)) != 0xff) {        /* 3519-351C */
            for (uint16_t hl = 0xc24b; hl < 0xc24e; hl++) {   /* 351F-3527 cpir */
                if (M(hl) == a) {
                    M(hl) = 0xff;              /* 3529-352A */
                    break;
                }
            }
        }
        fase_07_352e(&z);
        return;
    }
    uint16_t p = (uint16_t)(0xc24b + M(0xc24a));   /* 352E-3536 */
    if (M(p) == 0xff) {                        /* 3537-3539 */
        fase_07_352e(&z);                      /* 353B, poi 3547 */
        return;
    }
    sub_354b(&z);                              /* 3547 */
    NEMICI_PUNTO(0x354a, 0x354a);
}

/* Fase 3, 37D7: griglia di c0bd. HL = 48·c243 + 1557 diventa IX; due
 * volte 16 byte FF (c0bd, c18d); due espansioni di mappe di bit da IX
 * (3812, C = B7 poi F8); infine le 224 celle di c0bd-c19c a zero
 * diventano 37. */
void fase_07_37d7(void)
{
    struct z80 z = {0};
    z.a = M(0xc243);
    sub_240b(&z);                              /* 37DA */
    SET_HL(&z, (uint16_t)(HL(&z) + 0x1557));   /* 37DD-37E0 */
    z.ix = HL(&z);                             /* 37E1-37E2 */
    NEMICI_PUNTO(0x37d7, 0x37e7);
    SET_HL(&z, 0xc0bd);
    sub_380a(&z);                              /* 37E7 */
    NEMICI_PUNTO(0x37ea, 0x37ed);
    SET_HL(&z, 0xc18d);
    sub_380a(&z);                              /* 37ED */
    NEMICI_PUNTO(0x37f0, 0x37f2);
    z.c = 0xb7;
    sub_3812(&z);                              /* 37F2 */
    NEMICI_PUNTO(0x37f5, 0x37f7);
    z.c = 0xf8;
    sub_3812(&z);                              /* 37F7 */
    for (int i = 0; i < 0xe0; i++) {           /* 37FA-3806 */
        NEMICI_PUNTO(i == 0 ? 0x37fa : 0x37ff, 0x3806);
        if (M(0xc0bd + i) == 0)
            M(0xc0bd + i) = 0x37;              /* 3803 */
    }
    NEMICI_PUNTO(0x3805, 0x3809);
}
