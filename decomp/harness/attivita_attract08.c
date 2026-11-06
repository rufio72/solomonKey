/* Corpi 08 (7C98), 0C (7BF6) e 0B (2C27) dell'avvio, lotto H4b3 (brief 47):
 * le schermate di fine attract (quadro delle monete, attesa dell'avvio
 * partita, ritorno al quadro di partenza). Stesse regole di
 * attivita_attract.c: `pop hl; ret` diventa `return` e
 * `pop hl; ld a,n; call 0493; ret` diventa una posta seguita da `return`;
 * i registri del Z80 servono solo a passare gli argomenti alle routine
 * tradotte. NEMICI_PUNTO: [da, a] dalla prima istruzione dopo l'ultima
 * scrittura in RAM fino all'istruzione che scrive (o chiama) inclusa;
 * PUNTO_LARGO per i ritorni dentro routine tradotte senza punti interni. */
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

/* 6771 con HL = stringa e DE = destinazione; i ritorni delle NMI dentro
 * la routine (6775-6783) si eseguono al suo termine. */
static void stringa(struct z80 *z, uint16_t hl, uint16_t de)
{
    SET_HL(z, hl);
    SET_DE(z, de);
    sub_6771(z);
    PUNTO_LARGO(0x6771, 0x6784);
}

/* 75AA: i ritorni delle NMI nel ciclo che pulisce i piani video (75B1-75B8)
 * precedono l'azzeramento di c299-c2a8 (75C3) e si eseguono prima della
 * routine; quelli dopo (75BD-75E6, per esempio 75E0) a routine finita. */
static void quadro_75aa(struct z80 *z)
{
    PUNTO_LARGO(0x75aa, 0x75bc);
    sub_75aa(z);
    PUNTO_LARGO(0x75bd, 0x75e6);
}

/* 0B, ingresso 2C27: azzera c234-c236, prepara il tabellone (33EB), scambia
 * i due blocchi c237 e c268 (2D8A), ripete 33EB, azzera la tabella delle
 * attività (673F) e c19d, riempie con 0 d440-d45f e il piano d040-d05f,
 * posta 1F, ridisegna le cifre del punteggio (4D05; con il bit 2 di c045
 * acceso anche la copia con c045 + 1), poi posta 07. */
void attivita_0b_2c27(uint8_t slot)
{
    (void)slot;
    struct z80 z = {0};
    for (int i = 0; i < 3; i++) {              /* 2C2D-2C2F */
        NEMICI_PUNTO(i == 0 ? 0x2c27 : 0x2c2d, 0x2c2d);
        M(0xc234 + i) = 0x00;                  /* 2C2D */
    }
    NEMICI_PUNTO(0x2c2e, 0x2c31);
    sub_33eb(&z);                              /* 2C31 */
    NEMICI_PUNTO(0x2c34, 0x2c34);
    sub_2d8a(&z);                              /* 2C34 */
    PUNTO_LARGO(0x2d8a, 0x2d9c);               /* ritorni dentro lo scambio */
    NEMICI_PUNTO(0x2c37, 0x2c37);
    sub_33eb(&z);                              /* 2C37 */
    NEMICI_PUNTO(0x2c3a, 0x2c3a);
    sub_673f(&z);                              /* 2C3A */
    NEMICI_PUNTO(0x2c3d, 0x2c3e);
    M(0xc19d) = 0x00;                          /* 2C3E */
    for (int i = 0; i < 0x20; i++) {           /* 2C47-2C4E */
        NEMICI_PUNTO(i == 0 ? 0x2c41 : 0x2c4d, 0x2c47);
        M(0xd440 + i) = 0x00;                  /* 2C47 */
        NEMICI_PUNTO(0x2c48, 0x2c4a);
        M(0xd040 + i) = 0x00;                  /* 2C4A */
    }
    NEMICI_PUNTO(0x2c4d, 0x2c52);
    posta(0x1f);                               /* 2C52 */
    NEMICI_PUNTO(0x2c55, 0x2c59);
    sub_4d05(&z);                              /* 2C59 */
    NEMICI_PUNTO(0x2c5c, 0x2c64);
    if (M(0xc045) & 0x04) {                    /* 2C5D-2C5F */
        nemici_manca(0x2c61);
        return;
    }
    NEMICI_PUNTO(0x2c6b, 0x2c6e);
    if (partita_stanza >= 0) {                 /* collaudo (gioca --stanza N): */
        M(0xc243) = (uint8_t)partita_stanza;   /* quadro del giocatore 1 */
        M(0xc274) = (uint8_t)partita_stanza;   /* e del 2 (blocco c268, 2D8A) */
    }
    posta(0x07);                               /* 2C6E */
    NEMICI_PUNTO(0x2c71, 0x2c71);
}

/* 0C, ingresso 7BF6: schermata dei crediti. Con c04c = 1 la stringa 7C58,
 * altrimenti d711 = 73 e la stringa 7C6E, a d646 (6771); le cifre di c04c
 * (3A1C) in d712-d713. Con c052 & 3 pari (0 o 3, `jp pe`) ritorna: così
 * 191 volte su 192. Con 1 o 2 (avvio partita a 1 o 2 giocatori, C) e
 * crediti sufficienti toglie C crediti, c045 = 2·(C | 1), azzera la tabella
 * delle attività (673F), accende il bit 7 di c044, palette (3F88 con HL =
 * 80D3), 7C84, 75AA e posta 0B. */
void attivita_0c_7bf6(uint8_t slot)
{
    (void)slot;
    struct z80 z = {0};
    uint8_t crediti = M(0xc04c);               /* 7BF6 */
    uint16_t hl = 0x7c58;
    if (crediti != 1) {                        /* 7BFD-7BFE */
        NEMICI_PUNTO(0x7bf6, 0x7c03);
        M(0xd711) = 0x73;                      /* 7C03 */
        hl = 0x7c6e;
    }
    NEMICI_PUNTO(0x7c05, 0x7c0b);
    stringa(&z, hl, 0xd646);                   /* 7C0B */
    z.a = crediti;
    sub_3a1c(&z);                              /* 7C10 */
    NEMICI_PUNTO(0x7c13, 0x7c16);
    M(0xd713) = z.e;                           /* 7C16 */
    if (z.d != 0x30) {                         /* 7C17-7C1A */
        NEMICI_PUNTO(0x7c17, 0x7c1d);
        M(0xd712) = z.d;                       /* 7C1D */
    }
    NEMICI_PUNTO(0x7c1e, 0x7c23);
    uint8_t c = (uint8_t)(M(0xc052) & 0x03);   /* 7C1E-7C21 */
    if (c == 0 || c == 3) {                    /* 7C23 jp pe,7C28 */
        NEMICI_PUNTO(0x7c28, 0x7c29);
        return;
    }
    if (crediti < c) {                         /* 7C2A-7C2D */
        NEMICI_PUNTO(0x7c28, 0x7c29);
        return;
    }
    for (int i = 0; i < c; i++) {              /* 7C33-7C34 */
        NEMICI_PUNTO(i == 0 ? 0x7c2f : 0x7c34, 0x7c33);
        M(0xc04c)--;                           /* 7C33 */
    }
    NEMICI_PUNTO(0x7c36, 0x7c3a);
    M(0xc045) = (uint8_t)((c | 0x01) << 1);    /* 7C3A */
    NEMICI_PUNTO(0x7c3b, 0x7c3d);
    sub_673f(&z);                              /* 7C3D */
    NEMICI_PUNTO(0x7c40, 0x7c43);
    M(0xc044) |= 0x80;                         /* 7C43 */
    NEMICI_PUNTO(0x7c44, 0x7c48);
    SET_HL(&z, 0x80d3);
    sub_3f88(&z);                              /* 7C48 */
    NEMICI_PUNTO(0x7c4b, 0x7c4b);
    sub_7c84(&z);                              /* 7C4B */
    NEMICI_PUNTO(0x7c4e, 0x7c4e);
    quadro_75aa(&z);                           /* 7C4E */
    NEMICI_PUNTO(0x7c51, 0x7c53);
    posta(0x0b);                               /* 7C53 */
    NEMICI_PUNTO(0x7c56, 0x7c57);
}

/* 08, 7D6A (indice 0 della tabella 7CED, DSW2 e605 con i bit 5-7 a 0): le
 * tre stringhe dell'esito (7DEA) e tre righe di punteggio (7E24 con A = 0,
 * 1, 2 e HL = 7E46, 7E56, 7E66). */
static void esiti_7d6a(void)
{
    struct z80 z = {0};
    NEMICI_PUNTO(0x7d6a, 0x7d6a);
    sub_7dea(&z);                              /* 7D6A */
    PUNTO_LARGO(0x6771, 0x6784);
    static const uint16_t righe[3] = { 0x7e46, 0x7e56, 0x7e66 };
    for (int i = 0; i < 3; i++) {
        NEMICI_PUNTO(0x7d6d, 0x7d71);
        z.a = (uint8_t)i;
        SET_HL(&z, righe[i]);
        sub_7e24(&z);                          /* 7D71, 7D79, 7D81 */
        PUNTO_LARGO(0x6771, 0x6784);
    }
    NEMICI_PUNTO(0x7d84, 0x7d84);
}

/* 08, ingresso 7C98: quadro dell'esito di fine partita. 75AA, 7C84, suono
 * 3E, azzera la tabella delle attività (673F), posta 14, palette (3F88 con
 * HL = 80B3), tre stringhe (7EA7 a d70B, 7EB2 a d748, 7EC4 a d62E),
 * c243 = 0, fondale (322A), palette (33BA con C = 1), cornice (7ECA), posta
 * 0C; poi `rst 20h` sulla tabella 7CED con l'indice (e605 >> 5) & 7: solo
 * la voce 0 (7D6A) è eseguita. */
void attivita_08_7c98(uint8_t slot)
{
    (void)slot;
    struct z80 z = {0};
    NEMICI_PUNTO(0x7c98, 0x7c98);
    quadro_75aa(&z);                           /* 7C98 */
    NEMICI_PUNTO(0x7c9b, 0x7c9b);
    sub_7c84(&z);                              /* 7C9B */
    NEMICI_PUNTO(0x7c9e, 0x7ca0);
    z.c = 0x3e;
    sub_0028(&z);                              /* 7CA0 rst 28h */
    NEMICI_PUNTO(0x7ca1, 0x7ca1);
    sub_673f(&z);                              /* 7CA1 */
    NEMICI_PUNTO(0x7ca4, 0x7ca6);
    posta(0x14);                               /* 7CA6 */
    NEMICI_PUNTO(0x7ca9, 0x7cac);
    SET_HL(&z, 0x80b3);
    sub_3f88(&z);                              /* 7CAC */
    scrivi_porta(0xe604, 0x00);                /* 7CB0 */
    NEMICI_PUNTO(0x7cb3, 0x7cb9);
    stringa(&z, 0x7ea7, 0xd70b);               /* 7CB9 */
    NEMICI_PUNTO(0x7cbc, 0x7cc2);
    stringa(&z, 0x7eb2, 0xd748);               /* 7CC2 */
    NEMICI_PUNTO(0x7cc5, 0x7ccb);
    stringa(&z, 0x7ec4, 0xd62e);               /* 7CCB */
    NEMICI_PUNTO(0x7cce, 0x7cd0);
    M(0xc243) = 0x00;                          /* 7CD0 */
    NEMICI_PUNTO(0x7cd1, 0x7cd3);
    fondale_322a(&z);                          /* 7CD3 */
    NEMICI_PUNTO(0x7cd6, 0x7cd8);
    PUNTO_LARGO(0x33ba, 0x33e9);               /* ritorni dentro 33BA: prima, 33BA accende il bit 6 di c044 */
    z.c = 0x01;
    sub_33ba(&z);                              /* 7CD8 */
    NEMICI_PUNTO(0x7cdb, 0x7cdb);
    sub_7eca(&z);                              /* 7CDB */
    PUNTO_LARGO(0x7eca, 0x7f40);               /* ritorni dentro 7ECA e figlie */
    NEMICI_PUNTO(0x7cde, 0x7ce0);
    posta(0x0c);                               /* 7CE0 */
    NEMICI_PUNTO(0x7ce3, 0x7cec);
    uint8_t voce = (uint8_t)((leggi_porta(0xe605) >> 5) & 0x07);   /* 7CE4-7CEA */
    NEMICI_PUNTO(0x0008, 0x0026);              /* gestore del rst 20h */
    switch (voce) {                          /* 7CEC rst 20h */
    case 0: esiti_7d6a(); break;
    case 1: nemici_manca(0x7dc6); break;
    case 2: nemici_manca(0x7da0); break;
    case 3: nemici_manca(0x7dde); break;
    case 4: nemici_manca(0x7d85); break;
    case 5: nemici_manca(0x7dd2); break;
    case 6: nemici_manca(0x7db3); break;
    case 7: nemici_manca(0x3b62); break;
    }
}
