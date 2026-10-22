/* Interprete delle sequenze del Z80 audio: 0147 (chiamata dall'IRQ), la
 * lettura dei byte di sequenza (01E8/01EF), la nota (0217-029D), il
 * glissando (01B2), l'inviluppo software (0172) e i 16 gestori della
 * tabella 03CE (03EE-05C5), smistati dal jp (hl) a 0216.
 *
 * Il registro DE dell'originale si porta nella variabile aud_de: conta in
 * 0286 (pausa con il bit 6 del flag), dove 10/11 riceve de >> 3 qualunque
 * sia l'ultimo valore lasciato dal percorso. All'ingresso del blocco 0 de è
 * quello del codice interrotto dall'IRQ (029E o il ciclo 009E): non è
 * modellabile e qui vale 0. Ai blocchi k >= 1 vale 4240 - (4000 + 20k),
 * lasciato da 019F-01A7.
 *
 * Tutte le letture passano per aud_leggi (le sequenze stanno in ROM), le
 * scritture ai blocchi per AM(). Gestore: ritorna 1 = torna a 01EF (byte
 * seguente), 0 = fine del blocco per questo IRQ. */
#include <stdio.h>
#include <stdlib.h>
#include "audio.h"

static uint16_t aud_de;                          /* registro DE dell'originale */

/* parola little endian di un campo del blocco */
static uint16_t campo16(uint16_t ix, int off)
{
    return (uint16_t)(AM(ix + off) | (AM(ix + off + 1) << 8));
}

static void scrivi16(uint16_t ix, int off, uint16_t v)
{
    AM(ix + off) = v & 0xFF;
    AM(ix + off + 1) = v >> 8;
}

/* parola little endian nello spazio del Z80 */
static uint16_t leggi16(uint16_t a)
{
    return (uint16_t)(aud_leggi(a) | (aud_leggi((uint16_t)(a + 1)) << 8));
}

/* 0x0250: durata della nota +03/+04 = +12/+13. Inviluppo software (flag &
 * 60 = 0): volume +08 = (s), contatore +05 = (s+1), puntatore +06/+07 =
 * s+1 con s = +14/+15. Inviluppo hardware col bit 6 (0286): +10/+11 = de
 * >> 3, dove de è l'ultimo valore lasciato dal percorso (non sempre il
 * periodo: con una pausa resta quello di prima). Poi 0279: fine del
 * blocco. */
static void aud_0250(uint16_t ix)
{
    scrivi16(ix, BL_DURATA, campo16(ix, BL_DUR_NOTA));
    uint8_t a = AM(ix + BL_FLAG) & 0x60;         /* and 60h; jp nz,0286h */
    if (a == 0) {
        uint16_t s = campo16(ix, BL_INV_INIZIO);
        AM(ix + BL_VOLUME) = aud_leggi(s);
        s++;
        AM(ix + BL_INV_CONT) = aud_leggi(s);
        scrivi16(ix, BL_INV_PUNT, s);
        return;
    }
    if (a & 0x40)                                /* 0286: bit 6,a; jp z,0279h */
        scrivi16(ix, BL_INV_FINE, aud_de >> 3);  /* srl d; rra (x3) */
}

/* 0x024A: +0B/+0C = de (periodo del tono), poi 0250. */
static void aud_024a(uint16_t ix)
{
    scrivi16(ix, BL_TONO_FINE, aud_de);          /* ld (ix+0bh),e; ld (ix+0ch),d */
    aud_0250(ix);
}

/* 0x0217: nota o pausa (byte 00-7F). Nibble basso >= 0C: pausa (set 3),
 * il flag non perde il glissando. Altrimenti flag &= E7 (via muto e
 * glissando), indice = 12 * ottava + nota + (+17) a 8 bit; < 30: parola
 * della tabella 0700, altrimenti byte della tabella 0760 (d = 0). */
static void aud_0217(uint16_t ix, uint8_t c)
{
    uint8_t b = c & 0x0F;
    if (b >= 0x0C) {                             /* 027F */
        AM(ix + BL_FLAG) |= 0x08;                /* set 3,(ix+0) */
        aud_0250(ix);
        return;
    }
    AM(ix + BL_FLAG) &= 0xE7;
    uint8_t a = (uint8_t)((c & 0xF0) >> 1);      /* rrca: 8 * ottava */
    uint8_t otto = a;
    a = (uint8_t)(a >> 1);                       /* rrca: 4 * ottava */
    a = (uint8_t)(a + otto + b + AM(ix + BL_17));
    if (a < 0x30) {
        uint16_t hl = aud_0008(AUD_TAB_TONI, a);  /* rst 8 */
        aud_de = aud_0010(&hl);                  /* rst 10h */
    } else {
        aud_de = aud_leggi((uint16_t)(AUD_TAB_TONI_B + (uint8_t)(a - 0x30)));
    }
    aud_024a(ix);
}

/* ---- Gestori (tabella 03CE, indice = byte & 0F) ---- */

/* 0x03EE (F0): inviluppo. n = byte seguente.
 * n = FF: forma = byte, periodo = parola seguente → +0F, +10/+11;
 *   puntatore sull'ultimo byte; flag = (flag & 9F) | 20.
 * n = FE: +0F = byte seguente; se il byte dopo è FF: puntatore su quell'FF,
 *   flag = (flag & 9F) | 40; altrimenti puntatore sulla forma,
 *   flag = (flag & 9F) | 60.
 * altrimenti: puntatore su n, +14/+15 = parola della tabella 0850 di
 *   indice n, flag &= 9F (inviluppo software). */
static int aud_03ee(uint16_t ix)
{
    uint16_t hl = (uint16_t)(campo16(ix, BL_PUNT) + 1);
    uint8_t n = aud_leggi(hl);
    if (n == 0xFF) {                             /* 041C */
        hl++;
        uint8_t forma = aud_leggi(hl);
        hl++;
        uint16_t p = leggi16(hl);
        hl++;
        aud_de = p;
        scrivi16(ix, BL_PUNT, hl);
        AM(ix + BL_FORMA) = forma;
        scrivi16(ix, BL_INV_FINE, p);
        AM(ix + BL_FLAG) = (AM(ix + BL_FLAG) & 0x9F) | 0x20;
        return 1;
    }
    if (n == 0xFE) {                             /* 043E */
        hl++;
        AM(ix + BL_FORMA) = aud_leggi(hl);
        hl++;
        if (aud_leggi(hl) == 0xFF) {             /* 045D */
            scrivi16(ix, BL_PUNT, hl);
            AM(ix + BL_FLAG) = (AM(ix + BL_FLAG) & 0x9F) | 0x40;
        } else {
            hl--;
            scrivi16(ix, BL_PUNT, hl);
            AM(ix + BL_FLAG) = (AM(ix + BL_FLAG) & 0x9F) | 0x60;
        }
        return 1;                                /* de resta quello d'ingresso */
    }
    scrivi16(ix, BL_PUNT, hl);
    uint16_t t = aud_0008(AUD_TAB_INVILUPPI, n); /* rst 8 */
    aud_de = aud_0010(&t);                       /* rst 10h */
    scrivi16(ix, BL_INV_INIZIO, aud_de);
    AM(ix + BL_FLAG) &= 0x9F;
    return 1;
}

/* 0x0470 (F1): +09 (attenuazione) = byte. */
static int aud_0470(uint16_t ix)
{
    AM(ix + BL_ATTENUA) = aud_0018(ix);
    return 1;
}

/* 0x0477 (F2): +17 (trasposizione) = byte. */
static int aud_0477(uint16_t ix)
{
    AM(ix + BL_17) = aud_0018(ix);
    return 1;
}

/* 0x047E (F3): +16 (scostamento delle durate) = byte. Tradotto dal
 * dasm, non osservato. */
static int aud_047e(uint16_t ix)
{
    AM(ix + BL_16) = aud_0018(ix);
    return 1;
}

/* 0x0485 (F4): salto. puntatore = parola seguente - 1. */
static int aud_0485(uint16_t ix)
{
    uint16_t hl = (uint16_t)(campo16(ix, BL_PUNT) + 1);
    aud_de = (uint16_t)(leggi16(hl) - 1);        /* dec de */
    scrivi16(ix, BL_PUNT, aud_de);
    return 1;
}

/* 0x0499 (F5): inizio ripetizione. +1A = contatore (byte), +18/+19 =
 * indirizzo del contatore (puntatore dopo rst 18h). */
static int aud_0499(uint16_t ix)
{
    AM(ix + BL_RIP_CONT) = aud_0018(ix);
    scrivi16(ix, BL_RIP_PUNT, campo16(ix, BL_PUNT));
    return 1;
}

/* 0x04A6 (F6): fine ripetizione. +1A -= 1; se non è zero il puntatore
 * torna a +18/+19 (contatore 0 = 256 giri). */
static int aud_04a6(uint16_t ix)
{
    if (--AM(ix + BL_RIP_CONT) == 0) return 1;   /* dec (ix+1ah); jp z,01EFh */
    aud_de = campo16(ix, BL_RIP_PUNT);
    scrivi16(ix, BL_PUNT, aud_de);
    return 1;
}

/* 0x04BB (F7): chiamata. puntatore = parola seguente - 1; +1B/+1C =
 * indirizzo del byte alto della parola (un solo livello di ritorno). */
static int aud_04bb(uint16_t ix)
{
    uint16_t hl = (uint16_t)(campo16(ix, BL_PUNT) + 1);
    uint16_t p = leggi16(hl);
    hl++;
    aud_de = (uint16_t)(p - 1);
    scrivi16(ix, BL_PUNT, aud_de);
    scrivi16(ix, BL_RIT, hl);
    return 1;
}

/* 0x04D5 (F8): ritorno. puntatore = +1B/+1C. */
static int aud_04d5(uint16_t ix)
{
    aud_de = campo16(ix, BL_RIT);
    scrivi16(ix, BL_PUNT, aud_de);
    return 1;
}

/* 0x04E4 (F9): rumore. +0A = byte + 1. */
static int aud_04e4(uint16_t ix)
{
    AM(ix + BL_RUMORE) = (uint8_t)(aud_0018(ix) + 1);
    return 1;
}

/* 0x04EC (FA): tono. +0A = 0. */
static int aud_04ec(uint16_t ix)
{
    AM(ix + BL_RUMORE) = 0;
    return 1;
}

/* 0x04F3 (FB): nota con glissando. flag &= E7; p0 = parola 1 → +0B/+0C;
 * p1 = parola 2; diff = p1 - p0, se negativo set 4 e diff = -diff;
 * durata = parola 3 → +12/+13 (senza - 1), puntatore sul suo byte alto;
 * passo = (diff << 4) / durata con la divisione a 12 giri di 0547-0559,
 * cambiato di segno col bit 4; +0D/+0E = passo; set 4; de = p0; 0250.
 * Fine del blocco. */
static int aud_04f3(uint16_t ix)
{
    AM(ix + BL_FLAG) &= 0xE7;
    uint16_t hl = (uint16_t)(campo16(ix, BL_PUNT) + 1);
    uint16_t bc = leggi16(hl);                   /* p0 */
    hl++;
    scrivi16(ix, BL_TONO_FINE, bc);
    uint16_t p0 = bc;                            /* push bc */
    hl++;
    uint16_t p1 = leggi16(hl);
    hl++;
    uint16_t de = hl;                            /* ex de,hl */
    hl = (uint16_t)(p1 - bc);                    /* or a; sbc hl,bc */
    if (p1 < bc) {
        AM(ix + BL_FLAG) |= 0x10;                /* set 4,(ix+0) */
        bc = hl;
        hl = (uint16_t)(0 - bc);
    }
    uint16_t t = de; de = hl; hl = t;            /* ex de,hl */
    de = (uint16_t)(de << 4);                    /* sla e; rla (x4); ld d,a */
    hl++;
    bc = leggi16(hl);                            /* durata */
    hl++;
    scrivi16(ix, BL_PUNT, hl);
    scrivi16(ix, BL_DUR_NOTA, bc);
    hl = 0;
    for (int giro = 12; giro != 0; giro--) {     /* ld a,0ch ... dec a; jp nz */
        unsigned alto = de >> 15;                /* sla e; rl d */
        de = (uint16_t)(de << 1);
        hl = (uint16_t)((hl << 1) | alto);       /* rl l; rl h */
        int prestito = hl < bc;                  /* or a; sbc hl,bc */
        hl = (uint16_t)(hl - bc);
        de++;                                    /* inc de */
        if (prestito) {
            de--;                                /* dec de */
            hl = (uint16_t)(hl + bc);            /* add hl,bc */
        }
    }
    if (AM(ix + BL_FLAG) & 0x10)                 /* bit 4,(ix+0) */
        de = (uint16_t)(0 - de);
    scrivi16(ix, BL_GLISSA, de);
    AM(ix + BL_FLAG) |= 0x10;
    aud_de = p0;                                 /* pop de */
    aud_0250(ix);
    return 0;
}

/* 0x0578 (FC): nota a periodo diretto. flag &= E7; periodo = parola
 * seguente → +0B/+0C (024A); puntatore sul byte alto; 0250. Fine del
 * blocco. */
static int aud_0578(uint16_t ix)
{
    AM(ix + BL_FLAG) &= 0xE7;
    uint16_t hl = (uint16_t)(campo16(ix, BL_PUNT) + 1);
    aud_de = aud_0010(&hl);                      /* rst 10h */
    scrivi16(ix, BL_PUNT, hl);
    aud_024a(ix);
    return 0;
}

/* 0x0591 (FD): comando. (4243) = byte: 00E4 lo avvia nello stesso IRQ.
 * Tradotto dal dasm, non osservato. */
static int aud_0591(uint16_t ix)
{
    AM(AUD_COMANDO) = aud_0018(ix);
    return 1;
}

static int aud_05a6(uint16_t ix);

/* 0x0598 (FE): attenuazione + 1; arrivata a 0F prosegue in FF (fine) senza
 * aggiornare +09. Tradotto dal dasm, non osservato. */
static int aud_0598(uint16_t ix)
{
    uint8_t a = (uint8_t)(AM(ix + BL_ATTENUA) + 1);
    if (a >= 0x0F) return aud_05a6(ix);          /* cp 0fh; jr nc,05A6h */
    AM(ix + BL_ATTENUA) = a;
    return 1;
}

/* 0x05A6 (FF): fine della sequenza. flag = (flag & 04) | 03 (029E spegne
 * la voce); un blocco del secondo strato (oltre 4100, blocchi 9-17) toglie
 * il bit 2 «coperto» al blocco - 9. Fine del blocco. */
static int aud_05a6(uint16_t ix)
{
    AM(ix + BL_FLAG) = (AM(ix + BL_FLAG) & 0x04) | 0x03;
    aud_de = (uint16_t)(0x4100 - ix);            /* or 3 azzera il riporto; sbc hl,de */
    if (ix > 0x4100)                             /* jp nc,01ACh */
        AM(ix - 0x120) &= (uint8_t)~0x04;        /* res 2,(hl) */
    return 0;
}

/* Smistamento: tabella C indicizzata da byte & 0F, controllata una volta
 * contro le parole della ROM a 03CE. */
typedef int (*gestore)(uint16_t ix);
static const gestore gestori[16] = {
    aud_03ee, aud_0470, aud_0477, aud_047e, aud_0485, aud_0499, aud_04a6,
    aud_04bb, aud_04d5, aud_04e4, aud_04ec, aud_04f3, aud_0578, aud_0591,
    aud_0598, aud_05a6,
};
static const uint16_t indirizzi[16] = {
    0x03EE, 0x0470, 0x0477, 0x047E, 0x0485, 0x0499, 0x04A6, 0x04BB,
    0x04D5, 0x04E4, 0x04EC, 0x04F3, 0x0578, 0x0591, 0x0598, 0x05A6,
};

static void controlla_tabella(void)
{
    static int fatto;
    if (fatto) return;
    fatto = 1;
    for (int i = 0; i < 16; i++) {
        uint16_t w = leggi16((uint16_t)(AUD_TAB_GESTORI + 2 * i));
        if (w != indirizzi[i]) {
            fprintf(stderr, "ERRORE: gestore %X a 03CE vale %04X, atteso %04X\n",
                    i, w, indirizzi[i]);
            exit(2);
        }
    }
}

/* 0x01E8: lettura della sequenza (ix = blocco), ciclo 01EF. F0-FF →
 * gestore; 80-EF → durata +12/+13 = parola della tabella 0790 di indice
 * (b & 7F) + (+16), meno 1; 00-7F → nota 0217, che chiude il blocco. */
static void aud_01e8(uint16_t ix)
{
    for (;;) {
        uint8_t a = aud_0018(ix);                /* 01EF: rst 18h */
        if (a >= 0xF0) {                         /* 020E */
            uint8_t i = a & 0x0F;
            aud_de = (uint16_t)(AUD_TAB_GESTORI + 1 + 2 * i);  /* ex de,hl */
            if (!gestori[i](ix)) return;
            continue;
        }
        if (a < 0x80) {
            aud_0217(ix, a);
            return;
        }
        a = (uint8_t)((a & 0x7F) + AM(ix + BL_16));
        uint16_t hl = aud_0008(AUD_TAB_DURATE, a);   /* rst 8 */
        aud_de = (uint16_t)(aud_0010(&hl) - 1);      /* rst 10h; dec de */
        scrivi16(ix, BL_DUR_NOTA, aud_de);
    }
}

/* 0x01B2: glissando. periodo +0B/+0C += passo +0D/+0E (16 bit, si
 * avvolge; nessuna fine: lo ferma solo una nota nuova). Col bit 6 del
 * flag di partenza (01CF) anche +10/+11 = periodo >> 3. */
static void aud_01b2(uint16_t ix, uint8_t flag)
{
    uint16_t p = (uint16_t)(campo16(ix, BL_TONO_FINE) + campo16(ix, BL_GLISSA));
    scrivi16(ix, BL_TONO_FINE, p);
    if (flag & 0x40)
        scrivi16(ix, BL_INV_FINE, p >> 3);
}

/* 0x0172: inviluppo software, solo se i bit 5 e 6 del flag di partenza
 * sono a zero. +05 -= 1; a zero: p = +06/+07; se (p+1) non è FF: volume
 * +08 = (p+1), puntatore = p+2, +05 = (p+2); se è FF riparte da s =
 * +14/+15: +08 = (s), puntatore = s+1, +05 = (s+1). */
static void aud_0172(uint16_t ix, uint8_t flag)
{
    if (flag & 0x60) return;                     /* and 18h sul flag ruotato */
    if (--AM(ix + BL_INV_CONT) != 0) return;     /* dec (hl); jp nz,019Ch */
    uint16_t p = (uint16_t)(campo16(ix, BL_INV_PUNT) + 1);
    uint8_t a = aud_leggi(p);
    if (a == 0xFF) {
        p = campo16(ix, BL_INV_INIZIO);
        a = aud_leggi(p);
    }
    AM(ix + BL_VOLUME) = a;
    p++;
    scrivi16(ix, BL_INV_PUNT, p);
    AM(ix + BL_INV_CONT) = aud_leggi(p);
}

/* 0x0147: interprete delle sequenze, una passata sui 18 blocchi. Per ogni
 * blocco attivo (bit 0) e non da spegnere (bit 1): durata a 16 bit +03/+04
 * meno 1; col prestito su +04 = 0 legge la sequenza (01E8) e il blocco
 * finisce qui (una durata d dura d + 1 IRQ); altrimenti glissando (bit 4)
 * e inviluppo software col flag letto alla partenza del blocco. */
void aud_0147(void)
{
    controlla_tabella();
    aud_de = 0;                                  /* blocco 0: de del codice interrotto, non modellabile */
    for (int k = 0; k < AUD_N_BLOCCHI; k++) {
        uint16_t ix = (uint16_t)(AUD_BLOCCHI + k * AUD_PASSO_BLOCCO);
        if (k > 0)
            aud_de = (uint16_t)(AUD_OMBRE_MIXER - ix);   /* 019F-01A7 */
        uint8_t flag = AM(ix + BL_FLAG);
        if (!(flag & 0x01) || (flag & 0x02)) continue;  /* rrca; jp nc / rrca; jr c */
        uint8_t d = AM(ix + BL_DURATA);
        AM(ix + BL_DURATA) = (uint8_t)(d - 1);   /* sub 1 */
        if (d == 0) {                            /* prestito */
            if (AM(ix + BL_04) == 0) {           /* jp z,01E8h */
                aud_01e8(ix);
                continue;
            }
            AM(ix + BL_04)--;
        }
        if (flag & 0x10)                         /* bit 2,c sul flag ruotato */
            aud_01b2(ix, flag);
        aud_0172(ix, flag);
    }
}
