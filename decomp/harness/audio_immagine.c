/* Prova di andata e ritorno dei dati audio: audio_immagine <file> scrive
 * nel file i byte 05C6-3FFF ricomposti da aud_dati_serializza (14906
 * byte), da confrontare con la stessa regione di 1.3jk. Il file è un dump:
 * va scritto fuori dal repository. */
#include <stdio.h>
#include "audio_dati.h"

int main(int argc, char **argv)
{
    static uint8_t img[0x4000];
    if (argc != 2) {
        fprintf(stderr, "uso: audio_immagine <file>\n");
        return 2;
    }
    aud_dati_serializza(img);
    FILE *f = fopen(argv[1], "wb");
    if (!f) {
        fprintf(stderr, "ERRORE: non riesco a scrivere %s\n", argv[1]);
        return 2;
    }
    size_t n = AUD_DATI_FINE - AUD_DATI_INIZIO;
    int ok = fwrite(img + AUD_DATI_INIZIO, 1, n, f) == n;
    if (fclose(f) != 0 || !ok) {
        fprintf(stderr, "ERRORE: scrittura di %s non riuscita\n", argv[1]);
        return 2;
    }
    return 0;
}
