/* Routine del Z80 principale fra 0x6000 e 0x6fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* Tabella delle attività dello scheduler (64 byte). */
#define TABELLA_ATTIVITA 0xc000
#define LUNG_ATTIVITA    0x40

/* 0x673F: azzera la tabella delle attività c000-c03f.
 * Uscita: BC = 0, HL = c040; A invariato. */
void sub_673f(struct z80 *z)
{
    for (int i = 0; i < LUNG_ATTIVITA; i++)
        M(TABELLA_ATTIVITA + i) = 0;
    SET_BC(z, 0);
    SET_HL(z, TABELLA_ATTIVITA + LUNG_ATTIVITA);
}

/* 0x6771: scrive una stringa in due piani.
 * Ingresso: HL sorgente (primo byte = attributo, poi caratteri fino a
 * 0xff), DE destinazione. Per ogni carattere: (DE) = carattere, poi DE col
 * bit 2 di D spento = attributo, e DE col bit 2 di D acceso avanza di 1
 * (il codice si estende fino a 6784, oltre il ret a 677B).
 * Uscita: A = 0xff, C = attributo, HL dopo il terminatore, DE = ultima
 * destinazione. */
void sub_6771(struct z80 *z)
{
    uint16_t src = HL(z), dst = DE(z);
    z->c = leggi(src);
    for (;;) {
        uint8_t ch = leggi(++src);
        if (ch == 0xff)
            break;
        M(dst) = ch;
        dst &= ~0x0400;
        M(dst) = z->c;
        dst |= 0x0400;
        dst++;
    }
    z->a = 0xff;
    SET_HL(z, src + 1);
    SET_DE(z, dst);
}

/* 0x6C2C (6C1F; 551C statico): inizializzazione a passi di un oggetto
 * (IX = slot di appoggio, IY = oggetto, A = posa). Primo passo (bit 5 di
 * (IX+1) spento): lo accende, posa A in (IY+1D), azzera (IX+0D) e mette
 * in (IY+1E) il bit 2 di (IY+4) portato al bit 0 (direzione). I passi
 * successivi (6C48-6C5E: `set 0` su (IX+1) e (IY+0), oppure suono 30 via
 * rst 28) non sono mai eseguiti nelle tracce: dal disassembly statico. */
void sub_6c2c(struct z80 *z)
{
    if (!(M(z->ix + 1) & 0x20)) {
        M(z->ix + 1) |= 0x20;
        M(z->iy + 0x1d) = z->a;
        M(z->ix + 0x0d) = 0;
        uint8_t a = (uint8_t)(M(z->iy + 4) & 0x04);
        a = (uint8_t)((a >> 1) | (a << 7));    /* rrca */
        a = (uint8_t)((a >> 1) | (a << 7));    /* rrca */
        M(z->iy + 0x1e) = a;
        z->a = a;
        return;
    }
    /* mai eseguito nelle tracce */
    uint8_t a = M(z->ix + 1);
    uint8_t riporto = (uint8_t)(a & 1);        /* rrca */
    z->a = (uint8_t)((a >> 1) | (a << 7));
    if (!riporto) {
        M(z->ix + 1) |= 0x01;
        M(z->iy + 0) |= 0x01;
        M(z->ix + 0x0d) = 0;
        return;
    }
    z->c = 0x30;                               /* suono 30 via rst 28 */
    sub_0028(z);
}
