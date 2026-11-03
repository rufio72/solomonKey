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
 * Dopo il boot (lotto H4b1, brief 45) lo schema è quello di replay.c: gli
 * stessi corpi tradotti (corpo_attivita_replay[]), le NMI nella scansione
 * dello spedidore, prima del corpo e a metà attività (punti NEMICI_PUNTO),
 * confrontate con l'impronta dell'oracolo; arresto al primo corpo non
 * tradotto. Le divergenze nello spedidore fermano il giro, quelle a metà
 * attività si contano. Il registro R non è modellato: una lettura ferma il
 * giro (nemici_r_gancio). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "routine.h"
#include "sched.h"
#include "nemici.h"
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
struct evento { char tipo; long frame; uint8_t slot, codice; uint16_t ritorno; int letture; };

static struct riga_f *ff;
static size_t n_ff, k_ff;              /* righe F e prossima NMI da eseguire */
static struct evento *ev;              /* righe D e N del registro */
static size_t n_ev;
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

/* Righe D (spedizioni) e N (ingresso della NMI) del registro, come
 * replay.c; le W e le I le rifà il C. */
static void carica_registro(const char *nome)
{
    FILE *f = apri(nome);
    char riga[160];
    size_t cap = 0;
    while (fgets(riga, sizeof riga, f)) {
        struct evento e = {0};
        unsigned a, b;
        if (riga[0] == 'D' && sscanf(riga + 1, "%ld %x %x", &e.frame, &a, &b) == 3) {
            e.slot = (uint8_t)a; e.codice = (uint8_t)b;
        } else if (riga[0] == 'N' && sscanf(riga + 1, "%ld %x %d", &e.frame, &a, &e.letture) == 3) {
            e.ritorno = (uint16_t)a;
        } else {
            continue;
        }
        e.tipo = riga[0];
        ev = cresci(ev, &cap, n_ev, sizeof *ev);
        ev[n_ev++] = e;
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
static long spedizioni;                /* corpi eseguiti dopo il boot */
static long dopo_pulite, dopo_meta;    /* NMI dopo il boot per tipo */
static long div_pulite, div_meta;      /* e divergenti per tipo */
static long punti;                     /* chiamate di punto dall'ultima NMI */
static int in_nmi;
static uint8_t maschera_nmi;           /* e600 in scrittura, bit 0 */
static uint8_t ribalta;                /* e604 in scrittura */

static void riepilogo(void)
{
    printf("spedizioni eseguite: %ld\n", spedizioni);
    printf("NMI confrontate: %ld, divergenti: %ld\n", nmi_totali, nmi_divergenti);
    printf("dopo il boot: nello spedidore %ld (divergenti %ld), a metà attività %ld "
           "(divergenti %ld)\n", dopo_pulite, div_pulite, dopo_meta, div_meta);
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

/* Confronto byte per byte col dump di MAME del frame, prima della NMI di
 * quel frame: richiude il buco delle esclusioni (GIRO.md). Stampa i primi
 * 32 byte diversi; restituisce il loro numero, o -1 se il dump manca. */
static int diff_dump(long frame)
{
    char nome[600];
    snprintf(nome, sizeof nome, "%s_%ld.bin", prefisso, frame);
    FILE *f = fopen(nome, "rb");
    static uint8_t mame[0x4000];
    if (!f || fread(mame, 1, sizeof mame, f) != sizeof mame) {
        printf("dump %s assente o corto (REPLAY_DUMPS=%ld per averlo)\n", nome, frame);
        if (f) fclose(f);
        return -1;
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
    return tot;
}

static void sanita(void)
{
    int tot = diff_dump(FRAME_DUMP);
    if (tot < 0)
        return;
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

/* ---- spedidore (dopo `jp 3AF3`): stesso schema di replay.c, vedi lì i
 * commenti estesi. Una NMI è «pulita» se il ritorno sta in 3AF3-3B10 o se
 * viene eseguita prima del corpo; «a metà attività» altrimenti. */

static int nel_rst20(uint16_t r)
{
    return (r >= 0x0008 && r <= 0x000d) || (r >= 0x0010 && r <= 0x0014) ||
           (r >= 0x0018 && r <= 0x001a) || (r >= 0x0020 && r <= 0x0026);
}

static int pendente(const struct evento *n)
{
    return n->ritorno >= 0x3afb && n->ritorno <= 0x3b10 && n->letture == 0;
}

static int nello_spedidore(uint16_t r)
{
    return r >= 0x3af3 && r <= 0x3b10;
}

static int prima_del_corpo;            /* NMI eseguita prima del corpo */
static const struct evento *ultima_d;
static const struct evento *nmi_corpo, *nmi_scan[256];   /* corpi lunghi: fino a 9 NMI in uno (frame 603-611) */
static const struct evento *attese[64];    /* N con 0 letture dopo la D, in ordine */
static int n_attese, k_attese;             /* quante, e quante già eseguite */
static int n_scan, i_scan;
#define nmi_attesa (k_attese < n_attese ? attese[k_attese] : NULL)
static int corpo_mancante = -1;
static uint8_t indice_prima;           /* c04f prima della spedizione in corso */

/* NMI di un evento N dopo il boot: confronto d'impronta con la riga F,
 * poi esecuzione. La divergenza di una NMI pulita ferma il giro. */
static void esegui_nmi_ev(const struct evento *n)
{
    if (k_ff >= n_ff || ff[k_ff].frame != n->frame) {
        printf("oracolo e registro disallineati alla NMI del frame %ld\n", n->frame);
        ferma(2);
    }
    const struct riga_f *r = &ff[k_ff];
    uint32_t h = impronta();
    int pulita = nello_spedidore(r->ritorno) || prima_del_corpo;
    if (pulita) dopo_pulite++; else dopo_meta++;
    if (h != r->hash) {
        if (pulita) {
            div_pulite++;
            nmi_divergenti++;
            printf("DIVERGENZA frame %ld: hash C %08X MAME %08X; ritorno NMI oracolo %04X "
                   "(spedidore); ultima D %02X %02X\n", r->frame, h, r->hash, r->ritorno,
                   ultima_d ? ultima_d->slot : 0xff, ultima_d ? ultima_d->codice : 0xff);
            if (diff_dump(r->frame) >= 0)
                printf("  (confronto con il dump del frame %ld)\n", r->frame);
            ferma(1);
        }
        div_meta++;
        nmi_divergenti++;
    }
    esegui_nmi(r);
}

/* Con `largo` l'impronta non si controlla: il ritorno sta dentro una
 * routine tradotta senza punti interni, la cui RAM a metà non esiste nel
 * C; la NMI si esegue al confine (dopo la routine) e il confronto con la
 * riga F la classifica a metà attività. */
static int prova_punto_x(uint16_t da, uint16_t a, int largo)
{
    const struct evento *n = nmi_attesa;
    if (!n || n->ritorno < da || n->ritorno > a)
        return 0;
    if (k_ff >= n_ff || ff[k_ff].frame != n->frame || (!largo && impronta() != ff[k_ff].hash))
        return 0;
    k_attese++;
    esegui_nmi_ev(n);
    return 1;
}

static int prova_punto(uint16_t da, uint16_t a)
{
    return prova_punto_x(da, a, 0);
}

static void punto_largo(uint16_t da, uint16_t a)
{
    while (prova_punto_x(da, a, 1))
        ;                              /* più NMI in coda nella stessa routine */
}

static void punto_nemici(uint16_t da, uint16_t a)
{
    prova_punto(da, a);
}

static void gancio_scansione(int lettura)
{
    while (i_scan < n_scan && nmi_scan[i_scan]->letture <= lettura)
        esegui_nmi_ev(nmi_scan[i_scan++]);
}

/* Sentinella del registro R: il giro non ha valori di R (registra_r.sh). */
static void r_letto(void)
{
    printf("R letto prima della NMI del frame %ld: oracolo del registro R assente "
           "(registra_r.sh)\n", k_ff < n_ff ? ff[k_ff].frame : -1L);
    ferma(1);
}

/* Involucro di tutti i corpi: la NMI pendente, poi il corpo tradotto. */
static void involucro(uint8_t slot)
{
    uint8_t codice = tabella[slot];   /* letto prima della NMI, come B a 3B02 */
    if (nmi_corpo) {
        const struct evento *n = nmi_corpo;
        nmi_corpo = NULL;
        prima_del_corpo = 1;
        if (n->ritorno < 0x3b0f) {
            uint8_t nuovo = indice;
            indice = indice_prima;
            esegui_nmi_ev(n);
            indice = nuovo;
        } else {
            esegui_nmi_ev(n);          /* 3B0F-3B10: c04f già scritto */
        }
        prima_del_corpo = 0;
    }
    if (nmi_attesa) {
        uint16_t r = nmi_attesa->ritorno;
        if (nel_rst20(r) || (codice < SCHED_CODICI && r == rst20_3B10[codice]) ||
            (codice == 0x0a && r >= 0x0038 && r <= 0x0042)) {
            prima_del_corpo = 1;       /* c04f già scritto a 3B0C */
            prova_punto(r, r);
            prima_del_corpo = 0;
        }
    }
    if (codice >= SCHED_CODICI || !corpo_attivita_replay[codice]) {
        corpo_mancante = codice;
        return;
    }
    corpo_attivita_replay[codice](slot);
}

/* Dopo il boot: eventi D/N del registro dalla D che segue la N dell'ultima
 * NMI del boot, fino all'ultima N entro frame_fine. Ritorna il codice di
 * uscita. */
static int giro_spedidore(void)
{
    size_t i0 = 0;
    long f_boot = k_ff ? ff[k_ff - 1].frame : -1;
    while (i0 < n_ev && !(ev[i0].tipo == 'N' && ev[i0].frame == f_boot)) i0++;
    i0++;
    if (i0 >= n_ev || ev[i0].tipo != 'D' || ev[i0].frame != f_boot || ev[i0].slot != 0 ||
        ev[i0].codice != 0x1f) {
        printf("registro: dopo la NMI del frame %ld manca la D 00 1F\n", f_boot);
        ferma(2);
    }
    size_t i1 = i0;
    for (size_t i = i0; i < n_ev; i++)
        if (ev[i].tipo == 'N' && (frame_fine < 0 || ev[i].frame <= frame_fine)) i1 = i + 1;
    if (frame_fine < 0)
        frame_fine = ff[n_ff - 1].frame;

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
        if (i + 1 < i1 && ev[i + 1].tipo == 'N' && pendente(&ev[i + 1])) {
            nmi_corpo = &ev[i + 1];
            i++;
        } else {
            for (size_t j = i + 1; j < i1 && n_attese < 64 && ev[j].tipo == 'N' &&
                                   ev[j].letture == 0; j++)
                attese[n_attese++] = &ev[j];
        }
        ultima_d = e;
        uint8_t slot = 0xff, codice = 0;
        indice_prima = indice;
        int r = spedisci_una(&slot, &codice);
        i += k_attese;                 /* N già eseguite in un punto di sincronia */
        n_attese = k_attese = 0;
        if (r != 1 || slot != e->slot || codice != e->codice) {
            printf("DISPATCH frame %ld: atteso %02X/%02X, ottenuto %02X/%02X (esito %d)\n",
                   e->frame, e->slot, e->codice, slot, codice, r);
            ferma(1);
        }
        if (corpo_mancante >= 0) {
            printf("CORPO %02X (%04X) non tradotto al frame %ld\n", corpo_mancante,
                   corpo_mancante < SCHED_CODICI ? rst20_3B10[corpo_mancante] : 0, e->frame);
            ferma(1);
        }
        spedizioni++;
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
    for (; i_scan < n_scan; i_scan++)  /* NMI finali dopo l'ultima spedizione */
        esegui_nmi_ev(nmi_scan[i_scan]);
    riepilogo();
    return div_pulite == 0 ? 0 : 1;
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
    /* f000-ffff è ROM (tabelle dei fondali e delle palette) ma leggi() la
     * cerca in ram[]: il replay la ha nel dump, qui si copia (brief 45) */
    memcpy(&ram[0xf000 - 0xc000], &rom[0xf000], 0x1000);
    macchina_punto = punto;
    porte_gancio_scrittura = gancio_porta;

    boot_2833();

    /* fine del boot: i punti del boot non iniettano più NMI, subentrano
     * quelli dello spedidore e dei corpi */
    macchina_punto = NULL;
    nemici_punto = punto_nemici;
    attract_punto_largo = punto_largo;
    sched_gancio_scansione = gancio_scansione;
    nemici_r_gancio = r_letto;
    for (int i = 0; i < SCHED_CODICI; i++)
        corpo_attivita[i] = involucro;
    return giro_spedidore();
}
