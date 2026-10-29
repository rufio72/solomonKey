/* Motore dei nemici, lotto N1 (brief 25): routine in comune fra il corpo
 * 0A e i corpi 01 (48D1) e 10 (457B). Temi: coordinate di cella e di
 * schermo (2482, 248D, 24BE, 4C11), presa di un blocco della griglia c0bd
 * da parte di un oggetto (4592, 4938, 49AE), contatto fra due oggetti
 * (4B20). Ingressi: 2482, 248D, 24BE, 4592, 4938, 49AE, 4B20, 4C11.
 *
 * Tradotto da out/main64.dasm. La tabella ROM 9495 ha 8 byte per tipo di
 * oggetto (+0 altezza, +1 e +2 scostamenti in X, +3 larghezza: nomi
 * provvisori, dall'uso). La griglia c0bd ha un byte per cella. */
#include "routine.h"
#include "nemici.h"

#define GRIGLIA_C0BD   0xc0bd
#define SAGOME_9495    0x9495   /* ROM: 8 byte per tipo */
#define PIANO_D400     0xd400   /* piano dei caratteri; attributi 0x400 sotto */
#define POSA_C08A      0xc08a

/* Indirizzo della sagoma del tipo: (IX+4), o (IX+3) se (IX+4) >= 9. */
static uint16_t sagoma(uint16_t ix)
{
    uint8_t tipo = M(ix + 0x04);
    if (tipo >= 0x09)
        tipo = M(ix + 0x03);
    return (uint16_t)(SAGOME_9495 + 8 * tipo);
}

/* ------------------------------------------------------------------ */
/* 0x2482: HL = 32·D + E (posizione nel piano), D = 0, A = 0. */
void sub_2482(struct z80 *z)
{
    uint16_t hl = (uint16_t)(z->d * 32 + z->e);   /* 2482-248B */
    z->a = 0;
    z->d = 0;
    SET_HL(z, hl);
}

/* 0x248D: scrive C nel blocco 2×2 a HL (HL, HL+1, HL+20, HL+21).
 * Uscita: HL = HL+21, DE = 001f, A = 1f. */
void sub_248d(struct z80 *z)
{
    uint16_t hl = HL(z);
    M(hl) = z->c;                              /* 248D */
    M(hl + 1) = z->c;                          /* 248E-248F */
    M(hl + 0x20) = z->c;                       /* 2490-249A */
    M(hl + 0x21) = z->c;                       /* 249B-249C */
    z->a = 0x1f;
    SET_DE(z, 0x001f);
    SET_HL(z, hl + 0x21);
}

/* 0x24BE: da coordinate di schermo a cella: D = (-D) >> 3 - 2,
 * E = A = E >> 3. */
void sub_24be(struct z80 *z)
{
    z->d = (uint8_t)((((uint8_t)-z->d) >> 3) - 2);  /* 24BE-24CF */
    z->e = (uint8_t)(z->e >> 3);               /* 24D0-24DD */
    z->a = z->e;
}

/* 0x4C11: scrive il carattere B nel blocco 2×2 della cella D (riga),
 * E (colonna) del piano d400 e l'attributo C nel piano sotto (sub_248d).
 * AF, HL e DE conservati, BC invariato. */
void sub_4c11(struct z80 *z)
{
    uint8_t a = z->a, f = z->f;                /* 4C11 push af */
    uint16_t hl0 = HL(z), de0 = DE(z);         /* 4C12-4C13 */
    sub_2482(z);                               /* 4C14 */
    uint16_t hl = (uint16_t)(HL(z) + PIANO_D400);  /* 4C17-4C1A */
    M(hl) = z->b;                              /* 4C1B */
    M(hl + 1) = z->b;                          /* 4C1D */
    M(hl + 0x20) = z->b;                       /* 4C22 */
    M(hl + 0x21) = z->b;                       /* 4C24 */
    SET_HL(z, hl + 0x21 - 0x421);              /* 4C25-4C28 add hl,fbdf */
    sub_248d(z);                               /* 4C29 */
    SET_DE(z, de0);                            /* 4C2C-4C2E */
    SET_HL(z, hl0);
    z->a = a;
    z->f = f;
}

/* ------------------------------------------------------------------ */
/* 0x4592: l'oggetto IY tenta di prendere il blocco sotto di sé (IX =
 * oggetto che lo rappresenta). Esce col riporto acceso se (IY+0C) ha il
 * bit 7, se nessuna delle due celle sotto i suoi bordi ha il bit 7, o se
 * la cella scelta vale >= f8. Altrimenti, con il bit 6 della cella spento,
 * la segna (bit 6), piazza IX sulla cella e la ridisegna (tile 18,
 * attributo 20); con il bit 6 acceso c08a = 4 e passa a sub_4938. In
 * questi due casi esce col riporto spento. */
void sub_4592(struct z80 *z)
{
    z->a = M(z->iy + 0x0c);                    /* 4592 */
    if (z->a & 0x80) {                         /* 4595-4597 */
        NEMICI_PONI_RIPORTO(z, 1);             /* 45F7 scf; ret */
        return;
    }
    uint16_t hl = (uint16_t)(sagoma(z->iy) + 1);   /* 4599-45AD */
    z->a = M(z->iy + 0x15);                    /* 45AE */
    z->c = z->a;                               /* 45B1 */
    z->a = (uint8_t)(z->a + leggi(hl));        /* 45B2 */
    z->b = z->a;                               /* 45B3 */
    z->a = (uint8_t)(z->a + leggi(hl + 1));    /* 45B4-45B5 */
    z->l = z->a;                               /* 45B6 */
    z->h = (uint8_t)(0x1a + M(z->iy + 0x0e));  /* 45B7-45BC */
    sub_36f0(z);                               /* 45BD */
    uint8_t cella1 = (uint8_t)(M(HL(z)) >> 7); /* 45C0 rla */
    z->l = z->b;                               /* 45C2 */
    uint8_t b = cella1;                        /* 45C3-45C7 rl b; and b */
    if (z->c & 0x08)                           /* 45C8-45CC */
        b |= 0x80;
    z->b = b;                                  /* 45CE */
    z->c = z->e;                               /* 45CF */
    z->h = (uint8_t)(M(z->iy + 0x0e) + 0x1a);  /* 45D0-45D5 */
    sub_36f0(z);                               /* 45D6 */
    uint8_t cella2 = (uint8_t)(M(HL(z)) >> 7); /* 45D9 rla */
    if (!cella1 && !cella2) {                  /* 45DB rl b; jr z */
        z->a = (uint8_t)(M(HL(z)) << 1);       /* 45DA rla: A resta così (riporto di 36F0 = 0) */
        z->b = (uint8_t)((b << 1) | cella2);
        NEMICI_PONI_RIPORTO(z, 1);
        return;
    }
    /* 45DF rr b: B torna com'era, riporto = cella2 */
    if (cella2 && (z->b & 0x80)) {
        /* 45E3: cella sotto il secondo bordo, E invariato */
    } else if (z->b & 0x01) {
        z->e = z->c;                           /* 45EB */
    }
    z->d = 0;
    hl = (uint16_t)(GRIGLIA_C0BD + z->e);      /* 45EC-45F1 */
    SET_HL(z, hl);
    z->a = M(hl);                              /* 45F2 */
    if (z->a >= 0xf8) {                        /* 45F3-45F5 */
        NEMICI_PONI_RIPORTO(z, 1);
        return;
    }
    SET_BC(z, hl);                             /* 45F9-45FA */
    z->d = z->a;                               /* 45FB */
    z->a = z->e;                               /* 45FC */
    sub_4b0e(z);                               /* 45FD */
    z->a = z->d;                               /* 4600 */
    if (z->a & 0x40) {                         /* 4601-4603 */
        SET_HL(z, POSA_C08A);                  /* 4626 */
        M(POSA_C08A) = 0x04;                   /* 4629 */
        z->b = 0x00;                           /* 462B */
        sub_4938(z);                           /* 462D */
        NEMICI_PONI_RIPORTO(z, 0);             /* 4630 or a */
        return;
    }
    M(z->ix + 0x0e) = z->h;                    /* 4605 */
    M(z->ix + 0x15) = z->l;                    /* 4608 */
    z->a |= 0x40;                              /* 460B */
    M(BC(z)) = z->a;                           /* 460D */
    z->b = 0x03;                               /* 460E */
    z->c = 0x02;                               /* 4610 */
    z->d = z->e;                               /* 4612 */
    z->e = 0x01;                               /* 4613 */
    z->a = 0xc4;                               /* 4615 */
    sub_4af6(z);                               /* 4617 */
    z->e = z->d;                               /* 461A */
    sub_2457(z);                               /* 461B */
    SET_BC(z, 0x1820);                         /* 461E */
    sub_3a54(z);                               /* 4621 */
    NEMICI_PONI_RIPORTO(z, 0);                 /* 4624 or a */
}

/* ------------------------------------------------------------------ */
/* 0x4938: posa l'oggetto IX sulla cella E di valore A. Con A in 80-f7 la
 * cella perde i bit 6-7 e si ridisegna (carattere dalla tabella di
 * sub_3a2e, o carattere 0 con 4C11 se la cella vale 37); altrimenti
 * (bit 7 di B acceso) l'oggetto si sposta di 8 secondo i bit 0-1 di B.
 * Poi sub_4af6 con A = c7, E = 1, B = c08a (1 col bit 7), C = direzione. */
void sub_4938(struct z80 *z)
{
    if ((uint8_t)(z->a - 0x80) >= 0x78) {      /* 4938-493C */
        z->b |= 0x80;                          /* 493E */
    } else {
        z->d = 0x00;                           /* 4942 */
        uint16_t hl = (uint16_t)(GRIGLIA_C0BD + z->e);  /* 4944-4947 */
        z->c = M(hl) & 0x3f;                   /* 4948-494B */
        M(hl) = z->c;                          /* 494D */
        SET_HL(z, hl);
    }
    z->a = z->e;                               /* 494E */
    sub_4b0e(z);                               /* 494F */
    z->a = z->h;                               /* 4952 */
    if ((z->b & 0x80) && (z->b & 0x02))        /* 4953-4959 */
        z->a = (uint8_t)(z->a + 0x08);         /* 495B */
    z->h = z->a;                               /* 495D */
    M(z->ix + 0x0e) = z->a;                    /* 495E */
    z->a = z->l;                               /* 4961 */
    if ((z->b & 0x80) && !(z->b & 0x02)) {     /* 4962-4968 */
        z->a = (uint8_t)(z->a - 0x08);         /* 496A */
        if (z->b & 0x01)                       /* 496C-496E */
            z->a = (uint8_t)(z->a + 0x10);     /* 4970 */
    }
    M(z->ix + 0x15) = z->a;                    /* 4972 */
    z->l = z->a;                               /* 4975 */
    if (!(z->b & 0x80)) {                      /* 4976-4978 */
        uint16_t de = DE(z);                   /* 497A ex de,hl */
        SET_DE(z, HL(z));
        SET_HL(z, de);
        sub_24be(z);                           /* 497B */
        uint8_t b = z->b, c = z->c;            /* 497E push bc */
        z->a = (uint8_t)(z->c - 0x37);         /* 497F-4980 */
        if (z->a == 0) {                       /* 4982 */
            SET_BC(z, 0x0000);                 /* 4984 */
            sub_4c11(z);                       /* 4987 */
        } else {
            uint16_t de2 = DE(z);              /* 498D push de */
            z->a = z->c;                       /* 498E */
            sub_3a2e(z);                       /* 498F */
            SET_DE(z, de2);                    /* 4992 pop de */
            sub_3a54(z);                       /* 4993 */
        }
        z->b = b;                              /* 4996 pop bc */
        z->c = c;
    }
    z->a = 0x01;                               /* 4997 */
    if (!(z->b & 0x80)) {                      /* 4999-499B */
        z->a = M(POSA_C08A);                   /* 499D */
        z->b |= 0x02;                          /* 49A0 */
    }
    z->b &= 0x7f;                              /* 49A2 */
    z->c = z->b;                               /* 49A4 */
    z->b = z->a;                               /* 49A5 */
    z->e = 0x01;                               /* 49A6 */
    z->a = 0xc7;                               /* 49A8 */
    sub_4af6(z);                               /* 49AA */
}

/* ------------------------------------------------------------------ */
/* 0x4B20: contatto fra l'oggetto IX e la posizione H (Y), L (X).
 * Riporto acceso = contatto. Esce subito senza contatto se l'oggetto è di
 * tipo 6 con (IX+1D) nullo o se (IX+1) ha il bit 0. Altrimenti confronta
 * le distanze con la sagoma del tipo (9495): in verticale con +0 (o 0e se
 * l'oggetto sta sopra), in orizzontale con +1 (+ 0c), oppure, se la X
 * dell'oggetto è minore di L, con sub_2474. HL e BC conservati. */
void sub_4b20(struct z80 *z)
{
    uint16_t hl0 = HL(z);                      /* 4B20-4B22 push ix; ex (sp),hl */
    z->a = M(z->ix + 0x04);                    /* 4B23-4B27 */
    z->d = 0x00;
    z->e = M(z->ix + 0x1d);                    /* 4B28-4B2B; 4B2C pop hl */
    if (z->a == 0x06) {                        /* 4B2D-4B2F */
        z->d = z->a;                           /* 4B31 */
        z->a = z->e;                           /* 4B32 */
        if (z->a == 0) {                       /* 4B33-4B34 or a; ret z */
            NEMICI_PONI_RIPORTO(z, 0);
            return;
        }
        z->a = z->d;                           /* 4B35 */
    }
    NEMICI_PONI_RIPORTO(z, 0);                 /* 4B36 or a */
    if (M(z->ix + 0x01) & 0x01)                /* 4B37-4B3B ret nz */
        return;
    uint16_t p = sagoma(z->ix);                /* 4B3C-4B53 (C invariato) */
    z->d = (uint8_t)(hl0 >> 8);                /* 4B54 pop de: DE = HL */
    z->e = (uint8_t)hl0;
    uint8_t somma = (uint8_t)(leggi(p) + M(z->ix + 0x0e));   /* 4B55-4B59 */
    int sopra = somma >= z->d;                 /* 4B5C sub d: nessun prestito */
    z->a = (uint8_t)(somma - z->d);
    z->d = z->a;                               /* 4B5D */
    p += 3;                                    /* 4B56-4B58 */
    if (sopra) {                               /* 4B5E ccf; jr c */
        z->a = 0x0e;                           /* 4B68 */
    } else {
        z->a = (uint8_t)-z->a;                 /* 4B61 neg */
        z->d = z->a;                           /* 4B63 */
        z->a = leggi(p);                       /* 4B64 */
    }
    if (z->a < z->d) {                         /* 4B6A-4B6B */
        SET_HL(z, hl0);                        /* 4B6D or a; 4B6E pop hl */
        NEMICI_PONI_RIPORTO(z, 0);
        return;
    }
    p -= 2;                                    /* 4B70-4B71 */
    uint8_t x = M(z->ix + 0x15);               /* 4B72 */
    if (x < z->e) {                            /* 4B75-4B76 */
        z->a = (uint8_t)(x - z->e);
        SET_HL(z, p);
        sub_2474(z);                           /* 4B7F: riporto da 2474 */
        SET_HL(z, hl0);                        /* 4B82 */
        return;
    }
    z->d = (uint8_t)(x - z->e);                /* 4B78 */
    z->a = (uint8_t)(leggi(p) + z->d);         /* 4B79-4B7A */
    NEMICI_PONI_RIPORTO(z, z->a < 0x0c);       /* 4B7B cp 0ch */
    SET_HL(z, hl0);                            /* 4B7D pop hl */
}

/* ------------------------------------------------------------------ */
/* 0x49AE: posa l'oggetto IX sulla cella E di valore A (< 80) e ne decide
 * il seguito; restituisce in A il codice dell'esito (2, 4 o 1a).
 * A >= 37: l'oggetto va sulla cella; se tocca c4ce (sub_4b20) ne prende
 * la posizione (A = 4); se tocca Dana o uno degli oggetti fino a c82e
 * (c85e se c862 = a1) reagisce secondo il tipo (A = 4); altrimenti la
 * cella prende il bit 7 (perdendo il 6) e l'oggetto diventa e5 (A = 2).
 * A < 5: l'oggetto va sulla cella come e diventa c7 (A = 4).
 * A in 5-7: sceglie uno dei tre slot c24b non pieni a partire da c24a+1,
 * sub_4ada, oggetto c7 (A = 1a). A in 8-0f: sub_4ada sul valore ruotato
 * nei bit 0-1. A in 10-36 (e dopo 8-0f): (IX+0) = 43, A = 2. */
static void posa_49ae(struct z80 *z, uint8_t b, uint8_t c, uint8_t a)
{
    z->b = b;
    z->c = c;
    z->e = 0x01;
    z->a = a;
    sub_4af6(z);
}

void sub_49ae(struct z80 *z)
{
    if (z->a < 0x37) {                         /* 49AE-49B0 */
        if (z->a < 0x05) {                     /* 4A7C-4A7E */
            z->a = z->e;                       /* 4A80 */
            sub_4b0e(z);                       /* 4A81 */
            uint16_t de = DE(z);               /* 4A84 ex de,hl */
            SET_DE(z, HL(z));
            SET_HL(z, de);
            M(z->ix + 0x0e) = z->d;            /* 4A25 */
            M(z->ix + 0x15) = z->e;            /* 4A28 */
            posa_49ae(z, 0x03, 0x00, 0xc7);    /* 4A2B-4A32 */
            z->a = 0x04;                       /* 4A35 */
            return;
        }
        if (z->a < 0x10) {                     /* 4A87-4A89 */
            if (z->a < 0x08) {                 /* 4A8B-4A8D */
                z->b = z->e;                   /* 4A8F */
                z->a = M(0xc24a);              /* 4A90 */
                SET_DE(z, 0xc24b);             /* 4A93 */
                do {                           /* 4A96-4AA2: mai pieni tutti e tre */
                    z->a++;
                    if (z->a == 0x03)
                        z->a = 0x00;
                    z->l = (uint8_t)(M(0xc24b + z->a) + 1);
                    z->h = 0x00;
                } while (z->l == 0);
                M(0xc24a) = z->a;              /* 4AA4 */
                z->a = (uint8_t)(z->a + 0x05); /* 4AA7 */
                z->c = z->a;                   /* 4AA9 */
                z->a = z->b;                   /* 4AAA */
                sub_4b0e(z);                   /* 4AAB */
                M(z->ix + 0x0e) = z->h;        /* 4AAE */
                M(z->ix + 0x15) = z->l;        /* 4AB1 */
                z->e = z->b;                   /* 4AB4 */
                z->a = z->c;                   /* 4AB5 */
                sub_4ada(z);                   /* 4AB6 */
                posa_49ae(z, 0x04, 0x00, 0xc7);    /* 4AB9-4AC0 */
                z->a = 0x1a;                   /* 4AC3 */
                return;
            }
            z->c = z->a;                       /* 4AC6 */
            z->b = z->a & 0x0c;                /* 4AC7-4AC9 */
            z->a = (uint8_t)(((z->c + 1) & 0x03) | z->b);  /* 4ACA-4ACE */
            sub_4ada(z);                       /* 4ACF */
        }
        z->a = 0x43;                           /* 4AD2 */
        M(z->ix) = z->a;                       /* 4AD4 */
        z->a = 0x02;                           /* 4AD7 */
        return;
    }
    z->a = z->e;                               /* 49B3 */
    sub_4b0e(z);                               /* 49B4 */
    M(z->ix + 0x0e) = z->h;                    /* 49B7 */
    M(z->ix + 0x15) = z->l;                    /* 49BA */
    uint16_t ix0 = z->ix;                      /* 49BD push ix */
    z->ix = 0xc4ce;                            /* 49BF */
    z->c = z->e;                               /* 49C3 */
    if (M(z->ix) & 0x80) {                     /* 49C4-49C8 */
        sub_4b20(z);                           /* 49CA */
        if (NEMICI_RIPORTO_ACCESO(z)) {        /* 49CD jr c,4a1d */
            z->d = M(z->ix + 0x0e);            /* 4A1D */
            z->e = M(z->ix + 0x15);            /* 4A20 */
            z->ix = ix0;                       /* 4A23 pop ix */
            M(z->ix + 0x0e) = z->d;            /* 4A25 */
            M(z->ix + 0x15) = z->e;            /* 4A28 */
            posa_49ae(z, 0x03, 0x00, 0xc7);    /* 4A2B-4A32 */
            z->a = 0x04;                       /* 4A35 */
            return;
        }
    }
    z->a = M(0xc862);                          /* 49CF */
    z->ix = 0xc82e;                            /* 49D4 */
    z->b = 0x12;                               /* 49D8 */
    if (z->a == 0xa1) {                        /* 49D2, 49DA */
        z->ix = 0xc85e;                        /* 49DC */
        z->b++;                                /* 49E0 */
    }
    for (; z->b; z->b--) {                     /* 49E1-49F8 */
        if ((M(z->ix) & 0x80) && M(z->ix + 0x04) != 0x01) {
            sub_4b20(z);                       /* 49EE */
            if (NEMICI_RIPORTO_ACCESO(z)) {    /* 49F1 jr c,4a38 */
                z->a = M(z->ix + 0x04);        /* 4A38 */
                if (z->a >= 0xa0) {            /* 4A3B-4A3D */
                    z->a = M(z->ix + 0x1d);    /* 4A3F */
                    z->b = z->a < 0x07 ? 0x03 : 0x00;  /* 4A42-4A48 */
                    M(z->ix + 0x01) |= 0x02;   /* 4A4A */
                    z->ix = ix0;               /* 4A4E */
                    z->a = 0x04;               /* 4A50 */
                    return;
                }
                z->c = M(z->ix + 0x0e);        /* 4A53 */
                z->b = M(z->ix + 0x15);        /* 4A56 */
                z->ix = ix0;                   /* 4A59 */
                z->a = M(z->iy + 0x15);        /* 4A5B */
                M(z->ix + 0x0e) = z->c;        /* 4A5E */
                int prestito = z->a < z->b;    /* 4A61 sub b */
                z->c = 0x00;                   /* 4A62 */
                z->a = 0x08;                   /* 4A64 */
                if (prestito) {                /* 4A66 */
                    z->a = 0xf8;               /* 4A68 */
                    z->c = 0x01;               /* 4A6A */
                }
                z->a = (uint8_t)(z->a + z->b); /* 4A6C */
                M(z->ix + 0x15) = z->a;        /* 4A6D */
                posa_49ae(z, 0x02, z->c, 0xc7);    /* 4A70-4A76 */
                z->a = 0x04;                   /* 4A79 */
                return;
            }
        }
        z->ix = (uint16_t)(z->ix - 0x30);      /* 49F3-49F6 */
    }
    z->e = z->c;                               /* 49FA */
    z->d = 0x00;                               /* 49FB */
    uint16_t hl = (uint16_t)(GRIGLIA_C0BD + z->e);  /* 49FD-4A00 */
    SET_HL(z, hl);
    z->a = M(hl);                              /* 4A01 */
    if (z->a >= 0x40)                          /* 4A02-4A04 */
        z->a = (uint8_t)(z->a - 0x40);         /* 4A06 */
    z->a |= 0x80;                              /* 4A08 */
    M(hl) = z->a;                              /* 4A0A */
    z->ix = ix0;                               /* 4A0B pop ix */
    posa_49ae(z, M(POSA_C08A), 0x00, 0xe5);    /* 4A0D-4A17 */
    z->a = 0x02;                               /* 4A1A */
}
