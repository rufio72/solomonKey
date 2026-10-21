/* Routine del Z80 principale fra 0x2000 e 0x2fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* 0x237E: aggiorna un piccolo record a HL.
 * (HL) diventa ((HL) + 1) & 0x7f; la parola big-endian in (HL+1),(HL+2)
 * (byte alto prima) diminuisce di 0x0150.
 * Uscita: A = byte alto nuovo, BC = 0150, DE = parola nuova, HL = HL+1. */
void sub_237e(struct z80 *z)
{
    uint16_t hl = HL(z);
    M(hl) = (uint8_t)((M(hl) + 1) & 0x7f);
    uint16_t parola = (uint16_t)((M(hl + 1) << 8) | M(hl + 2));
    parola -= 0x0150;
    M(hl + 2) = parola & 0xff;
    M(hl + 1) = parola >> 8;
    z->a = parola >> 8;
    SET_BC(z, 0x0150);
    SET_DE(z, parola);
    SET_HL(z, hl + 1);
}

/* 0x23A1: somma a 16 bit della parola in DE alla parola in HL (byte basso
 * prima). Uscita: A = byte alto della somma, DE e HL avanzati di 2. */
void sub_23a1(struct z80 *z)
{
    uint16_t hl = HL(z), de = DE(z);
    uint16_t somma = (uint16_t)(leggi(hl) | (leggi(hl + 1) << 8)) +
                     (uint16_t)(leggi(de) | (leggi(de + 1) << 8));
    M(hl) = somma & 0xff;
    M(hl + 1) = somma >> 8;
    z->a = somma >> 8;
    SET_HL(z, hl + 2);
    SET_DE(z, de + 2);
}

/* 0x2418: indirizzo di una cella in una griglia di 32 colonne.
 * Ingresso: D riga, E colonna. Uscita: HL = 32·D + E, D = 0; A invariato. */
void sub_2418(struct z80 *z)
{
    SET_HL(z, 32 * z->d + z->e);
    z->d = 0;
}

/* 0x249E: spostamento di un record a HL.
 * (HL) += C; parola a HL+3 (byte basso prima) += D:C; (HL+5) += C.
 * Uscita: A = nuovo (HL+5), HL = HL + 5 + DE. */
void sub_249e(struct z80 *z)
{
    uint16_t hl = HL(z);
    M(hl) += z->c;
    uint16_t parola = (uint16_t)(M(hl + 3) | (M(hl + 4) << 8));
    parola += (uint16_t)((z->d << 8) | z->c);
    M(hl + 3) = parola & 0xff;
    M(hl + 4) = parola >> 8;
    z->a = M(hl + 5) = (uint8_t)(M(hl + 5) + z->c);
    SET_HL(z, hl + 5 + DE(z));
}

/* 0x24B1: riempie tre byte di un record a HL.
 * Il riporto in ingresso entra nel bit 7 di A (rra): serve davvero, quindi
 * si legge dal bit 0 di F.
 * (HL+1) = E, (HL+2) = A ruotato, (HL+3) = C.
 * Uscita: HL = HL+3, E = B, C = B = C+1, A = 0. */
void sub_24b1(struct z80 *z)
{
    uint16_t hl = HL(z);
    uint8_t a = (uint8_t)((z->a >> 1) | ((z->f & 1) << 7));
    M(hl + 1) = z->e;
    M(hl + 2) = a;
    M(hl + 3) = z->c;
    SET_HL(z, hl + 3);
    z->e = z->b;
    z->c++;
    z->b = z->c;
    z->a = 0;
}

/* 0x2368: valore di una tabella di byte moltiplicato per 12.
 * Ingresso: HL base, A indice. Uscita: HL = 12·(HL+A), DE = 2·(HL+A)
 * (byte letto anche in ROM); A invariato. */
void sub_2368(struct z80 *z)
{
    uint16_t valore = leggi((uint16_t)(HL(z) + z->a));
    SET_DE(z, 2 * valore);
    SET_HL(z, 12 * valore);
}

/* 0x23AC: azzera il byte a DE e quello a D:(E+0x20) (stessa pagina, riga
 * sotto in una griglia di 32 colonne).
 * Uscita: A = 0, HL = D:(E+0x20), DE = DE+1. */
void sub_23ac(struct z80 *z)
{
    uint16_t de = DE(z);
    M(de) = 0;
    z->l = (uint8_t)(z->e + 0x20);
    z->h = z->d;
    M(HL(z)) = 0;
    z->a = 0;
    SET_DE(z, de + 1);
}

/* 0x23F8: record di 3 byte in una tabella (rst 08 più un terzo di passo).
 * Ingresso: HL base, A indice. Il record sta a HL + 3·A: DE = parola,
 * C = terzo byte. Uscita: B = 0, A = 0, HL sul terzo byte. */
void sub_23f8(struct z80 *z)
{
    uint16_t hl = (uint16_t)(HL(z) + 3 * z->a);
    z->e = leggi(hl);
    z->d = leggi(hl + 1);
    z->c = leggi(hl + 2);
    SET_HL(z, hl + 2);
    z->b = 0;
    z->a = 0;
}

/* 0x240B: HL = 48·A (il codice fa 2·A + A, poi quattro raddoppi).
 * Uscita: BC = A, A invariato. */
void sub_240b(struct z80 *z)
{
    SET_BC(z, z->a);
    SET_HL(z, 48 * z->a);
}

/* 0x2457: separa i due nibble di E.
 * Uscita: D = (E >> 4)·2 + 2, E = A = (E & 0x0f)·2 - 1 (0xff se nibble 0). */
void sub_2457(struct z80 *z)
{
    uint8_t e = z->e;
    z->d = (uint8_t)(((e & 0xf0) >> 3) + 2);
    z->e = z->a = (uint8_t)(((e & 0x0f) << 1) - 1);
}

/* Tabelle ROM di 0x2B08: destinazioni video (indice B) e stringhe (indice C). */
#define TAB_DEST_2B29  0x2b29
#define TAB_TESTI_2B61 0x2b61

/* 0x2B08 (test RAM e titolo): copia la stringa numero C (tabella 2B61,
 * terminata da 0xff) nel piano alto della destinazione numero B (tabella
 * 2B29), attributo 0x40 nel piano basso; con il bit 7 di B acceso la
 * destinazione è HL del chiamante. Uscita: A = 0, DE sul terminatore,
 * HL dopo l'ultimo carattere scritto; B e C invariati. */
void sub_2b08(struct z80 *z)
{
    uint8_t b = z->b, c = z->c;
    if (!(b & 0x80)) {
        z->a = b;
        SET_HL(z, TAB_DEST_2B29);
        sub_0008(z);           /* rst 08 */
        sub_0018(z);           /* rst 18: HL = parola della tabella */
    }
    uint16_t dest = HL(z);
    z->a = c;
    SET_HL(z, TAB_TESTI_2B61);
    sub_0008(z);               /* rst 08 */
    sub_0010(z);               /* rst 10: DE = inizio della stringa */
    uint16_t src = DE(z);
    for (;;) {
        uint8_t ch = leggi(src);
        if (ch == 0xff)
            break;
        M(dest) = ch;
        M(dest & ~0x0400) = 0x40;
        dest = (uint16_t)((dest | 0x0400) + 1);
        src++;
    }
    z->a = 0;                  /* inc a sul terminatore */
    z->b = b; z->c = c;
    SET_DE(z, src);
    SET_HL(z, dest);
}

/* 0x2AF5 (test RAM/titolo): stampa 8 righe di testo con sub_2b08.
 * A è un vettore di bit ruotato col riporto in ingresso (`rra` con
 * push/pop af attorno a ogni chiamata): il bit estratto sceglie la
 * stringa C = 0x0c (riporto) o 0x0d; B va da 0x80 a 0x88, quindi
 * sub_2b08 usa sempre HL del chiamante come destinazione (che avanza
 * di riga in riga). Uscita: B = 0x88, C ultima stringa, A ruotato. */
void sub_2af5(struct z80 *z)
{
    uint8_t cy = (uint8_t)(z->f & 1);
    z->b = 0x80;
    do {
        uint8_t bit = (uint8_t)(z->a & 1);     /* rra */
        z->a = (uint8_t)((cy << 7) | (z->a >> 1));
        cy = bit;
        uint8_t a_sal = z->a;                  /* push af */
        uint8_t b_sal = z->b;
        z->c = (uint8_t)(cy ? 0x0c : 0x0d);
        sub_2b08(z);
        z->a = a_sal;                          /* pop af */
        z->b = (uint8_t)(b_sal + 1);           /* inc b */
    } while (!(z->b & 0x08));                  /* bit 3,b */
}

/* 0x2FF3: differenze di coordinate in passi di griglia.
 * Ingresso: HL e DE = due coppie (y, x). Uscita: HL = (L-H d'appoggio)·4,
 * DE = (L-E)·4, entrambe col segno dal prestito della sottrazione
 * (passato a sub_3000 come riporto in ingresso); A = seconda differenza. */
void sub_2ff3(struct z80 *z)
{
    uint8_t a = z->l;
    int prestito = a < z->e;
    z->a = (uint8_t)(a - z->e);
    z->e = z->h;
    z->f = (uint8_t)((z->f & ~1) | prestito);
    sub_3000(z);
    uint16_t t = HL(z);                        /* ex de,hl */
    SET_HL(z, DE(z));
    SET_DE(z, t);
    a = z->l;
    prestito = a < z->h;
    z->a = (uint8_t)(a - z->h);
    z->f = (uint8_t)((z->f & ~1) | prestito);
    sub_3000(z);
}

/* 0x2F74: aggancia una cella a video a un oggetto: salva H/L in
 * (IX+0E)/(IX+15), (IX+3) = 0x0a, inizializza i campi fissi con sub_4af6
 * (A = 0xc0 stato, E = 0x0a), poi passa all'oggetto dopo.
 * Uscita: A = L = L d'ingresso + 0x10, D = 0, E = 0x30, IX += 0x30;
 * B, C e H invariati. */
void sub_2f74(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + 0x0e) = z->h;
    M(ix + 0x15) = z->l;
    M(ix + 0x03) = 0x0a;
    z->d = 0;
    z->e = 0x0a;
    z->a = 0xc0;
    sub_4af6(z);
    z->a = (uint8_t)(z->l + 0x10);
    z->l = z->a;
    z->e = 0x30;
    z->ix = (uint16_t)(ix + 0x30);
}

/* 0x234A (da 2980): deposita una coppia e estrae il campo Lives.
 * (HL) = C, (HL+1) = A; A = bit 6-7 di B (con B = DSW1 a nibble scambiati
 * sono i bit 2-3 di DSW1 = Lives). Uscita: HL = HL+1. */
void sub_234a(struct z80 *z)
{
    uint16_t hl = HL(z);
    M(hl) = z->c;
    M(hl + 1) = z->a;
    SET_HL(z, hl + 1);
    z->a = (uint8_t)(((z->b << 2) | (z->b >> 6)) & 0x03);   /* rlca ×2, and 3 */
}

/* Tabelle ROM dei crediti per i campi monete di DSW1 (4 byte ciascuna). */
#define TAB_MONETE_29C7 0x29c7
#define TAB_VITE_29CB   0x29cb

/* 0x2980 (avvio, 28F4/28FA): estrae i campi di DSW1 nelle celle credito.
 * Azzera c046-c04b; legge e604 (DSW1: bit 4-5 Coin B, bit 6-7 Coin A,
 * bit 2-3 Lives); se Coin A = Coin B scrive 0x80 in c048-c049; c04a/c04b =
 * tabella 29C7 ai valori di Coin A e Coin B (via sub_234a), c04d =
 * tabella 29CB al valore di Lives.
 * Uscita: A = c04d, B = DSW1 a nibble scambiati, C = 29C7[Coin A],
 * DE = Lives, HL = 29CB + Lives. */
void sub_2980(struct z80 *z)
{
    for (int i = 0; i < 6; i++)
        M(0xc046 + i) = 0;
    uint8_t dsw1 = leggi_porta(0xe604);        /* DSW1 */
    uint8_t b = (uint8_t)((dsw1 << 4) | (dsw1 >> 4));      /* rlca ×4 */
    z->b = b;
    uint8_t coin_a = (uint8_t)(((b >> 2) | (b << 6)) & 0x03);
    if (((((b >> 2) | (b << 6)) ^ b) & 0x03) == 0) {
        M(0xc048) = 0x80;
        M(0xc049) = 0x80;
    }
    z->d = 0;
    z->e = coin_a;
    z->c = leggi((uint16_t)(TAB_MONETE_29C7 + coin_a));
    uint8_t coin_b = (uint8_t)(b & 0x03);
    z->e = coin_b;
    z->a = leggi((uint16_t)(TAB_MONETE_29C7 + coin_b));
    SET_HL(z, 0xc04a);
    sub_234a(z);                               /* c04a, c04b; A = Lives */
    z->e = z->a;
    SET_HL(z, (uint16_t)(TAB_VITE_29CB + z->e));
    z->a = leggi(HL(z));
    M(0xc04d) = z->a;
}

/* 0x2AE2 (attract, 2AAA): disegna gli 8 bit di e604 (DSW1) in d765 e di
 * e605 (DSW2) in d785 (riga video di servizio) via sub_2af5 (verificata).
 * Uscita: registri della seconda sub_2af5. */
void sub_2ae2(struct z80 *z)
{
    uint8_t dsw1 = leggi_porta(0xe604);        /* DSW1 */
    z->a = dsw1;
    SET_HL(z, 0xd765);
    sub_2af5(z);
    /* il riporto lasciato da sub_2af5 (ultimo bit estratto = bit 7 di
     * DSW1) entra nella rotazione della seconda chiamata */
    z->f = (uint8_t)((z->f & ~1) | (dsw1 >> 7));
    z->a = leggi_porta(0xe605);                /* DSW2 */
    SET_HL(z, 0xd785);
    sub_2af5(z);
}

/* 0x2BC3 (attract, 29EB/2AAD; coda esclusiva 2353-2367): lettura degli
 * ingressi di avvio: A = ((P1 | P2) & 0x3f) | (SYSTEM & 0x03), cioè i sei
 * tasti dei due giocatori più i due start.
 * Uscita: C = (P1 | P2) & 0x3f, HL = e602. */
void sub_2bc3(struct z80 *z)
{
    uint8_t c = (uint8_t)((leggi_porta(0xe600) |   /* P1 */
                           leggi_porta(0xe601)) &  /* P2 */
                          0x3f);
    z->c = c;
    z->a = (uint8_t)((leggi_porta(0xe602) & 0x03) | c);   /* SYSTEM */
    SET_HL(z, 0xe602);
}

/* 0x2C1A (da 2BC9/2BF3; l'ingresso è anche il ciclo): riempie BC byte da
 * HL con il pattern rotante D; a ogni byte una lettura a vuoto di e602
 * (A subito sovrascritto: probabile watchdog).
 * Uscita: A = 0, BC = 0, HL avanzato, D ruotato di BC posizioni. */
void sub_2c1a(struct z80 *z)
{
    uint16_t hl = HL(z), bc = BC(z);
    uint8_t d = z->d;
    do {
        M(hl) = d;
        d = (uint8_t)((d << 1) | (d >> 7));    /* rlc d */
        hl++;
        bc--;
        (void)leggi_porta(0xe602);             /* SYSTEM a vuoto: watchdog */
    } while (bc != 0);
    z->a = 0;                                  /* ld a,b; or c con BC = 0 */
    z->d = d;
    SET_HL(z, hl);
    SET_BC(z, 0);
}

/* 0x2BC9 (test RAM d'avvio, 2892/289E): per gli 8 pattern E = 1, 2, ...,
 * 0x80 riempie BC byte da HL con sub_2c1a e li rilegge a coppie: il primo
 * byte intero, del secondo solo il nibble basso (RAM video a 4 bit).
 * Il riporto d'uscita è l'esito (1 = sana, 0 = guasta, coda 2C16): il
 * banco non confronta F ma serve al chiamante, emulato su z->f.
 * Uscita: HL e BC d'ingresso; A = 0 e E = 1 se sana, A = byte in errore
 * e E = pattern se guasta; D = pattern di lavoro. */
void sub_2bc9(struct z80 *z)
{
    uint16_t hl0 = HL(z), bc0 = BC(z);
    uint8_t e = 1, d;
    for (;;) {
        z->d = e;
        sub_2c1a(z);                           /* riempie col pattern */
        SET_HL(z, hl0);                        /* pop hl / pop bc */
        SET_BC(z, bc0);
        d = e;
        uint16_t hl = hl0, bc = bc0;
        for (;;) {
            z->a = d;
            if (M(hl) != d)
                goto guasto;
            d = (uint8_t)((d << 1) | (d >> 7));
            hl++;
            bc--;
            z->a = (uint8_t)((d ^ M(hl)) & 0x0f);
            if (z->a != 0)
                goto guasto;
            hl++;
            bc--;
            d = (uint8_t)((d << 1) | (d >> 7));
            if (bc == 0)
                break;
        }
        z->a = 0;                              /* ld a,b; or c */
        uint8_t riporto = (uint8_t)(e >> 7);   /* rlc e */
        e = (uint8_t)((e << 1) | riporto);
        if (riporto) {
            z->d = d;
            z->e = e;
            z->f = (uint8_t)(z->f | 1);        /* riporto 1: RAM sana */
            return;
        }
    }
guasto:                                        /* coda 2C16: or a; pop; ret */
    z->d = d;
    z->e = e;
    SET_HL(z, hl0);
    SET_BC(z, bc0);
    z->f = (uint8_t)(z->f & ~1);               /* riporto 0: RAM guasta */
}

/* 0x2BF3 (test RAM d'avvio, 5 siti in 2843-28AD): come 2BC9 ma la
 * rilettura confronta ogni byte intero; nel ciclo di confronto una lettura
 * a vuoto di e602 per byte (watchdog). Riporto d'uscita come 2BC9.
 * Uscita: HL e BC d'ingresso; A = 0 e E = 1 se sana, A = byte in errore
 * e E = pattern se guasta; D = pattern di lavoro. */
void sub_2bf3(struct z80 *z)
{
    uint16_t hl0 = HL(z), bc0 = BC(z);
    uint8_t e = 1, d;
    for (;;) {
        z->d = e;
        sub_2c1a(z);
        SET_HL(z, hl0);
        SET_BC(z, bc0);
        d = e;
        uint16_t hl = hl0, bc = bc0;
        for (;;) {
            z->a = d;
            if (M(hl) != d)
                goto guasto;
            hl++;
            bc--;
            d = (uint8_t)((d << 1) | (d >> 7));
            (void)leggi_porta(0xe602);         /* SYSTEM a vuoto: watchdog */
            z->a = (uint8_t)(bc >> 8);         /* ld a,b; or c */
            if (bc == 0)
                break;
        }
        z->a = 0;
        uint8_t riporto = (uint8_t)(e >> 7);   /* rlc e */
        e = (uint8_t)((e << 1) | riporto);
        if (riporto) {
            z->d = d;
            z->e = e;
            z->f = (uint8_t)(z->f | 1);        /* riporto 1: RAM sana */
            return;
        }
    }
guasto:                                        /* coda 2C16 condivisa */
    z->d = d;
    z->e = e;
    SET_HL(z, hl0);
    SET_BC(z, bc0);
    z->f = (uint8_t)(z->f & ~1);               /* riporto 0: RAM guasta */
}

/* 0x2E11 (corpi di fase 2DFF/2E0B): c19e = 0 e confronto della stanza
 * corrente con quella chiesta. Lo Z del cp finale serve al chiamante
 * (il banco non confronta F): emulato su z->f insieme al riporto.
 * Uscita: A = c248, C = c249. */
void sub_2e11(struct z80 *z)
{
    M(0xc19e) = 0;
    z->c = M(0xc249);
    z->a = M(0xc248);
    z->f = (uint8_t)((z->a == z->c ? 0x40 : 0) | (z->a < z->c ? 1 : 0));
}

/* 0x2E90 (trampolino 2E8B dei corpi di fase): conto alla rovescia di un
 * cancello: decrementa (HL); se il contatore c467 non ha raggiunto C
 * ritorna, altrimenti azzera c467 e rincrementa (HL) (annulla il passo).
 * Uscita: A = c467 letto (0 nel ramo che annulla). */
void sub_2e90(struct z80 *z)
{
    uint16_t hl = HL(z);
    M(hl)--;
    z->a = M(0xc467);
    if (z->a < z->c)
        return;                                /* cp c; ret c */
    z->a = 0;
    M(0xc467) = 0;
    M(hl)++;
}

/* 0x2F57 (2F2D): carica il fondale di gioco: sub_3f88 con HL = 80D3
 * (palette di base), azzera d5c0-d61f (96 byte), registra i gestori 1 e 0
 * con sub_4398 e azzera il contatore c467.
 * Uscita: registri della seconda sub_4398. */
void sub_2f57(struct z80 *z)
{
    SET_HL(z, 0x80d3);
    sub_3f88(z);
    for (int i = 0; i < 0x60; i++)
        M(0xd5c0 + i) = 0;
    SET_HL(z, 0xd5c0 + 0x60);
    SET_BC(z, 0);
    z->a = 1;
    sub_4398(z);
    z->a = 0;
    M(0xc467) = 0;
    sub_4398(z);
}

/* 0x2551 (7650; altri siti in zone future): record di 3 byte all'indice
 * BC della tabella HL (anche ROM: il chiamante osservato passa HL=765B);
 * incrementa il contatore a DE+1 e lo confronta col byte del record.
 * Uscita: A = byte del record, DE = HL + 3·BC, HL = DE d'ingresso + 1;
 * riporto = byte ≥ contatore incrementato (il banco non confronta F ma
 * serve al chiamante, emulato su z->f). */
void sub_2551(struct z80 *z)
{
    uint16_t record = (uint16_t)(HL(z) + 3 * BC(z));
    uint16_t contatore = (uint16_t)(DE(z) + 1);
    M(contatore)++;
    z->a = leggi(record);
    uint8_t riporto = (uint8_t)(z->a >= M(contatore));  /* cp + ccf */
    z->f = (uint8_t)((z->f & ~1) | riporto);
    SET_DE(z, record);
    SET_HL(z, contatore);
}
