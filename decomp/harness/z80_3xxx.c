/* Routine del Z80 principale fra 0x3000 e 0x3fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* Tabella in ROM di 33 valori (indici 0..0x20), letta a specchio. */
#define TABELLA_3111 0x3111

/* 0x30EF: valore di una tabella simmetrica su 64 passi (forma d'onda).
 * Ingresso: A angolo. Indice = A & 0x3f, ribaltato (^0x3f) se il bit 5 è
 * acceso. Uscita: HL = 3111 + indice, A = valore letto. */
void sub_30ef(struct z80 *z)
{
    uint8_t indice = z->a & 0x3f;
    if (indice & 0x20)
        indice ^= 0x3f;
    uint16_t hl = TABELLA_3111 + indice;
    SET_HL(z, hl);
    z->a = leggi(hl);
}

/* 0x3A1C: due cifre ASCII di A (0..99 atteso).
 * Uscita: D = '0' + A/10, E = '0' + A%10, A = D. */
void sub_3a1c(struct z80 *z)
{
    uint8_t decine = z->a / 10, unita = z->a % 10;
    z->d = (uint8_t)('0' + decine);
    z->e = (uint8_t)('0' + unita);
    z->a = z->d;
}

/* Tempo in BCD (3 byte, cifre basse prima) e sue cifre separate. */
#define TEMPO_BCD    0xc093
#define TEMPO_CIFRE  0xc061   /* 6 cifre, meno significativa prima, poi terminatore */
#define FINE_CIFRE   0xff

/* 0x3F10: scompone le 3 parole BCD di c093-c095 in 6 cifre a c061-c066 e
 * scrive 0xff subito dopo la cifra non nulla più significativa (dopo
 * c061 se sono tutte zero).
 * Uscita: A = 0xff, B = posizione (1..6, 0 se tutte zero), C = byte c095,
 * DE = indirizzo del terminatore, HL = c096. */
void sub_3f10(struct z80 *z)
{
    for (int i = 0; i < 3; i++) {
        uint8_t c = M(TEMPO_BCD + i);
        M(TEMPO_CIFRE + 2 * i) = c & 0x0f;
        M(TEMPO_CIFRE + 2 * i + 1) = c >> 4;
        z->c = c;
    }
    /* dall'ultima cifra verso la prima, la prima non nulla */
    uint8_t b = 6;
    uint16_t hl = TEMPO_CIFRE + 5;
    for (; b > 0; b--, hl--)
        if (M(hl) != 0)
            break;
    if (b == 0)
        hl = TEMPO_CIFRE;   /* nessuna cifra non nulla: si ferma su c061 */
    M(hl + 1) = FINE_CIFRE;
    z->b = b;
    z->a = FINE_CIFRE;
    SET_DE(z, hl + 1);
    SET_HL(z, TEMPO_BCD + 3);
}

/* 0x34D4: azzera 10 byte da HL in due piani: per ogni byte scrive 0 a HL
 * con il bit 2 di H spento e poi a HL con il bit 2 di H acceso (prima
 * scrittura all'indirizzo d'ingresso, come nel codice).
 * Uscita: B = 0, C = 0, HL avanzato di 10 col bit 2 di H acceso. */
void sub_34d4(struct z80 *z)
{
    uint16_t hl = HL(z);
    for (int i = 0; i < 10; i++) {
        M(hl) = 0;
        hl &= ~0x0400;
        M(hl) = 0;
        hl |= 0x0400;
        hl++;
    }
    SET_BC(z, 0);
    SET_HL(z, hl);
}

/* 0x380A: riempie 16 byte da HL con 0xff.
 * Uscita: B = 0, C = 0xff, HL avanzato di 16. */
void sub_380a(struct z80 *z)
{
    uint16_t hl = HL(z);
    for (int i = 0; i < 16; i++)
        M(hl++) = 0xff;
    SET_BC(z, 0x00ff);
    SET_HL(z, hl);
}

/* Tabelle ROM di coppie di byte. */
#define TABELLA_8578 0x8578   /* indici 0..7 e 8, 9, 10 per A >= 0x40 */
#define TABELLA_858E 0x858e   /* indici A-8 per 8 <= A < 0x40 */

/* 0x3A2E: coppia di byte scelta da A.
 * A < 8: indice A in 8578; A < 0x40: indice A-8 in 858E; altrimenti indice
 * 8 (A < c0), 9 (A < f8) o 10 in 8578.
 * Uscita: B, C = coppia, A = DE = indice, HL sul secondo byte. */
void sub_3a2e(struct z80 *z)
{
    uint8_t a = z->a, indice;
    uint16_t tabella = TABELLA_8578;
    if (a < 0x08)
        indice = a;
    else if (a < 0x40) {
        indice = a - 8;
        tabella = TABELLA_858E;
    } else if (a < 0xc0)
        indice = 8;
    else if (a < 0xf8)
        indice = 9;
    else
        indice = 10;
    z->a = indice;
    SET_HL(z, tabella);
    sub_0008(z);            /* rst 08: DE = indice, HL = tabella + 2·indice */
    uint16_t hl = HL(z);
    z->b = leggi(hl);
    z->c = leggi(hl + 1);
    SET_HL(z, hl + 1);
}

/* 0x3000: estensione con segno e moltiplicazione per 4.
 * Ingresso: A, e il riporto del chiamante come segno (1 = negativo).
 * Uscita: HL = A·4 oppure (0xff00 | A)·4; A invariato. */
void sub_3000(struct z80 *z)
{
    uint16_t hl = z->a;
    if (z->f & 1)              /* riporto in ingresso: byte alto 0xff */
        hl |= 0xff00;
    SET_HL(z, (uint16_t)(hl << 2));
}

/* 0x3101: moltiplicazione seriale a 8 bit (byte alto del prodotto).
 * Ingresso: H moltiplicando, L moltiplicatore. Uscita: A = H =
 * byte alto di H·L raddoppiato più il riporto dell'ultimo passo;
 * L = bit bassi del prodotto (ruotati dentro durante i passi);
 * D = B d'ingresso (copia di lavoro), B invariato. */
void sub_3101(struct z80 *z)
{
    uint8_t a = 0, cy = 0, l = z->l, h = z->h;
    z->d = z->b;               /* ld d,b */
    for (int i = 0; i < 8; i++) {
        uint8_t bit = (uint8_t)(l & 1);        /* rr l */
        l = (uint8_t)((cy << 7) | (l >> 1));
        cy = bit;
        if (cy) {                              /* add a,h */
            unsigned s = (unsigned)a + h;
            a = (uint8_t)s;
            cy = (uint8_t)(s >> 8);
        }
        bit = (uint8_t)(a & 1);                /* rra */
        a = (uint8_t)((cy << 7) | (a >> 1));
        cy = bit;
    }
    a = (uint8_t)((a << 1) | cy);              /* rla */
    z->a = z->h = a;
    z->l = l;
}

/* 0x31CE: scrive la scritta fissa a 31E1 («P.0...», attributo 0x70) in
 * dfa7 con sub_6771, poi la cifra c238 in ASCII a dfac.
 * Uscita: A = c238 + 0x30, HL = dfac; C e DE da sub_6771. */
#define CIFRA_C238 0xc238
void sub_31ce(struct z80 *z)
{
    SET_HL(z, 0x31e1);
    SET_DE(z, 0xdfa7);
    sub_6771(z);
    SET_HL(z, 0xdfac);
    z->a = (uint8_t)(M(CIFRA_C238) + 0x30);
    M(0xdfac) = z->a;
}

/* Tabella ROM delle sovrapposizioni del fondale (record da 4 byte). */
#define TAB_SOVRAPPOSIZIONI 0xf138

/* 0x325E: decodifica la sovrapposizione numero A (1-based) del fondale:
 * record F138[(A-1)·2] = flusso (DE), poi C e B = parametro di sub_326c.
 * Uscita: registri di sub_326c. */
void sub_325e(struct z80 *z)
{
    z->a = (uint8_t)((z->a - 1) * 2);
    SET_HL(z, TAB_SOVRAPPOSIZIONI);
    sub_0008(z);               /* rst 08 */
    sub_0010(z);               /* rst 10: DE = flusso */
    uint16_t hl = HL(z);
    z->c = leggi(hl);
    hl++;
    z->b = leggi(hl);
    SET_HL(z, hl);
    sub_326c(z);
}

/* Palette del fondale in RAM e bit di consegna alla NMI. */
#define PALETTE_BG   0xc9ca   /* 0x80 byte copiati a e500 dalla NMI */
#define CONSEGNA_NMI 0xc044   /* bit 6: la NMI ricopia la palette */

/* 0x33AA: azzera la palette di stanza c9ca-ca49 e accende il bit 6 di
 * c044 (coda comune con sub_33ba). Uscita: A = 0, B = 0, HL = c044. */
void sub_33aa(struct z80 *z)
{
    for (int i = 0; i < 0x80; i++)
        M(PALETTE_BG + i) = 0;
    M(CONSEGNA_NMI) |= 0x40;
    z->a = 0;
    z->b = 0;
    SET_HL(z, CONSEGNA_NMI);
}

/* rld: scambio di nibble fra A e il byte in RAM (per sub_33ba). */
static uint8_t rld_ram(uint16_t addr, uint8_t a)
{
    uint8_t m = M(addr);
    M(addr) = (uint8_t)((m << 4) | (a & 0x0f));
    return (uint8_t)((a & 0xf0) | (m >> 4));
}

/* 0x33BA: carica il record palette della stanza c243 (tabella F000):
 * azzera e802, copia 2·(testa & 0x7f) byte del record a c9ca e, se il bit
 * 0 del C d'ingresso è acceso, fa una passata di rld/srl sui byte copiati;
 * coda comune 33B4: bit 6 di c044 (vedi DATI.md, il passo dipende da C,
 * non dal bit 7 della testa). Uscita: BC = 0, DE dopo l'ultimo byte
 * copiato, HL = c044. */
void sub_33ba(struct z80 *z)
{
    M(0xe802) = 0;
    z->a = M(0xc243);
    SET_HL(z, 0xf000);
    sub_0008(z);               /* rst 08 */
    sub_0018(z);               /* rst 18: HL = parola → record */
    uint16_t rec = HL(z);
    uint8_t cy = (uint8_t)(z->c & 1);          /* rrc c: serve solo il riporto */
    uint8_t conta = (uint8_t)(leggi(rec) << 1);/* add a,a */
    uint8_t a = (uint8_t)((cy << 7) | (conta >> 1));  /* rra */
    uint16_t src = rec + 1, dst = PALETTE_BG;
    for (unsigned i = 0; i < conta; i++)       /* ldir */
        M(dst + i) = leggi(src + i);
    dst = (uint16_t)(dst + conta);
    cy = (uint8_t)(a >> 7);                    /* add a,a */
    a = (uint8_t)(a << 1);
    if (cy) {                                  /* passata rld (bit 0 di C) */
        uint8_t giri = a;
        uint16_t p = PALETTE_BG;
        a = 0;
        for (unsigned i = giri; i > 0; i--) {
            a = rld_ram(p, a);
            a >>= 1;                           /* srl a */
            a = rld_ram(p, a);
            a >>= 1;
            a = rld_ram(p, a);
            p++;
        }
    }
    M(CONSEGNA_NMI) |= 0x40;                   /* coda 33B4 */
    z->a = a;
    SET_BC(z, 0);
    SET_DE(z, dst);
    SET_HL(z, CONSEGNA_NMI);
}

/* 0x34E1: scrive i caratteri A e A+1 uno sotto l'altro (righe da 0x20)
 * nel piano alto df00|L, attributo 0x70 nel piano basso.
 * Uscita: A = L = L d'ingresso (L + 0x40 + 0xc0 modulo 256),
 * H = A d'ingresso + 2, B = 0. */
void sub_34e1(struct z80 *z)
{
    uint8_t a = z->a, l = z->l;
    for (int i = 0; i < 2; i++) {
        uint16_t v = (uint16_t)(0xdf00 | l);
        M(v) = a;
        M(v & ~0x0400) = 0x70;
        a++;
        l = (uint8_t)(l + 0x20);
    }
    z->h = a;                  /* ld h,a dentro il ciclo */
    z->l = z->a = (uint8_t)(l + 0xc0);
    z->b = 0;
}

/* 0x36BD: ripete sub_2402 (parola DE a HL, DE += BC) finché il contatore
 * A non arriva a 0 (lo Z viene dal dec a dentro sub_2402). */
void sub_36bd(struct z80 *z)
{
    do
        sub_2402(z);
    while (z->a != 0);
}
