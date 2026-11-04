/* Corpo dell'attività 16 (6496) del gioco libero, lotto 55 (brief 55): la
 * fine partita, macchina a passi sul contatore c19d (tabella `rst 20h`
 * 649B, 10 voci), tradotta dal disassemblato out/main64.dasm. Regole come
 * il 13 (attivita_12_13.c): HL = c19d all'ingresso, A = passo, c19d avanza
 * di uno; `dec (hl)` resta sullo stesso passo. Il passo 4 (6842, la
 * classifica con 7A56) sta in attivita_16b.c. I passi 4, 7, 9 e la coda
 * del passo 8 chiamano 75AA, che dura più di un frame: non si catturano e
 * si verificano per composizione (75AA, 3569, 3F88, 673F, 0493, 6771,
 * 3A54, 3A1C, 342D, 31E9, 2D8A, 4D87 nel banco). */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"

/* Passo 0, 674A: suono 1D, gestore 33BA con C = 1, c467 = c19e = 0. */
static void passo_674a(void)
{
    struct z80 z = {0};
    z.c = 0x1d;                                /* 674A */
    sub_0028(&z);                              /* 674C rst 28h */
    z.c = 0x01;                                /* 674D */
    sub_33ba(&z);                              /* 674F */
    z.a = 0x00;                                /* 6752 */
    M(0xc467) = z.a;                           /* 6753 */
    M(0xc19e) = z.a;                           /* 6756 */
}

/* Passo 2, 675B: scritta 6766 a d5ec (6771). */
static void passo_675b(void)
{
    struct z80 z = {0};
    SET_DE(&z, 0xd5ec);                        /* 675B */
    SET_HL(&z, 0x6766);                        /* 675E */
    sub_6771(&z);                              /* 6761 */
}

/* Passo 5, 6785: immissione del nome del record. c1af conta i secondi
 * (c467 fino a 80) e a 1E chiude; c1b0 tiene la direzione del joystick
 * (c053) e il bit 4 del pulsante, c1b1 la lettera (1-25) che scorre a
 * ogni cambio di direzione (o ogni 10 giri di c466 con la stessa) e si
 * scrive alla posizione c1b4 della riga (c1b2) + 1. Alla pressione del
 * pulsante (bit 4 di c053, fronte di salita) passa alla lettera seguente;
 * alla terza copia i tre caratteri nel record (c1b5) e passa al passo 6
 * con il suono 3E. */
static void passo_6785(void)
{
    struct z80 z = {0};
    M(0xc19d)--;                               /* 6785 dec (hl) */
    SET_DE(&z, 0xc1af);                        /* 6786 */
    z.a = M(0xc1af);                           /* 6789 */
    if (z.a >= 0x1e) {                         /* 678A-678C */
        z.a = 0x00;                            /* 678E */
        M(0xc1af) = z.a;                       /* 678F */
        goto fine_6822;                        /* 6790 */
    }
    z.a = M(0xc467);                           /* 6793 */
    SET_DE(&z, 0xc19d);                        /* 6798 ex de,hl */
    SET_HL(&z, 0xc1af);
    if (z.a >= 0x80) {                         /* 6796, 6799 */
        M(0xc1af)++;                           /* 679B */
        z.a = 0x00;                            /* 679C */
        M(0xc467) = z.a;                       /* 679D */
    }
    SET_HL(&z, 0xc1b0);                        /* 67A0 inc hl */
    z.a = M(0xc053);                           /* 67A1 */
    z.c = z.a;                                 /* 67A4 */
    if (((z.a ^ M(0xc1b0)) & 0x03) == 0) {     /* 67A5-67A8 */
        z.a = M(0xc466);                       /* 67AA */
        int sotto = z.a < 0x0a;                /* 67AD */
        z.a = 0x00;                            /* 67AF */
        if (sotto)                             /* 67B1 */
            goto lettera_67c0;
        M(0xc466) = z.a;                       /* 67B3 */
    }
    z.a = (uint8_t)(0x03 & z.c);               /* 67B6-67B8 */
    if (M(0xc1b0) & 0x10)                      /* 67B9-67BB */
        z.a = (uint8_t)(z.a + 0x10);           /* 67BD */
    M(0xc1b0) = z.a;                           /* 67BF */
lettera_67c0:
    SET_HL(&z, 0xc1b1);                        /* 67C0 inc hl */
    if (z.a & 0x01)                            /* 67C1-67C2 rrca; jr nc */
        M(0xc1b1)++;                           /* 67C4 */
    if (z.a & 0x02)                            /* 67C5-67C6 rrca; jr nc */
        M(0xc1b1)--;                           /* 67C8 */
    z.a = M(0xc1b1);                        /* 67C9 */
    if (z.a == 0)                              /* 67CA-67CB */
        z.a = 0x25;                            /* 67CD */
    if (z.a == 0x26)                           /* 67CF-67D1 */
        z.a = 0x01;                            /* 67D3 */
    M(0xc1b1) = z.a;                           /* 67D5 */
    {
        int prestito = z.a < 0x1b;             /* 67D6 sub 1bh */
        z.a = (uint8_t)(z.a - 0x1b);
        int bit5 = (z.c & 0x20) != 0;          /* 67D8 bit 5,c (solo Z) */
        if (prestito) {                        /* 67DA jr c: lettera A-Z */
            z.b = bit5 ? 0x7b : 0x5b;          /* 67F3-67F7 */
            z.a = (uint8_t)(z.a + z.b);        /* 67F9 */
        } else if (bit5) {                     /* 67DC jr nz: segni da 6837 */
            SET_DE(&z, 0x6837 + z.a);          /* 67E8-67EF */
            z.a = leggi(DE(&z));               /* 67F0 */
        } else {                               /* cifre */
            z.a = (uint8_t)(z.a + 0x2f);       /* 67DE */
            if (z.a == 0x2f)                   /* 67E0-67E2 */
                z.a = 0x2e;                    /* 67E4 */
        }
    }
    SET_HL(&z, 0xc1b2);                        /* 67FA inc hl */
    sub_0010(&z);                              /* 67FB rst 10h: DE = (c1b2), HL = c1b4 */
    SET_DE(&z, DE(&z) + 1);                    /* 67FC */
    z.l = M(HL(&z));                           /* 67FD */
    z.h = 0x00;                                /* 67FE */
    SET_HL(&z, HL(&z) + DE(&z));               /* 6800 */
    scrivi_mem(HL(&z), z.a);                   /* 6801 */
    {
        int pulsante = (z.c & 0x10) != 0;      /* 6802 bit 4,c */
        SET_DE(&z, HL(&z));                    /* 6804 ex de,hl */
        SET_HL(&z, 0xc1b0);                    /* 6805 */
        if (!pulsante) {                       /* 6808 */
            M(0xc1b0) &= (uint8_t)~0x10;       /* 6833 */
            return;                            /* 6835 pop hl; ret */
        }
    }
    if (M(0xc1b0) & 0x10)                      /* 680A-680C */
        return;
    M(0xc1b0) |= 0x10;                         /* 680E */
    z.a = (uint8_t)(M(0xc1b4) + 1);            /* 6810-6813 */
    if (z.a < 0x03) {                          /* 6814-6816 */
        M(0xc1b1) = 0x01;                      /* 682B-682C */
        M(0xc1b4) = z.a;                       /* 682E */
        return;
    }
    {
        uint16_t da = DE(&z);                  /* 6818 ex de,hl */
        uint16_t a = (uint16_t)(M(0xc1b5) | (M(0xc1b6) << 8));     /* 6819 */
        for (int i = 0; i < 3; i++)            /* 681D-6820 lddr */
            scrivi_mem((uint16_t)(a - i), leggi((uint16_t)(da - i)));
    }
fine_6822:
    M(0xc19d)++;                               /* 6822-6825 */
    z.c = 0x3e;                                /* 6826 */
    sub_0028(&z);                              /* 6828 rst 28h */
}

/* Passo 7, 6A43: bit 3 di c09b spento, record 5 da 81A1 (3569), 75AA. Con
 * la stanza c243 >= 24 salta il passo 8; altrimenti schermata del
 * proseguimento (3F88 da 80B3, scritte da 6A9E, d5ec-d5f4 a zero,
 * c19f = 0, riquadro 2×2 A4 alla cella 140F). */
static void passo_6a43(void)
{
    struct z80 z = {0};
    SET_HL(&z, 0xc09b);                        /* 6A44 */
    M(0xc09b) &= (uint8_t)~0x08;               /* 6A47 */
    z.a = 0x05;                                /* 6A49 */
    SET_HL(&z, 0x81a1);                        /* 6A4B */
    sub_3569(&z);                              /* 6A4E */
    sub_75aa(&z);                              /* 6A51 */
    SET_HL(&z, 0xc19d);                        /* 6A54 pop hl */
    z.a = M(0xc243);                           /* 6A55 */
    if (z.a >= 0x24) {                         /* 6A58-6A5A */
        M(0xc19d)++;                           /* 6A5C */
        return;
    }
    SET_HL(&z, 0x80b3);                        /* 6A5F */
    sub_3f88(&z);                              /* 6A62 */
    SET_HL(&z, 0x6a9e);                        /* 6A65 */
    SET_DE(&z, 0xd568);                        /* 6A68 */
    sub_6771(&z);                              /* 6A6B */
    SET_DE(&z, 0xd588);                        /* 6A6E */
    sub_6771(&z);                              /* 6A71 */
    SET_DE(&z, 0xd5ec);                        /* 6A74 */
    z.b = 0x09;                                /* 6A77 */
    z.a = 0x00;                                /* 6A79 */
    do {
        M(DE(&z)) = z.a;                       /* 6A7A */
        SET_DE(&z, DE(&z) + 1);                /* 6A7B */
    } while (--z.b != 0);                      /* 6A7C djnz */
    M(0xc19f) = z.a;                           /* 6A7E */
    SET_DE(&z, 0xd5ee);                        /* 6A81 */
    sub_6771(&z);                              /* 6A84 */
    SET_DE(&z, 0xd62b);                        /* 6A87 */
    sub_6771(&z);                              /* 6A8A */
    SET_DE(&z, 0xd70b);                        /* 6A8D */
    sub_6771(&z);                              /* 6A90 */
    SET_DE(&z, 0x140f);                        /* 6A93 */
    SET_BC(&z, 0xa440);                        /* 6A96 */
    sub_3a54(&z);                              /* 6A99 */
}

/* Passo 8, 69A7: il conto alla rovescia del proseguimento. Resta sul
 * passo; con i bit 4-5 di c053 accesi, o a c19f = 0A (un passo ogni 40 di
 * c467, con il riquadro A4 - 4 · c19f alla cella 140F), passa al 9. Scrive
 * i crediti c04c a d712-d713 (3A1C, decina solo se non zero); col bit 0
 * di c052 e crediti: un credito in meno, c233 + 1, punteggio c239-c23c a
 * zero, 342D, vite da c04d (31E9), 2D8A, cifre FF00 a d44a o d45f (bit 0
 * di c045, 4D87), giocatore scambiato, c19d = 0, 75AA e slot 17. */
static void passo_69a7(uint8_t slot)
{
    struct z80 z = {0};
    M(0xc19d)--;                               /* 69A7 dec (hl) */
    z.a = (uint8_t)(M(0xc053) & 0x30);         /* 69A8-69AB */
    if (z.a == 0x30) {                         /* 69AD-69AF */
        M(0xc19d)++;                           /* 69C7 */
        return;                                /* 69C8 pop hl; ret */
    }
    z.a = M(0xc467);                           /* 69B1 */
    if (z.a >= 0x40) {                         /* 69B4-69B6 */
        z.a = 0x00;                            /* 69B8 */
        M(0xc467) = z.a;                       /* 69B9 */
        M(0xc19f)++;                           /* 69BD-69C0 */
        z.a = M(0xc19f);                       /* 69C1 */
        SET_HL(&z, 0xc19d);                    /* 69C4 ex de,hl */
        SET_DE(&z, 0xc19f);
        if (z.a >= 0x0a) {                     /* 69C2, 69C5 */
            M(0xc19d)++;                       /* 69C7 */
            return;
        }
        z.a = (uint8_t)(z.a * 4);              /* 69CA-69CB */
        z.b = z.a;                             /* 69CC */
        z.a = (uint8_t)(0xa4 - z.b);           /* 69CD-69CF */
        z.b = z.a;                             /* 69D0 */
        z.c = 0x40;                            /* 69D1 */
        SET_DE(&z, 0x140f);                    /* 69D3 */
        sub_3a54(&z);                          /* 69D6 */
    }
    z.a = M(0xc04c);                           /* 69D9 */
    z.c = z.a;                                 /* 69DC */
    sub_3a1c(&z);                              /* 69DD */
    SET_HL(&z, 0xd712);                        /* 69E0 */
    z.a = z.d;                                 /* 69E3 */
    if (z.a != 0x30)                           /* 69E4-69E6 */
        M(0xd712) = z.d;                       /* 69E8 */
    M(0xd713) = z.e;                           /* 69E9-69EA */
    SET_HL(&z, 0xd713);
    z.a = (uint8_t)(M(0xc052) & 0x01);         /* 69EB-69EE */
    if (z.a == 0)                              /* 69F0 */
        return;
    z.a = z.c;                                 /* 69F2 */
    if (z.a == 0)                              /* 69F3-69F4 */
        return;
    M(0xc04c)--;                               /* 69F6-69F9 */
    M(0xc233)++;                               /* 69FA-69FD */
    SET_HL(&z, 0xc239);                        /* 69FE */
    SET_BC(&z, 0x0400);                        /* 6A01 */
    do {
        M(HL(&z)) = z.c;                       /* 6A04 */
        SET_HL(&z, HL(&z) + 1);                /* 6A05 */
    } while (--z.b != 0);                      /* 6A06 djnz */
    sub_342d(&z);                              /* 6A08 */
    z.a = M(0xc04d);                           /* 6A0B */
    SET_HL(&z, 0xc237);                        /* 6A0E */
    M(0xc237) = z.a;                           /* 6A11 */
    sub_31e9(&z);                              /* 6A12 */
    sub_2d8a(&z);                              /* 6A15 */
    SET_HL(&z, (M(0xc045) & 0x01) ? 0xd45f : 0xd44a);  /* 6A18-6A23 */
    SET_DE(&z, 0xff00);                        /* 6A26 */
    M(0xc061) = 0x00;                          /* 6A29 */
    M(0xc062) = 0xff;
    SET_DE(&z, 0xc061);                        /* 6A2D */
    sub_4d87(&z);                              /* 6A30 */
    SET_HL(&z, 0xc045);                        /* 6A33 pop hl */
    z.a = (uint8_t)(0x01 ^ M(0xc045));         /* 6A34-6A36 */
    M(0xc045) = z.a;                           /* 6A37 */
    z.a = 0x00;                                /* 6A38 */
    M(0xc19d) = z.a;                           /* 6A39 */
    sub_75aa(&z);                              /* 6A3C */
    tabella[slot] = 0x17;                      /* 6A3F-6A40 pop hl; ld (hl),17 */
}

/* Passo 9, 6ADE: la stanza c243 entra fra le cinque migliori (c22e-c232,
 * in ordine decrescente), schermata 80D3 (3F88), tabella delle attività
 * vuota (673F), posta 14, 75AA; in c045 si spegne il bit 1 o 2 secondo il
 * giocatore (bit 0); c19d = c467 = 0, c1ae = 5, c1af = 0, bit 5 e 0 di
 * c044 spenti, posta 17. */
static void passo_6ade(void)
{
    struct z80 z = {0};
    z.b = 0x05;                                /* 6ADE */
    z.a = M(0xc243);                           /* 6AE0 */
    SET_HL(&z, 0xc22e);                        /* 6AE3 */
    for (;;) {
        if (z.a >= M(HL(&z))) {                /* 6AE6-6AE7 cp (hl); jr nc */
            do {
                z.c = M(HL(&z));               /* 6AEE */
                M(HL(&z)) = z.a;               /* 6AEF */
                z.a = z.c;                     /* 6AF0 */
                SET_HL(&z, HL(&z) + 1);        /* 6AF1 */
            } while (--z.b != 0);              /* 6AF2 djnz */
            break;
        }
        SET_HL(&z, HL(&z) + 1);                /* 6AE9 */
        if (--z.b == 0)                        /* 6AEA djnz */
            break;
    }
    SET_HL(&z, 0x80d3);                        /* 6AF4 */
    sub_3f88(&z);                              /* 6AF7 */
    sub_673f(&z);                              /* 6AFA */
    z.a = 0x14;                                /* 6AFD */
    sub_0493(&z);                              /* 6AFF */
    sub_75aa(&z);                              /* 6B02 */
    z.a = M(0xc045);                           /* 6B05 */
    z.c = z.a;                                 /* 6B08 */
    z.a = (uint8_t)(((z.a & 0x01) + 1) * 2);   /* 6B09-6B0C */
    z.a = (uint8_t)(~z.a & z.c);               /* 6B0D-6B0E */
    M(0xc045) = z.a;                           /* 6B0F */
    /* 6B12-6B14: and 6; jr nz,6B16 salta all'istruzione seguente */
    z.a = 0x00;                                /* 6B16 */
    M(0xc19d) = z.a;                           /* 6B17 */
    M(0xc467) = z.a;                           /* 6B1A */
    M(0xc1ae) = 0x05;                          /* 6B1D-6B20 */
    M(0xc1af) = z.a;                           /* 6B22-6B23 */
    M(0xc044) &= (uint8_t)~0x21;               /* 6B24-6B29 */
    z.a = 0x17;                                /* 6B2B */
    sub_0493(&z);                              /* 6B2D */
}

/* 16, ingresso 6496: passo da c19d (`rst 20h` 649B). */
void attivita_16_6496(uint8_t slot)
{
    uint8_t passo = M(0xc19d);                 /* 6496-6499 */
    M(0xc19d) = (uint8_t)(passo + 1);          /* 649A */
    switch (passo) {                           /* 649B rst 20h */
    case 0: passo_674a(); break;
    case 1: attesa_64ce(0x20); break;          /* 64BC */
    case 2: passo_675b(); break;
    case 3: attesa_64ce(0x60); break;          /* 64C0 */
    case 4: passo_6842(); break;               /* attivita_16b.c */
    case 5: passo_6785(); break;
    case 6: attesa_64ce(0x78); break;          /* 64CC */
    case 7: passo_6a43(); break;
    case 8: passo_69a7(slot); break;
    case 9: passo_6ade(); break;
    default: nemici_fuori_tabella(0x649b, passo); break;
    }
}
