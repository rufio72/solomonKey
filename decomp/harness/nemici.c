/* Motore dei nemici, lotto N1 (brief 25): corpo dell'attività 0A (4DA2),
 * le tre fasi smistate da c2b8 (4DAD, 4632, 4EE6), il gestore per tipo
 * 4F0F e la voce nulla 3B62; qui stanno anche le variabili e le funzioni
 * che registrano le routine mancanti (nemici.h).
 * Lotto N7 (brief 33): in coda 4649 e 46A3 (le due vie di 4632 a 4644),
 * 470A, 4717 e 4767 (condivisa con 37B4, non tradotta).
 *
 * Strutture: 17 slot da 0x18 byte a c2c1 (IX) e i 17 oggetti da 0x30 byte
 * corrispondenti a c4fe (IY). c2b8 è la fase del corpo 0A (0, 1, 2, poi di
 * nuovo 0). Tradotto da out/main64.dasm; le tabelle dei `rst 38h`/`rst 20h`
 * vengono da rst38_tables.h e rst20_tables.h. */
#include <stddef.h>
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "rst38_tables.h"
#include "rst20_tables.h"

#define FASE_C2B8      0xc2b8   /* fase del corpo 0A (indice del rst 38 di 4DA6) */
#define SLOT_C2C1      0xc2c1   /* primo slot nemico, passo 0x18 */
#define OGGETTI_C4FE   0xc4fe   /* primo oggetto nemico, passo 0x30 */
#define SLOT_PASSO     0x18
#define OGGETTO_PASSO  0x30
#define NUM_SLOT       0x11

/* ------------------------------------------------------------------ */
/* Registro delle routine mancanti e degli indici fuori tabella. */

uint16_t nemici_mancante;
uint16_t nemici_fuori_sito;
uint8_t nemici_fuori_indice;

void nemici_manca(uint16_t ingresso)
{
    if (!nemici_mancante)
        nemici_mancante = ingresso;
}

void nemici_fuori_tabella(uint16_t sito, uint8_t idx)
{
    if (!nemici_fuori_sito) {
        nemici_fuori_sito = sito;
        nemici_fuori_indice = idx;
    }
}

/* ------------------------------------------------------------------ */
/* 0x3B62: un `ret` solo, la voce nulla di molte tabelle rst 38. */
void sub_3b62(struct z80 *z)
{
    (void)z;
}

/* ------------------------------------------------------------------ */
/* 0x4F0F: gestore per tipo dell'oggetto IY (slot IX). Con (IY+4) < 6
 * ritorna (riporto acceso); altrimenti smista con `rst 38h` a 4F1B
 * sull'indice (IY+3) - 6 (18 voci). */
static void salta_4f1b(struct z80 *z, uint16_t dest)
{
    switch (dest) {
    case 0x4F40: sub_4f40(z); break;
    case 0x3B62: sub_3b62(z); break;
    case 0x73A3: sub_73a3(z); break;
    case 0x7288: sub_7288(z); break;
    case 0x4FD4: sub_4fd4(z); break;
    case 0x5CFB: sub_5cfb(z); break;
    case 0x6ECA: sub_6eca(z); break;
    case 0x57CD: sub_57cd(z); break;
    case 0x714B: sub_714b(z); break;
    case 0x56E9: sub_56e9(z); break;
    case 0x55B2: sub_55b2(z); break;
    case 0x54D8: sub_54d8(z); break;
    case 0x6CE8: sub_6ce8(z); break;
    case 0x6C0B: sub_6c0b(z); break;
    case 0x6B43: sub_6b43(z); break;
    case 0x7F68: sub_7f68(z); break;
    default:     nemici_manca(dest); break;
    }
}

void sub_4f0f(struct z80 *z)
{
    z->d = 0x06;                               /* 4F0F */
    z->a = M(z->iy + 0x04);                    /* 4F11 */
    if (z->a < z->d) {                         /* 4F14 cp d; ret c */
        NEMICI_PONI_RIPORTO(z, 1);
        return;
    }
    z->a = (uint8_t)(M(z->iy + 0x03) - z->d);  /* 4F16-4F19 */
    z->e = z->a;                               /* 4F1A */
    uint16_t dest = nemici_rst38(z, 0x4F1B, rst38_4F1B,
                                 sizeof rst38_4F1B / sizeof rst38_4F1B[0]);
    NEMICI_PONI_RIPORTO(z, 0);                 /* rst 38: add hl,de spegne il riporto (letto da 6CE8) */
    if (dest)
        salta_4f1b(z, dest);                   /* 4F1B rst 38 */
}

/* ------------------------------------------------------------------ */
/* 0x4DAD, fase 0: sposta i 17 slot di c2b7 (sub_249e da c2c9, passo 0x18),
 * conta in c2af gli slot con i bit 7 e 4 accesi (23 slot da c2c1), poi
 * sub_4e87, sub_4e31 e, se c2b5 non è nullo, sub_4dee. Fase successiva. */
static void sub_4dad(struct z80 *z)
{
    z->a = M(0xc2b7);                          /* 4DAD-4DB0 */
    M(0xc2b7) = 0x00;                          /* 4DB1 */
    z->c = z->a;                               /* 4DB3 */
    SET_HL(z, 0xc2c9);                         /* 4DB6 */
    SET_DE(z, 0x0013);                         /* 4DB9 */
    for (z->b = NUM_SLOT; z->b; z->b--)
        sub_249e(z);                           /* 4DBC-4DBF djnz */
    uint16_t hl = SLOT_C2C1;                   /* 4DC1 */
    z->c = 0x00;                               /* 4DC4 ld bc,1700 */
    for (z->b = 0x17; z->b; z->b--) {
        if ((M(hl) & 0x80) && (M(hl) & 0x10))  /* 4DC7-4DCD */
            z->c++;                            /* 4DCF */
        hl += SLOT_PASSO;                      /* 4DD0-4DD4 */
    }
    SET_HL(z, hl);
    SET_DE(z, SLOT_PASSO);
    z->a = z->c;                               /* 4DD6 */
    M(0xc2af) = z->a;                          /* 4DD7 */
    sub_4e87(z);                               /* 4DDA */
    sub_4e31(z);                               /* 4DDD */
    SET_HL(z, 0xc2b5);                         /* 4DE0 */
    z->a = M(0xc2b5);                          /* 4DE3 */
    if (z->a)                                  /* 4DE4 or a */
        sub_4dee(z);                           /* 4DE5 call nz */
    M(FASE_C2B8)++;                            /* 4DE8-4DEC pop hl, inc (hl) */
}

/* 0x4632, fase 1: fase successiva; se c459 ha il bit 5 e uno dei bit 6-7,
 * smista con `rst 20h` a 4644 sui bit 0-2 (46A3, 4649). */
static void salta_4644(struct z80 *z, uint16_t dest)
{
    switch (dest) {
    case 0x46A3: sub_46a3(z); break;
    case 0x4649: sub_4649(z); break;
    default:     nemici_manca(dest); break;
    }
}

static void sub_4632(struct z80 *z)
{
    SET_HL(z, FASE_C2B8);                      /* 4632 */
    M(FASE_C2B8)++;                            /* 4635; 4636 pop hl */
    z->a = M(0xc459);                          /* 4637 */
    z->c = z->a;                               /* 463A */
    if (!(z->c & 0x20))                        /* 463B bit 5,c; ret z */
        return;
    if (!(z->a & 0xc0))                        /* 463E and 0c0h; ret z */
        return;
    z->a = z->c & 0x07;                        /* 4641-4642 */
    uint16_t dest = nemici_rst20(z, 0x4644, rst20_4644,
                                 sizeof rst20_4644 / sizeof rst20_4644[0]);
    if (dest)
        salta_4644(z, dest);                   /* 4644 rst 20 */
}

/* Punto di sincronia per la NMI del replay (nemici.h, brief 26). */
void (*nemici_punto)(uint16_t da, uint16_t a);

/* 0x4EE6, fase 2: per ognuno dei 17 slot (IX da c2c1) con l'oggetto (IY da
 * c4fe) a (IY+0) >= 0xc0 chiama il gestore per tipo 4F0F; poi la fase
 * torna a 0. */
static void sub_4ee6(struct z80 *z)
{
    z->ix = SLOT_C2C1;                         /* 4EE6 */
    z->iy = OGGETTI_C4FE;                      /* 4EEA */
    for (z->b = NUM_SLOT; z->b; z->b--) {      /* 4EEE, 4F06 djnz */
        /* fra due slot: qualunque ritorno, anche dentro il gestore dello
         * slot prima della sua prima scrittura; decide l'impronta */
        NEMICI_PUNTO(0x0000, 0xffff);
        z->a = M(z->iy);                       /* 4EF0 */
        if (z->a >= 0xc0) {                    /* 4EF3-4EF5 */
            uint8_t b = z->b, c = z->c;        /* 4EF7 push bc */
            sub_4f0f(z);                       /* 4EF8 */
            z->b = b;                          /* 4EFB pop bc */
            z->c = c;
        }
        SET_DE(z, OGGETTO_PASSO);
        z->ix = (uint16_t)(z->ix + SLOT_PASSO);    /* 4EFC-4EFF */
        z->iy = (uint16_t)(z->iy + OGGETTO_PASSO); /* 4F01-4F04 */
    }
    NEMICI_PUNTO(0x0000, 0xffff);              /* dopo l'ultimo slot */
    SET_HL(z, FASE_C2B8);                      /* 4F08 pop hl; 4F09 */
    M(FASE_C2B8) = 0x00;                       /* 4F0C */
}

/* ------------------------------------------------------------------ */
/* 0A, ingresso 4DA2: smista con `rst 38h` a 4DA6 sulla fase c2b8 (4DAD,
 * 4632, 4EE6). Ogni fase termina con `pop hl; ret`: lo slot resta 0A. */
void attivita_0a_4da2(uint8_t slot)
{
    (void)slot;
    struct z80 z = {0};
    z.a = M(FASE_C2B8);                        /* 4DA2 */
    z.e = z.a;                                 /* 4DA5 */
    uint16_t dest = nemici_rst38(&z, 0x4DA6, rst38_4DA6,
                                 sizeof rst38_4DA6 / sizeof rst38_4DA6[0]);
    switch (dest) {                            /* 4DA6 rst 38 */
    case 0x4DAD: sub_4dad(&z); break;
    case 0x4632: sub_4632(&z); break;
    case 0x4EE6: sub_4ee6(&z); break;
    case 0:      break;                        /* fuori tabella, già registrato */
    default:     nemici_manca(dest); break;
    }
}

/* ------------------------------------------------------------------ */
/* Lotto N7 (brief 33): corpo 0A, vie 4649 e 46A3 di 4632 e loro aiuti. */

#define RECORD_488D    0x488d   /* ROM: tabella dei record per indice */
#define OGGETTO_48AF   0x48af   /* ROM: tabella degli oggetti per indice */
#define DATI_47C7      0x47c7   /* ROM: 4 record da 10 byte del ramo 47B0 */
#define STATO_C459     0xc459   /* bit 7 e 6: due nemici da creare; poi contatore */

/* `ld hl,tab; rst 8; rst 10h` (ultimo = 0) oppure `rst 18h` (ultimo = 1):
 * DE = parola della tabella (rst 10) oppure HL = parola e DE = HL+2 (rst 18). */
static void voce_tabella(struct z80 *z, uint16_t tab, int rst18)
{
    SET_HL(z, tab);
    sub_0008(z);
    if (rst18)
        sub_0018(z);
    else
        sub_0010(z);
}

/* 0x470A: lista terminata da FF a HL; HL = HL - (byte successivo) con la
 * aritmetica a 16 bit di neg/adc (n = 0 dà HL - 256), A = (HL). */
void sub_470a(struct z80 *z)
{
    uint8_t n = leggi(HL(z) + 1);              /* 470A-470B */
    uint8_t neg = (uint8_t)-n;                 /* 470D neg */
    unsigned somma = (unsigned)neg + z->l;     /* 470F add a,l */
    z->l = (uint8_t)somma;                     /* 4710 */
    z->h = (uint8_t)(0xff + z->h + (somma >> 8));  /* 4711-4714 adc */
    z->a = leggi(HL(z));                       /* 4715 */
}

/* 0x4717: inizializza lo slot IX con A=E5, E=03, BC=0. */
void sub_4717(struct z80 *z)
{
    z->a = 0xe5;
    z->e = 0x03;
    SET_BC(z, 0);
    sub_4af6(z);
}

/* 0x4767: A = codice, C = indice del record. Smista per intervalli di A
 * sulle tabelle 488D/48AF. Il riporto in uscita non è modellato (sempre 0,
 * come dopo `and`/`add hl,de`); fa eccezione il ramo 47B0 (dopo sub_3569). */

/* 0x481D: B = A; scrive (+1D) = 6 e (+1E) = (B & 4) >> 2 dell'oggetto C. */
static void ramo_481d(struct z80 *z)
{
    z->b = z->a;                               /* 481D */
    z->a = z->c;                               /* 481E */
    voce_tabella(z, OGGETTO_48AF, 0);          /* 481F-4823 */
    SET_HL(z, (uint16_t)(0x001d + DE(z)));     /* 4824-4827 */
    M(HL(z)) = 0x06;                           /* 4828 */
    SET_HL(z, HL(z) + 1);                      /* 482A */
    z->a = (uint8_t)((z->b & 0x04) >> 2);      /* 482B-482F */
    M(HL(z)) = z->a;                           /* 4830 */
}

/* 0x47E5 e 0x480A: azzera (+1D) e (+1E) dell'oggetto HL+1D. */
static void ramo_480a(struct z80 *z)
{
    SET_DE(z, 0x001d);                         /* 480A */
    SET_HL(z, HL(z) + DE(z));                  /* 480D */
    z->a = 0;                                  /* 480E */
    M(HL(z)) = 0;                              /* 480F */
    SET_HL(z, HL(z) + 1);                      /* 4810 */
    M(HL(z)) = 0;                              /* 4811 */
}

void sub_4767(struct z80 *z)
{
    uint8_t a = z->a;
    if (a < 0x10 || a >= 0xa4) {               /* 4767-4769, 4793-4795 -> 4798 */
        z->a = z->c;                           /* 4798 */
        voce_tabella(z, OGGETTO_48AF, 1);      /* 4799-479D */
        M(HL(z)) |= 0x01;                      /* 479E */
        SET_DE(z, 0x000a);                     /* 47A0 */
        SET_HL(z, HL(z) + DE(z));              /* 47A3 */
        z->a = 0x01;                           /* 47A4 */
        M(HL(z)) = z->a;                       /* 47A6 */
        z->e = 0x07;                           /* 47A7 */
        SET_HL(z, HL(z) + DE(z));              /* 47A9 */
        M(HL(z)) = z->a;                       /* 47AA */
        z->e = 0x0c;                           /* 47AB */
        SET_HL(z, HL(z) + DE(z));              /* 47AD */
        M(HL(z)) = z->d;                       /* 47AE */
    } else if (a < 0x16) {                     /* 47B0: record di 10 byte da 47C7 */
        uint8_t n = (a - 0x10) & 0x03;         /* 47B0-47B2 */
        z->b = (uint8_t)(2 * n);               /* 47B4-47B5 */
        SET_HL(z, (uint16_t)(DATI_47C7 + 10 * n));  /* 47B6-47C0 */
        z->a = 0x04;                           /* 47C1 */
        sub_3569(z);                           /* 47C3 */
    } else if (a < 0x1c) {                     /* 47E5 */
        z->b = a;                              /* 47E5 */
        z->a = z->c;                           /* 47E6 */
        voce_tabella(z, RECORD_488D, 1);       /* 47E7-47EB */
        M(HL(z)) &= (uint8_t)~0x40;            /* 47EC */
        SET_HL(z, HL(z) + 0x000e);             /* 47EE-47F1 */
        z->e = z->b;                           /* 47F2 */
        z->a = 0;                              /* 47F3 */
        for (z->b = 4; z->b; z->b--) {         /* 47F4-47F8 */
            M(HL(z)) = z->a;
            SET_HL(z, HL(z) + 1);
        }
        z->e--;                                /* 47FA */
        if (!(z->e & 0x04))                    /* 47FB-47FD bit 2,e; jr nz */
            z->a++;                            /* 47FF */
        M(HL(z)) = z->a;                       /* 4800 */
        z->a = z->c;                           /* 4801 */
        voce_tabella(z, OGGETTO_48AF, 0);      /* 4802-4806 */
        SET_HL(z, DE(z));                      /* 4807 ex de,hl: HL = oggetto */
        M(HL(z)) &= (uint8_t)~0x20;            /* 4808 */
        ramo_480a(z);                          /* 480A */
    } else if (a < 0x20) {                     /* 1C-1F */
        ramo_481d(z);
    } else if (a < 0x40) {                     /* 486B */
        int bit3 = (a & 0x08) != 0;            /* 486B */
        z->b = a;                              /* 486D */
        z->a = z->c;                           /* 486E */
        if (bit3) {                            /* 486F */
            voce_tabella(z, RECORD_488D, 1);   /* 4871-4875 */
            M(HL(z)) |= 0x40;                  /* 4876 */
        }
        voce_tabella(z, OGGETTO_48AF, 1);      /* 4878-487C */
        M(HL(z)) &= (uint8_t)~0x20;            /* 487D */
        SET_DE(z, 0x001d);                     /* 487F */
        SET_HL(z, HL(z) + DE(z));              /* 4882 */
        M(HL(z)) = 0;                          /* 4883 */
        SET_HL(z, HL(z) + 1);                  /* 4885 */
        z->a = (uint8_t)((z->b & 0x0c) >> 2);  /* 4886-488B */
        M(HL(z)) = z->a;                       /* 488B */
    } else if (a < 0x50) {                     /* 4813 */
        z->b = a;                              /* 4813 */
        z->a = z->c;                           /* 4814 */
        voce_tabella(z, RECORD_488D, 1);       /* 4815-4819 */
        M(HL(z)) &= (uint8_t)~0x40;            /* 481A */
        z->a = z->b;                           /* 481C */
        ramo_481d(z);
    } else if (a < 0x68) {                     /* 50-67 */
        ramo_481d(z);
    } else if (a < 0x80) {                     /* 4832 */
        z->a = z->c;                           /* 4832 */
        voce_tabella(z, OGGETTO_48AF, 0);      /* 4833-4837 */
        SET_HL(z, (uint16_t)(0x001d + DE(z))); /* 4838-483B */
        M(HL(z)) = 0x06;                       /* 483C */
    } else if (a < 0xa0) {                     /* 483F */
        z->b = a;                              /* 483F */
        z->a = z->c;                           /* 4840 */
        voce_tabella(z, RECORD_488D, 0);       /* 4841-4845 */
        SET_HL(z, (uint16_t)(0x000e + DE(z))); /* 4846-4849 */
        M(HL(z)) = z->b;                       /* 484A */
        voce_tabella(z, OGGETTO_48AF, 1);      /* 484B-484F */
        M(HL(z)) &= (uint8_t)~0x20;            /* 4850 */
        SET_DE(z, 0x001d);                     /* 4852 */
        SET_HL(z, HL(z) + DE(z));              /* 4855 */
        M(HL(z)) = 0x02;                       /* 4856 */
    } else {                                   /* 4859: A0-A3 */
        z->a = z->c;                           /* 4859 */
        voce_tabella(z, OGGETTO_48AF, 1);      /* 485A-485E */
        M(HL(z)) |= 0x04;                      /* 485F */
        SET_DE(z, 0x001d);                     /* 4861 */
        SET_HL(z, HL(z) + DE(z));              /* 4864 */
        M(HL(z)) = 0x02;                       /* 4865 */
        SET_HL(z, HL(z) + 1);                  /* 4867 */
        M(HL(z)) = 0x00;                       /* 4868 */
    }
}

/* Un nemico di 46A3: HL = slot libero di 7379 (riporto acceso), A = indice.
 * `reg` = c45b o c45d, `lista` = c2b1 o c2b3, `pos` = c45f o c461. */
static void crea_46b4(struct z80 *z, uint16_t reg, uint16_t lista, uint16_t pos)
{
    M(HL(z)) = 0xd0;                           /* 46B4 */
    SET_DE(z, reg);                            /* 46B6 */
    M(DE(z)) = z->a;                           /* 46B9 */
    z->c = z->a;                               /* 46BA */
    SET_DE(z, DE(z) + 1);                      /* 46BB */
    SET_HL(z, (uint16_t)(M(lista) | (M(lista + 1) << 8)));  /* 46BC */
    z->a = leggi(HL(z));                       /* 46BF */
    if (z->a == 0xff)                          /* 46C0 */
        sub_470a(z);                           /* 46C2 call z */
    SET_HL(z, HL(z) + 1);                      /* 46C5 */
    M(lista) = z->l;                           /* 46C6 */
    M(lista + 1) = z->h;
    M(DE(z)) = z->a;                           /* 46C9 */
    SET_HL(z, (uint16_t)(M(pos) | (M(pos + 1) << 8)));  /* 46CA */
    sub_472e(z);                               /* 46CD */
    sub_4717(z);                               /* 46D0 */
}

/* 0x46A3 (via 4644 con A = 0): per il bit 7 e poi il bit 6 di c459 prende
 * uno slot libero (sub_7379) e crea il nemico dalla lista a (c2b1)/(c2b3);
 * poi c459++. Senza slot libero: bit 7 -> c459 = 20 e fine; bit 6 -> si
 * spegne il bit 6. Il riporto fra i due `rla` è sempre 0 (dopo sub_472e
 * c'è `and 7`; con il bit 7 spento `jr nc` lo ha già azzerato). */
void sub_46a3(struct z80 *z)
{
    z->a = M(STATO_C459);                      /* 46A3 */
    if (z->a & 0x80) {                         /* 46A6-46A7 rla; jr nc */
        sub_7379(z);                           /* 46A9 */
        if (!NEMICI_RIPORTO_ACCESO(z)) {       /* 46AC jr c */
            z->a = 0x20;                       /* 46AE */
            M(STATO_C459) = z->a;              /* 46B0 */
            return;                            /* 46B3 */
        }
        crea_46b4(z, 0xc45b, 0xc2b1, 0xc45f);
    }
    uint8_t a0 = M(STATO_C459);                /* 46D3 */
    z->a = (uint8_t)((a0 << 2) | (a0 >> 7));   /* 46D6-46D7 rla; rla (cin = 0) */
    if (a0 & 0x40) {                           /* 46D8 jr nc */
        sub_7379(z);                           /* 46DA */
        if (!NEMICI_RIPORTO_ACCESO(z)) {       /* 46DD jr c */
            SET_HL(z, STATO_C459);             /* 46DF */
            M(STATO_C459) &= (uint8_t)~0x40;   /* 46E2 */
        } else {
            crea_46b4(z, 0xc45d, 0xc2b3, 0xc461);
        }
    }
    SET_HL(z, STATO_C459);                     /* 4705 */
    M(STATO_C459)++;                           /* 4708 */
}

/* 0x4649 (via 4644 con A = 1): se (c2a9) & 3F < 36 esce con A = (c2a9) & 3F.
 * Altrimenti, per i bit 7 e 6 di C = (c459) (HL = c45b, +2 per giro):
 * azzera 3 byte del record 488D, prende l'oggetto 48AF in IX, lo inizializza
 * (4AF6, 4722, 4767) e accende il bit 6 di (IX+0). In uscita (c459) = 20.
 * Il riporto che entra nel secondo `rl c` è sempre 0. */
void sub_4649(struct z80 *z)
{
    z->a = M(0xc2a9) & 0x3f;                   /* 4649-464C */
    if (z->a < 0x36)                           /* 464E-4650 ret c */
        return;
    z->c = M(STATO_C459);                      /* 4651-4654 */
    z->b = 0x02;                               /* 4655 */
    SET_HL(z, 0xc45b);                         /* 4657 */
    do {
        int bit7 = (z->c & 0x80) != 0;         /* 465A rl c */
        z->c = (uint8_t)(z->c << 1);
        if (bit7) {                            /* 465C jr nc */
            uint16_t hl = HL(z);               /* 465E push hl */
            uint8_t b = z->b, c = z->c;        /* 465F push bc */
            z->a = M(hl);                      /* 4660 */
            z->b = M(hl + 1);                  /* 4661-4662 */
            voce_tabella(z, RECORD_488D, 0);   /* 4663-4667: DE = record */
            SET_HL(z, (uint16_t)(0x0008 + DE(z)));  /* 4668-466B */
            M(HL(z)) = 0x00;                   /* 466C */
            SET_DE(z, 0x0003);                 /* 466E */
            SET_HL(z, HL(z) + DE(z));          /* 4671 */
            M(HL(z)) = z->d;                   /* 4672 */
            SET_HL(z, HL(z) + 1);              /* 4673 */
            M(HL(z)) = z->d;                   /* 4674 */
            voce_tabella(z, OGGETTO_48AF, 0);  /* 4675-4679: DE = oggetto */
            z->ix = DE(z);                     /* 467A-467B */
            z->d = z->a;                       /* 467D */
            z->e = z->b;                       /* 467E */
            SET_BC(z, 0x0500);                 /* 467F */
            z->a = 0xa0;                       /* 4682 */
            sub_4af6(z);                       /* 4684 */
            z->a = z->e;                       /* 4687 */
            sub_4722(z);                       /* 4688 */
            M(z->ix + 3) = z->a;               /* 468B */
            z->a = z->e;                       /* 468E */
            z->c = z->d;                       /* 468F */
            sub_4767(z);                       /* 4690 */
            z->b = b;                          /* 4693 pop bc */
            z->c = c;
            SET_HL(z, hl);                     /* 4694 pop hl */
            M(z->ix) |= 0x40;                  /* 4695 */
        }
        SET_HL(z, HL(z) + 2);                  /* 4699-469A */
    } while (--z->b);                          /* 469B djnz */
    z->a = 0x20;                               /* 469D */
    M(STATO_C459) = z->a;                      /* 469F */
}
