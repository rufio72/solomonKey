/* Prova dello spedidore a funzioni contro il registro di MAME.
 * Uso: ./harness/sched_proto out/sched_log.txt [frame_max]
 * Il registro (sched_log.lua) ha righe D (spedizione), W (scrittura nella
 * tabella) e I (scrittura di c04e/c04f fuori da 0493 e 3AF3). Le W e I prima
 * della prima D si applicano subito; poi a ogni spedizione lo stub chiamato
 * applica le righe che seguono la D corrente fino alla D successiva: le W con
 * pc 04AB passano per posta_attivita e se ne confronta lo slot, le altre
 * scrivono direttamente. Le W con campo letture m > 0 sono della NMI entrata
 * a metà della scansione successiva: si applicano dal gancio di scansione
 * prima della lettura m (0-based), cioè dopo m letture come in MAME.
 * Si confrontano slot e codice di ogni D. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sched.h"

uint8_t ram[0x4000];

struct evento {
    char tipo;             /* 'D', 'W', 'I' */
    long frame;
    uint16_t pc;           /* W, I */
    uint16_t indirizzo;    /* D, W: slot; I: indirizzo */
    uint8_t valore;        /* D: codice; W, I: valore scritto */
    int letture;           /* W: letture della scansione prima della scrittura */
};

static struct evento *eventi;
static size_t n_eventi;
static size_t cursore;     /* evento D in corso */
static size_t pendente;    /* prossimo evento da applicare dalla scansione */
static long posta_totale, posta_coincidenti;
static int posta_errata_stampata;

static void carica(const char *nome)
{
    FILE *f = fopen(nome, "r");
    if (!f) { perror(nome); exit(2); }
    size_t cap = 1 << 18;
    eventi = malloc(cap * sizeof *eventi);
    char riga[128];
    while (fgets(riga, sizeof riga, f)) {
        struct evento e = {0};
        unsigned a, b, c;
        if (riga[0] == 'D' && sscanf(riga + 1, "%ld %x %x", &e.frame, &a, &b) == 3) {
            e.indirizzo = a; e.valore = b;
        } else if ((riga[0] == 'W' || riga[0] == 'I')
                   && sscanf(riga + 1, "%ld %x %x %x %d", &e.frame, &a, &b, &c, &e.letture) >= 4) {
            e.pc = a; e.indirizzo = b; e.valore = c;
        } else {
            continue;
        }
        e.tipo = riga[0];
        if (n_eventi == cap) {
            cap *= 2;
            eventi = realloc(eventi, cap * sizeof *eventi);
        }
        eventi[n_eventi++] = e;
    }
    fclose(f);
}

/* Applica un evento W o I alla RAM. */
static void applica(const struct evento *e)
{
    if (e->tipo == 'I') {
        M(e->indirizzo) = e->valore;
    } else if (e->pc == 0x04AB) {
        uint8_t s = posta_attivita(e->valore);
        posta_totale++;
        if (s == e->indirizzo) {
            posta_coincidenti++;
        } else if (!posta_errata_stampata) {
            posta_errata_stampata = 1;
            printf("posta_attivita diversa al frame %ld: codice %02X, slot atteso %02X, ottenuto %02X\n",
                   e->frame, e->valore, e->indirizzo, s);
        }
    } else {
        tabella[e->indirizzo & 0x3f] = e->valore;
    }
}

/* Stub di tutte le attività: replica le scritture misurate durante il corpo
 * (letture = 0), fino alla D successiva o alla prima scrittura della NMI a
 * metà scansione, che resta pendente per il gancio di scansione. */
static void gancio(uint8_t codice, uint8_t slot)
{
    (void)codice; (void)slot;
    size_t i = cursore + 1;
    for (; i < n_eventi && eventi[i].tipo != 'D' && eventi[i].letture == 0; i++)
        applica(&eventi[i]);
    pendente = i;
}

/* Effetti della NMI a metà scansione: prima della lettura numero `lettura`. */
static void gancio_scansione(int lettura)
{
    while (pendente < n_eventi && eventi[pendente].tipo != 'D'
           && eventi[pendente].letture <= lettura)
        applica(&eventi[pendente++]);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "uso: %s out/sched_log.txt [frame_max]\n", argv[0]);
        return 2;
    }
    long frame_max = argc > 2 ? atol(argv[2]) : -1;
    carica(argv[1]);
    memset(ram, 0, sizeof ram);
    sched_gancio_stub = gancio;
    sched_gancio_scansione = gancio_scansione;

    size_t i = 0;
    while (i < n_eventi && eventi[i].tipo != 'D' && eventi[i].letture == 0)
        applica(&eventi[i++]);
    pendente = i;

    long confrontate = 0, differenze = 0;
    for (; i < n_eventi; i++) {
        const struct evento *e = &eventi[i];
        if (e->tipo != 'D')
            continue;
        if (frame_max >= 0 && e->frame > frame_max)
            break;
        cursore = i;
        uint8_t slot = 0xff, codice = 0;
        int r = spedisci_una(&slot, &codice);
        confrontate++;
        if (r != 1 || slot != e->indirizzo || codice != e->valore) {
            differenze++;
            printf("differenza al frame %ld, spedizione %ld: atteso slot %02X codice %02X, "
                   "ottenuto slot %02X codice %02X (esito %d)\n",
                   e->frame, confrontate, e->indirizzo, e->valore, slot, codice, r);
            break;
        }
    }
    printf("spedizioni confrontate: %ld, differenze: %ld; posta_attivita: %ld, slot coincidenti: %ld\n",
           confrontate, differenze, posta_totale, posta_coincidenti);
    return (differenze == 0 && posta_totale == posta_coincidenti) ? 0 : 1;
}
