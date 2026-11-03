/* Mostra il giro replay 2410-5499 in una finestra SDL2 (lotto H3):
 *   mostra_replay [-s N] [-n N | -m N] [-o cartella]
 * A fine di ogni NMI del replay rende la RAM video con rendi_frame (gancio
 * replay_dopo_nmi). Con -n N non usa SDL. Riepilogo: HASH primo, HASH ultimo,
 * NMI primo/ultimo, FRAME rese. replay.c è compilato con -Dmain=replay_main. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "video_sdl.h"
#include "ram.h"

int replay_main(int argc, char **argv);
extern void (*replay_dopo_nmi)(long frame);

static vsdl_opz opz;
static uint8_t reso[VID_ALTO][VID_LARGO][3];
static long rese, frame_primo = -1, frame_ultimo = -1;
static uint64_t hash_primo, hash_ultimo;

static void riepilogo(void)
{
    printf("HASH primo %016llx\n", (unsigned long long)hash_primo);
    printf("HASH ultimo %016llx\n", (unsigned long long)hash_ultimo);
    printf("NMI primo %ld ultimo %ld\n", frame_primo, frame_ultimo);
    printf("FRAME rese %ld\n", rese);
    fflush(stdout);
}

static void esci_ok(void)
{
    if (!opz.headless) vsdl_chiudi();
    riepilogo();
    exit(0);
}

static void dopo_nmi(long frame)
{
    /* indirizzi Z80 d000, d400, d800, dc00, e000, e400 meno la base c000 */
    vid_ram v = { &ram[0xd000 - 0xc000], &ram[0xd400 - 0xc000], &ram[0xd800 - 0xc000],
                  &ram[0xdc00 - 0xc000], &ram[0xe000 - 0xc000], &ram[0xe400 - 0xc000] };
    rendi_frame(&v, reso);
    uint64_t h = vsdl_hash(reso);
    if (rese == 0) { hash_primo = h; frame_primo = frame; }
    hash_ultimo = h;
    frame_ultimo = frame;
    rese++;
    if (opz.headless) {
        if (rese >= opz.headless) esci_ok();
        return;
    }
    int r = vsdl_presenta(reso);
    if (r != 0 || (opz.massimo && rese >= opz.massimo)) {
        if (r < 0) {
            vsdl_chiudi();
            riepilogo();
            exit(2);
        }
        esci_ok();
    }
}

int main(int argc, char **argv)
{
    const char *dir = "out";
    char *av[64];
    int ac = 0;
    if (argc > 64) { fprintf(stderr, "ERRORE: troppi argomenti\n"); return 2; }
    for (int i = 0; i < argc; i++) {
        if (i > 0 && !strcmp(argv[i], "-o")) {
            if (i + 1 >= argc) { fprintf(stderr, "ERRORE: -o vuole una cartella\n"); return 2; }
            dir = argv[++i];
        } else
            av[ac++] = argv[i];
    }
    int k = vsdl_leggi_opzioni(ac, av, &opz);
    if (k < 0) return 2;
    if (k != ac) {
        fprintf(stderr, "uso: mostra_replay [-s N] [-n N | -m N] [-o cartella]\n");
        return 2;
    }
    char p1[1024], p2[1024], p3[1024];
    snprintf(p1, sizeof p1, "%s/replay_oracolo.txt", dir);
    snprintf(p2, sizeof p2, "%s/replay_disp.txt", dir);
    snprintf(p3, sizeof p3, "%s/replay_2410.bin", dir);
    char *argr[] = { "replay", p1, p2, p3, "2410" };

    if (!opz.headless && vsdl_apri("Solomon's Key - replay", opz.scala) != 0) return 2;
    replay_dopo_nmi = dopo_nmi;
    int rc = replay_main(5, argr);
    if (!opz.headless) vsdl_chiudi();
    riepilogo();
    return rc;
}
