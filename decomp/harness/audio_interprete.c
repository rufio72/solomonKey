/* Interprete delle sequenze (0147 e i gestori del jp (hl) a 0216):
 * lotto successivo. Con i flag dei blocchi a 00 o 03 l'originale non fa
 * nulla (rrca; jp nc,01ACh / rrca; jr c,01ACh), quindi lo stub vuoto è
 * esatto nei tratti confrontati dal banco: avvio, attesa, codice 00, codici
 * 41-FF. */
#include "audio.h"

/* 0x0147: interprete delle sequenze, lotto successivo. */
void aud_0147(void)
{
}
