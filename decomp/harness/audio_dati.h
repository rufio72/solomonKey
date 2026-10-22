/* Dati del Z80 audio (regione 05C6-3FFF di 1.3jk) come sorgente C.
 * I file audio_dati_*.c e audio_dati_*.h li scrive decomp/estrai_dati.py
 * leggendo la ROM: non modificarli a mano, si rigenerano.
 *
 * Ogni dato è un pezzo a indirizzo ROM fisso (ADR_*, in
 * audio_dati_indirizzi.h): i puntatori dei flussi (F4, F7), dei comandi e
 * degli inviluppi sono indirizzi della ROM e l'interprete li legge così.
 * aud_dati_serializza ricompone la regione byte per byte.
 *
 * Macro dei flussi: ognuna si espande nei byte esatti che l'interprete
 * legge (audio_interprete.c, AUDIO-SEQUENZE.md). Le parole sono little
 * endian come nella ROM. */
#ifndef AUDIO_DATI_H
#define AUDIO_DATI_H

#include <stdint.h>
#include "audio_dati_indirizzi.h"

#define AUD_DATI_INIZIO 0x05C6   /* primo byte dopo il codice del driver */
#define AUD_DATI_FINE   0x4000   /* fine della ROM */

struct aud_pezzo {
    uint16_t indirizzo, lung;
    const uint8_t *dati;         /* NULL = riempimento FF */
    const char *nome;
};

/* Voci degli elenchi di pezzi: array con lo stesso nome dell'etichetta */
#define PEZZO(nome)              { ADR_##nome, sizeof nome, nome, #nome }
#define RIEMPIMENTO(nome, lung)  { ADR_##nome, lung, NULL, #nome }

/* Riempie img (0x4000 byte) con i pezzi; exit(2) se due pezzi si
 * sovrappongono o se un byte di 05C6-3FFF resta scoperto. */
void aud_dati_serializza(uint8_t img[0x4000]);

/* ---- Byte e parole ---- */
#define B(x)  ((uint8_t)(x))                             /* byte non decodificato */
#define W(x)  ((uint8_t)((x) & 0xFF)), ((uint8_t)(((x) >> 8) & 0xFF))

/* ---- Note: byte 00-7F = (ottava << 4) | nota; nibble basso C-F = pausa ---- */
enum { DO, DOd, RE, REd, MI, FA, FAd, SOL, SOLd, LA, LAd, SI };
#define NOTA(o, n)  ((uint8_t)(((o) << 4) | (n)))
#define PAUSA       0x0C                                 /* altre pause in B() */

/* ---- Durata: byte 80-EF, indice nella tabella tab_durate ---- */
#define DUR(i)  ((uint8_t)(0x80 | (i)))

/* ---- Gestori F0-FF (tabella 03CE) ---- */
#define STRUM(n)             0xF0, ((uint8_t)(n))        /* inviluppo software n (0850) */
#define INV_HW(forma, p)     0xF0, 0xFF, ((uint8_t)(forma)), W(p)  /* forma e periodo */
#define INV_HW_NOTA(forma)   0xF0, 0xFE, ((uint8_t)(forma))        /* periodo dalla nota */
#define INV_HW_FORMA(forma)  0xF0, 0xFE, ((uint8_t)(forma)), 0xFF  /* solo forma */
#define ATTEN(x)             0xF1, ((uint8_t)(x))        /* attenuazione */
#define TRASP(x)             0xF2, ((uint8_t)((x) & 0xFF))  /* trasposizione in semitoni */
#define SCOST(x)             0xF3, ((uint8_t)(x))        /* scostamento delle durate */
#define SALTA(a)             0xF4, W(a)                  /* salto all'indirizzo a */
#define RIP_INIZIO(n)        0xF5, ((uint8_t)(n))        /* ripeti n volte (0 = 256) */
#define RIP_FINE             0xF6
#define CHIAMA(a)            0xF7, W(a)                  /* chiamata, un livello */
#define RITORNA              0xF8
#define RUMORE(x)            0xF9, ((uint8_t)(x))        /* rumore di periodo x */
#define RUMORE_OFF           0xFA
#define GLISSA(p0, p1, d)    0xFB, W(p0), W(p1), W(d)    /* da p0 a p1 in d + 1 IRQ */
#define PERIODO(p)           0xFC, W(p)                  /* nota a periodo diretto */
#define COMANDO(x)           0xFD, ((uint8_t)(x))        /* comando a catena */
#define ATTEN_PIU            0xFE                        /* attenuazione + 1 */
#define FINE                 0xFF                        /* fine del flusso */

/* ---- Elenchi dei comandi e inviluppi software ---- */
#define REC(blocco, a)       ((uint8_t)(blocco)), W(a)   /* blocco 0-17 e flusso */
#define FINE_ELENCO          0xFF
#define PASSO(vol, irq)      ((uint8_t)(vol)), ((uint8_t)(irq))  /* volume per irq IRQ */
#define RIPARTI              0xFF                        /* l'inviluppo riparte dall'inizio */

#endif
