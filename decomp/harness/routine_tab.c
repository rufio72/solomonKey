/* Tabella delle routine tradotte, in ordine di indirizzo.
 * blocco_ini/blocco_fine: intervallo scritto da verificare in modo esplicito
 * (0, 0 = nessuno; la RAM c000-e5ff si confronta comunque tutta). */
#include "routine.h"

const struct voce_routine tabella_routine[] = {
    { "0x0008", sub_0008, 0, 0 },
    { "0x0010", sub_0010, 0, 0 },
    { "0x026C", sub_026c, 0xc08f, 0xc091 },
    { "0x05B3", sub_05b3, 0xc8ee, 0xc94d },
    { "0x237E", sub_237e, 0, 0 },
    { "0x23A1", sub_23a1, 0, 0 },
    { "0x2402", sub_2402, 0xe400, 0xe401 },
    { "0x2418", sub_2418, 0, 0 },
    { "0x249E", sub_249e, 0, 0 },
    { "0x24B1", sub_24b1, 0, 0 },
    { "0x30EF", sub_30ef, 0, 0 },
    { "0x326C", sub_326c, 0xd880, 0xdfff },
    { "0x3A1C", sub_3a1c, 0, 0 },
    { "0x3F10", sub_3f10, 0xc061, 0xc067 },
    { "0x4AF6", sub_4af6, 0, 0 },
    { "0x4B0E", sub_4b0e, 0, 0 },
    { "0x5C78", sub_5c78, 0, 0 },
    { "0x673F", sub_673f, 0xc000, 0xc03f },
};

const unsigned num_routine = sizeof tabella_routine / sizeof tabella_routine[0];
