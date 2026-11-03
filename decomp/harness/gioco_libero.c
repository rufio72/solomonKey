/* Giro dal boot (lotto H4a, brief 44): il gioco parte dal vettore 0000 e
 * gira senza che l'oracolo di MAME lo guidi; dell'oracolo restano il
 * controllo (impronta a ogni NMI, boot 66-602) e gli ingressi registrati.
 * Uso: ./gioco_libero <oracolo.txt> <disp.txt> <ingressi.txt> [frame_fine]
 *   oracolo.txt, disp.txt: out/giroboot_oracolo.txt e out/giroboot_disp.txt
 *     (giro_boot.sh); accanto serve il dump out/giroboot_602.bin
 *   ingressi.txt: out/ingressi_replay.txt (estrai_ingressi.sh)
 * Si ferma, con codice 1, alla prima attività il cui corpo non è tradotto
 * (oggi: CORPO 07 al frame 1188, brief 45); vedi GIRO.md. */
#include <stdio.h>
#include <stdlib.h>

int macchina_esegui(const char *oracolo, const char *disp, const char *ingr, long fine);

int main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr, "uso: %s <oracolo.txt> <disp.txt> <ingressi.txt> [frame_fine]\n",
                argv[0]);
        return 2;
    }
    long fine = argc > 4 ? atol(argv[4]) : -1;
    return macchina_esegui(argv[1], argv[2], argv[3], fine);
}
