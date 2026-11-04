/* Passo 4 del corpo 16 (6842, la classifica) e la routine 7A56 che la
 * disegna, lotto 55 (brief 55). Il disassemblato out/main64.dasm decodifica
 * male 6837-6848 (la tabella di 11 byte 6837-6841 dei segni del passo 5
 * trascina la decodifica): l'istruzione a 6842 non vi compare. Byte veri
 * (xxd -s 0x6837 out/main64.bin): 21 4E C2 CB 56 CB 96 C4 76 69 CD AA 75 =
 * `ld hl,c24e; bit 2,(hl); res 2,(hl); call nz,6976; call 75aa`; da 6849
 * in poi il disassemblato è corretto (6849 è la parte `call nz` vista a
 * metà, 684C la `call 75aa`).
 *
 * Entrambi durano più di un frame (75AA in testa a 6842, 7A56 non
 * catturabile in tre varianti): si verificano per composizione (6976,
 * 75AA, 2533, 6771, 3A54, 4398, 3F88, 4D37, 4D87, 3A1C nel banco).
 *
 * Classifica: 10 record da 9 byte a ca4c-caa5: punteggio in 4 byte (byte
 * meno significativo per primo, come c239-c23c), nome in 3 byte, stanza
 * c244 e c245 - 30. */
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "attivita_libera.h"

/* lddr di BC byte da HL a DE all'indietro: uscita HL, DE arretrati, BC = 0. */
static void lddr(struct z80 *z)
{
    uint16_t da = HL(z), a = DE(z), n = BC(z);
    do {
        M(a) = M(da);
        a--;
        da--;
    } while (--n != 0);
    SET_HL(z, da);
    SET_DE(z, a);
    SET_BC(z, 0);
}

/* Passo 4, 6842: col bit 2 di c24e conta la partita (6976, bit spento),
 * azzeramenti di cambio quadro (75AA); cerca il primo record (su 10) con
 * punteggio non superiore a c239-c23c. Se nessuno: c19d += 2 (salta il
 * passo 5 dell'immissione del nome). Altrimenti fa scorrere in giù i
 * record seguenti (2533: 9 byte per record), scrive il record nuovo
 * (punteggio, nome vuoto, c244, c245 - 30; c1b5 = ultimo byte del nome),
 * disegna titolo e classifica (6771 da 6933, 3A54, 7A56), c1af-c1b4 da
 * 6970, suono 1F (primo posto) o 20, la riga del record evidenziata
 * (attributo 60 al primo posto, 40 altrimenti), il riquadro del nome da
 * 695B (c1b2 = inizio della riga del nome), gestore 3 (4398). */
void passo_6842(void)
{
    struct z80 z = {0};
    SET_HL(&z, 0xc24e);                        /* 6842 */
    int conta = (M(0xc24e) & 0x04) != 0;       /* 6845 bit 2,(hl) */
    M(0xc24e) &= (uint8_t)~0x04;               /* 6847 res 2,(hl) */
    if (conta)
        sub_6976(&z);                          /* 6849 call nz */
    sub_75aa(&z);                              /* 684C */
    z.c = 0x0a;                                /* 684F */
    SET_HL(&z, 0xca4c);                        /* 6851 */
    for (;;) {
        z.b = 0x04;                            /* 6854 */
        SET_DE(&z, 0xc239);                    /* 6856 */
        int prestito = 0;                      /* 6859 or a */
        do {
            int r = M(DE(&z)) - M(HL(&z)) - prestito;  /* 685A-685B sbc a,(hl) */
            z.a = (uint8_t)r;
            prestito = r < 0;
            SET_HL(&z, HL(&z) + 1);            /* 685C */
            SET_DE(&z, DE(&z) + 1);            /* 685D */
        } while (--z.b != 0);                  /* 685E djnz */
        if (!prestito)                         /* 6860 jr nc */
            break;
        SET_DE(&z, 0x0005);                    /* 6862 */
        SET_HL(&z, HL(&z) + DE(&z));           /* 6865 */
        if (--z.c == 0) {                      /* 6866-6867 */
            M(0xc19d) += 2;                    /* 6869-686D */
            return;                            /* 686E pop hl; ret */
        }
    }
    uint16_t salvato = BC(&z);                 /* 6870 push bc */
    SET_DE(&z, HL(&z) - 4);                    /* 6871-6875 ex de,hl: DE = record */
    SET_HL(&z, 0xfffc);
    if (--z.c == 0) {                          /* 6876-6877 ultimo record */
        SET_HL(&z, DE(&z));                    /* 6886-688A */
        SET_DE(&z, DE(&z) + 8);
    } else {
        sub_2533(&z);                          /* 6879: BC = 9 · C */
        SET_DE(&z, 0xcaa5);                    /* 687C */
        SET_HL(&z, 0xca9c);                    /* 687F */
        lddr(&z);                              /* 6882 */
    }
    z.a = (uint8_t)(M(0xc245) - 0x30);         /* 688B-688E */
    M(DE(&z)) = z.a;                           /* 6890 */
    SET_DE(&z, DE(&z) - 1);                    /* 6891 */
    z.a = M(0xc244);                           /* 6892 */
    M(DE(&z)) = z.a;                           /* 6895 */
    SET_DE(&z, DE(&z) - 1);                    /* 6896 */
    M(0xc1b5) = z.e;                           /* 6897 */
    M(0xc1b6) = z.d;
    z.a = 0x00;                                /* 689B */
    z.b = 0x03;                                /* 689C */
    do {
        M(DE(&z)) = z.a;                       /* 689E */
        SET_DE(&z, DE(&z) - 1);                /* 689F */
    } while (--z.b != 0);                      /* 68A0 djnz */
    SET_HL(&z, 0xc23c);                        /* 68A2 */
    z.c = 0x04;                                /* 68A5 */
    lddr(&z);                                  /* 68A7 */
    SET_DE(&z, 0xd4a8);                        /* 68A9 */
    SET_HL(&z, 0x6933);                        /* 68AC */
    sub_6771(&z);                              /* 68AF */
    SET_DE(&z, 0xd4c6);                        /* 68B2 */
    sub_6771(&z);                              /* 68B5 */
    SET_DE(&z, 0x0503);                        /* 68B8 */
    SET_BC(&z, 0xa870);                        /* 68BB */
    sub_3a54(&z);                              /* 68BE */
    SET_DE(&z, 0x051b);                        /* 68C1 */
    SET_BC(&z, 0xac70);                        /* 68C4 */
    sub_3a54(&z);                              /* 68C7 */
    sub_7a56(&z);                              /* 68CA */
    for (int i = 0; i < 6; i++)                /* 68CD-68D6 ldir */
        M(0xc1af + i) = leggi((uint16_t)(0x6970 + i));
    SET_HL(&z, 0x6976);
    SET_DE(&z, 0xc1b5);
    SET_BC(&z, 0);
    SET_DE(&z, salvato);                       /* 68D8 pop de */
    z.a = (uint8_t)(0x0a - z.e);               /* 68D9-68DB */
    z.c = 0x1f;                                /* 68DC */
    if (z.a != 0)                              /* 68DE */
        z.c++;                                 /* 68E0 */
    sub_0028(&z);                              /* 68E1 rst 28h */
    SET_DE(&z, 0x0040);                        /* 68E2 */
    SET_HL(&z, 0x0000);                        /* 68E5-68E6 ld h,d; ld l,d */
    z.c = 0x60;                                /* 68E7 */
    for (;;) {
        z.a--;                                 /* 68E9 */
        if (z.a & 0x80)                        /* 68EA jp m */
            break;
        z.c = 0x40;                            /* 68ED */
        SET_HL(&z, HL(&z) + DE(&z));           /* 68EF */
    }
    SET_DE(&z, 0xd102);                        /* 68F2 */
    SET_HL(&z, HL(&z) + DE(&z));               /* 68F5 */
    SET_DE(&z, HL(&z));                        /* 68F6-68F7 */
    z.b = 0x1c;                                /* 68F8 */
    do {
        M(HL(&z)) = z.c;                       /* 68FA */
        SET_HL(&z, HL(&z) + 1);                /* 68FB */
    } while (--z.b != 0);                      /* 68FC djnz */
    SET_BC(&z, 0x03d1);                        /* 68FE */
    SET_HL(&z, HL(&z) + BC(&z));               /* 6901 */
    {
        uint16_t t = HL(&z);                   /* 6902 ex de,hl */
        SET_HL(&z, DE(&z));
        SET_DE(&z, t);
    }
    SET_HL(&z, 0x695b);                        /* 6903 */
    sub_6771(&z);                              /* 6906 */
    SET_BC(&z, 0x001b);                        /* 6909 */
    SET_DE(&z, DE(&z) + BC(&z));               /* 690C-690E */
    M(0xc1b2) = z.e;                           /* 690F */
    M(0xc1b3) = z.d;
    sub_6771(&z);                              /* 6913 */
    SET_BC(&z, 0x001b);                        /* 6916 */
    SET_DE(&z, DE(&z) + BC(&z));               /* 6919-691B */
    sub_6771(&z);                              /* 691C */
    SET_HL(&z, M(0xc1b2) | (M(0xc1b3) << 8));  /* 691F */
    SET_HL(&z, HL(&z) + 1);                    /* 6922 */
    z.h &= (uint8_t)~0x04;                     /* 6923 res 2,h */
    z.b = 0x03;                                /* 6925 */
    do {
        M(HL(&z)) = 0x40;                      /* 6927 */
        SET_HL(&z, HL(&z) + 1);                /* 6929 */
    } while (--z.b != 0);                      /* 692A djnz */
    z.a = 0x03;                                /* 692C */
    sub_4398(&z);                              /* 692E */
}

/* 0x7A56 (68CA, 7A2B): la classifica. Schermata 80B3 (3F88); c1b0 = d502
 * (riga), c1b2 = ca4c (record) da 7AEF; per i 10 record: ordinale da
 * 7AF3 (6 byte per voce, l'ultima per il primo posto), punteggio
 * (4D37, 4D87 a riga + 0B, attributo 0), nome di 3 byte a riga + 0E con
 * attributo 0 (C = 0 da 0300), scritte da 7B2F, stanza a due cifre (3A1C,
 * decina solo se non zero), «-», c245 - 30 + 30. Riga successiva a +40.
 * Uscita: B = 0, C del primo push (0), A = ultimo carattere, HL = riga
 * corrente, DE = c1b0 di partenza della riga. */
void sub_7a56(struct z80 *z)
{
    SET_HL(z, 0x80b3);                         /* 7A56 */
    sub_3f88(z);                               /* 7A59 */
    for (int i = 0; i < 4; i++)                /* 7A5C-7A65 ldir */
        M(0xc1b0 + i) = leggi((uint16_t)(0x7aef + i));
    SET_DE(z, 0xc1b4);
    SET_HL(z, 0x7af3);
    SET_BC(z, 0x0000);
    z->b = 0x0a;                               /* 7A67 */
    do {
        uint16_t conto = BC(z);                /* 7A69 push bc */
        z->a = (uint8_t)(z->b - 1);            /* 7A6A-7A6B */
        z->l = z->a;                           /* 7A6C */
        z->a = (uint8_t)(z->a + z->a + z->l);  /* 7A6D-7A6E */
        SET_HL(z, 0x7af3);                     /* 7A6F */
        sub_0008(z);                           /* 7A72 rst 8 */
        SET_DE(z, M(0xc1b0) | (M(0xc1b1) << 8));       /* 7A73 */
        sub_6771(z);                           /* 7A77 */
        SET_HL(z, M(0xc1b2) | (M(0xc1b3) << 8));       /* 7A7A */
        sub_0010(z);                           /* 7A7D rst 10h */
        uint16_t basso = DE(z);                /* 7A7E push de */
        sub_0010(z);                           /* 7A7F rst 10h */
        M(0xc1b2) = z->l;                      /* 7A80 */
        M(0xc1b3) = z->h;
        SET_HL(z, basso);                      /* 7A83 pop hl */
        sub_4d37(z);                           /* 7A84 */
        SET_DE(z, M(0xc1b0) | (M(0xc1b1) << 8));       /* 7A87 */
        SET_HL(z, DE(z) + 0x000b);             /* 7A8B-7A8E */
        SET_DE(z, 0xc061);                     /* 7A8F */
        z->c = 0x00;                           /* 7A92 */
        sub_4d87(z);                           /* 7A94 */
        SET_DE(z, M(0xc1b0) | (M(0xc1b1) << 8));       /* 7A97 */
        SET_HL(z, DE(z) + 0x000e);             /* 7A9B-7A9E */
        SET_DE(z, M(0xc1b2) | (M(0xc1b3) << 8));       /* 7A9F */
        SET_BC(z, 0x0300);                     /* 7AA3 */
        do {
            z->a = leggi(DE(z));               /* 7AA6 */
            M(HL(z)) = z->a;                   /* 7AA7 */
            z->h &= (uint8_t)~0x04;            /* 7AA8 res 2,h */
            M(HL(z)) = z->c;                   /* 7AAA */
            z->h |= 0x04;                      /* 7AAB set 2,h */
            SET_HL(z, HL(z) + 1);              /* 7AAD */
            SET_DE(z, DE(z) + 1);              /* 7AAE */
        } while (--z->b != 0);                 /* 7AAF djnz */
        uint16_t record = DE(z);               /* 7AB1 push de */
        SET_HL(z, HL(z) + 2);                  /* 7AB2-7AB3 */
        SET_DE(z, HL(z));                      /* 7AB4 ex de,hl */
        SET_HL(z, 0x7b2f);                     /* 7AB5 */
        sub_6771(z);                           /* 7AB8 */
        SET_DE(z, DE(z) + 1);                  /* 7ABB */
        uint16_t riga = DE(z);                 /* 7ABC push de */
        sub_6771(z);                           /* 7ABD */
        SET_HL(z, riga);                       /* 7AC0 pop hl */
        SET_BC(z, record);                     /* 7AC1 pop bc */
        z->a = leggi(BC(z));                   /* 7AC3 */
        SET_BC(z, BC(z) + 1);                  /* 7AC4 */
        sub_3a1c(z);                           /* 7AC5 */
        z->a = z->d;                           /* 7AC8 */
        SET_HL(z, riga);                       /* 7AC2 push hl / 7AC9 pop hl */
        if (z->a != 0x30)                      /* 7ACA-7ACC */
            M(HL(z)) = z->a;                   /* 7ACE */
        SET_HL(z, HL(z) + 1);                  /* 7ACF */
        M(HL(z)) = z->e;                       /* 7AD0 */
        SET_HL(z, HL(z) + 1);                  /* 7AD1 */
        M(HL(z)) = 0x2d;                       /* 7AD2 */
        SET_HL(z, HL(z) + 1);                  /* 7AD4 */
        z->a = leggi(BC(z));                   /* 7AD5 */
        SET_BC(z, BC(z) + 1);                  /* 7AD6 */
        M(0xc1b2) = z->c;                      /* 7AD7 */
        M(0xc1b3) = z->b;
        z->a = (uint8_t)(z->a + 0x30);         /* 7ADB */
        M(HL(z)) = z->a;                       /* 7ADD */
        SET_DE(z, M(0xc1b0) | (M(0xc1b1) << 8));       /* 7ADE */
        SET_HL(z, DE(z) + 0x0040);             /* 7AE2-7AE5 */
        M(0xc1b0) = z->l;                      /* 7AE6 */
        M(0xc1b1) = z->h;
        SET_BC(z, conto);                      /* 7AE9 pop bc */
    } while (--z->b != 0);                     /* 7AEA-7AEB dec b; jp nz */
}
