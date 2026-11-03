/* Scheda audio di gioca (lotto H6, brief 50): raccordo fra il gioco in C e il
 * driver audio tradotto (audio_*.c). Le scritture del gioco al latch e800
 * diventano NMI del driver; per ogni frame il driver gira due IRQ (0038 +
 * passata 029E); i cambi dei registri YM2149 vanno a emu2149 (scheda_pcm.c),
 * che produce PCM a 44100 Hz mono, 735 campioni per frame.
 * Ordine misurato su MAME nel frame F: IRQ A, latch (NMI 0066), IRQ B. */
#ifndef SCHEDA_AUDIO_H
#define SCHEDA_AUDIO_H

#include <stdint.h>

#define SCHEDA_CAMPIONI_FRAME 735

/* Apre i file facoltativi (NULL = non scrivere): flusso dei cambi AY (formato
 * di audio.lua) e registro dei latch (formato di partita_latch.txt). */
void scheda_audio_apri(const char *flusso_ay, const char *registro_latch);
/* Chiamata dal gancio di e800 (nella NMI del frame corrente). */
void scheda_audio_latch(uint8_t v);
/* Da passo(): completa il frame precedente (latch in coda, IRQ B) ed esegue
 * l'IRQ A del frame. La prima chiamata avvia il driver e recupera i frame
 * 0..frame-1 con soli IRQ. */
void scheda_audio_frame(long frame);
/* Completa il frame corrente, chiude i file e il chip. */
void scheda_audio_chiudi(void);
/* Se dall'ultima chiamata si è chiuso un frame "vero" (non di recupero)
 * copia in buf i 735 campioni e ritorna 735, altrimenti 0. */
int scheda_audio_pcm(int16_t *buf);
/* FNV-1a 64 bit dei byte (little-endian) di tutti i campioni dei frame chiusi,
 * recupero compreso; numero dei campioni. */
uint64_t scheda_audio_hash(void);
long scheda_audio_campioni(void);
/* 1 = A, latch, B (predefinita); 0 = latch, A, B; 2 = A, B, latch. */
extern int scheda_audio_fase;

/* Interno (scheda_pcm.c): resa del frame appena chiuso; fine = libera i chip. */
void scheda_pcm_frame(int recupero);
void scheda_pcm_fine(void);

#endif
