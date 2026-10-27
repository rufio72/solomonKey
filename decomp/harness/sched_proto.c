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

/* Una funzione per codice di attività, con l'ingresso della tabella 3B11
 * nel nome. I corpi tradotti stanno in attivita.c (replay); qui restano
 * stub che chiamano il gancio della prova. */
#define STUB(cod, ingresso) \
    static void attivita_##cod##_##ingresso(uint8_t slot) \
    { if (sched_gancio_stub) sched_gancio_stub(0x##cod, slot); }

STUB(00, 3b61) STUB(01, 48d1) STUB(02, 4b84) STUB(03, 4bc6) STUB(04, 4c30)
STUB(05, 4c42) STUB(06, 75e7) STUB(07, 2ccd) STUB(08, 7c98) STUB(09, 4d29)
STUB(0a, 4da2) STUB(0b, 2c27) STUB(0c, 7bf6) STUB(0d, 5c8e) STUB(0e, 63a4)
STUB(0f, 640e) STUB(10, 457b) STUB(11, 456e) STUB(12, 426a) STUB(13, 6480)
STUB(14, 44b5) STUB(15, 6466) STUB(16, 6496) STUB(17, 6429) STUB(18, 4124)
STUB(19, 3792) STUB(1a, 411d) STUB(1b, 4111) STUB(1c, 3f40) STUB(1d, 3f39)
STUB(1e, 3b61) STUB(1f, 43c8) STUB(20, 3e90) STUB(21, 3b61) STUB(22, 12d6)
STUB(23, 3e4e) STUB(24, 2c72) STUB(25, 3b77) STUB(26, 3b63) STUB(27, 62b6)

attivita_fn corpo_attivita[SCHED_CODICI] = {
    attivita_00_3b61, attivita_01_48d1, attivita_02_4b84, attivita_03_4bc6,
    attivita_04_4c30, attivita_05_4c42, attivita_06_75e7, attivita_07_2ccd,
    attivita_08_7c98, attivita_09_4d29, attivita_0a_4da2, attivita_0b_2c27,
    attivita_0c_7bf6, attivita_0d_5c8e, attivita_0e_63a4, attivita_0f_640e,
    attivita_10_457b, attivita_11_456e, attivita_12_426a, attivita_13_6480,
    attivita_14_44b5, attivita_15_6466, attivita_16_6496, attivita_17_6429,
    attivita_18_4124, attivita_19_3792, attivita_1a_411d, attivita_1b_4111,
    attivita_1c_3f40, attivita_1d_3f39, attivita_1e_3b61, attivita_1f_43c8,
    attivita_20_3e90, attivita_21_3b61, attivita_22_12d6, attivita_23_3e4e,
    attivita_24_2c72, attivita_25_3b77, attivita_26_3b63, attivita_27_62b6,
};

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
