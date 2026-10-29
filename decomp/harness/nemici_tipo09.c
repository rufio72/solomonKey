/* Motore dei nemici, tipo 09 (ingresso 57CD, oggetto che raccoglie e porta
 * blocchi). Lotto N1 (brief 25): parti del tipo 09 usate dal
 * tipo 12 (5693): la cella davanti all'oggetto (5928), la presa del blocco
 * da parte dell'oggetto portato (58CF) e la sua chiusura (5917).
 * Ingressi: 58CF, 5917, 5928; dal lotto N3, anticipate dal lotto N2
 * (brief 27) per il tipo 14: 6EB7, 6EC1.
 *
 * IX = slot (c2c1 + i·0x18), IY = oggetto (c4fe + i·0x30); (IX+0E) è
 * l'indice dell'oggetto portato nella tabella ROM 48AF. Tradotto da
 * out/main64.dasm. */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"

#define IXB(o) M(z->ix + (o))
#define IYB(o) M(z->iy + (o))

#define PORTATI_48AF   0x48af   /* ROM: indirizzo dell'oggetto per indice */
#define POSA_C08A      0xc08a
#define OGGETTI_488D   0x488d   /* ROM: indirizzo per indice (IX+0E) */

/* IX = indirizzo dell'oggetto portato (tabella 48AF all'indice (IX+0E)),
 * come `ld a,(ix+0e); ld hl,48af; rst 8; rst 10h; push de; ex (sp),ix`.
 * Restituisce l'IX di prima (quello che resta in pila). */
static uint16_t prendi_portato(struct z80 *z)
{
    z->a = M(z->ix + 0x0e);
    SET_HL(z, PORTATI_48AF);
    sub_0008(z);                               /* rst 8 */
    sub_0010(z);                               /* rst 10 */
    uint16_t ix0 = z->ix;
    z->ix = DE(z);                             /* push de; ex (sp),ix: DE invariato */
    return ix0;
}

/* ------------------------------------------------------------------ */
/* 0x5928: cella davanti all'oggetto IY secondo la direzione (IY+1E):
 * Y = (IY+0E) + 8 (f8 col bit 1), X = (IY+15) + 1a (f6 col bit 0), poi
 * sub_36f0. B = bit 1 e bit 0 della direzione; HL = cella, E = indice. */
void sub_5928(struct z80 *z)
{
    NEMICI_PUNTO(0x5928, 0x594e);              /* NMI del replay: 5928 non scrive */
    uint8_t dir = M(z->iy + 0x1e);             /* 5928 */
    z->a = 0x08;                               /* 592B */
    z->b = 0x00;                               /* 592F */
    if (dir & 0x02) {                          /* 592D, 5931 */
        z->a = 0xf8;                           /* 5933 */
        z->b |= 0x02;                          /* 5935 */
    }
    z->h = (uint8_t)(z->a + M(z->iy + 0x0e));  /* 5937-593A */
    z->a = 0x1a;                               /* 593F */
    if (M(z->iy + 0x1e) & 0x01) {              /* 593B, 5941 */
        z->a = 0xf6;                           /* 5943 */
        z->b |= 0x01;                          /* 5945 */
    }
    z->l = (uint8_t)(z->a + M(z->iy + 0x15));  /* 5947-594A */
    sub_36f0(z);                               /* 594B */
}

/* 0x58CF: l'oggetto IY porta l'oggetto della tabella 48AF sul blocco
 * davanti. Bit 6 di (IY+0) acceso, bit 5 spento, (IX+0D) = 0, bit 3 di
 * (IX+1). Se la cella davanti vale meno di 8 esce col riporto acceso.
 * Altrimenti c08a = 0 e l'oggetto portato si posa: sub_4938 per le celle
 * >= 80, sub_49ae (bit 0 di (IY+1) acceso durante la chiamata) per le
 * altre; c835 = ff, riporto spento. */
void sub_58cf(struct z80 *z)
{
    M(z->iy) |= 0x40;                          /* 58CF */
    M(z->iy) &= ~0x20;                         /* 58D3 */
    M(z->ix + 0x0d) = 0x00;                    /* 58D7 */
    M(z->ix + 0x01) |= 0x08;                   /* 58DB */
    NEMICI_PUNTO(0x58df, 0x58ea);              /* NMI del replay */
    uint16_t ix0 = prendi_portato(z);          /* 58DF-58E8 */
    sub_5928(z);                               /* 58EA */
    z->a = M(HL(z));                           /* 58ED */
    if (z->a < 0x08) {                         /* 58EE-58F0 */
        z->ix = ix0;                           /* 58F2 pop ix */
        NEMICI_PONI_RIPORTO(z, 1);
        return;
    }
    SET_HL(z, POSA_C08A);                      /* 58F7 */
    M(POSA_C08A) = 0x00;                       /* 58FA */
    if (z->a >= 0x80) {                        /* 58F5, 58FC */
        sub_4938(z);                           /* 590B */
    } else {
        M(z->iy + 0x01) |= 0x01;               /* 58FE */
        sub_49ae(z);                           /* 5902 */
        M(z->iy + 0x01) &= ~0x01;              /* 5905 */
    }
    z->a = 0xff;                               /* 590E */
    M(0xc835) = z->a;                          /* 5910 */
    z->ix = ix0;                               /* 5913 pop ix */
    NEMICI_PONI_RIPORTO(z, 0);                 /* 5915 or a */
}

/* 0x5917: chiude l'oggetto portato (tabella 48AF all'indice (IX+0E)) con
 * sub_4b98. */
void sub_5917(struct z80 *z)
{
    uint16_t ix0 = prendi_portato(z);          /* 5917-5920 */
    sub_4b98(z->ix);                           /* 5922 */
    z->ix = ix0;                               /* 5925 pop ix */
}

/* ------------------------------------------------------------------ */
/* Lotto N3, anticipate dal lotto N2 (brief 27). */

/* Scrittura a un indirizzo del Z80: sotto c000 è ROM e non cambia. */
static void scrivi_ram(uint16_t a, uint8_t v)
{
    if (a >= 0xc000)
        M(a) = v;
}

/* 0x6EB7: azzera i due byte puntati dalla tabella 488D all'indice A,
 * poi cade in 6EC1. */
void sub_6eb7(struct z80 *z)
{
    SET_HL(z, OGGETTI_488D);                   /* 6EB7 */
    z->c = 0x00;                               /* 6EBA */
    sub_0008(z);                               /* 6EBC rst 8 */
    sub_0018(z);                               /* 6EBD rst 18 */
    scrivi_ram(HL(z), z->c);                   /* 6EBE */
    SET_HL(z, (uint16_t)(HL(z) + 1));          /* 6EBF */
    scrivi_ram(HL(z), z->c);                   /* 6EC0 */
    sub_6ec1(z);
}

/* 0x6EC1: azzera il byte puntato dalla tabella 48AF all'indice A. */
void sub_6ec1(struct z80 *z)
{
    z->c = 0x00;                               /* 6EC1 */
    SET_HL(z, PORTATI_48AF);                   /* 6EC3 */
    sub_0008(z);                               /* 6EC6 rst 8 */
    sub_0018(z);                               /* 6EC7 rst 18 */
    scrivi_ram(HL(z), z->c);                   /* 6EC8 */
}

/* ------------------------------------------------------------------ */
/* Lotto N3 (brief 29): gestore del tipo 09. 57CD smista con `rst 38h`
 * (tabella 57D3) in base alla fase (IY+1D); 5995 e le sue code stanno in
 * nemici_tipo09b.c. (IX+0D) è il contatore di fotogrammi della fase,
 * (IY+1E) la direzione (bit 0 destra, bit 1 su/giù), (IX+1) i flag del
 * portato (bit 3 = porta un blocco), (IX+0E) l'indice del portato. */

/* 0x5894 (ingresso interno di 580A e 582B): lascia il blocco, fase 6,
 * azzera i due byte dell'oggetto portato (6EB7). */
static void sub_5894(struct z80 *z)
{
    IXB(0x01) &= ~0x08;                        /* 5894 */
    IYB(0x1d) = 0x06;                          /* 5898 */
    z->a = IXB(0x0e);                          /* 589C */
    sub_6eb7(z);                               /* 589F */
}

/* 0x57E4: dopo 0x20 fotogrammi inverte la direzione (bit 0), accende il
 * bit 0 di (IX+1) e passa alla fase 5. */
void sub_57e4(struct z80 *z)
{
    z->a = IXB(0x0d);                          /* 57E4 */
    if (z->a < 0x20)                           /* 57E7 cp 20; ret c */
        return;
    z->a = (uint8_t)(0x01 ^ IYB(0x1e));        /* 57EA-57EC */
    IYB(0x1e) = z->a;                          /* 57EF */
    IXB(0x01) |= 0x01;                         /* 57F2 */
    IYB(0x1d) = 0x05;                          /* 57F6 */
}

/* 0x57FB: dopo 4 fotogrammi spegne il bit 3 di (IY+1) e passa alla fase 3. */
void sub_57fb(struct z80 *z)
{
    z->a = IXB(0x0d);                          /* 57FB */
    if (z->a < 0x04)                           /* 57FE cp 4; ret c */
        return;
    IYB(0x01) &= ~0x08;                        /* 5801 */
    IYB(0x1d) = 0x03;                          /* 5805 */
}

/* 0x580A: la prima volta accende il bit 1 di (IY+1E) e riparte il
 * contatore; poi, a 8 fotogrammi, chiude con 5894 se porta un blocco,
 * altrimenti passa alla fase 6. */
void sub_580a(struct z80 *z)
{
    if (!(IYB(0x1e) & 0x02)) {                 /* 580A */
        IYB(0x1e) |= 0x02;                     /* 5810 */
        IXB(0x0d) = 0x00;                      /* 5814 */
        return;
    }
    z->a = IXB(0x0d);                          /* 5819 */
    if (z->a < 0x08)                           /* 581C cp 8; ret c */
        return;
    if (IXB(0x01) & 0x08) {                    /* 581F */
        sub_5894(z);                           /* 5823 jp 5894 */
        return;
    }
    IYB(0x1d) = 0x06;                          /* 5826 */
}

/* 0x582B: presa del blocco. Con il bit 2 di (IY+1) spento: se il bit 1 di
 * (IY+1E) è acceso lo accende in (IY+1) e riparte; altrimenti prende il
 * portato (indice già in (IX+1) bit 1, oppure uno slot libero con 7379) e
 * chiama 4592 sul suo oggetto. Con il bit 2 acceso: a 8 fotogrammi passa
 * alla fase 5, spegne il bit, inverte il bit 1 della direzione e, se ora
 * è spento, chiude con 5894. */
void sub_582b(struct z80 *z)
{
    if (!(IYB(0x01) & 0x04)) {                 /* 582B */
        if (IYB(0x1e) & 0x02) {                /* 5831 */
            IYB(0x01) |= 0x04;                 /* 5837 */
            IXB(0x01) &= ~0x20;                /* 583B */
            IXB(0x0d) = 0x00;                  /* 583F */
            return;
        }
        if (IXB(0x01) & 0x02) {                /* 5844 */
            z->a = IXB(0x0e);                  /* 584A */
            IXB(0x01) &= ~0x02;                /* 584D */
            SET_HL(z, 0xc2ba);                 /* 5851 */
            M(HL(z)) &= ~0x01;                 /* 5854 */
        } else {
            sub_7379(z);                       /* 5858 */
            if (!NEMICI_RIPORTO_ACCESO(z))     /* 585B ret nc */
                return;
            IXB(0x0e) = z->a;                  /* 585C */
            M(HL(z)) = 0x80;                   /* 585F */
        }
        IYB(0x01) |= 0x04;                     /* 5861 */
        IXB(0x01) |= 0x08;                     /* 5865 */
        IXB(0x0d) = 0x00;                      /* 5869 */
        uint16_t ix0 = prendi_portato(z);      /* 586D-5873 */
        sub_4592(z);                           /* 5875 */
        z->ix = ix0;                           /* 5878 pop ix */
        return;
    }
    z->a = IXB(0x0d);                          /* 587B */
    if (z->a < 0x08)                           /* 587E cp 8; ret c */
        return;
    IYB(0x1d) = 0x05;                          /* 5881 */
    IYB(0x01) &= ~0x04;                        /* 5885 */
    z->a = (uint8_t)(0x02 ^ IYB(0x1e));        /* 5889-588B */
    IYB(0x1e) = z->a;                          /* 588E */
    if (z->a & 0x02)                           /* 5891 bit 1,a; ret z */
        sub_5894(z);                           /* cade in 5894 */
}

/* 0x58A3: se non porta un blocco lo prende (58CF; riporto acceso = cella
 * davanti vuota, esce); se lo porta, a 0x16 fotogrammi lo posa (5917).
 * In fondo (58B8): lascia il blocco, accende il bit 5 di (IY+0) e il bit 1
 * della direzione, fase 6, e azzera il portato (6EB7). */
void sub_58a3(struct z80 *z)
{
    if (!(IXB(0x01) & 0x08)) {                 /* 58A3 */
        sub_58cf(z);                           /* 58A9 */
        if (!NEMICI_RIPORTO_ACCESO(z))         /* 58AC jr c */
            return;
    } else {
        z->a = IXB(0x0d);                      /* 58AF */
        if (z->a < 0x16)                       /* 58B2 cp 16; ret c */
            return;
        sub_5917(z);                           /* 58B5 */
    }
    IXB(0x01) &= ~0x08;                        /* 58B8 */
    IYB(0x00) |= 0x20;                         /* 58BC */
    IYB(0x1e) |= 0x02;                         /* 58C0 */
    IYB(0x1d) = 0x06;                          /* 58C4 */
    z->a = IXB(0x0e);                          /* 58C8 */
    sub_6eb7(z);                               /* 58CB */
}

/* 0x594F: se (IY+0C) non è ff esce (A = (IY+0C) + 1). Altrimenti accende il
 * bit 5 di (IX+1) la prima volta (invertendo il bit 1 della direzione);
 * con il bit 1 spento esce. La prima volta (bit 4 spento) passa alla fase 7
 * e spegne il bit 1 della direzione; la seconda toglie i bit 4, 1 e 0
 * (and ed), azzera il bit 0 di c2ba e, se il bit 1 di (IY+1) è acceso, spegne
 * il bit 1 della direzione. */
void sub_594f(struct z80 *z)
{
    z->a = (uint8_t)(IYB(0x0c) + 1);           /* 594F-5952 */
    if (z->a != 0)                             /* 5953 ret nz */
        return;
    z->a = IXB(0x01);                          /* 5954 */
    if (!(z->a & 0x20)) {                      /* 5957 */
        z->a |= 0x20;                          /* 595B */
        z->c = z->a;                           /* 595D */
        z->a = (uint8_t)(0x02 ^ IYB(0x1e));    /* 595E-5960 */
        IYB(0x1e) = z->a;                      /* 5963 */
        z->a = z->c;                           /* 5966 */
    }
    IXB(0x01) = z->a;                          /* 5967 */
    if (!(z->a & 0x02))                        /* 596A bit 1,a; ret z */
        return;
    if (!(z->a & 0x10)) {                      /* 596D */
        z->a |= 0x10;                          /* 5971 */
        IXB(0x01) = z->a;                      /* 5973 */
        IYB(0x1d) = 0x07;                      /* 5976 */
        IYB(0x1e) &= ~0x02;                    /* 597A */
        return;
    }
    z->c = z->a;                               /* 597F */
    z->a = (uint8_t)(0xed & z->c);             /* 5980-5982 */
    IXB(0x01) = z->a;                          /* 5983 */
    SET_HL(z, 0xc2ba);                         /* 5986 */
    M(0xc2ba) &= ~0x01;                        /* 5989 */
    if (IYB(0x01) & 0x02)                      /* 598B */
        IYB(0x1e) &= ~0x02;                    /* 5990 */
}

/* 0x57CD: gestore del tipo 09. 52FD, poi salto per fase (IY+1D) con
 * `rst 38h` al sito 57D3 (8 voci). */
void sub_57cd(struct z80 *z)
{
    sub_52fd(z);                               /* 57CD */
    z->e = IYB(0x1d);                          /* 57D0 */
    uint16_t dest = nemici_rst38(z, 0x57d3, rst38_57D3,
                                 sizeof rst38_57D3 / sizeof rst38_57D3[0]);
    NEMICI_PONI_RIPORTO(z, 0);                 /* rst 38: add hl,de spegne il riporto */
    switch (dest) {
    case 0x57fb: sub_57fb(z); break;
    case 0x582b: sub_582b(z); break;
    case 0x580a: sub_580a(z); break;
    case 0x3b62: sub_3b62(z); break;
    case 0x57e4: sub_57e4(z); break;
    case 0x5995: sub_5995(z); break;
    case 0x594f: sub_594f(z); break;
    case 0x58a3: sub_58a3(z); break;
    default: break;
    }
}
