/* Prova dei pixel delle scanlines (addendum 57C del brief 57).
 * Include video_sdl.c, apre il renderer (driver dummy: renderer software),
 * presenta un framebuffer uniforme e rilegge i pixel con SDL_RenderReadPixels.
 * Atteso: a scala N >= 2, in ogni blocco di N righe fisiche le ultime
 * max(1, round(N/4)) scurite a v*(255-64)/255 (±2), le altre intatte;
 * a scala 1 e senza --scanlines tutte intatte.
 * Uso: prova_scanlines [N...]   (da decomp/harness: make prova_scanlines)
 * Uscita 0 se tutte le scale provate rispettano l'atteso, 1 altrimenti. */
#include "video_sdl.c"

static uint8_t fb[VID_ALTO][VID_LARGO][3];

/* Una prova: scala n, scanlines on/off, finestra o --pieno. Restituisce
 * il numero di pixel fuori tolleranza. */
static int prova(int n, int on, int pieno)
{
    vsdl_scanlines(on);
    vsdl_pieno(pieno);
    if (vsdl_apri("prova", n) != 0) return -1;
    vsdl_presenta(fb);
    float sx = 1, sy = 1;
    SDL_RenderGetScale(rend, &sx, &sy);
    int ow, oh;
    SDL_GetRendererOutputSize(rend, &ow, &oh);
    int ne = pieno ? (int)(sy + 0.5f) : n;          /* scala effettiva */
    int w = VID_LARGO * ne, h = VID_ALTO * ne;
    int x0 = (ow - w) / 2, y0 = (oh - h) / 2;      /* area centrata, il resto è nero */
    /* lettura fuori dalla scala logica: coordinate fisiche */
    SDL_RenderSetLogicalSize(rend, 0, 0);
    SDL_RenderSetScale(rend, 1, 1);
    SDL_RenderSetViewport(rend, NULL);
    static uint8_t px[VID_ALTO * 8 * VID_LARGO * 8 * 4];
    SDL_Rect r = { x0, y0, w, h };
    if (SDL_RenderReadPixels(rend, &r, SDL_PIXELFORMAT_RGBA32, px, w * 4) != 0) {
        fprintf(stderr, "ReadPixels: %s\n", SDL_GetError());
        vsdl_chiudi();
        return -1;
    }
    int t = spessore(ne), errori = 0, scure = 0;
    for (int y = 0; y < h; y++) {
        int scura = on && ne >= 2 && (y % ne) >= ne - t;
        scure += scura;
        for (int x = 0; x < w; x++) {
            const uint8_t *p = &px[(y * w + x) * 4];
            for (int c = 0; c < 3; c++) {
                int v = fb[y / ne][x / ne][c];
                int atteso = scura ? v * (255 - SCANLINE_ALPHA) / 255 : v;
                int d = p[c] - atteso;
                if (d < -2 || d > 2) errori++;
            }
        }
    }
    const uint8_t *a = &px[0], *b = &px[(size_t)(ne - 1) * w * 4];
    printf("scala %d%s %s: uscita %dx%d, righe scure %d su %d, "
           "riga 0 = %d,%d,%d  riga %d = %d,%d,%d, fuori tolleranza %d\n",
           ne, pieno ? " (--pieno)" : "", on ? "--scanlines" : "senza opzione",
           ow, oh, scure, h, a[0], a[1], a[2], ne - 1, b[0], b[1], b[2], errori);
    vsdl_chiudi();
    return errori;
}

int main(int argc, char **argv)
{
    for (int y = 0; y < VID_ALTO; y++)
        for (int x = 0; x < VID_LARGO; x++) {
            fb[y][x][0] = 200; fb[y][x][1] = 100; fb[y][x][2] = 50;
        }
    int scale[8] = { 1, 2, 3, 4, 8 }, ns = 5;
    if (argc > 1) {
        ns = 0;
        for (int i = 1; i < argc && ns < 8; i++) scale[ns++] = atoi(argv[i]);
    }
    int male = 0;
    for (int i = 0; i < ns; i++) {
        if (prova(scale[i], 1, 0) != 0) male = 1;
        if (prova(scale[i], 0, 0) != 0) male = 1;
    }
    if (prova(1, 1, 1) != 0) male = 1;   /* --pieno: scala decisa da SDL */
    printf(male ? "PROVA SCANLINES FALLITA\n" : "PROVA SCANLINES PASSATA\n");
    return male;
}
