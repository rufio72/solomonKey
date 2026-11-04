/* Corpi di attività del gioco libero, lotto 52 (brief 52): prototipi dei
 * corpi 04 05 09 0F 12 13 17 18 19 1A 1B 1D 23, delle routine nuove che
 * usano (4C89, 6735, la voce 2E79 condivisa) e dei wrapper del banco dei
 * corpi (banco_corpi.c). Convenzioni dei corpi come attivita_01_10.c. */
#ifndef ATTIVITA_LIBERA_H
#define ATTIVITA_LIBERA_H

#include <stdint.h>
#include "ram.h"
#include "sched.h"

/* Bandiera Z del Z80 in struct z80.f (6735 la lascia per i chiamanti). */
#define LIBERA_ZERO 0x40

/* attivita_piccoli.c */
void attivita_04_4c30(uint8_t slot);
void attivita_05_4c42(uint8_t slot);
void attivita_09_4d29(uint8_t slot);
void attivita_0f_640e(uint8_t slot);
void attivita_17_6429(uint8_t slot);
void attivita_19_3792(uint8_t slot);
void attivita_1a_411d(uint8_t slot);
void attivita_1b_4111(uint8_t slot);
void attivita_1d_3f39(uint8_t slot);
void attivita_23_3e4e(uint8_t slot);
void sub_4c89(struct z80 *z);

/* attivita_12_13.c */
void attivita_12_426a(uint8_t slot);
void attivita_13_6480(uint8_t slot);

/* attivita_18.c */
void attivita_18_4124(uint8_t slot);
void sub_6735(struct z80 *z);
/* 2E79: voce di attesa `ld c,42h; jr 2e8b` (sub_2e90 su HL = contatore,
 * C = 42), presente anche nelle tabelle dei corpi 25 e 26 (lotto 53). */
void passo_2e79(uint16_t contatore);

/* banco_corpi.c: un wrapper per corpo, slot = [SP] - C000. */
void banco_corpo_3792(struct z80 *z);
void banco_corpo_3e4e(struct z80 *z);
void banco_corpo_3f39(struct z80 *z);
void banco_corpo_4111(struct z80 *z);
void banco_corpo_411d(struct z80 *z);
void banco_corpo_4124(struct z80 *z);
void banco_corpo_426a(struct z80 *z);
void banco_corpo_4c30(struct z80 *z);
void banco_corpo_4c42(struct z80 *z);
void banco_corpo_4d29(struct z80 *z);
void banco_corpo_640e(struct z80 *z);
void banco_corpo_6429(struct z80 *z);
void banco_corpo_6480(struct z80 *z);

#endif
