/* Routine del Z80 principale fra 0x4000 e 0x4fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* 0x4AF6: inizializza i campi di un oggetto a IX.
 * IX+04 = E, IX+05 = ff, IX+1B = E, IX+1C = ff, IX+1D = B, IX+1E = C,
 * IX+00 = A (stato). Registri invariati. */
void sub_4af6(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + 0x04) = z->e;
    M(ix + 0x05) = 0xff;
    M(ix + 0x1b) = z->e;
    M(ix + 0x1c) = 0xff;
    M(ix + 0x1d) = z->b;
    M(ix + 0x1e) = z->c;
    M(ix + 0x00) = z->a;
}

/* 0x4B0E: da un indice di cella (riga nel nibble alto, colonna nel basso)
 * alle coordinate in pixel.
 * Uscita: H = ((~A) - 0x10) & 0xf0 (y), L = (A & 0x0f)·16 - 8 (x), A = L. */
void sub_4b0e(struct z80 *z)
{
    uint8_t cella = z->a;
    z->h = (uint8_t)(((uint8_t)~cella - 0x10) & 0xf0);
    z->l = (uint8_t)(((cella & 0x0f) << 4) - 8);
    z->a = z->l;
}

/* Tabella ROM delle soglie, crescente (letta finché A >= soglia). */
#define SOGLIE_9484 0x9484

/* 0x4722: C = 7 + numero di soglie consecutive della tabella 9484 minori o
 * uguali ad A (dalla prima, fino alla prima maggiore).
 * Uscita: A = C, HL dopo la soglia maggiore; B invariato. */
void sub_4722(struct z80 *z)
{
    uint16_t hl = SOGLIE_9484;
    uint8_t c = 6, soglia;
    do {
        c++;
        soglia = leggi(hl++);
    } while (z->a >= soglia);
    z->c = c;
    z->a = c;
    SET_HL(z, hl);
}

/* Slot degli oggetti animati: 17 da 0x18 byte a c2c1. */
#define SLOT_INIZIO   0xc2c1
#define SLOT_PASSO    0x18
#define NUM_SLOT      0x11
/* Tabella ROM di puntatori alle sequenze di animazione. */
#define SEQUENZE_83FA 0x83fa
#define FINE_RIGA     0x80
#define FINE_SEQUENZA 0xff

/* 0x4DEE: cambia la sequenza di animazione corrente.
 * Ingresso: HL record, A nuovo numero + 1. Se (HL+1) vale già A-1 esce
 * (A = A-1, HL = HL+1). Altrimenti lo scrive e scorre la sequenza A-1
 * della tabella 83FA: ogni riga è «tipo, valori..., 0x80», l'ultima è
 * seguita da 0xff. Per ogni slot attivo (bit 7 di +0) con tipo (+6) uguale
 * a quello della riga scrive in +7 il valore successivo della riga
 * (ripartendo dal primo quando incontra 0x80).
 * Uscita: A = 0xff, B = 0, C = tipo dell'ultima corrispondenza (o
 * invariato), DE sul byte 0xff, HL sul tipo dell'ultima riga + 1,
 * IX = c2c1 + 17·0x18. */
void sub_4dee(struct z80 *z)
{
    uint16_t hl = HL(z) + 1;
    uint8_t a = z->a - 1;
    if (a == M(hl)) {
        z->a = a;
        SET_HL(z, hl);
        return;
    }
    M(hl) = a;
    z->a = a;
    SET_HL(z, SEQUENZE_83FA);
    sub_0008(z);
    sub_0010(z);
    uint16_t de = DE(z);
    for (;;) {
        uint8_t tipo = leggi(de++);
        uint16_t ix = SLOT_INIZIO;
        hl = de;
        for (int i = 0; i < NUM_SLOT; i++, ix += SLOT_PASSO) {
            if (!(M(ix) & 0x80) || tipo != M(ix + 6))
                continue;
            z->c = tipo;
            uint8_t v = leggi(hl);
            if (v == FINE_RIGA) {
                hl = de;
                v = leggi(hl);
            }
            M(ix + 7) = v;
            hl++;
        }
        z->ix = ix;
        while (leggi(hl++) != FINE_RIGA)
            ;
        int fine = leggi(hl) == FINE_SEQUENZA;
        uint16_t t = hl;
        hl = de;
        de = t;
        if (fine)
            break;
    }
    z->a = FINE_SEQUENZA;
    z->b = 0;
    SET_DE(z, de);
    SET_HL(z, hl);
}

/* Posizione di riferimento (colonna, riga) e maschera delle direzioni. */
#define RIF_X         0xc83c
#define RIF_Y         0xc843
#define DIREZIONI     0xc2b5
/* Oggetti di partenza: 17 da 0x30 byte a c4fe (x a +0e, y a +15). */
#define OGGETTI       0xc4fe
#define OGGETTO_PASSO 0x30

/* 0x4E31: per ogni slot c2c1 con byte +0 >= 0xc0 copia x e y dell'oggetto
 * corrispondente (c4fe + i·0x30) in +2, +3 e riempie +4..+6 con sub_24b1:
 * E = (x - rif_x) / 2 con il segno nel bit 7, A = y - rif_y col prestito
 * nel riporto, C = bit 0 se x < rif_x, bit 1 se y < rif_y. In c2b5 si
 * accende il bit C (direzione) di ogni slot trattato.
 * Uscita: A = 1, BC = 0030, DE = rif_y:rif_x, HL = c2c1 + 17·0x18,
 * IX = c4fe + 17·0x30. */
void sub_4e31(struct z80 *z)
{
    uint8_t rif_x = M(RIF_X), rif_y = M(RIF_Y);
    M(DIREZIONI) = 0;
    uint16_t hl = SLOT_INIZIO, ix = OGGETTI;
    for (int i = NUM_SLOT; i > 0; i--, hl += SLOT_PASSO, ix += OGGETTO_PASSO) {
        if (M(hl) < 0xc0)
            continue;
        uint8_t c = 0;
        uint8_t x = M(ix + 0x0e), y = M(ix + 0x15);
        M(hl + 2) = x;
        M(hl + 3) = y;
        int prestito_x = x < rif_x, prestito_y = y < rif_y;
        if (prestito_x) c |= 1;
        if (prestito_y) c |= 2;
        /* rra col prestito di sub e */
        z->e = (uint8_t)(((uint8_t)(x - rif_x) >> 1) | (prestito_x << 7));
        z->a = (uint8_t)(y - rif_y);
        z->b = (uint8_t)i;
        z->c = c;
        SET_HL(z, hl + 3);
        z->f = (uint8_t)((z->f & ~1) | prestito_y);   /* riporto di sub d */
        sub_24b1(z);
        /* dopo sub_24b1 (scf) i rla danno 1 << C d'ingresso */
        M(DIREZIONI) |= (uint8_t)(1 << c);
    }
    z->a = 1;
    SET_BC(z, OGGETTO_PASSO);
    z->d = rif_y;
    z->e = rif_x;
    SET_HL(z, hl);
    z->ix = ix;
}

/* Variabili usate da 4E87 (significato non ancora ricostruito). */
#define TABELLA_PTR   0xc2a9   /* parola: contatore, ×4 dà in D il passo */
#define PASSO_ATTUALE 0xc2ab
#define PTR_RIGHE     0xc2ac   /* parola: puntatore a righe di 2 byte */
#define STATO_GIOCO   0xc459

/* 0x4E87: con D = byte alto di 4·parola(c2a9), se diverso da c2ab lo
 * memorizza (e se vale 0x20 avanza di 2 il puntatore c2ac); poi verifica
 * due bit della riga puntata da c2ac (colonna (D-1)>>3 & 3, bit (D-1)&7
 * contato dall'alto, nelle righe +0 e +6): con un bit acceso, se c459 ha i
 * bit 6-7 spenti e (c2af) < (c2ae) & 0x1f, accende in c459 i bit trovati
 * (bit 7 riga +0, bit 6 riga +6). Uscite intermedie con i registri del
 * punto di uscita, come nel codice. */
void sub_4e87(struct z80 *z)
{
    SET_HL(z, TABELLA_PTR);
    sub_0010(z);                        /* DE = parola, HL = c2ab */
    SET_DE(z, DE(z) * 4);
    z->a = z->d;
    if (z->a == M(PASSO_ATTUALE))
        return;
    if (z->a == 0x20) {
        uint16_t p = (uint16_t)(M(PTR_RIGHE) | (M(PTR_RIGHE + 1) << 8)) + 2;
        M(PTR_RIGHE) = p & 0xff;
        M(PTR_RIGHE + 1) = p >> 8;
        SET_DE(z, p);
    }
    M(PASSO_ATTUALE) = z->a;
    uint8_t v = z->a - 1;
    z->c = (v & 0x18) >> 3;
    uint8_t passi = (v & 7) + 1;
    SET_HL(z, PTR_RIGHE);
    sub_0010(z);                        /* DE = puntatore, HL = c2ae */
    uint16_t ix = (uint16_t)(DE(z) + z->c);
    z->d = passi;
    /* add a,a ripetuto: il riporto finale è il bit 8-passi */
    int alto = (leggi(ix) >> (8 - passi)) & 1;
    ix += 6;
    uint8_t a = leggi(ix);
    int basso = (a >> (8 - passi)) & 1;
    z->a = (uint8_t)(a << passi);
    z->b = 0;
    z->c = 6;
    z->ix = ix;
    /* rl e, rr e, rr e: bit 7 dalla riga +0, bit 6 dalla riga +6 */
    z->e = (uint8_t)((alto << 7) | (basso << 6));
    if (z->e == 0)
        return;
    z->a = M(STATO_GIOCO) & 0xc0;
    if (z->a)
        return;
    uint16_t hl = HL(z);
    z->c = M(hl) & 0x1f;
    hl++;
    SET_HL(z, hl);
    uint8_t limite = M(hl);
    z->a = (uint8_t)(limite - z->c);
    if (limite >= z->c)
        return;
    z->a = M(STATO_GIOCO) |= z->e;
    SET_HL(z, STATO_GIOCO);
}

/* 0x452B: programma un canale di coppie (registro, valore).
 * Ingresso: HL = tabella dei record da 3 byte (numero di coppie,
 * destinazione), B = indice del record, D = codice scritto in (IX+0),
 * IX = canale (+2 contatore, +3 limite, +4/5 lista delle coppie).
 * Legge la coppia numero contatore dalla lista (0xff come secondo byte =
 * riavvolgimento alla testa della lista) e la copia nella destinazione,
 * per «numero di coppie» volte. Uscita: A = contatore prima
 * dell'incremento, HL dopo l'ultima coppia letta; BC e DE invariati. */
void sub_452b(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + 0) = z->d;
    uint16_t rec = (uint16_t)(HL(z) + 3 * z->b);   /* rst 08 + add hl,de */
    uint8_t coppie = leggi(rec);
    uint16_t dest = (uint16_t)(leggi(rec + 1) | (leggi(rec + 2) << 8));
    uint8_t contatore = M(ix + 2);
    M(ix + 2) = (uint8_t)(contatore + 1);
    if (contatore == M(ix + 3))
        M(ix + 2) = 0;
    uint16_t lista = (uint16_t)(M(ix + 4) | (M(ix + 5) << 8));
    uint16_t p = (uint16_t)(lista + 2 * contatore);
    for (unsigned resto = coppie ? coppie : 256; resto > 0; ) {
        uint8_t reg = leggi(p);
        uint8_t val = leggi(p + 1);
        p += 2;
        if (val == 0xff) {     /* riavvolgimento, non consuma il conteggio */
            p = lista;
            continue;
        }
        M(dest) = reg;
        M(dest + 1) = val;
        dest += 2;
        resto--;
    }
    z->a = contatore;
    SET_HL(z, p);
}

/* Tabelle ROM di 0x472E: record per indice C, parole per c2ae, oggetti. */
#define TAB_RECORD_488D  0x488d
#define TAB_PAROLE_8B2D  0x8b2d
#define TAB_OGGETTI_48AF 0x48af

/* 0x472E: inizializza il record numero C (tabella 488D): campo +4 = 0x7f,
 * campi +1 e +8 = 0, parola della tabella 8B2D (indice = bit 5-7 di c2ae)
 * nei campi +9/+10; poi IX = parola della tabella 48AF (indice C) e
 * HL d'ingresso salvato in (IX+0E)/(IX+15).
 * Uscita: A = C, HL invariato, DE = IX = parola di 48AF; BC invariati. */
void sub_472e(struct z80 *z)
{
    uint16_t hl_in = HL(z);
    z->a = z->c;
    SET_HL(z, TAB_RECORD_488D);
    sub_0008(z);               /* rst 08 */
    sub_0010(z);               /* rst 10: DE = record */
    uint16_t rec = DE(z);
    M(rec + 4) = 0x7f;
    M(rec + 1) = 0;
    M(rec + 8) = 0;
    z->a = (uint8_t)((M(0xc2ae) >> 5) & 7);    /* rlca ×3, and 7 */
    SET_HL(z, TAB_PAROLE_8B2D);
    sub_0008(z);
    sub_0010(z);               /* DE = parola */
    M(rec + 9) = z->e;
    M(rec + 10) = z->d;
    z->a = z->c;
    SET_HL(z, TAB_OGGETTI_48AF);
    sub_0008(z);
    sub_0010(z);               /* DE = oggetto */
    z->ix = DE(z);
    M(z->ix + 0x0e) = (uint8_t)(hl_in >> 8);
    M(z->ix + 0x15) = (uint8_t)hl_in;
    SET_HL(z, hl_in);
}

/* Cifre a video del punteggio/tempo e contatori associati. */
#define CIFRE_C061 0xc061
#define AZZERA_C239 0xc239

/* 0x4D37: converte il valore a 32 bit DE:HL in cifre decimali a c061
 * (meno significativa per prima, terminatore 0xff): per ogni cifra una
 * divisione per 10 con 32 scorrimenti. Con più di 9 cifre azzera
 * c239-c23c e scrive solo il terminatore. Uscita normale: A = L = ultima
 * cifra (quoziente finale), DE = 0, B = 0, C = cifre scritte,
 * IX sul terminatore. */
void sub_4d37(struct z80 *z)
{
    uint16_t ix = CIFRE_C061;
    uint8_t conta = 0;
    uint32_t v = ((uint32_t)DE(z) << 16) | HL(z);
    for (;;) {
        if (conta >= 9) {      /* troppe cifre */
            for (int i = 0; i < 4; i++)
                M(AZZERA_C239 + i) = 0;
            M(CIFRE_C061) = 0xff;
            z->a = 0xff;
            z->b = 0;
            z->c = conta;
            SET_DE(z, (uint16_t)(v >> 16));
            SET_HL(z, AZZERA_C239 + 4);
            z->ix = ix;
            return;
        }
        uint8_t resto = 0;     /* divisione per 10: v = v/10, resto in A */
        for (int b = 0; b < 32; b++) {
            uint8_t alto = (uint8_t)(v >> 31);
            v <<= 1;
            resto = (uint8_t)((resto << 1) | alto);
            if (resto >= 10) { /* sub 10 senza prestito: bit del quoziente */
                resto -= 10;
                v |= 1;
            }
        }
        M(ix) = resto;
        ix++;
        conta++;
        if (v >= 10)
            continue;
        if (v != 0) {          /* ultima cifra non nulla */
            M(ix) = (uint8_t)v;
            ix++;
        }
        M(ix) = 0xff;
        z->a = (uint8_t)v;
        z->b = 0;
        z->c = conta;
        SET_DE(z, 0);
        SET_HL(z, (uint16_t)v);
        z->ix = ix;
        return;
    }
}

/* 0x4D87: scrive 8 byte letti in avanti da (DE) in (HL) all'indietro,
 * nel piano alto con +0x30 e attributo C nel piano basso; dal
 * terminatore 0xff in poi azzera il piano alto (senza attributo).
 * Uscita: A = ultimo carattere + 0x30 oppure 0xff, B = 0,
 * DE e HL avanzati/arretrati di conseguenza. */
void sub_4d87(struct z80 *z)
{
    uint16_t src = DE(z), dst = HL(z);
    uint8_t a = 0;
    int b = 8;
    while (b > 0) {
        a = leggi(src);
        src++;
        if (a == 0xff)
            break;
        a = (uint8_t)(a + 0x30);
        M(dst) = a;
        M(dst & ~0x0400) = z->c;
        dst = (uint16_t)((dst | 0x0400) - 1);  /* set 2,h; dec hl */
        b--;
    }
    while (b > 0) {            /* coda 4D9C: azzera il resto */
        M(dst) = 0;
        dst--;
        b--;
    }
    z->a = a;
    z->b = 0;
    SET_DE(z, src);
    SET_HL(z, dst);
}

/* 0x4398: registra un gestore in uno di due banchi di record: con il bit 4
 * di A spento tabella 43B9 e maschera a c09b, acceso tabella 43C5 e c0b5;
 * record da 3 byte all'indice A & 0x0f (parola di destinazione, bit di
 * maschera). OR del terzo byte in (BC), (c464) alla parola, 0 alla
 * parola+2. Uscita: A = 0, BC = maschera, DE = parola+2, HL = record+2. */
void sub_4398(struct z80 *z)
{
    uint16_t tab = 0x43b9, bc = 0xc09b;
    if (z->a & 0x10) {
        tab = 0x43c5;
        bc = 0xc0b5;
    }
    uint16_t rec = (uint16_t)(tab + 3 * (z->a & 0x0f));
    uint16_t de = (uint16_t)(leggi(rec) | (leggi(rec + 1) << 8));
    M(bc) = (uint8_t)(M(bc) | leggi(rec + 2));
    M(de) = M(0xc464);
    M(de + 2) = 0;
    z->a = 0;
    SET_BC(z, bc);
    SET_DE(z, de + 2);
    SET_HL(z, rec + 2);
}

/* 0x4D05: cifre del punteggio c239/c23b a c061 (sub_4d37); se c045 & 6 le
 * ricopia a video con sub_4d87 (da d44a, o d45f col bit 0 di c045 acceso),
 * attributo 0x71. Uscita: A = 0 e C = c045 se nessuna copia; altrimenti
 * registri di sub_4d87 con C = 0x71. */
void sub_4d05(struct z80 *z)
{
    SET_DE(z, (uint16_t)(M(0xc23b) | (M(0xc23c) << 8)));
    SET_HL(z, (uint16_t)(M(0xc239) | (M(0xc23a) << 8)));
    sub_4d37(z);
    uint8_t a = M(0xc045);
    z->c = a;
    if ((a & 6) == 0) {
        z->a = 0;              /* and 006h */
        return;
    }
    z->a = (uint8_t)(a & 6);
    uint8_t cy = (uint8_t)(z->c & 1);          /* rrc c */
    z->c = (uint8_t)((z->c >> 1) | (cy << 7));
    SET_HL(z, cy ? 0xd45f : 0xd44a);
    SET_DE(z, 0xc061);
    z->c = 0x71;
    sub_4d87(z);
}
