/* La NMI del Z80 principale (0066) e le sue callee di servizio: monete
 * (01D5, 0219) e lo stub 03EE. Una NMI per frame, a maschera abilitata.
 * Come per le altre traduzioni si riproducono l'effetto sulla RAM e i
 * registri A, BC, DE, HL, IX, IY all'uscita (le bandiere F no). */
#include <stdio.h>
#include "routine.h"

/* Celle di sistema usate dalla catena della NMI. */
#define FLAG_C044     0xc044   /* bit 0 gioco attivo, 2 video, 5 ingresso→c053, 6 palette piena */
#define FLAG_C045     0xc045   /* bit 1 bonus tempo, 1-2 modo, 3 copione, 5 ribaltato, 7 reset in corso */
#define SALVA_IX      0xc040   /* parola: IX salvato dalla NMI */
#define SALVA_IY      0xc042   /* parola: IY salvato dalla NMI */
#define INGRESSI_P1   0xc050   /* copie di e600-e602: P1, P2, SYSTEM */
#define COMANDO       0xc053   /* comando corrente (da c050/c051 o dal copione) */
#define CODA_SUONI    0xc054   /* coda suoni c054-c057 (brief 05) */
#define DANA          0xc82e   /* slot oggetto di Dana */
#define CONTA_FRAME_A 0xc2b7   /* contatore di frame letto dall'attività 0A */
#define CONTA_FRAME_D 0xc29a   /* contatore di frame letto dall'attività 0D */
#define CONTA_C463    0xc463   /* 6 contatori per-frame c463-c468 */
#define PAROLA_C2A9   0xc2a9   /* parola incrementata a ogni frame */
#define BONUS_TEMPO   0xc234   /* parola del bonus tempo (bit 1 di c045) */

/* 0x03EE (dalla NMI, 010B, quando c078 non è zero: mai eseguito nelle
 * tracce): consegnerebbe il record c078-c07b a sub_3a54 (non tradotta,
 * filone rst 20). Stub: avviso su stderr, stato invariato. */
static void sub_03ee(struct z80 *z)
{
    (void)z;
    fprintf(stderr, "03ee: c078 non zero, sub_3a54 non tradotta: nessun effetto\n");
}

/* 0x0219 (da 01D5, fronte di rilascio di una moneta): HL = c046 (COIN1) o
 * c047 (COIN2). Spegne il bit 0 (moneta in corso); la durata della
 * pressione ((HL)/2, incrementata di 2 a frame da 01D5) deve stare fra 1 e
 * 0x25. Accoda il suono 0x18, incrementa il contatore monete c048/c049
 * (quello con il bit 7 acceso ridirige su c048), applica la tariffa
 * c04a/c04b (nibble basso = monete necessarie, alto = crediti dati):
 * al traboccare toglie le monete e somma i crediti in c04c; se i crediti
 * erano zero e si è in attract (c045 & 6 = 0) posta l'attività 08.
 * Uscita: HL = contatore usato (c048/c049) nel ramo pieno, A/BC/DE secondo
 * il ramo (commenti sotto). */
void sub_0219(struct z80 *z)
{
    uint16_t moneta = HL(z);               /* c046 o c047 */
    M(moneta) &= (uint8_t)~0x01;
    uint8_t a = (uint8_t)(M(moneta) >> 1); /* durata della pressione in frame */
    z->a = a;
    if (a < 1)
        return;                            /* ret c: rimbalzo */
    if (a >= 0x26)
        return;                            /* ret nc: pressione troppo lunga */
    z->c = 0x18;
    sub_004f(z);                           /* suono della moneta in coda */
    /* pop hl: si riparte dal puntatore della moneta */
    uint16_t cnt = (uint16_t)(moneta + 2); /* c048 o c049 */
    if (M(cnt) & 0x80)
        cnt = 0xc048;
    M(cnt)++;                              /* monete inserite (bit 7 = flag) */
    uint8_t tariffa = M(cnt + 2);          /* c04a o c04b (da sub_2980) */
    uint8_t servono = (uint8_t)(tariffa & 0x0f);
    a = (uint8_t)(M(cnt) & 0x7f);
    SET_HL(z, moneta);                     /* ex de,hl */
    SET_DE(z, cnt);
    z->b = tariffa;
    z->c = servono;
    z->a = (uint8_t)(a - servono);
    if (a < servono)
        return;                            /* ret c: monete non bastano */
    z->c = z->a;                           /* resto di monete */
    M(cnt) = (uint8_t)((M(cnt) & 0x80) | z->c);
    a = (uint8_t)(tariffa >> 4);           /* crediti da aggiungere */
    uint8_t vecchi = M(0xc04c);
    z->b = (uint8_t)((vecchi << 1) | (vecchi >> 7));   /* rlc b */
    z->a = (uint8_t)(a + vecchi);
    M(0xc04c) = z->a;
    if (vecchi == 0) {                     /* rlc b: Z solo partendo da 0 */
        z->a = (uint8_t)(M(FLAG_C045) & 0x06);
        if (z->a == 0) {
            z->a = 0x08;
            sub_0493(z);                   /* attività 08: moneta in attract */
        }
    }
    SET_HL(z, cnt);                        /* pop hl */
}

/* 0x01D5 (dalla NMI, 00C4): anti-rimbalzo delle monete. A ogni frame
 * c046 e c047 crescono di 2; dai bit 2-3 di c052 (copia di SYSTEM: COIN1,
 * COIN2) il fronte di pressione scrive 1 nella cella (bit 0 = moneta in
 * corso, contatore riazzerato) e il fronte di rilascio chiama sub_0219.
 * Uscita: B = 0; A = bit monete ruotati due volte a destra; C, DE, HL
 * secondo il ramo (dopo sub_0219 restano i suoi). */
void sub_01d5(struct z80 *z)
{
    M(0xc046) += 2;
    M(0xc047) += 2;
    z->b = 0;                              /* dal djnz del ciclo iniziale */
    uint16_t hl = 0xc046;
    uint8_t premute = (uint8_t)((M(0xc052) & 0x0c) >> 2); /* bit 0 COIN1, 1 COIN2 */
    z->c = premute;
    if (premute != 0) {
        /* fronte di pressione: si segna la moneta in corso */
        if (!(M(0xc046) & 0x01) && (premute & 0x01))
            M(0xc046) = 0x01;
        if (!(M(0xc047) & 0x01) && (premute & 0x02))
            M(0xc047) = 0x01;              /* 01FE: mai eseguito nelle tracce */
    }
    /* al nodo 0203 A vale di nuovo i bit monete (ld a,c a 01F9) */
    uint8_t a = (uint8_t)((premute >> 1) | (premute << 7));   /* rrca */
    if (!(premute & 0x01) && (M(0xc046) & 0x01)) {
        /* COIN1 rilasciata con moneta in corso: accettata (push af/pop af) */
        SET_HL(z, 0xc046);
        sub_0219(z);
        hl = HL(z);                        /* HL resta quello di sub_0219 */
    }
    uint8_t riporto = (uint8_t)(a & 0x01); /* rrca: riporto = COIN2 premuta */
    z->a = (uint8_t)((a >> 1) | (a << 7));
    if (riporto) {
        SET_HL(z, hl);
        return;                            /* ret c: COIN2 ancora premuta */
    }
    hl++;
    SET_HL(z, hl);
    if (M(hl) & 0x01)
        sub_0219(z);                       /* COIN2 rilasciata: accettata */
}

/* 0x0066: la NMI, una per frame a maschera abilitata. Salva AF e il banco
 * alternativo (exx) e IX/IY in c040/c042, poi: copia le palette e gli
 * sprite nei registri video, legge gli ingressi e600-e602 in c050-c052,
 * gestisce le monete (01D5), smista lo stato di Dana (03FB/042F/027C/04AC/
 * 0CE6/1288 col bit 0 di c044), fa un passo del motore oggetti (0602 sui
 * 24 slot da c46e) e della tabella sprite (05B3), avanza i contatori di
 * frame, il copione (01C5) e il tempo (026C), legge la protezione e603 e
 * il watchdog e606, spedisce la coda suoni al latch audio e800 e riarma la
 * maschera NMI su e600. All'uscita (retn) i registri del chiamante sono
 * intatti: la funzione non tocca z e lavora su una copia locale (il banco
 * alternativo). */
void sub_0066(struct z80 *z)
{
    struct z80 n = *z;                     /* banco alternativo per le callee */

    M(SALVA_IX) = (uint8_t)z->ix;          /* ld (c040),ix */
    M(SALVA_IX + 1) = (uint8_t)(z->ix >> 8);
    M(SALVA_IY) = (uint8_t)z->iy;          /* ld (c042),iy */
    M(SALVA_IY + 1) = (uint8_t)(z->iy >> 8);

    uint8_t c044 = M(FLAG_C044);
    if (c044 & 0x04) {                     /* bit 2: parte video attiva */
        unsigned i;
        if (c044 & 0x40) {                 /* bit 6: palette piena, una volta */
            M(FLAG_C044) = (uint8_t)(c044 & ~0x40);
            for (i = 0; i < 0x20; i++)     /* palette di quadro c994 → e400 */
                M(0xe400 + i) = M(0xc994 + i);
            for (i = 0; i < 0x80; i++)     /* palette di stanza c9ca → e500 */
                M(0xe500 + i) = M(0xc9ca + i);
        } else {                           /* altrimenti le due code corte */
            for (i = 0; i < 0x26; i++)
                M(0xe4c6 + i) = M(0xc96e + i);
            for (i = 0; i < 0x16; i++)
                M(0xe5d4 + i) = M(0xc9b4 + i);
        }
        for (i = 0; i < 0x60; i++)         /* tabella sprite c8ee → e020 */
            M(0xe020 + i) = M(0xc8ee + i);
        /* ldir da e600: P1, P2, SYSTEM vivi nelle copie c050-c052 */
        M(INGRESSI_P1 + 0) = leggi_porta(0xe600);   /* P1 */
        M(INGRESSI_P1 + 1) = leggi_porta(0xe601);   /* P2 */
        M(INGRESSI_P1 + 2) = leggi_porta(0xe602);   /* SYSTEM: start/coin */
    }

    sub_01d5(&n);                          /* monete */

    uint8_t c = M(FLAG_C044);
    if (c & 0x20) {                        /* bit 5: ingresso corrente in c053 */
        uint16_t sorgente = INGRESSI_P1;
        if (M(FLAG_C045) & 0x20)
            sorgente++;                    /* 00D9 (P2): mai eseguito nelle tracce */
        M(COMANDO) = M(sorgente);
    }
    if (c & 0x01) {                        /* bit 0: logica di Dana attiva */
        uint8_t comando = M(COMANDO);
        if (comando & 0x20) {              /* bit 5: spegnimento richiesto */
            if ((M(0xc46e) & 0xc1) == 0)
                sub_03fb(&n);
        } else if (comando & 0x10) {       /* bit 4: cancello */
            if ((M(0xc46e) & 0xc1) == 0)
                sub_042f(&n);
        } else {
            M(0xc46e) &= (uint8_t)~0x01;
        }
        if (M(0xc078) != 0) {
            SET_HL(&n, 0xc078);
            n.a = M(0xc078);
            sub_03ee(&n);                  /* mai eseguito nelle tracce */
        }
        sub_027c(&n);                      /* collisioni di Dana */
        sub_04ac(&n);                      /* animazione/stati di Dana */
        sub_0ce6(&n);                      /* stati della chiave */
        sub_1288(&n);                      /* raccolta sotto Dana */
    }

    /* motore per-oggetto: 24 slot da 0x30 byte da c46e (ciclo 0124-0131) */
    for (unsigned i = 0; i < 0x18; i++) {
        uint16_t slot = (uint16_t)(0xc46e + 0x30 * i);
        SET_HL(&n, slot);
        n.ix = slot;
        n.b = (uint8_t)(0x18 - i);         /* contatore del djnz */
        n.c = 0;
        sub_0602(&n);
    }
    sub_05b3(&n);                          /* tabella sprite dagli oggetti */

    for (unsigned i = 0; i < 6; i++)       /* contatori c463-c468 */
        M(CONTA_C463 + i)++;
    uint16_t c2a9 = (uint16_t)((M(PAROLA_C2A9) | (M(PAROLA_C2A9 + 1) << 8)) + 1);
    M(PAROLA_C2A9) = (uint8_t)c2a9;
    M(PAROLA_C2A9 + 1) = (uint8_t)(c2a9 >> 8);

    uint8_t c045 = M(FLAG_C045);           /* resta in C fino a 0183 */
    if ((c045 & 0x06) == 0 && (c045 & 0x08)) {
        SET_HL(&n, c2a9);                  /* L = byte basso del nuovo c2a9 */
        sub_01c5(&n);                      /* copione dell'attract */
    }
    M(CONTA_FRAME_A)++;
    M(CONTA_FRAME_D)++;
    sub_026c(&n);                          /* tempo di gioco */

    /* protezione e603 (bit 0; in MAME il PC 015E legge sempre 00) */
    if (leggi_e603(0x015E) & 0x01) {
        /* finestra 0164-0182, mai eseguita in MAME: reset del gioco */
        c045 = M(FLAG_C045);
        if (!(c045 & 0x80) && !(c045 & 0x06)) {
            c045 |= 0x80;
            M(FLAG_C045) = c045;
            sub_673f(&n);                  /* azzera la tabella attività */
            n.a = 0x27;
            sub_0493(&n);                  /* posta l'attività 27 */
            M(0xc1ae) = 0;
        }
    }
    if (c045 & 0x02) {                     /* bit 1: bonus tempo */
        uint16_t bonus = (uint16_t)(M(BONUS_TEMPO) | (M(BONUS_TEMPO + 1) << 8));
        if (bonus == 0xffff)
            M(BONUS_TEMPO + 2)++;          /* 0193: mai eseguito nelle tracce */
        bonus++;
        M(BONUS_TEMPO) = (uint8_t)bonus;
        M(BONUS_TEMPO + 1) = (uint8_t)(bonus >> 8);
    }

    (void)leggi_porta(0xe606);             /* lettura a vuoto: watchdog */

    uint8_t suono = M(CODA_SUONI);
    if (suono != 0) {
        scrivi_porta(0xe800, suono);       /* latch audio verso il Z80 suoni */
        M(CODA_SUONI + 0) = M(CODA_SUONI + 1);   /* la coda scorre */
        M(CODA_SUONI + 1) = M(CODA_SUONI + 2);
        M(CODA_SUONI + 2) = M(CODA_SUONI + 3);
        M(CODA_SUONI + 3) = 0;
    }

    /* ld ix,(c040) / ld iy,(c042) / exx / pop af: i registri del chiamante
     * tornano intatti; z non è mai stato toccato. Riarmo della NMI: */
    scrivi_porta(0xe600, 0);               /* maschera NMI spenta... */
    scrivi_porta(0xe600, 1);               /* ...e riaccesa (impulso) */
}
