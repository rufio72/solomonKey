/* Prova di andata e ritorno della grafica estratta: video_immagine <cartella>
 * ricompone dai dodici array gli otto file ROM grafici (12.3t 11.3r 10.3p
 * 9.3m 2.5lm 3.6lm 4.7lm 5.8lm), da confrontare con roms/ext/solomon/.
 * Sono dump: vanno scritti fuori dal repository. */
#include <stdio.h>
#include "video.h"

static int scrivi(const char *dir, const char *nome, const uint8_t *a, const uint8_t *b)
{
    char p[1024];
    snprintf(p, sizeof p, "%s/%s", dir, nome);
    FILE *f = fopen(p, "wb");
    if (!f) {
        fprintf(stderr, "ERRORE: non riesco a scrivere %s\n", p);
        return 2;
    }
    int ok = fwrite(a, 1, VID_PEZZO, f) == VID_PEZZO &&
             (!b || fwrite(b, 1, VID_PEZZO, f) == VID_PEZZO);
    if (fclose(f) != 0 || !ok) {
        fprintf(stderr, "ERRORE: scrittura di %s non riuscita\n", p);
        return 2;
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "uso: video_immagine <cartella>\n");
        return 2;
    }
    const char *d = argv[1];
    if (scrivi(d, "12.3t", vid_car_a, vid_car_b) || scrivi(d, "11.3r", vid_car_c, vid_car_d) ||
        scrivi(d, "10.3p", vid_fon_a, vid_fon_b) || scrivi(d, "9.3m", vid_fon_c, vid_fon_d) ||
        scrivi(d, "2.5lm", vid_spr_a, 0) || scrivi(d, "3.6lm", vid_spr_b, 0) ||
        scrivi(d, "4.7lm", vid_spr_c, 0) || scrivi(d, "5.8lm", vid_spr_d, 0))
        return 2;
    return 0;
}
