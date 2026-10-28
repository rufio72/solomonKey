/* Giro di gioco in C contro l'oracolo di MAME (brief 23).
 * Uso: ./replay <oracolo.txt> <disp.txt> <dump_F0.bin> <F0> [frame_fine] [--oltre]
 *
 * Si parte dal dump c000-ffff scaricato da replay.lua all'ingresso della
 * NMI del frame F0 e si fa girare il modello: spedidore (sched.c), corpi
 * delle attività (attivita.c) e NMI (sub_0066) con gli ingressi e600-e602
 * dell'oracolo. La NMI si inietta nel punto misurato da replay.lua (riga
 * «N <f> <ritorno> <letture>» di disp.txt):
 *   - ritorno in 3AFB-3B10 con letture 0: la NMI è entrata dopo la lettura
 *     che ha trovato l'attività e prima del corpo: si esegue prima del corpo,
 *     con c04f ancora al valore vecchio se il ritorno precede 3B0F;
 *   - altrimenti si esegue nella scansione successiva prima della lettura
 *     numero <letture> (0 se la NMI è entrata a metà di un corpo, che nel C
 *     non si può spezzare: lì la divergenza è attesa e si classifica).
 * A ogni NMI, prima di eseguirla, l'impronta della RAM del C si confronta
 * con la riga F dell'oracolo. Le divergenze ai frame con la NMI entrata
 * nello spedidore («pulite») fermano il replay; quelle con la NMI a metà
 * attività si contano senza fermarsi. Con --oltre nemmeno le pulite
 * fermano: a fine giro si dice se il C riconverge. Ogni spedizione si
 * confronta con la riga D attesa (slot e codice). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "rst20_tables.h"

attivita_fn corpo_attivita[SCHED_CODICI];
extern attivita_fn corpo_attivita_replay[SCHED_CODICI];

/* ---- impronta: FNV-1a 32 bit di c000-e5ff esclusi c040-c043 e cf00-cfff
 * (stesse zone di replay.lua) */
static int escluso(unsigned a)
{
    return (a >= 0xc040 && a <= 0xc043) || (a >= 0xcf00 && a <= 0xcfff);
}

static uint32_t impronta(const uint8_t *m)
{
    uint32_t h = 2166136261u;
    for (unsigned a = 0xc000; a < 0xe600; a++) {
        if (escluso(a))
            continue;
        h = (h ^ m[a - 0xc000]) * 16777619u;
    }
    return h;
}

/* ---- oracolo e registro */
struct riga_f { long frame; uint8_t p[3]; uint16_t ritorno; uint32_t hash; };
struct evento { char tipo; long frame; uint8_t slot, codice; uint16_t ritorno; int letture; };

static struct riga_f *ff;
static size_t n_ff;
static struct evento *ev;
static size_t n_ev;

static void *cresci(void *p, size_t *cap, size_t n, size_t dim)
{
    if (n < *cap)
        return p;
    *cap = *cap ? *cap * 2 : 4096;
    p = realloc(p, *cap * dim);
    if (!p) { perror("realloc"); exit(2); }
    return p;
}

static void carica_oracolo(const char *nome)
{
    FILE *f = fopen(nome, "r");
    if (!f) { perror(nome); exit(2); }
    char riga[160];
    size_t cap = 0;
    while (fgets(riga, sizeof riga, f)) {
        struct riga_f r;
        unsigned a, b, c, rit, h;
        if (sscanf(riga, "F %ld %x %x %x %x %x", &r.frame, &a, &b, &c, &rit, &h) != 6)
            continue;
        r.p[0] = a; r.p[1] = b; r.p[2] = c; r.ritorno = rit; r.hash = h;
        ff = cresci(ff, &cap, n_ff, sizeof *ff);
        ff[n_ff++] = r;
    }
    fclose(f);
}

static void carica_registro(const char *nome)
{
    FILE *f = fopen(nome, "r");
    if (!f) { perror(nome); exit(2); }
    char riga[160];
    size_t cap = 0;
    while (fgets(riga, sizeof riga, f)) {
        struct evento e = {0};
        unsigned a, b;
        if (riga[0] == 'D' && sscanf(riga + 1, "%ld %x %x", &e.frame, &a, &b) == 3) {
            e.slot = a; e.codice = b;
        } else if (riga[0] == 'N' && sscanf(riga + 1, "%ld %x %d", &e.frame, &a, &e.letture) == 3) {
            e.ritorno = a;
        } else {
            continue;                  /* W e I: le scritture le rifà il C */
        }
        e.tipo = riga[0];
        ev = cresci(ev, &cap, n_ev, sizeof *ev);
        ev[n_ev++] = e;
    }
    fclose(f);
}

static int pendente(const struct evento *n)
{
    return n->ritorno >= 0x3afb && n->ritorno <= 0x3b10 && n->letture == 0;
}

static int nello_spedidore(uint16_t r)
{
    return r >= 0x3af3 && r <= 0x3b10;
}

/* ---- stato del giro */
static char prefisso[512];     /* dump: <prefisso>_<f>.bin */
static int oltre;
static size_t k_ff;            /* prossima riga F */
static long frame_fine;
static const struct evento *ultima_d;
static long nmi_totali, nmi_pulite, div_pulite, div_meta;
static long prima_div_pulita = -1, prima_div_meta = -1, ultima_div_pulita = -1;
static long ultima_meta_da_conv = -1;  /* ultima NMI a metà attività dopo l'ultima coincidenza pulita */
static int ultima_pulita_coincide = 1;

static void riepilogo(void)
{
    printf("NMI confrontate: %ld (nello spedidore %ld, a metà attività %ld); "
           "divergenti: nello spedidore %ld, a metà attività %ld\n",
           nmi_totali, nmi_pulite, nmi_totali - nmi_pulite, div_pulite, div_meta);
    if (prima_div_meta >= 0)
        printf("prima divergenza a metà attività (attesa): frame %ld\n", prima_div_meta);
    if (prima_div_pulita >= 0)
        printf("prima divergenza nello spedidore: frame %ld; ultima: frame %ld; "
               "ultima NMI pulita %s\n", prima_div_pulita, ultima_div_pulita,
               ultima_pulita_coincide ? "coincidente (riconverge)" : "divergente");
    else
        printf("nessuna divergenza nello spedidore\n");
}

static void ferma(int codice)
{
    riepilogo();
    exit(codice);
}

/* Primi 32 byte diversi dal dump di MAME del frame, se c'è. */
static void stampa_diff(long frame)
{
    char nome[600];
    snprintf(nome, sizeof nome, "%s_%ld.bin", prefisso, frame);
    FILE *f = fopen(nome, "rb");
    if (!f) {
        printf("dump %s assente (REPLAY_DUMPS=%ld per averlo)\n", nome, frame);
        return;
    }
    static uint8_t mame[0x4000];
    size_t n = fread(mame, 1, sizeof mame, f);
    fclose(f);
    if (n != sizeof mame) { printf("%s: lunghezza sbagliata\n", nome); return; }
    int stampati = 0, totali = 0;
    for (unsigned a = 0xc000; a < 0xe600; a++) {
        if (escluso(a) || ram[a - 0xc000] == mame[a - 0xc000])
            continue;
        totali++;
        if (stampati++ < 32)
            printf("  %04X: C %02X MAME %02X\n", a, ram[a - 0xc000], mame[a - 0xc000]);
    }
    printf("  byte diversi in c000-e5ff (escluse c040-c043, cf00-cfff): %d\n", totali);
}

/* Confronto d'impronta e NMI del prossimo frame dell'oracolo. */
static void esegui_nmi(const struct evento *n)
{
    if (k_ff >= n_ff || ff[k_ff].frame != n->frame) {
        printf("oracolo e registro disallineati alla NMI del frame %ld\n", n->frame);
        ferma(2);
    }
    const struct riga_f *r = &ff[k_ff++];
    uint32_t h = impronta(ram);
    int pulita = nello_spedidore(r->ritorno);
    nmi_totali++;
    nmi_pulite += pulita;
    if (h != r->hash) {
        if (pulita) {
            div_pulite++;
            ultima_div_pulita = r->frame;
            ultima_pulita_coincide = 0;
            if (prima_div_pulita < 0 || !oltre) {
                if (prima_div_pulita < 0)
                    prima_div_pulita = r->frame;
                printf("DIVERGENZA frame %ld: hash C %08X MAME %08X; ritorno NMI oracolo %04X "
                       "(spedidore); ultima D %02X %02X",
                       r->frame, h, r->hash, r->ritorno,
                       ultima_d ? ultima_d->slot : 0xff, ultima_d ? ultima_d->codice : 0xff);
                if (ultima_meta_da_conv >= 0)
                    printf("; ultima NMI a metà attività dopo l'ultima coincidenza: frame %ld",
                           ultima_meta_da_conv);
                printf("\n");
                stampa_diff(r->frame);
                if (!oltre)
                    ferma(1);
            }
        } else {
            div_meta++;
            if (prima_div_meta < 0)
                prima_div_meta = r->frame;
            ultima_meta_da_conv = r->frame;
        }
    } else if (pulita) {
        ultima_pulita_coincide = 1;
        ultima_meta_da_conv = -1;
    }
    M(0xe600) = r->p[0];
    M(0xe601) = r->p[1];
    M(0xe602) = r->p[2];
    struct z80 z = {0};
    sub_0066(&z);
}

/* NMI da eseguire: prima del corpo (pendente) e nella scansione. */
static const struct evento *nmi_corpo;
static const struct evento *nmi_scan[8];
static int n_scan, i_scan;

static void gancio_scansione(int lettura)
{
    while (i_scan < n_scan && nmi_scan[i_scan]->letture <= lettura)
        esegui_nmi(nmi_scan[i_scan++]);
}

static int corpo_mancante = -1;

/* c04f prima della spedizione in corso: lo spedidore lo scrive a 3B0C, e
 * una NMI pendente entrata prima (ritorno < 3B0F) deve vedere il valore
 * vecchio, mentre nel C spedisci_una lo ha già scritto. */
static uint8_t indice_prima;

/* Involucro di tutti i corpi: la NMI pendente, poi il corpo tradotto. */
static void involucro(uint8_t slot)
{
    uint8_t codice = tabella[slot];   /* letto prima della NMI, come B a 3B02 */
    if (nmi_corpo) {
        const struct evento *n = nmi_corpo;
        nmi_corpo = NULL;
        if (n->ritorno < 0x3b0f) {
            uint8_t nuovo = indice;
            indice = indice_prima;
            esegui_nmi(n);
            indice = nuovo;
        } else {
            esegui_nmi(n);
        }
    }
    if (codice >= SCHED_CODICI || !corpo_attivita_replay[codice]) {
        corpo_mancante = codice;
        return;
    }
    corpo_attivita_replay[codice](slot);
}

int main(int argc, char **argv)
{
    const char *pos[5];
    int n_pos = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--oltre") == 0) oltre = 1;
        else if (n_pos < 5) pos[n_pos++] = argv[i];
    }
    if (n_pos < 4) {
        fprintf(stderr, "uso: %s <oracolo.txt> <disp.txt> <dump_F0.bin> <F0> [frame_fine] [--oltre]\n",
                argv[0]);
        return 2;
    }
    long f0 = atol(pos[3]);
    frame_fine = n_pos > 4 ? atol(pos[4]) : -1;
    carica_oracolo(pos[0]);
    carica_registro(pos[1]);
    snprintf(prefisso, sizeof prefisso, "%s", pos[2]);
    char suff[32];
    snprintf(suff, sizeof suff, "_%ld.bin", f0);
    size_t lp = strlen(prefisso), ls = strlen(suff);
    if (lp > ls && strcmp(prefisso + lp - ls, suff) == 0)
        prefisso[lp - ls] = 0;

    carica_rom();
    FILE *fd = fopen(pos[2], "rb");
    if (!fd || fread(ram, 1, sizeof ram, fd) != sizeof ram) {
        fprintf(stderr, "%s: dump mancante o corto\n", pos[2]);
        return 2;
    }
    fclose(fd);
    for (int i = 0; i < SCHED_CODICI; i++)
        corpo_attivita[i] = involucro;
    sched_gancio_scansione = gancio_scansione;

    /* sanità a F0 */
    while (k_ff < n_ff && ff[k_ff].frame != f0) k_ff++;
    if (k_ff == n_ff) { printf("frame %ld assente dall'oracolo\n", f0); return 2; }
    uint32_t h0 = impronta(ram);
    printf("SANITA F0 %ld: hash C %08X MAME %08X %s\n", f0, h0, ff[k_ff].hash,
           h0 == ff[k_ff].hash ? "coincide" : "DIVERSO");
    if (h0 != ff[k_ff].hash) {
        printf("impronte diverse a F0: esclusioni da tarare\n");
        return 1;
    }
    if (frame_fine < 0) frame_fine = ff[n_ff - 1].frame;
    if (!nello_spedidore(ff[k_ff].ritorno)) {
        size_t k = k_ff;
        while (k < n_ff && !nello_spedidore(ff[k].ritorno)) k++;
        printf("attenzione: a F0 la NMI è entrata a metà attività (ritorno %04X): il resto "
               "di quel corpo non c'è nel C; primo frame utile con la NMI nello spedidore: %ld\n",
               ff[k_ff].ritorno, k < n_ff ? ff[k].frame : -1L);
    }

    /* punto di partenza nel registro: la N di F0, o la D che la precede se
     * la NMI è entrata fra la lettura e il corpo */
    size_t i0 = 0;
    while (i0 < n_ev && !(ev[i0].tipo == 'N' && ev[i0].frame == f0)) i0++;
    if (i0 == n_ev) { printf("NMI del frame %ld assente dal registro\n", f0); return 2; }
    if (pendente(&ev[i0])) {
        if (i0 == 0 || ev[i0 - 1].tipo != 'D') { printf("D pendente a F0 mancante\n"); return 2; }
        i0--;
        if (ev[i0 + 1].ritorno >= 0x3b0f)      /* c04f già scritto a 3B0C */
            indice = ev[i0].slot;
    }
    /* fine: dopo l'ultima N entro frame_fine */
    size_t i1 = i0;
    for (size_t i = i0; i < n_ev; i++)
        if (ev[i].tipo == 'N' && ev[i].frame <= frame_fine) i1 = i + 1;

    long spedizioni = 0;
    for (size_t i = i0; i < i1; i++) {
        const struct evento *e = &ev[i];
        if (e->tipo == 'N') {
            if (n_scan == (int)(sizeof nmi_scan / sizeof nmi_scan[0])) {
                printf("troppe NMI fra due spedizioni al frame %ld\n", e->frame);
                ferma(2);
            }
            nmi_scan[n_scan++] = e;
            continue;
        }
        /* D: corpo tradotto? */
        if (e->codice >= SCHED_CODICI || !corpo_attivita_replay[e->codice]) {
            printf("CORPO %02X (%04X) non tradotto al frame %ld\n", e->codice,
                   e->codice < SCHED_CODICI ? rst20_3B10[e->codice] : 0, e->frame);
            ferma(1);
        }
        if (i + 1 < i1 && ev[i + 1].tipo == 'N' && pendente(&ev[i + 1])) {
            nmi_corpo = &ev[i + 1];
            i++;
        }
        ultima_d = e;
        uint8_t slot = 0xff, codice = 0;
        indice_prima = indice;
        int r = spedisci_una(&slot, &codice);
        spedizioni++;
        if (r != 1 || slot != e->slot || codice != e->codice) {
            printf("DISPATCH frame %ld: atteso %02X/%02X, ottenuto %02X/%02X (esito %d)\n",
                   e->frame, e->slot, e->codice, slot, codice, r);
            ferma(1);
        }
        if (corpo_mancante >= 0) {
            printf("CORPO %02X non tradotto al frame %ld\n", corpo_mancante, e->frame);
            ferma(1);
        }
        if (nemici_mancante) {
            printf("ROUTINE %04X mancante (motore dei nemici) al frame %ld\n",
                   nemici_mancante, e->frame);
            ferma(1);
        }
        if (nemici_fuori_sito) {
            printf("INDICE %02X fuori dalla tabella rst del sito %04X al frame %ld\n",
                   nemici_fuori_indice, nemici_fuori_sito, e->frame);
            ferma(1);
        }
        if (i_scan < n_scan) {
            printf("NMI del frame %ld non iniettata nella scansione\n", nmi_scan[i_scan]->frame);
            ferma(1);
        }
        n_scan = i_scan = 0;
    }
    /* NMI finali dopo l'ultima spedizione */
    for (; i_scan < n_scan; i_scan++)
        esegui_nmi(nmi_scan[i_scan]);
    printf("spedizioni eseguite: %ld, fino al frame %ld\n", spedizioni, frame_fine);
    riepilogo();
    return div_pulite == 0 ? 0 : 1;
}
