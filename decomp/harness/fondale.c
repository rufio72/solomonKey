/* 0x326C: decodificatore del fondale (layer BG) di Solomon's Key, in C leggibile.
 * Riproduce l'effetto sulla RAM e i registri A, BC, DE, HL all'uscita
 * (le bandiere F no). Il flusso dei comandi sta in ROM f000-ffff, che nel
 * banco di prova è dentro ram[] (la cattura copre c000-ffff).
 *
 * Ingresso: DE = flusso, BC = parametro di partenza:
 *   B bit 0-4 larghezza della riga (in blocchi, più uno),
 *   B bit 6   la colonna di partenza è a destra (C&0x0f + larghezza - 2),
 *   B bit 7   righe verso l'alto (bit 0 della riga, anche ribaltamento Y),
 *   C bit 4-7 riga di partenza, C bit 0-3 colonna di partenza.
 * Ogni byte del flusso ha il tipo nei bit 0-1:
 *   0  blocco 2x2 con codice base c&0xfc nella cella del cursore;
 *   1  attributi (0x333C, o 0x23CB se il bit 7 è 1);
 *   2  inizio ripetizione (0x335A): il gruppo che segue si ripete;
 *   3  fine del flusso (0x336A). */
#include <stdio.h>
#include "ram.h"

/* Variabili del decodificatore (c082-c089). */
#define RIGA            0xc082  /* riga << 1 | verso (1 = su, ribaltamento Y) */
#define COLONNA         0xc083  /* colonna << 1 | verso (1 = sinistra, ribaltamento X) */
#define COLONNA_INIZIO  0xc084  /* colonna di inizio riga, stesso formato */
#define CONTATORE       0xc085  /* blocchi che mancano al cambio di riga */
#define LARGHEZZA       0xc086  /* valore di ricarica del contatore, più uno */
#define ATTRIBUTI       0xc087  /* attributi con i nibble scambiati */
#define RIBALTA_UNA     0xc088  /* ribaltamenti per il solo blocco successivo */
#define RIPETIZIONE     0xc089  /* bit 4-7 giri, bit 2-3 ricarica, bit 0-1 contatore */

/* Piani BG: videoram dc00, colorram d800; il fondale parte dalla riga 4 colonna 1. */
#define BG_VIDEO_ORIGINE 0xdc80
#define DISTANZA_COLORE  0x0400

/* 0x23BB: inizializza il cursore da HL = COLONNA in poi. A e il riporto
 * danno la colonna (rla), E la larghezza; attributi e ripetizione a zero. */
void sub_23bb(struct z80 *z, int riporto)
{
    uint8_t colonna = (uint8_t)((z->a << 1) | riporto);
    z->a = 0;
    M(COLONNA) = colonna;
    M(COLONNA_INIZIO) = colonna;
    M(CONTATORE) = z->e;
    M(LARGHEZZA) = z->e;
    M(ATTRIBUTI) = 0;
    M(RIBALTA_UNA) = 0;
    M(RIPETIZIONE) = 0;
    SET_HL(z, RIPETIZIONE);
}

/* 0x23CB: attributi «estesi» (byte con il bit 7 a 1): bit 4-6 del comando
 * nel banco colore, bit 2-3 nei bit alti del codice, conservati i bit 3 e 7. */
void sub_23cb(struct z80 *z)
{
    z->b = (uint8_t)((z->c & 0x70) | ((z->c & 0x0c) >> 2));
    z->a = (uint8_t)((M(HL(z)) & 0x88) | z->b);
    M(HL(z)) = z->a;
}

/* 0x23EC: HL = D * 64 (riga di blocchi -> spiazzamento di due righe di tile). */
uint16_t sub_23ec(uint8_t riga)
{
    return (uint16_t)(riga * 64);
}

/* 0x336C: disegna il blocco 2x2 (codici base+0..3, scambiati dai
 * ribaltamenti) in videoram e colorram BG. Fuori dalla griglia 15x12 non
 * disegna. Restituisce 1 se ha disegnato (all'uscita B vale 0). */
int sub_336c(uint8_t base, uint8_t attributi, uint8_t riga, uint8_t colonna)
{
    if (riga > 11 || colonna > 14)
        return 0;
    uint16_t hl = (uint16_t)(BG_VIDEO_ORIGINE + sub_23ec(riga) + 2 * colonna + 1);
    for (int i = 0; i < 4; i++) {
        uint8_t indice = (uint8_t)i;
        if (attributi & 0x08) indice ^= 2;   /* ribaltamento Y: scambia le righe */
        if (attributi & 0x80) indice ^= 1;   /* ribaltamento X: scambia le colonne */
        M(hl) = (uint8_t)(base + indice);
        M(hl - DISTANZA_COLORE) = attributi;
        hl++;
        if (i == 1) hl += 30;                /* seconda riga del blocco */
    }
    return 1;
}

/* Avanza di uno nel verso dato dal bit 0 (1 = indietro), come 32AF-32B9. */
static uint8_t passo(uint8_t valore, int indietro)
{
    return indietro ? (uint8_t)(valore - 1) : (uint8_t)(valore + 1);
}

void sub_326c(struct z80 *z)
{
    uint16_t flusso = DE(z);
    uint8_t b = z->b, c = z->c;

    /* 326C-328B: posizione e verso iniziali */
    uint8_t larghezza = b & 0x1f;
    M(RIGA) = (uint8_t)(((c >> 4) << 1) | (b >> 7));
    int a_destra = (b >> 6) & 1;
    b = (uint8_t)((b << 2) | (((c >> 4) & 1) << 1));   /* B dopo i due rl b */
    z->e = larghezza;
    if (a_destra)
        z->a = (uint8_t)((c & 0x0f) + larghezza - 2);
    else
        z->a = c & 0x0f;
    sub_23bb(z, a_destra);

    /* Pila locale delle ripetizioni: 335A spinge DE sotto il ritorno a 328F,
     * 3320 lo rilegge (pop de; push de), 3327-3329 lo scarta. */
    uint16_t pila[16];
    int livello = 0;

    for (;;) {
        c = M(flusso);                     /* 328F */
        flusso++;
        uint8_t tipo = c & 3;
        if (tipo != 0) {
            /* 332D: rst $20 su tipo-1, tabella rst20_3335 = {333C, 335A, 336A} */
            switch (tipo - 1) {
            case 0:                        /* 333C: attributi */
                if (c & 0x80) {
                    z->c = c;
                    SET_HL(z, ATTRIBUTI);
                    sub_23cb(z);
                    b = z->b;
                } else {
                    b = c & 0x70;
                    M(ATTRIBUTI) = (uint8_t)((M(ATTRIBUTI) & 0x8b) | b);
                    uint8_t r = 0;
                    if (c & 0x04) r |= 0x80;
                    if (c & 0x08) r |= 0x08;
                    M(RIBALTA_UNA) = r;
                }
                break;
            case 1:                        /* 335A: inizio ripetizione */
                c &= 0xfc;
                M(RIPETIZIONE) = (uint8_t)(c | ((c & 0x0c) >> 2));
                if (livello < 16)
                    pila[livello++] = flusso;
                break;
            default:                       /* 336A: fine del flusso */
                if (livello)
                    fprintf(stderr, "326C: fine flusso con %d ripetizioni aperte\n", livello);
                z->a = 2;
                z->b = b;
                z->c = c;
                SET_DE(z, flusso);
                SET_HL(z, 0x328f);
                return;
            }
            continue;
        }

        /* 3297: blocco con codice base c & 0xfc nella cella del cursore */
        uint8_t base = c & 0xfc;
        uint8_t attributi = 0;
        uint8_t riga = M(RIGA) >> 1;
        int su = M(RIGA) & 1;
        if (su) attributi |= 0x08;
        int sinistra = M(COLONNA) & 1;
        uint8_t colonna = M(COLONNA) >> 1;
        if (sinistra) attributi |= 0x80;
        M(COLONNA) = (uint8_t)((passo(colonna, sinistra) << 1) | sinistra);

        M(CONTATORE)--;
        if (M(CONTATORE) == 0) {
            /* 32C2: a capo, il blocco corrente va già sulla riga nuova */
            riga = passo(riga, su);
            M(RIGA) = (uint8_t)((riga << 1) | su);
            uint8_t inizio = M(COLONNA_INIZIO);
            int verso = inizio & 1;
            /* rra con il riporto lasciato dal rla della riga */
            colonna = (uint8_t)((riga & 0x80) | (inizio >> 1));
            M(COLONNA) = (uint8_t)((passo(colonna, verso) << 1) | verso);
            M(CONTATORE) = (uint8_t)(M(LARGHEZZA) - 1);
        }

        /* 32E6: attributi correnti (nibble scambiati) e ribaltamenti */
        uint8_t a = M(ATTRIBUTI);
        a = (uint8_t)((a >> 4) | (a << 4));
        attributi ^= a ^ M(RIBALTA_UNA);
        M(RIBALTA_UNA) = 0;
        if (sub_336c(base, attributi, riga, colonna))
            b = 0;
        else
            b = base;

        /* 32F8: ripetizione */
        uint8_t rip = M(RIPETIZIONE);
        if ((rip & 0x0c) == 0 && (rip & 0xf0) == 0)
            continue;
        uint8_t ricarica = rip & 0x0c;
        ricarica |= ricarica >> 2;
        b = ricarica;
        if (rip & 3) {
            M(RIPETIZIONE) = (uint8_t)(rip - 1);
            continue;
        }
        uint8_t giri = (uint8_t)((rip & 0xf0) - 0x10);
        if (giri == 0) {                   /* 3327: ultimo giro, si scarta DE */
            M(RIPETIZIONE) = 0;
            if (livello) livello--;
            continue;
        }
        if (livello)                       /* 3320: si torna all'inizio del gruppo */
            flusso = pila[livello - 1];
        M(RIPETIZIONE) = (uint8_t)(giri | ricarica);
    }
}
