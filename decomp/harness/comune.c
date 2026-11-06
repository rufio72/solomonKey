/* Memoria comune a harness e replay: RAM c000-ffff, immagine della ROM e
 * suo caricamento (spostati da harness.c, brief 23); dal brief 62 la ROM si
 * ricompone dalle tabelle di tecmo/ invece di leggere main64.bin. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ram.h"
#include "../../tecmo/rom_dati.h"

uint8_t ram[0x4000];
uint8_t rom[0x10000];
uint8_t rom_coperto[0xC000];

/* Punti di sincronia per la NMI del giro dal boot (ram.h); NULL = spento. */
void (*macchina_punto)(uint16_t da, uint16_t a) = NULL;
/* Frame pieno del gioco libero (ram.h); NULL = spento. */
void (*macchina_frame_pieno)(void) = NULL;
/* Costo del gioco libero (ram.h); NULL = spento. */
void (*macchina_costo)(int n) = NULL;

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

/* Ricompone rom[] dalle tabelle di tecmo/ (estrai_rom.py, brief 62): zero
 * ovunque, poi ogni voce al suo indirizzo; rom_coperto[] segna i byte sotto
 * c000 che vengono da una voce (la sentinella della lettura avvisa sugli altri).
 * Non legge più out/main64.bin. */
void carica_rom(void)
{
    memset(rom, 0, sizeof rom);
    memset(rom_coperto, 0, sizeof rom_coperto);
    for (unsigned i = 0; i < rom_voci_n; i++) {
        const RomVoce *v = &rom_voci[i];
        memcpy(&rom[v->inizio], v->dati, v->lung);
        for (unsigned k = 0; k < v->lung; k++) {
            unsigned a = (unsigned)v->inizio + k;
            if (a < 0xc000) rom_coperto[a] = 1;
        }
    }
}

/* Sentinella della lettura in ram.h: una lettura sotto c000 fuori da ogni
 * voce del censimento (CENSIMENTO-ROM.md). Avvisa una volta per indirizzo e
 * lascia leggere lo zero: il gioco non si ferma. */
void rom_fuori_voce(uint16_t a)
{
    static uint8_t visti[0x10000 / 8];
    if (visti[a >> 3] & (1 << (a & 7))) return;
    visti[a >> 3] |= (uint8_t)(1 << (a & 7));
    fprintf(stderr, "AVVISO: lettura ROM fuori voce a %04X\n", a);
}
