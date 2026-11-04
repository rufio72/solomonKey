/* Finestra SDL2 per il framebuffer di rendi_frame (lotto H3). */
#ifndef VIDEO_SDL_H
#define VIDEO_SDL_H
#include <stdint.h>
#include "video.h"

typedef struct { int scala; long headless; long massimo; } vsdl_opz;

/* -s/--scala N (1-8, predefinita 3), -n/--headless N (N>=1, senza finestra,
 * SDL non viene nemmeno inizializzata), -m/--max N (con finestra, esce dopo
 * N frame; serve alle prove con SDL_VIDEODRIVER=dummy). -n e -m si escludono.
 * Le opzioni con argomento proprio del programma (-d, -o) non sono gestite
 * qui: vanno tolte da argv prima, oppure il programma le legge a parte.
 * Ritorna l'indice del primo argomento non opzione, -1 se errore (messaggio
 * in italiano su stderr). Azzerata = finestra, scala 3, nessun limite. */
int vsdl_leggi_opzioni(int argc, char **argv, vsdl_opz *o);
/* Da chiamare prima di vsdl_apri: schermo intero «desktop» con scala intera
 * (proporzioni mantenute, bordi neri), cursore nascosto. Predefinito: spento. */
void vsdl_pieno(int on);
/* Da chiamare prima di vsdl_apri: righe scure alla CRT (lotto 57). Una
 * texture 1x(2*VID_ALTO) semitrasparente sopra il frame scurisce la metà
 * inferiore di ogni riga sorgente (alpha 80, circa 31%). Solo presentazione:
 * il framebuffer e vsdl_hash non cambiano. Predefinito: spento. */
void vsdl_scanlines(int quarti);
int vsdl_apri(const char *titolo, int scala);   /* 0 ok, -1 errore */
/* Mostra il frame (SDL_UpdateTexture + RenderCopy + RenderPresent), smaltisce
 * gli eventi. 0 = continua; 1 = l'utente vuole uscire (ESC o chiusura finestra);
 * -1 = errore. */
int vsdl_presenta(const uint8_t fb[VID_ALTO][VID_LARGO][3]);
int vsdl_vsync(void);                            /* 1 se il vsync è attivo */
void vsdl_chiudi(void);
/* FNV-1a 64 bit sui VID_ALTO*VID_LARGO*3 byte, riga per riga. Non usa SDL. */
uint64_t vsdl_hash(const uint8_t fb[VID_ALTO][VID_LARGO][3]);
#endif
