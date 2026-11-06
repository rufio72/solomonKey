/* Prova di andata e ritorno dell'audio: audio_immagine <file> scrive nel
 * file l'immagine intera di 1.3jk (16384 byte) ricomposta come fa
 * aud_carica_rom: dati 05C6-3FFF da aud_dati_serializza e driver 0000-05C5
 * dal blob di tecmo/audio_driver.c. Da confrontare con cmp contro 1.3jk.
 * Il file è un dump: va scritto fuori dal repository. */
#include <stdio.h>
#include <string.h>
#include "audio_dati.h"
#include "audio_driver.h"

int main(int argc, char **argv)
{
    static uint8_t img[0x4000];
    if (argc != 2) {
        fprintf(stderr, "uso: audio_immagine <file>\n");
        return 2;
    }
    aud_dati_serializza(img);
    memcpy(img, aud_driver, sizeof aud_driver);
    FILE *f = fopen(argv[1], "wb");
    if (!f) {
        fprintf(stderr, "ERRORE: non riesco a scrivere %s\n", argv[1]);
        return 2;
    }
    int ok = fwrite(img, 1, sizeof img, f) == sizeof img;
    if (fclose(f) != 0 || !ok) {
        fprintf(stderr, "ERRORE: scrittura di %s non riuscita\n", argv[1]);
        return 2;
    }
    return 0;
}
