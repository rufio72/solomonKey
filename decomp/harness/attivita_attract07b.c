/* Corpo 07 (2CCD), fasi 4 (358F) e 7 (3A7D): lotto H4b2 (brief 46).
 * Stesse regole di attivita_attract07a.c (punti [da, a], rami mai eseguiti
 * con nemici_manca). Ogni fase inizia con `pop hl` e termina con `ret`. */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_attract07.h"

/* Due cifre del BCD per la fase 4: `add a,a; daa` su A = e. Restituisce il
 * risultato; il riporto del daa va in *cy. */
static uint8_t raddoppia_bcd(uint8_t e, uint8_t *cy)
{
    uint8_t r = (uint8_t)(e << 1);
    int riporto = e >> 7;
    int mezzo = ((e & 0x0f) << 1) > 0x0f;
    uint8_t adj = 0;
    if (mezzo || (r & 0x0f) > 9)
        adj |= 0x06;
    if (riporto || r > 0x99) {
        adj |= 0x60;
        riporto = 1;
    }
    *cy = (uint8_t)riporto;
    return (uint8_t)(r + adj);
}

/* Fase 4, 358F: record della stanza c243 nella tabella di puntatori 8EA4
 * (rst 08 e rst 10). Il record, letto in avanti con BC: due indici di cella
 * (c247 e, per ciascuno, la cella di c0bd = 1 e 4, con le coordinate di
 * 4B0E in c45f-c462), una coppia (D, E) che aggiorna c248, c249 e la cella
 * E di c0bd se c248 cambia, il byte scelto dai bit 2-3 di DSW2 (e605) in
 * c08f, il tempo BCD in c090-c099, poi la lista (spostamento, valore) che
 * scrive in c0bd fino al terminatore FF. Un valore con i 6 bit bassi = 36
 * accoda il suono 23 (366C); con i 6 bit bassi >= 32, se il bit 0 di c24e
 * era già acceso (L = 0 a 363D), il valore scritto diventa (bit 6-7) + 29
 * (365F-3665). Questi rami (lotto 65) non hanno punti di sincronia. */
void fase_07_358f(void)
{
    struct z80 z = {0};
    z.a = M(0xc243);
    SET_HL(&z, 0x8ea4);
    NEMICI_PUNTO(0x358f, 0x359e);
    NEMICI_PUNTO(0x0008, 0x0014);              /* rst 08h, rst 10h */
    sub_0008(&z);                              /* 3595 */
    sub_0010(&z);                              /* 3596 */
    uint16_t bc = DE(&z);                      /* 3597-3598 */

    uint8_t v = leggi(bc++);                   /* 359C */
    M(0xc247) = v;                             /* 359E */
    NEMICI_PUNTO(0x359f, 0x35a5);
    M(0xc0bd + v) = 0x01;                      /* 35A5 */
    v = leggi(bc++);                           /* 35A7 */
    NEMICI_PUNTO(0x35a6, 0x35b0);
    M(0xc0bd + v) = 0x04;                      /* 35B0 */
    z.a = v;
    sub_4b0e(&z);                              /* 35B2 */
    NEMICI_PUNTO(0x35b1, 0x35b9);
    M(0xc45f) = z.l;                           /* 35B9 */
    NEMICI_PUNTO(0x35ba, 0x35bb);
    M(0xc460) = z.h;                           /* 35BB */
    v = leggi(bc++);                           /* 35BD */
    NEMICI_PUNTO(0x35bc, 0x35c6);
    M(0xc0bd + v) = 0x04;                      /* 35C6 */
    z.a = v;
    sub_4b0e(&z);                              /* 35C8 */
    NEMICI_PUNTO(0x35c7, 0x35cf);
    M(0xc461) = z.l;                           /* 35CF */
    NEMICI_PUNTO(0x35d0, 0x35d1);
    M(0xc462) = z.h;                           /* 35D1 */

    uint8_t d = leggi(bc++);                   /* 35D2-35D4 */
    uint8_t e = leggi(bc++);                   /* 35D9-35DA */
    if (M(0xc248) != d) {                      /* 35D5-35DB */
        NEMICI_PUNTO(0x35d2, 0x35de);
        M(0xc248) = e;                         /* 35DE */
        NEMICI_PUNTO(0x35df, 0x35e2);
        M(0xc249) = d;                         /* 35E2 */
        NEMICI_PUNTO(0x35e3, 0x35f5);
        M(0xc0bd + e) = (uint8_t)((M(0xc0bd + e) & 0x80) + M(0xc24a) + 5);   /* 35F5 */
    }
    uint8_t idx = (uint8_t)((leggi_porta(0xe605) >> 2) & 0x03);   /* 35F6-35FB */
    uint8_t a = leggi((uint16_t)(bc + idx));   /* 3600-3601 */
    bc = (uint16_t)(bc + 4);                   /* 3602-3607 */
    NEMICI_PUNTO(0x35f6, 0x360b);
    M(0xc08f) = a;                             /* 360B */
    NEMICI_PUNTO(0x360c, 0x360e);
    M(0xc08d) = 0x00;                          /* 360E */
    NEMICI_PUNTO(0x360f, 0x3611);
    M(0xc090) = 0x00;                          /* 3611 */
    NEMICI_PUNTO(0x3612, 0x3613);
    M(0xc091) = 0x00;                          /* 3613 */
    NEMICI_PUNTO(0x3614, 0x3615);
    M(0xc092) = 0x01;                          /* 3615 */
    NEMICI_PUNTO(0x3616, 0x3618);
    M(0xc093) = 0x00;                          /* 3618 */
    NEMICI_PUNTO(0x3619, 0x361a);
    M(0xc094) = 0x00;                          /* 361A */
    e = leggi(bc++);                           /* 361C-361E */
    NEMICI_PUNTO(0x361b, 0x361f);
    M(0xc094) = (uint8_t)((e & 0x0f) << 4);    /* 361F rrd (hl) */
    NEMICI_PUNTO(0x3620, 0x3622);
    M(0xc095) = 0x00;                          /* 3622 */
    NEMICI_PUNTO(0x3623, 0x3627);
    M(0xc095) = (uint8_t)(e >> 4);             /* 3627 rld (hl) */
    NEMICI_PUNTO(0x3628, 0x362a);
    M(0xc096) = 0x00;                          /* 362A */
    uint8_t cy;
    uint8_t bcd = raddoppia_bcd(e, &cy);       /* 362C-362E */
    NEMICI_PUNTO(0x362b, 0x362f);
    M(0xc097) = bcd;                           /* 362F */
    NEMICI_PUNTO(0x3630, 0x3633);
    M(0xc098) = cy;                            /* 3633 rla */
    NEMICI_PUNTO(0x3634, 0x3635);
    M(0xc099) = cy;                            /* 3635 */

    uint8_t l = (M(0xc24e) & 0x01) ? 0x00 : 0x01;   /* 3639-3641: L, usato a 365F */
    NEMICI_PUNTO(0x3636, 0x363b);
    M(0xc24e) |= 0x01;                         /* 363B */
    for (int it = 0;; it++) {                  /* 3642-3668 */
        if (it == 0)
            NEMICI_PUNTO(0x363c, 0x3667);
        else
            NEMICI_PUNTO(0x3642, 0x3668);
        uint8_t s = leggi(bc++);               /* 3645 */
        if (s == 0xff)
            break;                             /* 3647-3649 */
        uint16_t dest = (uint16_t)(0xc0bd + s);    /* 364B-364F */
        uint8_t h = leggi(bc++);               /* 3650 */
        uint8_t m = h & 0x3f;
        if (m == 0x36) {                       /* 3655-3657 call z,366C */
            z.a = 0x23;                        /* 366F (push/pop af, hl, de) */
            sub_0493(&z);                      /* 3671 */
        }
        uint8_t w = h;                         /* 365C ld a,h */
        if (m >= 0x32 && !l)                   /* 365A-3661 */
            w = (uint8_t)((h & 0xc0) + 0x29);  /* 3663-3665 */
        M(dest) = w;                           /* 3667 */
    }
    NEMICI_PUNTO(0x366a, 0x366b);
}

/* Fase 7, 3A7D: scrive il tabellone del titolo. 3ADF a d460 e a dc60
 * (E = 51); 24 iterazioni a dc80 di due coppie (tile, attributo) che
 * avanzano di 1E (tile da 5108 che torna a 08 al bit 4); 32 coppie a d780
 * (tile da 50DC che torna a DC al bit 5); azzera c467; tempo (3F10) e cifre
 * a d472 (4D87 con C = 71). Il piano degli attributi è l'indirizzo con il
 * bit 2 di H spento. */
void fase_07_3a7d(void)
{
    struct z80 z = {0};
    SET_HL(&z, 0xd460);
    z.e = 0x51;
    NEMICI_PUNTO(0x3a7d, 0x3a82);
    sub_3adf(&z);                              /* 3A82 */
    PUNTO_LARGO(0x3adf, 0x3af2);               /* ritorni dentro 3ADF */
    uint16_t hl = 0xdc80;
    uint8_t e = 0x08, dd = 0x51;               /* de = 5108 */
    for (int i = 0; i < 0x18; i++) {           /* 3A8E-3AA9 */
        if (i == 0)
            NEMICI_PUNTO(0x3a85, 0x3a8e);
        else {
            NEMICI_PUNTO(0x3a9f, 0x3aa9);
            NEMICI_PUNTO(0x3a8e, 0x3a8e);
        }
        M(hl) = e;                             /* 3A8E */
        NEMICI_PUNTO(0x3a8f, 0x3a91);
        M(hl & 0xfbff) = dd;                   /* 3A91 */
        hl++;
        e++;
        hl = (uint16_t)(hl + 0x1e);            /* 3A96-3A9A */
        NEMICI_PUNTO(0x3a92, 0x3a9b);
        M(hl) = e;                             /* 3A9B */
        NEMICI_PUNTO(0x3a9c, 0x3a9e);
        M(hl & 0xfbff) = dd;                   /* 3A9E */
        hl++;
        e++;
        if (e & 0x10)
            e = 0x08;                          /* 3AA7 */
    }
    hl = 0xd780;
    e = 0xdc;
    dd = 0x50;                                 /* de = 50DC */
    for (int i = 0; i < 0x20; i++) {           /* 3AB3-3AC1 */
        if (i == 0)
            NEMICI_PUNTO(0x3a9f, 0x3ab3);
        else {
            NEMICI_PUNTO(0x3ab7, 0x3ac1);
            NEMICI_PUNTO(0x3ab3, 0x3ab3);
        }
        M(hl) = e;                             /* 3AB3 */
        NEMICI_PUNTO(0x3ab4, 0x3ab6);
        M(hl & 0xfbff) = dd;                   /* 3AB6 */
        hl++;
        e++;
        if (e & 0x20)
            e = 0xdc;                          /* 3ABF */
    }
    NEMICI_PUNTO(0x3ab7, 0x3ac8);
    SET_HL(&z, 0xdc60);
    z.e = 0x51;
    sub_3adf(&z);                              /* 3AC8 */
    PUNTO_LARGO(0x3adf, 0x3af2);
    NEMICI_PUNTO(0x3acb, 0x3acc);
    M(0xc467) = 0x00;                          /* 3ACC */
    NEMICI_PUNTO(0x3acd, 0x3acf);
    sub_3f10(&z);                              /* 3ACF */
    NEMICI_PUNTO(0x3ad2, 0x3ada);
    SET_HL(&z, 0xd472);
    SET_DE(&z, 0xc061);
    z.c = 0x71;
    sub_4d87(&z);                              /* 3ADA */
    NEMICI_PUNTO(0x3add, 0x3ade);
}
