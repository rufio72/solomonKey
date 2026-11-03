/* Joystick di gioca (lotto H7, brief 51): API joystick grezza di SDL (non
 * GameController), così gli indici dei pulsanti sono quelli veri del
 * dispositivo. Tutti i joystick presenti sono aperti (hotplug compreso) e si
 * sommano in OR alla tastiera. */
#ifndef GIOCA_JOY_H
#define GIOCA_JOY_H
#include <stdint.h>
#include <SDL.h>

typedef struct { int b1, b2, start, coin; } joy_mappa;   /* indici pulsante, -1 = nessuno */
extern joy_mappa joy_mappa_corrente;                       /* predefinita: 0, 1, 7, 6 */

int  joy_analizza_mappa(const char *s, joy_mappa *m);      /* "B1,B2,START,COIN"; 0 ok, -1 errore */
void joy_apri(void);                  /* init sottosistema, apre i joystick presenti, hotplug */
void joy_chiudi(void);
void joy_ingressi(uint8_t out[3]);    /* OR sui byte e600/e602 di tutti i joystick aperti */
void joy_elenca(void);                /* una riga JOYSTICK per dispositivo (diagnostica) */

/* Azione di gioco assegnata dalla mappa corrente (NULL se nessuna); per la
 * diagnostica. L'hat restituisce le direzioni unite da '+' (buffer statico). */
const char *joy_azione_pulsante(int numero);
const char *joy_azione_asse(int asse, int valore);
const char *joy_azione_hat(int hat, uint8_t valore);
#endif
