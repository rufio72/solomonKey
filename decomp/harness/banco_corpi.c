/* Banco dei corpi di attività (brief 52): un wrapper per corpo con la firma
 * delle voci di tabella_routine. All'ingresso catturato da snap.lua con
 * SNAP_CORPO, [SP] è il puntatore allo slot (C000 + slot) che il corpo
 * toglie con `pop hl`, e [SP+2] il ritorno a 3AF3: il wrapper ricava lo
 * slot da [SP] e chiama il corpo C, che non tocca z. Le voci hanno
 * solo_ram = 1: il banco confronta la RAM e non i registri di uscita.
 * Lotto 54: 12D6 (22), 2CCD (07, fase 1), 3B63 (26), 3B77 (25), 3F40 (1C).
 * Lotto 55: 2C72 (24), 3B61 (voce vuota di 1E e 21), 6466 (15), 6496 (16). */
#include "routine.h"
#include "sched.h"
#include "attivita_libera.h"
#include "nemici.h"

#define BANCO_CORPO(sfx, fn) void banco_corpo_##sfx(struct z80 *z) \
    { fn((uint8_t)((M(z->sp) | (M(z->sp + 1) << 8)) - 0xc000)); }

BANCO_CORPO(12d6, attivita_22_12d6)
BANCO_CORPO(2c72, attivita_24_2c72)
BANCO_CORPO(2ccd, attivita_07_2ccd)
BANCO_CORPO(3792, attivita_19_3792)
BANCO_CORPO(3b61, attivita_vuota_3b61)
BANCO_CORPO(3b63, attivita_26_3b63)
BANCO_CORPO(3b77, attivita_25_3b77)
BANCO_CORPO(3e4e, attivita_23_3e4e)
BANCO_CORPO(3f39, attivita_1d_3f39)
BANCO_CORPO(3f40, attivita_1c_3f40)
BANCO_CORPO(4111, attivita_1b_4111)
BANCO_CORPO(411d, attivita_1a_411d)
BANCO_CORPO(4124, attivita_18_4124)
BANCO_CORPO(426a, attivita_12_426a)
BANCO_CORPO(4c30, attivita_04_4c30)
BANCO_CORPO(4c42, attivita_05_4c42)
BANCO_CORPO(4d29, attivita_09_4d29)
BANCO_CORPO(640e, attivita_0f_640e)
BANCO_CORPO(6429, attivita_17_6429)
BANCO_CORPO(6466, attivita_15_6466)
BANCO_CORPO(6480, attivita_13_6480)
BANCO_CORPO(6496, attivita_16_6496)
