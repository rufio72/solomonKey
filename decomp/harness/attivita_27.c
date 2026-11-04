/* Corpo dell'attività 27 (62B6) del gioco libero, lotto 55 (brief 55), la
 * routine 6379 e la voce vuota 3B61 dei codici 1E e 21, tradotti dal
 * disassemblato out/main64.dasm.
 *
 * Il 27 è postato solo da 017B (NMI con il bit 0 di e603 acceso): la
 * schermata dei contatori di partita. L'indice dei passi è c1ae (non
 * c19d) e l'ingresso non lo avanza (`ld a,(c1ae); rst 20h`): lo fa il
 * passo 0. Entrambi i passi durano più di un frame (75AA, azzeramento di
 * d400-e3ff): non catturabili, verificati per composizione (6379, 673F,
 * 75AA, 3A1C, 4D37, 4D87 nel banco). */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"

/* 0x6379: B righe di contatori, una per byte (IY+0) con IY che avanza:
 * cifre (4D37 sul valore a 16 bit 00:(IY+0)) a HL con attributo 70 se il
 * piano basso (HL - 400) è a zero, 50 altrimenti (4D87); HL scende di una
 * riga (+20). Uscita: B = 0, C quello d'ingresso (push/pop), DE = 20,
 * HL dopo l'ultima riga, IY dopo l'ultimo byte, A dell'ultima 4D87. */
void sub_6379(struct z80 *z)
{
    do {
        uint16_t bc = BC(z);                   /* 6379 push bc */
        uint16_t hl = HL(z);                   /* 637A push hl */
        z->l = M(z->iy);                       /* 637B ld l,(iy+0) */
        z->iy++;                               /* 637E */
        z->h = 0x00;                           /* 6380 */
        z->d = z->h;                           /* 6382 */
        z->e = z->h;                           /* 6383 */
        sub_4d37(z);                           /* 6384 */
        SET_HL(z, hl);                         /* 6387 pop hl; 6388 push hl */
        z->h &= (uint8_t)~0x04;                /* 6389 res 2,h */
        z->a = M(HL(z));                       /* 638B */
        z->c = 0x70;                           /* 638C */
        if (z->a != 0)                         /* 638E-638F */
            z->c = 0x50;                       /* 6391 */
        z->h |= 0x04;                          /* 6393 set 2,h */
        SET_DE(z, 0xc061);                     /* 6395 */
        sub_4d87(z);                           /* 6398 */
        SET_HL(z, hl);                         /* 639B pop hl */
        SET_BC(z, bc);                         /* 639C pop bc */
        SET_DE(z, 0x0020);                     /* 639D */
        SET_HL(z, HL(z) + DE(z));              /* 63A0 */
    } while (--z->b != 0);                     /* 63A1 djnz 6379 */
}

/* Azzera i 4096 byte da d400 (B = 0, C = 10: 16 giri di 256). */
static void azzera_d400(struct z80 *z)
{
    SET_HL(z, 0xd400);
    SET_BC(z, 0x0010);
    z->a = 0x00;
    do {
        do {
            M(HL(z)) = z->a;
            SET_HL(z, HL(z) + 1);
        } while (--z->b != 0);
    } while (--z->c != 0);
}

/* Passo 0, 62E0: tabella delle attività vuota (673F), 75AA, schermo
 * d400-e3ff a zero, d180-d19f e d2c0-d2df ad attributo 50, sette gruppi di
 * contatori (6379) e la colonna dei numeri di stanza 19..01 a d764 verso
 * l'alto (3A1C, due cifre per riga); c1ae + 1 e slot 27. */
static void passo_62e0(uint8_t slot)
{
    static const struct { uint16_t hl, iy; uint8_t b; } gruppi[7] = {
        { 0xd47c, 0xc215, 0x19 },              /* 6307-6310 */
        { 0xd478, 0xc1fc, 0x19 },              /* 6313-631C */
        { 0xd473, 0xc1e3, 0x19 },              /* 631F-6328 */
        { 0xd46f, 0xc1ca, 0x19 },              /* 632B-6334 */
        { 0xd6aa, 0xc233, 0x01 },              /* 6337-6340 */
        { 0xd6ea, 0xc22e, 0x05 },              /* 6343-634C */
        { 0xd46a, 0xc1ba, 0x10 },              /* 634F-6358 */
    };
    struct z80 z = {0};
    sub_673f(&z);                              /* 62E0 */
    sub_75aa(&z);                              /* 62E3 */
    azzera_d400(&z);                           /* 62E6-62F2 */
    SET_HL(&z, 0xd180);                        /* 62F4 */
    SET_BC(&z, 0x2050);                        /* 62F7 */
    do {
        M(HL(&z)) = z.c;                       /* 62FA */
        SET_HL(&z, HL(&z) + 1);                /* 62FB */
    } while (--z.b != 0);                      /* 62FC djnz */
    SET_HL(&z, 0xd2c0);                        /* 62FE */
    z.b = 0x20;                                /* 6301 */
    do {
        M(HL(&z)) = z.c;                       /* 6303 */
        SET_HL(&z, HL(&z) + 1);                /* 6304 */
    } while (--z.b != 0);                      /* 6305 djnz */
    for (int i = 0; i < 7; i++) {
        SET_HL(&z, gruppi[i].hl);
        z.iy = gruppi[i].iy;
        z.b = gruppi[i].b;
        sub_6379(&z);
    }
    SET_DE(&z, 0xd764);                        /* 635B */
    z.b = 0x19;                                /* 635E */
    do {
        uint16_t riga = DE(&z);                /* 6360 push de */
        z.a = z.b;                             /* 6361 */
        sub_3a1c(&z);                          /* 6362 */
        SET_HL(&z, riga);                      /* 6365 pop hl */
        M(HL(&z)) = z.e;                       /* 6366 */
        SET_HL(&z, HL(&z) - 1);                /* 6367 */
        M(HL(&z)) = z.d;                       /* 6368 */
        SET_HL(&z, HL(&z) + 1);                /* 6369 */
        SET_DE(&z, 0xffe0);                    /* 636A */
        SET_HL(&z, HL(&z) + DE(&z));           /* 636D */
        SET_DE(&z, HL(&z));                    /* 636E ex de,hl */
        SET_HL(&z, 0xffe0);
    } while (--z.b != 0);                      /* 636F djnz */
    M(0xc1ae)++;                               /* 6371-6374 */
    tabella[slot] = 0x27;                      /* 6375-6376 pop hl; ld (hl),27 */
}

/* Passo 1, 62BE: aspetta il bit 0 di e603 spento; poi bit 7 di c045
 * spento, c1ae = 0, slot 06 (la ripresa) e schermo d400-e3ff a zero. Con
 * il bit acceso `pop hl; ret c`: slot invariato. */
static void passo_62be(uint8_t slot)
{
    struct z80 z = {0};
    z.a = leggi_e603(0x62be);                  /* 62BE */
    if (z.a & 0x01)                            /* 62C1-62C3 rrca; pop hl; ret c */
        return;
    M(0xc045) &= (uint8_t)~0x80;               /* 62C5-62C8 */
    z.a = 0x00;                                /* 62CA */
    M(0xc1ae) = z.a;                           /* 62CB */
    tabella[slot] = 0x06;                      /* 62CE-62D0 ld (de),a: DE = slot */
    azzera_d400(&z);                           /* 62D1-62DD */
}

/* 27, ingresso 62B6: passo da c1ae (`rst 20h` 62B9, 2 voci). */
void attivita_27_62b6(uint8_t slot)
{
    uint8_t passo = M(0xc1ae);                 /* 62B6 */
    switch (passo) {                           /* 62B9 rst 20h */
    case 0: passo_62e0(slot); break;
    case 1: passo_62be(slot); break;
    default: nemici_fuori_tabella(0x62b9, passo); break;
    }
}

/* 3B61 (codici 1E e 21): `pop hl; ret`, lo slot non cambia. */
void attivita_vuota_3b61(uint8_t slot)
{
    (void)slot;
}
