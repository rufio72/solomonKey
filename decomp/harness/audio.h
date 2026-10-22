/* Z80 audio (ROM 1.3jk) tradotto in C: modello di memoria e routine.
 * Spazio del Z80 audio: 0000-3fff ROM, 4000-47ff RAM, 8000 latch (lettura),
 * ffff nop (scrittura); io: tre YM2149 a 10/11, 20/21, 30/31 (pari =
 * indirizzo, dispari = dato). Nomi col prefisso aud_ per non urtare quelli
 * del principale. Nessun byte della ROM qui: la ROM si legge a tempo di
 * esecuzione (aud_carica_rom). */
#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

extern uint8_t aud_ram[0x800];                 /* 4000-47ff */
extern uint8_t aud_rom[0x4000];                /* 1.3jk */
#define AM(a) aud_ram[(uint16_t)(a) - 0x4000]

/* Indirizzi RAM con nome */
#define AUD_RAM            0x4000   /* inizio della RAM e del primo blocco */
#define AUD_RAM_LUNG       0x0800
#define AUD_BLOCCHI        0x4000   /* 18 blocchi voce da 0x20 byte */
#define AUD_N_BLOCCHI      18
#define AUD_PASSO_BLOCCO   0x20
#define AUD_OMBRE_MIXER    0x4240   /* 4240-4242: registro 7 dei chip 1-3 */
#define AUD_COMANDO        0x4243   /* comando scritto dalla NMI (+1) */
#define AUD_PILA           0x4800   /* sp iniziale */
#define AUD_LATCH          0x8000   /* lettura del latch audio */

/* Tabelle della ROM (solo indirizzi) */
#define AUD_TAB_COMANDI    0x0AF3   /* parola per comando 01-40 */
#define AUD_TAB_GESTORI    0x03CE   /* 16 parole: gestori dei byte F0-FF */
#define AUD_TAB_TONI       0x0700   /* parole: periodo per indice < 30 */
#define AUD_TAB_TONI_B     0x0760   /* byte: periodo per indice >= 30 */
#define AUD_TAB_DURATE     0x0790   /* parole: durate dei byte 80-EF */
#define AUD_TAB_INVILUPPI  0x0850   /* parole: inizio degli inviluppi software */

/* Campi del blocco (scostamento dall'inizio); +1D-+1F mai usati */
/* flag: bit 0 attivo, bit 1 da spegnere, bit 2 coperto (dal secondo
 * strato), bit 3 muto (pausa), bit 4 glissando, bit 5-6 inviluppo
 * hardware (20 forma e periodo, 40 solo forma, 60 forma e periodo dalla
 * nota) */
#define BL_FLAG      0x00
#define BL_PUNT      0x01   /* +01/+02: puntatore di sequenza (parola) */
#define BL_DURATA    0x03   /* +03/+04: IRQ restanti della nota (16 bit) */
#define BL_04        0x04
#define BL_INV_CONT  0x05   /* IRQ restanti del passo d'inviluppo software */
#define BL_INV_PUNT  0x06   /* +06/+07: passo corrente dell'inviluppo software */
#define BL_VOLUME    0x08
#define BL_ATTENUA   0x09
#define BL_RUMORE    0x0A   /* 0 = tono; altrimenti periodo del rumore + 1 */
#define BL_TONO_FINE 0x0B
#define BL_TONO_GROS 0x0C
#define BL_GLISSA    0x0D   /* +0D/+0E: passo del glissando (16 bit) */
#define BL_FORMA     0x0F   /* forma dell'inviluppo, azzerata dopo l'uso */
#define BL_INV_FINE  0x10
#define BL_INV_GROS  0x11
#define BL_DUR_NOTA  0x12   /* +12/+13: durata delle note (meno 1) */
#define BL_INV_INIZIO 0x14  /* +14/+15: inizio dell'inviluppo software */
#define BL_16        0x16
#define BL_17        0x17
#define BL_RIP_PUNT  0x18   /* +18/+19: inizio della ripetizione (F5/F6) */
#define BL_RIP_CONT  0x1A   /* giri restanti della ripetizione */
#define BL_RIT       0x1B   /* +1B/+1C: ritorno della chiamata (F7/F8) */

/* Piattaforma (audio_piattaforma.c) */
extern int aud_frame;                          /* frame corrente per il flusso */
void aud_carica_rom(void);                     /* 1.3jk o exit(2) */
void aud_azzera(void);                         /* RAM, ombre, contatori */
extern uint8_t aud_latch;                      /* ultimo valore del latch (8000) */
uint8_t aud_leggi(uint16_t a);                 /* ROM sotto 4000, RAM 4000-47ff */
void aud_scrivi(uint16_t a, uint8_t v);        /* solo RAM 4000-47ff, il resto si perde */
void aud_out(uint8_t porta, uint8_t v);        /* out (c),r: semantica di audio.lua */
int aud_ombra(int chip, int reg);              /* -1 se mai scritto */
unsigned aud_ignote(uint8_t porta);            /* scritture a porte ignote */
int aud_flusso_apri(const char *percorso);     /* 0 se non si apre */
void aud_flusso_codice(int frame, uint8_t cc); /* riga «# codice» */
void aud_flusso_chiudi(void);
/* chiamata a ogni cambio di registro (dopo la scrittura nel flusso) */
extern void (*aud_osservatore)(int chip, int reg, int val);

/* Ossatura (audio_ossatura.c) */
void aud_avvio(void);                          /* 0072-009D senza il ciclo infinito */
void aud_00a4(void);
void aud_0038(void);                           /* IRQ: aud_0147(); aud_00e4(); */
void aud_0066(uint8_t latch);                  /* NMI */
void aud_00e4(void);
uint16_t aud_0008(uint16_t hl, uint8_t a);
uint16_t aud_0010(uint16_t *hl);
uint8_t aud_0018(uint16_t ix);

/* Registri AY (audio_registri.c) */
void aud_029e(void);                           /* una passata */

/* Interprete (audio_interprete.c) */
void aud_0147(void);                           /* interprete: una passata sui 18 blocchi */

#endif
