/* Corpo dell'attività 18 (4124) del gioco libero, lotto 52 (brief 52):
 * macchina a passi sul contatore c19e (tabella `rst 20h` 4171, 10 voci),
 * con 6735 e la voce di attesa 2E79 condivisa con i corpi 25 e 26.
 * Tradotto dal disassemblato out/main64.dasm; regole come attivita_01_10.c
 * e, per i passi, come il 07 (attivita_attract07.c): HL = c19e, A = passo,
 * c19e avanza di uno; `dec (hl)` resta sullo stesso passo. */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"

/* 0x6735 (4164): cerca A nella tabella delle attività c000-c03f (cpir).
 * Uscita: HL sullo slot trovato (c03f se manca, con lo Z spento), BC =
 * byte non esaminati, bandiera Z accesa se trovato. */
void sub_6735(struct z80 *z)
{
    uint16_t hl = 0xc000, bc = 0x0040;         /* 6735-6738 */
    int trovato = 0;
    do {                                       /* 673B cpir */
        trovato = (M(hl) == z->a);
        hl++;
        bc--;
    } while (!trovato && bc != 0);
    SET_HL(z, hl - 1);                         /* 673D */
    SET_BC(z, bc);
    z->f = (uint8_t)((z->f & ~LIBERA_ZERO) | (trovato ? LIBERA_ZERO : 0));
}

/* Voce 2E79: `ld c,42h; jr 2e8b`, attesa sul contatore (2E90). */
void passo_2e79(uint16_t contatore)
{
    struct z80 z = {0};
    SET_HL(&z, contatore);
    z.c = 0x42;                                /* 2E79 */
    sub_2e90(&z);                              /* 2E8B */
}

/* Attese 41A3, 41A8, 41AD: `ld c,n; jp 2e8b` su c19e. */
static void attesa_18(uint8_t c)
{
    struct z80 z = {0};
    SET_HL(&z, 0xc19e);
    z.c = c;
    sub_2e90(&z);                              /* 2E8B */
}

/* Blocco della cella c248 (2457, 4C11 con BC = 0) e cella c247 della
 * griglia c0bd a 2 (4143-415B, 41D7-41EF). */
static void blocco_c248(struct z80 *z)
{
    z->a = M(0xc248);
    z->e = z->a;
    sub_2457(z);                               /* 4147 */
    SET_BC(z, 0x0000);
    sub_4c11(z);                               /* 414D */
    z->a = M(0xc247);
    SET_HL(z, 0xc0bd + z->a);                  /* 4153-415A */
    M(HL(z)) = 0x02;                           /* 415B */
}

/* Passo 0, 41B7: salva in c9ca i 19 byte (c4ce + 30·i) e ne spegne il
 * bit 6. */
static void passo_41b7(void)
{
    uint16_t hl = 0xc4ce, de = 0xc9ca;         /* 41B7-41BD */
    for (int i = 0; i < 0x13; i++) {           /* 41C0-41CB */
        uint8_t a = M(hl);
        M(de) = a;
        M(hl) = (uint8_t)(a & 0xbf);
        de++;
        hl = (uint16_t)(hl + 0x30);
    }
}

/* Passo 1, 41CF: resta sul passo finché c49e ha il bit 7; poi blocco di
 * c248, l'oggetto c49e alle coordinate di c248 (4B0E), c4a6 = F0, parola
 * c4a9 = 0100, stato C5 (4AF6, BC = 0402, E = 40), suono 37, c467 = 0. */
static void passo_41cf(void)
{
    struct z80 z = {0};
    if (M(0xc49e) & 0x80) {                    /* 41D0-41D5 dec (hl); ret c */
        M(0xc19e)--;
        return;
    }
    blocco_c248(&z);                           /* 41D6-41EF */
    z.a = M(0xc248);                           /* 41F1 */
    sub_4b0e(&z);                              /* 41F4 */
    z.ix = 0xc49e;                             /* 41F7 */
    M(z.ix + 0x0e) = z.h;                      /* 41FB */
    M(z.ix + 0x15) = z.l;                      /* 41FE */
    M(0xc4a6) = 0xf0;                          /* 4201-4204 */
    SET_HL(&z, 0x0100);                        /* 4206 */
    M(0xc4a9) = 0x00;                          /* 4209 */
    M(0xc4aa) = 0x01;
    SET_BC(&z, 0x0402);                        /* 420C */
    z.e = 0x40;                                /* 420F */
    z.a = 0xc5;                                /* 4211 */
    sub_4af6(&z);                              /* 4213 */
    z.c = 0x37;                                /* 4216 */
    sub_0028(&z);                              /* 4218 rst 28 */
    M(0xc467) = 0x00;                          /* 4219-421A */
}

/* Passo 5, 4185: sull'oggetto c49e la differenza in passi (2FF3) fra le
 * coordinate di c247 (HL) e di c248 (DE), suono 38, poi come 2FD3-2FF2:
 * (IX+0B/0C) = HL + 100, (IX+08) = F8, (IX+12/13) = DE, stato C0 (4AF6,
 * E = 40, BC = 0301). */
static void passo_4185(void)
{
    struct z80 z = {0};
    z.ix = 0xc49e;                             /* 4185 */
    z.a = M(0xc248);                           /* 4189 */
    sub_4b0e(&z);                              /* 418C */
    uint16_t primo = HL(&z);                   /* 418F push hl */
    z.a = M(0xc247);                           /* 4190 */
    sub_4b0e(&z);                              /* 4193 */
    SET_DE(&z, primo);                         /* 4196 pop de */
    z.c = 0x38;                                /* 4197 */
    sub_0028(&z);                              /* 4199 rst 28 */
    sub_2ff3(&z);                              /* 419A jp 2fd3 */
    z.h++;                                     /* 2FD6 */
    M(z.ix + 0x0b) = z.l;                      /* 2FD7 */
    M(z.ix + 0x0c) = z.h;                      /* 2FDA */
    M(z.ix + 0x08) = 0xf8;                     /* 2FDD */
    M(z.ix + 0x12) = z.e;                      /* 2FE1 */
    M(z.ix + 0x13) = z.d;                      /* 2FE4 */
    z.e = 0x40;                                /* 2FE7 */
    SET_BC(&z, 0x0301);                        /* 2FE9 */
    z.a = 0xc0;                                /* 2FEC */
    sub_4af6(&z);                              /* 2FEE */
}

/* Passo 7, 421E: l'oggetto c49e alle coordinate di c247 (4B0E), riquadro
 * della cella (2457, 3A54 con BC = c870), stato C0 (4AF6, E = 1, BC =
 * 0401), suono 39. */
static void passo_421e(void)
{
    struct z80 z = {0};
    z.ix = 0xc49e;                             /* 421E */
    z.a = M(0xc247);                           /* 4222 */
    z.e = z.a;                                 /* 4225 */
    sub_4b0e(&z);                              /* 4226 */
    M(z.ix + 0x0e) = z.h;                      /* 4229 */
    M(z.ix + 0x15) = z.l;                      /* 422C */
    sub_2457(&z);                              /* 422F */
    SET_BC(&z, 0xc870);                        /* 4232 */
    sub_3a54(&z);                              /* 4235 */
    z.e = 0x01;                                /* 4238 */
    SET_BC(&z, 0x0401);                        /* 423A */
    z.a = 0xc0;                                /* 423D */
    sub_4af6(&z);                              /* 423F */
    z.c = 0x39;                                /* 4242 */
    sub_0028(&z);                              /* 4244 rst 28 */
}

/* Passo 9, 4247: c248 = c249, rimette i 19 byte salvati da c9ca in c4ce
 * + 30·i, c29a = 0, c49e = 0. */
static void passo_4247(void)
{
    M(0xc248) = M(0xc249);                     /* 4247-424A */
    uint16_t hl = 0xc4ce, de = 0xc9ca;         /* 424D-4253 */
    for (int i = 0; i < 0x13; i++) {           /* 4256-425E */
        M(hl) = M(de);
        de++;
        hl = (uint16_t)(hl + 0x30);
    }
    M(0xc29a) = 0x00;                          /* 4260-4261 */
    M(0xc49e) = 0x00;                          /* 4264-4265 */
}

/* 18, ingresso 4124 (resta 18). Con c19e = 0: blocco di c248, bit 7 di
 * c08d spento, e se un'attività 0A è nella tabella (6735) il suo slot si
 * azzera; poi il passo. Con c19e >= 0A: posta 0A, bit 7 di c08d acceso,
 * slot, c19e e c2b7 a 0. */
void attivita_18_4124(uint8_t slot)
{
    struct z80 z = {0};
    uint8_t passo = M(0xc19e);                 /* 4124-4128 */
    if (passo != 0 && passo >= 0x0a) {         /* 4129-412D */
        z.a = 0x0a;                            /* 412F */
        sub_0493(&z);                          /* 4131 */
        M(0xc08d) |= 0x80;                     /* 4134-4137 */
        tabella[slot] = 0x00;                  /* 4139-413B */
        M(0xc19e) = 0x00;                      /* 413C */
        M(0xc2b7) = 0x00;                      /* 413F */
        return;
    }
    if (passo == 0) {
        blocco_c248(&z);                       /* 4143-415B */
        M(0xc08d) &= (uint8_t)~0x80;           /* 415D-4160 */
        z.a = 0x0a;                            /* 4162 */
        sub_6735(&z);                          /* 4164 */
        if (z.f & LIBERA_ZERO)                 /* 4167 jr nz */
            M(HL(&z)) = 0x00;                  /* 4169 */
    }
    passi_416b();
}

/* 416B: HL = c19e, A = passo, c19e avanza di uno, `rst 20h` sulla tabella
 * 4171 (10 voci). Ci salta anche la fase 16 del 07 (2D9D, brief 66B) con
 * c19e diverso da 0A. */
void passi_416b(void)
{
    uint8_t passo = M(0xc19e);                 /* 416B-416E */
    M(0xc19e) = (uint8_t)(passo + 1);          /* 416F */
    switch (passo) {                           /* 4170 rst 20h */
    case 0: passo_41b7(); break;
    case 1: passo_41cf(); break;
    case 2: attesa_18(0x20); break;            /* 41A3 */
    case 3: M(0xc4bc)++; break;                /* 419D-41A0 */
    case 4: attesa_18(0x14); break;            /* 41A8 */
    case 5: passo_4185(); break;
    case 6: passo_2e79(0xc19e); break;         /* 2E79 */
    case 7: passo_421e(); break;
    case 8: attesa_18(0x18); break;            /* 41AD */
    case 9: passo_4247(); break;
    default: nemici_fuori_tabella(0x4170, passo); break;
    }
}
