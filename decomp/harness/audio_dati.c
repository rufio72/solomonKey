/* Serializzatore dei dati audio: ricompone la regione 05C6-3FFF di 1.3jk
 * dai pezzi dei file audio_dati_*.c (generati da decomp/estrai_dati.py).
 * L'immagine parte piena di 0xAA (byte avvelenato); la copertura si
 * controlla con una mappa a parte, perché 0xAA è anche un byte valido dei
 * dati (DUR(42)). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio_dati.h"
#include "audio_dati_gruppi.h"

struct gruppo { const struct aud_pezzo *v; const unsigned *n; };
static const struct gruppo gruppi[] = { AUD_DATI_GRUPPI };

void aud_dati_serializza(uint8_t img[0x4000])
{
    static const char *chi[0x4000];             /* pezzo che copre ogni byte */
    memset(img, 0xAA, 0x4000);
    memset(chi, 0, sizeof chi);
    for (size_t g = 0; g < sizeof gruppi / sizeof gruppi[0]; g++) {
        for (unsigned i = 0; i < *gruppi[g].n; i++) {
            const struct aud_pezzo *p = &gruppi[g].v[i];
            if (p->indirizzo < AUD_DATI_INIZIO || p->indirizzo + p->lung > AUD_DATI_FINE) {
                fprintf(stderr, "ERRORE: pezzo %s fuori da 05C6-3FFF (%04X, %u byte)\n",
                        p->nome, p->indirizzo, p->lung);
                exit(2);
            }
            for (unsigned k = 0; k < p->lung; k++) {
                unsigned a = p->indirizzo + k;
                if (chi[a]) {
                    fprintf(stderr, "ERRORE: i pezzi %s e %s si sovrappongono a %04X\n",
                            chi[a], p->nome, a);
                    exit(2);
                }
                chi[a] = p->nome;
                img[a] = p->dati ? p->dati[k] : 0xFF;
            }
        }
    }
    for (unsigned a = AUD_DATI_INIZIO; a < AUD_DATI_FINE; a++) {
        if (!chi[a]) {
            fprintf(stderr, "ERRORE: byte %04X scoperto (resta 0xAA)\n", a);
            exit(2);
        }
    }
}
