/* Corpi delle attività (ingressi della tabella 3B11), tradotti dal
 * disassemblato out/main64.dasm (brief 23). Firma attivita_fn di sched.h:
 * `pop hl; ret` verso 3AF3 diventa `return`, `pop hl; ld (hl),n; ret`
 * diventa `tabella[slot] = n; return`. Le chiamate vanno alle sub_* già
 * tradotte (routine.h); i registri del Z80 servono solo a passare gli
 * argomenti a quelle routine.
 *
 * Tradotti qui: 02 (4B84), 03 (4BC6), 0D (5C8E), 0E (63A4), 11 (456E),
 * 14 (44B5), 20 (3E90). Il corpo 0A (4DA2), motore dei nemici, sta in
 * nemici.c (lotto N1, brief 25). Restano NULL in corpo_attivita_replay 01
 * (48D1) e 10 (457B): vedi REPLAY.md. */
#include <stddef.h>
#include "routine.h"
#include "sched.h"
#include "nemici.h"

/* Lettura di una parola in RAM (byte basso al primo indirizzo). */
static uint16_t parola(uint16_t a)
{
    return (uint16_t)(M(a) | (M(a + 1) << 8));
}

/* ------------------------------------------------------------------ */
/* 0x4B98: chiusura dell'oggetto IX. Se il bit 1 di (ix+0) è acceso
 * conserva solo i bit 0 e 6; altrimenti, se la cella sotto l'oggetto
 * (sub_36f0 sulle coordinate +0e, +15) ha il bit 7, ridisegna il blocco
 * 2×2 (tile 0x10, attributo 0x20, sub_3a54) e poi conserva i bit 0 e 6.
 * Usata anche da 5917 (nemici_tipo09.c), prototipo in nemici.h. */
void sub_4b98(uint16_t ix)
{
    if (M(ix) & 0x02) {                        /* 4B98 bit 1,(ix+0) */
        M(ix) &= 0x41;                         /* 4B9E-4BA3 */
        return;
    }
    struct z80 z = {0};
    z.ix = ix;
    z.h = M(ix + 0x0e);                        /* 4BA7 */
    z.l = M(ix + 0x15);
    sub_36f0(&z);                              /* 4BAD: HL = cella, E = offset */
    if (M(HL(&z)) & 0x80) {                    /* 4BB0 bit 7,(hl) */
        sub_2457(&z);                          /* 4BB4: D = riga, E = colonna */
        z.b = 0x10;                            /* 4BB7 ld bc,1020 */
        z.c = 0x20;
        sub_3a54(&z);                          /* 4BBA */
    }
    M(ix) &= 0x41;                             /* 4BBD-4BC2 */
}

/* 02, ingresso 4B84: attende c463 >= 0x0e, poi chiude l'oggetto c46e
 * (sub_4b98) e diventa 03. */
static void attivita_02_4b84(uint8_t slot)
{
    if (M(0xc463) < 0x0e)                      /* 4B84-4B89 */
        return;                                /* 4B8B pop hl; ret */
    sub_4b98(0xc46e);                          /* 4B8D-4B91 */
    tabella[slot] = 0x03;                      /* 4B94-4B97 */
}

/* 03, ingresso 4BC6: attende c463 >= 0x0e, poi riaccende Dana (c82e):
 * posa, velocità e direzione dalle copie salvate in +29..+2c, X ± 3 dal
 * bit 0 della direzione, bit 5 di +0; c46e conserva solo il bit 0;
 * l'attività si spegne. */
static void attivita_03_4bc6(uint8_t slot)
{
    if (M(0xc463) < 0x0e)                      /* 4BC6-4BCB */
        return;                                /* 4BCD */
    const uint16_t ix = 0xc82e;                /* 4BCF */
    M(ix + 0x1d) = M(ix + 0x29);               /* 4BD3-4BD6 */
    M(ix + 0x07) = 0xff;                       /* 4BD9 */
    M(ix + 0x0b) = M(ix + 0x2a);               /* 4BDD-4BE0 */
    M(ix + 0x0c) = M(ix + 0x2b);               /* 4BE3-4BE6 */
    uint8_t dir = M(ix + 0x2c);                /* 4BE9 */
    M(ix + 0x1e) = dir;                        /* 4BEC */
    uint8_t passo = (dir & 0x01) ? 0x03 : 0xfd;    /* 4BEF-4BF5 */
    M(ix + 0x15) = (uint8_t)(M(ix + 0x15) + passo);    /* 4BF7-4BFA */
    M(ix + 0x07) = 0xff;                       /* 4BFD */
    M(ix) |= 0x20;                             /* 4C01 set 5,(ix+0) */
    M(0xc46e) &= 0x01;                         /* 4C05-4C0A */
    tabella[slot] = 0x00;                      /* 4C0D-4C10 */
}

/* ------------------------------------------------------------------ */
/* 0x250E: A = 0, C = (HL) e (HL) = 0; la parola a HL+1 cresce di C.
 * Restituisce la parola nuova (HL dell'originale) e in *riporto il
 * riporto a 16 bit di `add hl,bc`, che arriva al `sbc hl,bc` di 5CA5.
 * DE dell'originale finisce su HL+1 (c29b per 5C9E). */
static uint16_t sub_250e(uint16_t hl, int *riporto)
{
    uint8_t c = M(hl);                         /* 250F ld c,(hl); ld b,a */
    M(hl) = 0;                                 /* 2511 */
    uint16_t p = parola(hl + 1);               /* 2512 inc hl; rst 10 */
    uint32_t somma = (uint32_t)p + c;          /* 2516 add hl,bc */
    *riporto = somma > 0xffff;
    uint16_t nuova = (uint16_t)somma;
    M(hl + 2) = (uint8_t)(nuova >> 8);         /* 2518 ld (hl),d */
    M(hl + 1) = (uint8_t)nuova;                /* 251A ld (hl),e */
    return nuova;
}

/* 0D, ingresso 5C8E: il cancello c4ce. Con il bit 7 di (c4ce) spento
 * esce; con i bit 7 e 6 accesi accumula c29a nella parola c29b e la
 * confronta con c25a: sotto la soglia esce, entro 0x10 oltre la soglia
 * fissa la posa 2 (una volta), oltre spegne cancello e contatore, suono
 * 0x0e, bit 0 di c258 spento e tabellone (sub_349f). Con il solo bit 7:
 * se il bit 1 di (c4ce) è acceso e (c4d2) != 5 spegne cancello e c299,
 * bit 0 di c258 e tabellone; con (c4d2) == 5 (5CEA, mai eseguito nelle
 * tracce) mette c0 in c4ce. L'attività resta sempre. */
static void attivita_0d_5c8e(uint8_t slot)
{
    (void)slot;
    const uint16_t iy = 0xc4ce;                /* 5C8E */
    uint8_t a = M(iy);                         /* 5C95 */
    if (!(a & 0x80))                           /* 5C98 rla; jr nc */
        return;
    if (a & 0x40) {                            /* 5C9B rla; jr nc 5CDE */
        int riporto;
        uint16_t hl = sub_250e(0xc29a, &riporto);   /* 5C9E */
        uint16_t bc = parola(0xc25a);          /* 5CA1 */
        int32_t diff = (int32_t)hl - bc - riporto;  /* 5CA5 sbc hl,bc */
        if (diff < 0)
            return;                            /* 5CA7 jr c 5CF9 */
        if ((uint8_t)diff < 0x10) {            /* 5CA9-5CAC */
            if (M(iy + 0x1d) == 0x02)          /* 5CAE-5CB3 */
                return;
            M(iy) &= (uint8_t)~0x40;           /* 5CB5 */
            M(iy + 0x1d) = 0x02;               /* 5CB9 */
            M(iy + 0x1e) = 0x02;               /* 5CBD */
            M(iy) |= 0x40;                     /* 5CC1 */
            return;
        }
        M(0xc299) = 0;                         /* 5CC7-5CCA: DE-2 = c299 */
        M(iy) = 0;                             /* 5CCB */
        M(iy + 0x1d) = 0;                      /* 5CCE */
        struct z80 z = {0};
        z.c = 0x0e;                            /* 5CD1 */
        sub_0028(&z);                          /* 5CD3 rst 28 */
    } else {
        /* 5CDE: A è (c4ce) ruotato due volte a sinistra, il suo bit 3 è
         * il bit 1 di (c4ce) */
        if (!(a & 0x02))
            return;                            /* 5CE0 */
        if (M(iy + 0x04) == 0x05) {            /* 5CE3-5CE8 */
            M(iy) = 0xc0;                      /* 5CEA-5CEC */
            return;
        }
        M(0xc299) = 0;                         /* 5CF1-5CF3: HL-1 = c299 */
        M(iy) = 0;                             /* 5CF4 */
    }
    M(0xc258) &= (uint8_t)~0x01;               /* 5CD4-5CD7 */
    struct z80 z = {0};
    sub_349f(&z);                              /* 5CD9 */
}

/* ------------------------------------------------------------------ */
/* 0E, ingresso 63A4: ingresso nel cancello. Se Dana (c82e) ha il bit 1,
 * o il bit 0 di c258 è acceso, o i due bit alti di c25d sono nulli:
 * c299 = 0 e l'attività diventa 03. Altrimenti accende il bit 0 di c258,
 * posiziona il cancello c4ce sopra Dana, fa scorrere di 2 bit la parola
 * c25c, inizializza l'oggetto (sub_4af6, A = c0, E = 4 o 5 dal bit 7 di
 * c25d), l'attività diventa 0F e accoda il suono 0x0f o 0x0d. */
static void attivita_0e_63a4(uint8_t slot)
{
    uint8_t d = 0;
    if (!(M(0xc82e) & 0x02) && !(M(0xc258) & 0x01)) {   /* 63A4-63B0 */
        d = M(0xc25d);                         /* 63B2 ld de,(c25c) */
    }
    if ((d & 0xc0) == 0) {                     /* 63B6-63B9 */
        M(0xc299) = 0;                         /* 63BB-63BC */
        tabella[slot] = 0x03;                  /* 63BF-63C2 */
        return;
    }
    M(0xc258) |= 0x01;                         /* 63C3 set 0,(hl) */
    const uint16_t ix = 0xc4ce;                /* 63C5 */
    uint8_t b = (d & 0x80) ? 0x05 : 0x04;      /* 63C9-63CE add a,a; inc b */
    M(0xc29e) = 0x02;                          /* 63CF-63D2 */
    uint8_t c = M(0xc84c);                     /* 63D4 */
    M(ix + 0x0e) = (uint8_t)(M(0xc83c) - 2);   /* 63D8-63DD */
    M(ix + 0x15) = M(0xc843);                  /* 63E0-63E3 */
    uint16_t w = parola(0xc25c);               /* 63E6-63EB add hl,hl ×2 */
    w = (uint16_t)(w << 2);
    M(0xc25c) = (uint8_t)w;
    M(0xc25d) = (uint8_t)(w >> 8);
    struct z80 z = {0};
    z.ix = ix;                                 /* 63EF */
    z.e = b;                                   /* 63EE ld e,b */
    M(ix + 0x03) = 0;                          /* 63F3 */
    z.b = 0x02;                                /* 63F7 */
    z.c = (uint8_t)(c & ~0x02);                /* 63F9 res 1,c */
    z.a = 0xc0;                                /* 63FB */
    sub_4af6(&z);                              /* 63FD */
    tabella[slot] = 0x0f;                      /* 6400-6403 */
    z.a = 0x0f;
    z.c = (b & 0x01) ? 0x0f : 0x0d;            /* 6404-640A */
    sub_0028(&z);                              /* 640C rst 28 */
}

/* 11, ingresso 456E: attende c466 >= 6, poi azzera c49e e si spegne. */
static void attivita_11_456e(uint8_t slot)
{
    if (M(0xc466) < 0x06)                      /* 456E-4574 ret c */
        return;
    M(0xc49e) = 0;                             /* 4575-4576 */
    tabella[slot] = 0;                         /* 4579 */
}

/* ------------------------------------------------------------------ */
/* 14, ingresso 44B5: canali dei registri (sub_452b). Per i 4 canali di
 * c09d (passo 6) accesi nella maschera c09b, se c464 - (ix+0) >= (ix+1)
 * avanza il canale con la tabella 451C. Poi per il canale c0b7 della
 * maschera c0b5 lo stesso con la tabella 4528: quando il contatore del
 * canale ha raggiunto il limite (ix+3) il bit si spegne. La maschera
 * c0b5 ruotata torna al suo posto. L'attività resta. */
static void attivita_14_44b5(uint8_t slot)
{
    (void)slot;
    uint8_t c = M(0xc09b);                     /* 44B5-44B8 */
    uint8_t d = M(0xc464);                     /* 44B9-44BC */
    uint16_t ix = 0xc09d;                      /* 44BD */
    struct z80 z = {0};
    for (uint8_t b = 0; b != 4; b++) {         /* 44C1, 44DD-44E1 */
        int acceso = c & 0x01;                 /* 44C3 rrc c */
        c = (uint8_t)((c >> 1) | (c << 7));
        if (acceso && (uint8_t)(d - M(ix)) >= M(ix + 1)) {    /* 44C7-44CE */
            z.ix = ix; z.b = b; z.d = d;
            SET_HL(&z, 0x451c);                /* 44D0 */
            sub_452b(&z);                      /* 44D3 */
        }
        ix = (uint16_t)(ix + 6);               /* 44D6-44DA */
    }
    ix = 0xc0b7;                               /* 44E3 */
    c = M(0xc0b5);                             /* 44E9-44EC */
    {
        uint8_t b = 0;                         /* un solo canale (44E7, 450C) */
        int acceso = c & 0x01;                 /* 44ED rrc c */
        c = (uint8_t)((c >> 1) | (c << 7));
        if (acceso && (uint8_t)(d - M(ix)) >= M(ix + 1)) {    /* 44F1-44F8 */
            /* il flag Z all'uscita di 452B è quello del `cp (ix+3)` a 453C
             * (push af / pop af): contatore prima dell'incremento == limite */
            int fine = M(ix + 2) == M(ix + 3);
            z.ix = ix; z.b = b; z.d = d;
            SET_HL(&z, 0x4528);                /* 44FA */
            sub_452b(&z);                      /* 44FD */
            if (fine)                          /* 4500 jr nz */
                c &= 0x7f;                     /* 4502 res 7,c */
        }
    }
    /* 4511-4515: sette rrca = una rotazione a sinistra */
    M(0xc0b5) = (uint8_t)((c << 1) | (c >> 7));    /* 4517 */
}

/* ------------------------------------------------------------------ */
/* Sottrazione BCD di un byte: `sbc a,c; daa` con riporto in ingresso e
 * in uscita (*riporto). DAA dopo una sottrazione (N = 1): correzione 06
 * se mezzo prestito o nibble basso > 9, 60 se prestito o A > 0x99. */
static uint8_t sbc_daa(uint8_t a, uint8_t c, int *riporto)
{
    int cin = *riporto;
    int r = a - c - cin;
    int h = (a & 0x0f) < (c & 0x0f) + cin;
    int cy = r < 0;
    uint8_t v = (uint8_t)r;
    uint8_t corr = 0;
    if (h || (v & 0x0f) > 9)
        corr |= 0x06;
    if (cy || v > 0x99) {
        corr |= 0x60;
        cy = 1;
    }
    *riporto = cy;
    return (uint8_t)(v - corr);
}

/* 20, ingresso 3E90: il tempo. Il nibble basso di c091 (preso con rrd,
 * che fa scorrere c091 di 4 bit) moltiplicato per 16 e per c092 si
 * sottrae in BCD dal tempo c093-c095. Se il tempo si esaurisce: c08d =
 * c467 = 0, Dana (c82e) perde il bit 6 e prende 42; l'attività diventa
 * 15 (26 col bit 7 di c24e; 0 se Dana ha il bit 1). Altrimenti cifre a
 * c061 (sub_3f10), colore 71 o 31 (tempo sotto c096-c098), c099 aggiornato
 * con il suono di c09a o 24 al cambio e il tempo a d472 (sub_4d87). */
static void attivita_20_3e90(uint8_t slot)
{
    uint8_t v = M(0xc091);                     /* 3E90-3E94 xor a; rrd */
    uint8_t n = v & 0x0f;
    M(0xc091) = (uint8_t)(v >> 4);
    if (n == 0)
        return;                                /* 3E98 */
    uint8_t c = (uint8_t)(n << 4);             /* 3E9A-3E9F */
    uint8_t b = M(0xc092);                     /* 3EA0 */
    uint8_t a = 0;                             /* 3EA2 */
    int riporto = 0;
    do {                                       /* 3EA3 add a,c; djnz */
        int s = a + c;
        riporto = s > 0xff;
        a = (uint8_t)s;
    } while (--b != 0);
    c = a;                                     /* 3EA6 */
    for (uint16_t hl = 0xc093; hl < 0xc096; hl++) {  /* 3EA7-3EB0 */
        M(hl) = sbc_daa(M(hl), c, &riporto);
        c = 0;
    }
    if (riporto) {                             /* 3EB2 jr c 3EEE */
        M(0xc08d) = 0;                         /* 3EEE-3EF2 */
        M(0xc467) = 0;
        uint8_t nuovo = (M(0xc24e) & 0x80) ? 0x26 : 0x15;   /* 3EF5-3EFD */
        M(0xc82e) &= (uint8_t)~0x40;           /* 3F02 res 6,(hl) */
        if (M(0xc82e) & 0x02)                  /* 3F04-3F08 */
            nuovo = 0;
        tabella[slot] = nuovo;                 /* 3F09-3F0A */
        M(0xc82e) |= 0x42;                     /* 3F0B-3F0E */
        return;
    }
    struct z80 z = {0};
    sub_3f10(&z);                              /* 3EB4: HL = c096 */
    /* 3EB7-3EBC: ld a,(c045); ld c,1e; and 10 non lasciano nulla di
     * vivo se non il riporto azzerato da `and` */
    c = M(0xc09a);                             /* 3EBE-3EC1 */
    riporto = 0;
    for (int i = 0; i < 3; i++) {              /* 3EC2-3ECB: c093.. - c096.. */
        int r = M(0xc093 + i) - M(0xc096 + i) - riporto;
        riporto = r < 0;
    }
    uint8_t colore = 0x71;                     /* 3ECD */
    if (riporto) {                             /* 3ECF */
        c = 0x24;                              /* 3ED1 */
        colore = 0x31;                         /* 3ED3 */
    }
    if (M(0xc099) != c) {                      /* 3ED5-3ED7 */
        if (M(0xc045) & 0x06) {                /* 3ED9-3EDE */
            z.a = M(0xc045) & 0x06;
            z.c = c;
            sub_0028(&z);                      /* 3EE0 rst 28 */
        }
        M(0xc099) = c;                         /* 3EE1 */
    }
    SET_HL(&z, 0xd472);                        /* 3EE2 */
    SET_DE(&z, 0xc061);                        /* 3EE5 */
    z.c = colore;                              /* 3EE8 ld c,b */
    sub_4d87(&z);                              /* 3EE9 */
}

/* Corpi tradotti per il replay; NULL = non tradotto (replay.c si ferma). */
attivita_fn corpo_attivita_replay[SCHED_CODICI] = {
    [0x02] = attivita_02_4b84,
    [0x03] = attivita_03_4bc6,
    [0x0a] = attivita_0a_4da2,
    [0x0d] = attivita_0d_5c8e,
    [0x0e] = attivita_0e_63a4,
    [0x11] = attivita_11_456e,
    [0x14] = attivita_14_44b5,
    [0x20] = attivita_20_3e90,
};
