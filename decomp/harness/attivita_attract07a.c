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

/* Fase 0, 3861: quadro di presentazione. c09a = 1A, 1B con c243 >= 24
 * (386A inc c), oppure il secondo byte della coppia di 3A0E il cui primo
 * byte vale c243 (3874, prima stanza di ogni quadro dal 5: 13 o 14), barra
 * (31E9), tabellone (349F), scritta (31CE), suono 3F, palette (3F88 con
 * HL = 80B3), spegne il bit 0 di c044, riempie d840-d85f con 50, due
 * stringhe a dc40 e dc4d (6771 con HL = 39C0) e, con i bit 1-2 di c045
 * entrambi accesi, una terza a dc55 (38BA, HL = 39CB). Il bit 0 di c045
 * sceglie la cella dei due tile 60: d855 (38CD) invece di d840. Scrive in
 * e604 (bit 1 di e604 spento → 1, 38D9) AND c045 e porta il bit 0 del
 * risultato nel bit 5 di c045 (bit 4-5 spenti prima). Poi dalla tabella
 * ROM 85EC il quadro b e il resto c di c243: cifre a d611, d613 (e c245 =
 * cifra + 1), a dfbe-dfbf il numero della stanza c246 + 1 (decine solo se
 * diverse da '0', 3986) oppure, con il bit 1 di c24e, la parola D7D6
 * (398B); c244 = b e, se b è cambiato, c251-c257 = FF; con i bit 1-2 di
 * c045 spenti scrive una stringa a dfb8 (6771 con HL = 39FA) e, per
 * c243 = 33, «  DEMO  » a d60c (39B9, HL = 3A04). Con c243 nella tabella
 * 82D3 (stanze bonus/finale, lotto 59) accende il bit 4 di c045 (non per
 * 32), scrive «BONUS» o «FINAL» a d5ca e «PRISON!» a d5d0 e, salvo 32,
 * c09a = 1E; con il quadro >= 10 scrive la cifra delle decine a d610
 * (3960). I rami fuori dal giro (lotti 59 e 65) non hanno punti di
 * sincronia. */
void fase_07_3861(void)
{
    struct z80 z = {0};
    uint8_t a = M(0xc243);                     /* 3861 */
    uint8_t c = (a >= 0x24) ? 0x1b : 0x1a;     /* 3864-386A (inc c) */
    for (int i = 0; i < 7; i++)                /* 386D-3876: nessuna uscita */
        if (a == leggi((uint16_t)(0x3a0e + 2 * i)))
            c = leggi((uint16_t)(0x3a0e + 2 * i + 1));   /* 3874 ld c,(hl) */
    NEMICI_PUNTO(0x3861, 0x3879);
    M(0xc09a) = c;                             /* 3879 (ld a,c) */
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
        SET_HL(&z, 0x39cb);                    /* 38BA */
        SET_DE(&z, 0xdc55);
        sub_6771(&z);                          /* 38C0 (B non toccato) */
    }
    uint16_t cella = (b & 0x01) ? 0xd855 : 0xd840;   /* 38C3-38CD */
    uint8_t cc = (leggi_porta(0xe604) & 0x02) ? 0x00 : 0x01;   /* 38D0-38D9 */
    uint8_t p = (uint8_t)(cc & b);             /* 38DA-38DB */
    scrivi_porta(0xe604, p);                   /* 38DC */
    NEMICI_PUNTO(0x38af, 0x38ec);
    M(0xc045) = (uint8_t)((M(0xc045) & 0xcf)   /* 38E0-38EC: rr c, or 20 */
                          | ((p & 0x01) ? 0x20 : 0x00));
    NEMICI_PUNTO(0x38ed, 0x38ef);
    M(cella) = 0x60;                           /* 38EF */
    NEMICI_PUNTO(0x38f0, 0x38f2);
    M((uint16_t)(cella + 1)) = 0x60;           /* 38F2 */
    uint8_t q = M(0xc243);                     /* 38F4-38F7 */
    uint16_t t = 0x82d3;                       /* 38F8: 03 0C 15 1F 28 32 FF */
    int uguale = 0;
    for (;;) {                                 /* 38FB-3901 */
        uint8_t v = leggi(t++);                /* 38FB cp (hl); 38FC inc hl */
        if (q == v) {                          /* 38FD jr z,3903 */
            uguale = 1;
            break;
        }
        if (q < v)                             /* 38FF jr c,3931 */
            break;
    }                                          /* 3901 jr 38FB (FF chiude) */
    if (uguale) {                              /* 3903-392F: bonus/finale */
        uint8_t bb = 0x10;                     /* 3904 */
        uint16_t s = 0x39e3;                   /* 3906 «BONUS» */
        if (q == 0x32) {                       /* 3909-390B */
            bb = 0x00;                         /* 390D */
            s = 0x39ea;                        /* 390F «FINAL» */
        }
        M(0xc045) = (uint8_t)(M(0xc045) | bb); /* 3912-3914: E = c045 di 38EC */
        SET_HL(&z, s);
        SET_DE(&z, 0xd5ca);
        sub_6771(&z);                          /* 391A */
        SET_HL(&z, 0x39f1);                    /* 391D «PRISON!» */
        SET_DE(&z, 0xd5d0);
        sub_6771(&z);                          /* 3923 */
        if (bb)                                /* 3926-3928 */
            M(0xc09a) = 0x1e;                  /* 392C */
    }                                          /* 392F pop bc; 3930 ld a,c */
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
    if (z.d != 0x30)                           /* 395C-395E */
        M(0xd610) = z.d;                       /* 3960 ld (hl),d (HL = D610) */
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
        M(0xdfbe) = 0xd6;                      /* 398B-398E ld (dfbe),hl */
        M(0xdfbf) = 0xd7;                      /* HL = D7D6, poi 3991 */
    } else {
        z.a = (uint8_t)(M(0xc246) + 1);        /* 3977-397A */
        sub_3a1c(&z);                          /* 397B */
        if (z.d != 0x30)                       /* 3981-3984 */
            M(0xdfbe) = z.d;                   /* 3986 */
        NEMICI_PUNTO(0x396e, 0x3988);
        M(0xdfbf) = z.e;                       /* 3988, poi 3989 jr 3991 */
    }
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
    if (M(0xc243) == 0x33) {                   /* 39B3-39B8 ret nz */
        SET_DE(&z, 0xd60c);                    /* 39B9: HL = 3A04, da 39B0 */
        sub_6771(&z);                          /* 39BC «  DEMO  » */
    }                                          /* 39BF ret */
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
