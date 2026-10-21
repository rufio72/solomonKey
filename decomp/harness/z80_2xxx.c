/* Routine del Z80 principale fra 0x2000 e 0x2fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* 0x237E: aggiorna un piccolo record a HL.
 * (HL) diventa ((HL) + 1) & 0x7f; la parola big-endian in (HL+1),(HL+2)
 * (byte alto prima) diminuisce di 0x0150.
 * Uscita: A = byte alto nuovo, BC = 0150, DE = parola nuova, HL = HL+1. */
void sub_237e(struct z80 *z)
{
    uint16_t hl = HL(z);
    M(hl) = (uint8_t)((M(hl) + 1) & 0x7f);
    uint16_t parola = (uint16_t)((M(hl + 1) << 8) | M(hl + 2));
    parola -= 0x0150;
    M(hl + 2) = parola & 0xff;
    M(hl + 1) = parola >> 8;
    z->a = parola >> 8;
    SET_BC(z, 0x0150);
    SET_DE(z, parola);
    SET_HL(z, hl + 1);
}

/* 0x23A1: somma a 16 bit della parola in DE alla parola in HL (byte basso
 * prima). Uscita: A = byte alto della somma, DE e HL avanzati di 2. */
void sub_23a1(struct z80 *z)
{
    uint16_t hl = HL(z), de = DE(z);
    uint16_t somma = (uint16_t)(leggi(hl) | (leggi(hl + 1) << 8)) +
                     (uint16_t)(leggi(de) | (leggi(de + 1) << 8));
    M(hl) = somma & 0xff;
    M(hl + 1) = somma >> 8;
    z->a = somma >> 8;
    SET_HL(z, hl + 2);
    SET_DE(z, de + 2);
}

/* 0x2418: indirizzo di una cella in una griglia di 32 colonne.
 * Ingresso: D riga, E colonna. Uscita: HL = 32·D + E, D = 0; A invariato. */
void sub_2418(struct z80 *z)
{
    SET_HL(z, 32 * z->d + z->e);
    z->d = 0;
}

/* 0x249E: spostamento di un record a HL.
 * (HL) += C; parola a HL+3 (byte basso prima) += D:C; (HL+5) += C.
 * Uscita: A = nuovo (HL+5), HL = HL + 5 + DE. */
void sub_249e(struct z80 *z)
{
    uint16_t hl = HL(z);
    M(hl) += z->c;
    uint16_t parola = (uint16_t)(M(hl + 3) | (M(hl + 4) << 8));
    parola += (uint16_t)((z->d << 8) | z->c);
    M(hl + 3) = parola & 0xff;
    M(hl + 4) = parola >> 8;
    z->a = M(hl + 5) = (uint8_t)(M(hl + 5) + z->c);
    SET_HL(z, hl + 5 + DE(z));
}

/* 0x24B1: riempie tre byte di un record a HL.
 * Il riporto in ingresso entra nel bit 7 di A (rra): serve davvero, quindi
 * si legge dal bit 0 di F.
 * (HL+1) = E, (HL+2) = A ruotato, (HL+3) = C.
 * Uscita: HL = HL+3, E = B, C = B = C+1, A = 0. */
void sub_24b1(struct z80 *z)
{
    uint16_t hl = HL(z);
    uint8_t a = (uint8_t)((z->a >> 1) | ((z->f & 1) << 7));
    M(hl + 1) = z->e;
    M(hl + 2) = a;
    M(hl + 3) = z->c;
    SET_HL(z, hl + 3);
    z->e = z->b;
    z->c++;
    z->b = z->c;
    z->a = 0;
}

/* 0x2368: valore di una tabella di byte moltiplicato per 12.
 * Ingresso: HL base, A indice. Uscita: HL = 12·(HL+A), DE = 2·(HL+A)
 * (byte letto anche in ROM); A invariato. */
void sub_2368(struct z80 *z)
{
    uint16_t valore = leggi((uint16_t)(HL(z) + z->a));
    SET_DE(z, 2 * valore);
    SET_HL(z, 12 * valore);
}

/* 0x23AC: azzera il byte a DE e quello a D:(E+0x20) (stessa pagina, riga
 * sotto in una griglia di 32 colonne).
 * Uscita: A = 0, HL = D:(E+0x20), DE = DE+1. */
void sub_23ac(struct z80 *z)
{
    uint16_t de = DE(z);
    M(de) = 0;
    z->l = (uint8_t)(z->e + 0x20);
    z->h = z->d;
    M(HL(z)) = 0;
    z->a = 0;
    SET_DE(z, de + 1);
}

/* 0x23F8: record di 3 byte in una tabella (rst 08 più un terzo di passo).
 * Ingresso: HL base, A indice. Il record sta a HL + 3·A: DE = parola,
 * C = terzo byte. Uscita: B = 0, A = 0, HL sul terzo byte. */
void sub_23f8(struct z80 *z)
{
    uint16_t hl = (uint16_t)(HL(z) + 3 * z->a);
    z->e = leggi(hl);
    z->d = leggi(hl + 1);
    z->c = leggi(hl + 2);
    SET_HL(z, hl + 2);
    z->b = 0;
    z->a = 0;
}

/* 0x240B: HL = 48·A (il codice fa 2·A + A, poi quattro raddoppi).
 * Uscita: BC = A, A invariato. */
void sub_240b(struct z80 *z)
{
    SET_BC(z, z->a);
    SET_HL(z, 48 * z->a);
}

/* 0x2457: separa i due nibble di E.
 * Uscita: D = (E >> 4)·2 + 2, E = A = (E & 0x0f)·2 - 1 (0xff se nibble 0). */
void sub_2457(struct z80 *z)
{
    uint8_t e = z->e;
    z->d = (uint8_t)(((e & 0xf0) >> 3) + 2);
    z->e = z->a = (uint8_t)(((e & 0x0f) << 1) - 1);
}

/* Tabelle ROM di 0x2B08: destinazioni video (indice B) e stringhe (indice C). */
#define TAB_DEST_2B29  0x2b29
#define TAB_TESTI_2B61 0x2b61

/* 0x2B08 (test RAM e titolo): copia la stringa numero C (tabella 2B61,
 * terminata da 0xff) nel piano alto della destinazione numero B (tabella
 * 2B29), attributo 0x40 nel piano basso; con il bit 7 di B acceso la
 * destinazione è HL del chiamante. Uscita: A = 0, DE sul terminatore,
 * HL dopo l'ultimo carattere scritto; B e C invariati. */
void sub_2b08(struct z80 *z)
{
    uint8_t b = z->b, c = z->c;
    if (!(b & 0x80)) {
        z->a = b;
        SET_HL(z, TAB_DEST_2B29);
        sub_0008(z);           /* rst 08 */
        sub_0018(z);           /* rst 18: HL = parola della tabella */
    }
    uint16_t dest = HL(z);
    z->a = c;
    SET_HL(z, TAB_TESTI_2B61);
    sub_0008(z);               /* rst 08 */
    sub_0010(z);               /* rst 10: DE = inizio della stringa */
    uint16_t src = DE(z);
    for (;;) {
        uint8_t ch = leggi(src);
        if (ch == 0xff)
            break;
        M(dest) = ch;
        M(dest & ~0x0400) = 0x40;
        dest = (uint16_t)((dest | 0x0400) + 1);
        src++;
    }
    z->a = 0;                  /* inc a sul terminatore */
    z->b = b; z->c = c;
    SET_DE(z, src);
    SET_HL(z, dest);
}
