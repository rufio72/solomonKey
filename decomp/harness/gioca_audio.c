/* Dispositivo audio SDL di gioca (lotto H6): anello di 8 frame e callback.
 * Il PCM (44100 Hz mono int16) lo produce scheda_audio; qui si accoda e
 * il callback di SDL lo consuma, riempiendo di zeri se l'anello è vuoto. */
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include "scheda_audio.h"

#define ANELLO (8 * SCHEDA_CAMPIONI_FRAME)

static SDL_AudioDeviceID dispositivo;
static int16_t anello[ANELLO];
static int testa, pieni;               /* indice di lettura e campioni presenti */

static void callback(void *ud, Uint8 *flusso, int byte)
{
    int16_t *out = (int16_t *)flusso;
    int n = byte / 2, i = 0;
    (void)ud;
    for (; i < n && pieni > 0; i++) {
        out[i] = anello[testa];
        testa = (testa + 1) % ANELLO;
        pieni--;
    }
    if (i < n) memset(out + i, 0, (size_t)(n - i) * 2);
}

int gioca_audio_apri(void)
{
    SDL_AudioSpec want, have;
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "avviso: audio SDL non disponibile (%s): si gioca muti\n", SDL_GetError());
        return -1;
    }
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = callback;
    dispositivo = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!dispositivo) {
        fprintf(stderr, "avviso: dispositivo audio non aperto (%s): si gioca muti\n", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return -1;
    }
    printf("AUDIO dispositivo %d Hz mono\n", have.freq);
    SDL_PauseAudioDevice(dispositivo, 0);
    return 0;
}

void gioca_audio_spingi(const int16_t *buf, int n)
{
    if (!dispositivo) return;
    SDL_LockAudioDevice(dispositivo);
    if (pieni + n > ANELLO) {          /* anello pieno: via i campioni più vecchi */
        int troppi = pieni + n - ANELLO;
        testa = (testa + troppi) % ANELLO;
        pieni -= troppi;
    }
    for (int i = 0; i < n; i++)
        anello[(testa + pieni + i) % ANELLO] = buf[i];
    pieni += n;
    SDL_UnlockAudioDevice(dispositivo);
}

void gioca_audio_chiudi(void)
{
    if (!dispositivo) return;
    SDL_CloseAudioDevice(dispositivo);
    dispositivo = 0;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
