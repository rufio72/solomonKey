/* Porte di ingresso del Z80 principale che dipendono dal PC. */
#include <stdio.h>
#include "ram.h"
#include "e603.h"

/* Protezione e603: MAME restituisce un valore secondo il PC dell'istruzione
 * di lettura (tabella generata da e603.lua). Per PC non misurati si
 * restituisce 0, come MAME fuori dai PC noti, con un avviso. */
uint8_t leggi_e603(uint16_t pc)
{
    for (unsigned i = 0; i < sizeof e603_tabella / sizeof e603_tabella[0]; i++)
        if (e603_tabella[i].pc_lettura == pc)
            return e603_tabella[i].valore;
    fprintf(stderr, "e603: lettura dal PC %04X non misurata, restituisco 0\n", pc);
    return 0;
}
