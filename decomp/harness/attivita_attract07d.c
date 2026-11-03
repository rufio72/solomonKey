/* Corpo 07 (2CCD), fasi 18 (3709), 19 (3829), 20 (321E), 22 (2EF5), 24
 * (2F1A) e 25 (2E1E): lotto H4b3 (brief 47). Stesse regole di
 * attivita_attract07c.c: `pop hl` iniziale e `ret` finale sono il
 * `return`; la fase 25 azzera lo slot (`pop hl; ld (hl),0`), per cui
 * riceve `slot`. */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_attract07.h"

/* 0493 con A = codice. */
static void posta(uint8_t codice)
{
    struct z80 z = {0};
    z.a = codice;
    sub_0493(&z);
}

/* Fase 18, 3709: azzera il primo byte dei tre oggetti c52e (passo 30); dalla
 * tabella di puntatori 85F9 (indice c243, rst 08 e rst 10) legge quattro
 * byte che scrive come coppie (valore & F8, valore & 07) in c1a2-c1a9 dopo
 * tre `rlca`; sull'oggetto c82e stato A0 e (IX+04) = 0; poi per ogni record
 * (B, cella) della lista fino a FF cerca uno slot libero (7379) e vi crea
 * l'oggetto (37B4). Con i bit 1-2 di c045 spenti e c243 diverso da 33 mette
 * due oggetti c4fe/c52e (4AF6 con E = 01) e lo stato 80 in c2c1 e c2d9;
 * infine c459 = 20. */
void fase_07_3709(void)
{
    struct z80 z = {0};
    for (int i = 0; i < 3; i++) {              /* 3711-3713 */
        if (i == 0)
            NEMICI_PUNTO(0x3709, 0x3711);
        else
            NEMICI_PUNTO(0x3711, 0x3713);
        M(0xc52e + 0x30 * i) = 0x00;           /* 3711 */
    }
    NEMICI_PUNTO(0x3715, 0x371c);
    NEMICI_PUNTO(0x0008, 0x0014);              /* rst 08h, rst 10h */
    z.a = M(0xc243);
    SET_HL(&z, 0x85f9);
    sub_0008(&z);                              /* 371B */
    sub_0010(&z);                              /* 371C */
    uint16_t de = DE(&z);
    for (int i = 0; i < 4; i++) {              /* 3722-3731 */
        uint8_t a = leggi(de++);               /* 3722-3723 */
        uint8_t c = (uint8_t)((a << 3) | (a >> 5));    /* 3724-3727 rlca ×3 */
        if (i == 0)
            NEMICI_PUNTO(0x371d, 0x372a);
        else
            NEMICI_PUNTO(0x372b, 0x372a);
        M(0xc1a2 + 2 * i) = (uint8_t)(c & 0xf8);   /* 372A */
        NEMICI_PUNTO(0x372b, 0x372f);
        M(0xc1a3 + 2 * i) = (uint8_t)(c & 0x07);   /* 372F */
    }
    NEMICI_PUNTO(0x3730, 0x3737);
    z.ix = 0xc82e;
    M(z.ix) = 0xa0;                            /* 3737 */
    NEMICI_PUNTO(0x3738, 0x373b);
    M(z.ix + 0x04) = 0x00;                     /* 373B */
    for (;;) {                                 /* 373F-374E */
        NEMICI_PUNTO(0x373c, 0x373f);
        sub_7379(&z);                          /* 373F */
        uint8_t c = z.a;                       /* 3742 */
        uint8_t b = leggi(de++);               /* 3743-3745 */
        NEMICI_PUNTO(0x3742, 0x374b);
        if (b == 0xff)                         /* 3746-3747 */
            break;
        z.a = leggi(de++);                     /* 3749-374A */
        z.b = b;
        z.c = c;
        SET_DE(&z, de);
        sub_37b4(&z);                          /* 374B */
        de = DE(&z);
        PUNTO_LARGO(0x37b4, 0x37d6);           /* ritorni dentro 37B4 */
    }
    NEMICI_PUNTO(0x3750, 0x3755);
    if (!(M(0xc045) & 0x06) && M(0xc243) != 0x33) {   /* 3750-375C */
        uint8_t l = 2, d = 0x70;
        z.c = 2;
        z.b = 2;
        z.e = 0x01;
        z.ix = 0xc4fe;
        do {                                   /* 376B-377D */
            NEMICI_PUNTO(0x3757, 0x376b);
            M(z.ix + 0x0e) = 0x78;             /* 376B */
            NEMICI_PUNTO(0x376f, 0x376f);
            M(z.ix + 0x15) = d;                /* 376F */
            z.a = 0xc5;
            z.d = d;
            NEMICI_PUNTO(0x3770, 0x3772);
            sub_4af6(&z);                      /* 3772 */
            z.c++;                             /* 3775 */
            z.ix = 0xc52e;                     /* 3776 */
            l--;                               /* 377A */
            d = 0x80;                          /* 377B */
        } while (l != 0);
        NEMICI_PUNTO(0x377f, 0x3783);
        M(0xc2c1) = 0x80;                      /* 3783 */
        NEMICI_PUNTO(0x3784, 0x3787);
        M(0xc2c1 + 0x18) = 0x80;               /* 3787 */
    }
    NEMICI_PUNTO(0x378b, 0x378d);
    M(0xc459) = 0x20;                          /* 378D */
    NEMICI_PUNTO(0x3790, 0x3791);
}

/* Fase 19, 3829: sulla griglia c0bd, 12 righe di 15 coppie da c0cd (ix
 * avanza di uno poi di due), ogni cella diversa da 37 e con (cella - 40)
 * >= 37 disegna un riquadro (3A2E sceglie tile e attributo, 3A54 lo scrive)
 * nella posizione di (riga, colonna) rovesciate. */
void fase_07_3829(void)
{
    struct z80 z = {0};
    uint16_t ix = 0xc0cd;
    for (uint8_t c = 0x18; c != 0; c = (uint8_t)(c - 2)) {   /* 382F-385D */
        for (uint8_t b = 0x1e; b != 0; b = (uint8_t)(b - 2)) {   /* 3831-3857 */
            ix++;                              /* 3831 */
            uint8_t a = M(ix);
            if (a == 0x37 || (uint8_t)(a - 0x40) < 0x37)   /* 3836-383E */
                continue;
            z.a = a;                           /* 3840: (a - 40) + 40 */
            sub_3a2e(&z);                      /* 3844 */
            uint8_t tb = z.b, tc = z.c;
            z.d = (uint8_t)(0x1c - c);         /* 3848-384C: pop hl = (b, c) */
            z.e = (uint8_t)(0x1f - b);
            z.b = tb;
            z.c = tc;
            NEMICI_PUNTO(0x3831, 0x3851);
            sub_3a54(&z);                      /* 3851 */
            PUNTO_LARGO(0x3a54, 0x3a7c);       /* ritorni dentro 3A54 */
        }
        ix++;                                  /* 3859 */
    }
    NEMICI_PUNTO(0x385f, 0x3860);
}

/* Fase 20, 321E: palette della stanza c243 (33BA con C = 0). */
void fase_07_321e(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x321e, 0x3220);
    PUNTO_LARGO(0x33ba, 0x33e9);               /* ritorni dentro 33BA: prima, 33BA accende il bit 6 di c044 */
    z.c = 0x00;
    sub_33ba(&z);                              /* 3220 */
    NEMICI_PUNTO(0x3223, 0x3224);
}

/* Fase 22, 2EF5: sui 17 oggetti da c4fe (passo 30) con il bit 7 acceso
 * avanza l'animazione del record a c519 + n·30 (07DE); poi 05B5 con C = 40. */
void fase_07_2ef5(void)
{
    struct z80 z = {0};
    uint16_t ix = 0xc4fe, hl = 0xc519;
    for (int n = 0; n < 0x11; n++) {           /* 2EFF-2F12 */
        if (M(ix) & 0x80) {                    /* 2EFF-2F03 */
            z.ix = ix;
            SET_HL(&z, hl);
            NEMICI_PUNTO(n == 0 ? 0x2ef5 : 0x2eff, 0x2f07);
            sub_07de(&z);                      /* 2F07 */
        }
        hl = (uint16_t)(hl + 0x30);
        ix = (uint16_t)(ix + 0x30);
    }
    NEMICI_PUNTO(0x2f0c, 0x2f16);
    z.c = 0x40;
    sub_05b5(&z);                              /* 2F16 */
    NEMICI_PUNTO(0x2f19, 0x2f19);
}

/* Fase 24, 2F1A: accende il bit 6 dei 17 oggetti da c4fe (passo 30) con il
 * bit 7 acceso. */
void fase_07_2f1a(void)
{
    NEMICI_PUNTO(0x2f1a, 0x2f26);
    for (int n = 0; n < 0x11; n++) {           /* 2F22-2F29 */
        uint16_t p = (uint16_t)(0xc4fe + 0x30 * n);
        if (M(p) & 0x80) {                     /* 2F22-2F24 */
            NEMICI_PUNTO(0x2f22, 0x2f26);
            M(p) |= 0x40;                      /* 2F26 */
            NEMICI_PUNTO(0x2f28, 0x2f29);
        }
    }
    NEMICI_PUNTO(0x2f2b, 0x2f2c);
}

/* Fase 25, 2E1E: azzera c2a9-c2ab, c82e = E0, posta le attività 0A, 0D e 20
 * (0493), accende il bit 7 di c08d, i bit 5 (se c044 ha il 7) e 0 di c044,
 * il bit 3 di c045, azzera c19d e c2b8 (A = 0) e libera lo slot: la corsa
 * del corpo 07 finisce. */
void fase_07_2e1e(uint8_t slot)
{
    for (int i = 0; i < 3; i++) {              /* 2E24-2E26 */
        if (i == 0)
            NEMICI_PUNTO(0x2e1e, 0x2e24);
        else
            NEMICI_PUNTO(0x2e24, 0x2e26);
        M(0xc2a9 + i) = 0x00;                  /* 2E24 */
    }
    NEMICI_PUNTO(0x2e28, 0x2e2a);
    M(0xc82e) = 0xe0;                          /* 2E2A */
    static const uint8_t codici[3] = { 0x0a, 0x0d, 0x20 };
    for (int i = 0; i < 3; i++) {              /* 2E30-2E38 */
        NEMICI_PUNTO(i == 0 ? 0x2e2d : 0x2e30, 0x2e35);
        posta(codici[i]);                      /* 2E35 */
    }
    NEMICI_PUNTO(0x2e30, 0x2e3d);
    M(0xc08d) |= 0x80;                         /* 2E3D */
    NEMICI_PUNTO(0x2e3f, 0x2e46);
    if (M(0xc044) & 0x80)                      /* 2E42-2E44 */
        M(0xc044) |= 0x20;                     /* 2E46 */
    NEMICI_PUNTO(0x2e47, 0x2e48);
    M(0xc044) |= 0x01;                         /* 2E48 */
    NEMICI_PUNTO(0x2e4a, 0x2e4d);
    M(0xc045) |= 0x08;                         /* 2E4D */
    NEMICI_PUNTO(0x2e4e, 0x2e4f);
    M(0xc19d) = 0x00;                          /* 2E4F */
    NEMICI_PUNTO(0x2e50, 0x2e52);
    M(0xc2b8) = 0x00;                          /* 2E52 */
    NEMICI_PUNTO(0x2e53, 0x2e56);
    tabella[slot] = 0x00;                      /* 2E55-2E56 */
    NEMICI_PUNTO(0x2e58, 0x2e58);
}
