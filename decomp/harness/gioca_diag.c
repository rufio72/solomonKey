/* Diagnostica joystick di gioca (lotto H7, brief 51): gioca --joystick.
 * Elenca i dispositivi e stampa gli eventi in diretta, con l'azione di gioco
 * assegnata dalla mappa corrente. Non apre l'emulazione. */
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include "gioca_joy.h"

static volatile sig_atomic_t interrotto;
static void su_sigint(int s) { (void)s; interrotto = 1; }

static void coda(const char *azione)
{
    if (azione) printf(" (%s)", azione);
    printf("\n");
    fflush(stdout);
}

static void nome_cappello(uint8_t v, char *buf, size_t n)
{
    static const struct { uint8_t bit; const char *nome; } d[] = {
        { SDL_HAT_UP, "su" }, { SDL_HAT_DOWN, "giu" },
        { SDL_HAT_LEFT, "sinistra" }, { SDL_HAT_RIGHT, "destra" } };
    buf[0] = 0;
    for (int i = 0; i < 4; i++)
        if (v & d[i].bit) {
            if (buf[0]) strncat(buf, "+", n - strlen(buf) - 1);
            strncat(buf, d[i].nome, n - strlen(buf) - 1);
        }
    if (!buf[0]) snprintf(buf, n, "centro");
}

static void evento(const SDL_Event *e, int ultimo[8][16])
{
    char h[40];
    switch (e->type) {
    case SDL_JOYAXISMOTION: {
        int id = e->jaxis.which & 7, a = e->jaxis.axis & 15, v = e->jaxis.value;
        if (v - ultimo[id][a] <= 2000 && ultimo[id][a] - v <= 2000) break;
        ultimo[id][a] = v;
        printf("ASSE %d %d %d", (int)e->jaxis.which, (int)e->jaxis.axis, v);
        coda(joy_azione_asse(e->jaxis.axis, v));
        break;
    }
    case SDL_JOYBUTTONDOWN: case SDL_JOYBUTTONUP:
        printf("PULSANTE %d %d %s", (int)e->jbutton.which, (int)e->jbutton.button,
               e->type == SDL_JOYBUTTONDOWN ? "giu" : "su");
        coda(joy_azione_pulsante(e->jbutton.button));
        break;
    case SDL_JOYHATMOTION:
        nome_cappello(e->jhat.value, h, sizeof h);
        printf("CAPPELLO %d %d %s", (int)e->jhat.which, (int)e->jhat.hat, h);
        coda(joy_azione_hat(e->jhat.hat, e->jhat.value));
        break;
    case SDL_JOYDEVICEADDED:
        memset(ultimo[SDL_JoystickGetDeviceInstanceID(e->jdevice.which) & 7], 0, sizeof ultimo[0]);
        printf("COLLEGATO %d \"%s\"\n", (int)SDL_JoystickGetDeviceInstanceID(e->jdevice.which),
               SDL_JoystickNameForIndex(e->jdevice.which));
        fflush(stdout);
        break;
    case SDL_JOYDEVICEREMOVED:
        printf("SCOLLEGATO %d\n", (int)e->jdevice.which);
        fflush(stdout);
        break;
    }
}

/* 0 = uscita normale (ESC, finestra, Ctrl-C o scadenza di secondi_max). */
int joy_diagnostica(long secondi_max)
{
    static int ultimo[8][16];
    int con_video = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == 0;
    SDL_Window *fin = NULL;
    if (!con_video) {
        fprintf(stderr, "avviso: video non disponibile (%s): si esce con Ctrl-C\n", SDL_GetError());
        if (SDL_Init(SDL_INIT_JOYSTICK) != 0) {
            fprintf(stderr, "ERRORE: joystick SDL: %s\n", SDL_GetError());
            return 2;
        }
    } else {
        fin = SDL_CreateWindow("gioca --joystick: ESC per uscire", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, 320, 120, SDL_WINDOW_SHOWN);
        if (!fin) fprintf(stderr, "avviso: finestra non creata (%s): si esce con Ctrl-C\n",
                          SDL_GetError());
    }
    signal(SIGINT, su_sigint);
    joy_apri();
    SDL_PumpEvents();
    SDL_FlushEvent(SDL_JOYDEVICEADDED);      /* i presenti sono già elencati sotto */
    if (SDL_NumJoysticks() == 0) printf("nessun joystick rilevato\n");
    else joy_elenca();
    fflush(stdout);

    Uint32 inizio = SDL_GetTicks();
    int esci = 0;
    while (!esci && !interrotto) {
        SDL_Event e;
        if (secondi_max > 0 && SDL_GetTicks() - inizio >= (Uint32)secondi_max * 1000u) break;
        if (!SDL_WaitEventTimeout(&e, 50)) continue;
        if (e.type == SDL_QUIT) esci = 1;
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) esci = 1;
        else evento(&e, ultimo);
    }
    joy_chiudi();
    if (fin) SDL_DestroyWindow(fin);
    SDL_Quit();
    return 0;
}
