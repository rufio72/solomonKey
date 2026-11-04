/* Corpi delle attività 12 (426A) e 13 (6480, a passi su c19d) del gioco
 * libero, lotto 52 (brief 52), tradotti dal disassemblato out/main64.dasm.
 * Regole come attivita_01_10.c; il corpo a passi come il 07
 * (attivita_attract07.c): HL = c19d all'ingresso, A = passo, c19d avanza
 * di uno e `rst 20h` salta al passo; `dec (hl)` resta sullo stesso passo.
 *
 * Il corpo 12 arriva a 4302, che legge il registro R: nel gioco libero R
 * non è modellato (brief 52, domanda 3) e la sentinella di macchina_libera
 * ferma il gioco quando un oggetto dei 17 soddisfa le condizioni. */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"

/* 12, ingresso 426A: sui 17 oggetti c4fe (passo 30) con i bit 1 e 7 di
 * (IX+0) accesi e (IX+4) >= 7, con HL che scorre c2c9 (passo 18): azione
 * del codice (IX+3) (4319), azzera (HL); col codice 0A suono 31 e C = 2;
 * col codice 10 l'indice è 0E + ((IX+4) - 20) >> 3 & 3, altrimenti
 * codice - 0A; sceglie un byte a caso (4302) dalla riga dell'indice, che
 * con indice 1-4 e il bit 5 di e604 acceso diventa il primo della riga e
 * 10 diventa 4. Lo scrive in (HL+6) e con valore < 3 posta l'attività 23;
 * (HL+4) = 7F, bit 6 di (HL-8), (IX+3) = 6 e stato C7 (4AF6, B = 0, E = 6).
 * Alla fine suono 2F e slot 0. */
void attivita_12_426a(uint8_t slot)
{
    struct z80 z = {0};
    z.b = 0x11;                                /* 426A */
    z.ix = 0xc4fe;                             /* 426C */
    SET_HL(&z, 0xc2c9);                        /* 4270 */
    do {
        if ((M(z.ix) & 0x82) == 0x82 &&        /* 4273-427A */
            M(z.ix + 0x04) >= 0x07) {          /* 427C-4281 */
            z.a = M(z.ix + 0x03);              /* 4283 */
            z.c = z.a;                         /* 4286 */
            uint16_t ogg = HL(&z);             /* 4287 push hl */
            sub_4319(&z);                      /* 4288 */
            SET_HL(&z, ogg);                   /* 428B pop hl */
            z.a = z.c;                         /* 428C */
            z.c = 0x00;                        /* 428D */
            M(HL(&z)) = z.c;                   /* 428F */
            z.a = (uint8_t)(z.a - 0x0a);       /* 4290 */
            if (z.a == 0) {                    /* 4292 */
                z.c = 0x31;                    /* 4294 */
                sub_0028(&z);                  /* 4296 rst 28 */
                z.c = 0x02;                    /* 4297 */
            }
            if (z.a == 0x06)                   /* 4299-429B */
                z.a = (uint8_t)((((uint8_t)(M(z.ix + 0x04) - 0x20) >> 3) & 0x03) + 0x0e);
            ogg = HL(&z);                      /* 42A9 push hl */
            sub_4302(&z);                      /* 42AA */
            if ((uint8_t)(z.a - 1) < 0x04 &&   /* 42AD-42B1 */
                (leggi_porta(0xe604) & 0x20))  /* 42B3-42B8 */
                z.a = leggi(DE(&z));           /* 42BA */
            if (z.a == 0x10)                   /* 42BB-42BD */
                z.a = 0x04;                    /* 42BF */
            SET_DE(&z, ogg);                   /* 42C1 pop de */
            M(ogg + 0x06) = z.a;               /* 42C2-42C6 */
            if (z.a < 0x03) {                  /* 42C7-42C9 */
                uint16_t bc = BC(&z);          /* 42CB-42CC push bc, de */
                z.a = 0x23;                    /* 42CD */
                sub_0493(&z);                  /* 42CF */
                SET_DE(&z, ogg);               /* 42D2-42D3 pop de, bc */
                SET_BC(&z, bc);
            }
            M(ogg + 0x04) = 0x7f;              /* 42D4-42D8 */
            M((uint16_t)(ogg - 0x08)) |= 0x40; /* 42DA-42DE */
            SET_HL(&z, ogg);                   /* 42E0 ex de,hl */
            SET_DE(&z, ogg - 0x08);
            z.d = z.b;                         /* 42E1 */
            z.b = 0x00;                        /* 42E2 */
            z.e = 0x06;                        /* 42E4 */
            M(z.ix + 0x03) = z.e;              /* 42E6 */
            z.a = 0xc7;                        /* 42E9 */
            sub_4af6(&z);                      /* 42EB */
            z.b = z.d;                         /* 42EE */
        }
        SET_DE(&z, 0x0030);                    /* 42EF */
        z.ix = (uint16_t)(z.ix + 0x30);        /* 42F2 */
        z.e = 0x18;                            /* 42F4 */
        SET_HL(&z, HL(&z) + DE(&z));           /* 42F6 */
    } while (--z.b);                           /* 42F7-42F8 */
    z.c = 0x2f;                                /* 42FB */
    sub_0028(&z);                              /* 42FD rst 28 */
    tabella[slot] = 0x00;                      /* 42FE-4301 */
}

/* Passi 0, 4 e 6 (64B0, 64B8): attesa su c19d con soglia C (2E90). */
static void attesa_13(uint8_t c)
{
    struct z80 z = {0};
    SET_HL(&z, 0xc19d);
    z.c = c;
    sub_2e90(&z);                              /* 64CE */
}

/* Passo 1, 6676: resta sul passo finché c46e ha i bit 6-7. Poi c459 = 0,
 * bit 0 di c044 spento, Dana (c82e) a 2, l'oggetto c46e prende le
 * coordinate di c83c/c843 e lo stato E2 (4AF6, B = 8, C = c84c senza il
 * bit 1, DE = 0, IX+1A = 0); suono 0C; col bit 1 di c82f copia il record
 * 5 da 66C4 (3569). */
static void passo_6676(void)
{
    struct z80 z = {0};
    if (M(0xc46e) & 0xc0) {                    /* 6677-667C */
        M(0xc19d)--;                           /* 667E */
        return;
    }
    M(0xc459) = 0x00;                          /* 6680-6681 */
    M(0xc044) &= (uint8_t)~0x01;               /* 6684-6687 */
    z.ix = 0xc46e;                             /* 668C */
    SET_DE(&z, 0x000e);                        /* 6690 */
    M(0xc82e) = 0x02;                          /* 6689, 6693 */
    z.a = M(0xc83c);                           /* 6695-6696 */
    M(z.ix + 0x0e) = z.a;                      /* 6697 */
    z.e = 0x07;                                /* 669A */
    z.a = M(0xc843);                           /* 669C-669D */
    M(z.ix + 0x15) = z.a;                      /* 669E */
    z.e = 0x09;                                /* 66A1 */
    SET_HL(&z, 0xc84c);                        /* 66A3 */
    z.c = (uint8_t)(M(0xc84c) & ~0x02);        /* 66A4-66A5 */
    M(z.ix + 0x1a) = z.d;                      /* 66A7 */
    z.b = 0x08;                                /* 66AA */
    z.e = z.d;                                 /* 66AC */
    z.a = 0xe2;                                /* 66AD */
    sub_4af6(&z);                              /* 66AF */
    z.c = 0x0c;                                /* 66B2 */
    sub_0028(&z);                              /* 66B4 rst 28 */
    if (!(M(0xc82f) & 0x02))                   /* 66B5-66BA */
        return;
    SET_HL(&z, 0x66c4);                        /* 66BB */
    z.a = 0x05;                                /* 66BE */
    sub_3569(&z);                              /* 66C0 */
}

/* Passo 2, 66D4: resta sul passo finché non c'è c48b = 1 col bit 1 di
 * c48c; poi c467 = 0, bit 6 di c46e spento, suono 0B. */
static void passo_66d4(void)
{
    struct z80 z = {0};
    if (M(0xc48b) != 0x01 || !(M(0xc48c) & 0x02)) {    /* 66D6-66E1 */
        M(0xc19d)--;                           /* 66E3-66E4 */
        return;
    }
    M(0xc467) = 0x00;                          /* 66E6-66E7 */
    M(0xc46e) &= (uint8_t)~0x40;               /* 66EA-66ED */
    z.c = 0x0b;                                /* 66EF */
    sub_0028(&z);                              /* 66F1 rst 28 */
}

/* Passo 3, 66F3: c48b = 8, bit 1 di c48c acceso, c46e = E2. */
static void passo_66f3(void)
{
    M(0xc48b) = 0x08;                          /* 66F4-66F7 */
    M(0xc48c) |= 0x02;                         /* 66F9-66FA */
    M(0xc46e) = 0xe2;                          /* 66FC-66FE */
}

/* Passo 5, 6702: +1 a (c46e+1D), bit 1 di (c46e+1E) spento. */
static void passo_6702(void)
{
    M(0xc46e + 0x1d)++;                        /* 6702-6706 */
    M(0xc46e + 0x1e) &= (uint8_t)~0x02;        /* 6709 */
}

/* Passo 7, 670F: azzeramenti di cambio quadro (75AA), record 5 da 81A1
 * (3569), tabella delle attività vuota (673F), posta 14, c19d = 0, una
 * vita in meno (c237) e posta 16 se finite, 17 altrimenti. Dura più di un
 * frame: non catturabile, verificato per composizione (brief 52). */
static void passo_670f(void)
{
    struct z80 z = {0};
    sub_75aa(&z);                              /* 670F */
    SET_HL(&z, 0x81a1);                        /* 6712 */
    z.a = 0x05;                                /* 6715 */
    sub_3569(&z);                              /* 6717 */
    sub_673f(&z);                              /* 671A */
    z.a = 0x14;                                /* 671D */
    sub_0493(&z);                              /* 671F */
    M(0xc19d) = 0x00;                          /* 6722-6723 */
    M(0xc237)--;                               /* 6726-6729 */
    z.a = M(0xc237) == 0 ? 0x16 : 0x17;        /* 672B-672F */
    sub_0493(&z);                              /* 6731 */
}

/* 13, ingresso 6480: tabella `rst 20h` 6486, 8 voci. */
void attivita_13_6480(uint8_t slot)
{
    (void)slot;                                /* ogni passo: pop hl o pop bc/de */
    uint8_t passo = M(0xc19d);                 /* 6480-6483 */
    M(0xc19d) = (uint8_t)(passo + 1);          /* 6484 */
    switch (passo) {                           /* 6485 rst 20h */
    case 0: attesa_13(0x18); break;            /* 64B0 */
    case 1: passo_6676(); break;
    case 2: passo_66d4(); break;
    case 3: passo_66f3(); break;
    case 4: attesa_13(0x30); break;            /* 64B8 */
    case 5: passo_6702(); break;
    case 6: attesa_13(0x30); break;            /* 64B8 */
    case 7: passo_670f(); break;
    default: nemici_fuori_tabella(0x6485, passo); break;
    }
}
