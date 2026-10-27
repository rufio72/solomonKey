/* Memoria comune a harness e replay: RAM c000-ffff, immagine della ROM e
 * caricamento di main64.bin (spostati da harness.c, brief 23). */
#include <stdio.h>
#include <stdlib.h>
#include "ram.h"

uint8_t ram[0x4000];
uint8_t rom[0x10000];

/* Carica main64.bin (ROM a indirizzi reali) da out/ o ../out/. */
void carica_rom(void)
{
    const char *nomi[] = { "out/main64.bin", "../out/main64.bin" };
    for (unsigned i = 0; i < 2; i++) {
        FILE *f = fopen(nomi[i], "rb");
        if (!f) continue;
        size_t n = fread(rom, 1, sizeof rom, f);
        fclose(f);
        if (n != sizeof rom) {
            fprintf(stderr, "%s: lunghezza sbagliata\n", nomi[i]);
            exit(2);
        }
        return;
    }
    fprintf(stderr, "main64.bin non trovato in out/ né in ../out/\n");
    exit(2);
}
