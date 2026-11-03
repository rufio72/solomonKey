/* Corpo 07 (2CCD) dell'avvio, lotto H4b2 (brief 46): macro e prototipi
 * condivisi dai moduli attivita_attract07.c (ingresso, fasi brevi, attese
 * e fase 12), attivita_attract07a.c (fasi 0, 1, 3) e attivita_attract07b.c
 * (fasi 4 e 7). Le fasi sono funzioni senza argomenti: il puntatore allo
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
void fase_07_37d7(void);   /* fase 3 */
void fase_07_358f(void);   /* fase 4 */
void fase_07_3a7d(void);   /* fase 7 */

#endif
