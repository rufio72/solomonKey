/* Corpo dell'attività 07 (2CCD) dell'avvio: macchina a fasi sul contatore
 * c19d, lotto H4b2 (brief 46). Qui l'ingresso, le fasi brevi (2, 5, 6, 8,
 * 9, 10, 11) e la fase 12 con la sua seconda tabella `rst 20` (2DB8) sul
 * sotto-contatore c19e; le fasi 0, 1, 3 stanno in attivita_attract07a.c, la
 * 4 e la 7 in attivita_attract07b.c. Le fasi 13-25 sono del lotto H4b3:
 * nemici_manca ferma il giro sul loro gestore.
 *
 * Regole come attivita_01_10.c: ogni fase inizia con `pop hl` (puntatore
 * allo slot) e termina con `ret`, quindi il `pop hl; ret` diventa `return`;
 * i registri del Z80 (struct z80) servono solo a passare gli argomenti alle
 * routine tradotte. HL vale c19d dall'ingresso: le attese (2E8B/2E90)
 * decrementano c19d, quelle della fase 12 c19e (HL = c19e dopo il `rst 20`
 * di 2DB8).
 *
 * Punti di sincronia (NEMICI_PUNTO): [da, a] va dalla prima istruzione dopo
 * l'ultima scrittura in RAM fino all'istruzione che scrive inclusa. Rami
 * mai eseguiti nei frame 0-2410: nemici_manca(indirizzo). Il registro R
 * (2D1F, bit 4 di c045) passa dalla sentinella nemici_r_somma(). */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_attract07.h"

/* 2E8B-2E8F: attesa a cancello. Come sub_2e90 (decrementa (HL); se c467 non
 * ha raggiunto C ritorna, altrimenti azzera c467 e rincrementa (HL)) ma con
 * i punti di sincronia fra una scrittura e l'altra. `sito` è l'ingresso
 * della voce (2E89, 2E5D, 2E79, 2E71, 2E7D) che carica C. */
static void attesa(uint16_t hl, uint8_t c, uint16_t sito)
{
    NEMICI_PUNTO(sito, 0x2e90);
    M(hl)--;                                   /* 2E90 */
    NEMICI_PUNTO(0x2e91, 0x2e95);
    if (M(0xc467) >= c) {                      /* 2E91-2E95 cp c; ret c */
        NEMICI_PUNTO(0x2e96, 0x2e97);
        M(0xc467) = 0x00;                      /* 2E97 */
        NEMICI_PUNTO(0x2e98, 0x2e9a);
        M(hl)++;                               /* 2E9A */
        NEMICI_PUNTO(0x2e9b, 0x2e9b);
    }
    NEMICI_PUNTO(0x2e8e, 0x2e8f);              /* pop hl; ret */
}

/* Fase 2, 2DEE: azzera le 224 celle di c0bd-c19c. */
static void fase_2dee(void)
{
    for (int i = 0; i < 0xe0; i++) {           /* 2DF4-2DF6 */
        NEMICI_PUNTO(i == 0 ? 0x2dee : 0x2df4, 0x2df6);
        M(0xc0bd + i) = 0x00;                  /* 2DF4 */
    }
    NEMICI_PUNTO(0x2df8, 0x2df9);
}

/* Fase 5, 2D07: con c243 = 1D chiamerebbe 3812 (mai eseguito); altrimenti,
 * con il bit 4 di c045 spento, ritorna senza scrivere. Con il bit acceso
 * legge R (2D1F): sentinella. */
static void fase_2d07(void)
{
    NEMICI_PUNTO(0x2d07, 0x2d1e);
    if (M(0xc243) == 0x1d) {                   /* 2D08-2D0D */
        nemici_manca(0x2d0f);
        return;
    }
    if (M(0xc045) & 0x10) {                    /* 2D19-2D1E */
        nemici_r_somma();
        nemici_manca(0x2d1f);
    }
}

/* 322A come sub_322a (z80_3xxx.c) ma con i ritorni delle NMI dentro il
 * decodificatore (32AC, 337F-33B0, frame 1189-1195: la routine dura sette
 * frame) eseguiti prima di sub_33ba, che accende il bit 6 di c044 (consegna
 * della palette): chiamando sub_322a intera le NMI correrebbero a flag già
 * acceso e lo consumerebbero sei frame prima di MAME. */
static void fondale_322a(struct z80 *z)
{
    sub_33aa(z);
    scrivi_porta(0xe802, 0);
    z->a = (uint8_t)(M(0xc243) * 2);
    SET_HL(z, 0xf068);
    sub_0008(z);                               /* rst 08: HL = f068 + 4·c243 */
    sub_0010(z);                               /* rst 10: DE = flusso */
    uint16_t flusso = DE(z);
    sub_0018(z);                               /* rst 18: HL = parola 2 */
    uint16_t sovrapposizioni = HL(z);
    SET_HL(z, DE(z));
    SET_DE(z, flusso);
    SET_BC(z, 0x1000);
    sub_326c(z);
    SET_HL(z, sovrapposizioni);
    z->a = z->h;
    if (z->a != 0) {
        SET_HL(z, 0xf138);
        sub_325e(z);
        SET_HL(z, sovrapposizioni);
    }
    z->a = z->l;
    if (z->a != 0)
        sub_325e(z);
    PUNTO_LARGO(0x3225, 0x33b9);               /* ritorni in 322A e figlie (32xx-33xx) */
    z->c = 1;
    sub_33ba(z);
}

/* Fase 6, 3225: fondale della stanza c243 (322A). */
static void fase_3225(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x3225, 0x3225);
    fondale_322a(&z);                          /* 3225 */
    NEMICI_PUNTO(0x3228, 0x3229);
}

/* Fase 9, 2F2D: fondale di gioco (2F57), poi sull'oggetto c85e: coordinate
 * di c247 (4B0E) in c86c e c873, stato C0 (4AF6 con E = 41, BC = 0300); il
 * riquadro 3A54 alla cella di c247 (2457) con tile C4 e attributo 70. */
static void fase_2f2d(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x2f2d, 0x2f2d);
    sub_2f57(&z);                              /* 2F2D */
    PUNTO_LARGO(0x2f57, 0x2f5f);               /* ritorni dentro 2F57 */
    uint8_t d = M(0xc247);                     /* 2F30-2F33 */
    z.a = d;
    sub_4b0e(&z);                              /* 2F34 */
    z.ix = 0xc85e;
    NEMICI_PUNTO(0x2f37, 0x2f3b);
    M(0xc86c) = z.h;                           /* 2F3B */
    NEMICI_PUNTO(0x2f3c, 0x2f3e);
    M(0xc873) = z.l;                           /* 2F3E */
    SET_BC(&z, 0x0300);
    z.e = 0x41;
    z.a = 0xc0;
    NEMICI_PUNTO(0x2f3f, 0x2f48);
    sub_4af6(&z);                              /* 2F48 */
    z.e = d;                                   /* 2F4B */
    sub_2457(&z);                              /* 2F4C */
    SET_BC(&z, 0xc470);
    NEMICI_PUNTO(0x2f4b, 0x2f52);
    sub_3a54(&z);                              /* 2F52 */
    PUNTO_LARGO(0x3a54, 0x3a7c);               /* ritorni dentro 3A54 */
    NEMICI_PUNTO(0x2f55, 0x2f56);
}

/* Fase 11, 2DFA: c85e = 0; con c248 diverso da c249 (2E11 azzera anche
 * c19e) suona 38 e ritorna; con i due uguali incrementerebbe c19d (2E04,
 * mai eseguito). */
static void fase_2dfa(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x2dfa, 0x2dfb);
    M(0xc85e) = 0x00;                          /* 2DFB */
    NEMICI_PUNTO(0x2dfc, 0x2dff);
    sub_2e11(&z);                              /* 2DFF */
    NEMICI_PUNTO(0x2e02, 0x2e08);
    if (z.f & 0x40) {                          /* 2E02 jr nz,2E06 */
        nemici_manca(0x2e04);
        return;
    }
    z.c = 0x38;
    sub_0028(&z);                              /* 2E08 rst 28h */
    NEMICI_PUNTO(0x2e09, 0x2e09);
}

/* Fase 12, voce 0 di 2DB8 (2FC1): sull'oggetto c85e le coordinate di c248
 * e di c247 (4B0E), la differenza in passi (2FF3) in c86b, c86c, c87a,
 * c87b (+1 sull'alto), stato C0 con E = 40 e BC = 0301. */
static void t2_2fc1(void)
{
    struct z80 z = {0};
    z.ix = 0xc85e;
    NEMICI_PUNTO(0x2fc1, 0x2fd7);
    z.a = M(0xc247);
    sub_4b0e(&z);                              /* 2FC8 */
    uint16_t primo = HL(&z);
    z.a = M(0xc248);
    sub_4b0e(&z);                              /* 2FCF */
    SET_DE(&z, primo);                         /* 2FCB push hl, 2FD2 pop de */
    sub_2ff3(&z);                              /* 2FD3 */
    z.h++;                                     /* 2FD6 */
    M(0xc85e + 0x0b) = z.l;                    /* 2FD7 */
    NEMICI_PUNTO(0x2fd8, 0x2fda);
    M(0xc85e + 0x0c) = z.h;                    /* 2FDA */
    NEMICI_PUNTO(0x2fdb, 0x2fdd);
    M(0xc85e + 0x08) = 0xf8;                   /* 2FDD */
    NEMICI_PUNTO(0x2fde, 0x2fe1);
    M(0xc85e + 0x12) = z.e;                    /* 2FE1 */
    NEMICI_PUNTO(0x2fe2, 0x2fe4);
    M(0xc85e + 0x13) = z.d;                    /* 2FE4 */
    z.e = 0x40;
    SET_BC(&z, 0x0301);
    z.a = 0xc0;
    NEMICI_PUNTO(0x2fe5, 0x2fee);
    sub_4af6(&z);                              /* 2FEE */
    NEMICI_PUNTO(0x2ff1, 0x2ff2);
}

/* Fase 12, voce 2 di 2DB8 (2FA9): coordinate di c248 in c86c e c873, +1 a
 * c87c, suono 36. */
static void t2_2fa9(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x2fa9, 0x2fb3);
    z.a = M(0xc248);
    sub_4b0e(&z);                              /* 2FB0 */
    M(0xc86c) = z.h;                           /* 2FB3 */
    NEMICI_PUNTO(0x2fb4, 0x2fb6);
    M(0xc873) = z.l;                           /* 2FB6 */
    NEMICI_PUNTO(0x2fb7, 0x2fb9);
    M(0xc87c)++;                               /* 2FB9 */
    NEMICI_PUNTO(0x2fba, 0x2fbe);
    z.c = 0x36;
    sub_0028(&z);                              /* 2FBE rst 28h */
    NEMICI_PUNTO(0x2fbf, 0x2fc0);
}

/* Fase 12, voce 4 di 2DB8 (2F8F): riquadro 3A54 alla cella di c248 (2457),
 * tile 4·c24a + D0 e attributo 70; poi +1 a c87c. */
static void t2_2f8f(void)
{
    struct z80 z = {0};
    z.e = M(0xc248);                           /* 2F8F-2F92 */
    sub_2457(&z);                              /* 2F93 */
    z.b = (uint8_t)(4 * M(0xc24a) + 0xd0);     /* 2F96-2F9D */
    z.c = 0x70;
    NEMICI_PUNTO(0x2f8f, 0x2fa0);
    sub_3a54(&z);                              /* 2FA0 */
    PUNTO_LARGO(0x3a54, 0x3a7c);               /* ritorni dentro 3A54 */
    NEMICI_PUNTO(0x2fa3, 0x2fa6);
    M(0xc87c)++;                               /* 2FA6 */
    NEMICI_PUNTO(0x2fa7, 0x2fa8);
}

/* Fase 12, 2DAD: tiene la fase (c19d - 1) e avanza il sotto-contatore c19e;
 * `rst 20` sulla tabella 2DB8 (6 voci: 2FC1, attesa 42, 2FA9, attesa 10,
 * 2F8F, attesa 0C); con c19e >= 6 chiude: c19e = c85e = 0 e la fase
 * avanza. */
static void fase_2dad(void)
{
    NEMICI_PUNTO(0x2dad, 0x2dad);
    M(0xc19d)--;                               /* 2DAD */
    NEMICI_PUNTO(0x2dae, 0x2db3);
    uint8_t sotto = M(0xc19e);                 /* 2DB2 */
    M(0xc19e) = (uint8_t)(sotto + 1);          /* 2DB3 */
    if (sotto >= 6) {                          /* 2DB4-2DB6 */
        NEMICI_PUNTO(0x2db4, 0x2dc6);
        M(0xc19e) = 0x00;                      /* 2DC6 */
        NEMICI_PUNTO(0x2dc7, 0x2dc7);
        M(0xc85e) = 0x00;                      /* 2DC7 */
        NEMICI_PUNTO(0x2dca, 0x2dcb);
        M(0xc19d)++;                           /* 2DCB */
        NEMICI_PUNTO(0x2dcc, 0x2dcd);
        return;
    }
    NEMICI_PUNTO(0x2db4, 0x2db8);
    NEMICI_PUNTO(0x0008, 0x0026);              /* gestore del rst 20h */
    switch (sotto) {                           /* 2DB8 rst 20h */
    case 0: t2_2fc1(); break;
    case 1: attesa(0xc19e, 0x42, 0x2e79); break;
    case 2: t2_2fa9(); break;
    case 3: attesa(0xc19e, 0x10, 0x2e71); break;
    case 4: t2_2f8f(); break;
    case 5: attesa(0xc19e, 0x0c, 0x2e7d); break;
    }
}

/* 07, ingresso 2CCD: HL = c19d, A = fase, c19d avanza di uno, salto con
 * `rst 20h` alla tabella 2CD2 (26 voci). Fasi 13-25: H4b3. */
void attivita_07_2ccd(uint8_t slot)
{
    (void)slot;
    NEMICI_PUNTO(0x2ccd, 0x2cd1);
    uint8_t fase = M(0xc19d);                  /* 2CCD-2CD0 */
    M(0xc19d) = (uint8_t)(fase + 1);           /* 2CD1 */
    NEMICI_PUNTO(0x2cd2, 0x2cd2);
    NEMICI_PUNTO(0x0008, 0x0026);              /* gestore del rst 20h */
    switch (fase) {                            /* 2CD2 rst 20h */
    case 0:  fase_07_3861(); break;
    case 1:  fase_07_34fa(); break;
    case 2:  fase_2dee(); break;
    case 3:  fase_07_37d7(); break;
    case 4:  fase_07_358f(); break;
    case 5:  fase_2d07(); break;
    case 6:  fase_3225(); break;
    case 7:  fase_07_3a7d(); break;
    case 8:  attesa(0xc19d, 0x60, 0x2e89); break;
    case 9:  fase_2f2d(); break;
    case 10: attesa(0xc19d, 0x0d, 0x2e5d); break;
    case 11: fase_2dfa(); break;
    case 12: fase_2dad(); break;
    case 13: nemici_manca(0x2dce); break;
    case 14: nemici_manca(0x31c5); break;
    case 15: nemici_manca(0x2e0a); break;
    case 16: nemici_manca(0x2d9d); break;
    case 17: nemici_manca(0x2e9c); break;
    case 18: nemici_manca(0x3709); break;
    case 19: nemici_manca(0x3829); break;
    case 20: nemici_manca(0x321e); break;
    case 21: nemici_manca(0x2e9c); break;
    case 22: nemici_manca(0x2ef5); break;
    case 23: nemici_manca(0x2e6d); break;
    case 24: nemici_manca(0x2f1a); break;
    case 25: nemici_manca(0x2e1e); break;
    default: nemici_fuori_tabella(0x2cd2, fase); break;
    }
}
