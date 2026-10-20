/* Routine del Z80 principale fra 0x3000 e 0x3fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* Tabella in ROM di 33 valori (indici 0..0x20), letta a specchio. */
#define TABELLA_3111 0x3111

/* 0x30EF: valore di una tabella simmetrica su 64 passi (forma d'onda).
 * Ingresso: A angolo. Indice = A & 0x3f, ribaltato (^0x3f) se il bit 5 è
 * acceso. Uscita: HL = 3111 + indice, A = valore letto. */
void sub_30ef(struct z80 *z)
{
    uint8_t indice = z->a & 0x3f;
    if (indice & 0x20)
        indice ^= 0x3f;
    uint16_t hl = TABELLA_3111 + indice;
    SET_HL(z, hl);
    z->a = leggi(hl);
}

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

/* 0x34D4: azzera 10 byte da HL in due piani: per ogni byte scrive 0 a HL
 * con il bit 2 di H spento e poi a HL con il bit 2 di H acceso (prima
 * scrittura all'indirizzo d'ingresso, come nel codice).
 * Uscita: B = 0, C = 0, HL avanzato di 10 col bit 2 di H acceso. */
void sub_34d4(struct z80 *z)
{
    uint16_t hl = HL(z);
    for (int i = 0; i < 10; i++) {
        M(hl) = 0;
        hl &= ~0x0400;
        M(hl) = 0;
        hl |= 0x0400;
        hl++;
    }
    SET_BC(z, 0);
    SET_HL(z, hl);
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
