/* Routine del Z80 principale fra 0x7000 e 0x7fff, tradotte in C leggibile.
 * Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY
 * all'uscita (le bandiere F no). */
#include "routine.h"

/* Due aree di destinazione scelte dal bit 0 di C. */
#define AREA_PARI    0xd486
#define AREA_DISPARI 0xd496

/* 0x7EFE: scrive una cella in due piani.
 * Ingresso: D riga, E colonna, C attributo. A = bit mescolati di E e D:
 * quattro volte rr l, rra, rr h, rra (riporto iniziale nullo) su HL = DE;
 * l'indirizzo è HL così ruotato + 0x20·D (D = 0 vale 256) + E + d486
 * (d496 se C è dispari). Si scrive A all'indirizzo e C allo stesso col bit
 * 2 di H spento.
 * Uscita: B = D, DE e C invariati, HL = indirizzo col bit 2 di H spento. */
void sub_7efe(struct z80 *z)
{
    uint8_t h = z->d, l = z->e, a = 0, cy = 0, nuovo;
    for (int i = 0; i < 4; i++) {
        nuovo = l & 1; l = (uint8_t)((l >> 1) | (cy << 7)); cy = nuovo;
        nuovo = a & 1; a = (uint8_t)((a >> 1) | (cy << 7)); cy = nuovo;
        nuovo = h & 1; h = (uint8_t)((h >> 1) | (cy << 7)); cy = nuovo;
        nuovo = a & 1; a = (uint8_t)((a >> 1) | (cy << 7)); cy = nuovo;
    }
    unsigned righe = z->d ? z->d : 256;
    uint16_t hl = (uint16_t)((h << 8) | l);
    hl = (uint16_t)(hl + 0x20 * righe);
    hl = (uint16_t)(hl + ((z->c & 1) ? AREA_DISPARI : AREA_PARI) + z->e);
    M(hl) = a;
    hl &= ~0x0400;
    M(hl) = z->c;
    z->a = a;
    z->b = z->d;
    SET_HL(z, hl);
}

/* Tabella ROM delle destinazioni a video delle stringhe d'esito. */
#define TAB_ESITI_7E3A 0x7e3a

/* 0x7E32: destinazione numero A dalla tabella di parole 7E3A.
 * Uscita: DE = parola, HL invariato (push/pop); A invariato. */
void sub_7e32(struct z80 *z)
{
    uint16_t hl = HL(z);
    SET_HL(z, TAB_ESITI_7E3A);
    sub_0008(z);               /* rst 08 */
    sub_0010(z);               /* rst 10: DE = parola */
    SET_HL(z, hl);
}
