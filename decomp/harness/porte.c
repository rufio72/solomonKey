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

/* Porte mappate in memoria (proposta A del brief 12). I dump di snap.lua
 * passano dagli handler MAME, quindi ram[a - 0xc000] contiene il valore
 * vivo della porta all'istante della cattura: la lettura dal dump è
 * l'oracolo esatto delle letture. */
uint8_t leggi_porta(uint16_t a)
{
    if (a == 0xe603)
        fprintf(stderr, "porte: lettura di e603 da leggi_porta, usare leggi_e603\n");
    else if (a < 0xe600 || a > 0xe606)
        fprintf(stderr, "porte: lettura dall'indirizzo %04X non censito\n", a);
    return ram[a - 0xc000];
}

/* Scrittura su porta o indirizzo non mappato (e600, e604, e800, e802,
 * e803): si deposita in ram[] come già fa sub_33ba con e802. La zona
 * e600-ffff è fuori confronto (il banco confronta c000-e5ff), quindi la
 * scrittura è visibile al debug e innocua per l'oracolo. */
void scrivi_porta(uint16_t a, uint8_t v)
{
    ram[a - 0xc000] = v;
}
