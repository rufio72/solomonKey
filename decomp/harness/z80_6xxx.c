/* Routine del Z80 principale fra 0x6000 e 0x6fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"
#include "nemici.h"

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
        NEMICI_PONI_RIPORTO(z, 0);             /* 6C3B-6C3C: and 04, rrca su zero */
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
        NEMICI_PONI_RIPORTO(z, 0);
        return;
    }
    z->c = 0x30;                               /* suono 30 via rst 28 */
    sub_0028(z);
    NEMICI_PONI_RIPORTO(z, 1);                 /* rst 28 preserva AF: riporto del rrca */
}

/* ------------------------------------------------------------------ */
/* Lotto 38 (compito 38): contatori di quadro. */

/* 0x6976: incrementa tre contatori. (c1ba + H di 8·(c235 parola)); poi,
 * con la parola W = (c23a) e il byte (c23c): offset = 0x31 se (c23c) non
 * è zero o H(W) > 0D, altrimenti H(4·W); (c1ca + offset) e
 * (c1fc + (c243)). Uscita: A = C = (c243), B = 0, HL = c1fc + B:C,
 * DE = parola W (se (c23c) != 0) oppure c23c. */
void sub_6976(struct z80 *z)
{
    uint16_t w8 = (uint16_t)((M(0xc235) | (M(0xc236) << 8)) << 3);   /* 6976-697B */
    SET_DE(z, w8 >> 8);                        /* 697C-697D */
    M(0xc1ba + (w8 >> 8))++;                   /* 697F-6983 */
    SET_HL(z, 0xc23a);                         /* 6984 */
    sub_0010(z);                               /* 6987: DE = parola, HL = c23c */
    SET_BC(z, 0x0031);                         /* 6988 */
    z->a = M(0xc23c);                          /* 698B */
    if (z->a == 0) {                           /* 698C-698D */
        uint16_t w = DE(z);                    /* 698F ex de,hl */
        SET_DE(z, HL(z));
        SET_HL(z, w);
        z->a = 0x0d;                           /* 6990 */
        if (!(0x0d < (w >> 8))) {              /* 6992-6993 cp h; jr c */
            uint16_t w4 = (uint16_t)(w << 2);  /* 6995-6996 */
            SET_HL(z, w4);
            z->c = (uint8_t)(w4 >> 8);         /* 6997 */
        }
    }
    M(0xc1ca + BC(z))++;                       /* 6998-699C */
    z->a = M(0xc243);                          /* 69A0 */
    z->c = z->a;                               /* 69A3 */
    SET_HL(z, 0xc1fc + BC(z));                 /* 699D, 69A4 */
    M(HL(z))++;                                /* 69A5 */
}
