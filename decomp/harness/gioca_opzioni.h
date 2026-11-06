/* Opzioni di gioca aggiunte in H7 (brief 51), estratte prima di
 * vsdl_leggi_opzioni. */
#ifndef GIOCA_OPZIONI_H
#define GIOCA_OPZIONI_H

typedef struct {
    int pieno;          /* --pieno */
    int volume;         /* --volume N, 0-100 */
    int joystick;       /* --joystick */
    int senza_joy;      /* --senza-joystick */
    long tempo;         /* -t SEC (solo con --joystick) */
    int tempo_dato;
    int scanlines;      /* --scanlines [S] (lotto 57): 0 spente, 1-3 quarti di fila */
    int stanza;         /* --stanza N (collaudo): quadro iniziale c243 0-51, -1 = spento */
} gioca_extra;

extern gioca_extra gx;

/* Prova argv[*i]: 1 = riconosciuta (e consumati gli argomenti, *i aggiornato),
 * 0 = non è una delle nuove, -1 = errore (messaggio su stderr, uscire con 2).
 * --aiuto/-h stampano il testo ed escono con 0. */
int gioca_opzione_extra(int argc, char **argv, int *i);
/* Controlli finali (-t solo con --joystick). 0 ok, -1 errore. */
int gioca_extra_valida(void);

void gioca_aiuto(void);
int joy_diagnostica(long secondi_max);
#endif
