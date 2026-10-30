/* Motore dei nemici, lotto N3 (brief 29): il registro R del Z80.
 *
 * 54C5 e 59E5 leggono R con due `ld a,r` ravvicinati per avere un numero
 * a caso: R1, poi R2 = R1 + 3 sui 7 bit bassi (due istruzioni con prefisso
 * ED e un `ld` in mezzo), e usano solo la somma R1 + R2. Le catture di
 * MAME non registrano R, quindi il valore non si può ricavare dal «prima».
 * nemici_r_somma() lo prende da una piccola tabella (r_valori) che il
 * banco riempie con la ricerca qui sotto; fuori dal banco (replay) vale 0.
 *
 * Ricerca (solo banco, nelle voci che arrivano a un `ld a,r`): il
 * wrapper riconosce la cattura confrontando RAM e registri con i file
 * snap_<ENTRY>_*_prima.bin/_regs.txt, carica il «dopo» e prova i 128
 * valori di R1 (e, per le routine con due letture, le 128 coppie) fino a
 * trovare quello per cui RAM e registri coincidono con MAME. Tutto il
 * resto della routine resta verificato; di R si verifica solo che esista
 * un valore compatibile. Se la cattura non si riconosce o nessun valore
 * va bene, resta il risultato con R = 0 e il banco segnala la differenza. */
#define _POSIX_C_SOURCE 200809L
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "routine.h"
#include "nemici.h"

static uint8_t r_valori[2];
static unsigned r_usati;

/* Come nemici_r_somma, ma scrive anche R1 (5DDF lascia C = R1). */
uint8_t nemici_r_somma_r1(uint8_t *r1)
{
    uint8_t v = r_valori[r_usati < 2 ? r_usati : 1];
    r_usati++;
    uint8_t r2 = (uint8_t)((v & 0x80) | ((v + 3) & 0x7f));
    *r1 = v;
    return (uint8_t)(v + r2);
}

uint8_t nemici_r_somma(void)
{
    uint8_t r1;
    return nemici_r_somma_r1(&r1);
}

/* Stato «prima» o «dopo» di una cattura. */
struct stato_cattura {
    struct z80 z;
    uint8_t ram[0x4000];
};

static int carica_ram(const char *prefisso, const char *suffisso, uint8_t *dest)
{
    char nome[640];
    snprintf(nome, sizeof nome, "%s%s", prefisso, suffisso);
    FILE *f = fopen(nome, "rb");
    if (!f) return 0;
    size_t n = fread(dest, 1, 0x4000, f);
    fclose(f);
    return n == 0x4000;
}

static int carica_reg(const char *prefisso, const char *quale, struct z80 *z)
{
    char nome[640], fmt[128], riga[256];
    snprintf(nome, sizeof nome, "%s_regs.txt", prefisso);
    FILE *f = fopen(nome, "r");
    if (!f) return 0;
    snprintf(fmt, sizeof fmt,
             "%s PC=%%x AF=%%x BC=%%x DE=%%x HL=%%x IX=%%x IY=%%x SP=%%x", quale);
    unsigned pc, af, bc, de, hl, ix, iy, sp;
    int ok = 0;
    while (!ok && fgets(riga, sizeof riga, f))
        if (sscanf(riga, fmt, &pc, &af, &bc, &de, &hl, &ix, &iy, &sp) == 8) {
            z->a = af >> 8; z->f = af & 0xff;
            SET_BC(z, bc); SET_DE(z, de); SET_HL(z, hl);
            z->ix = ix; z->iy = iy; z->sp = sp;
            ok = 1;
        }
    fclose(f);
    return ok;
}

/* Come confronta il banco: RAM c000-e5ff salvo la pila libera, e A BC DE
 * HL IX IY. */
static int coincide(const struct z80 *z, const struct z80 *atteso,
                    const uint8_t *dopo, uint16_t sp)
{
    if (z->a != atteso->a || BC(z) != BC(atteso) || DE(z) != DE(atteso) ||
        HL(z) != HL(atteso) || z->ix != atteso->ix || z->iy != atteso->iy)
        return 0;
    unsigned pila_ini = (unsigned)sp - 0x40, pila_fine = sp;
    for (unsigned i = 0; i < 0x2600; i++) {
        if (0xc000 + i >= pila_ini && 0xc000 + i < pila_fine) continue;
        if (ram[i] != dopo[i]) return 0;
    }
    return 1;
}

/* Cerca la cattura che ha dato lo stato di ingresso (ram, z) fra i file
 * della voce; riempie prefisso e restituisce 1 se la trova. */
static int trova_cattura(const char *entry, const struct stato_cattura *in,
                         char *prefisso, size_t cap)
{
    const char *dirs[] = { getenv("SNAP_DIR"), "out", "../out" };
    static struct stato_cattura prima;
    for (unsigned i = 0; i < 3; i++) {
        if (!dirs[i]) continue;
        char modello[640];
        snprintf(modello, sizeof modello, "%s/snap_%s_*_prima.bin", dirs[i], entry);
        glob_t g = { 0 };
        if (glob(modello, 0, NULL, &g) != 0) continue;
        for (size_t k = 0; k < g.gl_pathc; k++) {
            snprintf(prefisso, cap, "%s", g.gl_pathv[k]);
            prefisso[strlen(prefisso) - strlen("_prima.bin")] = '\0';
            if (carica_ram(prefisso, "_prima.bin", prima.ram) &&
                carica_reg(prefisso, "prima", &prima.z) &&
                memcmp(prima.ram, in->ram, sizeof prima.ram) == 0 &&
                prima.z.a == in->z.a && BC(&prima.z) == BC(&in->z) &&
                DE(&prima.z) == DE(&in->z) && HL(&prima.z) == HL(&in->z) &&
                prima.z.ix == in->z.ix && prima.z.iy == in->z.iy) {
                globfree(&g);
                return 1;
            }
        }
        globfree(&g);
    }
    return 0;
}

static void prova_con_r(struct z80 *z, const char *entry, void (*fn)(struct z80 *))
{
    static struct stato_cattura in, atteso;
    static uint8_t dopo[0x4000];
    char prefisso[640];

    in.z = *z;
    memcpy(in.ram, ram, sizeof in.ram);
    r_valori[0] = r_valori[1] = 0;
    if (trova_cattura(entry, &in, prefisso, sizeof prefisso) &&
        carica_ram(prefisso, "_dopo.bin", dopo) &&
        carica_reg(prefisso, "dopo", &atteso.z)) {
        for (unsigned v1 = 0; v1 < 0x80; v1++) {
            for (unsigned v2 = 0; v2 < 0x80; v2++) {
                memcpy(ram, in.ram, sizeof in.ram);
                *z = in.z;
                r_valori[0] = (uint8_t)v1;
                r_valori[1] = (uint8_t)v2;
                r_usati = 0;
                fn(z);
                if (coincide(z, &atteso.z, dopo, in.z.sp))
                    return;
                if (r_usati < 2)               /* il secondo valore non conta */
                    break;
            }
            if (r_usati == 0)                  /* nessuna lettura di R */
                break;
        }
    }
    memcpy(ram, in.ram, sizeof in.ram);        /* nessuna coincidenza: R = 0 */
    *z = in.z;
    r_valori[0] = r_valori[1] = 0;
    r_usati = 0;
    fn(z);
}

void banco_r_54c5(struct z80 *z) { prova_con_r(z, "0x54C5", sub_54c5); }
void banco_r_51d7(struct z80 *z) { prova_con_r(z, "0x51D7", sub_51d7); }
void banco_r_5995(struct z80 *z) { prova_con_r(z, "0x5995", sub_5995); }
void banco_r_57cd(struct z80 *z) { prova_con_r(z, "0x57CD", sub_57cd); }
void banco_r_51fa(struct z80 *z) { prova_con_r(z, "0x51FA", sub_51fa); }
void banco_r_5ddf(struct z80 *z) { prova_con_r(z, "0x5DDF", sub_5ddf); }
void banco_r_5e1a(struct z80 *z) { prova_con_r(z, "0x5E1A", sub_5e1a); }
