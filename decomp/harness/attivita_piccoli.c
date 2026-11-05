/* Corpi brevi delle attività del gioco libero, lotto 52 (brief 52):
 * 04 (4C30), 05 (4C42, con 4C89), 09 (4D29), 0F (640E), 17 (6429),
 * 19 (3792), 1A (411D), 1B (4111), 1D (3F39), 23 (3E4E), tradotti dal
 * disassemblato out/main64.dasm. Regole come attivita_01_10.c: `pop hl;
 * ret` diventa `return`, `pop hl; ld (hl),n; ret` diventa `tabella[slot]
 * = n; return`; i registri del Z80 (struct z80) servono solo a passare
 * gli argomenti alle routine tradotte e ne seguono il valore originale. */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"

/* 04, ingresso 4C30: attende c463 >= 0A, poi c46e = 41 e slot 03. */
void attivita_04_4c30(uint8_t slot)
{
    if (M(0xc463) < 0x0a)                      /* 4C30-4C38 pop hl; ret c */
        return;
    M(0xc46e) = 0x41;                          /* 4C39-4C3B */
    tabella[slot] = 0x03;                      /* 4C3E-4C41 */
}

/* 05, ingresso 4C42: c465 = 0, slot 09; l'oggetto c88e prende le
 * coordinate della parola c058 e lo stato C1 (4AF6) con B = (c05a & 7C)
 * >> 2, C = c05a & 3, E = 2. Col bit 7 di c05a spento scrive il blocco
 * della cella (24BE, 4C11 con BC = 0). Se c1aa-c1ac non sono nulli
 * aggiunge il premio al punteggio (4C89) e lo ridisegna (4D05). */
void attivita_05_4c42(uint8_t slot)
{
    struct z80 z = {0};
    M(0xc465) = 0x00;                          /* 4C42-4C46 */
    tabella[slot] = 0x09;                      /* 4C47-4C48 */
    z.l = M(0xc058);                           /* 4C4A ld hl,(c058) */
    z.h = M(0xc059);
    z.ix = 0xc88e;                             /* 4C4D */
    M(z.ix + 0x0e) = z.h;                      /* 4C51 */
    M(z.ix + 0x15) = z.l;                      /* 4C54 */
    z.a = M(0xc05a);                           /* 4C57 */
    z.d = z.a;                                 /* 4C5A */
    z.b = (uint8_t)((z.a & 0x7c) >> 2);        /* 4C5B-4C5F */
    z.e = 0x02;                                /* 4C60 */
    z.c = (uint8_t)(z.d & 0x03);               /* 4C62-4C65 */
    z.a = 0xc1;                                /* 4C66 */
    sub_4af6(&z);                              /* 4C68 */
    if (!(z.d & 0x80)) {                       /* 4C6B-4C6D */
        uint16_t de = DE(&z);                  /* 4C6F ex de,hl */
        SET_DE(&z, HL(&z));
        SET_HL(&z, de);
        sub_24be(&z);                          /* 4C70 */
        SET_BC(&z, 0x0000);                    /* 4C73 */
        sub_4c11(&z);                          /* 4C76 */
    }
    z.l = M(0xc1aa);                           /* 4C79 ld hl,(c1aa) */
    z.h = M(0xc1ab);
    z.a = (uint8_t)(M(0xc1ac) | z.h | z.l);    /* 4C7C-4C80 */
    if (z.a == 0)                              /* 4C81 ret z */
        return;
    sub_4c89(&z);                              /* 4C82 */
    sub_4d05(&z);                              /* 4C85 */
}

/* 0x4C89 (4C82): premio c1aa-c1ac. Registra il gestore 10 (4398), mette
 * le cifre del premio a c061 (4D37) e le scrive a video (4D87 da dc6a, o
 * dc7f col bit 0 di c045, attributo 61). Poi, in ciclo: somma il premio
 * (somma binaria a 24 bit con riporto) al punteggio c239-c23c e azzera
 * c1aa-c1ac; se il punteggio a 32 bit è sotto la soglia puntata da c23d
 * ritorna, altrimenti avanza la soglia di 4 byte, incrementa le vite c237
 * (barra 31E9 con c237, meno uno se il bit 7 di c82e è acceso), suono 12,
 * e ribalta il bit 3 di c1b9/e803 se coincide col bit 3 di e603 (con il
 * bit diverso chiama 75AA). Uscita: registri dell'ultimo confronto. */
void sub_4c89(struct z80 *z)
{
    z->a = 0x10;                               /* 4C89 */
    sub_4398(z);                               /* 4C8B */
    z->l = M(0xc1aa);                          /* 4C8E ld hl,(c1aa) */
    z->h = M(0xc1ab);
    z->a = M(0xc1ac);                          /* 4C91 */
    z->e = z->a;                               /* 4C94 */
    z->d = 0x00;                               /* 4C95 */
    sub_4d37(z);                               /* 4C97 */
    z->a = M(0xc045);                          /* 4C9A */
    SET_HL(z, (z->a & 0x01) ? 0xdc7f : 0xdc6a);    /* 4C9D-4CA3 rrca */
    z->a = (uint8_t)((z->a >> 1) | (z->a << 7));
    SET_DE(z, 0xc061);                         /* 4CA6 */
    z->c = 0x61;                               /* 4CA9 */
    sub_4d87(z);                               /* 4CAB */
    for (;;) {
        z->b = 0x03;                           /* 4CAE */
        uint16_t hl = 0xc1aa, de = 0xc239;     /* 4CB0-4CB6 or a */
        unsigned riporto = 0, somma;
        for (; z->b > 0; z->b--) {             /* 4CB7-4CBE */
            somma = M(de) + M(hl) + riporto;
            M(de) = (uint8_t)somma;
            riporto = somma >> 8;
            M(hl) = 0x00;
            hl++;
            de++;
        }
        somma = M(de) + riporto;               /* 4CC0-4CC3 adc a,0 */
        M(de) = (uint8_t)somma;
        de++;                                  /* 4CC4 */
        SET_HL(z, de);                         /* 4CC5 ex de,hl */
        SET_DE(z, hl);
        sub_0018(z);                           /* 4CC6 rst 18: HL = soglia */
        hl = HL(z);
        de = 0xc239;                           /* 4CC7 */
        unsigned prestito = 0;                 /* 4CCC xor a */
        for (z->b = 0x04; z->b > 0; z->b--) {  /* 4CCD-4CD1 */
            unsigned d = (unsigned)M(de) - leggi(hl) - prestito;
            z->a = (uint8_t)d;
            prestito = (d >> 8) & 1;
            hl++;
            de++;
        }
        SET_HL(z, hl);
        SET_DE(z, de);
        if (prestito)                          /* 4CD3 ret c */
            return;
        M(0xc23d) = z->l;                      /* 4CD4 ld (c23d),hl */
        M(0xc23e) = z->h;
        uint8_t bit7 = M(0xc82e) & 0x80;       /* 4CD7, 4CDE rlca */
        SET_HL(z, 0xc237);                     /* 4CDA */
        M(0xc237)++;                           /* 4CDD */
        z->a = M(0xc237);                      /* 4CDF */
        if (bit7)                              /* 4CE0-4CE2 */
            z->a--;
        sub_31e9(z);                           /* 4CE3 */
        z->c = 0x12;                           /* 4CE6 */
        sub_0028(z);                           /* 4CE8 rst 28 */
        z->a = M(0xc1b9);                      /* 4CE9 */
        z->c = z->a;                           /* 4CEC */
        z->a = (uint8_t)(z->c & 0x08);         /* 4CED: la protezione di MAME restituisce
                                                  BC & 08 (att_25_e603.txt) */
        z->a = (uint8_t)((z->a ^ z->c) & 0x08);    /* 4CF0-4CF1 */
        if (z->a != 0) {                       /* 4CF3 */
            sub_75aa(z);                       /* 4D00 */
            continue;                          /* 4D03 */
        }
        z->a = (uint8_t)(z->c ^ 0x08);         /* 4CF5-4CF6 */
        scrivi_porta(0xe803, z->a);            /* 4CF8 */
        M(0xc1b9) = z->a;                      /* 4CFB */
    }                                          /* 4CFE */
}

/* 09, ingresso 4D29: con c465 >= 14 azzera c88e e lo slot. */
void attivita_09_4d29(uint8_t slot)
{
    if (M(0xc465) < 0x14)                      /* 4D29-4D2F */
        return;
    M(0xc88e) = 0x00;                          /* 4D31-4D32 */
    tabella[slot] = 0x00;                      /* 4D35 */
}

/* 0F, ingresso 640E: attende c463 >= 8, poi tabellone (349F), chiusura
 * dell'oggetto c46e (4B98), c4eb = 0 e slot 03. */
void attivita_0f_640e(uint8_t slot)
{
    struct z80 z = {0};
    if (M(0xc463) < 0x08)                      /* 640E-6416 */
        return;
    sub_349f(&z);                              /* 6417 */
    sub_4b98(0xc46e);                          /* 641A-641E */
    M(0xc4eb) = 0x00;                          /* 6421-6422 */
    tabella[slot] = 0x03;                      /* 6425-6428 */
}

/* 17, ingresso 6429: spegne il bit 3 di c045. Senza i bit 1-2 di c045
 * spegne il bit 7 di c044 e lo slot prende 06 (c04c nullo) o 08.
 * Altrimenti, se il bit del giocatore che segue (bit 2 con il bit 0
 * spento, bit 1 con il bit 0 acceso) è acceso, ribalta il bit 0 di c045
 * e scambia i dati dei giocatori (2D8A); lo slot prende 07, o 24 col bit 7
 * di c24e acceso. */
void attivita_17_6429(uint8_t slot)
{
    struct z80 z = {0};
    M(0xc045) &= (uint8_t)~0x08;               /* 6429-642C */
    uint8_t c = M(0xc045);                     /* 642E-642F */
    uint8_t b;
    if (!(c & 0x06)) {                         /* 6430-6432 */
        M(0xc044) &= (uint8_t)~0x80;           /* 6434-6437 */
        b = M(0xc04c) ? 0x08 : 0x06;           /* 6439-6441 */
        tabella[slot] = b;                     /* 6463-6465 */
        return;
    }
    uint8_t a = (uint8_t)(((c & 0x01) ^ 0x01) + 1);    /* 6445-644A */
    a = (uint8_t)((a << 1) | (a >> 7));        /* 644B rlca */
    if (a & c) {                               /* 644C-644D */
        M(0xc045) = (uint8_t)(c ^ 0x01);       /* 644F-6452 */
        sub_2d8a(&z);                          /* 6455 */
    }
    b = (M(0xc24e) & 0x80) ? 0x24 : 0x07;      /* 6458-6461 rl a */
    tabella[slot] = b;                         /* 6463-6465 */
}

/* 19, ingresso 3792: cerca un posto libero fra gli oggetti (7379); se non
 * c'è resta 19. Altrimenti mette l'oggetto della cella c247 (37B4 con A =
 * c247, B = c1ad, C = E = indice), accende il bit 6 del byte puntato dalla
 * voce E della tabella 48AF, suono 2B e slot 0. */
void attivita_19_3792(uint8_t slot)
{
    struct z80 z = {0};
    sub_7379(&z);                              /* 3792 */
    if (!NEMICI_RIPORTO_ACCESO(&z))            /* 3795-3798 */
        return;
    z.c = z.a;                                 /* 3799 */
    z.e = z.a;                                 /* 379A */
    z.b = M(0xc1ad);                           /* 379B-379E */
    z.a = M(0xc247);                           /* 379F */
    sub_37b4(&z);                              /* 37A2 */
    z.a = z.e;                                 /* 37A5 */
    SET_HL(&z, 0x48af);                        /* 37A6 */
    sub_0008(&z);                              /* 37A9 rst 8 */
    sub_0018(&z);                              /* 37AA rst 18 */
    M(HL(&z)) |= 0x40;                         /* 37AB */
    z.c = 0x2b;                                /* 37AD */
    sub_0028(&z);                              /* 37AF rst 28 */
    tabella[slot] = 0x00;                      /* 37B0-37B3 */
}

/* 1A, ingresso 411D: record della stanza-bonus (354B), slot 1B. */
void attivita_1a_411d(uint8_t slot)
{
    struct z80 z = {0};
    sub_354b(&z);                              /* 411D */
    tabella[slot] = 0x1b;                      /* 4120-4123 */
}

/* 1B, ingresso 4111: attende c463 >= 12, poi slot 02. */
void attivita_1b_4111(uint8_t slot)
{
    if (M(0xc463) < 0x12)                      /* 4111-4119 */
        return;
    tabella[slot] = 0x02;                      /* 411A-411C */
}

/* 1D, ingresso 3F39: tabellone (349F), slot 05. */
void attivita_1d_3f39(uint8_t slot)
{
    struct z80 z = {0};
    sub_349f(&z);                              /* 3F39 */
    tabella[slot] = 0x05;                      /* 3F3C-3F3F */
}

/* 23, ingresso 3E4E (resta 23): ogni 5 tick di c464 (da ca4a) avanza il
 * contatore ca4b ciclico su 5 e copia il record 8 (3569) dalla voce
 * B·3 parole della tabella 3E72, B = valore di ca4b prima dell'avanzo. */
void attivita_23_3e4e(uint8_t slot)
{
    (void)slot;                                /* 3E4E pop hl */
    struct z80 z = {0};
    uint8_t c = M(0xc464);                     /* 3E4F-3E55 */
    if ((uint8_t)(c - M(0xca4a)) < 0x05)       /* 3E56-3E59 ret c */
        return;
    M(0xca4a) = c;                             /* 3E5A */
    uint8_t b = M(0xca4b);                     /* 3E5B-3E5D */
    uint8_t a = (uint8_t)(b + 1);              /* 3E5E-3E63 */
    if (a >= 0x05)
        a = 0x00;
    M(0xca4b) = a;                             /* 3E64 */
    z.b = b;
    z.c = c;
    z.a = (uint8_t)(b * 3);                    /* 3E65-3E67 */
    SET_HL(&z, 0x3e72);                        /* 3E68 */
    sub_0008(&z);                              /* 3E6B rst 8 */
    z.a = 0x08;                                /* 3E6C */
    sub_3569(&z);                              /* 3E6E */
}
