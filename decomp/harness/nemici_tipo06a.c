/* Motore dei nemici, lotto N3 (brief 29): due dipendenze del tipo 09 che il
 * censimento assegna al lotto N10 (tipo 06, parte A): 51D7, che sceglie a
 * caso uno dei bit accesi di una maschera, e 24DF, un piccolo calcolo su
 * nibble con una coppia di byte della tabella puntata da HL.
 * Lotto N6 (brief 32): le cinque routine del tipo 06 che il censimento
 * assegna a N10 e che le 8 routine di nemici_tipo06b.c richiedono: 5068
 * (coda condivisa), 51FA (smista con `rst 38` a 5207 sul bit acceso scelto
 * da 51D7), 5224, 523C e 5254 (tre destinazioni della tabella).
 * Lotto N8 (brief 34): 4FEF, che 5CFB e 5FAC richiedono.
 * Lotto N10 (brief 36): 4FD4, 5005, 5023, 5035, 5066, 5092, 50BA, 50DF,
 * 5115, 5124, 513A, 513E, 514C, 516B, 5186, 51A4, 51B1, 51B5, 51D3; tipo 6 =
 * 4FD4.
 * Tradotto da out/main64.dasm (out/nemici_n6.dasm). */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"

#define TAB_C06A 0xc06a   /* RAM: posizioni dei bit accesi della maschera */
#define C2B6     0xc2b6   /* RAM: scritta a ff da 5023, 5092 e 51B5 */

#define IXB(o) M(z->ix + (o))
#define IYB(o) M(z->iy + (o))

/* Scambio dei nibble: quattro `rrca`. */
static uint8_t scambia(uint8_t x)
{
    return (uint8_t)((x >> 4) | (x << 4));
}

/* 0x24DF: A, (HL) e (HL+1) sono tre indici a nibble. E = (8 + A + (HL)),
 * D = nibble alto di E scambiato (& 0f), poi (HL+1) + E scambiato di nuovo
 * dà E (& 0f). C = E + B, A = B = 0, HL avanzato di uno. La tabella può
 * stare in ROM: si legge con leggi(). */
void sub_24df(struct z80 *z)
{
    uint16_t hl = HL(z);
    uint8_t e = (uint8_t)(0x08 + z->a + leggi(hl));       /* 24DF-24E7 */
    hl++;                                                  /* 24E8 */
    z->d = scambia(e) & 0x0f;                              /* 24E9-24F7 */
    uint8_t a = scambia((uint8_t)(leggi(hl) + e)) & 0x0f;  /* 24F8-2507 */
    z->e = a;                                              /* 2508 */
    z->c = (uint8_t)(a + z->b);                            /* 2509 */
    z->a = 0x00;                                           /* 250B */
    z->b = 0x00;                                           /* 250C */
    SET_HL(z, hl);
}

/* 0x51D7: BC = maschera a 14 bit. Da c06a si scrivono le posizioni dei bit
 * accesi (b - 1, con b da 14 a 1) e C ne conta il numero; 54C5 sceglie a
 * caso un indice C minore del numero. Uscita: A = 0d - (c06a + C), B = 0,
 * C = E = indice, D = 0, HL = c06a + indice. */
void sub_51d7(struct z80 *z)
{
    uint16_t de = BC(z);                       /* 51D7-51D8 */
    uint16_t hl = TAB_C06A;                    /* 51DC */
    z->c = 0x00;                               /* 51D9 ld bc,0e00 */
    for (z->b = 0x0e; z->b; z->b--) {          /* 51DF-51EA */
        int bit = de & 1;                      /* rr d; rr e: il riporto d'ingresso */
        de >>= 1;                              /* non arriva a un bit esaminato */
        if (bit) {
            M(hl) = (uint8_t)(z->b - 1);       /* 51E5-51E6 */
            z->c++;                            /* 51E8 */
            hl++;                              /* 51E9 */
        }
    }
    sub_54c5(z);                               /* 51EC */
    z->e = z->c;                               /* 51EF */
    z->d = 0x00;                               /* 51F0 */
    hl = (uint16_t)(TAB_C06A + z->e);          /* 51F2-51F5 */
    SET_HL(z, hl);
    z->a = (uint8_t)(0x0d - M(hl));            /* 51F6-51F8 */
}

/* 0x5068: coda condivisa (531C, 5092, 51B5): (IY+0) con i bit 5-6 portati a
 * 01, (IY+4) += 3, (IY+3) = 0d, (IY+1E) = 2, (IY+1A) = B, (IX+0E) = ff,
 * bit 6 di (IY+0) acceso, bit 0 di (IX+1) spento. Esce con A = (IY+4). */
void sub_5068(struct z80 *z)
{
    M(z->iy) = (uint8_t)((M(z->iy) & 0x9f) | 0x10);       /* 5068-506F */
    z->a = (uint8_t)(0x03 + M(z->iy + 0x04));             /* 5072-5074 */
    M(z->iy + 0x04) = z->a;                               /* 5077 */
    M(z->iy + 0x03) = 0x0d;                               /* 507A */
    M(z->iy + 0x1e) = 0x02;                               /* 507E */
    M(z->iy + 0x1a) = z->b;                               /* 5082 */
    M(z->ix + 0x0e) = 0xff;                               /* 5085 */
    M(z->iy) |= 0x40;                                     /* 5089 */
    M(z->ix + 0x01) &= (uint8_t)~0x01;                    /* 508D */
}

/* Parte comune di 5224, 523C e 5254: con il bit 0 di (IX+1) acceso, A = 6
 * e C = 1 se (IX+10) > 6; altrimenti C resta com'è (in 5224 e 5254 è 0). */
static void confronta_6(struct z80 *z)
{
    z->a = 0x06;
    if (z->a < M(z->ix + 0x10))                           /* cp (ix+10); jr nc */
        z->c++;
}

/* 0x5224: C = 0; con il bit 0 di (IX+1) acceso C = 1 se (IX+10) > 6;
 * (IY+1E) = C; bit 0 di (IX+1) acceso. Destinazione 0 di 5207. */
void sub_5224(struct z80 *z)
{
    z->c = 0x00;                                          /* 5228 */
    if (M(z->ix + 0x01) & 0x01)                           /* 5224 */
        confronta_6(z);                                   /* 522C-5233 */
    M(z->iy + 0x1e) = z->c;                               /* 5234 */
    M(z->ix + 0x01) |= 0x01;                              /* 5237 */
}

/* 0x523C: come 5224 ma senza azzerare C (con il bit spento (IY+1E) = C
 * d'ingresso) e con il bit 0 di (IX+1) spento. Senza cattura propria. */
static void sub_523c(struct z80 *z)
{
    if (M(z->ix + 0x01) & 0x01) {                         /* 523C */
        z->c = 0x00;                                      /* 5242 */
        confronta_6(z);                                   /* 5244-524B */
    }
    M(z->iy + 0x1e) = z->c;                               /* 524C */
    M(z->ix + 0x01) &= (uint8_t)~0x01;                    /* 524F */
}

/* 0x5254: come 5224, in più (IY+1D) = 0 e bit 2 di (IX+0) spento.
 * Senza cattura propria. */
static void sub_5254(struct z80 *z)
{
    int acceso = M(z->ix + 0x01) & 0x01;                  /* 5254 */
    z->c = 0x00;                                          /* 5258 */
    if (acceso)
        confronta_6(z);                                   /* 525C-5263 */
    M(z->iy + 0x1d) = 0x00;                               /* 5264 */
    M(z->iy + 0x1e) = z->c;                               /* 5268 */
    M(z->ix) &= (uint8_t)~0x04;                           /* 526B */
    M(z->ix + 0x01) |= 0x01;                              /* 526F */
}

/* 0x51FA: tipo 06 di 4F1B. 51D7 sceglie un bit acceso della maschera BC
 * (A = E = indice); con il bit 0 di (IX+1) spento (IX+10) = A. Poi
 * `rst 38` al sito 5207 su 14 voci (indice E = A): le destinazioni
 * ritornano al chiamante di 51FA. Legge R tramite 51D7 e 54C5: nel banco
 * serve banco_r_51fa. */
void sub_51fa(struct z80 *z)
{
    sub_51d7(z);                                          /* 51FA */
    if (!(M(z->ix + 0x01) & 0x01))                        /* 51FD */
        M(z->ix + 0x10) = z->a;                           /* 5203 */
    z->e = z->a;                                          /* 5206 */
    uint16_t dest = nemici_rst38(z, 0x5207, rst38_5207,
                                 sizeof rst38_5207 / sizeof rst38_5207[0]);
    switch (dest) {                                       /* 5207 rst 38 */
    case 0x5224: sub_5224(z); break;
    case 0x5274: sub_5274(z); break;
    case 0x523C: sub_523c(z); break;
    case 0x5254: sub_5254(z); break;
    case 0x5289: sub_5289(z); break;
    case 0x5292: sub_5292(z); break;
    case 0x52DB: sub_52db(z); break;
    case 0x52AA: sub_52aa(z); break;
    case 0x52C2: sub_52c2(z); break;
    case 0x52EC: sub_52ec(z); break;
    case 0:      break;                                   /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}

/* 0x4FEF: BC = parola n. (A - 0x10) della tabella 4FF9 (anticipata da N10,
 * richiesta da 5CFB e 5FAC). Uscita: A = A - 0x10, DE = A, HL sul secondo
 * byte della parola, B = byte alto, C = byte basso. */
void sub_4fef(struct z80 *z)
{
    z->a = (uint8_t)(z->a - 0x10);                        /* 4FEF */
    SET_HL(z, 0x4ff9);                                    /* 4FF1 */
    sub_0008(z);                                          /* 4FF4 */
    z->c = leggi(HL(z));                                  /* 4FF5 */
    SET_HL(z, (uint16_t)(HL(z) + 1));                     /* 4FF6 */
    z->b = leggi(HL(z));                                  /* 4FF7 */
}

/* ---- Lotto N10 (brief 36) ---- */

/* 0x5005: destinazione 0 di 4FD4. Al primo passaggio accende il bit 2 di
 * (IX+0) e azzera (IX+0D); poi, quando (IX+0D) >= 3, spegne il bit e porta
 * (IY+1D) a 3. */
void sub_5005(struct z80 *z)
{
    if (!(IXB(0x00) & 0x04)) {                            /* 5005 */
        IXB(0x00) |= 0x04;                                /* 500B */
        IXB(0x0d) = 0x00;                                 /* 500F */
        return;                                           /* 5013 */
    }
    z->a = IXB(0x0d);                                     /* 5014 */
    if (z->a < 0x03)                                      /* 5017 */
        return;                                           /* 5019 */
    IXB(0x00) &= (uint8_t)~0x04;                          /* 501A */
    IYB(0x1d) = 0x03;                                     /* 501E */
}

/* 0x5066: ingresso interno, B = 3 e cade nella coda 5068. */
void sub_5066(struct z80 *z)
{
    z->b = 0x03;                                          /* 5066 */
    sub_5068(z);                                          /* 5068 */
}

/* 0x5035: ingresso interno (da 5023 e 5092). Con B + 3 (nibble basso) < 6
 * riallinea la posizione dell'oggetto: (IY+0D) = 0, (IY+0E) = nibble alto
 * di (IY+0E) + 0a, (IY+14) = 0, (IY+15) = B arrotondato al nibble alto,
 * (IY+1D) = 5, bit 1 di (IY+1E) acceso. Altrimenti 5066. */
void sub_5035(struct z80 *z)
{
    z->a = (uint8_t)((z->b + 0x03) & 0x0f);               /* 5035-5038 */
    if (z->a >= 0x06) {                                   /* 503A */
        sub_5066(z);                                      /* 503C */
        return;
    }
    z->a = z->b;                                          /* 503E */
    if (z->a & 0x08)                                      /* 503F */
        z->a = (uint8_t)(z->a + 0x08);                    /* 5043 */
    z->a &= 0xf0;                                         /* 5045 */
    z->c = z->a;                                          /* 5047 */
    z->a = (uint8_t)((IYB(0x0e) & 0xf0) + 0x0a);          /* 5048-504D */
    z->b = z->a;                                          /* 504F */
    z->a = 0x00;                                          /* 5050 */
    IYB(0x0d) = z->a;                                     /* 5051 */
    IYB(0x0e) = z->b;                                     /* 5054 */
    IYB(0x14) = z->a;                                     /* 5057 */
    IYB(0x15) = z->c;                                     /* 505A */
    IYB(0x1d) = 0x05;                                     /* 505D */
    IYB(0x1e) |= 0x02;                                    /* 5061 */
}

/* 0x5023: destinazione 1 di 4FD4. c2b6 = ff, bit 7 di (IX+1) spento,
 * B = (IY+15); con il bit 1 di (IY+1E) spento 5066, altrimenti 5035. */
void sub_5023(struct z80 *z)
{
    z->a = 0xff;                                          /* 5023 */
    M(C2B6) = z->a;                                       /* 5025 */
    IXB(0x01) &= (uint8_t)~0x80;                          /* 5028 */
    z->b = IYB(0x15);                                     /* 502C */
    if (!(IYB(0x1e) & 0x02)) {                            /* 502F */
        sub_5066(z);                                      /* 5033 */
        return;
    }
    sub_5035(z);                                          /* 5035 */
}

/* 0x5092: destinazione 2 di 4FD4. c2b6 = ff; con (IY+0C) in 80-fe: bit 7 di
 * (IX+1) spento e 5035 con B = (IY+15); altrimenti B = 6 (9 con il bit 0
 * di (IY+1E)) e la coda 5068. */
void sub_5092(struct z80 *z)
{
    z->a = 0xff;                                          /* 5092 */
    M(C2B6) = z->a;                                       /* 5094 */
    z->a = IYB(0x0c);                                     /* 5097 */
    if ((z->a & 0x80) && z->a < 0xff) {                   /* 509A-50A1 */
        z->b = IYB(0x15);                                 /* 50A3 */
        IXB(0x01) &= (uint8_t)~0x80;                      /* 50A6 */
        sub_5035(z);                                      /* 50AA */
        return;
    }
    z->b = IYB(0x1e);                                     /* 50AD */
    z->b = (z->b & 0x01) ? 0x09 : 0x06;                   /* 50B0-50B6 */
    sub_5068(z);                                          /* 50B8 */
}

/* 0x50BA: destinazione 6 di 4FD4. Con (IY+13) + 1 < 2: bit 1 di (IY+1E)
 * acceso -> 531C (che con la cella piena chiude da sé); spento ->
 * (IY+14) = 0, (IY+15) = ((IY+15) + 4) & f8, (IY+1E) = 2. Esce con
 * A = (IY+13) + 1 nel caso normale. */
void sub_50ba(struct z80 *z)
{
    z->a = (uint8_t)(IYB(0x13) + 1);                      /* 50BA-50BD */
    if (z->a >= 0x02)                                     /* 50BE */
        return;                                           /* 50DE */
    if (IYB(0x1e) & 0x02) {                               /* 50C2 */
        sub_531c(z);                                      /* 50C8 */
        return;
    }
    z->a = (uint8_t)((IYB(0x15) + 0x04) & 0xf8);          /* 50CC-50D1 */
    IYB(0x14) = 0x00;                                     /* 50D3 */
    IYB(0x15) = z->a;                                     /* 50D7 */
    IYB(0x1e) = 0x02;                                     /* 50DA */
}

/* 0x5115: B &= 11, C &= a3; se entrambi zero BC = 0081. Esce con
 * A = B | C. */
void sub_5115(struct z80 *z)
{
    z->b = 0x11 & z->b;                                   /* 5115-5118 */
    z->c = 0xa3 & z->c;                                   /* 5119-511C */
    z->a = (uint8_t)(z->c | z->b);                        /* 511D */
    if (z->a == 0) {                                      /* 511E */
        z->b = 0x00;                                      /* 5120 */
        z->c = 0x81;
    }
}

/* 0x513A: BC = 0040 (destinazione 0 di 5127). */
void sub_513a(struct z80 *z)
{
    z->b = 0x00;                                          /* 513A */
    z->c = 0x40;
}

/* 0x513E: C &= 21, B = 0; con C = 0 accende il bit 7 di (IX+1) e C = 1. */
void sub_513e(struct z80 *z)
{
    z->a = 0x21 & z->c;                                   /* 513E-5140 */
    z->c = z->a;                                          /* 5141 */
    z->b = 0x00;                                          /* 5142 */
    if (z->a != 0)                                        /* 5144 */
        return;
    IXB(0x01) |= 0x80;                                    /* 5145 */
    z->c = 0x01;                                          /* 5149 */
}

/* 0x514C: L = C & 21, H = B & 20; se uno è acceso BC = HL; altrimenti con
 * B & 10 BC = (B & 10, 0); altrimenti bit 7 di (IX+1) acceso e BC = 0021.
 * HL lasciato come calcolato. */
void sub_514c(struct z80 *z)
{
    z->l = 0x21 & z->c;                                   /* 514C-514F */
    z->h = 0x20 & z->b;                                   /* 5150-5153 */
    z->a = (uint8_t)(z->h | z->l);                        /* 5154 */
    if (z->a != 0) {                                      /* 5155 */
        z->c = z->l;                                      /* 5157 */
        z->b = z->h;                                      /* 5158 */
        return;
    }
    z->a = 0x10 & z->b;                                   /* 515A-515C */
    if (z->a != 0) {                                      /* 515D */
        z->b = z->a;                                      /* 515F */
        z->c = 0x00;                                      /* 5160 */
        return;
    }
    IXB(0x01) |= 0x80;                                    /* 5163 */
    z->b = 0x00;                                          /* 5167 */
    z->c = 0x21;
}

/* 0x516B: con B & 20 BC = (B & 20, 0); altrimenti B &= 11, C &= a3 e, se
 * entrambi zero, bit 7 di (IX+1) acceso e BC = 0081. */
void sub_516b(struct z80 *z)
{
    z->a = 0x20 & z->b;                                   /* 516B-516D */
    if (z->a != 0) {                                      /* 516E */
        z->b = z->a;                                      /* 5170 */
        z->c = 0x00;                                      /* 5171 */
        return;
    }
    z->b = 0x11 & z->b;                                   /* 5174-5177 */
    z->c = 0xa3 & z->c;                                   /* 5178-517B */
    z->a = (uint8_t)(z->c | z->b);                        /* 517C */
    if (z->a != 0)                                        /* 517D */
        return;
    IXB(0x01) |= 0x80;                                    /* 517E */
    z->b = 0x00;                                          /* 5182 */
    z->c = 0x81;
}

/* 0x5186: L = B & 30, H = C & 80; se H BC = (L, H); altrimenti con C & 20
 * BC = (0, C & 20); altrimenti bit 7 di (IX+1) acceso e BC = 0080. */
void sub_5186(struct z80 *z)
{
    z->l = 0x30 & z->b;                                   /* 5186-5189 */
    z->h = 0x80 & z->c;                                   /* 518A-518D */
    z->a = z->h;
    if (z->h != 0) {                                      /* 518E */
        z->b = z->l;                                      /* 5190 */
        z->c = z->h;                                      /* 5191 */
        return;
    }
    z->a = 0x20 & z->c;                                   /* 5193-5195 */
    if (z->a != 0) {                                      /* 5196 */
        z->c = z->a;                                      /* 5198 */
        z->b = 0x00;                                      /* 5199 */
        return;
    }
    IXB(0x01) |= 0x80;                                    /* 519C */
    z->b = 0x00;                                          /* 51A0 */
    z->c = 0x80;
}

/* 0x51B1: BC = 0080 (voce della tabella 5127 e coda di 51A4). */
void sub_51b1(struct z80 *z)
{
    z->b = 0x00;                                          /* 51B1 */
    z->c = 0x80;
}

/* 0x51A4: B &= 10, C &= 80; se uno è acceso esce; altrimenti bit 7 di
 * (IX+1) acceso e BC = 0080 (cade in 51B1). */
void sub_51a4(struct z80 *z)
{
    z->b = 0x10 & z->b;                                   /* 51A4-51A7 */
    z->a = 0x80 & z->c;                                   /* 51A8-51AA */
    z->c = z->a;                                          /* 51AB */
    if (z->a != 0 || z->b != 0)                           /* 51AC: ret nz */
        return;
    IXB(0x01) |= 0x80;                                    /* 51AD */
    sub_51b1(z);                                          /* 51B1 */
}

/* 0x51B5: A = (IY+4), BC da 4FEF; (IX+0B/0C) = BC - 3, bit 2 di (IX+0)
 * spento, c2b6 = ff, B = 3c e la coda 5068. HL = BC - 3. */
void sub_51b5(struct z80 *z)
{
    z->a = M(z->iy + 0x04);                               /* 51B5 */
    sub_4fef(z);                                          /* 51B8 */
    SET_HL(z, (uint16_t)(0xfffd + BC(z)));                /* 51BB-51BE */
    IXB(0x0b) = z->l;                                     /* 51BF */
    IXB(0x0c) = z->h;                                     /* 51C2 */
    IXB(0x00) &= (uint8_t)~0x04;                          /* 51C5 */
    z->a = 0xff;                                          /* 51C9 */
    M(C2B6) = z->a;                                       /* 51CB */
    z->b = 0x3c;                                          /* 51CE */
    sub_5068(z);                                          /* 51D0 */
}

/* 0x51D3: BC = 0001. */
void sub_51d3(struct z80 *z)
{
    z->b = 0x00;                                          /* 51D3 */
    z->c = 0x01;
}

/* 0x5124: tipo 06, scelta della direzione. E = (IX+7); `rst 38` al sito
 * 5127 su 9 voci (le destinazioni ritornano al chiamante di 5124). */
void sub_5124(struct z80 *z)
{
    z->e = IXB(0x07);                                     /* 5124 */
    uint16_t dest = nemici_rst38(z, 0x5127, rst38_5127,
                                 sizeof rst38_5127 / sizeof rst38_5127[0]);
    switch (dest) {                                       /* 5127 rst 38 */
    case 0x513A: sub_513a(z); break;
    case 0x513E: sub_513e(z); break;
    case 0x514C: sub_514c(z); break;
    case 0x516B: sub_516b(z); break;
    case 0x5186: sub_5186(z); break;
    case 0x51A4: sub_51a4(z); break;
    case 0x51B1: sub_51b1(z); break;
    case 0x51B5: sub_51b5(z); break;
    case 0x51D3: sub_51d3(z); break;
    case 0:      break;                                   /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}

/* 0x50DF: destinazione 5 di 4FD4. 531C (con la cella piena ha già chiuso:
 * si ritorna); HL = (IX+0B/0C) - BC resta nei registri anche con
 * riporto (uscita). Altrimenti (IX+0B/0C) = 0, 5345, BC = BC / 2 con il
 * bit 0 di B in testa a C, in (IX+0E/0F); con il bit 5 di (IX+0) 5124,
 * altrimenti 5115; poi 51FA. Legge R tramite 51FA: banco_r_50df. */
void sub_50df(struct z80 *z)
{
    sub_531c(z);                                          /* 50DF */
    if (NEMICI_RIPORTO_ACCESO(z))
        return;                                           /* uscita anomala */
    uint16_t hl = (uint16_t)(IXB(0x0b) | (IXB(0x0c) << 8)); /* 50E2-50E5 */
    int prestito = hl < BC(z);
    SET_HL(z, (uint16_t)(hl - BC(z)));                    /* 50E8 */
    if (prestito)
        return;                                           /* 50EA */
    z->a = 0x00;                                          /* 50EB */
    IXB(0x0b) = z->a;                                     /* 50EC */
    IXB(0x0c) = z->a;                                     /* 50EF */
    sub_5345(z);                                          /* 50F2 */
    int b0 = z->b & 1;
    z->c = (uint8_t)(z->c << 1);                          /* 50F5 sla c */
    z->b >>= 1;                                           /* 50F7 srl b */
    z->c = (uint8_t)((z->c >> 1) | (b0 << 7));            /* 50F9 rr c */
    IXB(0x0e) = z->c;                                     /* 50FB */
    IXB(0x0f) = z->b;                                     /* 50FE */
    if (IXB(0x00) & 0x20)                                 /* 5101 */
        sub_5124(z);                                      /* 5107 */
    else
        sub_5115(z);                                      /* 510E */
    sub_51fa(z);                                          /* 510A, 5111 */
}

/* 0x4FD4: tipo 6 (smistamento da 4F1B). 52FD (passo e direzione), 4FEF con
 * A = (IY+4), E = (IY+1D); `rst 38` al sito 4FE0 su 7 voci (le destinazioni
 * ritornano al chiamante). Legge R tramite 50DF: banco_r_4fd4. */
void sub_4fd4(struct z80 *z)
{
    sub_52fd(z);                                          /* 4FD4 */
    z->a = IYB(0x04);                                     /* 4FD7 */
    sub_4fef(z);                                          /* 4FDA */
    z->e = IYB(0x1d);                                     /* 4FDD */
    uint16_t dest = nemici_rst38(z, 0x4FE0, rst38_4FE0,
                                 sizeof rst38_4FE0 / sizeof rst38_4FE0[0]);
    switch (dest) {                                       /* 4FE0 rst 38 */
    case 0x5005: sub_5005(z); break;
    case 0x5023: sub_5023(z); break;
    case 0x5092: sub_5092(z); break;
    case 0x3B62: sub_3b62(z); break;
    case 0x50DF: sub_50df(z); break;
    case 0x50BA: sub_50ba(z); break;
    case 0:      break;                                   /* fuori tabella */
    default:     nemici_manca(dest); break;
    }
}
