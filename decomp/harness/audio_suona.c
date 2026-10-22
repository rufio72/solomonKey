/* Un comando dal C del driver audio (brief 22):
 * ./harness/audio_suona <cc> <frame_fine> <uscita_ay.txt>  (da decomp/)
 *
 * Stesso modello del tempo di audio_banco, senza oracolo: avvio al frame 0,
 * codice cc (esadecimale) al frame FRAME_CODICE come nelle corse lungo_*,
 * poi per ogni frame due volte IRQ (0038) e una passata di 029E. Scrive il
 * flusso dei cambi dei registri fino a frame_fine, nello stesso formato di
 * out/audio/banco/lungo_<cc>_ay.txt (identico a parità di frame_fine).
 * AUDIO_DATI=estratti / vuoto come in audio_banco (conferma su stderr).
 * Uscita 0 se tutto bene, 2 per errori di argomenti o di file. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"

#define FRAME_CODICE 120        /* frame del codice forzato nelle corse lungo_* */

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "uso (da decomp/): audio_suona <cc> <frame_fine> <uscita_ay.txt>\n");
        return 2;
    }
    char *fine;
    long cc = strtol(argv[1], &fine, 16);
    if (*fine || cc < 0 || cc > 0xFF) {
        fprintf(stderr, "ERRORE: codice non valido: %s\n", argv[1]);
        return 2;
    }
    int frame_fine = atoi(argv[2]);
    if (frame_fine < 0) {
        fprintf(stderr, "ERRORE: frame finale non valido: %s\n", argv[2]);
        return 2;
    }

    aud_carica_rom();
    const char *dati = getenv("AUDIO_DATI");   /* origine della regione 05C6-3FFF */
    if (dati && strcmp(dati, "estratti") == 0) {
        aud_usa_dati_estratti(1);
        fprintf(stderr, "dati: estratti\n");
    } else if (dati && strcmp(dati, "vuoto") == 0) {
        aud_usa_dati_estratti(2);
        fprintf(stderr, "dati: vuoto\n");
    }
    aud_azzera();
    if (!aud_flusso_apri(argv[3])) {
        fprintf(stderr, "ERRORE: non riesco a scrivere %s\n", argv[3]);
        return 2;
    }

    aud_frame = 0;
    aud_avvio();
    for (int f = 0; f <= frame_fine; f++) {
        aud_frame = f;
        if (f == FRAME_CODICE) {
            aud_flusso_codice(FRAME_CODICE, (uint8_t)cc);
            aud_0066((uint8_t)cc);
        }
        for (int irq = 0; irq < 2; irq++) {
            aud_0038();
            aud_029e();
        }
    }
    aud_flusso_chiudi();
    return 0;
}
