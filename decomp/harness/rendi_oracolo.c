/* Confronto del renderizzatore con l'oracolo MAME (lotto H2):
 *   rendi_oracolo <cartella> <stato> <riferimento.ppm> [uscita.ppm]
 * Legge i sei .bin di <cartella>/<stato>_*, rende il frame e lo confronta
 * pixel per pixel con il PPM P6 256x224 di riferimento (il PNG di MAME
 * convertito con png_ppm.py). Exit 0 uguali, 1 diversi, 2 errore. */
#include <stdio.h>
#include <string.h>
#include "video.h"

static uint8_t fg_c[1024], fg_v[1024], bg_c[1024], bg_v[1024], spr[128], pal[512];
static uint8_t reso[VID_ALTO][VID_LARGO][3], atteso[VID_ALTO][VID_LARGO][3];

static int errore(const char *m, const char *a)
{
    fprintf(stderr, "ERRORE: %s%s%s\n", m, a ? " " : "", a ? a : "");
    return 2;
}

static int leggi_bin(const char *dir, const char *stato, const char *nome,
                     uint8_t *buf, size_t n)
{
    char p[1024];
    snprintf(p, sizeof p, "%s/%s_%s.bin", dir, stato, nome);
    FILE *f = fopen(p, "rb");
    if (!f) return errore("non riesco a leggere", p);
    uint8_t extra;
    int ok = fread(buf, 1, n, f) == n && fread(&extra, 1, 1, f) == 0;
    fclose(f);
    return ok ? 0 : errore("dimensione sbagliata in", p);
}

static int leggi_ppm(const char *p, uint8_t dst[VID_ALTO][VID_LARGO][3])
{
    FILE *f = fopen(p, "rb");
    if (!f) return errore("non riesco a leggere", p);
    int l, a, m, n = 0;
    if (fscanf(f, "P6 %d %d %d%n", &l, &a, &m, &n) != 3 || l != VID_LARGO ||
        a != VID_ALTO || m != 255 || fgetc(f) != '\n') {
        fclose(f);
        return errore("intestazione PPM non valida in", p);
    }
    size_t tot = (size_t)VID_ALTO * VID_LARGO * 3;
    int ok = fread(dst, 1, tot, f) == tot;
    fclose(f);
    return ok ? 0 : errore("PPM troncato:", p);
}

static int scrivi_ppm(const char *p, uint8_t src[VID_ALTO][VID_LARGO][3])
{
    FILE *f = fopen(p, "wb");
    if (!f) return errore("non riesco a scrivere", p);
    size_t tot = (size_t)VID_ALTO * VID_LARGO * 3;
    fprintf(f, "P6\n%d %d\n255\n", VID_LARGO, VID_ALTO);
    int ok = fwrite(src, 1, tot, f) == tot;
    if (fclose(f) != 0 || !ok) return errore("scrittura non riuscita di", p);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "uso: rendi_oracolo <cartella> <stato> <riferimento.ppm> [uscita.ppm]\n");
        return 2;
    }
    const char *dir = argv[1], *stato = argv[2];
    if (leggi_bin(dir, stato, "fg_colorram", fg_c, 1024) ||
        leggi_bin(dir, stato, "fg_videoram", fg_v, 1024) ||
        leggi_bin(dir, stato, "bg_colorram", bg_c, 1024) ||
        leggi_bin(dir, stato, "bg_videoram", bg_v, 1024) ||
        leggi_bin(dir, stato, "spriteram", spr, 128) ||
        leggi_bin(dir, stato, "palette", pal, 512) ||
        leggi_ppm(argv[3], atteso))
        return 2;

    vid_ram ram = { fg_c, fg_v, bg_c, bg_v, spr, pal };
    rendi_frame(&ram, reso);
    if (argc == 5 && scrivi_ppm(argv[4], reso)) return 2;

    long diversi = 0;
    int px = -1, py = -1;
    for (int y = 0; y < VID_ALTO; y++)
        for (int x = 0; x < VID_LARGO; x++)
            if (memcmp(reso[y][x], atteso[y][x], 3) != 0) {
                if (!diversi) { px = x; py = y; }
                diversi++;
            }
    if (!diversi) {
        printf("%s: %d/%d pixel uguali\n", stato, VID_ALTO * VID_LARGO, VID_ALTO * VID_LARGO);
        return 0;
    }
    printf("%s: %ld pixel diversi, primo (%d,%d) atteso %02x%02x%02x reso %02x%02x%02x\n",
           stato, diversi, px, py,
           atteso[py][px][0], atteso[py][px][1], atteso[py][px][2],
           reso[py][px][0], reso[py][px][1], reso[py][px][2]);
    return 1;
}
