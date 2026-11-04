/* Analisi delle opzioni nuove di gioca (lotto H7, brief 51). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gioca_opzioni.h"
#include "gioca_joy.h"

gioca_extra gx = { 0, 100, 0, 0, 0, 0, 0 };

static int intero(const char *s, long *v)
{
    char *fine;
    if (!s || !*s) return -1;
    *v = strtol(s, &fine, 10);
    return *fine ? -1 : 0;
}

int gioca_opzione_extra(int argc, char **argv, int *i)
{
    const char *a = argv[*i];
    long v;
    if (!strcmp(a, "--aiuto") || !strcmp(a, "-h")) {
        gioca_aiuto();
        exit(0);
    }
    if (!strcmp(a, "--pieno")) { gx.pieno = 1; return 1; }
    if (!strcmp(a, "--scanlines")) { gx.scanlines = 1; return 1; }
    if (!strcmp(a, "--joystick")) { gx.joystick = 1; return 1; }
    if (!strcmp(a, "--senza-joystick")) { gx.senza_joy = 1; return 1; }
    if (!strcmp(a, "--volume") || !strcmp(a, "--joy-mappa") || !strcmp(a, "-t")) {
        if (*i + 1 >= argc) { fprintf(stderr, "ERRORE: %s vuole un valore\n", a); return -1; }
        const char *val = argv[++*i];
        if (!strcmp(a, "--volume")) {
            if (intero(val, &v) != 0 || v < 0 || v > 100) {
                fprintf(stderr, "ERRORE: --volume vuole un intero da 0 a 100\n");
                return -1;
            }
            gx.volume = (int)v;
        } else if (!strcmp(a, "-t")) {
            if (intero(val, &v) != 0 || v < 0) {
                fprintf(stderr, "ERRORE: -t vuole un numero di secondi (0 = senza limite)\n");
                return -1;
            }
            gx.tempo = v; gx.tempo_dato = 1;
        } else if (joy_analizza_mappa(val, &joy_mappa_corrente) != 0) {
            fprintf(stderr, "ERRORE: --joy-mappa vuole quattro numeri B1,B2,START,COIN "
                    "(0-31 oppure -1), es. 2,3,9,8\n");
            return -1;
        }
        return 1;
    }
    return 0;
}

int gioca_extra_valida(void)
{
    if (gx.tempo_dato && !gx.joystick) {
        fprintf(stderr, "ERRORE: -t SEC si usa solo con --joystick\n");
        return -1;
    }
    return 0;
}
