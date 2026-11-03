/* Hash dei framebuffer del giro dal boot (lotto H4c, brief 48). */
#ifndef GIRO_VIDEO_H
#define GIRO_VIDEO_H

#include <stdint.h>

/* Se `frame` è fra i campioni rende la RAM c000-ffff del C (`ram_c000`) e il
 * dump di MAME `<prefisso>_<frame>.bin`, ne confronta gli hash e con la
 * tabella, e stampa una riga FRAME_VIDEO. */
void giro_video_campione(long frame, const uint8_t *ram_c000, const char *prefisso);

/* Ritorna 0 se tutti i campioni della tabella sono stati visti e coincidono;
 * `campioni` = quanti visti, `coincidenti` = quanti coincidenti. */
int giro_video_riepilogo(long *campioni, long *coincidenti);

#endif
