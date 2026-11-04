/* Corpo dell'attività 25 (3B77) del gioco libero, lotto 54 (brief 54):
 * macchina a passi sul contatore c19d (tabella `rst 20h` 3B7C, 10 voci).
 * Tradotto dal disassemblato out/main64.dasm; regole come il 07
 * (attivita_attract07.c): HL = c19d, A = passo, c19d avanza di uno; i
 * passi finiscono con `pop hl; ret` salvo 3DB3 (`pop de`) e 3B91 (`pop
 * bc`, che scrive nello slot). Il passo 7 (3BAC) comincia con 75AA, che
 * dura più di un frame: non si cattura in MAME e si verifica per
 * composizione (le routine chiamate sono nel banco). */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"

/* Passo 0, 3DC6: tabella delle attività vuota (673F), posta 14 e 25; sui
 * 18 oggetti da c4ce (passo 30) bit 6 spento e bit 4 acceso; oggetto c88e
 * (3DEA), suono 23. */
static void passo_3dc6(void)
{
    struct z80 z = {0};
    sub_673f(&z);                              /* 3DC6 */
    z.a = 0x14;                                /* 3DC9 */
    sub_0493(&z);                              /* 3DCB */
    z.a = 0x25;                                /* 3DCE */
    sub_0493(&z);                              /* 3DD0 */
    uint16_t hl = 0xc4ce;                      /* 3DD3 */
    for (int b = 0x12; b > 0; b--) {           /* 3DD6-3DE0 */
        M(hl) = (uint8_t)((M(hl) & ~0x40) | 0x10);
        hl = (uint16_t)(hl + 0x30);
    }
    sub_3dea(&z);                              /* 3DE2 */
    z.c = 0x23;                                /* 3DE5 */
    sub_0028(&z);                              /* 3DE7 rst 28h */
}

/* Passo 1, 3DB3: resta sul passo finché c463 < 0E; poi bit 6 di c82e
 * spento e c467 = 0. */
static void passo_3db3(void)
{
    M(0xc19d)--;                               /* 3DB4 dec (hl) */
    if (M(0xc463) < 0x0e)                      /* 3DB5-3DBA ret c */
        return;
    M(0xc19d)++;                               /* 3DBB */
    M(0xc82e) &= (uint8_t)~0x40;               /* 3DBC-3DBF */
    M(0xc467) = 0x00;                          /* 3DC1-3DC2 */
}

/* Passo 3, 3D6A: i 17 oggetti da c4fe (passo 30) con il bit 7 acceso
 * verso la cella c247: passi (2FF3) dalle loro coordinate a quelle della
 * cella (4B0E), (IX+08) = (IX+0F) = 0, 3DA6, stato C5 (4AF6, BC = 0301,
 * E = 40). */
static void passo_3d6a(void)
{
    struct z80 z = {0};
    z.a = M(0xc247);                           /* 3D6A */
    sub_4b0e(&z);                              /* 3D6D */
    z.b = 0x11;                                /* 3D70 */
    z.ix = 0xc4fe;                             /* 3D72 */
    do {
        if (M(z.ix) & 0x80) {                  /* 3D76-3D7A */
            uint16_t hl = HL(&z), bc = BC(&z); /* 3D7C-3D7D push */
            z.d = M(z.ix + 0x0e);              /* 3D7E */
            z.e = M(z.ix + 0x15);              /* 3D81 */
            sub_2ff3(&z);                      /* 3D84 */
            z.a = 0x00;                        /* 3D87 */
            M(z.ix + 0x08) = z.a;              /* 3D88 */
            M(z.ix + 0x0f) = z.a;              /* 3D8B */
            sub_3da6(&z);                      /* 3D8E */
            SET_BC(&z, 0x0301);                /* 3D91 */
            z.e = 0x40;                        /* 3D94 */
            z.a = 0xc5;                        /* 3D96 */
            sub_4af6(&z);                      /* 3D98 */
            SET_BC(&z, bc);                    /* 3D9B-3D9C pop */
            SET_HL(&z, hl);
        }
        SET_DE(&z, 0x0030);                    /* 3D9D */
        z.ix = (uint16_t)(z.ix + 0x30);        /* 3DA0 */
    } while (--z.b != 0);                      /* 3DA2 djnz */
}

/* Passo 5, 3D4C: i 17 oggetti da c4fe (passo 30) spenti, poi 3D59. */
static void passo_3d4c(void)
{
    uint16_t hl = 0xc4fe;                      /* 3D4C */
    for (int b = 0x11; b > 0; b--) {           /* 3D4F-3D57 */
        M(hl) = 0x00;
        hl = (uint16_t)(hl + 0x30);
    }
    passo_3d59();                              /* cade in 3D59 */
}

/* Passo 7, 3BAC: schermata di fine quadro. Azzeramenti (75AA), c19e = 0,
 * palette 80B3 (3F88), 33BA (C = 1), 4398 (A = 3); cornice e riquadro in
 * d4a1-d7ff con 3CC1/3CC2 (le 5 righe di 26 caratteri 01-82, i bordi C6,
 * CA, CB, C7); le 4 scritte della tabella 3CCA e 5 volte la scritta 3D06
 * (6771), riquadro 1508 (3A54, BC = c035), punteggio del tempo (3FB2),
 * copia di 6 byte d46d -> d673, scritta 3D0A (3D15 con il bit 6 di c24e)
 * e i suoi 3 byte sommati a c1aa-c1ac; poi i bonus: c24f · 100000 (3D20),
 * c237 · 100000 (c237 = 0; con c237 = 0 il ciclo di 3D20 fa 256 giri, come
 * il Z80), c250 · 10000 (3D2F), ognuno scomposto (4D37) e scritto (3D43) a
 * d715, d6d5, d755; infine 4C89 e 4D05. */
static void passo_3bac(void)
{
    struct z80 z = {0};
    sub_75aa(&z);                              /* 3BAC */
    z.a = 0x00;                                /* 3BAF */
    M(0xc19e) = z.a;                           /* 3BB0 */
    SET_HL(&z, 0x80b3);                        /* 3BB3 */
    sub_3f88(&z);                              /* 3BB6 */
    z.c = 0x01;                                /* 3BB9 */
    sub_33ba(&z);                              /* 3BBB */
    z.a = 0x03;                                /* 3BBE */
    sub_4398(&z);                              /* 3BC0 */
    SET_HL(&z, 0xd4bd);                        /* 3BC3 */
    z.a = 0xc7;                                /* 3BC6 */
    sub_3cc1(&z);                              /* 3BC8 */
    z.a = 0xc9;                                /* 3BCB */
    sub_3cc1(&z);                              /* 3BCD */
    z.a--;                                     /* 3BD0 */
    SET_HL(&z, 0xd4a1);                        /* 3BD1 */
    sub_3cc1(&z);                              /* 3BD4 */
    z.a--;                                     /* 3BD7 */
    sub_3cc1(&z);                              /* 3BD8 */
    SET_DE(&z, 0x0026);                        /* 3BDB */
    z.c = 0x05;                                /* 3BDE */
    z.a = 0x00;                                /* 3BE0 */
    do {
        z.b = 0x1a;                            /* 3BE1 */
        do {
            z.a++;                             /* 3BE3 */
            sub_3cc1(&z);                      /* 3BE4 */
        } while (--z.b != 0);                  /* 3BE7 djnz */
        SET_HL(&z, HL(&z) + DE(&z));           /* 3BE9 */
    } while (--z.c != 0);                      /* 3BEA-3BEB */
    SET_HL(&z, 0xd4c1);                        /* 3BED */
    SET_BC(&z, 0x09c6);                        /* 3BF0 */
    z.e = 0x1c;                                /* 3BF3 */
    do {
        M(HL(&z)) = z.c;                       /* 3BF5 */
        sub_3cc2(&z);                          /* 3BF6 */
        SET_HL(&z, HL(&z) + DE(&z));           /* 3BF9 */
        M(HL(&z)) = z.c;                       /* 3BFA */
        sub_3cc2(&z);                          /* 3BFB */
        SET_HL(&z, HL(&z) + 2);                /* 3BFE-3BFF */
    } while (--z.b != 0);                      /* 3C00 djnz */
    M(HL(&z)) = 0xca;                          /* 3C02 */
    sub_3cc2(&z);                              /* 3C04 */
    z.a = 0xc7;                                /* 3C07 */
    z.b = 0x1c;                                /* 3C09 */
    do {
        sub_3cc1(&z);                          /* 3C0B */
    } while (--z.b != 0);                      /* 3C0E djnz */
    M(HL(&z)) = 0xcb;                          /* 3C10 */
    sub_3cc2(&z);                              /* 3C12 */
    SET_HL(&z, 0x3cca);                        /* 3C15 */
    z.b = 0x04;                                /* 3C18 */
    do {
        uint8_t b = z.b, c = z.c;              /* 3C1A push bc */
        sub_0010(&z);                          /* 3C1B rst 10h */
        sub_6771(&z);                          /* 3C1C */
        z.b = b;                               /* 3C1F pop bc */
        z.c = c;
    } while (--z.b != 0);                      /* 3C20 djnz */
    z.b = 0x05;                                /* 3C22 */
    do {
        sub_0010(&z);                          /* 3C24 rst 10h */
        uint16_t hl = HL(&z);                  /* 3C25 push hl */
        SET_HL(&z, 0x3d06);                    /* 3C26 */
        sub_6771(&z);                          /* 3C29 */
        SET_HL(&z, hl);                        /* 3C2C pop hl */
    } while (--z.b != 0);                      /* 3C2D djnz */
    SET_DE(&z, 0x1508);                        /* 3C2F */
    SET_BC(&z, 0xc035);                        /* 3C32 */
    sub_3a54(&z);                              /* 3C35 */
    sub_3fb2(&z);                              /* 3C38 */
    for (int i = 0; i < 6; i++)                /* 3C3B-3C44 ldir */
        M(0xd673 + i) = M(0xd46d + i);
    SET_BC(&z, 0x0000);
    z.a = (uint8_t)(M(0xc24e) & 0x40);         /* 3C49-3C4C */
    SET_HL(&z, z.a ? 0x3d15 : 0x3d0a);         /* 3C4E-3C53 */
    SET_DE(&z, 0xd633);                        /* 3C56 */
    sub_6771(&z);                              /* 3C59 */
    z.b = 0x03;                                /* 3C5C */
    SET_DE(&z, 0xc1aa);                        /* 3C5E */
    unsigned cy = 0;                           /* 3C61 or a */
    do {
        unsigned s = M(DE(&z)) + leggi(HL(&z)) + cy;   /* 3C62-3C63 */
        z.a = (uint8_t)s;
        cy = s > 0xff;
        M(DE(&z)) = z.a;                       /* 3C64 */
        SET_HL(&z, HL(&z) + 1);                /* 3C65 */
        SET_DE(&z, DE(&z) + 1);                /* 3C66 */
    } while (--z.b != 0);                      /* 3C67 djnz */
    SET_HL(&z, 0x0000);                        /* 3C69-3C6D */
    SET_DE(&z, 0x0000);
    z.a = M(0xc24f);                           /* 3C6E */
    if (z.a != 0) {                            /* 3C71-3C72 */
        z.b = z.a;                             /* 3C74 */
        sub_3d20(&z);                          /* 3C75 */
    }
    sub_4d37(&z);                              /* 3C78 */
    SET_HL(&z, 0xd715);                        /* 3C7B */
    sub_3d43(&z);                              /* 3C7E */
    z.b = M(0xc237);                           /* 3C81-3C84 */
    z.a = 0x00;                                /* 3C85 */
    M(0xc237) = z.a;                           /* 3C86 */
    SET_HL(&z, 0x0000);                        /* 3C87-3C8B */
    SET_DE(&z, 0x0000);
    sub_3d20(&z);                              /* 3C8C */
    sub_4d37(&z);                              /* 3C8F */
    SET_HL(&z, 0xd6d5);                        /* 3C92 */
    sub_3d43(&z);                              /* 3C95 */
    SET_HL(&z, 0x0000);                        /* 3C98-3C9C */
    SET_DE(&z, 0x0000);
    z.a = M(0xc250);                           /* 3C9D */
    if (z.a != 0) {                            /* 3CA0-3CA1 */
        SET_BC(&z, 0x2710);                    /* 3CA3 */
        do {
            unsigned s = HL(&z) + BC(&z);      /* 3CA6 */
            SET_HL(&z, (uint16_t)s);
            if (s > 0xffff)                    /* 3CA7 */
                SET_DE(&z, DE(&z) + 1);        /* 3CA9 */
        } while (--z.a != 0);                  /* 3CAA-3CAB */
    }
    sub_3d2f(&z);                              /* 3CAD */
    sub_4d37(&z);                              /* 3CB0 */
    SET_HL(&z, 0xd755);                        /* 3CB3 */
    sub_3d43(&z);                              /* 3CB6 */
    sub_4c89(&z);                              /* 3CB9 */
    sub_4d05(&z);                              /* 3CBC */
}

/* Passo 9, 3B91: torna al passo 8 (c19d − 2) finché c19e, che sale di
 * uno a ogni giro, era sotto 28; poi c19e = c19d = 0, lo slot diventa
 * l'attività 16, azzeramenti (75AA) e bit 3 di c09b spento. */
static void passo_3b91(uint8_t slot)
{
    struct z80 z = {0};
    M(0xc19d) = (uint8_t)(M(0xc19d) - 2);      /* 3B92-3B93 */
    uint8_t a = M(0xc19e);                     /* 3B95-3B98 */
    M(0xc19e) = (uint8_t)(a + 1);              /* 3B99 */
    if (a < 0x28)                              /* 3B9A-3B9C ret c */
        return;
    M(0xc19e) = 0x00;                          /* 3B9D-3B9E */
    M(0xc19d) = 0x00;                          /* 3B9F */
    tabella[slot] = 0x16;                      /* 3BA0-3BA2 ld (bc),a */
    sub_75aa(&z);                              /* 3BA3 */
    M(0xc09b) &= (uint8_t)~0x08;               /* 3BA6-3BA9 */
}

/* 25, ingresso 3B77: passo da c19d (`rst 20h` 3B7C). */
void attivita_25_3b77(uint8_t slot)
{
    uint8_t passo = M(0xc19d);                 /* 3B77-3B7A */
    M(0xc19d) = (uint8_t)(passo + 1);          /* 3B7B */
    switch (passo) {                           /* 3B7C rst 20h */
    case 0: passo_3dc6(); break;
    case 1: passo_3db3(); break;
    case 2: case 4: case 6: case 8: passo_2e79(0xc19d); break;
    case 3: passo_3d6a(); break;
    case 5: passo_3d4c(); break;
    case 7: passo_3bac(); break;
    case 9: passo_3b91(slot); break;
    default: nemici_fuori_tabella(0x3b7c, passo); break;
    }
}
