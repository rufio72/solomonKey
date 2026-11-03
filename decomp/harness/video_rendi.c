/* Renderizzatore software del video (lotto H2). Formato provato contro
 * l'oracolo MAME (decomp/VIDEO.md): schermo 32x32 celle 8x8, righe di
 * schermo 16-239 visibili, nessuno scorrimento. */
#include <string.h>
#include "video.h"

#define RIGA_PRIMA 16                 /* prima riga di schermo visibile */

/* Pen di un tile 8x8: 4 byte per riga, nibble alto se x pari. */
static int pen_tile(const uint8_t *const g[4], int codice, int x, int y)
{
    int o = codice * 32 + y * 4 + x / 2;
    uint8_t b = g[o >> 14][o & 0x3FFF];
    return (x & 1) ? (b & 15) : (b >> 4);
}

/* Pen di uno sprite 16x16: quattro piani, un file per piano. */
static int pen_sprite(const uint8_t *const g[4], int codice, int x, int y)
{
    int o = codice * 32 + ((y >= 8) ? 16 : 0) + ((x >= 8) ? 8 : 0) + (y & 7);
    int bit = 7 - (x & 7);
    return ((g[0][o] >> bit) & 1) << 3 | ((g[1][o] >> bit) & 1) << 2 |
           ((g[2][o] >> bit) & 1) << 1 | ((g[3][o] >> bit) & 1);
}

static void leggi_palette(const uint8_t *pal, uint8_t col[256][3])
{
    for (int i = 0; i < 256; i++) {
        int w = pal[2 * i] | pal[2 * i + 1] << 8;
        col[i][0] = (w & 15) * 17;
        col[i][1] = ((w >> 4) & 15) * 17;
        col[i][2] = ((w >> 8) & 15) * 17;
    }
}

/* Scrive un pixel se cade nell'area visibile (riga di schermo ys). */
static void punto(uint8_t rgb[VID_ALTO][VID_LARGO][3], const uint8_t col[256][3],
                  int x, int ys, int colore)
{
    int y = ys - RIGA_PRIMA;
    if (x < 0 || x >= VID_LARGO || y < 0 || y >= VID_ALTO) return;
    memcpy(rgb[y][x], col[colore], 3);
}

static void disegna_bg(const vid_ram *ram, const uint8_t *const g[4],
                       const uint8_t col[256][3], uint8_t rgb[VID_ALTO][VID_LARGO][3])
{
    for (int i = 0; i < 1024; i++) {
        int a = ram->bg_colorram[i];
        int codice = ram->bg_videoram[i] + 256 * (a & 7);
        int base = 128 + 16 * ((a >> 4) & 7);
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++) {
                int sx = (a & 0x80) ? 7 - x : x;
                int sy = (a & 0x08) ? 7 - y : y;
                punto(rgb, col, (i % 32) * 8 + x, (i / 32) * 8 + y,
                      base + pen_tile(g, codice, sx, sy));
            }
    }
}

/* FG: i bit 0x80 e 0x08 dell'attributo sono ignorati; pen 0 trasparente. */
static void disegna_fg(const vid_ram *ram, const uint8_t *const g[4],
                       const uint8_t col[256][3], uint8_t rgb[VID_ALTO][VID_LARGO][3])
{
    for (int i = 0; i < 1024; i++) {
        int a = ram->fg_colorram[i];
        int codice = ram->fg_videoram[i] + 256 * (a & 7);
        int base = 16 * ((a >> 4) & 7);
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++) {
                int pen = pen_tile(g, codice, x, y);
                if (pen) punto(rgb, col, (i % 32) * 8 + x, (i / 32) * 8 + y, base + pen);
            }
    }
}

/* Sprite dalla voce 31 alla 0: la voce 0 resta sopra. */
static void disegna_sprite(const vid_ram *ram, const uint8_t *const g[4],
                           const uint8_t col[256][3], uint8_t rgb[VID_ALTO][VID_LARGO][3])
{
    for (int v = 31; v >= 0; v--) {
        const uint8_t *e = ram->spriteram + 4 * v;
        int codice = e[0] + ((e[1] & 0x10) << 4);
        int base = 16 * ((e[1] >> 1) & 7);
        int sx = e[3], sy = 241 - e[2];
        for (int y = 0; y < 16; y++)
            for (int x = 0; x < 16; x++) {
                int px = (e[1] & 0x40) ? 15 - x : x;
                int py = (e[1] & 0x80) ? 15 - y : y;
                int pen = pen_sprite(g, codice, px, py);
                if (pen) punto(rgb, col, sx + x, sy + y, base + pen);
            }
    }
}

void rendi_frame(const vid_ram *ram, uint8_t rgb[VID_ALTO][VID_LARGO][3])
{
    const uint8_t *car[4] = { vid_car_a, vid_car_b, vid_car_c, vid_car_d };
    const uint8_t *fon[4] = { vid_fon_a, vid_fon_b, vid_fon_c, vid_fon_d };
    const uint8_t *spr[4] = { vid_spr_a, vid_spr_b, vid_spr_c, vid_spr_d };
    uint8_t col[256][3];

    leggi_palette(ram->palette, col);
    disegna_bg(ram, fon, col, rgb);
    disegna_fg(ram, car, col, rgb);
    disegna_sprite(ram, spr, col, rgb);
}
