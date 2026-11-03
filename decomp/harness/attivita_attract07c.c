/* Corpo 07 (2CCD), fasi 13 (2DCE con la tabella `rst 20` 2DDA), 14 (31C5),
 * 15 (2E0A) e 17/21 (2E9C): lotto H4b3 (brief 47). Stesse regole di
 * attivita_attract07a.c: ogni fase inizia con `pop hl`/`pop de` (puntatore
 * allo slot, inutile qui) e termina con `ret`, quindi il rientro è il
 * `return`; l'intervallo [da, a] di NEMICI_PUNTO va dalla prima istruzione
 * dopo l'ultima scrittura in RAM fino all'istruzione che scrive inclusa;
 * nei cicli il punto sta dentro il ciclo. PUNTO_LARGO per i ritorni dentro
 * routine tradotte senza punti interni. */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_attract07.h"

/* Voce 0 di 2DDA (3009): sull'oggetto c82e le coordinate della cella c249
 * (4B0E), la differenza in passi rispetto a 8078 (2FF3) in c2cb e c2cd, i
 * sette byte ROM 303E in c2c1, i 16 oggetti di riposo (3045), suono 34 e
 * c463 = 0. */
static void t3_3009(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x3009, 0x3014);
    z.a = M(0xc249);
    sub_4b0e(&z);                              /* 300D */
    z.ix = 0xc82e;
    M(z.ix + 0x0e) = z.h;                      /* 3014 */
    NEMICI_PUNTO(0x3015, 0x3017);
    M(z.ix + 0x15) = z.l;                      /* 3017 */
    NEMICI_PUNTO(0x3018, 0x3020);
    SET_DE(&z, 0x8078);
    sub_2ff3(&z);                              /* 301D */
    M(0xc2cb) = z.l;                           /* 3020 ld (c2cb),hl */
    M(0xc2cc) = z.h;
    NEMICI_PUNTO(0x3023, 0x3023);
    M(0xc2cd) = z.e;                           /* 3023 ld (c2cd),de */
    M(0xc2ce) = z.d;
    for (int i = 0; i < 7; i++) {              /* 3030 ldir */
        if (i == 0)
            NEMICI_PUNTO(0x3027, 0x3030);
        else
            NEMICI_PUNTO(0x3030, 0x3030);
        M(0xc2c1 + i) = leggi((uint16_t)(0x303e + i));
    }
    NEMICI_PUNTO(0x3032, 0x3032);
    sub_3045(&z);                              /* 3032 */
    PUNTO_LARGO(0x3045, 0x305e);               /* ritorni dentro 3045 */
    NEMICI_PUNTO(0x3035, 0x3037);
    z.c = 0x34;
    sub_0028(&z);                              /* 3037 rst 28h */
    NEMICI_PUNTO(0x3038, 0x3039);
    M(0xc463) = 0x00;                          /* 3039 */
    NEMICI_PUNTO(0x303c, 0x303d);
}

/* Voce 1 di 2DDA (305F): c19e - 1 (resta nella voce); c467 = 0; con c463 <
 * 40 sposta i 16 oggetti sul cerchio (3097), avanza la fase c2c5 (237e) e
 * somma due parole c2cb-c2cc a c2c1-c2c2, c2cd-c2ce a c2c3-c2c4 (23A1); con
 * c463 >= 40 azzera il primo byte di 16 oggetti (c46e, passo 30) e porta
 * c19e a 2. Il `ret z` di 3065 usa il risultato del `dec (hl)`: c19e - 1,
 * mai zero (la voce è la 1). */
static void t3_305f(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x305f, 0x305f);
    uint8_t v = (uint8_t)(M(0xc19e) - 1);
    M(0xc19e) = v;                             /* 305F */
    NEMICI_PUNTO(0x3060, 0x3066);
    if (v == 0) {                              /* 3065 ret z */
        nemici_manca(0x3065);
        return;
    }
    M(0xc467) = 0x00;                          /* 3066 */
    NEMICI_PUNTO(0x3067, 0x306f);
    if (M(0xc463) < 0x40) {                    /* 3068-306D */
        sub_3097(&z);                          /* 306F: punti propri (z80_3xxx.c) */
        NEMICI_PUNTO(0x3072, 0x3075);
        SET_HL(&z, 0xc2c5);
        sub_237e(&z);                          /* 3075 */
        SET_HL(&z, 0xc2c1);
        SET_DE(&z, 0xc2cb);
        for (int i = 0; i < 2; i++) {          /* 3080-3083 */
            if (i == 0)
                NEMICI_PUNTO(0x3078, 0x3080);
            else
                NEMICI_PUNTO(0x3080, 0x3083);
            sub_23a1(&z);                      /* 3080 */
        }
        NEMICI_PUNTO(0x3085, 0x3085);
        return;
    }
    for (int i = 0; i < 0x10; i++) {           /* 308E-3090 */
        if (i == 0)
            NEMICI_PUNTO(0x3086, 0x308e);
        else
            NEMICI_PUNTO(0x308e, 0x3090);
        M(0xc46e + 0x30 * i) = 0x00;           /* 308E */
    }
    NEMICI_PUNTO(0x3092, 0x3095);
    M(0xc19e)++;                               /* 3095 */
    NEMICI_PUNTO(0x3096, 0x3096);
}

/* Voce 2 di 2DDA (3131): tre agganci di celle a video (2F74, da HL = 7868,
 * IX = c52e, BC = 0400 con C da 0 a 2) poi l'oggetto c82e: stato 80, y e x
 * (IX+0E, IX+15) in c93e-c940 con 0099, suono 35. */
static void t3_3131(void)
{
    struct z80 z = {0};
    z.ix = 0xc52e;
    SET_BC(&z, 0x0400);
    SET_HL(&z, 0x7868);
    for (int i = 0; i < 3; i++) {              /* 313B-3142 */
        if (i == 0)
            NEMICI_PUNTO(0x3131, 0x313b);
        else
            NEMICI_PUNTO(0x313e, 0x313b);
        sub_2f74(&z);                          /* 313B */
        z.c++;                                 /* 313E */
    }
    NEMICI_PUNTO(0x313e, 0x3148);
    z.ix = 0xc82e;
    M(z.ix) = 0x80;                            /* 3148 */
    NEMICI_PUNTO(0x314c, 0x3155);
    z.l = M(z.ix + 0x0e);
    z.h = M(z.ix + 0x15);
    M(0xc93e) = 0x99;                          /* 3155 ld (c93e),de */
    M(0xc93f) = 0x00;
    NEMICI_PUNTO(0x3159, 0x3159);
    M(0xc940) = z.l;                           /* 3159 ld (c940),hl */
    M(0xc941) = z.h;
    NEMICI_PUNTO(0x315b, 0x315e);
    z.c = 0x35;
    sub_0028(&z);                              /* 315E rst 28h */
    NEMICI_PUNTO(0x315f, 0x3160);
}

/* Voce 4 di 2DDA (3161): quattro oggetti da c46e (passo 30) agli angoli del
 * quadrato attorno all'oggetto c82e (coordinate c83c e c843 meno 8, più 10
 * sui lati alti), stato C5 (4AF6 con B = 3, C da 3 a 0, E = 0A); poi
 * l'oggetto c82e: stato C0 (4AF6 con E = 0, B = 5, C = 2) e (IX+04) = 1. */
static void t3_3161(void)
{
    struct z80 z = {0};
    uint8_t h = (uint8_t)(M(0xc83c) - 0x08);   /* 3161-3166 */
    uint8_t l = (uint8_t)(M(0xc843) - 0x08);   /* 3167-316C */
    SET_BC(&z, 0x0303);
    SET_DE(&z, 0x000a);
    z.ix = 0xc46e;
    for (;;) {                                 /* 3177-319A */
        uint8_t a = h;
        if (z.c & 2)
            a = (uint8_t)(a + 0x10);
        if (z.c == 3)
            NEMICI_PUNTO(0x3161, 0x317e);
        else
            NEMICI_PUNTO(0x3177, 0x317e);
        M(z.ix + 0x0e) = a;                    /* 317E */
        a = l;
        if (z.c & 1)
            a = (uint8_t)(a + 0x10);
        NEMICI_PUNTO(0x317f, 0x3188);
        M(z.ix + 0x15) = a;                    /* 3188 */
        z.a = 0xc5;
        NEMICI_PUNTO(0x3189, 0x318d);
        sub_4af6(&z);                          /* 318D */
        z.ix = (uint16_t)(z.ix + 0x30);        /* 3190-3195 */
        z.c--;                                 /* 3196 */
        if (z.c == 0xff)                       /* 3197-319A: A = 2 + C */
            break;
    }
    NEMICI_PUNTO(0x319c, 0x31ae);
    z.ix = 0xc82e;
    z.e = 0x00;
    SET_BC(&z, 0x0502);
    if (M(z.ix + 0x15) & 0x80)                 /* 31A5-31AB */
        z.c++;
    z.a = 0xc0;
    sub_4af6(&z);                              /* 31AE */
    NEMICI_PUNTO(0x31b1, 0x31b1);
    M(z.ix + 0x04) = 0x01;                     /* 31B1 */
    NEMICI_PUNTO(0x31b5, 0x31b6);
}

/* Voce 6 di 2DDA (31B7): azzera il primo byte dei quattro oggetti c46e. */
static void t3_31b7(void)
{
    for (int i = 0; i < 4; i++) {              /* 31BF-31C1 */
        if (i == 0)
            NEMICI_PUNTO(0x31b7, 0x31bf);
        else
            NEMICI_PUNTO(0x31bf, 0x31c1);
        M(0xc46e + 0x30 * i) = 0x00;           /* 31BF */
    }
    NEMICI_PUNTO(0x31c3, 0x31c4);
}

/* Fase 13, 2DCE: come 2DAD ma con sette voci in 2DDA (3009, 305F, 3131,
 * attesa 07, 3161, attesa 0A, 31B7); con c19e >= 7 chiude: c19e = 0 e la
 * fase avanza. */
void fase_07_2dce(void)
{
    NEMICI_PUNTO(0x2dce, 0x2dce);
    M(0xc19d)--;                               /* 2DCE */
    NEMICI_PUNTO(0x2dcf, 0x2dd4);
    uint8_t sotto = M(0xc19e);                 /* 2DD3 */
    M(0xc19e) = (uint8_t)(sotto + 1);          /* 2DD4 */
    if (sotto >= 7) {                          /* 2DD5-2DD7 */
        NEMICI_PUNTO(0x2dd5, 0x2de8);
        M(0xc19e) = 0x00;                      /* 2DE8 */
        NEMICI_PUNTO(0x2dea, 0x2deb);
        M(0xc19d)++;                           /* 2DEB */
        NEMICI_PUNTO(0x2dec, 0x2ded);
        return;
    }
    NEMICI_PUNTO(0x2dd5, 0x2dd9);
    NEMICI_PUNTO(0x0008, 0x0026);              /* gestore del rst 20h */
    switch (sotto) {                           /* 2DD9 rst 20h */
    case 0: t3_3009(); break;
    case 1: t3_305f(); break;
    case 2: t3_3131(); break;
    case 3: attesa_07(0xc19e, 0x07, 0x2e65); break;
    case 4: t3_3161(); break;
    case 5: attesa_07(0xc19e, 0x0a, 0x2e81); break;
    case 6: t3_31b7(); break;
    }
}

/* Fase 14, 31C5: la barra della riga (31E9) con A = c237 - 1. */
void fase_07_31c5(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x31c5, 0x31c9);
    z.a = (uint8_t)(M(0xc237) - 1);
    sub_31e9(&z);                              /* 31C9 */
    NEMICI_PUNTO(0x31cc, 0x31cd);
}

/* Fase 15, 2E0A: con c248 diverso da c249 (2E11 azzera anche c19e) avanza
 * c19d; con i due uguali ritorna. */
void fase_07_2e0a(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x2e0a, 0x2e0b);
    sub_2e11(&z);                              /* 2E0B */
    NEMICI_PUNTO(0x2e0e, 0x2e0f);
    if (z.f & 0x40)                            /* 2E0E ret z */
        return;
    M(0xc19d)++;                               /* 2E0F */
    NEMICI_PUNTO(0x2e10, 0x2e10);
}

/* Fasi 17 e 21, 2E9C: sceglie i dati del quadro c243. La parola 12·t[c243]
 * + 8A01 (t da 89CD, via 2368) in c2ac-c2ad; il byte di 8B3D + 9·c243 in c2ae; due ldir da 2 byte dai record 8D11 scelti
 * dalla parola 2·(e605 & 3) di quel byte, in c2b1-c2b2 e c2b3-c2b4; FF in
 * c2b7; spegne il bit 0 di c2ba. */
void fase_07_2e9c(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x2e9c, 0x2ea5);
    z.a = M(0xc243);
    SET_HL(&z, 0x89cd);
    sub_2368(&z);                              /* 2EA5 */
    uint16_t base = (uint16_t)(HL(&z) + 0x8a01);   /* 2EA8-2EAC */
    NEMICI_PUNTO(0x2ea8, 0x2eb0);
    M(0xc2ac) = (uint8_t)base;                 /* 2EB0 */
    NEMICI_PUNTO(0x2eb1, 0x2eb2);
    M(0xc2ad) = (uint8_t)(base >> 8);          /* 2EB2 */
    NEMICI_PUNTO(0x2eb3, 0x2ec3);
    uint16_t hl = (uint16_t)(9 * M(0xc243) + 0x8b3d);   /* 2EB3-2EC0: rst 8 lascia DE = c243 */
    M(0xc2ae) = leggi(hl);                     /* 2EC3 */
    hl++;
    z.a = (uint8_t)(leggi_porta(0xe605) & 0x03);   /* 2EC6-2EC9 */
    SET_HL(&z, hl);
    sub_0008(&z);                              /* 2ECB */
    sub_0010(&z);                              /* 2ECC */
    uint16_t de = DE(&z);                      /* E in A, D dopo il pop */
    uint16_t src = (uint16_t)(0x8d11 + 2 * (de & 0xff));   /* 2ECD-2ED2 */
    for (int i = 0; i < 2; i++) {              /* 2ED9 ldir */
        if (i == 0)
            NEMICI_PUNTO(0x2ec4, 0x2ed9);
        else
            NEMICI_PUNTO(0x2ed9, 0x2ed9);
        M(0xc2b1 + i) = leggi((uint16_t)(src + i));
    }
    src = (uint16_t)(0x8d11 + 2 * (de >> 8));  /* 2EDB-2EE0 */
    for (int i = 0; i < 2; i++) {              /* 2EE7 ldir */
        if (i == 0)
            NEMICI_PUNTO(0x2edb, 0x2ee7);
        else
            NEMICI_PUNTO(0x2ee7, 0x2ee7);
        M(0xc2b3 + i) = leggi((uint16_t)(src + i));
    }
    NEMICI_PUNTO(0x2ee9, 0x2eed);
    M(0xc2b5 + 2) = 0xff;                      /* 2EED */
    NEMICI_PUNTO(0x2eee, 0x2ef1);
    M(0xc2ba) &= 0xfe;                         /* 2EF1 res 0,(hl) */
    NEMICI_PUNTO(0x2ef3, 0x2ef4);
}
