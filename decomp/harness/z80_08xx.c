/* Routine del Z80 principale fra 0x0800 e 0x0fff: lo smistamento d'urto
 * 08A2 e i gestori della tabella inline 08AE (lotto 7, compito 11).
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). IX = oggetto (slot da 0x30 byte a c46e). */
#include <stddef.h>

#include "routine.h"

/* Campi dell'oggetto usati dai gestori d'urto (offsetti da IX).
 * +0D/+0E e +14/+15 sono le posizioni (sottopixel, pixel) Y e X;
 * +0B/+0C e +12/+13 le velocità; +16..+19 i margini del riquadro
 * copiati da 0602; +1D lo stato, +1E i bit di direzione. */
#define OGG_SOTTO_Y   0x0d
#define OGG_Y         0x0e
#define OGG_SOTTO_X   0x14
#define OGG_X         0x15
#define OGG_VEL_Y     0x0b   /* parola +0B/+0C */
#define OGG_VEL_X     0x12   /* parola +12/+13 */
#define OGG_MARG_ALTO 0x16
#define OGG_MARG_SIN  0x17
#define OGG_MARG_DES  0x18
#define OGG_MARG_BAS  0x19
#define OGG_STATO     0x1d
#define OGG_DIR       0x1e

/* 0x08D8 (tabella 08AE, indici 0 e F): se lo stato è fra 3 e 6 (6 escluso)
 * e i bit 4-5 delle celle d'urto (C) sono spenti, lo stato diventa 6.
 * Uscita: A = stato, oppure C & 30 se si arriva al confronto. */
void sub_08d8(struct z80 *z)
{
    uint8_t a = M(z->ix + OGG_STATO);
    z->a = a;
    if (a >= 7 || a < 3 || a == 6)
        return;
    z->a = (uint8_t)(z->c & 0x30);
    if (z->a)
        return;
    M(z->ix + OGG_STATO) = 6;
}

/* 0x094D (tabella 08AE, indice 2): spinge l'oggetto fuori dalla cella
 * urtata sul lato di minor penetrazione: corregge X (IX+15) o Y (IX+0E)
 * dei margini residui, azzera i sottopixel (e la velocità X nel ramo X),
 * stato 2 (ramo X) o 1/2 dal bit 7 di (IX+0C) (ramo Y), direzione aggiornata.
 * Uscita: A = correzione o stato; D/E = X e penetrazione X; H/L = Y e
 * penetrazione Y. */
void sub_094d(struct z80 *z)
{
    uint16_t ix = z->ix;
    M(ix + 0x02) = 0;
    uint8_t h = M(ix + OGG_Y);
    uint8_t l = (uint8_t)((h & 0x0f) + M(ix + OGG_MARG_ALTO)
                          + M(ix + OGG_MARG_BAS) - 0x0f);
    uint8_t d = M(ix + OGG_X);
    uint8_t e = (uint8_t)(d - (uint8_t)(((uint8_t)(d - 8) & 0xf0) + 8));
    e = (uint8_t)(e + M(ix + OGG_MARG_SIN) + M(ix + OGG_MARG_DES) - 0x0f);
    z->h = h; z->l = l;
    z->d = d; z->e = e;
    if (e < l) {                       /* penetrazione X minore: corregge X */
        z->a = (uint8_t)(d - e);
        M(ix + OGG_X) = z->a;
        M(ix + OGG_SOTTO_X) = 0;
        M(ix + OGG_VEL_X) = 0;
        M(ix + OGG_VEL_X + 1) = 0;
        M(ix + OGG_STATO) = 2;
        M(ix + OGG_DIR) = 0;
    } else {                           /* corregge Y */
        M(ix + OGG_Y) = (uint8_t)(h - l);
        M(ix + OGG_SOTTO_Y) = 0;
        z->a = (M(ix + 0x0c) & 0x80) ? 2 : 1;
        M(ix + OGG_STATO) = z->a;
        M(ix + OGG_DIR) &= (uint8_t)~0x02;
    }
}

/* 0x0A98 (tabella 08AE, indice 3): appoggio sul pavimento: Y alla base
 * della riga meno i margini verticali, sottopixel azzerato; se lo stato
 * non è 2: stato 1, (IX+2) = 0, bit 1 della direzione spento.
 * Uscita: A = stato precedente, H = Y d'ingresso & f0. */
void sub_0a98(struct z80 *z)
{
    uint16_t ix = z->ix;
    uint8_t h = (uint8_t)(M(ix + OGG_Y) & 0xf0);
    z->h = h;
    M(ix + OGG_Y) = (uint8_t)(h + 0x10 - M(ix + OGG_MARG_ALTO)
                              - M(ix + OGG_MARG_BAS) - 1);
    M(ix + OGG_SOTTO_Y) = 0;
    z->a = M(ix + OGG_STATO);
    if (z->a == 2)
        return;
    M(ix + OGG_STATO) = 1;
    M(ix + 0x02) = 0;
    M(ix + OGG_DIR) &= (uint8_t)~0x02;
}

/* 0x0AC3 (tabella 08AE, indice C): urto col soffitto: Y alla riga sotto
 * meno il margine alto, sottopixel = f0; se stato - 4 non è 0, 1 o 3:
 * stato 1, bit 1 della direzione acceso, (IX+2) = 0; altrimenti azzera
 * (IX+0B)/(IX+0C). Uscita: A = stato - 4, H = Y d'ingresso. */
void sub_0ac3(struct z80 *z)
{
    uint16_t ix = z->ix;
    uint8_t h = M(ix + OGG_Y);
    z->h = h;
    M(ix + OGG_Y) = (uint8_t)((h & 0xf0) + 0x10 - M(ix + OGG_MARG_ALTO));
    M(ix + OGG_SOTTO_Y) = 0xf0;
    uint8_t a = (uint8_t)(M(ix + OGG_STATO) - 4);
    z->a = a;
    if (a < 2 || a == 3) {
        M(ix + 0x0c) = 0;
        M(ix + 0x0b) = 0;
    } else {
        M(ix + OGG_STATO) = 1;
        M(ix + OGG_DIR) |= 0x02;
        M(ix + 0x02) = 0;
    }
}

/* 0x0AF9 (tabella 08AE, indice 9): urto sul lato sinistro: X alla colonna
 * accanto meno il margine sinistro, sottopixel azzerato; se lo stato - 4 è
 * almeno 2: stato 2, direzione 1/3 dal bit 7 di (IX+0C), velocità X
 * azzerata; altrimenti (IX+7) = ff se il bit 0 della direzione è spento,
 * poi azzera velocità X e (IX+0B)/(IX+0C). Uscita: A = stato (o stato - 4),
 * C = direzione se scritta, D = X d'ingresso. */
void sub_0af9(struct z80 *z)
{
    uint16_t ix = z->ix;
    uint8_t d = M(ix + OGG_X);
    z->d = d;
    M(ix + OGG_X) = (uint8_t)((((uint8_t)(d - 8)) & 0xf0) + 8 + 0x10
                              - M(ix + OGG_MARG_SIN));
    M(ix + OGG_SOTTO_X) = 0;
    z->a = M(ix + OGG_STATO);
    if (z->a != 0) {
        z->a = (uint8_t)(z->a - 4);
        if (z->a >= 2) {
            M(ix + OGG_STATO) = 2;
            M(ix + 0x02) = 0;
            z->c = (M(ix + 0x0c) & 0x80) ? 1 : 3;
            M(ix + OGG_DIR) = z->c;
            M(ix + OGG_VEL_X) = 0;
            M(ix + OGG_VEL_X + 1) = 0;
            return;
        }
        if (!(M(ix + OGG_DIR) & 0x01))
            M(ix + 0x07) = 0xff;
    }
    M(ix + OGG_VEL_X) = 0;
    M(ix + OGG_VEL_X + 1) = 0;
    M(ix + 0x0b) = 0;
    M(ix + 0x0c) = 0;
}

/* 0x0B57 (tabella 08AE, indice 6): come 0AF9 sul lato destro: X alla
 * colonna meno i margini orizzontali, direzione 0/2 (bit 7 di (IX+0C)
 * acceso → 0), (IX+7) = ff se il bit 0 della direzione è acceso.
 * Uscita: A = stato (o stato - 4), C = direzione se scritta,
 * D = X allineato alla colonna. */
void sub_0b57(struct z80 *z)
{
    uint16_t ix = z->ix;
    uint8_t d = (uint8_t)((((uint8_t)(M(ix + OGG_X) - 8)) & 0xf0) + 8);
    z->d = d;
    M(ix + OGG_X) = (uint8_t)(0x10 - M(ix + OGG_MARG_SIN)
                              - M(ix + OGG_MARG_DES) - 1 + d);
    M(ix + OGG_SOTTO_X) = 0;
    z->a = M(ix + OGG_STATO);
    if (z->a != 0) {
        z->a = (uint8_t)(z->a - 4);
        if (z->a >= 2) {
            M(ix + OGG_STATO) = 2;
            M(ix + 0x02) = 0;
            z->c = (M(ix + 0x0c) & 0x80) ? 0 : 2;
            M(ix + OGG_DIR) = z->c;
            M(ix + OGG_VEL_X) = 0;
            M(ix + OGG_VEL_X + 1) = 0;
            return;
        }
        if (M(ix + OGG_DIR) & 0x01)
            M(ix + 0x07) = 0xff;
    }
    M(ix + 0x0b) = 0;
    M(ix + 0x0c) = 0;
    M(ix + OGG_VEL_X) = 0;
    M(ix + OGG_VEL_X + 1) = 0;
}

/* La tabella inline a 08AE (16 parole dopo la `call 08CE` di 08A2):
 * 08D8 08ED 094D 0A98 0A24 0CDA 0B57 0BBB 09B3 0AF9 0CCE 0BF6 0AC3 0C2D
 * 0C7C 08D8. Gli indici con NULL (gestori mai eseguiti nelle tracce:
 * 08ED, 09B3, 0A24, 0BBB, 0BF6, 0C2D, 0C7C, 0CCE, 0CDA) non sono tradotti
 * in questo lotto: lo stato resta intatto e un'eventuale cattura futura
 * farebbe fallire il confronto del banco in modo visibile. */
static void (*const gestori_08ae[16])(struct z80 *) = {
    sub_08d8, NULL,     sub_094d, sub_0a98,
    NULL,     NULL,     sub_0b57, NULL,
    NULL,     sub_0af9, NULL,     NULL,
    sub_0ac3, NULL,     NULL,     sub_08d8,
};

/* 0x08A2: smistamento d'urto dell'oggetto IX. Se il bit 5 di (IX+0) è
 * spento esce subito; altrimenti E = C & 0F (le celle d'urto raccolte da
 * 0602) sceglie il gestore nella tabella inline a 08AE tramite 08CE.
 * 08CE (pop hl / push de / ret) usa l'indirizzo di rientro come base
 * della tabella: qui la tabella si legge dalla ROM e il salto diventa la
 * chiamata del gestore tradotto; 08CE lascia DE = indirizzo del gestore
 * e HL = voce della tabella + 1, riprodotti prima della chiamata.
 * Uscita: registri del gestore (o intatti sul ret z). */
void sub_08a2(struct z80 *z)
{
    if (!(M(z->ix) & 0x20))
        return;
    uint8_t e = (uint8_t)(z->c & 0x0f);
    z->a = e;
    z->e = e;
    uint16_t voce = (uint16_t)(0x08ae + 2 * e);
    SET_HL(z, voce + 1);
    SET_DE(z, (uint16_t)(leggi(voce) | (leggi(voce + 1) << 8)));
    if (gestori_08ae[e])
        gestori_08ae[e](z);
}
