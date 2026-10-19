/* Memoria e registri per la prova di metodo della decompilazione.
 * ram[] copre c000-ffff come i file snap_*.bin scaricati da MAME. */
#ifndef RAM_H
#define RAM_H

#include <stdint.h>

extern uint8_t ram[0x4000];
#define M(a) ram[(uint16_t)(a) - 0xc000]

/* Registri del Z80 che servono alle routine tradotte (niente banco alternativo). */
struct z80 {
    uint8_t a, f, b, c, d, e, h, l;
    uint16_t ix, iy, sp;
};

/* Coppie di registri a 16 bit, in lettura e in scrittura. */
#define BC(z) ((uint16_t)(((z)->b << 8) | (z)->c))
#define DE(z) ((uint16_t)(((z)->d << 8) | (z)->e))
#define HL(z) ((uint16_t)(((z)->h << 8) | (z)->l))
#define SET_BC(z, v) do { uint16_t v_ = (v); (z)->b = v_ >> 8; (z)->c = v_ & 0xff; } while (0)
#define SET_DE(z, v) do { uint16_t v_ = (v); (z)->d = v_ >> 8; (z)->e = v_ & 0xff; } while (0)
#define SET_HL(z, v) do { uint16_t v_ = (v); (z)->h = v_ >> 8; (z)->l = v_ & 0xff; } while (0)

/* Routine tradotte (routine.c). */
void sub_05b3(struct z80 *z);
void sub_2402(struct z80 *z);

/* Decodificatore del fondale e callee (fondale.c). */
void sub_326c(struct z80 *z);
void sub_23bb(struct z80 *z, int riporto);
void sub_23cb(struct z80 *z);
uint16_t sub_23ec(uint8_t riga);
int sub_336c(uint8_t base, uint8_t attributi, uint8_t riga, uint8_t colonna);

/* Protezione e603 (porte.c). */
uint8_t leggi_e603(uint16_t pc);

#endif
