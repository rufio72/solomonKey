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
