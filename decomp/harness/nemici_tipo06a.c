/* Motore dei nemici, lotto N3 (brief 29): due dipendenze del tipo 09 che il
 * censimento assegna al lotto N10 (tipo 06, parte A): 51D7, che sceglie a
 * caso uno dei bit accesi di una maschera, e 24DF, un piccolo calcolo su
 * nibble con una coppia di byte della tabella puntata da HL.
 * Lotto N6 (brief 32): le cinque routine del tipo 06 che il censimento
 * assegna a N10 e che le 8 routine di nemici_tipo06b.c richiedono: 5068
 * (coda condivisa), 51FA (smista con `rst 38` a 5207 sul bit acceso scelto
 * da 51D7), 5224, 523C e 5254 (tre destinazioni della tabella).
 * Lotto N8 (brief 34): 4FEF, che 5CFB e 5FAC richiedono.
 * Tradotto da out/main64.dasm (out/nemici_n6.dasm). */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"

#define TAB_C06A 0xc06a   /* RAM: posizioni dei bit accesi della maschera */

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
