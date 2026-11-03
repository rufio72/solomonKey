/* Scheda audio di gioca (lotto H6): sequenza IRQ A / latch / IRQ B del driver. */
#include <stdio.h>
#include <stdlib.h>
#include "audio.h"
#include "scheda_audio.h"

#define MAX_LATCH 64

int scheda_audio_fase = 1;

static long corrente = -1;             /* frame aperto, -1 = nessuno */
static uint8_t coda[MAX_LATCH];        /* latch in attesa (solo fase 0) */
static int n_coda;
static FILE *registro;

static void irq(void)
{
    aud_0038();
    aud_029e();
}

static void esegui_latch(uint8_t v)
{
    aud_latch = v;
    aud_0066(v);
}

void scheda_audio_apri(const char *flusso_ay, const char *registro_latch)
{
    if (flusso_ay && !aud_flusso_apri(flusso_ay)) {
        fprintf(stderr, "ERRORE: non riesco a scrivere %s\n", flusso_ay);
        exit(2);
    }
    if (registro_latch) {
        registro = fopen(registro_latch, "w");
        if (!registro) { perror(registro_latch); exit(2); }
        fputs("# frame\tvalore\torigine\n", registro);
    }
}

/* Chiude il frame corrente: latch in coda (fase 0), A (fase 0), poi B. */
static void chiudi_frame(int recupero)
{
    if (corrente < 0) return;
    aud_frame = (int)corrente;
    if (scheda_audio_fase == 0) {
        for (int i = 0; i < n_coda; i++) esegui_latch(coda[i]);
        n_coda = 0;
        irq();
    }
    irq();
    if (scheda_audio_fase == 2) {      /* A, B, latch */
        for (int i = 0; i < n_coda; i++) esegui_latch(coda[i]);
        n_coda = 0;
    }
    scheda_pcm_frame(recupero);
}

void scheda_audio_latch(uint8_t v)
{
    if (corrente < 0) return;          /* prima dell'avvio: nessun frame */
    if (registro) fprintf(registro, "%ld\t%02X\tgioco\n", corrente, v);
    if (scheda_audio_fase != 1) {
        if (n_coda < MAX_LATCH) coda[n_coda++] = v;
    } else {
        aud_frame = (int)corrente;
        esegui_latch(v);
    }
}

void scheda_audio_frame(long frame)
{
    if (corrente < 0) {
        aud_carica_rom();
        aud_usa_dati_estratti(1);
        aud_azzera();
        aud_frame = 0;
        aud_avvio();
        for (long f = 0; f < frame; f++) {   /* recupero: solo A e B */
            corrente = f;
            aud_frame = (int)f;
            irq();
            irq();
            scheda_pcm_frame(1);
        }
    } else {
        chiudi_frame(0);
    }
    corrente = frame;
    aud_frame = (int)frame;
    if (scheda_audio_fase != 0) irq();
}

void scheda_audio_chiudi(void)
{
    chiudi_frame(0);
    corrente = -1;
    aud_flusso_chiudi();
    if (registro) { fclose(registro); registro = NULL; }
    scheda_pcm_fine();
}
