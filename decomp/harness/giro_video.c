/* Hash dei framebuffer del giro dal boot (lotto H4c, brief 48).
 * A 13 frame campione la RAM del C, prima della NMI del frame, viene resa con
 * rendi_frame e confrontata con il dump di MAME reso allo stesso modo e con
 * l'hash di riferimento in tabella. Hash: FNV-1a 64 bit sui byte RGB (come
 * vsdl_hash di video_sdl.c, copiato per non portare SDL nel giro). */
#include <stdio.h>
#include <stdlib.h>
#include "video.h"
#include "giro_video.h"

static const struct { long frame; uint64_t hash; } campione[] = {
    { 700, 0xecd5acf694285489ULL }, { 1000, 0xecd5acf694285489ULL }, { 1300, 0x72dcb8156116ea1aULL }, { 1600, 0x47ce347bb79ec1bbULL }, { 1900, 0x960052b7b34b1c54ULL },
    { 2100, 0x5f9c4bdae4b92d31ULL }, { 2400, 0x7a0831f8cdb6805dULL }, { 3000, 0xa095933c7af464d4ULL }, { 3500, 0x151fa1f690ea7454ULL }, { 4000, 0x477dc1ae3714e5efULL },
    { 4500, 0x57d536ce21b8e9cbULL }, { 5000, 0x8b0bb97c5ef23965ULL }, { 5499, 0xa239d0a940bec0b9ULL },
};
#define N_CAMPIONI ((int)(sizeof campione / sizeof campione[0]))

static int visto[N_CAMPIONI];
static int bene[N_CAMPIONI];

static uint64_t hash_fb(const uint8_t fb[VID_ALTO][VID_LARGO][3])
{
    const uint8_t *p = &fb[0][0][0];
    uint64_t h = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < (size_t)VID_ALTO * VID_LARGO * 3; i++)
        h = (h ^ p[i]) * 0x100000001b3ULL;
    return h;
}

static uint64_t rendi_hash(const uint8_t *ram)
{
    static uint8_t fb[VID_ALTO][VID_LARGO][3];
    vid_ram v = {
        ram + 0xd000 - 0xc000, ram + 0xd400 - 0xc000,
        ram + 0xd800 - 0xc000, ram + 0xdc00 - 0xc000,
        ram + 0xe000 - 0xc000, ram + 0xe400 - 0xc000,
    };
    rendi_frame(&v, fb);
    return hash_fb(fb);
}

void giro_video_campione(long frame, const uint8_t *ram_c000, const char *prefisso)
{
    int i;
    for (i = 0; i < N_CAMPIONI; i++)
        if (campione[i].frame == frame)
            break;
    if (i == N_CAMPIONI)
        return;
    visto[i] = 1;
    uint64_t hc = rendi_hash(ram_c000);
    char nome[600];
    snprintf(nome, sizeof nome, "%s_%ld.bin", prefisso, frame);
    static uint8_t mame[0x4000];
    FILE *f = fopen(nome, "rb");
    if (!f || fread(mame, 1, sizeof mame, f) != sizeof mame) {
        printf("FRAME_VIDEO %ld: dump %s assente, DIVERSO\n", frame, nome);
        if (f) fclose(f);
        return;
    }
    fclose(f);
    uint64_t hm = rendi_hash(mame);
    int ok = hc == hm && hc == campione[i].hash;
    bene[i] = ok;
    printf("FRAME_VIDEO %ld: hash C %016llx MAME %016llx tabella %016llx, %s\n", frame,
           (unsigned long long)hc, (unsigned long long)hm,
           (unsigned long long)campione[i].hash, ok ? "coincide" : "DIVERSO");
}

int giro_video_riepilogo(long *campioni, long *coincidenti)
{
    long v = 0, b = 0;
    for (int i = 0; i < N_CAMPIONI; i++) {
        v += visto[i];
        b += bene[i];
    }
    *campioni = v;
    *coincidenti = b;
    return (v == N_CAMPIONI && b == N_CAMPIONI) ? 0 : 1;
}
