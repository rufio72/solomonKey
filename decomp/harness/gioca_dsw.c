/* Opzioni DSW di gioca (lotto H5, brief 49): funzione pura. I bit sono quelli
 * di solomon.cpp (MAME): DSW1 0x01 demo (1 = spenta), 0x02 cabinet (2 =
 * upright), 0x0c vite; DSW2 0x03 difficoltà, 0x0c timer, 0x10 extra, 0xe0 bonus. */
#include <stdint.h>
#include <string.h>

static const struct { const char *nome; uint8_t v; } vite[] = {
    {"2", 0x0c}, {"3", 0x00}, {"4", 0x08}, {"5", 0x04}, {0, 0} };
static const struct { const char *nome; uint8_t v; } bonus[] = {
    {"30k-200k-500k", 0x00}, {"100k-300k-800k", 0x80}, {"30k-200k", 0x40},
    {"100k-300k", 0xc0}, {"30k", 0x20}, {"100k", 0xa0}, {"200k", 0x60},
    {"nessuno", 0xe0}, {0, 0} };
static const struct { const char *nome; uint8_t v; } difficolta[] = {
    {"facile", 0x02}, {"normale", 0x00}, {"dura", 0x01}, {"difficile", 0x03}, {0, 0} };
static const struct { const char *nome; uint8_t v; } timer[] = {
    {"lento", 0x08}, {"normale", 0x00}, {"veloce", 0x04}, {"velocissimo", 0x0c}, {0, 0} };
static const struct { const char *nome; uint8_t v; } extra[] = {
    {"normale", 0x00}, {"difficile", 0x10}, {0, 0} };
static const struct { const char *nome; uint8_t v; } cabinet[] = {
    {"upright", 0x02}, {"cocktail", 0x00}, {0, 0} };
static const struct { const char *nome; uint8_t v; } demo[] = {
    {"on", 0x00}, {"off", 0x01}, {0, 0} };

/* Applica `opzione` (senza i trattini: "vite", "cabinet"...) con `valore` a
 * *d1 e *d2: azzera i bit dell'opzione e imposta quelli del valore.
 * 0 = fatto, -1 = opzione sconosciuta, -2 = valore non valido. */
int dsw_applica(const char *opzione, const char *valore, uint8_t *d1, uint8_t *d2)
{
#define CERCA(t, reg, maschera) do { \
        for (int i = 0; (t)[i].nome; i++) \
            if (!strcmp((t)[i].nome, valore)) { \
                *(reg) = (uint8_t)((*(reg) & ~(maschera)) | (t)[i].v); \
                return 0; \
            } \
        return -2; \
    } while (0)
    if (!strcmp(opzione, "vite")) CERCA(vite, d1, 0x0c);
    if (!strcmp(opzione, "cabinet")) CERCA(cabinet, d1, 0x02);
    if (!strcmp(opzione, "demo")) CERCA(demo, d1, 0x01);
    if (!strcmp(opzione, "bonus")) CERCA(bonus, d2, 0xe0);
    if (!strcmp(opzione, "difficolta")) CERCA(difficolta, d2, 0x03);
    if (!strcmp(opzione, "timer")) CERCA(timer, d2, 0x0c);
    if (!strcmp(opzione, "extra")) CERCA(extra, d2, 0x10);
#undef CERCA
    return -1;
}
