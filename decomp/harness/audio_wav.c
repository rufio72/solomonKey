/* Renderizzatore dei flussi dei registri AY in wav (brief 22):
 * ./harness/audio_wav <flusso_ay.txt> <uscita.wav> [da [a]]  (da decomp/)
 *
 * Il flusso è quello di audio.lua (oracolo MAME) o di audio_banco /
 * audio_suona (C del driver): righe «frame chip reg val» (decimale, esa,
 * esa, esa) solo al cambio, commenti con «#», «# codice F CC» prima di un
 * codice forzato. da predefinito = frame del primo «# codice», se non c'è
 * 14; a predefinito = frame dell'ultima riga di dati.
 *
 * Tre YM2149 emulati con emu2149 (harness/emu2149, non modificato) a
 * 1 500 000 Hz, tabella dei volumi YM2149, uscita 44100 Hz, 16 bit, mono:
 * 735 campioni per frame a 60 Hz, somma dei tre chip × 7/8 (sulla scheda
 * i 9 canali vanno sull'altoparlante mono con lo stesso peso).
 *
 * A ogni frame lo stato del flusso a fine frame si scrive nel chip,
 * registro per registro, solo dove cambia. Registro 6 (periodo del
 * rumore): si scrive solo quando il rumore è acceso su almeno un canale
 * del chip ((r7 & 0x38) != 0x38): rimandarlo rende uguali C e MAME quando
 * il periodo cambia a rumore spento, e cambia solo la fase del generatore
 * di rumore, inudibile.
 *
 * Limiti noti: i cambi si applicano a confine di frame (il tempo dentro il
 * frame si perde, uguale per C e MAME); una riscrittura del registro 0D
 * con lo stesso valore, che sul chip riavvia l'inviluppo, non compare nei
 * flussi e quindi non si sente.
 *
 * Uscita 0 se tutto bene, 2 per errori di file o di argomenti. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "emu2149/emu2149.h"

#define CLOCK_AY 1500000
#define FREQ 44100
#define CAMPIONI_FRAME 735      /* 44100 / 60 */
#define DA_PREDEFINITO 14       /* primo frame dopo l'avvio */

struct cambio { int frame, chip, reg, val; };

static struct cambio *cambi;
static size_t n_cambi, cap_cambi;
static int primo_codice = -1;

static int leggi_flusso(const char *percorso)
{
    FILE *f = fopen(percorso, "r");
    if (!f) return 0;
    char riga[256];
    while (fgets(riga, sizeof riga, f)) {
        if (riga[0] == '#') {
            int fr; unsigned cc;
            if (sscanf(riga, "# codice %d %x", &fr, &cc) == 2 && primo_codice < 0)
                primo_codice = fr;
            continue;
        }
        int fr; unsigned chip, reg, val;
        if (sscanf(riga, "%d %x %x %x", &fr, &chip, &reg, &val) != 4) continue;
        if (chip < 1 || chip > 3 || reg > 13) {
            fprintf(stderr, "avviso: riga ignorata: %s", riga);
            continue;
        }
        if (n_cambi == cap_cambi) {
            cap_cambi = cap_cambi ? cap_cambi * 2 : 1024;
            cambi = realloc(cambi, cap_cambi * sizeof *cambi);
            if (!cambi) { fclose(f); return 0; }
        }
        cambi[n_cambi++] = (struct cambio){ fr, (int)chip, (int)reg, (int)(val & 0xFF) };
    }
    fclose(f);
    return 1;
}

static void scrivi16(FILE *f, uint16_t v)
{
    fputc(v & 0xFF, f);
    fputc(v >> 8, f);
}

static void scrivi32(FILE *f, uint32_t v)
{
    scrivi16(f, (uint16_t)(v & 0xFFFF));
    scrivi16(f, (uint16_t)(v >> 16));
}

/* intestazione RIFF/WAVE PCM canonica di 44 byte */
static void intestazione(FILE *f, uint32_t campioni)
{
    uint32_t dati = campioni * 2;
    fwrite("RIFF", 1, 4, f);
    scrivi32(f, 36 + dati);
    fwrite("WAVEfmt ", 1, 8, f);
    scrivi32(f, 16);            /* lunghezza del blocco fmt */
    scrivi16(f, 1);             /* PCM */
    scrivi16(f, 1);             /* canali */
    scrivi32(f, FREQ);
    scrivi32(f, FREQ * 2);      /* byte al secondo */
    scrivi16(f, 2);             /* byte per campione */
    scrivi16(f, 16);            /* bit per campione */
    fwrite("data", 1, 4, f);
    scrivi32(f, dati);
}

int main(int argc, char **argv)
{
    if (argc < 3 || argc > 5) {
        fprintf(stderr, "uso (da decomp/): audio_wav <flusso_ay.txt> <uscita.wav> [da [a]]\n");
        return 2;
    }
    if (!leggi_flusso(argv[1])) {
        fprintf(stderr, "ERRORE: non riesco a leggere %s\n", argv[1]);
        return 2;
    }
    int da = primo_codice >= 0 ? primo_codice : DA_PREDEFINITO;
    int a = n_cambi ? cambi[n_cambi - 1].frame : 0;
    if (argc >= 4) da = atoi(argv[3]);
    if (argc == 5) a = atoi(argv[4]);
    if (a < da) {
        fprintf(stderr, "ERRORE: frame finale %d prima dell'iniziale %d\n", a, da);
        return 2;
    }
    FILE *out = fopen(argv[2], "wb");
    if (!out) {
        fprintf(stderr, "ERRORE: non riesco a scrivere %s\n", argv[2]);
        return 2;
    }

    PSG *psg[3];
    for (int c = 0; c < 3; c++) {
        psg[c] = PSG_new(CLOCK_AY, FREQ);
        PSG_setVolumeMode(psg[c], 1);   /* tabella YM2149 */
        PSG_setQuality(psg[c], 1);      /* convertitore di frequenza */
        PSG_reset(psg[c]);
    }

    /* st: stato del flusso; sc: stato scritto nel chip; -1 = mai scritto */
    int st[3][14], sc[3][14];
    memset(st, 0xFF, sizeof st);
    memset(sc, 0xFF, sizeof sc);
    uint32_t campioni = (uint32_t)(a - da + 1) * CAMPIONI_FRAME;
    intestazione(out, campioni);

    size_t i = 0;
    for (int f = 0; f <= a; f++) {
        for (; i < n_cambi && cambi[i].frame <= f; i++)
            st[cambi[i].chip - 1][cambi[i].reg] = cambi[i].val;
        if (f < da) continue;
        for (int c = 0; c < 3; c++) {
            for (int r = 0; r < 14; r++) {
                if (r == 6 || st[c][r] < 0 || st[c][r] == sc[c][r]) continue;
                PSG_writeReg(psg[c], (uint32_t)r, (uint32_t)st[c][r]);
                sc[c][r] = st[c][r];
            }
            int r7 = sc[c][7] < 0 ? 0xFF : sc[c][7];
            if ((r7 & 0x38) != 0x38 && st[c][6] >= 0 && st[c][6] != sc[c][6]) {
                PSG_writeReg(psg[c], 6, (uint32_t)st[c][6]);
                sc[c][6] = st[c][6];
            }
        }
        for (int k = 0; k < CAMPIONI_FRAME; k++) {
            int32_t s = ((int32_t)PSG_calc(psg[0]) + PSG_calc(psg[1])
                         + PSG_calc(psg[2])) * 7 / 8;
            if (s > 32767) s = 32767;
            if (s < -32768) s = -32768;
            scrivi16(out, (uint16_t)(int16_t)s);
        }
    }

    for (int c = 0; c < 3; c++) PSG_delete(psg[c]);
    free(cambi);
    if (fclose(out) != 0) {
        fprintf(stderr, "ERRORE: scrittura di %s non riuscita\n", argv[2]);
        return 2;
    }
    return 0;
}
