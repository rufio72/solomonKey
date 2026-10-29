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
STUB_NEMICI(4fd4)
STUB_NEMICI(5cfb)
STUB_NEMICI(6b43)
STUB_NEMICI(6c0b)
STUB_NEMICI(7f48)
STUB_NEMICI(7f81)
STUB_NEMICI(7f8a)
STUB_NEMICI(7fa6)
STUB_NEMICI(7fcf)
STUB_NEMICI(801d)
STUB_NEMICI(8042)
STUB_NEMICI(809c)
