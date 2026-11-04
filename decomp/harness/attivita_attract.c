/* Corpi delle attività 1F (43C8) e 06 (75E7) dell'avvio, tradotti dal
 * disassemblato out/main64.dasm (brief 45, lotto H4b1). Stesse regole di
 * attivita_01_10.c: `pop hl; ld (hl),n; ret` diventa `tabella[slot] = n;
 * return`, `pop hl; ret` diventa `return`; i registri del Z80 servono solo
 * a passare gli argomenti alle routine tradotte.
 *
 * Punti di sincronia (NEMICI_PUNTO, nemici.h): in 06 dove cadono i ritorni
 * delle NMI entrate a metà attività. L'intervallo [da, a] va dalla prima
 * istruzione dopo l'ultima scrittura in RAM fino all'istruzione che scrive
 * inclusa (o alla chiamata che scrive). Le scritture alle porte (e604) non
 * contano: l'impronta arriva a e5ff. I ritorni dentro le routine tradotte
 * chiamate (32xx-33xx, 75B4, 7F03 ecc.) non hanno punti e restano a metà
 * attività, come nel replay.
 *
 * Routine chiamate: tutte del banco (routine.h), compresa la posta 0493
 * (sub_0493: il banco non ha lo spedidore sched.c). */
#include <stdio.h>
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"

/* 0493 con A = codice. */
static void posta(uint8_t codice)
{
    struct z80 z = {0};
    z.a = codice;
    sub_0493(&z);
}

void (*attract_punto_largo)(uint16_t da, uint16_t a) = NULL;
#define PUNTO_LARGO(da, a) \
    do { if (attract_punto_largo) attract_punto_largo((da), (a)); } while (0)

/* 1F, ingresso 43C8: azzera le maschere dei canali c09b e c0b5, copia i
 * 24 byte ROM 43E5 in c09d e i 6 byte ROM 43FD in c0b7 (due ldir
 * consecutivi, il secondo parte da HL = 43FD), poi l'attività diventa 14. */
void attivita_1f_43c8(uint8_t slot)
{
    M(0xc09b) = 0x00;                          /* 43C8-43C9 */
    M(0xc0b5) = 0x00;                          /* 43CC */
    for (int i = 0; i < 0x18; i++)             /* 43CF-43D8 ldir */
        M(0xc09d + i) = leggi((uint16_t)(0x43e5 + i));
    for (int i = 0; i < 0x06; i++)             /* 43DA-43DF ldir */
        M(0xc0b7 + i) = leggi((uint16_t)(0x43fd + i));
    tabella[slot] = 0x14;                      /* 43E1-43E2 */
}

/* 06, 760B (indici 2, 3, 4, 7 della tabella 75EC): spegne il
 * ribaltamento, c045 = 0, avanza il contatore di fase c1ae, l'attività
 * diventa 07, azzera il pannello (7C84) e i quadri (75AA), prepara il
 * tabellone (33EB), c25d = 50, quadro c243 e parola c1b7 dal record di 3
 * byte di indice (vecchia fase) della tabella ROM 765B, c053 = 0. */
static void fase_760b(uint8_t slot)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x760b, 0x760f);              /* prima di c045 */
    scrivi_porta(0xe604, 0x00);                /* 760C */
    M(0xc045) = 0x00;                          /* 760F */
    uint8_t c = M(0xc1ae);                     /* 7612 */
    NEMICI_PUNTO(0x7612, 0x7613);
    M(0xc1ae) = (uint8_t)(c + 1);              /* 7613 */
    NEMICI_PUNTO(0x7614, 0x7617);
    tabella[slot] = 0x07;                      /* 7616-7617 */
    NEMICI_PUNTO(0x7619, 0x761a);
    sub_7c84(&z);                              /* 761A */
    NEMICI_PUNTO(0x761d, 0x761d);
    sub_75aa(&z);                              /* 761D */
    NEMICI_PUNTO(0x7620, 0x7620);
    sub_33eb(&z);                              /* 7620 */
    NEMICI_PUNTO(0x7623, 0x7625);
    M(0xc25d) = 0x50;                          /* 7623-7625 */
    uint16_t rec = (uint16_t)(0x765b + 3 * c); /* 7628-762D */
    NEMICI_PUNTO(0x7628, 0x7630);
    M(0xc243) = leggi(rec);                    /* 762F-7630 */
    NEMICI_PUNTO(0x7633, 0x7635);
    NEMICI_PUNTO(0x0010, 0x0014);              /* rst 10h di 7634 */
    M(0xc1b7) = leggi((uint16_t)(rec + 1));    /* 7634-7635 ld (c1b7),de */
    M(0xc1b8) = leggi((uint16_t)(rec + 2));
    NEMICI_PUNTO(0x7639, 0x763a);
    M(0xc053) = 0x00;                          /* 7639-763A */
}

/* 06, 763E (indici 1, 6, 9): attesa. Con c467 < 40 non fa nulla. Altrimenti
 * c467 = 0 e il contatore c1af si avanza con il record di indice c1ae
 * della tabella 765B (sub_2551): se il byte del record è minore del
 * contatore, c1af = 0 e c1ae avanza di uno. */
static void fase_763e(void)
{
    NEMICI_PUNTO(0x763e, 0x7646);              /* stato d'ingresso */
    if (M(0xc467) >= 0x40) {                   /* 763E-7643 */
        M(0xc467) = 0x00;                      /* 7646 */
        NEMICI_PUNTO(0x7647, 0x7650);
        struct z80 z = {0};
        SET_BC(&z, M(0xc1ae));                 /* 7649-764B */
        SET_DE(&z, 0xc1ae);                    /* 764C ex de,hl */
        SET_HL(&z, 0x765b);                    /* 764D */
        sub_2551(&z);                          /* 7650 */
        NEMICI_PUNTO(0x7653, 0x7655);
        if (!(z.f & 0x01)) {                   /* 7653 jr c */
            M(HL(&z)) = 0x00;                  /* 7655 */
            NEMICI_PUNTO(0x7657, 0x7658);
            M(0xc1ae)++;                       /* 7657-7658 */
        }
    }
    NEMICI_PUNTO(0x7659, 0x765a);              /* pop hl; ret */
}

/* 06, 78E0 (indice 0): primo quadro dell'attract. Avanza c1ae, c1af = 0,
 * azzera i quadri (75AA) e la tabella delle attività (673F), posta 14 (e
 * 08 con c04c acceso, che termina). Altrimenti quadro 0, fondale (322A),
 * palette (33BA, C = 0), 3F88 con HL = 80B3, 7ECA, le quattro stringhe da
 * 79A4 a d6a8, d6e7, d72c, d74c; poi le righe d'inizio dal bit 7 di c048
 * (79DF due volte e le cifre di c04a-c04b, oppure 79FA e c04a); c467 = 0 e
 * posta 06. */
static void fase_78e0(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x78e0, 0x78e0);
    M(0xc1ae)++;                               /* 78E0 */
    NEMICI_PUNTO(0x78e1, 0x78e3);
    M(0xc1af) = 0x00;                          /* 78E1-78E3 */
    NEMICI_PUNTO(0x78e4, 0x78e4);
    PUNTO_LARGO(0x75aa, 0x75dc);               /* ritorni in 75AA (75B4, 75D3): prima, il loro c29a lo azzera la routine */
    sub_75aa(&z);                              /* 78E4 */
    NEMICI_PUNTO(0x78e7, 0x78e7);
    sub_673f(&z);                              /* 78E7 */
    NEMICI_PUNTO(0x78ea, 0x78ec);
    posta(0x14);                      /* 78EA-78EC */
    if (M(0xc04c) != 0) {                      /* 78EF-78F3 */
        NEMICI_PUNTO(0x78ef, 0x78f7);
        posta(0x08);                  /* 78F5-78F7 */
        NEMICI_PUNTO(0x78fa, 0x78fb);
        return;
    }
    NEMICI_PUNTO(0x78ef, 0x78fe);
    M(0xc243) = 0x00;                          /* 78FC-78FE */
    NEMICI_PUNTO(0x78ff, 0x7901);
    sub_322a(&z);                              /* 7901 */
    PUNTO_LARGO(0x3225, 0x33b9);               /* ritorni in 322A e figlie (32xx-33xx) */
    NEMICI_PUNTO(0x7904, 0x7906);
    z.c = 0x00;                                /* 7904 */
    sub_33ba(&z);                              /* 7906 */
    NEMICI_PUNTO(0x7909, 0x790c);
    SET_HL(&z, 0x80b3);                        /* 7909 */
    sub_3f88(&z);                              /* 790C */
    NEMICI_PUNTO(0x790f, 0x790f);
    sub_7eca(&z);                              /* 790F */
    PUNTO_LARGO(0x7eca, 0x7f40);               /* ritorni in 7ECA e figlie (7F03, 7F10) */
    static const uint16_t dest[4] = { 0xd6a8, 0xd6e7, 0xd72c, 0xd74c };
    static const uint16_t sito[4] = { 0x7912, 0x791b, 0x7921, 0x7927 };
    SET_HL(&z, 0x79a4);                        /* 7912 */
    for (int i = 0; i < 4; i++) {              /* 7915-792A, HL prosegue */
        NEMICI_PUNTO(sito[i], (uint16_t)(sito[i] + (i == 0 ? 6 : 3)));
        SET_DE(&z, dest[i]);
        sub_6771(&z);
    }
    if (M(0xc048) & 0x80) {                    /* 792D-7931 rlca; jr c */
        NEMICI_PUNTO(0x792d, 0x7967);
        SET_HL(&z, 0x79fa);                    /* 7961 */
        SET_DE(&z, 0xd649);
        sub_6771(&z);                          /* 7967 */
        NEMICI_PUNTO(0x796a, 0x7970);
        SET_HL(&z, 0xc04a);                    /* 796A */
        SET_DE(&z, 0xd649);
        sub_797e(&z);                          /* 7970 */
    } else {
        NEMICI_PUNTO(0x792d, 0x7939);
        SET_HL(&z, 0x79df);                    /* 7933 */
        SET_DE(&z, 0xd624);
        sub_6771(&z);                          /* 7939 */
        NEMICI_PUNTO(0x793c, 0x7942);
        SET_HL(&z, 0x79df);
        SET_DE(&z, 0xd664);
        sub_6771(&z);                          /* 7942 */
        NEMICI_PUNTO(0x7945, 0x7948);
        M(0xd629) = 0x41;                      /* 7948 */
        NEMICI_PUNTO(0x794a, 0x794d);
        M(0xd669) = 0x42;                      /* 794D */
        NEMICI_PUNTO(0x794f, 0x7955);
        SET_HL(&z, 0xc04a);
        SET_DE(&z, 0xd62e);
        sub_797e(&z);                          /* 7955 */
        NEMICI_PUNTO(0x7958, 0x795c);
        SET_HL(&z, 0xc04b);                    /* 7958 inc hl */
        SET_DE(&z, 0xd66e);
        sub_797e(&z);                          /* 795C */
    }
    NEMICI_PUNTO(0x7973, 0x7974);
    M(0xc467) = 0x00;                          /* 7973-7974 */
    NEMICI_PUNTO(0x7977, 0x7979);
    posta(0x06);                      /* 7977-7979 */
    NEMICI_PUNTO(0x797c, 0x797d);
}

/* 06, ingresso 75E7: salto con `rst 20h` alla tabella 75EC sulla fase
 * c1ae (10 voci: 78E0 763E 760B 760B 760B 7A0B 763E 760B 7B3B 763E). Le
 * fasi 5 (7A0B) e 8 (7B3B), mai eseguite nel giro dal boot, sono del
 * lotto 55 (attivita_06b.c, senza punti di sincronia). La tabella ha in
 * realtà 12 voci (rst20_tables.h ne dichiara 10): 10 è 760B e 11 è 7604,
 * la chiusura del ciclo subito dopo la tabella (lotto 55). */
void attivita_06_75e7(uint8_t slot)
{
    NEMICI_PUNTO(0x75e7, 0x75eb);              /* prima del rst 20h */
    NEMICI_PUNTO(0x0008, 0x0026);              /* gestore del rst 20h */
    uint8_t fase = M(0xc1ae);                  /* 75E7-75EA */
    switch (fase) {                            /* 75EB rst 20h */
    case 0:
        fase_78e0();
        break;
    case 1: case 6: case 9:
        fase_763e();
        break;
    case 2: case 3: case 4: case 7:
        fase_760b(slot);
        break;
    case 5:
        fase_06_7a0b();
        break;
    case 8:
        fase_06_7b3b();
        break;
    case 10:
        fase_760b(slot);
        break;
    case 11:
        fase_06_7604();
        break;
    default:
        nemici_fuori_tabella(0x75eb, fase);
        break;
    }
}
