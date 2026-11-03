/* Stub del motore dei nemici: routine che il C del lotto N1 chiama o
 * smista ma che non sono ancora tradotte (CENSIMENTO-NEMICI.md, «Piano dei
 * lotti»). Ognuna registra il proprio ingresso con nemici_manca e ritorna;
 * il replay si ferma alla fine della spedizione dicendo quale manca.
 * Ogni lotto N2-N12 toglie da qui le righe delle routine che traduce. */
#include "nemici.h"

#define STUB_NEMICI(x) \
    void sub_##x(struct z80 *z) { (void)z; nemici_manca(0x##x); }

STUB_NEMICI(2474)
STUB_NEMICI(4ada)
STUB_NEMICI(5fc8)
STUB_NEMICI(610c)
STUB_NEMICI(627f)
