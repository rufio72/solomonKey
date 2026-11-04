/* Corpo 07 (2CCD) dell'avvio, lotti H4b2 e H4b3 (brief 46 e 47): macro e
 * prototipi condivisi dai moduli attivita_attract07.c (ingresso, fasi
 * brevi, attese e fase 12), attivita_attract07a.c (fasi 0, 1, 3),
 * attivita_attract07b.c (fasi 4 e 7), attivita_attract07c.c (fasi 13, 14,
 * 15, 17, 21) e attivita_attract07d.c (fasi 18, 19, 20, 22, 24, 25). Le fasi sono funzioni senza argomenti: il puntatore allo
 * slot (`pop hl`) non serve a nessuna delle fasi tradotte. */
#ifndef ATTIVITA_ATTRACT07_H
#define ATTIVITA_ATTRACT07_H

#include <stdint.h>
#include "nemici.h"

/* Come NEMICI_PUNTO ma senza controllo d'impronta (copia della macro locale
 * di attivita_attract.c): per i ritorni dentro routine tradotte senza
 * punti interni. */
#define PUNTO_LARGO(da, a) \
    do { if (attract_punto_largo) attract_punto_largo((da), (a)); } while (0)

void fase_07_3861(void);   /* fase 0 */
void fase_07_34fa(void);   /* fase 1 */
void fase_2dee(void);      /* fase 2 (anche passo 1 del 24, lotto 55) */
void fase_07_37d7(void);   /* fase 3 */
void fase_07_358f(void);   /* fase 4 */
void fase_3225(void);      /* fase 6 (anche passo 4 del 24, lotto 55) */
void fase_07_3a7d(void);   /* fase 7 */
void fase_07_2dce(void);   /* fase 13 (07c) */
void fase_07_31c5(void);   /* fase 14 */
void fase_07_2e0a(void);   /* fase 15 */
void fase_07_2e9c(void);   /* fasi 17 e 21 */
void fase_07_3709(void);   /* fase 18 (07d) */
void fase_07_3829(void);   /* fase 19 */
void fase_07_321e(void);   /* fase 20 */
void fase_07_2ef5(void);   /* fase 22 */
void fase_07_2f1a(void);   /* fase 24 */
void fase_07_2e1e(uint8_t slot);   /* fase 25 */

/* Fondale 322A con i ritorni delle NMI (attivita_attract07.c, fasi 6 e 08). */
struct z80;
void fondale_322a(struct z80 *z);

/* Attesa a cancello 2E8B (attivita_attract07.c). */
void attesa_07(uint16_t hl, uint8_t c, uint16_t sito);

#endif
