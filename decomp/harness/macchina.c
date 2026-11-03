/* Macchina del giro dal boot (lotto H4a, brief 44): il gioco parte dal
 * vettore 0000 senza oracolo che lo guidi. Il C fa il boot 2833-2959, poi
 * lo spedidore; la NMI entra una volta per frame (sub_0066) in un punto di
 * sincronia, gli ingressi arrivano dal file estratto dalla partita
 * registrata, le porte sono quelle di MAME (e600 maschera NMI in scrittura,
 * e604 ribaltamento schermo in scrittura, DSW 02/00 in lettura).
 *
 * Dove entra la NMI. L'oracolo (giro_boot.lua) dice per ogni frame f il
 * ritorno della NMI (riga F) e l'impronta della RAM a quel momento. Le
 * routine del percorso di avvio chiamano MACCHINA_PUNTO(da, a) a ogni
 * iterazione dei loro cicli, con l'intervallo di indirizzi Z80 dove la RAM
 * è quella corrente. La NMI pendente si esegue al primo punto il cui
 * intervallo contiene il suo ritorno e in cui l'impronta del C coincide con
 * quella dell'oracolo; se i punti passano senza coincidenza oltre la
 * finestra legale (LIMITE_PUNTI), il giro si ferma con una diagnostica.
 * L'impronta è quella del replay più dbc0-dbff (pila del boot, GIRO.md).
 * Dopo il boot lo schema è quello di replay.c: gli stessi corpi tradotti
 * (corpo_attivita_replay[]) e arresto al primo non tradotto. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "routine.h"
#include "sched.h"
#include "rst20_tables.h"

attivita_fn corpo_attivita[SCHED_CODICI];
extern attivita_fn corpo_attivita_replay[SCHED_CODICI];

#define FRAME_DUMP   602       /* frame del dump completo (giro_boot.sh) */
#define LIMITE_PUNTI 40000     /* punti fra due NMI: ben oltre un frame di Z80 */
#define MAX_FRAME    8192

/* ---- impronta: FNV-1a 32 bit di c000-e5ff esclusi c040-c043, cf00-cfff e
 * dbc0-dbff (stesse zone di giro_boot.lua) */
static int escluso(unsigned a)
{
    return (a >= 0xc040 && a <= 0xc043) || (a >= 0xcf00 && a <= 0xcfff) ||
           (a >= 0xdbc0 && a <= 0xdbff);
}

static uint32_t impronta(void)
{
    uint32_t h = 2166136261u;
    for (unsigned a = 0xc000; a < 0xe600; a++)
        if (!escluso(a))
            h = (h ^ ram[a - 0xc000]) * 16777619u;
    return h;
}

/* ---- oracolo, registro delle spedizioni, ingressi */
struct riga_f { long frame; uint16_t ritorno; uint32_t hash; };
struct spedizione { long frame; uint8_t slot, codice; };

static struct riga_f *ff;
static size_t n_ff, k_ff;              /* righe F e prossima NMI da eseguire */
static struct spedizione *sp;
static size_t n_sp, k_sp;
static uint8_t ingressi[MAX_FRAME][3];

static void *cresci(void *p, size_t *cap, size_t n, size_t dim)
{
    if (n < *cap)
        return p;
    *cap = *cap ? *cap * 2 : 4096;
    p = realloc(p, *cap * dim);
    if (!p) { perror("realloc"); exit(2); }
    return p;
}

static FILE *apri(const char *nome)
{
    FILE *f = fopen(nome, "r");
    if (!f) { perror(nome); exit(2); }
    return f;
}

static void carica_oracolo(const char *nome)
{
    FILE *f = apri(nome);
    char riga[160];
    size_t cap = 0;
    while (fgets(riga, sizeof riga, f)) {
        struct riga_f r;
        unsigned a, b, c, rit, h;
        if (sscanf(riga, "F %ld %x %x %x %x %x", &r.frame, &a, &b, &c, &rit, &h) != 6)
            continue;
        r.ritorno = (uint16_t)rit; r.hash = h;
        ff = cresci(ff, &cap, n_ff, sizeof *ff);
        ff[n_ff++] = r;
    }
    fclose(f);
}

/* Solo le righe D: le W le rifà il C, le N non servono (il ritorno sta
 * nelle righe F). */
static void carica_registro(const char *nome)
{
    FILE *f = apri(nome);
    char riga[160];
    size_t cap = 0;
    while (fgets(riga, sizeof riga, f)) {
        struct spedizione e;
        unsigned a, b;
        if (riga[0] != 'D' || sscanf(riga + 1, "%ld %x %x", &e.frame, &a, &b) != 3)
            continue;
        e.slot = (uint8_t)a; e.codice = (uint8_t)b;
        sp = cresci(sp, &cap, n_sp, sizeof *sp);
        sp[n_sp++] = e;
    }
    fclose(f);
}

static void carica_ingressi(const char *nome)
{
    FILE *f = apri(nome);
    char riga[80];
    while (fgets(riga, sizeof riga, f)) {
        long fr;
        unsigned a, b, c;
        if (sscanf(riga, "%ld %x %x %x", &fr, &a, &b, &c) != 4 || fr < 0 || fr >= MAX_FRAME)
            continue;
        ingressi[fr][0] = (uint8_t)a; ingressi[fr][1] = (uint8_t)b; ingressi[fr][2] = (uint8_t)c;
    }
    fclose(f);
}

/* ---- stato del giro */
static char prefisso[512];
static long frame_fine = -1;
static long nmi_totali, nmi_divergenti;
static long punti;                     /* chiamate di punto dall'ultima NMI */
static int in_nmi;
static uint8_t maschera_nmi;           /* e600 in scrittura, bit 0 */
static uint8_t ribalta;                /* e604 in scrittura */

static void riepilogo(void)
{
    printf("NMI confrontate: %ld, divergenti: %ld\n", nmi_totali, nmi_divergenti);
}

static void ferma(int codice)
{
    riepilogo();
    exit(codice);
}

static void gancio_porta(uint16_t a, uint8_t v)
{
    if (a == 0xe600)
        maschera_nmi = v & 1;
    else if (a == 0xe604)
        ribalta = v;
    else
        ram[a - 0xc000] = v;           /* e800, e803 ecc.: fuori dal confronto */
}

/* Confronto byte per byte col dump di MAME al frame FRAME_DUMP, prima della
 * NMI di quel frame: richiude il buco delle esclusioni (GIRO.md). */
static void sanita(void)
{
    char nome[600];
    snprintf(nome, sizeof nome, "%s_%d.bin", prefisso, FRAME_DUMP);
    FILE *f = fopen(nome, "rb");
    static uint8_t mame[0x4000];
    if (!f || fread(mame, 1, sizeof mame, f) != sizeof mame) {
        printf("SANITA %d: dump %s assente o corto\n", FRAME_DUMP, nome);
        if (f) fclose(f);
        return;
    }
    fclose(f);
    int tot = 0, stampati = 0;
    for (unsigned a = 0xc000; a < 0xe600; a++) {
        if (escluso(a) || ram[a - 0xc000] == mame[a - 0xc000])
            continue;
        tot++;
        if (stampati++ < 32)
            printf("  %04X: C %02X MAME %02X\n", a, ram[a - 0xc000], mame[a - 0xc000]);
    }
    printf("SANITA %d: byte diversi in c000-e5ff (escluse c040-c043, cf00-cfff, dbc0-dbff): "
           "%d, %s\n", FRAME_DUMP, tot, tot ? "DIVERSO" : "coincide");
    if (tot) {
        nmi_divergenti++;
        ferma(1);
    }
}

static void esegui_nmi(const struct riga_f *r)
{
    if (!maschera_nmi) {
        printf("NMI del frame %ld con la maschera spenta (e600 bit 0 = 0)\n", r->frame);
        nmi_divergenti++;
        ferma(1);
    }
    nmi_totali++;
    if (r->frame == FRAME_DUMP)
        sanita();
    if (r->frame >= 0 && r->frame < MAX_FRAME) {   /* porte vive del frame */
        ram[0xe600 - 0xc000] = ingressi[r->frame][0];
        ram[0xe601 - 0xc000] = ingressi[r->frame][1];
        ram[0xe602 - 0xc000] = ingressi[r->frame][2];
    }
    struct z80 z = {0};
    in_nmi = 1;
    sub_0066(&z);
    in_nmi = 0;
    k_ff++;
    punti = 0;
}

/* Punto di sincronia (MACCHINA_PUNTO). In un tratto dove la RAM non cambia
 * (rilettura del test RAM, attesa dei 224 frame) più NMI cadono nello
 * stesso punto: dopo averne eseguita una si riprova con la successiva. */
static void punto(uint16_t da, uint16_t a)
{
    int eseguita;
    do {
        eseguita = 0;
        if (in_nmi || !maschera_nmi || k_ff >= n_ff)
            return;
        const struct riga_f *r = &ff[k_ff];
        if (frame_fine >= 0 && r->frame > frame_fine)
            return;
        if (++punti > LIMITE_PUNTI) {
            printf("NMI del frame %ld non collocata: ritorno %04X, hash C %08X MAME %08X "
                   "(%ld punti senza coincidenza, ultimo intervallo %04X-%04X)\n",
                   r->frame, r->ritorno, impronta(), r->hash, punti, da, a);
            nmi_divergenti++;
            ferma(1);
        }
        if (r->ritorno >= da && r->ritorno <= a && impronta() == r->hash) {
            esegui_nmi(r);
            eseguita = 1;
        }
    } while (eseguita);
}

/* ---- boot 2833-2959 */

/* Ciclo di azzeramento `ld (hl),a; inc hl; djnz; dec c; jr nz` di n byte:
 * `ini` è l'inizio del tratto (impostazione dei registri), `scr` l'istruzione
 * che scrive, `fine` l'ultima del giro dopo la scrittura. */
static void azzera(uint16_t hl, unsigned n, uint16_t ini, uint16_t scr, uint16_t fine)
{
    for (unsigned i = 0; i < n; i++, hl++) {
        MACCHINA_PUNTO(i ? scr : ini, scr);
        M(hl) = 0;
        MACCHINA_PUNTO(scr + 1, fine);
    }
}

/* Esito di un test RAM: `jr c` salta `inc ix`; poi `add ix,ix`. */
static void esito(uint16_t *ix, const struct z80 *z)
{
    if (!(z->f & 1))
        (*ix)++;
    *ix = (uint16_t)(*ix * 2);
}

static void test_rami(struct z80 *z, void (*fn)(struct z80 *), uint16_t hl, uint16_t bc,
                      uint16_t *ix)
{
    SET_HL(z, hl);
    SET_BC(z, bc);
    fn(z);
    esito(ix, z);
}

/* SP e registro I non si modellano: la pila (cf00-cfff dal frame 68, dbxx
 * prima) è fuori dall'impronta. IX è una variabile locale. In C i test RAM
 * passano sempre (IX = 0), quindi niente nuovo tentativo a 28D9. */
static void boot_2833(void)
{
    struct z80 z = {0};
    uint16_t ix = 0;
    ram[0xe604 - 0xc000] = 0x02;       /* DSW1 e DSW2 della registrazione */
    ram[0xe605 - 0xc000] = 0x00;
    scrivi_porta(0xe600, 0);           /* 0000: NMI spenta */
    test_rami(&z, sub_2bf3, 0xc000, 0x0800, &ix);
    test_rami(&z, sub_2bf3, 0xc800, 0x0800, &ix);
    M(0xc044) = 0;
    scrivi_porta(0xe600, 1);           /* 285D: NMI accesa */
    azzera(0xc000, 0x1000, 0x2860, 0x2867, 0x286c);
    test_rami(&z, sub_2bf3, 0xd000, 0x0800, &ix);
    test_rami(&z, sub_2bf3, 0xd800, 0x0800, &ix);
    test_rami(&z, sub_2bc9, 0xe400, 0x0100, &ix);
    test_rami(&z, sub_2bc9, 0xe500, 0x0100, &ix);
    test_rami(&z, sub_2bf3, 0xe020, 0x0060, &ix);
    azzera(0xd000, 0x1080, 0x28b6, 0x28bd, 0x28c2);    /* bc = 8011 */
    azzera(0xe400, 0x0200, 0x28c4, 0x28ca, 0x28cf);
    if (ix & 0x7f) {
        printf("boot: test RAM con IX = %04X, nuovo tentativo 28D9 non modellato\n", ix);
        ferma(2);
    }
    M(0xc044) = 4;
    sub_3678(&z);
    scrivi_porta(0xe604, z.a);         /* 28F1: ribaltamento schermo, non la DSW */
    sub_2980(&z);
    z.ix = ix;
    sub_2a03(&z);
    sub_2980(&z);
    sub_29cf(&z);
    uint16_t hl = 0x295c, de = 0xca4c; /* 2908: classifica iniziale, 10 voci */
    for (int n = 0; n < 10; n++) {
        for (int i = 0; i < 4; i++) M(de++) = leggi(0x297a + i);
        for (int i = 0; i < 3; i++) M(de++) = leggi(hl++);
        for (int i = 0; i < 2; i++) M(de++) = leggi(0x297e + i);
    }
    azzera(0xd000, 0x0800, 0x2922, 0x2929, 0x292e);
    sub_36c3(&z);
    azzera(0xc1ba, 0x7d, 0x2933, 0x2939, 0x293c);
    prossimo_libero = 0;               /* c04e */
    indice = 0;                        /* c04f */
    M(0xc1ae) = 0;
    sub_673f(&z);
    scrivi_porta(0xe803, 0xb7);
    posta_attivita(0x1f);
    posta_attivita(0x06);
}

/* ---- spedidore (dopo `jp 3AF3`): stesso schema di replay.c */

/* Ogni corpo passa di qui: confronta slot e codice con la prossima riga D
 * dell'oracolo, poi esegue il corpo tradotto o si ferma. */
static void involucro(uint8_t slot)
{
    uint8_t codice = tabella[slot];
    if (k_sp >= n_sp) {
        printf("spedizioni registrate finite\n");
        ferma(0);
    }
    const struct spedizione *e = &sp[k_sp++];
    if (slot != e->slot || codice != e->codice) {
        printf("DISPATCH frame %ld: atteso %02X/%02X, ottenuto %02X/%02X\n",
               e->frame, e->slot, e->codice, slot, codice);
        ferma(1);
    }
    if (codice >= SCHED_CODICI || !corpo_attivita_replay[codice]) {
        printf("CORPO %02X (%04X) non tradotto al frame %ld\n", codice,
               codice < SCHED_CODICI ? rst20_3B10[codice] : 0, e->frame);
        ferma(1);
    }
    corpo_attivita_replay[codice](slot);
}

int macchina_esegui(const char *oracolo, const char *disp, const char *ingr, long fine)
{
    frame_fine = fine;
    carica_oracolo(oracolo);
    carica_registro(disp);
    carica_ingressi(ingr);
    snprintf(prefisso, sizeof prefisso, "%s", oracolo);
    char *suff = strstr(prefisso, "_oracolo.txt");
    if (suff) *suff = 0;
    carica_rom();
    for (int i = 0; i < SCHED_CODICI; i++)
        corpo_attivita[i] = involucro;
    macchina_punto = punto;
    porte_gancio_scrittura = gancio_porta;

    boot_2833();
    for (;;) {
        uint8_t slot, codice;
        int r = spedisci_una(&slot, &codice);
        if (r != 1) {
            printf("spedidore: esito %d\n", r);
            ferma(1);
        }
    }
}
