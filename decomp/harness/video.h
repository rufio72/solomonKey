/* Strato video (lotto H2): renderizzatore software puro. Dipende solo da
 * <stdint.h>: riceve i blocchi di RAM video come puntatori e produce il
 * framebuffer RGB del frame. */
#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>

#define VID_LARGO 256
#define VID_ALTO 224          /* righe di schermo 16-239 */
#define VID_PEZZO 0x4000      /* byte di un file video_dati_*.c */

typedef struct {
    const uint8_t *fg_colorram;   /* d000-d3ff, 1024 byte */
    const uint8_t *fg_videoram;   /* d400-d7ff, 1024 byte */
    const uint8_t *bg_colorram;   /* d800-dbff, 1024 byte */
    const uint8_t *bg_videoram;   /* dc00-dfff, 1024 byte */
    const uint8_t *spriteram;     /* e000-e07f, 128 byte */
    const uint8_t *palette;       /* e400-e5ff, 512 byte */
} vid_ram;

/* Fondale BG, piano FG (pen 0 trasparente), sprite dalla voce 31 alla 0. */
void rendi_frame(const vid_ram *ram, uint8_t rgb[VID_ALTO][VID_LARGO][3]);

/* Grafica delle ROM (generata da decomp/estrai_grafica.py). */
extern const uint8_t vid_car_a[VID_PEZZO];   /* 12.3t 0000-3FFF */
extern const uint8_t vid_car_b[VID_PEZZO];   /* 12.3t 4000-7FFF */
extern const uint8_t vid_car_c[VID_PEZZO];   /* 11.3r 0000-3FFF */
extern const uint8_t vid_car_d[VID_PEZZO];   /* 11.3r 4000-7FFF */
extern const uint8_t vid_fon_a[VID_PEZZO];   /* 10.3p 0000-3FFF */
extern const uint8_t vid_fon_b[VID_PEZZO];   /* 10.3p 4000-7FFF */
extern const uint8_t vid_fon_c[VID_PEZZO];   /* 9.3m 0000-3FFF */
extern const uint8_t vid_fon_d[VID_PEZZO];   /* 9.3m 4000-7FFF */
extern const uint8_t vid_spr_a[VID_PEZZO];   /* 2.5lm */
extern const uint8_t vid_spr_b[VID_PEZZO];   /* 3.6lm */
extern const uint8_t vid_spr_c[VID_PEZZO];   /* 4.7lm */
extern const uint8_t vid_spr_d[VID_PEZZO];   /* 5.8lm */

#endif
