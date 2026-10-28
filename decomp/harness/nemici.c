/* Motore dei nemici, lotto N1 (brief 25): corpo dell'attività 0A (4DA2),
 * le tre fasi smistate da c2b8 (4DAD, 4632, 4EE6), il gestore per tipo
 * 4F0F e la voce nulla 3B62; qui stanno anche le variabili e le funzioni
 * che registrano le routine mancanti (nemici.h).
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
