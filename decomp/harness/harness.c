/* Banco di prova: ./harness <ENTRY>
 * Carica ../out/snap_<ENTRY>_prima.bin e i registri «prima», esegue la
 * routine tradotta e confronta RAM (c000-e5ff) e registri con lo stato «dopo»
 * misurato in MAME. Stampa OK <ENTRY> o FALLITO <ENTRY>. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ram.h"

uint8_t ram[0x4000];
static uint8_t dopo[0x4000];

struct voce {
    const char *entry;
    void (*fn)(struct z80 *);
    uint16_t blocco_ini, blocco_fine;  /* intervallo da verificare in modo esplicito */
};

static const struct voce tabella[] = {
    { "0x05B3", sub_05b3, 0xc8ee, 0xc94d },
    { "0x2402", sub_2402, 0xe400, 0xe401 },
};

/* Cartella delle catture: SNAP_DIR o ../out, poi out. */
static FILE *apri(const char *entry, const char *suffisso, const char *modo)
{
    const char *dirs[] = { getenv("SNAP_DIR"), "out", "../out" };
    char nome[512];
    for (unsigned i = 0; i < 3; i++) {
        if (!dirs[i]) continue;
        snprintf(nome, sizeof nome, "%s/snap_%s%s", dirs[i], entry, suffisso);
        FILE *f = fopen(nome, modo);
        if (f) return f;
    }
    fprintf(stderr, "file snap_%s%s non trovato\n", entry, suffisso);
    exit(2);
}

static void carica(const char *entry, const char *suffisso, uint8_t *dest)
{
    FILE *f = apri(entry, suffisso, "rb");
    if (fread(dest, 1, 0x4000, f) != 0x4000) {
        fprintf(stderr, "snap_%s%s: lunghezza sbagliata\n", entry, suffisso);
        exit(2);
    }
    fclose(f);
}

/* Legge una riga «prima|dopo PC=.. AF=.. ...» nei registri. */
static void leggi_registri(const char *entry, const char *quale, struct z80 *z)
{
    FILE *f = apri(entry, "_regs.txt", "r");
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
            return;
        }
    }
    fprintf(stderr, "riga «%s» mancante in snap_%s_regs.txt\n", quale, entry);
    exit(2);
}

static int confronta_reg(const char *nome, unsigned c, unsigned mame)
{
    if (c == mame) return 0;
    printf("registro %s: C=%04X MAME=%04X\n", nome, c, mame);
    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "uso: %s <ENTRY>   (es. 0x05B3)\n", argv[0]);
        return 2;
    }
    const struct voce *v = NULL;
    for (unsigned i = 0; i < sizeof tabella / sizeof tabella[0]; i++)
        if (strcmp(argv[1], tabella[i].entry) == 0) v = &tabella[i];
    if (!v) {
        fprintf(stderr, "routine %s non tradotta\n", argv[1]);
        return 2;
    }

    struct z80 z, atteso;
    carica(v->entry, "_prima.bin", ram);
    carica(v->entry, "_dopo.bin", dopo);
    leggi_registri(v->entry, "prima", &z);
    leggi_registri(v->entry, "dopo", &atteso);

    v->fn(&z);

    int errori = 0, stampate = 0;
    /* c000-e5ff: escluse le porte e la ROM f000 */
    for (unsigned i = 0; i < 0x2600; i++) {
        if (ram[i] != dopo[i]) {
            if (stampate++ < 8)
                printf("%04X: prima->%02X dopo %02X\n", 0xc000 + i, ram[i], dopo[i]);
            errori++;
        }
    }
    unsigned blocco = 0;
    for (unsigned a = v->blocco_ini; a <= v->blocco_fine; a++)
        blocco += (M(a) == dopo[a - 0xc000]);
    unsigned attesi = v->blocco_fine - v->blocco_ini + 1;
    printf("byte %04X-%04X coincidenti: %u su %u\n", v->blocco_ini, v->blocco_fine,
           blocco, attesi);
    if (blocco != attesi) errori++;

    errori += confronta_reg("A", z.a, atteso.a);
    errori += confronta_reg("BC", BC(&z), BC(&atteso));
    errori += confronta_reg("DE", DE(&z), DE(&atteso));
    errori += confronta_reg("HL", HL(&z), HL(&atteso));
    errori += confronta_reg("IX", z.ix, atteso.ix);
    errori += confronta_reg("IY", z.iy, atteso.iy);
    printf("registri C: A=%02X BC=%04X DE=%04X HL=%04X IX=%04X IY=%04X\n",
           z.a, BC(&z), DE(&z), HL(&z), z.ix, z.iy);

    printf("%s %s\n", errori ? "FALLITO" : "OK", v->entry);
    return errori ? 1 : 0;
}
