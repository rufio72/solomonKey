/* Motore dei nemici, lotto N3 (brief 29): dipendenze del tipo 09 che il
 * censimento assegna al lotto N6 (tipo 06, parte B): 52FD (contatore del
 * passo con inversione della direzione), 5345 (maschere delle celle
 * attorno all'oggetto IY, per lo slot IX) e 54C5 (numero a caso).
 * Lotto N6 (brief 32): le altre otto routine del tipo 06 (5274, 5289,
 * 5292, 52AA, 52C2, 52DB, 52EC, destinazioni di 51FA, e 531C, che prova due
 * celle e, se una è piena, chiude con la coda 5068). 531C: riporto 0 in
 * z->f = uscita normale; riporto 1 = uscita anomala (nell'originale
 * `pop hl; jp 5068` ritorna al chiamante del chiamante): il chiamante deve
 * ritornare subito.
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

/* 0x5274: (IY+1D) = (IY+1E) = 0, bit 0 di (IX+1) e bit 2 di (IX+0) spenti,
 * bit 7 di (IX+1) acceso. Destinazioni 1, 3, 5 di 5207. */
void sub_5274(struct z80 *z)
{
    IYB(0x1d) = 0x00;                          /* 5274 */
    IYB(0x1e) = 0x00;                          /* 5278 */
    RES(IXB(0x01), 0x01);                      /* 527C */
    RES(IXB(0x00), 0x04);                      /* 5280 */
    SET(IXB(0x01), 0x80);                      /* 5284 */
}

/* 0x5289: bit 1 di (IY+1E) acceso, bit 0 di (IX+1) spento. Senza cattura
 * propria, coperta dalle forzate di 51FA. */
void sub_5289(struct z80 *z)
{
    SET(IYB(0x1e), 0x02);                      /* 5289 */
    RES(IXB(0x01), 0x01);                      /* 528D */
}

/* Parte comune di 5292 e 52AA: C = 1, con il bit 0 di (IX+1) acceso e
 * (IX+10) <= 6 C = 0 (A = 6); (IY+1E) = C; bit 0 di (IX+1) acceso. */
static void quota_1(struct z80 *z)
{
    int acceso = IXB(0x01) & 0x01;
    z->c = 0x01;
    if (acceso) {
        z->a = 0x06;
        if (z->a >= IXB(0x10))                 /* cp (ix+10); jr c */
            z->c--;
    }
    IYB(0x1e) = z->c;
    SET(IXB(0x01), 0x01);
}

/* 0x5292: vedi quota_1 (destinazione 7 di 5207). */
void sub_5292(struct z80 *z) { quota_1(z); }

/* 0x52AA: identica a 5292 (destinazione 9). Senza cattura propria. */
void sub_52aa(struct z80 *z) { quota_1(z); }

/* 0x52C2: C = 1, 0 con il bit 0 di (IX+1) acceso; (IY+1D) = 0,
 * (IY+1E) = C; bit 0 di (IX+1) acceso, bit 2 di (IX+0) spento. Senza
 * cattura propria. */
void sub_52c2(struct z80 *z)
{
    int acceso = IXB(0x01) & 0x01;
    z->c = acceso ? 0x00 : 0x01;               /* 52C2-52CA */
    IYB(0x1d) = 0x00;                          /* 52CB */
    IYB(0x1e) = z->c;                          /* 52CF */
    SET(IXB(0x01), 0x01);                      /* 52D2 */
    RES(IXB(0x00), 0x04);                      /* 52D6 */
}

/* 0x52DB: (IY+1D) = 0, (IY+1E) = 1, bit 0 di (IX+1) e bit 2 di (IX+0)
 * spenti. Destinazioni 8, 10, 12; senza cattura propria. */
void sub_52db(struct z80 *z)
{
    IYB(0x1d) = 0x00;                          /* 52DB */
    IYB(0x1e) = 0x01;                          /* 52DF */
    RES(IXB(0x01), 0x01);                      /* 52E3 */
    RES(IXB(0x00), 0x04);                      /* 52E7 */
}

/* 0x52EC: (IY+1D) = 0, bit 1 di (IY+1E) acceso, bit 0 di (IX+1) e bit 2
 * di (IX+0) spenti. Senza cattura propria. */
void sub_52ec(struct z80 *z)
{
    IYB(0x1d) = 0x00;                          /* 52EC */
    SET(IYB(0x1e), 0x02);                      /* 52F0 */
    RES(IXB(0x01), 0x01);                      /* 52F4 */
    RES(IXB(0x00), 0x04);                      /* 52F8 */
}

/* 0x531C: H = (IY+0E) + 8, L = (IY+15) + 5; due celle di c0bd (L e L + 6,
 * per sub_36f0). Entrambe libere: riporto 0, HL e A come 36F0. Una piena:
 * l'originale scarta l'indirizzo di ritorno e salta a 5068 con B = (IY+1A),
 * quindi ritorna al chiamante del chiamante: qui si esegue sub_5068 e si
 * esce con riporto 1. HL in quel caso non è riprodotto (nel gioco è
 * l'indirizzo di ritorno scartato, 50CB o 50E2). Ramo non catturabile dal
 * banco (snap.lua non vede il ritorno), scoperto da gcov. */
void sub_531c(struct z80 *z)
{
    z->h = (uint8_t)(IYB(0x0e) + 0x08);        /* 531C-5321 */
    z->l = (uint8_t)(IYB(0x15) + 0x05);        /* 5322-5327 */
    uint8_t l0 = z->l;
    sub_36f0(z);                               /* 5329 */
    int piena = BIT7(HL(z));                   /* 532C */
    z->h = (uint8_t)(IYB(0x0e) + 0x08);        /* 532E pop hl */
    z->l = l0;
    if (!piena) {
        z->l = (uint8_t)(0x06 + z->l);         /* 5331-5334 */
        sub_36f0(z);                           /* 5335 */
        piena = BIT7(HL(z));                   /* 5338 */
    }
    if (!piena) {
        NEMICI_PONI_RIPORTO(z, 0);             /* 533C or a */
        return;
    }
    z->b = IYB(0x1a);                          /* 533F */
    sub_5068(z);                               /* 5342 jp 5068 */
    NEMICI_PONI_RIPORTO(z, 1);                 /* uscita anomala */
}
