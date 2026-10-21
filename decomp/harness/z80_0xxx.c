/* Routine del Z80 principale fra 0x0000 e 0x0fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* 0x0008 (rst 08): indice di una tabella di parole.
 * Ingresso: A indice, HL base. Uscita: DE = A, HL = HL + 2·A. */
void sub_0008(struct z80 *z)
{
    SET_DE(z, z->a);
    SET_HL(z, HL(z) + 2 * DE(z));
}

/* 0x0010 (rst 10): legge la parola a HL (anche in ROM).
 * Uscita: DE = parola (byte basso a HL), HL avanzato di 2. */
void sub_0010(struct z80 *z)
{
    uint16_t hl = HL(z);
    z->e = leggi(hl);
    z->d = leggi(hl + 1);
    SET_HL(z, hl + 2);
}

/* Contatore a 16 bit c090-c091 e suo incremento c08f, attivo col bit 7 di c08d. */
#define TEMPO_ATTIVO   0xc08d   /* bit 7: conteggio attivo */
#define TEMPO_PASSO    0xc08f   /* valore scelto dal record della stanza */
#define TEMPO_CONTO    0xc090   /* parola: byte basso c090, alto c091 */

/* 0x026C (chiamata dalla NMI): se il bit 7 di c08d è acceso somma c08f alla
 * parola c090-c091. Uscita: HL sull'ultimo byte toccato, A = byte basso nuovo. */
void sub_026c(struct z80 *z)
{
    SET_HL(z, TEMPO_ATTIVO);
    if (!(M(TEMPO_ATTIVO) & 0x80))
        return;
    unsigned somma = M(TEMPO_PASSO) + M(TEMPO_CONTO);
    z->a = M(TEMPO_CONTO) = (uint8_t)somma;
    SET_HL(z, TEMPO_CONTO);
    if (somma <= 0xff)
        return;
    /* riporto: si incrementa il byte alto */
    M(TEMPO_CONTO + 1)++;
    SET_HL(z, TEMPO_CONTO + 1);
}

/* 0x0018 (rst 18): come rst 10 ma con DE e HL scambiati all'uscita.
 * Uscita: HL = parola letta a HL (anche ROM), DE = HL d'ingresso + 2. */
void sub_0018(struct z80 *z)
{
    sub_0010(z);
    uint16_t de = DE(z);
    SET_DE(z, HL(z));
    SET_HL(z, de);
}

/* Copione a passi letto da 01C5: puntatore corrente e codice estratto. */
#define COPIONE_PTR  0xc1b7   /* parola: posizione nel copione (anche ROM) */
#define COPIONE_COD  0xc053   /* ultimo codice estratto */

/* 0x01C5 (chiamata dalla NMI, 0150): confronta L col byte corrente del
 * copione (c1b7); se coincide estrae il byte seguente in c053 e avanza
 * c1b7 di 2. Uscita: HL = puntatore (avanzato se il ramo è preso),
 * A = L d'ingresso oppure il codice estratto. */
void sub_01c5(struct z80 *z)
{
    uint16_t p = (uint16_t)(M(COPIONE_PTR) | (M(COPIONE_PTR + 1) << 8));
    z->a = z->l;
    SET_HL(z, p);
    if (leggi(p) != z->a)
        return;                        /* ret nz: quasi sempre */
    z->a = leggi(p + 1);
    p += 2;
    M(COPIONE_PTR) = (uint8_t)p;
    M(COPIONE_PTR + 1) = (uint8_t)(p >> 8);
    M(COPIONE_COD) = z->a;
    SET_HL(z, p);
}

/* 0x07DE: avanzamento dell'animazione di un record puntato da HL
 * (14 byte: 0 codice, 1 codice precedente, 2-3 coppia di indici, 4 indice
 * composto, 6 passo corrente, 7 contatore, 8-13 parametri del passo),
 * con le tabelle ROM a 9555 indicizzate dal codice. IX = oggetto: il bit 4
 * di (IX+0) salta l'aggiornamento della durata, il bit 3 fonde i nibble.
 * Uscita: HL = indirizzo del campo 6; A, BC, DE secondo il ramo (vedi coda). */
void sub_07de(struct z80 *z)
{
    uint16_t hl = HL(z);
    uint8_t ix0 = M(z->ix);
    uint8_t a, cy;
    uint8_t c = leggi(hl);             /* codice (resta in C) */
    uint8_t b;                         /* indice composto 4·rec2+rec3 */
    int de_intatto = 0;                /* ramo 07EE: DE del chiamante */
    uint16_t de = DE(z);
    hl++;
    if (c == leggi(hl)) {              /* codice invariato */
        hl++;
        a = (uint8_t)(leggi(hl) * 4); hl++;
        a = (uint8_t)(a + leggi(hl)); b = a; hl++;
        if (a == leggi(hl)) {          /* indice invariato: solo avanzamento */
            de_intatto = 1;
            goto avanzamento;
        }
        M(hl) = a;
    } else {                           /* codice cambiato: lo si annota */
        M(hl) = c; hl++;
        a = (uint8_t)(leggi(hl) * 4); hl++;
        a = (uint8_t)(a + leggi(hl)); b = a; hl++;
        M(hl) = a;
    }
    hl += 3;                           /* campo 7 */
    M(hl) = 0xe0; hl++;
    de = hl;                           /* campo 8: destinazione */
    {
        uint16_t tab = (uint16_t)(0x9555 + 2 * c);
        uint16_t p = (uint16_t)(leggi(tab) | (leggi(tab + 1) << 8));
        p += (uint8_t)(3 * b);         /* solo il riporto dell'ultima somma */
        a = leggi(p); p++;
        M(de) = a; de++;               /* campo 8: durata */
        M(de) = a; de++;               /* campo 9: ricarica */
        if (a & 0x80) {                /* variante: seconda tabella per codice&3 */
            c &= 3;
            uint16_t p2 = (uint16_t)(leggi(p) | (leggi(p + 1) << 8));
            p = (uint16_t)(p2 + 2 * c);
        }
        M(de) = leggi(p); p++; de++;   /* campo 10 */
        M(de) = leggi(p);              /* campo 11 (DE fermo) */
        hl = (uint16_t)(de - 7);       /* torna al campo 4 */
    }
avanzamento:                           /* 0842 */
    hl += 2;                           /* campo 6 */
    {
        uint16_t campo6 = hl;          /* push hl */
        uint8_t passo = leggi(hl);
        hl++;                          /* campo 7 */
        M(hl) = (uint8_t)(leggi(hl) + 1);
        if (passo >= M(hl)) {          /* contatore non scaduto */
            z->a = passo;
            z->b = b; z->c = c;
            if (!de_intatto)
                SET_DE(z, de);
            SET_HL(z, campo6);
            return;
        }
        M(hl) = 0; hl++;               /* campo 8 */
        uint8_t d8 = leggi(hl); hl++;  /* campo 9 */
        a = (uint8_t)(leggi(hl) & 0x7f);
        cy = a & 1; a >>= 1;           /* rra (riporto azzerato da and) */
        uint8_t e8 = a;
        if (!(ix0 & 0x10)) {           /* aggiorna la durata (campo 9) */
            a = (uint8_t)(e8 - 1);     /* dec a: lo Z deciderà il ricarico */
            int zero = (a == 0);
            uint8_t cy2 = a >> 7;      /* rla */
            a = (uint8_t)((a << 1) | cy);
            cy = cy2;
            if (zero)
                a = d8;                /* durata ricaricata dal campo 8 */
            M(hl) = a;
            cy2 = a & 1;               /* rra */
            a = (uint8_t)((cy << 7) | (a >> 1));
            cy = cy2;
        }
        e8--;
        hl++;                          /* campo 10 */
        uint16_t bc = (uint16_t)(leggi(hl) | (leggi(hl + 1) << 8));
        hl += 2;                       /* campo 12 */
        if (!cy) {                     /* passo diretto: terna a BC + 3·E */
            uint16_t dst = hl;
            uint16_t p = (uint16_t)(2 * e8);
            p = (uint16_t)(p + e8);    /* add a,l col riporto in H */
            p = (uint16_t)(p + bc);
            M(dst) = leggi(p); p++; dst++;
            M(dst) = leggi(p); p++;
            a = leggi(p);
            M(campo6) = a;             /* pop hl; ld (hl),a */
            z->a = a; SET_BC(z, bc);
            SET_DE(z, dst); SET_HL(z, campo6);
            return;
        }
        /* passo indiretto: indice a (BC), coppia a parola(BC)+2·E */
        a = leggi(bc); bc++;
        uint16_t dst = hl;
        uint16_t p = (uint16_t)(2 * e8 + bc);
        c = a;                         /* ld c,a */
        M(dst) = leggi(p); p++; dst++; /* campo 12 */
        a = c;
        b = (uint8_t)(bc >> 8);
        if (ix0 & 0x08) {              /* fonde i nibble col campo 13 */
            b = (uint8_t)(a & 0xf0);
            a = (uint8_t)((M(dst) & 0x0f) | b);
        }
        M(dst) = a;                    /* campo 13 */
        a = leggi(p);
        M(campo6) = a;
        z->a = a; z->b = b; z->c = c;
        SET_DE(z, dst); SET_HL(z, campo6);
    }
}

/* Coda di richieste audio/effetti c054-c056 (3 slot). */
#define CODA_C054 0xc054

/* 0x004F: scrive C nel primo slot libero (byte 0) fra c054-c056; se sono
 * tutti occupati scrive comunque a c057. Uscita: HL = slot scritto,
 * A = ultimo byte letto, B = slot restanti al momento della scrittura. */
void sub_004f(struct z80 *z)
{
    uint16_t hl = CODA_C054;
    uint8_t a = 0, b = 3;
    for (;;) {
        a = M(hl);
        if (a == 0)
            break;
        hl++;
        if (--b == 0)
            break;
    }
    M(hl) = z->c;
    z->a = a;
    z->b = b;
    SET_HL(z, hl);
}
