/* Mostra uno stato campione dell'oracolo video in una finestra SDL2 (lotto H3):
 *   mostra_oracolo [-s N] [-n N | -m N] [-d cartella] <stato>
 * Rende UNA volta con rendi_frame e ripresenta lo stesso frame a 60 Hz.
 * Con -n N non usa SDL (N "frame" = lo stesso frame contato N volte).
 * All'uscita stampa HASH primo, HASH ultimo, FRAME rese. Exit 0; 2 errore. */
#include <stdio.h>
#include <string.h>
#include "video_sdl.h"

static uint8_t fg_c[1024], fg_v[1024], bg_c[1024], bg_v[1024], spr[128], pal[512];
static uint8_t reso[VID_ALTO][VID_LARGO][3];

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

int main(int argc, char **argv)
{
    /* -d <cartella> è del programma: la toglie da argv prima delle opzioni SDL */
    const char *dir = "out/video";
    char *av[64];
    int ac = 0;
    if (argc > 64) return errore("troppi argomenti", NULL);
    for (int i = 0; i < argc; i++) {
        if (i > 0 && !strcmp(argv[i], "-d")) {
            if (i + 1 >= argc) return errore("-d vuole una cartella", NULL);
            dir = argv[++i];
        } else
            av[ac++] = argv[i];
    }
    vsdl_opz o;
    int k = vsdl_leggi_opzioni(ac, av, &o);
    if (k < 0 || k != ac - 1) {
        if (k >= 0) fprintf(stderr, "uso: mostra_oracolo [-s N] [-n N | -m N] [-d cartella] <stato>\n");
        return 2;
    }
    const char *stato = av[k];
    if (leggi_bin(dir, stato, "fg_colorram", fg_c, 1024) ||
        leggi_bin(dir, stato, "fg_videoram", fg_v, 1024) ||
        leggi_bin(dir, stato, "bg_colorram", bg_c, 1024) ||
        leggi_bin(dir, stato, "bg_videoram", bg_v, 1024) ||
        leggi_bin(dir, stato, "spriteram", spr, 128) ||
        leggi_bin(dir, stato, "palette", pal, 512))
        return 2;

    vid_ram ram = { fg_c, fg_v, bg_c, bg_v, spr, pal };
    rendi_frame(&ram, reso);

    long rese = 0;
    if (o.headless) {
        rese = o.headless;
    } else {
        if (vsdl_apri("Solomon's Key - oracolo", o.scala) != 0) return 2;
        for (;;) {
            int r = vsdl_presenta(reso);
            if (r < 0) { vsdl_chiudi(); return 2; }
            rese++;
            if (r > 0 || (o.massimo && rese >= o.massimo)) break;
        }
        vsdl_chiudi();
    }
    uint64_t h = vsdl_hash(reso);
    printf("HASH primo %016llx\n", (unsigned long long)h);
    printf("HASH ultimo %016llx\n", (unsigned long long)h);
    printf("FRAME rese %ld\n", rese);
    return 0;
}
