/* Routine del Z80 principale fra 0x7000 e 0x7fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* Due aree di destinazione scelte dal bit 0 di C. */
#define AREA_PARI    0xd486
#define AREA_DISPARI 0xd496

/* 0x7EFE: scrive una cella in due piani.
 * Ingresso: D riga, E colonna, C attributo. A = bit mescolati di E e D:
 * quattro volte rr l, rra, rr h, rra (riporto iniziale nullo) su HL = DE;
 * l'indirizzo è HL così ruotato + 0x20·D (D = 0 vale 256) + E + d486
 * (d496 se C è dispari). Si scrive A all'indirizzo e C allo stesso col bit
 * 2 di H spento.
 * Uscita: B = D, DE e C invariati, HL = indirizzo col bit 2 di H spento. */
void sub_7efe(struct z80 *z)
{
    uint8_t h = z->d, l = z->e, a = 0, cy = 0, nuovo;
    for (int i = 0; i < 4; i++) {
        nuovo = l & 1; l = (uint8_t)((l >> 1) | (cy << 7)); cy = nuovo;
        nuovo = a & 1; a = (uint8_t)((a >> 1) | (cy << 7)); cy = nuovo;
        nuovo = h & 1; h = (uint8_t)((h >> 1) | (cy << 7)); cy = nuovo;
        nuovo = a & 1; a = (uint8_t)((a >> 1) | (cy << 7)); cy = nuovo;
    }
    unsigned righe = z->d ? z->d : 256;
    uint16_t hl = (uint16_t)((h << 8) | l);
    hl = (uint16_t)(hl + 0x20 * righe);
    hl = (uint16_t)(hl + ((z->c & 1) ? AREA_DISPARI : AREA_PARI) + z->e);
    M(hl) = a;
    hl &= ~0x0400;
    M(hl) = z->c;
    z->a = a;
    z->b = z->d;
    SET_HL(z, hl);
}

/* Tabella ROM delle destinazioni a video delle stringhe d'esito. */
#define TAB_ESITI_7E3A 0x7e3a

/* 0x7E32: destinazione numero A dalla tabella di parole 7E3A.
 * Uscita: DE = parola, HL invariato (push/pop); A invariato. */
void sub_7e32(struct z80 *z)
{
    uint16_t hl = HL(z);
    SET_HL(z, TAB_ESITI_7E3A);
    sub_0008(z);               /* rst 08 */
    sub_0010(z);               /* rst 10: DE = parola */
    SET_HL(z, hl);
}

/* 0x7E1D: stampa la stringa d'esito: destinazione numero A dalla tabella
 * 7E3A (sub_7e32), stringa a HL in due piani (sub_6771). */
void sub_7e1d(struct z80 *z)
{
    sub_7e32(z);
    sub_6771(z);
}

/* 0x7E24: come 0x7E1D, poi salta un carattere e accoda la stringa fissa
 * a 7EA3 (sub_6771). */
void sub_7e24(struct z80 *z)
{
    sub_7e32(z);
    sub_6771(z);
    SET_DE(z, DE(z) + 1);
    SET_HL(z, 0x7ea3);
    sub_6771(z);
}

/* 0x7DEA: le tre stringhe dell'esito di fine partita, ognuna con
 * sub_7e1d: (A=3, 7E76), (A=4, 7E85), (A=5, 7E94). */
void sub_7dea(struct z80 *z)
{
    z->a = 3;
    SET_HL(z, 0x7e76);
    sub_7e1d(z);
    z->a = 4;
    SET_HL(z, 0x7e85);
    sub_7e1d(z);
    z->a = 5;
    SET_HL(z, 0x7e94);
    sub_7e1d(z);
}

/* 0x75AA (6655, 761D, 78E4, 7C4E, 7C98): azzeramenti di cambio quadro:
 * i due piani video d480-d77f e d080-d37f (0x300 byte), c299-c2a8
 * (0x10 byte), il bit 0 di c258, i 24 slot oggetto c46e-c8ed (0x480 byte)
 * e gli slot c2c1-c458 (0x198 byte).
 * Uscita: A = 0, BC = 0, HL = c459. */
void sub_75aa(struct z80 *z)
{
    uint16_t hl = 0xd480;
    for (int i = 0; i < 0x300; i++, hl++) {    /* due piani: res 2,h */
        M(hl) = 0;
        M(hl & ~0x0400) = 0;
    }
    for (int i = 0; i < 0x10; i++)
        M(0xc299 + i) = 0;
    M(0xc258) &= ~0x01;
    for (int i = 0; i < 0x480; i++)            /* 24 slot oggetto da 0x30 */
        M(0xc46e + i) = 0;
    for (int i = 0; i < 0x198; i++)
        M(0xc2c1 + i) = 0;
    z->a = 0;
    SET_BC(z, 0);
    SET_HL(z, 0xc2c1 + 0x198);
}

/* 0x797E (7970): le due cifre BCD di (HL) in ASCII a (DE) (unità) e
 * (DE+8) (decine, riga sotto). Il ramo per la cifra oltre '1' (scrive
 * 0x73 a DE+6 e usa il passo 2) non è mai eseguito nelle tracce: tradotto
 * dal disassembly statico. Uscita: A = cifra alta + 0x30, B = 0, C = passo
 * (8, 2 nel ramo mai visto), DE avanzato del passo, HL invariato. */
void sub_797e(struct z80 *z)
{
    uint16_t hl = HL(z), de = DE(z);
    uint16_t passo = 0x0008;
    z->a = (uint8_t)((leggi(hl) & 0x0f) + 0x30);
    M(de) = z->a;
    if (z->a >= 0x32) {            /* ramo mai eseguito nelle tracce */
        M(de + 6) = 0x73;
        passo = 0x0002;
    }
    de = (uint16_t)(de + passo);
    z->a = (uint8_t)(((leggi(hl) & 0xf0) >> 4) + 0x30);
    M(de) = z->a;
    SET_BC(z, passo);
    SET_DE(z, de);
}

/* 0x7C84 (761A, 7C4B, 7C9B): azzera le due righe del pannello ddaa-ddb7
 * e ddca-ddd7 (14 + 14 byte). Uscita: A = 0, B = 0, C = 0x0e, HL = ddd8. */
void sub_7c84(struct z80 *z)
{
    for (int i = 0; i < 0x0e; i++)
        M(0xddaa + i) = 0;
    for (int i = 0; i < 0x0e; i++)
        M(0xddca + i) = 0;
    z->a = 0;
    z->b = 0;
    z->c = 0x0e;
    SET_HL(z, 0xddca + 0x0e);
}

/* I 17 slot da 0x18 byte c2c1-c458; la ricerca parte dall'ultimo. */
#define ULTIMO_SLOT_C441 0xc441
#define PASSO_SLOT       0x18

/* 0x7379 (373F, 56AB; molti siti statici in zone mai eseguite): cerca uno
 * slot libero (bit 7 del byte 0 spento) fra i 17 slot c2c1 partendo
 * dall'ultimo (c441) all'indietro. Trovato (coda privata 2540): azzera il
 * byte +1, A = indice dello slot (0..16), HL = slot, riporto 1. Esauriti
 * (738A, mai eseguito nelle tracce): A = 1, B = 0, riporto 0. Il riporto
 * serve al chiamante (il banco non confronta F), emulato su z->f. */
void sub_7379(struct z80 *z)
{
    uint16_t hl = ULTIMO_SLOT_C441;
    for (uint8_t b = 0x11; b > 0; b--, hl -= PASSO_SLOT) {
        if (!(M(hl) & 0x80)) {                 /* coda 2540-2547 */
            M(hl + 1) = 0;
            z->a = (uint8_t)(b - 1);
            z->b = b;
            z->c = 0xe8;
            SET_HL(z, hl);
            z->f = (uint8_t)(z->f | 1);        /* scf */
            return;
        }
    }
    /* nessuno slot libero: mai eseguito nelle tracce */
    z->a = 1;
    z->b = 0;
    z->c = 0xe8;
    SET_HL(z, ULTIMO_SLOT_C441 - 0x11 * PASSO_SLOT);
    z->f = (uint8_t)(z->f & ~1);               /* or a */
}

/* 0x7ECA (790F, 7CDB): cornice del pannello di gioco: due file di 14
 * caratteri consecutivi (04-11 a ddaa e 12-1F a ddca, sub_7f2d) e i bordi
 * a celle con sub_7efe: attributo 2 su 11 righe × 16 colonne, attributo 3
 * su 11 righe × 5 colonne. */
void sub_7eca(struct z80 *z)
{
    SET_BC(z, 0x0e04);
    SET_HL(z, 0xddaa);
    sub_7f2d(z);
    z->b = 0x0e;
    SET_HL(z, 0xddca);
    sub_7f2d(z);
    z->c = 0x02;
    z->d = 0x0b;
    do {
        z->e = 0x0f;
        do {
            sub_7efe(z);
            z->e--;
        } while (!(z->e & 0x80));              /* dec e; bit 7,e */
        z->d--;
    } while (z->d != 0);
    z->c = 0x03;
    z->d = 0x0b;
    do {
        z->e = 0x04;
        do {
            sub_7efe(z);
            z->e--;
        } while (!(z->e & 0x80));
        z->d--;
    } while (z->d != 0);
}

/* 0x7F2D (7ED0, 7ED8, dentro 7ECA): scrive B caratteri consecutivi a
 * partire da C da HL in avanti, con l'attributo 60 sull'altro piano
 * (res 2,h). Uscita: C += B d'ingresso, B = 0, HL avanzato di B. */
void sub_7f2d(struct z80 *z)
{
    uint16_t hl = HL(z);
    uint8_t c = z->c;
    uint8_t b = z->b;
    do {
        M(hl) = c;
        M(hl & ~0x0400) = 0x60;
        c++;
        hl++;
    } while (--b);                             /* djnz */
    z->b = 0;
    z->c = c;
    SET_HL(z, hl);
}
