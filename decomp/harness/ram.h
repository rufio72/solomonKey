/* Memoria e registri per la prova di metodo della decompilazione.
 * ram[] copre c000-ffff come i file snap_*.bin scaricati da MAME. */
#ifndef RAM_H
#define RAM_H

#include <stdint.h>

extern uint8_t ram[0x4000];
#define M(a) ram[(uint16_t)(a) - 0xc000]

/* Immagine della ROM a indirizzi reali (main64.bin), definita in comune.c
 * con ram[] e carica_rom() (che la legge da out/ o ../out/, esce se manca). */
extern uint8_t rom[0x10000];
void carica_rom(void);
/* Lettura da tutto lo spazio del Z80: c000-ffff dalla cattura, il resto dalla ROM. */
static inline uint8_t leggi(uint16_t a) { return a >= 0xc000 ? ram[a - 0xc000] : rom[a]; }

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

/* Porte mappate in memoria (porte.c; solomon.cpp, MAME 0.264): e600 P1 /
 * maschera NMI; e601 P2; e602 SYSTEM (start 1-2 bit 0-1, coin 1-2 bit
 * 2-3); e603 protezione (solo via leggi_e603(pc)); e604 DSW1 /
 * ribaltamento schermo; e605 DSW2; e606 lettura a vuoto (0). Nei dump di
 * snap.lua ram[a - 0xc000] è il valore vivo all'istante della cattura
 * (proposta A del brief 12). Nel C tradotto i siti porta usano queste
 * funzioni con l'indirizzo esplicito, mai M(). */
uint8_t leggi_porta(uint16_t a);           /* e600-e606: ram[a - 0xc000] */
void scrivi_porta(uint16_t a, uint8_t v);  /* scrive ram[a - 0xc000] */

#endif
