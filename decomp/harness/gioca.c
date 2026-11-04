/* gioca: il gioco si gioca (lotto H5, brief 49). Unisce la macchina del giro
 * dal boot alla finestra SDL2 di H3, con tastiera/joystick, DSW da riga di
 * comando, registrazione e riproduzione degli ingressi (formato di
 * out/ingressi_replay.txt: "<frame> <e600> <e601> <e602>", byte in esadecimale).
 *   gioca [-s N] [-n N | -m N] [--registra F] [--riproduci F]
 *         [--hash-frame F[,F...]] [opzioni DSW] [--stampa-dsw] [--mappa]
 *   gioca --oracolo ORACOLO.txt DISP.txt --riproduci F ...   (macchina di H4)
 * Modo libero: NMI ai confini dello spedidore (macchina_libera.c), si ferma
 * in modo pulito su codice non tradotto. Modo --oracolo: riproduce la partita
 * registrata con l'istante delle NMI dell'oracolo. Comandi: ESC esce, P pausa,
 * F2 reset (non con --riproduci). Vedi GIRO.md. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <SDL.h>
#include "video_sdl.h"
#include "ram.h"
#include "macchina.h"
#include "nemici.h"
#include "scheda_audio.h"
#include "gioca_joy.h"
#include "gioca_opzioni.h"

int gioca_audio_apri(void);
void gioca_audio_spingi(const int16_t *buf, int n);
void gioca_audio_chiudi(void);
void gioca_audio_volume(int percento);

int dsw_applica(const char *opzione, const char *valore, uint8_t *d1, uint8_t *d2);
void tasti_stampa_mappa(void);
void tasti_ingressi(const uint8_t *stato, uint8_t out[3]);

#define MAX_FRAME_FILE 65536
#define MAX_HASH 64

static vsdl_opz opz;
static char **argv_orig;
static const char *file_registra, *file_riproduci;
static const char *orac_oracolo, *orac_disp;
static const char *file_flusso_ay, *file_registra_latch;
static int muto, audio_aperto;
static FILE *registra;
static uint8_t (*riprodotti)[3];
static long hash_frame[MAX_HASH];
static int n_hash_frame;
static uint8_t reso[VID_ALTO][VID_LARGO][3];
static long rese, frame_ultimo = -1;
static uint64_t hash_ultimo;
static int finestra, finito, p_prec, esito_sdl;

static void finale(void)
{
    if (finito) return;
    finito = 1;
    if (registra) { fclose(registra); registra = NULL; }
    scheda_audio_chiudi();
    if (audio_aperto) { gioca_audio_chiudi(); audio_aperto = 0; }
    if (finestra) { joy_chiudi(); vsdl_chiudi(); finestra = 0; }
    printf("FRAME rese %ld\n", rese);
    printf("HASH ultimo %016llx\n", (unsigned long long)hash_ultimo);
    if (orac_oracolo)
        printf("FRAME ultimo %ld\n", frame_ultimo);
    printf("AUDIO campioni %ld\n", scheda_audio_campioni());
    printf("AUDIO hash %016llx\n", (unsigned long long)scheda_audio_hash());
    fflush(stdout);
}

static void carica_riprodotti(const char *nome)
{
    FILE *f = fopen(nome, "r");
    if (!f) { perror(nome); exit(2); }
    riprodotti = calloc(MAX_FRAME_FILE, 3);
    if (!riprodotti) { perror("calloc"); exit(2); }
    char riga[80];
    while (fgets(riga, sizeof riga, f)) {
        long fr;
        unsigned a, b, c;
        if (sscanf(riga, "%ld %x %x %x", &fr, &a, &b, &c) == 4 && fr >= 0 && fr < MAX_FRAME_FILE) {
            riprodotti[fr][0] = (uint8_t)a; riprodotti[fr][1] = (uint8_t)b;
            riprodotti[fr][2] = (uint8_t)c;
        }
    }
    fclose(f);
}

/* Comandi da tastiera (solo con finestra): pausa e reset. 1 = esci. */
static int comandi(void)
{
    const uint8_t *k = SDL_GetKeyboardState(NULL);
    if (k[SDL_SCANCODE_F2] && !file_riproduci) {
        if (registra) fclose(registra);
        execv(argv_orig[0], argv_orig);
        perror("reset (execv)");       /* se execv fallisce si continua */
        registra = NULL;
    }
    int p = k[SDL_SCANCODE_P];
    int pausa = p && !p_prec;
    p_prec = p;
    while (pausa) {                    /* ripresenta l'ultimo frame, nessuna NMI */
        int r = vsdl_presenta(reso);
        if (r) { if (r < 0) esito_sdl = 2; return 1; }
        k = SDL_GetKeyboardState(NULL);
        p = k[SDL_SCANCODE_P];
        if (p && !p_prec) pausa = 0;
        p_prec = p;
    }
    return 0;
}

/* Resa del frame con la RAM prima della NMI; registrazione; hash; finestra.
 * Ritorna 1 per uscire. */
static int passo(long frame, const uint8_t v[3])
{
    int16_t pcm[SCHEDA_CAMPIONI_FRAME];
    scheda_audio_frame(frame);
    int n_pcm = scheda_audio_pcm(pcm);
    if (n_pcm && audio_aperto) gioca_audio_spingi(pcm, n_pcm);
    if (registra)
        fprintf(registra, "%ld %02X %02X %02X\n", frame, v[0], v[1], v[2]);
    vid_ram vr = { &ram[0xd000 - 0xc000], &ram[0xd400 - 0xc000], &ram[0xd800 - 0xc000],
                   &ram[0xdc00 - 0xc000], &ram[0xe000 - 0xc000], &ram[0xe400 - 0xc000] };
    rendi_frame(&vr, reso);
    hash_ultimo = vsdl_hash(reso);
    frame_ultimo = frame;
    rese++;
    for (int i = 0; i < n_hash_frame; i++)
        if (hash_frame[i] == frame)
            printf("HASH %ld %016llx\n", frame, (unsigned long long)hash_ultimo);
    if (opz.headless)
        return rese >= opz.headless;
    int r = vsdl_presenta(reso);
    if (r) { if (r < 0) esito_sdl = 2; return 1; }
    return opz.massimo && rese >= opz.massimo;
}

/* Modo libero: ingressi da tastiera/joystick o dal file. */
static int frame_libero(long frame, uint8_t v[3])
{
    if (file_riproduci) {
        if (frame >= 0 && frame < MAX_FRAME_FILE)
            memcpy(v, riprodotti[frame], 3);
    } else if (finestra) {
        tasti_ingressi(SDL_GetKeyboardState(NULL), v);
        joy_ingressi(v);
    }
    if (finestra && comandi())
        return 1;
    return passo(frame, v);
}

/* Modo oracolo: gli ingressi li ha già scritti macchina.c in ram. */
static void frame_oracolo(long frame)
{
    uint8_t v[3] = { ram[0xe600 - 0xc000], ram[0xe601 - 0xc000], ram[0xe602 - 0xc000] };
    if ((finestra && comandi()) || passo(frame, v)) {
        finale();
        exit(esito_sdl);
    }
}

static int numero(const char *s, long *out)
{
    char *fine;
    *out = strtol(s, &fine, 10);
    return *s && !*fine;
}

int main(int argc, char **argv)
{
    char *av[128];
    int ac = 0, stampa_dsw = 0, mappa = 0, dsw1_raw = -1, dsw2_raw = -1;
    uint8_t d1 = 0x02, d2 = 0x00;
    static const char *dsw_opz[] = { "vite", "cabinet", "demo", "bonus", "difficolta",
                                     "timer", "extra", NULL };
    argv_orig = argv;
    if (argc > 128) { fprintf(stderr, "ERRORE: troppi argomenti\n"); return 2; }
    for (int i = 0; i < argc; i++) {
        const char *a = argv[i];
        int dsw = 0;
        if (i == 0) { av[ac++] = argv[i]; continue; }
        if (!strncmp(a, "--", 2))
            for (int k = 0; dsw_opz[k]; k++)
                if (!strcmp(a + 2, dsw_opz[k])) dsw = 1;
        int extra = gioca_opzione_extra(argc, argv, &i);
        if (extra < 0) return 2;
        if (extra) continue;
        if (!strcmp(a, "--mappa")) mappa = 1;
        else if (!strcmp(a, "--muto")) muto = 1;
        else if (!strcmp(a, "--stampa-dsw")) stampa_dsw = 1;
        else if (!strcmp(a, "--oracolo")) {
            if (i + 2 >= argc) { fprintf(stderr, "ERRORE: --oracolo vuole oracolo e disp\n"); return 2; }
            orac_oracolo = argv[++i]; orac_disp = argv[++i];
        } else if (dsw || !strcmp(a, "--flusso-ay") || !strcmp(a, "--registra-latch") ||
                   !strcmp(a, "--fase-audio") || !strcmp(a, "--registra") || !strcmp(a, "--riproduci") ||
                   !strcmp(a, "--hash-frame") || !strcmp(a, "--dsw1") || !strcmp(a, "--dsw2")) {
            if (i + 1 >= argc) { fprintf(stderr, "ERRORE: %s vuole un valore\n", a); return 2; }
            const char *val = argv[++i];
            if (dsw) {
                if (dsw_applica(a + 2, val, &d1, &d2) != 0) {
                    fprintf(stderr, "ERRORE: valore \"%s\" non valido per %s\n", val, a);
                    return 2;
                }
            } else if (!strcmp(a, "--flusso-ay")) file_flusso_ay = val;
            else if (!strcmp(a, "--registra-latch")) file_registra_latch = val;
            else if (!strcmp(a, "--fase-audio")) {
                if (strcmp(val, "0") && strcmp(val, "1") && strcmp(val, "2")) {
                    fprintf(stderr, "ERRORE: --fase-audio vuole 0, 1 o 2\n");
                    return 2;
                }
                scheda_audio_fase = atoi(val);
            } else if (!strcmp(a, "--registra")) file_registra = val;
            else if (!strcmp(a, "--riproduci")) file_riproduci = val;
            else if (!strcmp(a, "--hash-frame")) {
                char copia[512], *s;
                snprintf(copia, sizeof copia, "%s", val);
                s = strtok(copia, ",");
                for (; s && n_hash_frame < MAX_HASH; s = strtok(NULL, ","))
                    if (!numero(s, &hash_frame[n_hash_frame++])) {
                        fprintf(stderr, "ERRORE: frame \"%s\" non valido\n", s);
                        return 2;
                    }
            } else {
                char *fine;
                long x = strtol(val, &fine, 16);
                if (!*val || *fine || x < 0 || x > 255) {
                    fprintf(stderr, "ERRORE: %s vuole un byte esadecimale\n", a);
                    return 2;
                }
                if (!strcmp(a, "--dsw1")) dsw1_raw = (int)x; else dsw2_raw = (int)x;
            }
        } else
            av[ac++] = argv[i];
    }
    if (mappa) { tasti_stampa_mappa(); return 0; }
    if (dsw1_raw >= 0) d1 = (uint8_t)dsw1_raw;
    if (dsw2_raw >= 0) d2 = (uint8_t)dsw2_raw;
    if (stampa_dsw) { printf("DSW1 %02X DSW2 %02X\n", d1, d2); return 0; }
    if (gioca_extra_valida() != 0) return 2;
    if (gx.joystick) return joy_diagnostica(gx.tempo);
    int k = vsdl_leggi_opzioni(ac, av, &opz);
    if (k < 0) return 2;
    if (k != ac) {
        fprintf(stderr, "uso: gioca [opzioni] — vedi gioca --aiuto\n");
        return 2;
    }
    if (orac_oracolo && !file_riproduci) {
        fprintf(stderr, "ERRORE: --oracolo richiede --riproduci\n");
        return 2;
    }
    macchina_dsw1 = d1; macchina_dsw2 = d2;
    if (file_riproduci) carica_riprodotti(file_riproduci);
    if (file_registra) {
        registra = fopen(file_registra, "w");
        if (!registra) { perror(file_registra); return 2; }
    }
    if (!opz.headless) {
        vsdl_pieno(gx.pieno);
        vsdl_scanlines(gx.scanlines);
        if (vsdl_apri("Solomon's Key - gioca", opz.scala) != 0) return 2;
        finestra = 1;
        if (!gx.senza_joy) joy_apri();
        gioca_audio_volume(gx.volume);
        if (!muto && gioca_audio_apri() == 0) audio_aperto = 1;
    }
    scheda_audio_apri(file_flusso_ay, file_registra_latch);
    macchina_latch_audio = scheda_audio_latch;

    int rc;
    macchina_prima_nmi = NULL;
    if (orac_oracolo) {
        macchina_prima_nmi = frame_oracolo;
        giro_dal_boot = 1;             /* come gioco_libero: accende i punti PUNTO_GIRO */
        rc = macchina_esegui(orac_oracolo, orac_disp, file_riproduci, -1);
    } else {
        libera_frame = frame_libero;
        rc = macchina_libera_esegui();
    }
    finale();
    return rc ? rc : esito_sdl;
}
