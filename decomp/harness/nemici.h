/* Motore dei nemici (corpo dell'attività 0A e routine collegate), lotti
 * N1-N12 di CENSIMENTO-NEMICI.md (motore completo). Prototipi delle routine
 * tradotte e meccanismo che ferma il replay quando il C
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

/* Punti di sincronia per la NMI (brief 26): il C del motore chiama
 * NEMICI_PUNTO(da, a) dove l'originale, interrotto con un ritorno fra
 * `da` e `a`, avrebbe la stessa RAM. replay.c ci aggancia nemici_punto
 * per eseguire lì una NMI entrata a metà del corpo 0A; altrove è NULL. */
extern void (*nemici_punto)(uint16_t da, uint16_t a);
#define NEMICI_PUNTO(da, a) \
    do { if (nemici_punto) nemici_punto((da), (a)); } while (0)

/* Come NEMICI_PUNTO ma attivo solo nel giro dal boot (lotto H4b3, brief 47):
 * i punti aggiunti alle routine già verificate del motore dei nemici e ai
 * corpi 14 e 0A non cambiano l'esito del replay storico. gioco_libero.c
 * accende giro_dal_boot. */
extern int giro_dal_boot;
#define PUNTO_GIRO(da, a) \
    do { if (giro_dal_boot && nemici_punto) nemici_punto((da), (a)); } while (0)

/* Corpo dell'attività 0A (nemici.c). */
void attivita_0a_4da2(uint8_t slot);

/* Corpi 01 e 10 (attivita_01_10.c). */
void attivita_01_48d1(uint8_t slot);
void attivita_10_457b(uint8_t slot);

/* Come NEMICI_PUNTO ma senza controllo d'impronta, per i ritorni dentro
 * routine tradotte senza punti interni (attivita_attract.c, lotto H4b1):
 * macchina.c ci aggancia il suo gancio, altrove è NULL. */
extern void (*attract_punto_largo)(uint16_t da, uint16_t a);

/* Corpi 1F e 06 (attivita_attract.c, lotto H4b1). */
void attivita_1f_43c8(uint8_t slot);
void attivita_06_75e7(uint8_t slot);

/* Corpo 07 (attivita_attract07*.c, lotto H4b2). */
void attivita_07_2ccd(uint8_t slot);

/* Corpi 08, 0C e 0B (attivita_attract08.c, lotto H4b3). */
void attivita_08_7c98(uint8_t slot);
void attivita_0c_7bf6(uint8_t slot);
void attivita_0b_2c27(uint8_t slot);

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

/* Lotto N4, comparsa (brief 30): nemici_comparsa.c e nemici_comparsab.c.
 * 14FC, 73C7 e 7456 sono statiche; 4FC5 non ha voce nel banco. */
void sub_134b(struct z80 *z);
void sub_139c(struct z80 *z);
void sub_13ab(struct z80 *z);
void sub_13b9(struct z80 *z);
void sub_13fc(struct z80 *z);
void sub_1413(struct z80 *z);
void sub_14d4(struct z80 *z);
void sub_1549(struct z80 *z);
void sub_2548(struct z80 *z);
void sub_4f40(struct z80 *z);
void sub_4fc5(struct z80 *z);
void sub_7288(struct z80 *z);
void sub_7294(struct z80 *z);
void sub_72b6(struct z80 *z);
void sub_72ce(struct z80 *z);
void sub_72da(struct z80 *z);
void sub_738f(struct z80 *z);
void sub_73a3(struct z80 *z);
void sub_7412(struct z80 *z);
void sub_741d(struct z80 *z);
void sub_7470(struct z80 *z);
void sub_74ab(struct z80 *z);
void sub_756c(struct z80 *z);
void sub_758e(struct z80 *z);
void sub_759f(struct z80 *z);

/* Lotto N1, nemici_tipi_15_17.c. */
void sub_7f51(struct z80 *z);
void sub_7f68(struct z80 *z);

/* Lotto N2, nemici_tipi_08_14.c (tipi 14 e 8; 6FC3, 6FDF e la coda
 * 7056 sono statiche). */
void sub_6ce8(struct z80 *z);
void sub_6d12(struct z80 *z);
void sub_6d1b(struct z80 *z);
void sub_6d37(struct z80 *z);
void sub_6dce(struct z80 *z);
void sub_6e55(struct z80 *z);
void sub_6e90(struct z80 *z);
void sub_6ea1(struct z80 *z);
void sub_6eca(struct z80 *z);
void sub_6f49(struct z80 *z);
void sub_6f4b(struct z80 *z);
void sub_6f5e(struct z80 *z);
void sub_6f6e(struct z80 *z);
void sub_6f81(struct z80 *z);
void sub_6f91(struct z80 *z);
void sub_6fb3(struct z80 *z);
void sub_6fd1(struct z80 *z);
void sub_6fed(struct z80 *z);
void sub_6ff7(struct z80 *z);
void sub_7001(struct z80 *z);
void sub_700b(struct z80 *z);
void sub_7015(struct z80 *z);
void sub_7025(struct z80 *z);
void sub_7036(struct z80 *z);
void sub_7045(struct z80 *z);
void sub_70a8(struct z80 *z);
void sub_70b0(struct z80 *z);
void sub_70ce(struct z80 *z);
void sub_70fe(struct z80 *z);

/* Lotto N2, dipendenze anticipate dai lotti N3-N5 (brief 27). */
void sub_571d(struct z80 *z);              /* nemici_tipi_10_13.c (N5) */
void sub_57aa(struct z80 *z);              /* nemici_tipi_10_13.c (N5) */
void sub_57b3(struct z80 *z);              /* nemici_tipi_10_13.c (N5) */
void sub_80a5(struct z80 *z);              /* nemici_comparsa.c (N4) */
void sub_6eb7(struct z80 *z);              /* nemici_tipo09.c (N3) */
void sub_6ec1(struct z80 *z);              /* nemici_tipo09.c (N3) */

/* Lotto N3, tipo 09 (brief 29): nemici_tipo09.c, nemici_tipo09b.c, con le
 * dipendenze anticipate da N6 (nemici_tipo06b.c) e N10 (nemici_tipo06a.c).
 * 5894, 5C2D e 5B09 sono statiche. */
void sub_57cd(struct z80 *z);
void sub_57e4(struct z80 *z);
void sub_57fb(struct z80 *z);
void sub_580a(struct z80 *z);
void sub_582b(struct z80 *z);
void sub_58a3(struct z80 *z);
void sub_594f(struct z80 *z);
void sub_5995(struct z80 *z);
void sub_5b12(struct z80 *z);
void sub_5c19(struct z80 *z);
void sub_5c1e(struct z80 *z);
void sub_5c23(struct z80 *z);
void sub_5c29(struct z80 *z);
void sub_52fd(struct z80 *z);
void sub_5345(struct z80 *z);
void sub_54c5(struct z80 *z);
void sub_51d7(struct z80 *z);
void sub_24df(struct z80 *z);

/* Lotto N5, tipi 10-13 (brief 31): 55C4, 6C1D e 719E in nemici_tipi_10_13.c,
 * le altre in nemici_tipi_10_13b.c. */
void sub_54d8(struct z80 *z);
void sub_54ea(struct z80 *z);
void sub_551a(struct z80 *z);
void sub_5529(struct z80 *z);
void sub_5584(struct z80 *z);
void sub_55c4(struct z80 *z);
void sub_56e9(struct z80 *z);
void sub_5713(struct z80 *z);
void sub_5726(struct z80 *z);
void sub_573c(struct z80 *z);
void sub_5749(struct z80 *z);
void sub_6bcd(struct z80 *z);
void sub_6c1d(struct z80 *z);
void sub_719e(struct z80 *z);

/* Lotto N6, tipo 6 seconda metà (brief 32): 5274-531C in nemici_tipo06b.c;
 * 5068, 51FA e 5224 (anticipate da N10) in nemici_tipo06a.c. 531C lascia il
 * riporto a 1 se la cella è piena (uscita anomala: il chiamante ritorna). */
void sub_5068(struct z80 *z);
void sub_51fa(struct z80 *z);
void sub_5224(struct z80 *z);
void sub_5274(struct z80 *z);
void sub_5289(struct z80 *z);
void sub_5292(struct z80 *z);
void sub_52aa(struct z80 *z);
void sub_52c2(struct z80 *z);
void sub_52db(struct z80 *z);
void sub_52ec(struct z80 *z);
void sub_531c(struct z80 *z);

/* Lotto N7, corpo 0A (brief 33): 4649 e 46A3 (vie di 4632), 470A, 4717 e
 * 4767 (condivisa con 37B4, non tradotta) in nemici.c. */
void sub_4649(struct z80 *z);
void sub_46a3(struct z80 *z);
void sub_470a(struct z80 *z);
void sub_4717(struct z80 *z);
void sub_4767(struct z80 *z);

/* Lotto N8, tipo 07 prima metà (brief 34): 5CFB-5FAC in nemici_tipo07a.c,
 * 4FEF (anticipata da N10) in nemici_tipo06a.c. 5E09 lascia il riporto
 * (A < 0x18) in z->f; 5E8F: A, E; 5F0C: A; 5F4B e 5F82: B, C. */
void sub_5cfb(struct z80 *z);
void sub_5d29(struct z80 *z);
void sub_5d76(struct z80 *z);
void sub_5ddf(struct z80 *z);
void sub_5e09(struct z80 *z);
void sub_5e1a(struct z80 *z);
void sub_5e4d(struct z80 *z);
void sub_5e60(struct z80 *z);
void sub_5e8f(struct z80 *z);
void sub_5e9d(struct z80 *z);
void sub_5edd(struct z80 *z);
void sub_5f0c(struct z80 *z);
void sub_5f33(struct z80 *z);
void sub_5f4b(struct z80 *z);
void sub_5f82(struct z80 *z);
void sub_5fac(struct z80 *z);
void sub_4fef(struct z80 *z);

/* Lotto N9, tipi 15-17 (brief 35), nemici_tipi_15_17.c. Nessuna lascia il
 * riporto fuori (6B9B e 6C8E lo usano solo dentro). */
void sub_6b43(struct z80 *z);
void sub_6b5f(struct z80 *z);
void sub_6b8f(struct z80 *z);
void sub_6b9b(struct z80 *z);
void sub_6c0b(struct z80 *z);
void sub_6c5f(struct z80 *z);
void sub_6c8e(struct z80 *z);
void sub_7f48(struct z80 *z);
void sub_7f81(struct z80 *z);
void sub_7f8a(struct z80 *z);
void sub_7fa6(struct z80 *z);
void sub_7fcf(struct z80 *z);
void sub_801d(struct z80 *z);
void sub_8042(struct z80 *z);
void sub_808d(struct z80 *z);
void sub_809c(struct z80 *z);

/* Lotto N10, tipo 06 prima metà (brief 36), nemici_tipo06a.c. 4FD4 e 50DF
 * leggono R tramite 51FA (banco_r_4fd4, banco_r_50df). */
void sub_4fd4(struct z80 *z);
void sub_5005(struct z80 *z);
void sub_5023(struct z80 *z);
void sub_5035(struct z80 *z);
void sub_5066(struct z80 *z);
void sub_5092(struct z80 *z);
void sub_50ba(struct z80 *z);
void sub_50df(struct z80 *z);
void sub_5115(struct z80 *z);
void sub_5124(struct z80 *z);
void sub_513a(struct z80 *z);
void sub_513e(struct z80 *z);
void sub_514c(struct z80 *z);
void sub_516b(struct z80 *z);
void sub_5186(struct z80 *z);
void sub_51a4(struct z80 *z);
void sub_51b1(struct z80 *z);
void sub_51b5(struct z80 *z);
void sub_51d3(struct z80 *z);

/* Lotto N11 (brief 37, nemici_tipo07b.c) e N12 (nemici_comuni.c). 6248 lascia
 * il riporto (A < 4); sub_610c lo lascia acceso solo con l'uscita anomala di
 * 627F (HL = 5D0F e 5CFB non prosegue); 2474 lascia il riporto D < E. */
void sub_2474(struct z80 *z);
void sub_4ada(struct z80 *z);
void sub_5fc8(struct z80 *z);
void sub_5fdc(struct z80 *z);
void sub_6046(struct z80 *z);
void sub_610c(struct z80 *z);
void sub_6151(struct z80 *z);
void sub_6165(struct z80 *z);
void sub_6179(struct z80 *z);
void sub_6183(struct z80 *z);
void sub_618b(struct z80 *z);
void sub_6196(struct z80 *z);
void sub_61b1(struct z80 *z);
void sub_61fa(struct z80 *z);
void sub_6221(struct z80 *z);
void sub_6248(struct z80 *z);
void sub_6252(struct z80 *z);
void sub_6261(struct z80 *z);
void sub_6270(struct z80 *z);
void sub_627f(struct z80 *z);

/* Il registro R (nemici_r.c): somma R1 + R2 dei due `ld a,r` di 54C5, 59E5
 * e 5DDF; nel banco i wrapper cercano il valore di R che riproduce
 * la cattura (vedi l'intestazione di nemici_r.c). */
uint8_t nemici_r_somma(void);
uint8_t nemici_r_somma_r1(uint8_t *r1);   /* somma e R1 */
/* Sentinella del registro R (brief 45): se non NULL, nemici_r_somma_r1 la
 * chiama prima di leggere R. macchina.c ci aggancia r_letto, che ferma il
 * giro: la partita registrata non legge R (registra_r.sh), un percorso
 * diverso sì. NULL nel banco, nel replay e nel gioco libero: nessun effetto. */
extern void (*nemici_r_gancio)(void);
/* Sorgente di R1 nel gioco libero (brief 56): se non NULL,
 * nemici_r_somma_r1 prende R1 da qui invece che da r_valori.
 * NULL nel banco, nel replay e nel modo oracolo. */
extern uint8_t (*nemici_r_valore)(void);

void banco_r_54c5(struct z80 *z);
void banco_r_51d7(struct z80 *z);
void banco_r_5995(struct z80 *z);
void banco_r_57cd(struct z80 *z);
void banco_r_51fa(struct z80 *z);
void banco_r_5ddf(struct z80 *z);
void banco_r_5e1a(struct z80 *z);
void banco_r_4fd4(struct z80 *z);
void banco_r_50df(struct z80 *z);
void banco_r_5cfb(struct z80 *z);
void banco_r_4302(struct z80 *z);

#endif
