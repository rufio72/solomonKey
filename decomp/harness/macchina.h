/* Macchina del giro dal boot: interfaccia verso gioca (lotto H5, brief 49). */
#ifndef MACCHINA_H
#define MACCHINA_H

#include <stdint.h>

/* Modo con oracolo (macchina.c): il giro di H4. */
int macchina_esegui(const char *oracolo, const char *disp, const char *ingr, long fine);
void boot_2833(void);
void gancio_porta(uint16_t a, uint8_t v);
extern uint8_t maschera_nmi, ribalta;

/* DSW scritti in e604/e605 dal boot (predefiniti 02 e 00, come la registrazione). */
extern uint8_t macchina_dsw1, macchina_dsw2;
/* Chiamata a ogni NMI dopo la scrittura delle porte e prima di sub_0066. */
extern void (*macchina_prima_nmi)(long frame);
/* Chiamata a ogni scrittura a e800 (latch audio), se non nulla (lotto H6). */
extern void (*macchina_latch_audio)(uint8_t v);

/* Modo libero (macchina_libera.c): senza oracolo. */
struct libera_opz { long frame_max; /* 0 = illimitato */ };
extern struct libera_opz libera_opz;
/* Chiamata a ogni NMI prima di sub_0066: scrive in v[0..2] gli ingressi del
 * frame (e600,e601,e602), rende/presenta. Ritorna 0 = continua, 1 = esci. */
extern int (*libera_frame)(long frame, uint8_t v[3]);
/* 0 fine pulita, 3 fermo per codice non tradotto. */
int macchina_libera_esegui(void);

#endif
