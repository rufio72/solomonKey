/* Spedidore 3AF3 e posta 0493 tradotti uno a uno (vedi SCHEDULER.md). */
#include <stddef.h>
#include "sched.h"

void (*sched_gancio_stub)(uint8_t codice, uint8_t slot) = NULL;
void (*sched_gancio_scansione)(int lettura) = NULL;

uint8_t posta_attivita(uint8_t codice)
{
    /* 0493 ld hl,c04e; ld d,a */
    for (int giri = 0; giri < 256; giri++) {
        /* 0496 ld a,3f; and (hl); inc (hl): c04e cresce senza maschera */
        uint8_t i = prossimo_libero & 0x3f;
        prossimo_libero++;
        /* se lo slot è occupato si ripete */
        if (tabella[i] != 0)
            continue;
        tabella[i] = codice;          /* 04AA ld (hl),d */
        return i;
    }
    return 0xff;
}

int spedisci_una(uint8_t *slot, uint8_t *codice)
{
    uint8_t l = indice;               /* 3AF3 ld a,(c04f); ld l,a; ld h,c0 */
    uint8_t a;
    /* 3AF9 xor a; 3AFA or (hl); jr nz; inc l; res 6,l */
    for (int lettura = 0;; lettura++) {
        if (sched_gancio_scansione)
            sched_gancio_scansione(lettura);
        a = M(0xc000 + l);
        if (a != 0)
            break;
        l = (uint8_t)((l + 1) & ~0x40);
        if (lettura > 4096)
            return 0;
    }
    *slot = l;                        /* 3B07 push hl */
    *codice = a;                      /* 3B02 ld b,a */
    indice = (uint8_t)((l + 1) & ~0x40);   /* 3B08 inc l; res 6,l; ld (c04f),a */
    if (a >= SCHED_CODICI)            /* 3B10 rst 20h fuori tabella */
        return -1;
    corpo_attivita[a](l);             /* il ritorno torna a 3AF3 (push de) */
    return 1;
}
