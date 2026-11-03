/* Tastiera di gioca (lotto H5, brief 49; il joystick sta in gioca_joy.c, H7). Gli ingressi del gioco
 * sono attivi alti (IP_ACTIVE_HIGH in solomon.cpp): premuto = 1. */
#include <stdio.h>
#include <SDL.h>

struct tasto { SDL_Scancode sc; const char *nome; int porta; uint8_t maschera; const char *azione; };

/* Ordine = ordine di stampa di --mappa (25 righe). porta: 0 e600, 1 e601, 2 e602. */
static const struct tasto tasti[] = {
    {SDL_SCANCODE_RIGHT, "Right", 0, 0x01, "destra"}, {SDL_SCANCODE_D, "D", 0, 0x01, "destra"},
    {SDL_SCANCODE_LEFT, "Left", 0, 0x02, "sinistra"}, {SDL_SCANCODE_A, "A", 0, 0x02, "sinistra"},
    {SDL_SCANCODE_UP, "Up", 0, 0x04, "su"}, {SDL_SCANCODE_W, "W", 0, 0x04, "su"},
    {SDL_SCANCODE_DOWN, "Down", 0, 0x08, "giu"}, {SDL_SCANCODE_S, "S", 0, 0x08, "giu"},
    {SDL_SCANCODE_X, "X", 0, 0x10, "BUTTON2"}, {SDL_SCANCODE_LALT, "LeftAlt", 0, 0x10, "BUTTON2"},
    {SDL_SCANCODE_Z, "Z", 0, 0x20, "BUTTON1"}, {SDL_SCANCODE_LCTRL, "LeftCtrl", 0, 0x20, "BUTTON1"},
    {SDL_SCANCODE_SPACE, "Space", 0, 0x20, "BUTTON1"},
    {SDL_SCANCODE_1, "1", 2, 0x01, "START1"}, {SDL_SCANCODE_RETURN, "Return", 2, 0x01, "START1"},
    {SDL_SCANCODE_2, "2", 2, 0x02, "START2"},
    {SDL_SCANCODE_5, "5", 2, 0x04, "COIN1"}, {SDL_SCANCODE_C, "C", 2, 0x04, "COIN1"},
    {SDL_SCANCODE_6, "6", 2, 0x08, "COIN2"},
    {SDL_SCANCODE_L, "L", 1, 0x01, "destra"}, {SDL_SCANCODE_J, "J", 1, 0x02, "sinistra"},
    {SDL_SCANCODE_I, "I", 1, 0x04, "su"}, {SDL_SCANCODE_K, "K", 1, 0x08, "giu"},
    {SDL_SCANCODE_M, "M", 1, 0x10, "BUTTON2"}, {SDL_SCANCODE_N, "N", 1, 0x20, "BUTTON1"},
};
#define N_TASTI ((int)(sizeof tasti / sizeof tasti[0]))
static const char *nome_porta[] = { "e600", "e601", "e602" };

void tasti_stampa_mappa(void)
{
    for (int i = 0; i < N_TASTI; i++)
        printf("%s %s %02X %s\n", tasti[i].nome, nome_porta[tasti[i].porta],
               tasti[i].maschera, tasti[i].azione);
}

/* Dallo stato della tastiera SDL (indicizzato per scancode) ai tre byte
 * e600, e601, e602. Direzioni opposte insieme si annullano. */
void tasti_ingressi(const uint8_t *stato, uint8_t out[3])
{
    out[0] = out[1] = out[2] = 0;
    for (int i = 0; i < N_TASTI; i++)
        if (stato[tasti[i].sc])
            out[tasti[i].porta] |= tasti[i].maschera;
    for (int p = 0; p < 2; p++) {
        if ((out[p] & 0x03) == 0x03) out[p] &= (uint8_t)~0x03;
        if ((out[p] & 0x0c) == 0x0c) out[p] &= (uint8_t)~0x0c;
    }
}
