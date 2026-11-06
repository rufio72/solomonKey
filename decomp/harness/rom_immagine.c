/* Prova di andata e ritorno delle tabelle di tecmo/ (brief 62): rom_immagine
 * confronta ogni voce di rom_voci[] con out/main64.bin (o ../out/) e
 * controlla che le voci F000-FFFF coprano l'intera finestra copiata in ram[]
 * da macchina.c e macchina_libera.c. Uscita 0 se coincide, 2 altrimenti. */
#include <stdio.h>
#include <stdint.h>
#include "../../tecmo/rom_dati.h"

static uint8_t immagine[0x10000];

int main(void)
{
    const char *nomi[2] = { "out/main64.bin", "../out/main64.bin" };
    const char *nome = NULL;
    for (unsigned i = 0; i < 2 && !nome; i++) {
        FILE *f = fopen(nomi[i], "rb");
        if (!f) continue;
        size_t n = fread(immagine, 1, sizeof immagine, f);
        fclose(f);
        if (n != sizeof immagine) {
            fprintf(stderr, "ERRORE: %s: lunghezza sbagliata\n", nomi[i]);
            return 2;
        }
        nome = nomi[i];
    }
    if (!nome) {
        fprintf(stderr, "ERRORE: main64.bin non trovato in out/ né in ../out/\n");
        return 2;
    }
    unsigned long totale = 0;
    unsigned finestra = 0;
    for (unsigned i = 0; i < rom_voci_n; i++) {
        const RomVoce *v = &rom_voci[i];
        for (unsigned k = 0; k < v->lung; k++)
            if (v->dati[k] != immagine[v->inizio + k]) {
                fprintf(stderr, "ERRORE: byte diverso a %04X\n", v->inizio + k);
                return 2;
            }
        totale += v->lung;
        if (v->inizio >= 0xf000) finestra += v->lung;
    }
    if (finestra != 0x1000) {
        fprintf(stderr, "ERRORE: le voci F000-FFFF coprono %u byte, non 4096\n", finestra);
        return 2;
    }
    printf("rom_immagine: %u voci, %lu byte, coincide con out/main64.bin\n", rom_voci_n, totale);
    return 0;
}
