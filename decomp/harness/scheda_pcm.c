/* Resa PCM della scheda audio (lotto H6): stessa procedura di audio_wav.c.
 * Tre YM2149 (emu2149) a 1,5 MHz, 44100 Hz mono, 735 campioni per frame,
 * somma dei tre chip x 7/8 saturata a int16. Lo stato dei registri a fine
 * frame viene da aud_ombra; il registro 6 (periodo del rumore) si scrive solo
 * se il rumore è acceso su un canale del chip. */
#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "scheda_audio.h"
#include "emu2149/emu2149.h"

#define CLOCK_AY 1500000
#define FREQ 44100

static PSG *psg[3];
static int sc[3][14];                  /* stato scritto nel chip, -1 = mai */
static int16_t buf_frame[SCHEDA_CAMPIONI_FRAME];
static int pronto;                     /* frame vero chiuso e non ancora letto */
static uint64_t hash = 0xcbf29ce484222325ULL;
static long campioni;

static void avvia(void)
{
    for (int c = 0; c < 3; c++) {
        psg[c] = PSG_new(CLOCK_AY, FREQ);
        PSG_setVolumeMode(psg[c], 1);
        PSG_setQuality(psg[c], 1);
        PSG_reset(psg[c]);
    }
    memset(sc, 0xFF, sizeof sc);
}

void scheda_pcm_frame(int recupero)
{
    if (!psg[0]) avvia();
    for (int c = 0; c < 3; c++) {
        for (int r = 0; r < 14; r++) {
            int v = aud_ombra(c + 1, r);
            if (r == 6 || v < 0 || v == sc[c][r]) continue;
            PSG_writeReg(psg[c], (uint32_t)r, (uint32_t)v);
            sc[c][r] = v;
        }
        int r7 = sc[c][7] < 0 ? 0xFF : sc[c][7];
        int v6 = aud_ombra(c + 1, 6);
        if ((r7 & 0x38) != 0x38 && v6 >= 0 && v6 != sc[c][6]) {
            PSG_writeReg(psg[c], 6, (uint32_t)v6);
            sc[c][6] = v6;
        }
    }
    for (int k = 0; k < SCHEDA_CAMPIONI_FRAME; k++) {
        int32_t s = ((int32_t)PSG_calc(psg[0]) + PSG_calc(psg[1]) + PSG_calc(psg[2])) * 7 / 8;
        if (s > 32767) s = 32767;
        if (s < -32768) s = -32768;
        buf_frame[k] = (int16_t)s;
        uint16_t u = (uint16_t)(int16_t)s;
        hash = (hash ^ (u & 0xFF)) * 0x100000001b3ULL;
        hash = (hash ^ (u >> 8)) * 0x100000001b3ULL;
    }
    campioni += SCHEDA_CAMPIONI_FRAME;
    pronto = !recupero;
}

int scheda_audio_pcm(int16_t *buf)
{
    if (!pronto) return 0;
    memcpy(buf, buf_frame, sizeof buf_frame);
    pronto = 0;
    return SCHEDA_CAMPIONI_FRAME;
}

uint64_t scheda_audio_hash(void) { return hash; }
long scheda_audio_campioni(void) { return campioni; }

void scheda_pcm_fine(void)
{
    for (int c = 0; c < 3; c++)
        if (psg[c]) { PSG_delete(psg[c]); psg[c] = NULL; }
}
