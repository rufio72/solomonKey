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

/* Un asse del moto dell'oggetto (0602, ramo 06A7): 7 byte da `base`:
 * +0 accelerazione (segnata a 8 bit), +1/+2 velocità massima, +3/+4
 * velocità corrente, +5/+6 posizione (sottopixel, pixel). La velocità
 * nuova = corrente + accelerazione; se supera la massima (confronto col
 * segno del Z80: la massima è negata quando la velocità è negativa e il
 * riporto della `sbc` è ribaltato) resta la corrente; la posizione avanza
 * della velocità scelta. */
static void sub_0602_asse(uint16_t base)
{
    uint16_t vel = (uint16_t)(M(base + 3) | (M(base + 4) << 8));
    uint16_t bc = (uint16_t)(vel + (int16_t)(int8_t)M(base));
    uint16_t massima = (uint16_t)(M(base + 1) | (M(base + 2) << 8));
    if (bc & 0x8000)
        massima = (uint16_t)-massima;
    int superata = massima < bc;           /* riporto della sbc hl,bc */
    if (bc & 0x8000)
        superata = !superata;              /* ccf */
    if (!superata) {
        M(base + 3) = (uint8_t)bc;
        M(base + 4) = (uint8_t)(bc >> 8);
    } else {
        bc = vel;
    }
    uint16_t pos = (uint16_t)((M(base + 5) | (M(base + 6) << 8)) + bc);
    M(base + 5) = (uint8_t)pos;
    M(base + 6) = (uint8_t)(pos >> 8);
}

/* 0x0602 (chiamata dalla NMI, 0126, per ognuno dei 24 slot oggetto da
 * 0x30 byte a partire da c46e, con HL = IX = slot): motore per-oggetto.
 * Se (HL) < c0 esce (slot non attivo). Altrimenti: (IX+6) = 4·(IX+1D) +
 * (IX+1E); aggiorna il sottorecord +4..+7 (due coppie valore/bersaglio):
 * se cambiate, sincronizza le copie e carica i campi del moto (+08..+13)
 * dal record parola(a746 + 2·(+4)) + 3·(+6), coi bit 7..2 del primo byte
 * a scegliere i campi (bit 1: puntatore ulteriore scelto da (+4) & 3);
 * se stabili, integra i due assi del moto (sub_0602_asse). Poi copia i
 * 4 margini del riquadro dal record ROM 9495 + 8·A (A = (IX+4), o (IX+3)
 * se ≥ 9) in +16..+19, raccoglie in C i bit 7 delle 6 celle della griglia
 * c0bd attorno al pixel ((IX+0E), (IX+15)) coi margini (bit 0-2 di B dai
 * cambi di riga/colonna), scrive C in +1A, chiama sub_08a2 (urti) e
 * prosegue in sub_07de (+1B..: animazione). Uscita: registri di sub_07de. */
void sub_0602(struct z80 *z)
{
    uint16_t o = HL(z);
    z->a = M(o);
    if (z->a < 0xc0)
        return;
    M(z->ix + 0x06) = (uint8_t)(4 * M(z->ix + 0x1d) + M(z->ix + 0x1e));

    /* sottorecord +4..+7: coppie (valore, copia) ai byte +4/+5 e +6/+7 */
    uint8_t v0 = M(o + 4), v1 = M(o + 6);
    if (M(o + 4) == M(o + 5) && M(o + 6) == M(o + 7)) {
        /* coppie stabili (06A7): integrazione del moto sui due assi */
        sub_0602_asse(o + 0x08);           /* asse Y: +08..+0E */
        sub_0602_asse(o + 0x0f);           /* asse X: +0F..+15 */
    } else {
        /* coppie cambiate (0627/062C): si sincronizzano le copie e si
         * caricano i campi del moto dal record in ROM */
        M(o + 5) = v0;
        M(o + 7) = v1;
        uint16_t rec = (uint16_t)((leggi(0xa746 + 2 * v0)
                                   | (leggi(0xa747 + 2 * v0) << 8)) + 3 * v1);
        uint8_t scelta = leggi(rec);
        uint16_t p = (uint16_t)(leggi(rec + 1) | (leggi(rec + 2) << 8));
        if (scelta & 0x02) {               /* puntatore ulteriore da (+4) & 3 */
            uint16_t q = (uint16_t)(p + 2 * (v0 & 3));
            p = (uint16_t)(leggi(q) | (leggi(q + 1) << 8));
        }
        /* campi del moto scelti dai bit 7..2: bit 7 → +08; 6 → +09, +0A;
         * 5 → +0B, +0C; 4 → +0F; 3 → +10, +11; 2 → +12, +13 */
        static const struct { uint8_t bit, campo; } copie[10] = {
            { 0x80, 0x08 }, { 0x40, 0x09 }, { 0x40, 0x0a },
            { 0x20, 0x0b }, { 0x20, 0x0c }, { 0x10, 0x0f },
            { 0x08, 0x10 }, { 0x08, 0x11 }, { 0x04, 0x12 }, { 0x04, 0x13 },
        };
        for (unsigned i = 0; i < 10; i++, p++)
            if (scelta & copie[i].bit)
                M(o + copie[i].campo) = leggi(p);
    }

    /* margini del riquadro dal record ROM 9495 + 8·A in +16..+19 */
    uint8_t a = M(z->ix + 0x04);
    if (a >= 9)
        a = M(z->ix + 0x03);
    uint16_t marg = (uint16_t)(0x9495 + 8 * a);
    for (unsigned i = 0; i < 4; i++)
        M(o + 0x16 + i) = leggi((uint16_t)(marg + i));

    /* celle della griglia c0bd attorno al riquadro: B = sconfinamenti
     * (bit 0: il bordo alto cade a inizio riga; bit 1: il riquadro copre
     * due righe; bit 2: due colonne), C = bit 7 delle 6 celle lette */
    uint8_t b = 0;
    uint8_t alto = (uint8_t)(M(z->ix + 0x0e) + M(o + 0x16));
    if (((uint8_t)(alto - 1) ^ alto) & 0xf0)
        b |= 0x01;
    uint8_t sin = (uint8_t)(M(z->ix + 0x15) + M(o + 0x17) - 8);
    if (((uint8_t)(sin + M(o + 0x18)) ^ sin) & 0xf0)
        b |= 0x04;
    sin += 8;
    if (((uint8_t)(alto + M(o + 0x19)) ^ alto) & 0xf0)
        b |= 0x02;
    uint8_t riga = (uint8_t)((uint8_t)(~alto - 0x10) & 0xf0);
    uint8_t spiazzo = (uint8_t)(riga + (((uint8_t)(sin + 8) >> 4) & 0x0f));
    uint16_t cella = (uint16_t)(0xc0bd + spiazzo);

    uint16_t pa = (uint16_t)(cella + ((b & 0x01) ? 0x10 : 0));
    uint8_t celle = (uint8_t)(M(pa) >> 7);
    if (b & 0x04)
        pa++;
    celle = (uint8_t)((celle << 1) | (M(pa) >> 7));
    uint16_t pb = cella;
    celle = (uint8_t)((celle << 1) | (M(pb) >> 7));
    if (b & 0x04)
        pb++;
    celle = (uint8_t)((celle << 1) | (M(pb) >> 7));
    uint16_t pc = pb;                      /* ex de,hl: si prosegue da pb */
    if (b & 0x02)
        pc -= 0x10;
    celle = (uint8_t)((celle << 1) | (M(pc) >> 7));
    if (b & 0x04)
        pc--;
    celle = (uint8_t)((celle << 1) | (M(pc) >> 7));

    M(o + 0x1a) = celle;
    z->a = celle;
    z->b = b;
    z->c = celle;
    SET_DE(z, pa);
    SET_HL(z, o + 0x1a);
    sub_08a2(z);                           /* urti (07D9) */
    SET_HL(z, o + 0x1b);
    sub_07de(z);                           /* prosecuzione a 07DE */
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

/* 0x0043 (dal gestore rst 28, 002B): filtro dei suoni in attract.
 * Se c045 & 6 è zero (attract) legge e604 (DSW1, bit 0 = Demo Sounds):
 * con il bit 0 acceso (suoni spenti) esce senza accodare; altrimenti
 * prosegue in sub_004f, che accoda C nella coda suoni c054-c057.
 * Uscita: nel ramo che esce A = DSW1 ruotato a destra; negli altri
 * i registri di sub_004f. */
void sub_0043(struct z80 *z)
{
    z->a = (uint8_t)(M(0xc045) & 0x06);
    if (z->a == 0) {
        uint8_t dsw1 = leggi_porta(0xe604);   /* DSW1: bit 0 = Demo Sounds */
        z->a = (uint8_t)((dsw1 >> 1) | ((dsw1 & 1) << 7));   /* rrca */
        if (dsw1 & 1)
            return;                           /* ret c: Demo Sounds Off */
    }
    sub_004f(z);
}
