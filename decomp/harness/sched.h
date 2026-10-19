/* Spedidore delle attività del Z80 principale (3AF3-3B10) e primitiva di
 * posta (0493), modello a funzioni: niente coroutine, niente pile separate.
 * Lo stato sta in RAM come nell'originale:
 *   c000-c03f  tabella delle attività, un codice per slot (0 = vuoto)
 *   c04f       indice del prossimo slot da esaminare (spedidore)
 *   c04e       indice di ricerca dello slot libero (0493), senza maschera */
#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include "ram.h"

#define SCHED_SLOT 64
#define SCHED_CODICI 40                  /* voci della tabella 3B11 */
#define tabella (&M(0xc000))             /* 64 byte */
#define indice M(0xc04f)
#define prossimo_libero M(0xc04e)

/* Corpo di un'attività: riceve lo slot (il puntatore che l'originale lascia
 * sulla pila a 3B07). Ritornare equivale a `pop hl; ret` verso 3AF3;
 * `pop hl; ld (hl),n; ret` diventa `tabella[slot] = n; return`. */
typedef void (*attivita_fn)(uint8_t slot);

/* Tabella 3B11 in C: codice -> funzione. Per ora sono tutte stub. */
extern attivita_fn corpo_attivita[SCHED_CODICI];

/* Gancio chiamato da ogni stub con codice e slot (lo imposta chi prova lo
 * spedidore, es. sched_proto; NULL = lo stub ritorna e basta). */
extern void (*sched_gancio_stub)(uint8_t codice, uint8_t slot);

/* Gancio chiamato dallo spedidore prima di ogni lettura della scansione
 * (lettura = 0 per la prima): è il punto in cui la prova inietta gli effetti
 * della NMI (posta 0267, 042B, 0465, 0513) misurati a metà scansione. NULL = nulla. */
extern void (*sched_gancio_scansione)(int lettura);

/* 0493: scrive codice nel primo slot libero a partire da c04e & 3f
 * (c04e cresce di uno a ogni slot esaminato) e restituisce lo slot;
 * 0xff se la tabella è piena (l'originale girerebbe per sempre). */
uint8_t posta_attivita(uint8_t codice);

/* 3AF3-3B10: cerca da c04f il primo slot non nullo (ciclico su 64), porta
 * c04f allo slot successivo, chiama corpo_attivita[codice](slot).
 * Restituisce 1 dopo la chiamata, 0 se la tabella è vuota (l'originale
 * girerebbe per sempre), -1 se il codice non è nella tabella 3B11. */
int spedisci_una(uint8_t *slot, uint8_t *codice);

#endif
