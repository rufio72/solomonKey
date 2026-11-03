/* Joystick di gioca (lotto H7, brief 51). Gli ingressi sono attivi alti:
 * premuto = 1. e600: 01 destra, 02 sinistra, 04 su, 08 giu, 10 BUTTON2,
 * 20 BUTTON1; e602: 01 START1, 04 COIN1. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "gioca_joy.h"

#define MAX_JOY 8
#define SOGLIA 8000

joy_mappa joy_mappa_corrente = { 0, 1, 7, 6 };

static SDL_Joystick *aperti[MAX_JOY];
static int attivo;

int joy_analizza_mappa(const char *s, joy_mappa *m)
{
    int v[4];
    for (int i = 0; i < 4; i++) {
        char *fine;
        if (!s || !*s) return -1;
        long x = strtol(s, &fine, 10);
        if (fine == s || x < -1 || x > 31) return -1;
        v[i] = (int)x;
        if (i < 3) { if (*fine != ',') return -1; s = fine + 1; }
        else if (*fine) return -1;
    }
    m->b1 = v[0]; m->b2 = v[1]; m->start = v[2]; m->coin = v[3];
    return 0;
}

static int cerca(SDL_JoystickID id)
{
    for (int i = 0; i < MAX_JOY; i++)
        if (aperti[i] && SDL_JoystickInstanceID(aperti[i]) == id) return i;
    return -1;
}

static void aggiungi(int indice_dispositivo)
{
    SDL_JoystickID id = SDL_JoystickGetDeviceInstanceID(indice_dispositivo);
    if (id < 0 || cerca(id) >= 0) return;          /* già aperto */
    for (int i = 0; i < MAX_JOY; i++)
        if (!aperti[i]) { aperti[i] = SDL_JoystickOpen(indice_dispositivo); return; }
}

static int osservatore(void *ud, SDL_Event *e)
{
    (void)ud;
    if (e->type == SDL_JOYDEVICEADDED) {
        aggiungi(e->jdevice.which);
    } else if (e->type == SDL_JOYDEVICEREMOVED) {
        int i = cerca(e->jdevice.which);
        if (i >= 0) { SDL_JoystickClose(aperti[i]); aperti[i] = NULL; }
    }
    return 0;
}

void joy_apri(void)
{
    if (attivo) return;
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) return;
    attivo = 1;
    SDL_AddEventWatch(osservatore, NULL);
    for (int i = 0; i < SDL_NumJoysticks(); i++)
        aggiungi(i);
}

void joy_chiudi(void)
{
    if (!attivo) return;
    SDL_DelEventWatch(osservatore, NULL);
    for (int i = 0; i < MAX_JOY; i++)
        if (aperti[i]) { SDL_JoystickClose(aperti[i]); aperti[i] = NULL; }
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    attivo = 0;
}

static int premuto(SDL_Joystick *j, int nb, int n)
{
    return n >= 0 && n < nb && SDL_JoystickGetButton(j, n);
}

/* Aggiunge a out[] tutti i joystick: assi 0/1 oltre ±SOGLIA e cappello 0
 * (le diagonali del cappello valgono due direzioni), pulsanti dalla mappa. */
void joy_ingressi(uint8_t out[3])
{
    for (int i = 0; i < MAX_JOY; i++) {
        SDL_Joystick *j = aperti[i];
        if (!j) continue;
        int x = SDL_JoystickGetAxis(j, 0), y = SDL_JoystickGetAxis(j, 1);
        uint8_t h = SDL_JoystickNumHats(j) > 0 ? SDL_JoystickGetHat(j, 0) : 0;
        if (x > SOGLIA || (h & SDL_HAT_RIGHT)) out[0] |= 0x01;
        if (x < -SOGLIA || (h & SDL_HAT_LEFT)) out[0] |= 0x02;
        if (y < -SOGLIA || (h & SDL_HAT_UP)) out[0] |= 0x04;
        if (y > SOGLIA || (h & SDL_HAT_DOWN)) out[0] |= 0x08;
        int nb = SDL_JoystickNumButtons(j);
        if (premuto(j, nb, joy_mappa_corrente.b1)) out[0] |= 0x20;
        if (premuto(j, nb, joy_mappa_corrente.b2)) out[0] |= 0x10;
        if (premuto(j, nb, joy_mappa_corrente.coin)) out[2] |= 0x04;
        if (premuto(j, nb, joy_mappa_corrente.start)) out[2] |= 0x01;
    }
    if ((out[0] & 0x03) == 0x03) out[0] &= (uint8_t)~0x03;
    if ((out[0] & 0x0c) == 0x0c) out[0] &= (uint8_t)~0x0c;
}

void joy_elenca(void)
{
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        SDL_Joystick *j = SDL_JoystickOpen(i);
        char guid[33];
        if (!j) continue;
        SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(j), guid, sizeof guid);
        printf("JOYSTICK %d \"%s\" assi=%d pulsanti=%d cappelli=%d guid=%s\n",
               (int)SDL_JoystickInstanceID(j), SDL_JoystickName(j),
               SDL_JoystickNumAxes(j), SDL_JoystickNumButtons(j),
               SDL_JoystickNumHats(j), guid);
        SDL_JoystickClose(j);
    }
    fflush(stdout);
}

const char *joy_azione_pulsante(int n)
{
    const joy_mappa *m = &joy_mappa_corrente;
    if (n < 0) return NULL;
    if (n == m->b1) return "BUTTON1";
    if (n == m->b2) return "BUTTON2";
    if (n == m->start) return "START1";
    if (n == m->coin) return "COIN1";
    return NULL;
}

const char *joy_azione_asse(int asse, int valore)
{
    if (valore > SOGLIA || valore < -SOGLIA) {
        if (asse == 0) return valore > 0 ? "destra" : "sinistra";
        if (asse == 1) return valore > 0 ? "giu" : "su";
    }
    return NULL;
}

const char *joy_azione_hat(int hat, uint8_t v)
{
    static char buf[40];
    buf[0] = 0;
    if (hat != 0) return NULL;
    if (v & SDL_HAT_UP) strcat(buf, "su");
    if (v & SDL_HAT_DOWN) strcat(buf, buf[0] ? "+giu" : "giu");
    if (v & SDL_HAT_LEFT) strcat(buf, buf[0] ? "+sinistra" : "sinistra");
    if (v & SDL_HAT_RIGHT) strcat(buf, buf[0] ? "+destra" : "destra");
    return buf[0] ? buf : NULL;
}
