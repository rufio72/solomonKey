/* Motore dei nemici (corpo dell'attività 0A e routine collegate), lotti
 * N1-N12 di CENSIMENTO-NEMICI.md. Prototipi delle routine tradotte, degli
 * stub (nemici_stub.c) e meccanismo che ferma il replay quando il C
 * raggiunge una routine non ancora tradotta o un indice fuori tabella.
 *
 * Convenzioni come in routine.h: `void sub_xxxx(struct z80 *z)`, i
 * registri all'uscita sono quelli del codice originale; il riporto, dove il
 * chiamante lo legge, sta nel bit 0 di z->f (come sub_5507 e sub_7379). */
#ifndef NEMICI_H
#define NEMICI_H

#include <stdint.h>
#include "ram.h"

/* Bit del riporto in z->f. */
#define NEMICI_RIPORTO 0x01
#define NEMICI_PONI_RIPORTO(z, c) \
    ((z)->f = (uint8_t)(((z)->f & ~NEMICI_RIPORTO) | ((c) ? NEMICI_RIPORTO : 0)))
#define NEMICI_RIPORTO_ACCESO(z) (((z)->f & NEMICI_RIPORTO) != 0)

/* Prima routine non tradotta raggiunta (0 = nessuna) e sito rst fuori tabella. */
extern uint16_t nemici_mancante;
extern uint16_t nemici_fuori_sito;
extern uint8_t nemici_fuori_indice;
void nemici_manca(uint16_t ingresso);      /* registra solo la prima */
void nemici_fuori_tabella(uint16_t sito, uint8_t idx);

/* `rst 38h` al sito (0038-0042): salto a tab[E], tabella subito dopo il
 * `rst`. Restituisce la destinazione e lascia DE come l'originale
 * (indirizzo del byte alto della voce, sito + 2 + 2E); HL e A intatti.
 * Con E oltre la tabella registra il sito e restituisce 0. */
static inline uint16_t nemici_rst38(struct z80 *z, uint16_t sito,
                                    const uint16_t *tab, unsigned voci)
{
    uint8_t e = z->e;
    if (e >= voci) {
        nemici_fuori_tabella(sito, e);
        return 0;
    }
    SET_DE(z, (uint16_t)(sito + 2 + 2 * e));
    return tab[e];
}

/* `rst 20h` al sito (0020-0026): salto a tab[A], HL, DE e A conservati.
 * Con A oltre la tabella registra il sito e restituisce 0. */
static inline uint16_t nemici_rst20(struct z80 *z, uint16_t sito,
                                    const uint16_t *tab, unsigned voci)
{
    if (z->a >= voci) {
        nemici_fuori_tabella(sito, z->a);
        return 0;
    }
    return tab[z->a];
}

/* Corpo dell'attività 0A (nemici.c). */
void attivita_0a_4da2(uint8_t slot);

/* 4B98, chiusura dell'oggetto IX (attivita.c). */
void sub_4b98(uint16_t ix);

/* Lotto N1, nemici.c (4DAD, 4632, 4EE6 sono statiche). */
void sub_4f0f(struct z80 *z);
void sub_3b62(struct z80 *z);

/* Lotto N1, nemici_comuni.c. */
void sub_2482(struct z80 *z);
void sub_248d(struct z80 *z);
void sub_24be(struct z80 *z);
void sub_4592(struct z80 *z);
void sub_4938(struct z80 *z);
void sub_49ae(struct z80 *z);
void sub_4b20(struct z80 *z);
void sub_4c11(struct z80 *z);

/* Lotto N1, nemici_tipi_10_13.c. */
void sub_55b2(struct z80 *z);
void sub_55c9(struct z80 *z);
void sub_561b(struct z80 *z);
void sub_5640(struct z80 *z);
void sub_5693(struct z80 *z);
void sub_714b(struct z80 *z);
void sub_7155(struct z80 *z);
void sub_7235(struct z80 *z);

/* Lotto N1, nemici_tipo09.c. */
void sub_58cf(struct z80 *z);
void sub_5917(struct z80 *z);
void sub_5928(struct z80 *z);

/* Lotto N1, nemici_comparsa.c. */
void sub_734d(struct z80 *z);

/* Lotto N1, nemici_tipi_15_17.c. */
void sub_7f51(struct z80 *z);
void sub_7f68(struct z80 *z);

/* Stub (nemici_stub.c): registrano l'ingresso con nemici_manca. */
void sub_2474(struct z80 *z);
void sub_4649(struct z80 *z);
void sub_46a3(struct z80 *z);
void sub_4ada(struct z80 *z);
void sub_4f40(struct z80 *z);
void sub_4fd4(struct z80 *z);
void sub_54d8(struct z80 *z);
void sub_55c4(struct z80 *z);
void sub_56e9(struct z80 *z);
void sub_57cd(struct z80 *z);
void sub_5cfb(struct z80 *z);
void sub_6b43(struct z80 *z);
void sub_6c0b(struct z80 *z);
void sub_6c1d(struct z80 *z);
void sub_6ce8(struct z80 *z);
void sub_6eca(struct z80 *z);
void sub_719e(struct z80 *z);
void sub_7288(struct z80 *z);
void sub_73a3(struct z80 *z);
void sub_7f48(struct z80 *z);
void sub_7f81(struct z80 *z);
void sub_7f8a(struct z80 *z);
void sub_7fa6(struct z80 *z);
void sub_7fcf(struct z80 *z);
void sub_801d(struct z80 *z);
void sub_8042(struct z80 *z);
void sub_809c(struct z80 *z);

#endif
