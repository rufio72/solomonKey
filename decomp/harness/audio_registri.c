/* Scrittura dei registri dei tre YM2149 dallo stato dei 18 blocchi voce:
 * 029E (una passata) e 036B. L'originale tiene porta, voce e puntatore
 * all'ombra del mixer nel banco alternativo (exx): qui sono variabili locali.
 * Blocco k → chip k/3 % 3 + 1 (porta 10/20/30), voce b = 3 - k%3; i blocchi
 * 9-17 sono il secondo strato con la stessa associazione. */
#include "audio.h"

/* 0x036B: e = 4240 + chip - 1 (ombra del mixer del chip), h = 9 << (b-1),
 * l = l << (b-1): bit di tono e rumore della voce b nel registro 7. */
static uint16_t aud_036b(uint8_t c, uint8_t b, uint8_t *h, uint8_t *l)
{
    uint16_t de = (uint16_t)(AUD_OMBRE_MIXER + ((c & 0xF0) >> 4) - 1);
    *h = 0x09;
    for (uint8_t a = b; --a != 0;) {             /* dec a; ret z; sla l; sla h */
        *l = (uint8_t)(*l << 1);
        *h = (uint8_t)(*h << 1);
    }
    return de;
}

/* 0x02E5: voce spenta: bit di tono e rumore a 1 nell'ombra del mixer,
 * volume (registro 8+b-1) = 0. */
static void aud_02e5(uint8_t c, uint8_t b)
{
    uint8_t h, l = 0x09;
    uint16_t de = aud_036b(c, b, &h, &l);
    AM(de) |= l;                                 /* ld a,(de); or l; ld (de),a */
    aud_out(c, (uint8_t)((b - 1) | 0x08));
    aud_out(c + 1, 0x00);
}

/* 0x0323: dal byte +0A in poi. Mixer: +0A = 0 → tono acceso e rumore
 * spento, altrimenti rumore acceso e tono spento; registro 6 = +0A - 1 se
 * +0A ≠ 0; tono (registri 2(b-1), 2(b-1)+1) da +0B/+0C. */
static void aud_0323(uint16_t blocco, uint8_t c, uint8_t b)
{
    uint8_t rumore = AM(blocco + BL_RUMORE);
    uint8_t h, l = rumore ? 0x08 : 0x01;         /* ld l,08h | ld l,01h */
    uint16_t de = aud_036b(c, b, &h, &l);
    AM(de) = (uint8_t)((AM(de) | h) & (uint8_t)~l);  /* cpl; or h; and l */
    if (rumore != 0) {
        aud_out(c, 0x06);                        /* periodo del rumore */
        aud_out(c + 1, (uint8_t)(rumore - 1));
    }
    uint8_t fine = AM(blocco + BL_TONO_FINE);
    uint8_t grosso = AM(blocco + BL_TONO_GROS);
    uint8_t reg = (uint8_t)((b - 1) << 1);       /* ld h,b; dec h; sla h */
    aud_out(c, reg);
    aud_out(c + 1, fine);
    aud_out(c, (uint8_t)(reg + 1));
    aud_out(c + 1, grosso);
}

/* 0x030B: volume = +08 - +09 a 8 bit, 0 se il risultato è >= 10h (cioè
 * negativo, o oltre il massimo); poi 0323. */
static void aud_030b(uint16_t blocco, uint8_t c, uint8_t b)
{
    uint8_t v = (uint8_t)(AM(blocco + BL_VOLUME) - AM(blocco + BL_ATTENUA));
    if (v >= 0x10) v = 0;                        /* cp 10h; jp c,0316h; xor a */
    aud_out(c, (uint8_t)((b - 1) | 0x08));       /* set 3,l */
    aud_out(c + 1, v);
    aud_0323(blocco, c, b);
}

/* 0x0381: inviluppo hardware. sel = bit 5-6 del flag (1-3). Volume della
 * voce = 10h (modo inviluppo); se +0F ≠ 0 forma (registro 0D) = +0F e +0F
 * azzerato; periodo (registri 0C/0B) da +11/+10. Poi, se sel ha il bit 0
 * (flag bit 5), prosegue con 0323 (mixer, rumore, tono); altrimenti con
 * 02E5, che spegne la voce e riscrive il volume a 0. */
static void aud_0381(uint16_t blocco, uint8_t c, uint8_t b, uint8_t sel)
{
    uint8_t inv_gros = AM(blocco + BL_INV_GROS);
    uint8_t inv_fine = AM(blocco + BL_INV_FINE);
    uint8_t forma = AM(blocco + BL_FORMA);
    if (forma != 0) AM(blocco + BL_FORMA) = 0x00;
    aud_out(c, (uint8_t)((b - 1) | 0x08));
    aud_out(c + 1, 0x10);
    if (forma != 0) {
        aud_out(c, 0x0D);
        aud_out(c + 1, forma);
    }
    aud_out(c, 0x0C);
    aud_out(c + 1, inv_gros);
    aud_out(c, 0x0B);
    aud_out(c + 1, inv_fine);
    if (sel & 0x01)                              /* bit 0,c; jp nz,0323h */
        aud_0323(blocco, c, b);
    else
        aud_02e5(c, b);                          /* jp 02E5h */
}

/* 0x029E: una passata sui 18 blocchi, poi registro 7 dei tre chip dalle
 * ombre 4240-4242 (02CA). Per blocco attivo (flag bit 0): bit 1 → 02DD
 * (flag &= 4, voce spenta); bit 2 → blocco saltato (coperto dal secondo
 * strato); bit 3 → voce spenta; bit 5-6 → 0381; altrimenti 030B. */
void aud_029e(void)
{
    uint8_t c = 0x10;                            /* ld c,10h */
    for (int k = 0; k < AUD_N_BLOCCHI; k++) {
        uint16_t blocco = (uint16_t)(AUD_BLOCCHI + k * AUD_PASSO_BLOCCO);
        uint8_t b = (uint8_t)(3 - k % 3);        /* ld b,03h; djnz */
        if (k % 3 == 0 && k != 0)                /* ld a,10h; add a,c; cp 40h */
            c = (c == 0x30) ? 0x10 : (uint8_t)(c + 0x10);
        uint8_t flag = AM(blocco + BL_FLAG);
        if (!(flag & 0x01)) continue;            /* rrca; jp c,02DDh */
        if (flag & 0x02) {                       /* 02DD */
            AM(blocco + BL_FLAG) = flag & 0x04;
            aud_02e5(c, b);
        } else if (flag & 0x04) {
            continue;                            /* 02FD: rrca; jp c,02AFh */
        } else if (flag & 0x08) {
            aud_02e5(c, b);                      /* 0301: rrca; jp c,02E5h */
        } else if ((flag >> 5) & 0x03) {         /* 0305: rrca; and 03h */
            aud_0381(blocco, c, b, (flag >> 5) & 0x03);
        } else {
            aud_030b(blocco, c, b);
        }
    }
    /* 02CA: ld bc,0310h; ld e,07h; registro 7 dei chip 1-3 */
    for (int chip = 0; chip < 3; chip++) {
        uint8_t porta = (uint8_t)(0x10 * (chip + 1));
        aud_out(porta, 0x07);
        aud_out(porta + 1, AM(AUD_OMBRE_MIXER + chip));
    }
}
