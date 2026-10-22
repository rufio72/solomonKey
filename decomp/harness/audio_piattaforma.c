/* Piattaforma del Z80 audio: memoria, caricamento di 1.3jk, porte dei tre
 * YM2149 con la stessa semantica dell'oracolo audio.lua e scrittura del
 * flusso dei cambi nello stesso formato di out/audio/<nome>_ay.txt.
 * Il flusso è un'interfaccia stabile: un lotto futuro lo darà a un
 * emulatore YM2149 per generare i .wav dei codici. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"

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

/* Carica 1.3jk da ../roms/ext/solomon/ o ../../roms/ext/solomon/ (da
 * decomp/ o da decomp/harness/); 16384 byte o exit(2). */
void aud_carica_rom(void)
{
    const char *nomi[] = { "../roms/ext/solomon/1.3jk",
                           "../../roms/ext/solomon/1.3jk" };
    for (unsigned i = 0; i < 2; i++) {
        FILE *f = fopen(nomi[i], "rb");
        if (!f) continue;
        size_t n = fread(aud_rom, 1, sizeof aud_rom, f);
        int in_piu = fgetc(f) != EOF;
        fclose(f);
        if (n != sizeof aud_rom || in_piu) {
            fprintf(stderr, "%s: lunghezza sbagliata (attesi 16384 byte)\n", nomi[i]);
            exit(2);
        }
        return;
    }
    fprintf(stderr, "1.3jk non trovato in ../roms/ext/solomon/ né in ../../roms/ext/solomon/\n");
    exit(2);
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
