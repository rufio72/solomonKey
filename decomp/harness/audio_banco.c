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
 * Il registro 6 (periodo del rumore) non si confronta quando il rumore è
 * spento su tutti i canali del chip sia in MAME sia nel C: inudibile.
 * Foto: se esiste out/audio/<nome>_foto.txt (audio.sh foto), a fine di
 * ognuno dei suoi frame confronta la RAM 4000-4243 del C con la foto.
 * Comando 40: la sua voce punta fuori dalla ROM (7A09) e in MAME tiene
 * l'IRQ occupato ~86 frame con IRQ annidati, un effetto di tempismo a
 * livello di cicli che questo modello a fine frame non simula; il gioco non
 * lo invia mai. Dal frame del primo latch 40 il confronto si ferma.
 * AUDIO_DATI=estratti: la regione 05C6-3FFF viene dalle tabelle estratte
 * (audio_dati_*.c); AUDIO_DATI=vuoto: regione tutta 0xAA (controllo
 * negativo). La conferma va su stderr, stdout non cambia.
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

#define FOTO_LUNG 0x244    /* 4000-4243: blocchi e globali */
struct foto { int frame; uint8_t ram[FOTO_LUNG]; };
static struct elenco foto = { NULL, 0, 0, sizeof(struct foto) };

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

/* Righe «frame 4000 byte» della foto della RAM; assente = nessuna foto. */
static void leggi_foto(const char *nome)
{
    char percorso[512];
    snprintf(percorso, sizeof percorso, "out/audio/%s_foto.txt", nome);
    FILE *f = fopen(percorso, "r");
    if (!f) return;
    static char r[4096];
    while (fgets(r, sizeof r, f)) {
        if (r[0] == '#' || r[0] == '\n') continue;
        struct foto *ft = aggiungi(&foto);
        unsigned inizio;
        int n = 0;
        if (sscanf(r, "%d %x %n", &ft->frame, &inizio, &n) != 2 || inizio != AUD_RAM
            || strlen(r + n) < 2 * FOTO_LUNG) {
            fprintf(stderr, "ERRORE: riga non valida nella foto: %.40s\n", r);
            exit(2);
        }
        for (int i = 0; i < FOTO_LUNG; i++) {
            unsigned b;
            if (sscanf(r + n + 2 * i, "%2x", &b) != 1) {
                fprintf(stderr, "ERRORE: byte non valido nella foto del frame %d\n", ft->frame);
                exit(2);
            }
            ft->ram[i] = (uint8_t)b;
        }
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
    const char *dati = getenv("AUDIO_DATI");   /* origine della regione 05C6-3FFF */
    if (dati && strcmp(dati, "estratti") == 0) {
        aud_usa_dati_estratti(1);
        fprintf(stderr, "dati: estratti\n");
    } else if (dati && strcmp(dati, "vuoto") == 0) {
        aud_usa_dati_estratti(2);
        fprintf(stderr, "dati: vuoto\n");
    }
    aud_azzera();
    leggi_oracolo(nome);
    leggi_latch(nome);
    leggi_foto(nome);
    const struct cambio *orc = oracolo.v;
    const struct latch *lat = latch.v;
    const struct foto *ft = foto.v;

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
    int frame_40 = -1;                           /* primo latch 40: fuori modello */
    for (size_t i = 0; i < latch.n && frame_40 < 0; i++)
        if (lat[i].valore == 0x40 && lat[i].frame >= PRIMO_LATCH) frame_40 = lat[i].frame;
    if (frame_40 >= 0 && frame_max >= frame_40) frame_max = frame_40 - 1;

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
    size_t ifoto = 0;
    int foto_confrontate = 0, foto_diversa = -1, foto_indirizzo = 0;

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
        for (; ifoto < foto.n && ft[ifoto].frame <= f; ifoto++) {
            if (ft[ifoto].frame < f || f > frame_max) continue;
            foto_confrontate++;
            if (foto_diversa >= 0) continue;
            for (int i = 0; i < FOTO_LUNG; i++)
                if (aud_ram[i] != ft[ifoto].ram[i]) {
                    foto_diversa = f;
                    foto_indirizzo = AUD_RAM + i;
                    break;
                }
        }
        for (; io < oracolo.n && orc[io].frame <= f; io++)
            stato[orc[io].chip][orc[io].reg] = orc[io].val;
        if (f < FINE_AVVIO || f > frame_max) continue;
        confrontati++;
        if (primo_diverso >= 0) continue;
        for (int chip = 1; chip <= 3 && primo_diverso < 0; chip++)
            for (int reg = 0; reg < 256; reg++) {
                /* registro 6 inudibile: rumore spento su tutti i canali
                 * del chip (bit 3-5 del registro 7) in MAME e nel C */
                if (reg == 6 && (stato[chip][7] & 0x38) == 0x38
                    && (aud_ombra(chip, 7) & 0x38) == 0x38) continue;
                if (stato[chip][reg] != aud_ombra(chip, reg)) {
                    primo_diverso = f;
                    break;
                }
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
    if (frame_40 >= 0)
        printf("%s: comando 40 al frame %d, confronto fermato (fuori modello)\n",
               nome, frame_40);
    if (foto.n) {
        printf("%s: foto RAM %d frame, primo frame diverso: ", nome, foto_confrontate);
        if (foto_diversa < 0) printf("nessuno\n");
        else printf("%d (%04X)\n", foto_diversa, foto_indirizzo);
    }
    printf("PORTE IGNOTE");
    int alcune = 0;
    for (int p = 0; p < 256; p++)
        if (aud_ignote((uint8_t)p)) {
            printf(" %02X=%u", p, aud_ignote((uint8_t)p));
            alcune = 1;
        }
    printf("%s\n", alcune ? "" : " nessuna");
    return (uguale && primo_diverso < 0 && foto_diversa < 0) ? 0 : 1;
}
