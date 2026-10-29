/* Motore dei nemici, comparsa (lotto N4, brief 30), seconda parte: i tipi
 * 5 (7288) e 4 (73A3) del gestore 4F1B, con le routine di movimento e di
 * urto che 74AB chiama (756C, 758E, 759F, 2548). Prima parte (4F40 e le
 * routine 13xx/14xx) in nemici_comparsa.c.
 * Ingressi: 2548, 7288, 7294, 72B6, 72CE, 72DA, 73A3, 7412, 741D, 7470,
 * 74AB, 756C, 758E, 759F; 73C7 e la coda 7456 sono statiche.
 *
 * Tradotto da out/main64.dasm. IX = slot, IY = oggetto. Il riporto in
 * uscita (756C, 758E) sta nel bit 0 di z->f; quello in ingresso a 758E
 * si legge da lì. */
#include "routine.h"
#include "nemici.h"
#include "rst38_tables.h"

#define IXB(o) M(z->ix + (o))
#define IYB(o) M(z->iy + (o))

#define TAB_7468 0x7468   /* ROM: velocità per tipo (coppie di byte) */
#define TAB_754C 0x754c   /* ROM: parole per l'urto in X */
#define TAB_755C 0x755c   /* ROM: parole per l'urto in Y */

static void sub_73c7(struct z80 *z);
static void coda_7456(struct z80 *z);

/* HL - DE - riporto in ingresso (bit 0 di F), 16 bit; il riporto in uscita
 * (prestito) va nel bit 0 di F. */
static uint16_t sbc16(struct z80 *z, uint16_t hl, uint16_t de)
{
    uint32_t r = (uint32_t)hl - de - (z->f & NEMICI_RIPORTO);
    NEMICI_PONI_RIPORTO(z, r > 0xffff);
    return (uint16_t)r;
}

/* ------------------------------------------------------------------ */
/* 0x2548: HL += 8 * A (A modulo 32). Uscita: E = A * 8 (8 bit), D = 0,
 * A = E. */
void sub_2548(struct z80 *z)
{
    z->e = (uint8_t)(z->a << 3);                             /* 2548-254B */
    z->d = 0;                                                /* 254C-254D */
    z->a = z->e;                                             /* 254E */
    SET_HL(z, HL(z) + DE(z));                                /* 254F */
}

/* 0x759F: HL += 8 * (B - 0A) (2548), A = (IX+6) ^ C. */
void sub_759f(struct z80 *z)
{
    z->a = (uint8_t)(z->b - 0x0a);                           /* 759F-75A0 */
    sub_2548(z);                                             /* 75A2 */
    z->a = IXB(0x06) ^ z->c;                                 /* 75A5-75A8 */
}

/* 0x756C: differenza di posizione fra due parole. HL = base, BC = indice;
 * C = (HL+BC) esteso nel segno in B; due parole successive: la prima in
 * HL, la seconda più BC in DE. HL = prima - seconda (in valore assoluto
 * se la seconda è negativa), riporto secondo il segno di D. */
void sub_756c(struct z80 *z)
{
    SET_HL(z, HL(z) + BC(z));                                /* 756C */
    z->c = leggi(HL(z));                                     /* 756D */
    SET_HL(z, HL(z) + 1);                                    /* 756E */
    if (z->c & 0x80)                                         /* 756F-7571 */
        z->b--;                                              /* 7573 */
    sub_0010(z);                                             /* 7574 rst 10h */
    uint16_t prima = DE(z);                                  /* push de */
    sub_0010(z);                                             /* 7576 rst 10h */
    uint32_t somma = (uint32_t)DE(z) + BC(z);                /* 7577-7579 */
    NEMICI_PONI_RIPORTO(z, somma > 0xffff);
    SET_DE(z, (uint16_t)somma);
    uint16_t hl = prima;                                     /* 757A pop hl */
    if (z->d & 0x80) {                                       /* 757B-757D */
        SET_BC(z, prima);                                    /* 757F-7580 */
        hl = sbc16(z, 0, prima);                             /* 7581-7584 */
        NEMICI_PONI_RIPORTO(z, 0);                           /* 7586 or a */
    }
    SET_HL(z, sbc16(z, hl, DE(z)));                          /* 7587 */
    if (!(z->d & 0x80))                                      /* 7589-758B */
        return;
    NEMICI_PONI_RIPORTO(z, !NEMICI_RIPORTO_ACCESO(z));       /* 758C ccf */
}

/* 0x758E: HL = 0050 (FFB0 se D ha il bit 7) - DE - riporto in ingresso;
 * riporto secondo il segno di D, come 756C. */
void sub_758e(struct z80 *z)
{
    SET_HL(z, (z->d & 0x80) ? 0xffb0 : 0x0050);              /* 758E-7595 */
    SET_HL(z, sbc16(z, HL(z), DE(z)));                       /* 7598 */
    if (!(z->d & 0x80))                                      /* 759A-759C */
        return;
    NEMICI_PONI_RIPORTO(z, !NEMICI_RIPORTO_ACCESO(z));       /* 759D ccf */
}

/* ------------------------------------------------------------------ */
/* 0x7294: tipo 5, fase 0: la prima volta accende il bit 0 di (IX+1) e
 * azzera il contatore (IX+8); poi a 10 fotogrammi passa alla fase 3,
 * spegne il bit 0 e il bit 2 di (IY+0). */
void sub_7294(struct z80 *z)
{
    if (!(IXB(0x01) & 0x01)) {                               /* 7294-7298 */
        IXB(0x01) |= 0x01;                                   /* 729A */
        IXB(0x08) = 0x00;                                    /* 729E */
        return;
    }
    z->a = IXB(0x08);                                        /* 72A3 */
    if (z->a < 0x10)                                         /* 72A6-72A8 */
        return;
    IYB(0x1d) = 0x03;                                        /* 72A9 */
    IXB(0x01) &= (uint8_t)~0x01;                             /* 72AD */
    IYB(0x00) &= (uint8_t)~0x04;                             /* 72B1 */
}

/* 0x72B6: tipo 5, fase 2: con direzione (IY+1A & 0F) guarda la cella
 * vicina (734D, BC = 0A05); se ha il bit 7 passa alla fase 1. */
void sub_72b6(struct z80 *z)
{
    z->a = IYB(0x1a) & 0x0f;                                 /* 72B6-72B9 */
    if (!z->a)                                               /* 72BB */
        return;
    SET_BC(z, 0x0a05);                                       /* 72BC */
    sub_734d(z);                                             /* 72BF */
    if (!(M(HL(z)) & 0x80))                                  /* 72C2-72C4 */
        return;
    IYB(0x1d) = 0x01;                                        /* 72C5 */
    IYB(0x1e) = 0x00;                                        /* 72C9 */
}

/* 0x72CE: tipo 5, fase 3: se (IY+0B) | (IY+12) non è nullo passa alla
 * fase 2. */
void sub_72ce(struct z80 *z)
{
    z->a = IYB(0x0b) | IYB(0x12);                            /* 72CE-72D1 */
    if (!z->a)                                               /* 72D4 */
        return;
    IYB(0x1d) = 0x02;                                        /* 72D5 */
}

/* 72EC: c835 = ff e libera slot e oggetto (80A5). */
static void libera_72ec(struct z80 *z)
{
    z->a = 0xff;                                             /* 72EC */
    M(0xc835) = z->a;                                        /* 72EE */
    sub_80a5(z);                                             /* 72F1 */
}

/* 0x72DA: tipo 5, fase 1, guidata dai bit 1 e 0 di (IX+1): bit 1 spento,
 * bit 0 spento: lo accende e azzera (IX+8); bit 0 acceso: a 10 fotogrammi
 * bit 0 spento e bit 1 acceso. Bit 1 acceso, bit 0 acceso: a 10 fotogrammi
 * libera (72EC). Bit 1 acceso, bit 0 spento (7312): senza direzione libera;
 * altrimenti guarda la cella vicina (734D) e, se ha il bit 7 e vale meno
 * di F8, chiama 4938 e 4AF6 con IX = IY. */
void sub_72da(struct z80 *z)
{
    if (IXB(0x01) & 0x02) {                                  /* 72DA-72DE */
        if (IXB(0x01) & 0x01) {                              /* 72E0-72E4 */
            z->a = IXB(0x08);                                /* 72E6 */
            if (z->a < 0x10)                                 /* 72E9-72EB */
                return;
            libera_72ec(z);
            return;
        }
        z->a = IYB(0x1a) & 0x0f;                             /* 7312-7315 */
        if (!z->a) {                                         /* 7317 */
            libera_72ec(z);
            return;
        }
        IXB(0x01) |= 0x01;                                   /* 7319 */
        IXB(0x08) = 0x00;                                    /* 731D */
        SET_BC(z, 0x0a05);                                   /* 7321 */
        sub_734d(z);                                         /* 7324 */
        z->a = M(HL(z));                                     /* 7327 */
        if (!(z->a & 0x80))                                  /* 7328-732A */
            return;                                          /* 732C scrive in ROM */
        if (z->a >= 0xf8) {                                  /* 7330-7332 */
            libera_72ec(z);
            return;
        }
        SET_HL(z, 0xc08a);                                   /* 7334 */
        M(0xc08a) = 0x00;                                    /* 7337 */
        z->b = 0x00;                                         /* 7339 */
        uint16_t ix0 = z->ix;                                /* 733B push iy */
        z->ix = z->iy;                                       /* 733D ex (sp),ix */
        sub_4938(z);                                         /* 733F */
        z->e = 0x0e;                                         /* 7342 */
        SET_BC(z, 0x0102);                                   /* 7344 */
        sub_4af6(z);                                         /* 7347 */
        z->ix = ix0;                                         /* 734A pop ix */
        return;
    }
    if (!(IXB(0x01) & 0x01)) {                               /* 72F4-72F8 */
        IXB(0x01) |= 0x01;                                   /* 72FA */
        IXB(0x08) = 0x00;                                    /* 72FE */
        return;
    }
    z->a = IXB(0x08);                                        /* 7303 */
    if (z->a < 0x10)                                         /* 7306-7308 */
        return;
    IXB(0x01) &= (uint8_t)~0x01;                             /* 7309 */
    IXB(0x01) |= 0x02;                                       /* 730D */
}

/* 0x7288: tipo 5 del gestore 4F1B: `rst 38` su (IY+1D), 4 fasi. */
void sub_7288(struct z80 *z)
{
    z->e = IYB(0x1d);                                        /* 7288 */
    switch (nemici_rst38(z, 0x728B, rst38_728B, 4)) {        /* 728B */
    case 0x7294: sub_7294(z); break;
    case 0x72DA: sub_72da(z); break;
    case 0x72B6: sub_72b6(z); break;
    case 0x72CE: sub_72ce(z); break;
    default: break;                                          /* fuori tabella */
    }
}

/* ------------------------------------------------------------------ */
/* 0x73A3: tipo 4 del gestore 4F1B: con il giocatore vicino (738F, B = C =
 * 08) fase 7 e accende il bit 2 di (IY+0); poi `rst 38` su (IY+1D). */
void sub_73a3(struct z80 *z)
{
    SET_BC(z, 0x0808);                                       /* 73A3 */
    sub_738f(z);                                             /* 73A6 */
    if (NEMICI_RIPORTO_ACCESO(z)) {                          /* 73A9 */
        IYB(0x1d) = 0x07;                                    /* 73AB */
        IYB(0x00) |= 0x04;                                   /* 73AF */
    }
    z->e = IYB(0x1d);                                        /* 73B3 */
    switch (nemici_rst38(z, 0x73B6, rst38_73B6, 8)) {        /* 73B6 */
    case 0x7412: sub_7412(z); break;
    case 0x741D: sub_741d(z); break;
    case 0x7470: sub_7470(z); break;
    case 0x3B62: sub_3b62(z); break;
    case 0x74AB: sub_74ab(z); break;
    case 0x73C7: sub_73c7(z); break;
    default: break;                                          /* fuori tabella */
    }
}

/* 0x73C7 (statica): fase 7. Prima volta (bit 4 di (IX+1) spento): suono
 * 32, accende il bit 4, c250 += 1, c238 sale di 1 (a 10: suono 12, c237 +=
 * 1, barra 31E9, c238 = 0), ridisegna (31CE), 4FC5 e posta 13FC con A =
 * (IY+4) + 22 (con (IY+4) = 0D accende il bit 6 di c24e). Dopo: a 10
 * fotogrammi libera (80A5). */
static void sub_73c7(struct z80 *z)
{
    if (IXB(0x01) & 0x10) {                                  /* 73C7-73CB */
        z->a = IXB(0x08);                                    /* 7409 */
        if (z->a < 0x10)                                     /* 740C-740E */
            return;
        sub_80a5(z);                                         /* 740F */
        return;
    }
    z->c = 0x32;                                             /* 73CD */
    sub_0028(z);                                             /* 73CF */
    IXB(0x01) |= 0x10;                                       /* 73D0 */
    SET_HL(z, 0xc250);                                       /* 73D4 */
    M(0xc250)++;                                             /* 73D7 */
    z->a = (uint8_t)(M(0xc238) + 1);                         /* 73D8-73DB */
    if (z->a >= 0x0a) {                                      /* 73DC-73DE */
        z->c = 0x12;                                         /* 73E0 */
        sub_0028(z);                                         /* 73E2 */
        SET_HL(z, 0xc237);                                   /* 73E3 */
        z->a = M(0xc237);                                    /* 73E6 */
        M(0xc237)++;                                         /* 73E7 */
        sub_31e9(z);                                         /* 73E8 */
        z->a = 0;                                            /* 73EB */
    }
    M(0xc238) = z->a;                                        /* 73EC */
    IXB(0x08) = z->a;                                        /* 73EF */
    sub_31ce(z);                                             /* 73F2 */
    sub_4fc5(z);                                             /* 73F5 */
    z->a = IYB(0x04);                                        /* 73F8 */
    SET_HL(z, 0xc24e);                                       /* 73FB */
    if (z->a == 0x0d)                                        /* 73FE-7400 */
        M(0xc24e) |= 0x40;                                   /* 7402 */
    z->a = (uint8_t)(z->a + 0x22);                           /* 7404 */
    sub_13fc(z);                                             /* 7406 */
}

/* 0x7412: tipo 4, fase 0: a 20 fotogrammi passa alla fase 2. */
void sub_7412(struct z80 *z)
{
    z->a = IXB(0x08);                                        /* 7412 */
    if (z->a < 0x20)                                         /* 7415-7417 */
        return;
    IYB(0x1d) = 0x02;                                        /* 7418 */
}

/* Velocità (D, E) della tabella 7468 per il tipo (IY+4) - 0A:
 * `ld hl,7468; rst 8; rst 10h`. */
static void velocita_7468(struct z80 *z)
{
    z->a = (uint8_t)(IYB(0x04) - 0x0a);                      /* 741D-7420 */
    SET_HL(z, TAB_7468);                                     /* 7422 */
    sub_0008(z);                                             /* rst 8 */
    sub_0010(z);                                             /* rst 10h */
}

/* 0x7456 (statica, coda di 741D e 7470): fase 6 e velocità azzerate
 * ((IY+0C), (IY+13), (IY+0B), (IY+12)). */
static void coda_7456(struct z80 *z)
{
    IYB(0x1d) = 0x06;                                        /* 7456 */
    z->a = 0;                                                /* 745A */
    IYB(0x0c) = z->a;
    IYB(0x13) = z->a;
    IYB(0x0b) = z->a;
    IYB(0x12) = z->a;                                        /* 7464 */
}

/* 0x741D: tipo 4, fase 1: velocità in X (E della tabella 7468) col segno
 * del bit 3 di (IX+1) in (IY+0F) (invertito se discorde), bit 2 di (IX+1)
 * dal segno di (IX+11), velocità Y (D) in (IY+8); poi 7456. */
void sub_741d(struct z80 *z)
{
    velocita_7468(z);
    z->a = z->e;                                             /* 7427 */
    if (!(IXB(0x01) & 0x08))                                 /* 7428-742C */
        z->a = (uint8_t)-z->a;                               /* 742E */
    z->c = z->a;                                             /* 7430 */
    z->a ^= IYB(0x0f);                                       /* 7431 */
    if (z->a & 0x80) {                                       /* 7434-7435 rlca */
        z->a = (uint8_t)-z->c;                               /* 7437-7438 */
        IYB(0x0f) = z->a;                                    /* 743A */
    }
    IXB(0x01) &= (uint8_t)~0x04;                             /* 743D */
    z->a = IXB(0x11);                                        /* 7441 */
    if (z->a & 0x80)                                         /* 7444-7445 */
        IXB(0x01) |= 0x04;                                   /* 7448 */
    z->a ^= z->d;                                            /* 744C */
    uint8_t negativo = z->a & 0x80;                          /* 744D rlca */
    z->a = z->d;                                             /* 744E */
    if (!negativo)                                           /* 744F */
        z->a = (uint8_t)-z->a;                               /* 7451 */
    IYB(0x08) = z->a;                                        /* 7453 */
    coda_7456(z);
}

/* 0x7470: tipo 4, fase 2: come 741D con gli assi scambiati (D in (IY+8)
 * col bit 2 di (IX+1), E in (IY+0F) dal segno di (IX+13) col bit 3). */
void sub_7470(struct z80 *z)
{
    velocita_7468(z);
    z->a = z->d;                                             /* 747A */
    if (!(IXB(0x01) & 0x04))                                 /* 747B-747F */
        z->a = (uint8_t)-z->a;                               /* 7481 */
    z->c = z->a;                                             /* 7483 */
    z->a ^= IYB(0x08);                                       /* 7484 */
    if (z->a & 0x80) {                                       /* 7487-7488 */
        z->a = (uint8_t)-z->c;                               /* 748A-748B */
        IYB(0x08) = z->a;                                    /* 748D */
    }
    IXB(0x01) &= (uint8_t)~0x08;                             /* 7490 */
    z->a = IXB(0x13);                                        /* 7494 */
    if (z->a & 0x80)                                         /* 7497-7498 */
        IXB(0x01) |= 0x08;                                   /* 749B */
    z->a ^= z->e;                                            /* 749F */
    uint8_t negativo = z->a & 0x80;                          /* 74A0 rlca */
    z->a = z->e;                                             /* 74A1 */
    if (!negativo)                                           /* 74A2 */
        z->a = (uint8_t)-z->a;                               /* 74A4 */
    IYB(0x0f) = z->a;                                        /* 74A6 */
    coda_7456(z);                                            /* 74A9 */
}

/* Parola della tabella HL scelta da 759F e dal bit `bit` di A: E = 0 o 2
 * (D = 0 dopo 2548), HL += DE, poi `rst 10h` (parola in DE). */
static void parola_tabella(struct z80 *z, uint8_t bit)
{
    sub_759f(z);                                             /* 74F8 / 7539 */
    z->e = z->d;                                             /* 74FD */
    if (z->a & bit)
        z->e = 0x02;                                         /* 7500 */
    SET_HL(z, HL(z) + DE(z));                                /* 7502 */
    sub_0010(z);                                             /* 7503 */
}

/* 0x74AB: tipo 4, fase 6: direzione (IY+1E) dai segni delle velocità
 * (IY+0C), (IY+13); poi, per ogni asse, 756C misura la distanza (da IY+8
 * per X, da IY+0F per Y): con riporto inverte la velocità e accende il bit
 * 0 (1) di (IX+1) la prima volta; senza, 758E decide se spegnere il bit e
 * leggere da 754C (755C) la nuova parola in (IY+9) e (IY+0A) ((IY+10) e
 * (IY+11)). */
void sub_74ab(struct z80 *z)
{
    z->c = 0;                                                /* 74AB */
    if (IYB(0x0c) & 0x80)                                    /* 74AD-74B1 */
        z->c++;                                              /* 74B3 */
    if (IYB(0x13) & 0x80)                                    /* 74B4-74B8 */
        z->c |= 0x02;                                        /* 74BA */
    z->a = (uint8_t)((z->c & 0x02) >> 1);                    /* 74BC-74BF */
    IYB(0x1e) = z->a;                                        /* 74C0 */
    z->b = IYB(0x04);                                        /* 74C3 */
    SET_HL(z, z->iy);                                        /* 74C7-74C9 */
    uint16_t bc = BC(z);                                     /* 74C6 push bc */
    SET_BC(z, 0x0008);                                       /* 74CA */
    sub_756c(z);                                             /* 74CD */
    IXB(0x10) = z->e;                                        /* 74D0 */
    IXB(0x11) = z->d;                                        /* 74D3 */
    SET_BC(z, bc);                                           /* 74D6 pop bc */
    if (NEMICI_RIPORTO_ACCESO(z)) {                          /* 74D7 */
        if (!(IXB(0x01) & 0x01)) {                           /* 74D9-74DD */
            z->a = (uint8_t)-IYB(0x08);                      /* 74DF-74E0 */
            IYB(0x08) = z->a;                                /* 74E3 */
            IXB(0x01) |= 0x01;                               /* 74E6 */
        }
    } else {
        sub_758e(z);                                         /* 74EC */
        if (!NEMICI_RIPORTO_ACCESO(z)) {                     /* 74EF */
            IXB(0x01) &= (uint8_t)~0x01;                     /* 74F1 */
            SET_HL(z, TAB_754C);                             /* 74F5 */
            parola_tabella(z, 0x01);
            IYB(0x09) = z->e;                                /* 7504 */
            IYB(0x0a) = z->d;                                /* 7507 */
        }
    }
    bc = BC(z);                                              /* 750A push bc */
    SET_HL(z, z->iy);                                        /* 750B-750D */
    SET_BC(z, 0x000f);                                       /* 750E */
    sub_756c(z);                                             /* 7511 */
    IXB(0x12) = z->e;                                        /* 7514 */
    IXB(0x13) = z->d;                                        /* 7517 */
    SET_BC(z, bc);                                           /* 751A pop bc */
    if (NEMICI_RIPORTO_ACCESO(z)) {                          /* 751B */
        if (IXB(0x01) & 0x02)                                /* 751D-7521 */
            return;
        z->a = (uint8_t)-IYB(0x0f);                          /* 7522-7523 */
        IYB(0x0f) = z->a;                                    /* 7526 */
        IXB(0x01) |= 0x02;                                   /* 7529 */
        return;
    }
    sub_758e(z);                                             /* 752E */
    if (NEMICI_RIPORTO_ACCESO(z))                            /* 7531 */
        return;
    IXB(0x01) &= (uint8_t)~0x02;                             /* 7532 */
    SET_HL(z, TAB_755C);                                     /* 7536 */
    parola_tabella(z, 0x02);
    IYB(0x10) = z->e;                                        /* 7545 */
    IYB(0x11) = z->d;                                        /* 7548 */
}
