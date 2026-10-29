/* Motore dei nemici, comparsa (lotto N4, brief 30), prima parte: i tipi 0
 * (4F40) del gestore 4F1B con le routine 13xx/14xx che posta e disegna
 * (tabellone punteggio, suoni, attività 05/19 postate con 0493), la
 * distanza di 738F e la cella vicina di 734D. Seconda parte nel modulo
 * nemici_comparsab.c (tipi 5 e 4: 7288 e 73A3).
 * Ingressi: 134B, 139C, 13AB, 13B9, 13FC, 1413, 14D4, 1549, 2548 (in
 * nemici_comparsab.c), 4F40, 4FC5, 734D, 738F, 80A5; 14FC è statica. 80A5
 * (N2) e 734D (N1) sono arrivate prima.
 *
 * Tradotto da out/main64.dasm. IX = slot, IY = oggetto. Il riporto in
 * uscita (738F) sta nel bit 0 di z->f. */
#include "routine.h"
#include "nemici.h"

/* 0x734D: sceglie gli scostamenti (H, L) fra B e C dal primo bit acceso
 * di A (dal bit 0: (B, C), (B, B), (C, B), (C, C)), li somma a (IY+0E) e
 * (IY+15) e chiama sub_36f0 (HL = cella, E = indice). Senza bit accesi
 * nei bit 0-3 l'originale fa `ld (0000h),a` (scrittura in ROM, nessun
 * effetto) e ritorna con H = C. */
void sub_734d(struct z80 *z)
{
    uint8_t a = z->a;
    z->h = z->b;                               /* 734D */
    if (a & 0x01) {                            /* 734E-734F rra */
        z->l = z->c;                           /* 7351 */
    } else if (a & 0x02) {                     /* 7354-7355 */
        z->l = z->b;                           /* 7357 */
    } else {
        z->h = z->c;                           /* 735A */
        if (a & 0x04) {                        /* 735B-735C */
            z->l = z->b;                       /* 735E */
        } else if (a & 0x08) {                 /* 7361-7362 */
            z->l = z->c;                       /* 7364 */
        } else {
            /* 7367 ld (0000h),a: A ruotato quattro volte attraverso il riporto */
            z->a = (uint8_t)(a >> 4 | (a << 5) | ((z->f & 1) << 4));
            return;                            /* 736A */
        }
    }
    z->h = (uint8_t)(z->h + M(z->iy + 0x0e));  /* 736B-736F */
    z->l = (uint8_t)(z->l + M(z->iy + 0x15));  /* 7370-7374 */
    sub_36f0(z);                               /* 7375 */
}

/* 0x80A5 (lotto N4, anticipata dal lotto N2): azzera (IX+0), (IX+1),
 * (IY+0), (IY+1): slot e oggetto liberi. A = 0. */
void sub_80a5(struct z80 *z)
{
    z->a = 0;                                  /* 80A5 */
    M(z->ix + 0x00) = z->a;                    /* 80A6 */
    M(z->ix + 0x01) = z->a;                    /* 80A9 */
    M(z->iy + 0x00) = z->a;                    /* 80AC */
    M(z->iy + 0x01) = z->a;                    /* 80AF */
}

#include "rst38_tables.h"

#define IXB(o) M(z->ix + (o))
#define IYB(o) M(z->iy + (o))

#define PAROLA(a) ((uint16_t)(M(a) | (M((a) + 1) << 8)))
#define PONI_PAROLA(a, v) do { uint16_t v_ = (v); M(a) = v_ & 0xff; M((a) + 1) = v_ >> 8; } while (0)

static void sub_14fc(struct z80 *z);

/* ------------------------------------------------------------------ */
/* 0x134B: (c25a) += DE; se il risultato ha il bit 9 (A = 2 & H) acceso
 * diventa 0200. Uscita: HL = nuovo valore, A = 2 & H. */
void sub_134b(struct z80 *z)
{
    uint16_t hl = (uint16_t)(PAROLA(0xc25a) + DE(z));        /* 134B-134E */
    z->a = 0x02 & (hl >> 8);                                 /* 134F-1351 */
    if (z->a)
        hl = 0x0200;                                         /* 1354 */
    SET_HL(z, hl);
    PONI_PAROLA(0xc25a, hl);                                 /* 1357 */
}

/* 0x1549: A = L; (c1aa) += DE; (c1ac) += A + riporto. Uscita: A = nuovo
 * (c1ac), HL = c1ac. */
void sub_1549(struct z80 *z)
{
    uint32_t somma = (uint32_t)PAROLA(0xc1aa) + DE(z);       /* 154A-154D */
    PONI_PAROLA(0xc1aa, (uint16_t)somma);                    /* 154E */
    uint8_t a = (uint8_t)(z->l + M(0xc1ac) + (somma >> 16)); /* 1549, 1554 */
    M(0xc1ac) = a;                                           /* 1555 */
    z->a = a;
    SET_HL(z, 0xc1ac);
}

/* 0x13FC: A -= 29; con A = 0d (suono 12 e barra) va a 14FC; altrimenti
 * somma alla tabella ROM 150C (3 byte per voce: parola in DE, byte in L)
 * con 1549 e posta l'attività 05. Termina con 0493. */
void sub_13fc(struct z80 *z)
{
    z->a = (uint8_t)(z->a - 0x29);                           /* 13FC */
    if (z->a == 0x0d) {                                      /* 13FE-1400 */
        sub_14fc(z);
        return;
    }
    SET_HL(z, 0x150c);                                       /* 1403 */
    sub_0008(z);                                             /* rst 8 */
    SET_HL(z, HL(z) + DE(z));                                /* 1407 */
    sub_0010(z);                                             /* rst 10h */
    z->l = leggi(HL(z));                                     /* 1409 */
    sub_1549(z);                                             /* 140A */
    z->a = 0x05;                                             /* 140D */
    sub_0493(z);                                             /* 140F */
}

/* 0x14FC (statica): suono 12, c237 += 1, barra con 31E9 (A = vecchio
 * c237), poi 13FC con A = 2c. */
static void sub_14fc(struct z80 *z)
{
    z->c = 0x12;                                             /* 14FC */
    sub_0028(z);                                             /* rst 28h */
    SET_HL(z, 0xc237);                                       /* 14FF */
    z->a = M(0xc237);                                        /* 1502 */
    M(0xc237)++;                                             /* 1503 */
    sub_31e9(z);                                             /* 1504 */
    z->a = 0x2c;                                             /* 1507 */
    sub_13fc(z);                                             /* 1509 */
}

/* 0x139C: (c1ad) = 0b, posta l'attività 19, A = 2a, 13FC. */
void sub_139c(struct z80 *z)
{
    z->a = 0x0b;                                             /* 139C */
    M(0xc1ad) = z->a;                                        /* 139E */
    z->a = 0x19;                                             /* 13A1 */
    sub_0493(z);                                             /* 13A3 */
    z->a = 0x2a;                                             /* 13A6 */
    sub_13fc(z);                                             /* 13A8 */
}

/* 0x13AB: come 139C con (c1ad) = 0c e A = 2b. */
void sub_13ab(struct z80 *z)
{
    z->a = 0x0c;                                             /* 13AB */
    M(0xc1ad) = z->a;                                        /* 13AD */
    z->a = 0x19;                                             /* 13B0 */
    sub_0493(z);                                             /* 13B2 */
    z->a = 0x2b;                                             /* 13B5 */
    sub_13fc(z);                                             /* 13B7 */
}

/* 0x13B9: DE = 10, 134B, A = 2a, 13FC. */
void sub_13b9(struct z80 *z)
{
    SET_DE(z, 0x0010);                                       /* 13B9 */
    sub_134b(z);                                             /* 13BC */
    z->a = 0x2a;                                             /* 13BF */
    sub_13fc(z);                                             /* 13C1 */
}

/* 0x1413: c259 sale di 1 fino a 9 (non oltre), ridisegna il tabellone
 * (349F), A = 2a, 13FC. */
void sub_1413(struct z80 *z)
{
    SET_HL(z, 0xc259);                                       /* 1413 */
    z->a = (uint8_t)(M(0xc259) + 1);                         /* 1416-1417 */
    if (z->a < 0x09)                                         /* 1418-141A */
        M(0xc259) = z->a;                                    /* 141C */
    sub_349f(z);                                             /* 141D */
    z->a = 0x2a;                                             /* 1420 */
    sub_13fc(z);                                             /* 1422 */
}

/* 0x14D4: cerca (fino a 8 volte, DE = 4000 che scorre di 2 bit) la prima
 * maschera che ha bit in comune con la parola c25c; se la trova la somma a
 * c25c e ridisegna il tabellone (349F). Poi A = 2a e 13FC. */
void sub_14d4(struct z80 *z)
{
    z->b = 0x08;                                             /* 14D4 */
    SET_HL(z, PAROLA(0xc25c));                               /* 14D6 */
    SET_DE(z, 0x4000);                                       /* 14D9 */
    for (;;) {
        z->a = z->e & z->l;                                  /* 14DC-14DE */
        if (!z->a) {
            z->a = z->d & z->h;                              /* 14E0-14E2 */
            if (!z->a) {
                SET_DE(z, DE(z) >> 2);                       /* 14E4-14EA */
                if (--z->b == 0)                             /* 14EC */
                    break;
                continue;
            }
        }
        SET_HL(z, HL(z) + DE(z));                            /* 14F0 */
        PONI_PAROLA(0xc25c, HL(z));                          /* 14F1 */
        sub_349f(z);                                         /* 14F4 */
        break;
    }
    z->a = 0x2a;                                             /* 14F7 */
    sub_13fc(z);                                             /* 14F9 */
}

/* ------------------------------------------------------------------ */
/* 0x738F: distanza dell'oggetto dal giocatore: |(IX+4)| contro B; se
 * |(IX+4)| >= B esce senza riporto, altrimenti |(IX+5)| contro C. Uscita:
 * riporto (bit 0 di F) acceso se entrambe le distanze sono sotto B e C;
 * A = ultimo valore assoluto confrontato. */
void sub_738f(struct z80 *z)
{
    uint8_t v = IXB(0x04);                                   /* 738F */
    uint8_t a = (uint8_t)(v << 1);                           /* 7392 */
    if (v & 0x80)
        a = (uint8_t)-a;                                     /* 7395 */
    z->a = a;
    if (a >= z->b) {                                         /* 7397 */
        NEMICI_PONI_RIPORTO(z, 0);
        return;                                              /* 7398 */
    }
    v = IXB(0x05);                                           /* 7399 */
    a = (uint8_t)(v << 1);                                   /* 739C */
    if (v & 0x80)
        a = (uint8_t)-a;                                     /* 739F */
    z->a = a;
    NEMICI_PONI_RIPORTO(z, a < z->c);                        /* 73A1 */
}

/* 0x4FC5: c05a = 80, (c058) = (L = (IY+15), H = (IY+0E)). Uscita: A = 80,
 * HL = posizione. */
void sub_4fc5(struct z80 *z)
{
    z->h = IYB(0x0e);                                        /* 4FC5 */
    z->l = IYB(0x15);                                        /* 4FC8 */
    z->a = 0x80;                                             /* 4FCB */
    M(0xc05a) = z->a;                                        /* 4FCD */
    PONI_PAROLA(0xc058, HL(z));                              /* 4FD0 */
}

/* 0x4F40: tipo 0 del gestore 4F1B (comparsa dell'entità). Con (IY+1D) != 0:
 * se il giocatore è vicino (738F, B = 09, C = 08) smista con `rst 38` su
 * 4FAC (il ritorno è 4FC2: 80A5) dopo 4FC5 e il suono 15; altrimenti, se
 * (IX+0B/0C) >= 0280 libera lo slot, sennò sceglie il bit 0 di (IY+1E) da
 * (IY+1A). Con (IY+1D) = 0 attende (IX+8) >= 23 e prende fase e direzione
 * da (IX+0E). */
void sub_4f40(struct z80 *z)
{
    z->a = IYB(0x1d);                                        /* 4F40 */
    if (z->a) {                                              /* 4F43-4F44 */
        SET_BC(z, 0x0908);                                   /* 4F46 */
        sub_738f(z);                                         /* 4F49 */
        if (!NEMICI_RIPORTO_ACCESO(z)) {                     /* 4F4C */
            z->l = IXB(0x0b);                                /* 4F4E */
            z->h = IXB(0x0c);                                /* 4F51 */
            SET_DE(z, 0x0280);                               /* 4F54 */
            uint16_t hl = HL(z);
            SET_HL(z, (uint16_t)(hl - 0x0280));              /* 4F57 */
            if (hl >= 0x0280) {                              /* 4F59 */
                sub_80a5(z);
                return;
            }
            z->a = IYB(0x1a) & 0x30;                         /* 4F5C-4F5F */
            if (z->a)
                IYB(0x1e) &= (uint8_t)~0x01;                 /* 4F63 */
            else
                IYB(0x1e) |= 0x01;                           /* 4F68 */
            return;
        }
        sub_4fc5(z);                                         /* 4F8F */
        uint8_t b = (uint8_t)(IYB(0x1e) >> 1);               /* 4F92-4F97 */
        z->b = b;
        z->a = (uint8_t)(((IYB(0x1d) - 1) * 2) + b);         /* 4F98-4F9D */
        if (z->a >= 0x06) {                                  /* 4F9E-4FA0 */
            z->a = (uint8_t)(z->a + 0x23);                   /* 4FB9 */
            z->c = 0x10;                                     /* 4FBB */
            sub_0028(z);                                     /* rst 28h */
            z->c = z->a;                                     /* 4FBE */
            sub_13fc(z);                                     /* 4FBF */
            sub_80a5(z);                                     /* 4FC2 */
            return;
        }
        z->c = 0x15;                                         /* 4FA2 */
        sub_0028(z);                                         /* 4FA4 */
        z->e = z->a;                                         /* 4FA7 */
        SET_HL(z, 0x4FC2);                                   /* 4FA8-4FAB push hl */
        switch (nemici_rst38(z, 0x4FAC, rst38_4FAC, 6)) {    /* 4FAC */
        case 0x14FC: sub_14fc(z); break;
        case 0x13AB: sub_13ab(z); break;
        case 0x139C: sub_139c(z); break;
        case 0x13B9: sub_13b9(z); break;
        case 0x1413: sub_1413(z); break;
        case 0x14D4: sub_14d4(z); break;
        default: return;                                     /* fuori tabella */
        }
        sub_80a5(z);                                         /* 4FC2 */
        return;
    }
    z->a = IXB(0x08);                                        /* 4F6D */
    if (z->a < 0x23)                                         /* 4F70-4F72 */
        return;
    z->a = IXB(0x0e);                                        /* 4F73 */
    if (!z->a) {                                             /* 4F76-4F77 */
        sub_80a5(z);
        return;
    }
    z->c = z->a;                                             /* 4F7A */
    IYB(0x1e) = (uint8_t)((z->a << 1) & 0x02);               /* 4F7B-4F7E */
    z->a = z->c;                                             /* 4F81 */
    z->a >>= 1;                                              /* 4F82 */
    IYB(0x1d) = z->a;                                        /* 4F84 */
    z->a = 0;                                                /* 4F87 */
    IXB(0x0b) = z->a;                                        /* 4F88 */
    IXB(0x0c) = z->a;                                        /* 4F8B */
}
