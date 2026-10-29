/* Motore dei nemici, lotto N3 (brief 29): dipendenze del tipo 09 che il
 * censimento assegna al lotto N6 (tipo 06, parte B): 52FD (contatore del
 * passo con inversione della direzione), 5345 (maschere delle celle
 * attorno all'oggetto IY, per lo slot IX) e 54C5 (numero a caso).
 * IX = slot (c2c1 + i·0x18), IY = oggetto (c4fe + i·0x30). Tradotto da
 * out/main64.dasm. */
#include "routine.h"
#include "nemici.h"

#define TAB_9495   0x9495   /* ROM: otto byte per tipo, maschere di base */
#define CELLE_C0BD 0xc0bd   /* RAM: classi delle celle intorno */

#define IXB(o) M(z->ix + (o))
#define IYB(o) M(z->iy + (o))
#define BIT7(hl) ((M(hl) & 0x80) != 0)

/* 0x52FD: confronta (IX+8) con il limite (IX+0A se il bit 5 di (IX+0) è
 * acceso, altrimenti IX+9); se lo raggiunge azzera (IX+8) e inverte il
 * bit 5 di (IX+0). Esce con A = (IX+8) se sotto il limite, altrimenti con
 * A = il nuovo (IX+0). */
void sub_52fd(struct z80 *z)
{
    z->a = IXB(0x08);                          /* 52FD */
    uint8_t lim = (IXB(0x00) & 0x20) ? IXB(0x0a) : IXB(0x09);  /* 5300-530E */
    if (z->a < lim)                            /* ret c */
        return;
    IXB(0x08) = 0x00;                          /* 530F-5310 */
    z->a = (uint8_t)(0x20 ^ IXB(0x00));        /* 5313-5315 */
    IXB(0x00) = z->a;                          /* 5318 */
}

/* 0x54C5: C = numero a caso minore di C (C = 0 non esce: nel gioco C >= 1).
 * Due `ld a,r` ravvicinati (nemici_r_somma) danno E; D scende da 7f
 * (srl) finché (D & E) < C. Uscita: A = C = D & E, D, E. */
void sub_54c5(struct z80 *z)
{
    uint8_t somma = nemici_r_somma();
    uint8_t e = (uint8_t)((somma >> 1) | (somma << 7));   /* rrca */
    z->e = e;                                  /* 54C5-54CC */
    z->d = 0xff;                               /* 54CD */
    for (;;) {
        z->d >>= 1;                            /* 54CF srl d */
        z->a = z->d & z->e;                    /* 54D1-54D2 */
        if (z->a < z->c)                       /* 54D3 cp c; jr nc */
            break;
        if (z->d == 0)                         /* con C = 0 l'originale non esce */
            break;
    }
    z->c = z->a;                               /* 54D6 */
}

/* ------------------------------------------------------------------ */
/* 0x5345, tre rami secondo le classi delle celle (c0bd + BC, bit 7 =
 * cella piena). Nei tre rami B e C partono da una costante e si
 * correggono bit per bit; HL scorre sulle celle e DE ne tiene l'ultimo
 * passo, come nell'originale. */

#define RES(r, m) ((r) &= (uint8_t)~(m))
#define SET(r, m) ((r) |= (uint8_t)(m))

/* 537B: D = E (le due colonne coincidono). */
static void ramo_537b(struct z80 *z, uint16_t hl)
{
    z->b = 0x0c; z->c = 0x4c;                  /* 537B */
    hl += 2;
    if (BIT7(hl)) { SET(z->c, 0x10); RES(z->c, 0x08); }          /* 5380 */
    hl--;
    if (BIT7(hl)) { RES(z->c, 0x04); SET(z->c, 0x01); }          /* 5389 */
    hl -= 2;
    if (BIT7(hl)) { RES(z->b, 0x04); SET(z->b, 0x01); }          /* 5393 */
    hl--;
    if (BIT7(hl)) { SET(z->b, 0x10); RES(z->b, 0x08); }          /* 539C */
    SET_DE(z, 0xfff4); hl = (uint16_t)(hl + 0xfff4);             /* 53A4 */
    if (BIT7(hl)) { RES(z->c, 0x10); RES(z->c, 0x08); }          /* 53A8 */
    hl--;
    if (BIT7(hl)) { SET(z->c, 0x02); RES(z->c, 0x01); RES(z->c, 0x04); }  /* 53B1 */
    hl -= 2;
    if (BIT7(hl)) { SET(z->b, 0x02); RES(z->b, 0x01); RES(z->b, 0x04); }  /* 53BD */
    hl--;
    if (BIT7(hl)) { RES(z->b, 0x10); RES(z->b, 0x08); }          /* 53C8 */
    SET_DE(z, 0xfff3); hl = (uint16_t)(hl + 0xfff3);             /* 53D0 */
    if (BIT7(hl)) { z->a = z->c & 0xe5; z->c = z->a; }           /* 53D4 */
    hl--;
    if (BIT7(hl)) {                                              /* 53DD */
        SET(z->b, 0x40);
        z->a = z->c & 0xc5; z->c = z->a;
        z->a = z->b & 0xc5; z->b = z->a;
    }
    hl--;
    if (BIT7(hl)) { z->a = z->b & 0xe5; z->b = z->a; }           /* 53EC */
    SET_HL(z, hl);
}

/* 53F5: D != E e la cella (HL) è libera. HL arriva già decrementato. */
static void ramo_53f5(struct z80 *z, uint16_t hl)
{
    z->b = 0x05; z->c = 0x4c;                  /* 53F5 */
    hl += 2;
    if (BIT7(hl)) { RES(z->c, 0x08); SET(z->c, 0x10); }          /* 53FA */
    hl--;
    if (BIT7(hl)) { RES(z->c, 0x04); SET(z->c, 0x01); }          /* 5403 */
    hl -= 2;
    if (BIT7(hl)) RES(z->b, 0x04);                               /* 540D */
    SET_DE(z, 0xfff3); hl = (uint16_t)(hl + 0xfff3);             /* 5413 */
    if (BIT7(hl)) { RES(z->c, 0x10); RES(z->c, 0x08); SET(z->c, 0x20); }  /* 5417 */
    hl--;
    if (BIT7(hl)) { RES(z->c, 0x01); RES(z->c, 0x04); SET(z->c, 0x02); }  /* 5422 */
    hl -= 2;
    if (BIT7(hl)) { RES(z->b, 0x04); SET(z->b, 0x02); }          /* 542E */
    SET_DE(z, 0xfff3); hl = (uint16_t)(hl + 0xfff3);             /* 5436 */
    if (BIT7(hl)) { z->a = z->c & 0xc7; z->c = z->a; }           /* 543A */
    hl--;
    if (BIT7(hl)) { z->a = z->c & 0xc5; z->c = z->a; SET(z->b, 0x40); }  /* 5443 */
    hl--;
    if (BIT7(hl)) { SET(z->b, 0x40); RES(z->b, 0x02); }          /* 544E */
    hl--;
    if (BIT7(hl)) RES(z->b, 0x02);                               /* 5457 */
    SET_HL(z, hl);
}

/* 545E: D != E e la cella (HL) è piena. */
static void ramo_545e(struct z80 *z, uint16_t hl)
{
    z->b = 0x0c; z->c = 0x45;                  /* 545E */
    hl -= 2;
    if (BIT7(hl)) { RES(z->b, 0x08); SET(z->b, 0x10); }          /* 5463 */
    hl++;
    if (BIT7(hl)) { RES(z->b, 0x04); SET(z->b, 0x01); }          /* 546C */
    hl += 2;
    if (BIT7(hl)) RES(z->c, 0x04);                               /* 5476 */
    SET_DE(z, 0xffed); hl = (uint16_t)(hl + 0xffed);             /* 547C */
    if (BIT7(hl)) { RES(z->b, 0x10); RES(z->b, 0x08); SET(z->b, 0x20); }  /* 5480 */
    hl++;
    if (BIT7(hl)) { RES(z->b, 0x01); RES(z->b, 0x04); SET(z->b, 0x02); }  /* 548B */
    hl += 2;
    if (BIT7(hl)) SET(z->c, 0x02);                               /* 5497 */
    SET_DE(z, 0xffed); hl = (uint16_t)(hl + 0xffed);             /* 549D */
    if (BIT7(hl)) { z->a = z->b & 0xc7; z->b = z->a; }           /* 54A1 */
    hl++;
    if (BIT7(hl)) { z->a = z->b & 0xc5; z->b = z->a; SET(z->b, 0x40); }  /* 54AA */
    hl++;
    if (BIT7(hl)) { SET(z->b, 0x40); RES(z->c, 0x02); }          /* 54B5 */
    hl++;
    if (BIT7(hl)) RES(z->c, 0x02);                               /* 54BE */
    SET_HL(z, hl);
}

/* 0x5345: B = ~((IX+2) + 8) & f0; A = (IY+4), se >= 9 (IY+3); la tabella
 * 9495 (8 byte per indice) dà HL = 9496 + 8·A; 24DF con A = (IX+3) ne
 * ricava C (indice della cella centrale in c0bd) e D, E (colonne). Poi uno
 * dei tre rami costruisce in B e C le maschere delle celle attorno. */
void sub_5345(struct z80 *z)
{
    z->a = (uint8_t)(~(uint8_t)(IXB(0x02) + 0x08) & 0xf0);   /* 5345-534B */
    z->b = z->a;                               /* 534D */
    z->a = IYB(0x04);                          /* 534E */
    if (z->a >= 0x09)                          /* 5351 */
        z->a = IYB(0x03);                      /* 5355 */
    SET_DE(z, TAB_9495);                       /* 535B */
    SET_HL(z, (uint16_t)(z->a * 8 + TAB_9495 + 1));   /* 5358-5362 */
    z->a = IXB(0x03);                          /* 5363 */
    sub_24df(z);                               /* 5366 */
    uint16_t hl = (uint16_t)(CELLE_C0BD + BC(z));   /* 5369-536C */
    z->a = z->e;                               /* 536D */
    if (z->e == z->d) {                        /* 536E-536F */
        ramo_537b(z, hl);
        return;
    }
    if (BIT7(hl))                              /* 5372 */
        ramo_545e(z, hl);
    else
        ramo_53f5(z, (uint16_t)(hl - 1));      /* 5377-5378 */
}
