/* Banco di prova: ./harness <ENTRY> | --tutte
 * Per ogni cattura della routine (snap_<ENTRY>_prima.bin, tag «base», e
 * snap_<ENTRY>_<tag>_prima.bin) carica RAM e registri «prima», esegue la
 * routine tradotta e confronta RAM (c000-e5ff) e registri con lo stato «dopo»
 * misurato in MAME. Stampa OK/FALLITO per cattura e un RIEPILOGO finale. */
#define _POSIX_C_SOURCE 200809L
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "routine.h"

uint8_t ram[0x4000];
uint8_t rom[0x10000];
static uint8_t dopo[0x4000];

/* Carica main64.bin (ROM a indirizzi reali) da out/ o ../out/. */
static void carica_rom(void)
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

/* Carica 16 KB da <prefisso><suffisso>; 0 se il file manca o è corto. */
static int carica(const char *prefisso, const char *suffisso, uint8_t *dest)
{
    char nome[512];
    snprintf(nome, sizeof nome, "%s%s", prefisso, suffisso);
    FILE *f = fopen(nome, "rb");
    if (!f) {
        printf("file %s non trovato\n", nome);
        return 0;
    }
    size_t n = fread(dest, 1, 0x4000, f);
    fclose(f);
    if (n != 0x4000) {
        printf("%s: lunghezza sbagliata\n", nome);
        return 0;
    }
    return 1;
}

/* Legge una riga «prima|dopo PC=.. AF=.. ...» nei registri; 0 se manca. */
static int leggi_registri(const char *prefisso, const char *quale, struct z80 *z)
{
    char nome[512];
    snprintf(nome, sizeof nome, "%s_regs.txt", prefisso);
    FILE *f = fopen(nome, "r");
    if (!f) {
        printf("file %s non trovato\n", nome);
        return 0;
    }
    char riga[256];
    unsigned pc, af, bc, de, hl, ix, iy, sp;
    char fmt[128];
    snprintf(fmt, sizeof fmt,
             "%s PC=%%x AF=%%x BC=%%x DE=%%x HL=%%x IX=%%x IY=%%x SP=%%x", quale);
    while (fgets(riga, sizeof riga, f)) {
        if (sscanf(riga, fmt, &pc, &af, &bc, &de, &hl, &ix, &iy, &sp) == 8) {
            z->a = af >> 8; z->f = af & 0xff;
            SET_BC(z, bc); SET_DE(z, de); SET_HL(z, hl);
            z->ix = ix; z->iy = iy; z->sp = sp;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    printf("riga «%s» mancante in %s\n", quale, nome);
    return 0;
}

static int confronta_reg(const char *nome, unsigned c, unsigned mame)
{
    if (c == mame) return 0;
    printf("registro %s: C=%04X MAME=%04X\n", nome, c, mame);
    return 1;
}

/* Prova una cattura (prefisso dei file senza suffisso); 1 se coincide. */
static int prova(const struct voce_routine *v, const char *prefisso, const char *tag)
{
    printf("cattura %s\n", tag);
    struct z80 z, atteso;
    if (!carica(prefisso, "_prima.bin", ram) || !carica(prefisso, "_dopo.bin", dopo) ||
        !leggi_registri(prefisso, "prima", &z) || !leggi_registri(prefisso, "dopo", &atteso)) {
        printf("FALLITO %s %s\n", v->entry, tag);
        return 0;
    }

    v->fn(&z);

    int errori = 0, stampate = 0, pila_libera = 0;
    /* Pila libera sotto lo SP di ingresso: i push/call interni della routine
     * vi lasciano residui che il C non riproduce, quindi non si confronta. */
    unsigned pila_ini = (unsigned)z.sp - 0x40, pila_fine = z.sp;
    /* c000-e5ff: escluse le porte e la ROM f000 */
    for (unsigned i = 0; i < 0x2600; i++) {
        if (0xc000 + i >= pila_ini && 0xc000 + i < pila_fine) {
            pila_libera += (ram[i] != dopo[i]);
            continue;
        }
        if (ram[i] != dopo[i]) {
            if (stampate++ < 8)
                printf("%04X: prima->%02X dopo %02X\n", 0xc000 + i, ram[i], dopo[i]);
            errori++;
        }
    }
    if (pila_libera)
        printf("pila libera %04X-%04X: %d byte diversi, non confrontati\n",
               pila_ini, pila_fine - 1, pila_libera);
    if (v->blocco_ini) {
        unsigned blocco = 0;
        for (unsigned a = v->blocco_ini; a <= v->blocco_fine; a++)
            blocco += (M(a) == dopo[a - 0xc000]);
        unsigned attesi = v->blocco_fine - v->blocco_ini + 1;
        printf("byte %04X-%04X coincidenti: %u su %u\n", v->blocco_ini, v->blocco_fine,
               blocco, attesi);
        if (blocco != attesi) errori++;
    }

    errori += confronta_reg("A", z.a, atteso.a);
    errori += confronta_reg("BC", BC(&z), BC(&atteso));
    errori += confronta_reg("DE", DE(&z), DE(&atteso));
    errori += confronta_reg("HL", HL(&z), HL(&atteso));
    errori += confronta_reg("IX", z.ix, atteso.ix);
    errori += confronta_reg("IY", z.iy, atteso.iy);
    printf("registri C: A=%02X BC=%04X DE=%04X HL=%04X IX=%04X IY=%04X\n",
           z.a, BC(&z), DE(&z), HL(&z), z.ix, z.iy);

    printf("%s %s %s\n", errori ? "FALLITO" : "OK", v->entry, tag);
    return !errori;
}

static int esiste(const char *nome)
{
    FILE *f = fopen(nome, "r");
    if (f) fclose(f);
    return f != NULL;
}

/* Prova tutte le catture di una routine nella prima cartella fra SNAP_DIR,
 * out e ../out che ne contiene; 1 se almeno una e tutte coincidono. */
static int prova_routine(const struct voce_routine *v)
{
    const char *dirs[] = { getenv("SNAP_DIR"), "out", "../out" };
    char base[512], modello[512];
    glob_t g = { 0 };
    const char *dir = NULL;
    for (unsigned i = 0; i < 3 && !dir; i++) {
        if (!dirs[i]) continue;
        snprintf(base, sizeof base, "%s/snap_%s_regs.txt", dirs[i], v->entry);
        snprintf(modello, sizeof modello, "%s/snap_%s_*_regs.txt", dirs[i], v->entry);
        globfree(&g);
        memset(&g, 0, sizeof g);
        int trovati = glob(modello, 0, NULL, &g) == 0 ? (int)g.gl_pathc : 0;
        if (esiste(base) || trovati) dir = dirs[i];
    }

    unsigned n = 0, k = 0;
    if (dir) {
        char prefisso[512];
        snprintf(prefisso, sizeof prefisso, "%s/snap_%s", dir, v->entry);
        if (esiste(base)) {
            n++;
            k += prova(v, prefisso, "base");
        }
        size_t lp = strlen(prefisso);
        for (size_t i = 0; i < g.gl_pathc; i++) {
            /* tag = parte fra «snap_<ENTRY>_» e «_regs.txt» */
            const char *p = g.gl_pathv[i];
            size_t lt = strlen(p) - lp - 1 - strlen("_regs.txt");
            char tag[128], pref_tag[640];
            snprintf(tag, sizeof tag, "%.*s", (int)lt, p + lp + 1);
            snprintf(pref_tag, sizeof pref_tag, "%s_%s", prefisso, tag);
            n++;
            k += prova(v, pref_tag, tag);
        }
    }
    globfree(&g);
    printf("RIEPILOGO %s: %u catture, %u OK\n", v->entry, n, k);
    return n >= 1 && k == n;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "uso: %s <ENTRY> | --tutte   (es. 0x05B3)\n", argv[0]);
        return 2;
    }
    carica_rom();
    if (strcmp(argv[1], "--tutte") == 0) {
        int tutte = 1;
        for (unsigned i = 0; i < num_routine; i++)
            tutte &= prova_routine(&tabella_routine[i]);
        return tutte ? 0 : 1;
    }
    for (unsigned i = 0; i < num_routine; i++)
        if (strcmp(argv[1], tabella_routine[i].entry) == 0)
            return prova_routine(&tabella_routine[i]) ? 0 : 1;
    fprintf(stderr, "routine %s non tradotta\n", argv[1]);
    return 2;
}
