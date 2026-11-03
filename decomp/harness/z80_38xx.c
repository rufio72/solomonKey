/* Routine del Z80 principale fra 0x3800 e 0x3fff, tradotte in C leggibile
 * (spostate da z80_3xxx.c quando ha superato le 600 righe, nell'ordine
 * che avevano). Si riproducono l'effetto sulla RAM e i registri A, BC,
 * DE, HL, IX, IY all'uscita (le bandiere F no). */
#include "routine.h"

/* 0x3A1C: due cifre ASCII di A (0..99 atteso).
 * Uscita: D = '0' + A/10, E = '0' + A%10, A = D. */
void sub_3a1c(struct z80 *z)
{
    uint8_t decine = z->a / 10, unita = z->a % 10;
    z->d = (uint8_t)('0' + decine);
    z->e = (uint8_t)('0' + unita);
    z->a = z->d;
}

/* Tempo in BCD (3 byte, cifre basse prima) e sue cifre separate. */
#define TEMPO_BCD    0xc093
#define TEMPO_CIFRE  0xc061   /* 6 cifre, meno significativa prima, poi terminatore */
#define FINE_CIFRE   0xff

/* 0x3F10: scompone le 3 parole BCD di c093-c095 in 6 cifre a c061-c066 e
 * scrive 0xff subito dopo la cifra non nulla più significativa (dopo
 * c061 se sono tutte zero).
 * Uscita: A = 0xff, B = posizione (1..6, 0 se tutte zero), C = byte c095,
 * DE = indirizzo del terminatore, HL = c096. */
void sub_3f10(struct z80 *z)
{
    for (int i = 0; i < 3; i++) {
        uint8_t c = M(TEMPO_BCD + i);
        M(TEMPO_CIFRE + 2 * i) = c & 0x0f;
        M(TEMPO_CIFRE + 2 * i + 1) = c >> 4;
        z->c = c;
    }
    /* dall'ultima cifra verso la prima, la prima non nulla */
    uint8_t b = 6;
    uint16_t hl = TEMPO_CIFRE + 5;
    for (; b > 0; b--, hl--)
        if (M(hl) != 0)
            break;
    if (b == 0)
        hl = TEMPO_CIFRE;   /* nessuna cifra non nulla: si ferma su c061 */
    M(hl + 1) = FINE_CIFRE;
    z->b = b;
    z->a = FINE_CIFRE;
    SET_DE(z, hl + 1);
    SET_HL(z, TEMPO_BCD + 3);
}

/* 0x380A: riempie 16 byte da HL con 0xff.
 * Uscita: B = 0, C = 0xff, HL avanzato di 16. */
void sub_380a(struct z80 *z)
{
    uint16_t hl = HL(z);
    for (int i = 0; i < 16; i++)
        M(hl++) = 0xff;
    SET_BC(z, 0x00ff);
    SET_HL(z, hl);
}

/* Tabelle ROM di coppie di byte. */
#define TABELLA_8578 0x8578   /* indici 0..7 e 8, 9, 10 per A >= 0x40 */
#define TABELLA_858E 0x858e   /* indici A-8 per 8 <= A < 0x40 */

/* 0x3A2E: coppia di byte scelta da A.
 * A < 8: indice A in 8578; A < 0x40: indice A-8 in 858E; altrimenti indice
 * 8 (A < c0), 9 (A < f8) o 10 in 8578.
 * Uscita: B, C = coppia, A = DE = indice, HL sul secondo byte. */
void sub_3a2e(struct z80 *z)
{
    uint8_t a = z->a, indice;
    uint16_t tabella = TABELLA_8578;
    if (a < 0x08)
        indice = a;
    else if (a < 0x40) {
        indice = a - 8;
        tabella = TABELLA_858E;
    } else if (a < 0xc0)
        indice = 8;
    else if (a < 0xf8)
        indice = 9;
    else
        indice = 10;
    z->a = indice;
    SET_HL(z, tabella);
    sub_0008(z);            /* rst 08: DE = indice, HL = tabella + 2·indice */
    uint16_t hl = HL(z);
    z->b = leggi(hl);
    z->c = leggi(hl + 1);
    SET_HL(z, hl + 1);
}

/* 0x3812 (corpo di fase 37D7, siti 37F2/37F7): espande una mappa di bit
 * di 24 righe per 8 colonne da IX nella griglia c0cd: scrive C nelle celle
 * il cui bit è 1 (rla: il riporto del chiamante entra nella rotazione, ma
 * l'ultimo A dipende solo dalle ultime due righe della mappa).
 * Uscita: A = ultima riga ruotata, B = 0, E = 0, HL = c18d, IX += 24. */
void sub_3812(struct z80 *z)
{
    uint16_t hl = 0xc0cd, ix = z->ix;
    uint8_t cy = (uint8_t)(z->f & 1);
    uint8_t a = z->a;
    for (int riga = 0; riga < 0x18; riga++) {
        a = leggi(ix);
        for (int bit = 0; bit < 8; bit++) {
            uint8_t fuori = (uint8_t)(a >> 7);
            a = (uint8_t)((a << 1) | cy);      /* rla */
            cy = fuori;
            if (fuori)
                M(hl) = z->c;
            hl++;
        }
        ix++;
    }
    z->a = a;
    z->b = 0;
    z->e = 0;
    SET_HL(z, hl);
    z->ix = ix;
}

/* 0x3F88 (2F5A, 388E, 790C, 7C48, 7CAC): copia il record 6 della tabella
 * 3574 (sub_3569: destinazione e lunghezza dalla tabella, sorgente HL)
 * e accende il bit 6 di c044 (consegna palette alla NMI).
 * Uscita: HL = c044, gli altri da sub_3569. */
void sub_3f88(struct z80 *z)
{
    z->a = 6;
    sub_3569(z);
    M(0xc044) |= 0x40;
    SET_HL(z, 0xc044);
}

/* ------------------------------------------------------------------ */
/* Lotto 38 (compito 38): punteggio e oggetti dei quadri bonus. */

/* 0x3CC2 (cade da 3CC1): scrive il tile 0x65 a HL con il bit 2 di H
 * spento, poi riaccende il bit 2 e avanza. Uscita: HL = HL + 1 con il
 * bit 2 di H acceso. */
void sub_3cc2(struct z80 *z)
{
    z->h &= (uint8_t)~0x04;                    /* 3CC2 res 2,h */
    M(HL(z)) = 0x65;                           /* 3CC4 */
    z->h |= 0x04;                              /* 3CC6 set 2,h */
    SET_HL(z, HL(z) + 1);                      /* 3CC8 */
}

/* 0x3CC1: (HL) = A e poi 3CC2. */
void sub_3cc1(struct z80 *z)
{
    M(HL(z)) = z->a;                           /* 3CC1 */
    sub_3cc2(z);
}

/* 0x3D2F: somma HL alla parola c1aa e la parte alta DE (solo E) al byte
 * c1ac: il valore a 24 bit c1aa-c1ac cresce di E:HL (con il riporto
 * dalla parola). Uscita: A = c1ac nuovo, BC = vecchia parola c1aa,
 * HL invariato, DE invariato, riporto in F bit 0. */
void sub_3d2f(struct z80 *z)
{
    uint16_t hl = HL(z);
    uint16_t bc = (uint16_t)(M(0xc1aa) | (M(0xc1ab) << 8));
    uint32_t somma = (uint32_t)hl + bc;        /* 3D30-3D34 */
    M(0xc1aa) = (uint8_t)somma;                /* 3D35 */
    M(0xc1ab) = (uint8_t)(somma >> 8);
    unsigned s = M(0xc1ac) + z->e + (somma > 0xffff);   /* 3D3A-3D3D adc a,e */
    M(0xc1ac) = (uint8_t)s;                    /* 3D3E */
    SET_BC(z, bc);
    z->a = (uint8_t)s;
    z->f = (uint8_t)((z->f & ~1) | (s > 0xff));
}

/* 0x3D20 (cade in 3D2F): B volte DE:HL += 0x000186A0 (100000), con il
 * riporto della parola nella parola alta; poi 3D2F. Uscita del ciclo:
 * B = 0, DE:HL = valore a 32 bit. */
void sub_3d20(struct z80 *z)
{
    uint16_t hl = HL(z), de = DE(z);
    uint8_t b = z->b;
    do {
        uint32_t s = (uint32_t)hl + 0x86a0;    /* 3D21-3D24 add hl,de */
        de = (uint16_t)(de + 1 + (s > 0xffff)); /* 3D26-3D29 adc hl,de */
        hl = (uint16_t)s;
        b--;                                   /* 3D2D djnz */
    } while (b != 0);
    z->b = 0;
    SET_HL(z, hl);
    SET_DE(z, de);
    sub_3d2f(z);
}

/* 0x3D43: scompone il punteggio con sub_4d87 verso c061 (C = 0). */
void sub_3d43(struct z80 *z)
{
    SET_DE(z, 0xc061);                         /* 3D43 */
    z->c = 0;                                  /* 3D46 */
    sub_4d87(z);                               /* 3D48 */
}

/* 0x3DA6: (IX+0B) = L, (IX+0C) = H, (IX+12) = E, (IX+13) = D. */
void sub_3da6(struct z80 *z)
{
    M(z->ix + 0x0b) = z->l;
    M(z->ix + 0x0c) = z->h;
    M(z->ix + 0x12) = z->e;
    M(z->ix + 0x13) = z->d;
}

/* 0x3DEA: prepara l'oggetto c88e del quadro bonus: (c87b) = 08,
 * (c87c) = 00, (IX+0E) = (c86c), (IX+15) = (c873) - 0x10, poi
 * sub_4af6 con C = 1, E = a1, A = c5. IX resta c88e (non ripristinato). */
void sub_3dea(struct z80 *z)
{
    SET_HL(z, 0xc87b);                         /* 3DEA */
    SET_BC(z, 0x0800);                         /* 3DED */
    M(0xc87b) = 0x08;                          /* 3DF0 */
    M(0xc87c) = 0x00;                          /* 3DF2 */
    SET_HL(z, 0xc87c);
    z->ix = 0xc88e;                            /* 3DF3 */
    z->a = M(0xc86c);                          /* 3DF7 */
    M(z->ix + 0x0e) = z->a;                    /* 3DFA */
    z->a = (uint8_t)(M(0xc873) - 0x10);        /* 3DFD-3E00 */
    M(z->ix + 0x15) = z->a;                    /* 3E02 */
    z->c = 1;                                  /* 3E05 inc c */
    z->e = 0xa1;                               /* 3E06 */
    z->a = 0xc5;                               /* 3E08 */
    sub_4af6(z);                               /* 3E0A */
}

/* 0x3FCE: somma a 24 bit di C volte i 3 byte ROM a DE in c1aa-c1ac, poi
 * DE += 3. Con Z acceso in ingresso (bit 6 di F: la cifra era zero) salta
 * la somma e avanza solo DE. Uscita: DE + 3, Z acceso (il ciclo finisce
 * con `dec c` a zero); con la somma B = 0, C = 0 e A = byte alto
 * dell'ultima somma. */
void sub_3fce(struct z80 *z)
{
    if (!(z->f & 0x40)) {
        uint16_t de = DE(z);
        uint8_t c = z->c, a = z->a;
        do {
            unsigned cy = 0;                   /* 3FD7 or a */
            for (int i = 0; i < 3; i++) {      /* 3FD8-3FDD */
                unsigned s = leggi(de + i) + M(0xc1aa + i) + cy;
                a = (uint8_t)s;
                M(0xc1aa + i) = a;
                cy = s > 0xff;
            }
            z->f = (uint8_t)((z->f & ~1) | cy);
            c--;                               /* 3FE0 */
        } while (c != 0);
        z->a = a;
        z->b = 0;
        z->c = 0;
    }
    SET_DE(z, DE(z) + 3);                      /* 3FE4-3FE6 */
    z->f |= 0x40;
}

/* 0x3FB2: somma al punteggio c1aa-c1ac le 6 cifre BCD di c095, c094 e
 * c093 (dall'ultimo byte al primo; per ogni cifra non nulla `rld`),
 * ognuna pesata dalla tabella ROM 3FE8 (3 byte per cifra, via
 * sub_3fce). Azzera le cifre lette. Uscita: HL = c092, DE = 3FE8 + 18,
 * B = 0, C invariato, A dall'ultima cifra. */
void sub_3fb2(struct z80 *z)
{
    uint8_t c_in = z->c;
    SET_HL(z, 0xc095);                         /* 3FB4 */
    SET_DE(z, 0x3fe8);                         /* 3FB7 */
    for (int giro = 0; giro < 3; giro++) {     /* 3FB2 ld b,3 ... 3FCB djnz */
        for (int meta = 0; meta < 2; meta++) { /* 3FBB-3FC6 */
            uint8_t m = M(HL(z));
            z->a = (uint8_t)(m >> 4);          /* xor a; rld */
            M(HL(z)) = (uint8_t)(m << 4);
            z->f = (uint8_t)((z->f & ~0x40) | (z->a == 0 ? 0x40 : 0));
            z->c = z->a;                       /* 3FBE */
            sub_3fce(z);
        }
        SET_HL(z, HL(z) - 1);                  /* 3FC9 */
    }
    z->b = 0;
    z->c = c_in;                               /* pop bc */
}

/* 0x3ADF (3A82, 3AC8): riga di 32 coppie (tile, attributo) per il tabellone.
 * Ingresso: HL, E. Per 32 volte scrive C in (HL) (C parte da 4 e dopo 7
 * torna a 4), E nel piano degli attributi (HL con il bit 2 di H spento) e
 * avanza HL. Uscita: B = 0, C = 4, HL += 32. */
void sub_3adf(struct z80 *z)
{
    uint16_t hl = HL(z);
    uint8_t c = 0x04;
    for (int i = 0; i < 0x20; i++) {
        M(hl) = c;
        M(hl & 0xfbff) = z->e;
        hl++;
        c++;
        if (c & 0x08)
            c = 0x04;
    }
    z->b = 0;
    z->c = c;
    SET_HL(z, hl);
}
