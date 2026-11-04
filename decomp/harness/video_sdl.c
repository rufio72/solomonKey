/* Finestra SDL2 per il framebuffer 256x224 RGB di rendi_frame (lotto H3).
 * SDL non cambia i pixel: il framebuffer mostrato è quello reso. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "video_sdl.h"

static SDL_Window *finestra;
static SDL_Renderer *rend;
static SDL_Texture *tex;
static int con_vsync;
static int a_schermo_pieno;   /* vsdl_pieno: finestra a schermo intero «desktop» */
static int con_scanlines;     /* vsdl_scanlines: righe scure alla CRT */
static SDL_Texture *overlay;  /* 1 x 2*VID_ALTO, righe dispari semitrasparenti */
static uint64_t scadenza;   /* contatore SDL del prossimo present, ripiego */
static uint64_t periodo;    /* 1/60 s in tick del contatore */

/* Legge un intero decimale intero (nessun resto); -1 se non valido. */
static int numero(const char *s, long *v)
{
    char *fine;
    if (!s || !*s) return -1;
    *v = strtol(s, &fine, 10);
    return *fine ? -1 : 0;
}

int vsdl_leggi_opzioni(int argc, char **argv, vsdl_opz *o)
{
    o->scala = 3;
    o->headless = 0;
    o->massimo = 0;
    int i = 1;
    for (; i < argc; i++) {
        const char *a = argv[i];
        if (a[0] != '-' || !a[1]) break;
        if (strcmp(a, "--") == 0) { i++; break; }
        int s = !strcmp(a, "-s") || !strcmp(a, "--scala");
        int n = !strcmp(a, "-n") || !strcmp(a, "--headless");
        int m = !strcmp(a, "-m") || !strcmp(a, "--max");
        if (!s && !n && !m) {
            fprintf(stderr, "ERRORE: opzione sconosciuta %s\n", a);
            return -1;
        }
        long v;
        if (i + 1 >= argc || numero(argv[i + 1], &v) != 0) {
            fprintf(stderr, "ERRORE: l'opzione %s vuole un numero intero\n", a);
            return -1;
        }
        i++;
        if (s) {
            if (v < 1 || v > 8) {
                fprintf(stderr, "ERRORE: la scala va da 1 a 8\n");
                return -1;
            }
            o->scala = (int)v;
        } else {
            if (v < 1) {
                fprintf(stderr, "ERRORE: %s vuole un numero di frame maggiore di zero\n", a);
                return -1;
            }
            if (n) o->headless = v; else o->massimo = v;
        }
    }
    if (o->headless && o->massimo) {
        fprintf(stderr, "ERRORE: -n e -m si escludono\n");
        return -1;
    }
    return i;
}

void vsdl_pieno(int on)
{
    a_schermo_pieno = on;
}

void vsdl_scanlines(int on)
{
    con_scanlines = on;
}

/* Overlay delle scanlines: riga pari trasparente, riga dispari nera con
 * alpha 80; con SDL_BLENDMODE_BLEND il pixel sotto diventa v*(1-80/255).
 * Se non si crea: avviso e si continua senza effetto. */
static void crea_overlay(void)
{
    static uint8_t px[VID_ALTO * 2][4];
    for (int y = 0; y < VID_ALTO * 2; y++) {
        px[y][0] = px[y][1] = px[y][2] = 0;
        px[y][3] = (y & 1) ? 80 : 0;
    }
    overlay = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC,
                                1, VID_ALTO * 2);
    if (!overlay || SDL_UpdateTexture(overlay, NULL, px, 4) != 0 ||
        SDL_SetTextureBlendMode(overlay, SDL_BLENDMODE_BLEND) != 0) {
        fprintf(stderr, "AVVISO: scanlines non disponibili: %s\n", SDL_GetError());
        if (overlay) SDL_DestroyTexture(overlay);
        overlay = NULL;
    }
}

int vsdl_apri(const char *titolo, int scala)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "ERRORE: SDL_Init: %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    finestra = SDL_CreateWindow(titolo, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                VID_LARGO * scala, VID_ALTO * scala,
                                SDL_WINDOW_SHOWN | (a_schermo_pieno ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    if (!finestra) {
        fprintf(stderr, "ERRORE: finestra: %s\n", SDL_GetError());
        vsdl_chiudi();
        return -1;
    }
    rend = SDL_CreateRenderer(finestra, -1,
                              SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!rend)
        rend = SDL_CreateRenderer(finestra, -1, SDL_RENDERER_SOFTWARE);
    if (!rend) {
        fprintf(stderr, "ERRORE: renderer: %s\n", SDL_GetError());
        vsdl_chiudi();
        return -1;
    }
    if (a_schermo_pieno) {
        /* proporzioni mantenute: scala intera, il resto è nero */
        SDL_RenderSetLogicalSize(rend, VID_LARGO, VID_ALTO);
        SDL_RenderSetIntegerScale(rend, SDL_TRUE);
        SDL_ShowCursor(SDL_DISABLE);
    }
    SDL_RendererInfo info;
    /* il renderer software (driver dummy compreso) dichiara il vsync ma non
     * attende davvero: lì vale il ripiego a temporizzazione software */
    con_vsync = SDL_GetRendererInfo(rend, &info) == 0 &&
                (info.flags & SDL_RENDERER_PRESENTVSYNC) &&
                !(info.flags & SDL_RENDERER_SOFTWARE);
    tex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
                            VID_LARGO, VID_ALTO);
    if (!tex) {
        fprintf(stderr, "ERRORE: texture: %s\n", SDL_GetError());
        vsdl_chiudi();
        return -1;
    }
    if (con_scanlines) crea_overlay();
    periodo = SDL_GetPerformanceFrequency() / 60;
    scadenza = 0;
    return 0;
}

int vsdl_vsync(void)
{
    return con_vsync;
}

int vsdl_presenta(const uint8_t fb[VID_ALTO][VID_LARGO][3])
{
    if (!tex) return -1;
    if (SDL_UpdateTexture(tex, NULL, fb, VID_LARGO * 3) != 0 ||
        SDL_RenderClear(rend) != 0 ||
        SDL_RenderCopy(rend, tex, NULL, NULL) != 0) {
        fprintf(stderr, "ERRORE: SDL: %s\n", SDL_GetError());
        return -1;
    }
    if (overlay && SDL_RenderCopy(rend, overlay, NULL, NULL) != 0) {
        fprintf(stderr, "ERRORE: SDL: %s\n", SDL_GetError());
        return -1;
    }
    if (!con_vsync) {
        /* ripiego: la scadenza avanza di 1/60 s, non di «ora + 1/60» */
        uint64_t ora = SDL_GetPerformanceCounter();
        if (scadenza == 0) scadenza = ora;
        if (ora < scadenza) {
            uint64_t ms = (scadenza - ora) * 1000 / SDL_GetPerformanceFrequency();
            if (ms > 0) SDL_Delay((Uint32)ms);
            while (SDL_GetPerformanceCounter() < scadenza) { }
        } else if (ora - scadenza > periodo * 5) {
            scadenza = ora;   /* in ritardo di molto: niente raffica di recupero */
        }
        scadenza += periodo;
    }
    SDL_RenderPresent(rend);
    int esci = 0;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) esci = 1;
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) esci = 1;
    }
    return esci;
}

void vsdl_chiudi(void)
{
    if (overlay) SDL_DestroyTexture(overlay);
    if (tex) SDL_DestroyTexture(tex);
    if (rend) SDL_DestroyRenderer(rend);
    if (finestra) SDL_DestroyWindow(finestra);
    overlay = NULL; tex = NULL; rend = NULL; finestra = NULL;
    SDL_Quit();
}

uint64_t vsdl_hash(const uint8_t fb[VID_ALTO][VID_LARGO][3])
{
    const uint8_t *p = &fb[0][0][0];
    uint64_t h = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < (size_t)VID_ALTO * VID_LARGO * 3; i++)
        h = (h ^ p[i]) * 0x100000001b3ULL;
    return h;
}
