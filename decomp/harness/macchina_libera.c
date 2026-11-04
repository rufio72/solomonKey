/* Macchina senza oracolo (lotto H5, brief 49): il gioco parte dal vettore
 * 0000 e gira senza registro né impronte. Il boot è quello di macchina.c
 * (boot_2833); dopo, lo spedidore gira in un ciclo e la NMI entra ai confini
 * (scansione dello spedidore, prima del corpo). Non c'è un modello dei cicli
 * del Z80: l'istante vero delle NMI dentro i corpi viene solo dall'oracolo
 * (modo --oracolo di gioca), qui la cadenza è un'approssimazione: 22 corpi
 * per frame, salvo le routine che dichiarano il frame pieno con
 * MACCHINA_FRAME_PIENO() (oggi 3097, cerchio di stelline; brief 53).
 * R: surrogato deterministico, vedi r_libero (brief 56). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "routine.h"
#include "sched.h"
#include "nemici.h"
#include "macchina.h"

/* Boot: una NMI ogni PUNTI_PER_NMI_BOOT chiamate di MACCHINA_PUNTO. Il boot
 * completo ne fa circa 660000 (misurato) e in MAME contiene 537 NMI (frame
 * 66-602). Con 1150 il boot libero finisce al frame ~602 (misurato: 1000 ->
 * 615, 1100 -> 605, 1200 -> 598, 1300 -> 592; con 32 non finiva entro 3000). */
#define PUNTI_PER_NMI_BOOT 1150
/* Dopo il boot: corpi per frame. Misurato sul registro di MAME (frame > 700):
 * da 1 a 30 spedizioni per frame, media 21,67. */
#define CORPI_PER_FRAME 22
/* Letture a vuoto della scansione (tabella senza attività) per frame; il giro
 * dell'attesa fa al più 4096 letture per chiamata di spedisci_una. */
#define LETTURE_PER_FRAME 2000
#define VUOTI_MASSIMI 8        /* spedisci_una a vuoto di fila: tabella morta */
#define PRIMO_FRAME 66         /* prima NMI di MAME */

struct libera_opz libera_opz;
int (*libera_frame)(long frame, uint8_t v[3]) = NULL;
extern attivita_fn corpo_attivita_replay[SCHED_CODICI];

static long frame_corr = PRIMO_FRAME;
static int in_nmi;
static long punti, corpi, letture;
static jmp_buf uscita;

static void fermo(const char *motivo)
{
    printf("FERMO %s al frame %ld\n", motivo, frame_corr);
    fflush(stdout);
    longjmp(uscita, 3);
}

/* Registro R surrogato (brief 56): nessun conteggio delle istruzioni. Lo
 * stato avanza di un passo a ogni NMI (il tempo passa) e di uno a ogni
 * lettura (due letture nello stesso frame danno valori diversi); R1 sono
 * i 7 bit alti. Deterministico a parità di ingressi. */
#define R_SEME 0x4302u
static uint32_t r_stato;
static void r_avanza(void) { r_stato = r_stato * 1103515245u + 12345u; }
static uint8_t r_libero(void) { r_avanza(); return (uint8_t)(r_stato >> 25); }

/* NMI libera: ingressi dal chiamante, poi sub_0066. Solo se la maschera la
 * consente e mai dentro un'altra NMI. */
static void nmi_libera(void)
{
    if (in_nmi || !maschera_nmi)
        return;
    if (libera_opz.frame_max && frame_corr >= PRIMO_FRAME + libera_opz.frame_max)
        longjmp(uscita, 1);
    uint8_t v[3] = {0, 0, 0};
    if (libera_frame && libera_frame(frame_corr, v))
        longjmp(uscita, 1);
    ram[0xe600 - 0xc000] = v[0];
    ram[0xe601 - 0xc000] = v[1];
    ram[0xe602 - 0xc000] = v[2];
    struct z80 z = {0};
    in_nmi = 1;
    sub_0066(&z);
    in_nmi = 0;
    frame_corr++;
    r_avanza();
    punti = corpi = letture = 0;
}

static void punto_libero(uint16_t da, uint16_t a)
{
    (void)da; (void)a;
    if (++punti >= PUNTI_PER_NMI_BOOT)
        nmi_libera();
}

static void scansione_libera(int lettura)
{
    (void)lettura;
    if (++letture >= LETTURE_PER_FRAME || corpi >= CORPI_PER_FRAME)
        nmi_libera();
}

/* Frame pieno (brief 53): la routine chiamante occupa nell'originale il resto
 * del frame; al confine seguente scansione_libera (o involucro_libero) vede
 * corpi >= CORPI_PER_FRAME e chiama nmi_libera. */
static void frame_pieno_libero(void)
{
    corpi = CORPI_PER_FRAME;
}

static void involucro_libero(uint8_t slot)
{
    uint8_t codice = tabella[slot];
    if (++corpi > CORPI_PER_FRAME)
        nmi_libera();
    if (codice >= SCHED_CODICI || !corpo_attivita_replay[codice]) {
        char m[64];
        snprintf(m, sizeof m, "corpo %02X non tradotto", codice);
        fermo(m);
    }
    corpo_attivita_replay[codice](slot);
}

int macchina_libera_esegui(void)
{
    int rc = setjmp(uscita);
    if (rc)
        return rc == 1 ? 0 : rc;
    r_stato = R_SEME;
    carica_rom();
    /* f000-ffff è ROM ma leggi() la cerca in ram[] (come macchina_esegui) */
    memcpy(&ram[0xf000 - 0xc000], &rom[0xf000], 0x1000);
    macchina_punto = punto_libero;
    porte_gancio_scrittura = gancio_porta;
    giro_dal_boot = 1;
    boot_2833();

    macchina_punto = NULL;
    nemici_punto = NULL;
    sched_gancio_scansione = scansione_libera;
    nemici_r_gancio = NULL;
    nemici_r_valore = r_libero;
    macchina_frame_pieno = frame_pieno_libero;  /* non durante il boot */
    for (int i = 0; i < SCHED_CODICI; i++)
        corpo_attivita[i] = involucro_libero;
    punti = corpi = letture = 0;
    int vuoti = 0;
    for (;;) {
        uint8_t slot = 0xff, codice = 0;
        int r = spedisci_una(&slot, &codice);
        if (r < 0) {
            char m[64];
            snprintf(m, sizeof m, "codice %02X fuori dalla tabella", codice);
            fermo(m);
        }
        if (r == 0) {
            if (++vuoti >= VUOTI_MASSIMI)
                fermo("tabella delle attività vuota");
        } else {
            vuoti = 0;
        }
        if (nemici_mancante) {
            char m[64];
            snprintf(m, sizeof m, "routine %04X mancante", nemici_mancante);
            fermo(m);
        }
        if (nemici_fuori_sito) {
            char m[80];
            snprintf(m, sizeof m, "indice %02X fuori dalla tabella rst del sito %04X",
                     nemici_fuori_indice, nemici_fuori_sito);
            fermo(m);
        }
    }
}
