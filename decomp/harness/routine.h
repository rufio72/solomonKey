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

/* Lotto 3 (compito 07). */
void sub_01c5(struct z80 *z);
void sub_07de(struct z80 *z);
void sub_2b08(struct z80 *z);
void sub_3000(struct z80 *z);
void sub_3101(struct z80 *z);
void sub_31ce(struct z80 *z);
void sub_325e(struct z80 *z);
void sub_33aa(struct z80 *z);
void sub_33ba(struct z80 *z);
void sub_34e1(struct z80 *z);
void sub_36bd(struct z80 *z);
void sub_452b(struct z80 *z);
void sub_472e(struct z80 *z);
void sub_4d37(struct z80 *z);
void sub_4d87(struct z80 *z);

/* Lotto 4 (compito 08). */
void sub_004f(struct z80 *z);
void sub_2af5(struct z80 *z);
void sub_2ff3(struct z80 *z);
void sub_3097(struct z80 *z);
void sub_31e9(struct z80 *z);
void sub_349f(struct z80 *z);
void sub_3678(struct z80 *z);
void sub_36c3(struct z80 *z);
void sub_36f0(struct z80 *z);
void sub_4398(struct z80 *z);
void sub_4d05(struct z80 *z);
void sub_7e32(struct z80 *z);

/* Lotto 5 (compito 09). */
void sub_3045(struct z80 *z);
void sub_5c32(struct z80 *z);
void sub_7e1d(struct z80 *z);
void sub_7e24(struct z80 *z);

/* Lotto 6 (compito 10). */
void sub_2f74(struct z80 *z);
void sub_7dea(struct z80 *z);

/* Lotto 7 (compito 11): il grappolo 0602 (motore per-oggetto della NMI).
 * Lo smistatore 08CE non ha prototipo: è il salto in tabella dentro
 * sub_08a2 (z80_08xx.c). */
void sub_0602(struct z80 *z);
void sub_08a2(struct z80 *z);
void sub_08d8(struct z80 *z);
void sub_094d(struct z80 *z);
void sub_0a98(struct z80 *z);
void sub_0ac3(struct z80 *z);
void sub_0af9(struct z80 *z);
void sub_0b57(struct z80 *z);

/* Lotto 9 (compito 13): porte come funzioni di piattaforma.
 * 3404 è la coda di 33EB (mai chiamata direttamente). */
void sub_0043(struct z80 *z);
void sub_234a(struct z80 *z);
void sub_2980(struct z80 *z);
void sub_2ae2(struct z80 *z);
void sub_2bc3(struct z80 *z);
void sub_2bc9(struct z80 *z);
void sub_2bf3(struct z80 *z);
void sub_2c1a(struct z80 *z);
void sub_2e11(struct z80 *z);
void sub_2e90(struct z80 *z);
void sub_2f57(struct z80 *z);
void sub_33eb(struct z80 *z);
void sub_3404(struct z80 *z);
void sub_342d(struct z80 *z);
void sub_354b(struct z80 *z);
void sub_3569(struct z80 *z);
void sub_3812(struct z80 *z);
void sub_3f88(struct z80 *z);
void sub_75aa(struct z80 *z);
void sub_797e(struct z80 *z);
void sub_7c84(struct z80 *z);

/* Lotto 10 (compito 14): la catena della NMI (nmi.c, nmi_oggetti.c,
 * z80_0xxx.c, z80_08xx.c, z80_1xxx.c). Lo stub 03EE è statico in nmi.c. */
void sub_0028(struct z80 *z);
void sub_0066(struct z80 *z);
void sub_01d5(struct z80 *z);
void sub_0219(struct z80 *z);
void sub_027c(struct z80 *z);
void sub_03fb(struct z80 *z);
void sub_042f(struct z80 *z);
void sub_0469(struct z80 *z);
void sub_0493(struct z80 *z);
void sub_04ac(struct z80 *z);
void sub_0ce6(struct z80 *z);
void sub_1288(struct z80 *z);

#endif
