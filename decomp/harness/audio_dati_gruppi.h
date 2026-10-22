/* Generato da decomp/estrai_dati.py dalla ROM audio 1.3jk: non modificare
 * a mano. Dati musicali derivati dal programma Tecmo (NOTICE.md). */

/* Gruppi di pezzi: uno per file audio_dati_*.c. */
#ifndef AUDIO_DATI_GRUPPI_H
#define AUDIO_DATI_GRUPPI_H

#include "audio_dati.h"

extern const struct aud_pezzo aud_pezzi_tabelle[];
extern const unsigned aud_n_pezzi_tabelle;
extern const struct aud_pezzo aud_pezzi_spartiti_a[];
extern const unsigned aud_n_pezzi_spartiti_a;
extern const struct aud_pezzo aud_pezzi_spartiti_b[];
extern const unsigned aud_n_pezzi_spartiti_b;
extern const struct aud_pezzo aud_pezzi_spartiti_c[];
extern const unsigned aud_n_pezzi_spartiti_c;
extern const struct aud_pezzo aud_pezzi_spartiti_d[];
extern const unsigned aud_n_pezzi_spartiti_d;
extern const struct aud_pezzo aud_pezzi_spartiti_e[];
extern const unsigned aud_n_pezzi_spartiti_e;
extern const struct aud_pezzo aud_pezzi_spartiti_f[];
extern const unsigned aud_n_pezzi_spartiti_f;
extern const struct aud_pezzo aud_pezzi_spartiti_g[];
extern const unsigned aud_n_pezzi_spartiti_g;

#define AUD_DATI_GRUPPI \
    { aud_pezzi_tabelle, &aud_n_pezzi_tabelle }, \
    { aud_pezzi_spartiti_a, &aud_n_pezzi_spartiti_a }, \
    { aud_pezzi_spartiti_b, &aud_n_pezzi_spartiti_b }, \
    { aud_pezzi_spartiti_c, &aud_n_pezzi_spartiti_c }, \
    { aud_pezzi_spartiti_d, &aud_n_pezzi_spartiti_d }, \
    { aud_pezzi_spartiti_e, &aud_n_pezzi_spartiti_e }, \
    { aud_pezzi_spartiti_f, &aud_n_pezzi_spartiti_f }, \
    { aud_pezzi_spartiti_g, &aud_n_pezzi_spartiti_g }, \

#endif
