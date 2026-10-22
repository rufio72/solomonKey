/* Banco del Z80 audio tradotto: ./harness/audio_banco <nome> [frame_max]
 * (da decomp/). Legge out/audio/<nome>_latch.txt e l'oracolo
 * out/audio/<nome>_ay.txt di audio.lua, esegue il C e scrive il flusso dei
 * cambi in out/audio/banco/<nome>_ay.txt nello stesso formato, sempre
 * fino all'ultimo latch o cambio dell'oracolo (frame_max limita solo i
 * confronti).
 * Modello del tempo: al frame 0 l'avvio; per ogni frame i latch del frame
 * (NMI), poi due volte IRQ (0038) e una passata di 029E.
 * Confronti: sequenza dei cambi (chip, reg, val) dei frame 0-13 (l'avvio);
 * stato dei registri a fine frame dal 13 a frame_max. L'ordine dei cambi
 * dentro un frame non si confronta: dipende da dove l'IRQ interrompe 029E.
 * Uscita 0 se tutto coincide, 1 se no, 2 per errori di file. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "audio.h"

#define FINE_AVVIO 13      /* ultimo frame dei cambi dell'avvio in MAME */
#define PRIMO_LATCH 14     /* latch prima: ignorati, l'avvio azzera la RAM */

struct cambio { int frame, chip, reg, val; };
struct latch { int frame, valore, forzato; };

struct elenco { void *v; size_t n, cap, dim; };

static void *aggiungi(struct elenco *e)
{
    if (e->n == e->cap) {
        e->cap = e->cap ? e->cap * 2 : 256;
        e->v = realloc(e->v, e->cap * e->dim);
        if (!e->v) { fprintf(stderr, "memoria esaurita\n"); exit(2); }
    }
    return (char *)e->v + e->dim * e->n++;
}

static struct elenco oracolo = { NULL, 0, 0, sizeof(struct cambio) };
static struct elenco latch = { NULL, 0, 0, sizeof(struct latch) };
static struct elenco avvio_c = { NULL, 0, 0, sizeof(struct cambio) };

/* Cambi del C nei frame dell'avvio, per il confronto della sequenza. */
static void osserva(int chip, int reg, int val)
{
    if (aud_frame > FINE_AVVIO) return;
    struct cambio *c = aggiungi(&avvio_c);
    c->frame = aud_frame; c->chip = chip; c->reg = reg; c->val = val;
}

static FILE *apri(const char *nome, const char *suffisso)
{
    char percorso[512];
    snprintf(percorso, sizeof percorso, "out/audio/%s_%s", nome, suffisso);
    FILE *f = fopen(percorso, "r");
    if (!f) {
        fprintf(stderr, "ERRORE: %s non trovato\n", percorso);
        exit(2);
    }
    return f;
}

/* Righe «frame chip reg val» (esadecimali chip, reg, val) dell'oracolo. */
static void leggi_oracolo(const char *nome)
{
    FILE *f = apri(nome, "ay.txt");
    char r[256];
    while (fgets(r, sizeof r, f)) {
        if (r[0] == '#' || r[0] == '\n') continue;
        struct cambio c;
        if (sscanf(r, "%d %x %x %x", &c.frame, &c.chip, &c.reg, &c.val) != 4
            || c.chip < 1 || c.chip > 3 || c.reg > 0xFF) {
            fprintf(stderr, "ERRORE: riga non valida nell'oracolo: %s", r);
            exit(2);
        }
        *(struct cambio *)aggiungi(&oracolo) = c;
    }
    fclose(f);
}

/* Righe «frame valore origine» del latch. */
static void leggi_latch(const char *nome)
{
    FILE *f = apri(nome, "latch.txt");
    char r[256], origine[32];
    while (fgets(r, sizeof r, f)) {
        if (r[0] == '#' || r[0] == '\n') continue;
        struct latch l;
        if (sscanf(r, "%d %x %31s", &l.frame, &l.valore, origine) != 3) {
            fprintf(stderr, "ERRORE: riga non valida nel latch: %s", r);
            exit(2);
        }
        l.forzato = strcmp(origine, "forzato") == 0;
        *(struct latch *)aggiungi(&latch) = l;
    }
    fclose(f);
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "uso (da decomp/): audio_banco <nome> [frame_max]\n");
        return 2;
    }
    const char *nome = argv[1];
    aud_carica_rom();
    aud_azzera();
    leggi_oracolo(nome);
    leggi_latch(nome);
    const struct cambio *orc = oracolo.v;
    const struct latch *lat = latch.v;

    /* fine naturale: ultimo latch o ultimo cambio dell'oracolo. Il flusso
     * si scrive sempre fino a lì (interfaccia stabile, con tutte le righe
     * «# codice»); frame_max limita solo il confronto. */
    int frame_fine = 0;
    for (size_t i = 0; i < latch.n; i++)
        if (lat[i].frame > frame_fine) frame_fine = lat[i].frame;
    for (size_t i = 0; i < oracolo.n; i++)
        if (orc[i].frame > frame_fine) frame_fine = orc[i].frame;
    int frame_max = argc == 3 ? atoi(argv[2]) : frame_fine;
    if (frame_max > frame_fine) frame_fine = frame_max;

    char percorso[512];
    mkdir("out/audio/banco", 0777);
    snprintf(percorso, sizeof percorso, "out/audio/banco/%s_ay.txt", nome);
    if (!aud_flusso_apri(percorso)) {
        fprintf(stderr, "ERRORE: non riesco a scrivere %s\n", percorso);
        return 2;
    }
    aud_osservatore = osserva;

    /* stato dell'oracolo a fine frame: -1 = mai scritto */
    static int stato[4][256];
    memset(stato, 0xFF, sizeof stato);
    size_t io = 0, il = 0;
    int primo_diverso = -1, confrontati = 0;

    aud_frame = 0;
    aud_avvio();
    for (int f = 0; f <= frame_fine; f++) {
        aud_frame = f;
        for (; il < latch.n && lat[il].frame <= f; il++) {
            if (lat[il].frame < f) continue;
            if (lat[il].forzato) aud_flusso_codice(f, (uint8_t)lat[il].valore);
            if (f < PRIMO_LATCH) {
                fprintf(stderr, "avviso: latch %02X al frame %d ignorato (avvio)\n",
                        lat[il].valore, f);
                continue;
            }
            aud_0066((uint8_t)lat[il].valore);
        }
        for (int irq = 0; irq < 2; irq++) {
            aud_0038();
            aud_029e();
        }
        for (; io < oracolo.n && orc[io].frame <= f; io++)
            stato[orc[io].chip][orc[io].reg] = orc[io].val;
        if (f < FINE_AVVIO || f > frame_max) continue;
        confrontati++;
        if (primo_diverso >= 0) continue;
        for (int chip = 1; chip <= 3 && primo_diverso < 0; chip++)
            for (int reg = 0; reg < 256; reg++)
                if (stato[chip][reg] != aud_ombra(chip, reg)) {
                    primo_diverso = f;
                    break;
                }
    }
    aud_flusso_chiudi();

    /* sequenza dell'avvio: cambi dei frame 0-13, senza la colonna frame */
    size_t n_avvio = 0;
    while (n_avvio < oracolo.n && orc[n_avvio].frame <= FINE_AVVIO) n_avvio++;
    int uguale = n_avvio == avvio_c.n;
    const struct cambio *cc = avvio_c.v;
    for (size_t i = 0; uguale && i < n_avvio; i++)
        uguale = orc[i].chip == cc[i].chip && orc[i].reg == cc[i].reg
                 && orc[i].val == cc[i].val;

    printf("%s: avvio %zu cambi, sequenza %s\n", nome, n_avvio,
           uguale ? "uguale" : "diversa");
    if (!uguale)
        fprintf(stderr, "avviso: avvio del C con %zu cambi\n", avvio_c.n);
    printf("%s: frame %d-%d confrontati %d, primo frame diverso: ", nome,
           FINE_AVVIO, frame_max, confrontati);
    if (primo_diverso < 0) printf("nessuno\n");
    else printf("%d\n", primo_diverso);
    printf("PORTE IGNOTE");
    int alcune = 0;
    for (int p = 0; p < 256; p++)
        if (aud_ignote((uint8_t)p)) {
            printf(" %02X=%u", p, aud_ignote((uint8_t)p));
            alcune = 1;
        }
    printf("%s\n", alcune ? "" : " nessuna");
    return (uguale && primo_diverso < 0) ? 0 : 1;
}
