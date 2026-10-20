/* Registro delle routine tradotte per il banco di prova (routine_tab.c)
 * e prototipi delle routine dei lotti (z80_*.c). */
#ifndef ROUTINE_H
#define ROUTINE_H

#include "ram.h"

struct voce_routine {
    const char *entry;                 /* "0x05B3", maiuscolo come nei file */
    void (*fn)(struct z80 *);
    uint16_t blocco_ini, blocco_fine;  /* 0, 0 = nessun blocco esplicito */
};
extern const struct voce_routine tabella_routine[];
extern const unsigned num_routine;

/* Lotto 1 (compito 05). */
void sub_0008(struct z80 *z);
void sub_0010(struct z80 *z);
void sub_026c(struct z80 *z);
void sub_237e(struct z80 *z);
void sub_23a1(struct z80 *z);
void sub_2418(struct z80 *z);
void sub_249e(struct z80 *z);
void sub_24b1(struct z80 *z);
void sub_30ef(struct z80 *z);
void sub_3a1c(struct z80 *z);
void sub_3f10(struct z80 *z);
void sub_4af6(struct z80 *z);
void sub_4b0e(struct z80 *z);
void sub_5c78(struct z80 *z);
void sub_673f(struct z80 *z);

/* Lotto 2 (compito 06). */
void sub_0018(struct z80 *z);
void sub_2368(struct z80 *z);
void sub_23ac(struct z80 *z);
void sub_23f8(struct z80 *z);
void sub_240b(struct z80 *z);
void sub_2457(struct z80 *z);
void sub_34d4(struct z80 *z);
void sub_380a(struct z80 *z);
void sub_3a2e(struct z80 *z);
void sub_4722(struct z80 *z);
void sub_4dee(struct z80 *z);
void sub_4e31(struct z80 *z);
void sub_4e87(struct z80 *z);
void sub_6771(struct z80 *z);
void sub_7efe(struct z80 *z);

#endif
