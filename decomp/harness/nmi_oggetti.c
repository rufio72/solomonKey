/* Lo stato di Dana dentro la NMI: collisioni (027C), spegnimento (03FB),
 * cancello (042F, con la callee comune 0469) e macchina degli stati
 * dell'animazione (04AC). Come sempre si riproducono RAM e registri
 * A, BC, DE, HL, IX, IY all'uscita (le bandiere F no). */
#include <stdio.h>
#include "routine.h"

/* Slot oggetto di Dana e sue celle di stato. */
#define DANA        0xc82e   /* slot oggetto (0x30 byte) */
#define DANA_FLAG   0xc82f   /* +1: bit 0 suono passi, bit 1 morte in scala, bit 2 anti-ripetizione */
#define DANA_PASSI  0xc830   /* +2: contapassi dell'animazione */
#define DANA_VELX   0xc83a   /* +c: velocità orizzontale (segnata) */
#define DANA_Y      0xc83c   /* +e: pixel y */
#define DANA_X      0xc843   /* +15: pixel x */
#define DANA_POSA   0xc84b   /* +1d: stato (7 = scala) */
#define DANA_DIR    0xc84c   /* +1e: direzione */
/* Campi di uno slot oggetto (brief 11). */
#define OGG_STATO   0x00
#define OGG_REC1    0x03
#define OGG_REC2    0x04
#define OGG_Y       0x0e
#define OGG_X       0x15
#define OGG_POSA    0x1d
#define OGG_DIR     0x1e
/* Margini del riquadro: record di 8 byte a 9495 + 8·codice; le collisioni
 * usano i campi +4..+7 (seconda metà del record). */
#define MARGINI_ROM 0x9495

/* 0x0469 (da 03FB e 042F): spegne l'oggetto IX: bit 5 spento, velocità
 * (+0b/+0c) salvate in +2a/+2b, posa = C e direzione = B, X spostata di
 * +3 o −3 secondo il bit 0 di B, c463 = 0. Uscita: A = 0, B, C, HL, DE
 * intatti. */
void sub_0469(struct z80 *z)
{
    uint16_t o = z->ix;
    M(o + OGG_STATO) &= (uint8_t)~0x20;
    M(o + 0x2a) = M(o + 0x0b);
    M(o + 0x2b) = M(o + 0x0c);
    M(o + OGG_POSA) = z->c;
    M(o + OGG_DIR) = z->b;
    uint8_t passo = (z->b & 0x01) ? 0xfd : 0x03;
    M(o + OGG_X) = (uint8_t)(M(o + OGG_X) + passo);
    z->a = 0;
    M(0xc463) = 0;
}

/* 0x03FB (dalla NMI, 00F0, quando il comando chiede lo spegnimento e
 * c46e & c1 = 0): se Dana è attivo salva posa e direzione in +29/+2c
 * (con il bit 1 della direzione spento, o acceso se la posa era 4:
 * 041C, mai eseguito nelle tracce), chiama sub_0469 (posa C, direzione B),
 * scrive 41 in c46e e posta l'attività 01. Uscita: IX = c82e; nel ramo
 * pieno i registri di sub_0493 (A = 1, DE = c000+slot, HL = c04e), C = 7. */
void sub_03fb(struct z80 *z)
{
    z->ix = DANA;
    z->a = M(DANA + OGG_STATO);
    if (z->a < 0xc0)
        return;                            /* ret c: Dana spento */
    M(DANA + 0x29) = M(DANA + OGG_POSA);
    z->c = 0x07;
    z->b = M(DANA + OGG_DIR);
    M(DANA + 0x2c) = z->b;
    z->b &= (uint8_t)~0x02;
    if (M(DANA + OGG_POSA) == 4)
        z->b |= 0x02;                      /* 041C: mai eseguito nelle tracce */
    M(DANA + OGG_DIR) = z->b;
    sub_0469(z);
    M(0xc46e) = 0x41;
    z->a = 0x01;
    sub_0493(z);                           /* posta l'attività 01 */
}

/* 0x042F (dalla NMI, 00FE, quando il comando chiede il cancello e
 * c46e & c1 = 0): come 03FB ma per l'ingresso nel cancello: esce se Dana
 * è spento o ha il bit 1 acceso, o se c299 ha già il bit 7; altrimenti
 * c299 = c0, azzera c29a-c29c, sub_0469 (posa 7), 43 in c46e e posta 0E.
 * Uscita: IX = c82e; nel ramo pieno i registri di sub_0493 (A = 0E). */
void sub_042f(struct z80 *z)
{
    z->ix = DANA;
    z->a = M(DANA + OGG_STATO);
    if (z->a < 0xc0)
        return;                            /* ret c */
    if (z->a & 0x02)
        return;                            /* ret nz */
    M(DANA + 0x29) = M(DANA + OGG_POSA);
    SET_HL(z, 0xc299);
    if (M(0xc299) & 0x80)
        return;                            /* ret nz: cancello già in corso */
    M(0xc299) = 0xc0;
    M(0xc29a) = 0;
    M(0xc29b) = 0;
    M(0xc29c) = 0;
    SET_HL(z, 0xc29c);
    z->c = 0x07;
    z->b = M(DANA + OGG_DIR);
    M(DANA + 0x2c) = z->b;
    z->b &= (uint8_t)~0x02;
    sub_0469(z);
    M(0xc46e) = 0x43;
    z->a = 0x0e;
    sub_0493(z);                           /* posta l'attività 0E */
}

/* 0x027C (dalla NMI, 010E): collisioni di Dana con i 17 slot da c4fe.
 * Con il bit 7 di c4ce acceso (mai nelle tracce) un primo giro marca gli
 * oggetti sovrapposti al riquadro c4dc/c4e3 e posta 12. Poi, se Dana è
 * attivo e non in stato 2 (altrimenti res 0 su c46e, mai eseguito),
 * confronta il riquadro di Dana (c83c+3, c843 corretta di ±3 sulla scala)
 * con ogni slot attivo di stato 0 (margini dalla seconda metà del record
 * 9495): il contatto apre la finestra di morte 03AA-03E0 (mai eseguita).
 * Il ramo eseguito non scrive la RAM. Uscita del giro completo: A =
 * ultimo confronto, B = 0, C = margine (0c, o 08 sulla scala), DE =
 * riquadro di Dana, HL = 0030, IX = c82e. */
void sub_027c(struct z80 *z)
{
    uint8_t a = M(0xc4ce);
    if (a & 0x80) {
        /* primo giro 0283-0308: mai eseguito nelle tracce (dal dasm statico) */
        int urto = 0;
        uint16_t ix = 0xc4fe;
        for (uint8_t b = 0x11; b != 0; b--, ix += 0x30) {
            uint8_t stato = M(ix + OGG_STATO);
            if (stato < 0xc0 || (stato & 0x06))
                continue;
            uint8_t rec = M(ix + OGG_REC2);
            if (rec >= 9)
                rec = M(ix + OGG_REC1);
            uint16_t m = (uint16_t)(MARGINI_ROM + 8 * rec + 4);
            uint8_t bordo = (uint8_t)(leggi(m) + M(ix + OGG_Y));
            uint8_t dist = (uint8_t)(M(0xc4dc) + 3 - bordo);
            uint8_t lato;
            if (M(0xc4dc) + 3 >= bordo) {
                lato = (uint8_t)(leggi(m + 3) + 1);
            } else {
                dist = (uint8_t)-dist;
                lato = 8;
            }
            if (lato < dist)
                continue;
            bordo = (uint8_t)(leggi(m + 1) + M(ix + OGG_X));
            dist = (uint8_t)(M(0xc4e3) + 4 - bordo);
            if (M(0xc4e3) + 4 >= bordo) {
                lato = (uint8_t)(leggi(m + 2) + 1);
            } else {
                dist = (uint8_t)-dist;
                lato = 8;
            }
            if (lato < dist)
                continue;
            M(ix + OGG_STATO) = (uint8_t)((M(ix + OGG_STATO) & ~0x40) | 0x02);
            urto = 1;
            M(0xc4ce) = 0x82;
        }
        if (urto) {
            z->a = 0x12;
            sub_0493(z);                   /* posta l'attività 12 */
        }
    }
    z->a = M(DANA + OGG_STATO);
    if (z->a < 0xc0)
        return;                            /* ret c: Dana spento */
    if (z->a & 0x02) {
        /* 0316-031B: mai eseguito nelle tracce */
        M(0xc46e) &= (uint8_t)~0x01;
        SET_HL(z, 0xc46e);
        return;
    }
    uint8_t e = 0;
    uint8_t posa = M(DANA_POSA);
    if (posa == 7) {                       /* sulla scala: X corretta di ±3 */
        e = 3;
        if (!(M(DANA_DIR) & 0x01))
            e = 0xfd;
    }
    uint8_t d = (uint8_t)(M(DANA_Y) + 3);
    e = (uint8_t)(M(DANA_X) + e);
    uint8_t margine = (posa != 4) ? 0x0c : 0x08;
    uint16_t ix = 0xc4fe;
    uint8_t a_fine = z->a;
    for (uint8_t b = 0x11; b != 0; b--, ix += 0x30) {
        a_fine = M(ix + OGG_STATO);
        if (a_fine < 0x80)
            continue;
        a_fine &= 0x03;
        if (a_fine != 0)
            continue;
        /* push bc: il margine fa da semiriquadro di Dana */
        uint8_t rec = M(ix + OGG_REC2);
        if (rec >= 9)
            rec = M(ix + OGG_REC1);
        uint16_t m = (uint16_t)(MARGINI_ROM + 8 * rec + 4);
        uint8_t bordo = (uint8_t)(leggi(m) + M(ix + OGG_Y));
        uint8_t dist, lato;
        if (d >= bordo) {
            dist = (uint8_t)(d - bordo);
            lato = (uint8_t)(leggi(m + 3) + 1);
        } else {
            dist = (uint8_t)(bordo - d);
            lato = margine;
        }
        a_fine = (uint8_t)(lato - dist);
        if (lato < dist)
            continue;                      /* niente contatto in y */
        bordo = (uint8_t)(leggi(m + 1) + M(ix + OGG_X));
        uint8_t x5 = (uint8_t)(5 + e);
        if (x5 >= bordo) {
            dist = (uint8_t)(x5 - bordo);
            lato = (uint8_t)(leggi(m + 2) + 1);
        } else {
            dist = (uint8_t)(bordo - x5);
            lato = 6;
        }
        a_fine = (uint8_t)(lato - dist);
        if (lato < dist)
            continue;                      /* niente contatto in x */
        /* contatto: finestra di morte 03AA-03E0, mai eseguita nelle tracce */
        a_fine = (uint8_t)(M(ix + OGG_REC2) - 0x10);
        if (a_fine >= 0x90)
            M(DANA_FLAG) |= 0x02;          /* morte sulla scala */
        z->ix = ix;
        sub_05b3(z);                       /* tabella sprite aggiornata */
        z->c = 0x0a;
        sub_0028(z);                       /* rst 28: suono della morte */
        M(DANA + OGG_STATO) = 0xa2;
        M(0xc467) = 0;
        M(0xc08d) = 0;
        M(0xc19d) = 0;
        /* res 6 sui 18 slot da c4ce; il passo somma DE intero, con D non
         * azzerato (probabile svista dell'originale): fuori da c000-ffff
         * la scrittura si ignora come sulla ROM */
        uint16_t p = 0xc4ce;
        uint16_t passo = (uint16_t)((d << 8) | 0x30);
        for (uint8_t i = 0x12; i != 0; i--) {
            if (p >= 0xc000)
                M(p) &= (uint8_t)~0x40;
            p = (uint16_t)(p + passo);
        }
        z->a = 0x13;
        z->d = d;
        z->e = e;
        sub_0493(z);                       /* posta l'attività 13 */
        return;
    }
    z->a = a_fine;
    z->b = 0;
    z->c = margine;
    z->d = d;
    z->e = e;
    SET_HL(z, 0x0030);
    z->ix = ix;                            /* c82e dopo i 17 passi */
}

/* 0x04AC (dalla NMI, 0111): macchina degli stati dell'animazione di Dana
 * (IX = c82e; esce se spento). Il segno della velocità c83a comanda il
 * suono dei passi 0C (bit 2 di c82f come anti-ripetizione); il contapassi
 * c830 cresce a ogni frame. Smistamento sulla posa (ix+1d): 0 = fermo
 * (dopo 5 passi suona 03 e rielabora il comando), 1 = passi a terra o in
 * scala (bit 1 della direzione), 2 = caduta (dopo 7 passi (ix+7) = ff),
 * 3-6 = rielaborazione del comando c053 (nuova posa in C e direzione in
 * A, con il blocco magico bit 2); ≥ 7 = solo riscrittura. In coda scrive
 * posa (ix+1d) = C e direzione (ix+1e) = A. Uscita: registri secondo il
 * ramo (commenti nel corpo). */
void sub_04ac(struct z80 *z)
{
    z->ix = DANA;
    z->a = M(DANA + OGG_STATO);
    if (z->a < 0xc0)
        return;                            /* ret c: Dana spento */

    uint16_t hl = DANA_FLAG;
    uint8_t a = M(DANA_VELX);
    uint8_t segno = (uint8_t)(a >> 7);     /* add a,a: riporto = bit 7 */
    a = (uint8_t)(a << 1);
    if (!segno || a == 0xfe) {
        M(DANA_FLAG) &= (uint8_t)~0x04;
    } else if (!(M(DANA_FLAG) & 0x04)) {
        M(DANA_FLAG) |= 0x04;
        z->c = 0x0c;
        sub_0028(z);                       /* rst 28: suono del passo */
    }
    hl = DANA_PASSI;
    M(DANA_PASSI)++;

    uint8_t c = M(DANA + OGG_POSA);
    z->a = c;
    z->c = c;
    uint8_t e = z->e;

    if (c >= 7)
        goto coda_1e;                      /* 0516: posa ≥ 7, solo riscrittura */
    if (c >= 3)
        goto comando;                      /* 0556 */
    if (c == 0) {                          /* posa 0: fermo */
        a = M(DANA_PASSI);
        z->a = a;
        if (a < 5) {
            SET_HL(z, hl);
            return;                        /* ret: ancora fermo */
        }
        z->c = 0x03;
        sub_0028(z);                       /* rst 28: suono 03 */
        c = 0x03;
        goto comando;
    }
    if (c == 2) {                          /* posa 2: caduta */
        a = M(DANA_PASSI);
        z->a = a;
        if (a < 7) {
            SET_HL(z, hl);
            return;                        /* ret c */
        }
        M(DANA + 0x07) = 0xff;
        c = 0x06;
        goto comando_055c;                 /* flags nc dal cp 7 */
    }
    /* posa 1: passi */
    z->b = M(DANA + OGG_DIR);
    if (z->b & 0x02)
        goto scala;                        /* 0532: bit 1 = in scala */
    a = M(DANA_PASSI);
    if (a >= 9) {                          /* 051C: ciclo dei passi chiuso */
        M(DANA_FLAG) &= (uint8_t)~0x01;
        c = 0x06;
        goto comando;
    }
    hl = DANA_FLAG;
    if (M(DANA_FLAG) & 0x01)
        goto coda_1e;                      /* passo già suonato */
    M(DANA_FLAG) |= 0x01;
    z->c = 0x07;
    sub_0028(z);                           /* rst 28: suono dei passi */
    c = 0x01;
    z->a = 0x10;
    sub_0493(z);                           /* posta l'attività 10 */
    hl = 0xc04e;                           /* HL lasciato da sub_0493 */
coda_1e:                                   /* 0516 */
    a = M(DANA + OGG_DIR);
    goto coda;

scala:                                     /* 0532: passi sulla scala */
    a = M(DANA_PASSI);
    if (a >= 9) {
        hl = DANA_FLAG;                    /* 0550: dec hl */
        goto spegni_passo;
    }
    hl = DANA_FLAG;
    if (!(M(DANA_FLAG) & 0x01)) {
        M(DANA_FLAG) |= 0x01;
        z->c = 0x06;
        sub_0028(z);                       /* rst 28: suono della scala */
        c = 0x01;
    }
    a = (uint8_t)(M(0xc053) & 0x0f);       /* comando direzionale */
    if (a != 0)
        goto spegni_passo;
    z->a = a = 0x02;
    goto blocco;                           /* 05A3 con HL = c82f */
spegni_passo:                              /* 0551 */
    M(DANA_FLAG) &= (uint8_t)~0x01;
    hl = DANA_PASSI;
    c = 0x05;

comando:                                   /* 0556: E = posa − 4 */
    e = (uint8_t)(c - 4);
comando_055c:                              /* 055C: comando corrente in B */
    a = M(0xc053);
    z->b = a;
    if ((uint8_t)(c - 4) >= 2)
        goto blocco_giu;                   /* jp nc (posa 3 o 6; o 2→6) */
    e = (uint8_t)((e >> 1) | (e << 7));    /* rrc e */
    if (e == 0) {                          /* posa 4 */
        if (a & 0x08)
            goto posa4;
        goto posa5;
    }
    a &= 0x0f;                             /* posa 5 */
    if (a == 0x08)
        goto posa4;
posa5:                                     /* 056F */
    c = 0x05;
    goto salto;
posa4:                                     /* 0579 */
    c = 0x04;
salto:                                     /* 057B: bit 2 del comando = salto */
    if (!(a & 0x04))
        goto sblocca;
    if (M(DANA + OGG_STATO) & 0x01)
        goto direzione;
    c = 0x00;
    M(DANA + OGG_STATO) |= 0x01;           /* blocco del salto */
    M(hl) = 0;                             /* azzera il contapassi */
    goto direzione;
blocco_giu:                                /* 0592 */
    if (a & 0x04)
        goto direzione;
sblocca:                                   /* 0597 */
    M(DANA + OGG_STATO) &= (uint8_t)~0x01;
direzione:                                 /* 059B: bit 0-1 del comando */
    a &= 0x03;
    a = (uint8_t)(a - 1);
    if (a & 0x80)
        a = 0x02;                          /* nessuna direzione: resta 2 */
blocco:                                    /* 05A3 */
    if (a == 0x02)
        a = (uint8_t)(M(DANA + OGG_DIR) | 0x02);
coda:                                      /* 05AC */
    M(DANA + OGG_POSA) = c;
    M(DANA + OGG_DIR) = a;
    z->a = a;
    z->c = c;
    z->e = e;
    SET_HL(z, hl);
}
