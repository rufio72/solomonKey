/* Memoria comune a harness e replay: RAM c000-ffff, immagine della ROM e
 * caricamento di main64.bin (spostati da harness.c, brief 23). */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ram.h"

uint8_t ram[0x4000];
uint8_t rom[0x10000];

/* Punti di sincronia per la NMI del giro dal boot (ram.h); NULL = spento. */
void (*macchina_punto)(uint16_t da, uint16_t a) = NULL;
/* Frame pieno del gioco libero (ram.h); NULL = spento. */
void (*macchina_frame_pieno)(void) = NULL;

/* Percorso `relativo` rispetto alla cartella dell'eseguibile (Linux:
 * /proc/self/exe), scritto in buf. NULL se non si può calcolare. */
const char *percorso_exe(const char *relativo, char *buf, size_t n)
{
    char exe[4096];
    ssize_t k = readlink("/proc/self/exe", exe, sizeof exe - 1);
    if (k <= 0) return NULL;
    exe[k] = 0;
    char *barra = strrchr(exe, '/');
    if (!barra) return NULL;
    *barra = 0;
    if (snprintf(buf, n, "%s/%s", exe, relativo) >= (int)n) return NULL;
    return buf;
}

/* Carica main64.bin (ROM a indirizzi reali) da out/ o ../out/ e, in coda,
 * da <cartella dell'eseguibile>/../out/ (così gioca parte da ovunque). */
void carica_rom(void)
{
    char esterno[4200];
    const char *nomi[3] = { "out/main64.bin", "../out/main64.bin", NULL };
    nomi[2] = percorso_exe("../out/main64.bin", esterno, sizeof esterno);
    for (unsigned i = 0; i < 3; i++) {
        if (!nomi[i]) continue;
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
    fprintf(stderr, "main64.bin non trovato in out/ né in ../out/\n"
                    "Esegui: make -C decomp/harness gioco\n");
    exit(2);
}
