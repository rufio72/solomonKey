/* Traduzione leggibile in C di routine del Z80 principale di Solomon's Key.
 * Non è un emulatore: si riproducono l'effetto sulla RAM e i registri
 * A, BC, DE, HL, IX, IY all'uscita (le bandiere F no). */
#include "ram.h"

/* Oggetti del gioco: 24 voci da 0x30 byte a partire da c46e. */
#define OGGETTI       0xc46e
#define NUM_OGGETTI   0x18
#define PASSO_OGGETTO 0x30
/* Campi di un oggetto (spiazzamenti dentro la voce). */
#define OGG_STATO     0x00   /* bit 7: oggetto attivo; bit 6: visibile */
#define OGG_Y         0x0e
#define OGG_X         0x15
#define OGG_CODICE    0x27   /* codice dello sprite */
#define OGG_ATTRIBUTI 0x28   /* colore e ribaltamenti */
/* Copia della tabella sprite in RAM (4 byte per sprite, formato di spriteram). */
#define TABELLA_SPRITE 0xc8ee
/* Variabile di sistema: il bit 5 indica lo schermo ribaltato. */
#define FLAG_SISTEMA  0xc045
#define FLAG_RIBALTATO 0x20

/* 0x05B3: costruisce la tabella sprite a c8ee dagli oggetti a c46e.
 * Oggetto non attivo: voce azzerata. Attivo ma non visibile: voce lasciata
 * com'è. Attivo e visibile: codice, attributi, y corretta di 1 pixel
 * (in su o in giù secondo il ribaltamento), x. */
void sub_05b3(struct z80 *z)
{
    uint8_t correzione_y = (M(FLAG_SISTEMA) & FLAG_RIBALTATO) ? 0xff : 0x01;
    uint16_t ogg = OGGETTI;
    uint16_t voce = TABELLA_SPRITE;
    uint8_t a = 0;

    for (int n = 0; n < NUM_OGGETTI; n++, ogg += PASSO_OGGETTO, voce += 4) {
        uint8_t stato = M(ogg + OGG_STATO);
        if (!(stato & 0x80)) {
            M(voce + 0) = M(voce + 1) = M(voce + 2) = M(voce + 3) = 0;
            a = 0;
        } else if (!(stato & 0x40)) {
            /* il codice originale fa due rla: in A resta stato << 2 | 1 */
            a = (uint8_t)((stato << 2) | 1);
        } else {
            M(voce + 0) = M(ogg + OGG_CODICE);
            M(voce + 1) = M(ogg + OGG_ATTRIBUTI);
            M(voce + 2) = (uint8_t)(M(ogg + OGG_Y) + correzione_y);
            M(voce + 3) = a = M(ogg + OGG_X);
        }
    }
    /* registri all'uscita come nell'originale */
    z->a = a;
    z->b = 0;
    z->c = correzione_y;
    SET_DE(z, PASSO_OGGETTO);
    SET_HL(z, voce);
    z->ix = ogg;
}

/* 0x2402: scrive la parola DE all'indirizzo HL, avanza HL di 2,
 * aggiunge BC a DE e decrementa A (contatore del chiamante). */
void sub_2402(struct z80 *z)
{
    uint16_t hl = HL(z);
    M(hl) = z->e;
    M(hl + 1) = z->d;
    SET_HL(z, hl + 2);
    SET_DE(z, DE(z) + BC(z));
    z->a--;
}
