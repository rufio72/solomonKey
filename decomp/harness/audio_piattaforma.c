/* Piattaforma del Z80 audio: memoria, immagine di 1.3jk ricomposta da
 * tecmo/, porte dei tre YM2149 con la stessa semantica dell'oracolo audio.lua e scrittura del
 * flusso dei cambi nello stesso formato di out/audio/<nome>_ay.txt.
 * Il flusso è un'interfaccia stabile: un lotto futuro lo darà a un
 * emulatore YM2149 per generare i .wav dei codici. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "audio_dati.h"
#include "audio_driver.h"

uint8_t aud_ram[0x800];
uint8_t aud_rom[0x4000];
int aud_frame;
uint8_t aud_latch;
void (*aud_osservatore)(int chip, int reg, int val);

/* registro selezionato per chip (-1 = nessuno) e ombra dei registri
 * (-1 = mai scritto); indice 1-3, il registro selezionato è il byte intero
 * come in audio.lua (nessuna maschera: senza effetto, il driver seleziona
 * solo 00-0D) */
static int selezionato[4];
static int ombra[4][256];
static unsigned ignote[256];
static FILE *flusso;

_Static_assert(sizeof aud_driver == AUD_DATI_INIZIO, "driver 0000-05C5");

/* Ricompone l'immagine di 1.3jk senza leggerla: regione dati 05C6-3FFF
 * dalle tabelle (audio_dati_*.c, che prima riempiono tutto di 0xAA) e
 * codice del driver 0000-05C5 dal blob di tecmo/audio_driver.c. */
void aud_carica_rom(void)
{
    aud_dati_serializza(aud_rom);
    memcpy(aud_rom, aud_driver, sizeof aud_driver);
}

/* Origine dei dati 05C6-3FFF (dopo aud_carica_rom): modo 1 = immagine
 * ricomposta dalle tabelle estratte (audio_dati_*.c), gli stessi byte che
 * aud_carica_rom ha già messo; modo 2 = controllo negativo, regione tutta
 * 0xAA; altrimenti la regione resta quella delle tabelle. Il codice del
 * driver (0000-05C5) resta sempre quello del blob di tecmo/. */
void aud_usa_dati_estratti(int modo)
{
    static uint8_t img[0x4000];
    if (modo == 1) {
        aud_dati_serializza(img);
        memcpy(aud_rom + AUD_DATI_INIZIO, img + AUD_DATI_INIZIO,
               AUD_DATI_FINE - AUD_DATI_INIZIO);
    } else if (modo == 2) {
        memset(aud_rom + AUD_DATI_INIZIO, 0xAA, AUD_DATI_FINE - AUD_DATI_INIZIO);
    }
}

/* Stato iniziale: RAM a zero (MAME la azzera all'avvio), nessun registro
 * selezionato, ombre mai scritte, nessuna porta ignota. */
void aud_azzera(void)
{
    memset(aud_ram, 0, sizeof aud_ram);
    for (int c = 0; c < 4; c++) {
        selezionato[c] = -1;
        for (int r = 0; r < 256; r++) ombra[c][r] = -1;
    }
    memset(ignote, 0, sizeof ignote);
    aud_frame = 0;
    aud_latch = 0;
}

/* Lettura nello spazio del Z80 audio: 8000 è il latch, il resto fuori da
 * ROM e RAM vale 00 in MAME (dedotto dal comando 40, la cui voce in 0AF3
 * punta a 7A09: in cod_40_ram.txt il blocco 0 è scritto 11436 volte, una
 * per terna fino a FFFF, e il byte del latch a 8000 tocca 46E0). */
uint8_t aud_leggi(uint16_t a)
{
    if (a < 0x4000) return aud_rom[a];
    if (a < AUD_RAM + AUD_RAM_LUNG) return AM(a);
    if (a == AUD_LATCH) return aud_latch;
    return 0x00;
}

/* Scrittura nello spazio del Z80 audio: fuori dalla RAM non ha effetto
 * (ROM, zone non mappate, il nop a ffff). */
void aud_scrivi(uint16_t a, uint8_t v)
{
    if (a >= AUD_RAM && a < AUD_RAM + AUD_RAM_LUNG) AM(a) = v;
}

/* out (c),r: chip = porta >> 4; porte fuori da x0/x1 dei chip 1-3 contate
 * come ignote (12 e 13 per l'errore di 00A4); pari = selezione, dispari =
 * dato, riga nel flusso solo se il valore cambia. */
void aud_out(uint8_t porta, uint8_t v)
{
    int chip = porta >> 4;
    if (chip < 1 || chip > 3 || (porta & 0x0f) > 1) {
        ignote[porta]++;
        return;
    }
    if ((porta & 1) == 0) {
        selezionato[chip] = v;
        return;
    }
    int reg = selezionato[chip];
    if (reg < 0) {
        ignote[porta]++;                       /* dato senza registro selezionato */
        return;
    }
    if (ombra[chip][reg] != v) {
        ombra[chip][reg] = v;
        if (flusso)
            fprintf(flusso, "%d\t%X\t%02X\t%02X\n", aud_frame, chip, reg, v);
        if (aud_osservatore) aud_osservatore(chip, reg, v);
    }
}

int aud_ombra(int chip, int reg)
{
    return ombra[chip][reg & 0xff];
}

unsigned aud_ignote(uint8_t porta)
{
    return ignote[porta];
}

/* Apre il flusso dei cambi con l'intestazione di audio.lua. */
int aud_flusso_apri(const char *percorso)
{
    flusso = fopen(percorso, "w");
    if (!flusso) return 0;
    fputs("# frame\tchip\treg\tval\n", flusso);
    return 1;
}

/* Riga «# codice <frame> <cc>» scritta da audio.lua prima di un codice
 * forzato nel latch. */
void aud_flusso_codice(int frame, uint8_t cc)
{
    if (flusso) fprintf(flusso, "# codice %d %02X\n", frame, cc);
}

void aud_flusso_chiudi(void)
{
    if (flusso) fclose(flusso);
    flusso = NULL;
}
