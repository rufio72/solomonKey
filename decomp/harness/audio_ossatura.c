/* Ossatura del driver audio: avvio, IRQ, NMI, smistatore dei comandi e rst.
 * Le scritture a ffff dell'originale (0079, 0088, 008D, 0093, 009A, 003F)
 * vanno a un nop di MAME e si omettono. */
#include <string.h>
#include "audio.h"

/* 0x0072: avvio dopo il reset (0000: di; jp 0072). sp = 4800, im 1, poi 16
 * giri di: RAM 4000-47ff a zero (ldir) e 00A4; infine ei. Il ciclo infinito
 * 009E (call 029E; jp 009E) non è qui: lo modella il banco. */
void aud_avvio(void)
{
    for (int giro = 0x10; giro != 0; giro--) {   /* ld a,10h ... ex af,af'; dec a; jr nz */
        memset(aud_ram, 0, sizeof aud_ram);      /* (4000)=0; ldir 7ffh byte */
        aud_00a4();
    }
}

/* 0x00A4: ombre del mixer 4240-4242 = FF, poi tre giri che vorrebbero
 * scrivere i registri 7-0A dei chip 1-3. Errore dell'originale: a 00CE
 * «ld a,0fh; add a,c; ld a,0ffh» scarta la somma, quindi c avanza solo di
 * 1 per giro (10, 11, 12) invece di 10, 20, 30. Effetto: il chip 1 riceve
 * 07..0A come dati nel registro 0A, le porte 12 e 13 (non mappate) ricevono
 * 8 e 4 scritture per chiamata. Si riproduce così com'è. */
void aud_00a4(void)
{
    AM(AUD_OMBRE_MIXER) = 0xFF;
    AM(AUD_OMBRE_MIXER + 1) = 0xFF;
    AM(AUD_OMBRE_MIXER + 2) = 0xFF;
    uint8_t c = 0x10;                            /* ld bc,0310h */
    for (int b = 3; b != 0; b--) {
        uint8_t a = 0xFF;
        uint8_t e = 0x07;                        /* ld e,07h */
        aud_out(c, e);                           /* out (c),e */
        aud_out(c + 1, a);                       /* out (c),a: registro 7 = FF */
        e++;
        a++;                                     /* a = 00 */
        aud_out(c, e);
        aud_out(c + 1, a);                       /* registro 8 = 0 */
        e++;
        aud_out(c, e);
        aud_out(c + 1, a);                       /* registro 9 = 0 */
        e++;
        aud_out(c, e);
        c++;                                     /* inc c (resta incrementato) */
        aud_out(c, a);                           /* registro 0A = 0 */
        /* ld a,0fh; add a,c; ld a,0ffh: la somma va persa, c resta c+1 */
    }
}

/* 0x00D6: arresto (comando 00): flag dei 18 blocchi = 03 (attivo e da
 * spegnere: 029E azzera volume e voce). */
static void aud_00d6(void)
{
    for (int k = 0; k < AUD_N_BLOCCHI; k++)      /* ld bc,1203h; djnz */
        AM(AUD_BLOCCHI + k * AUD_PASSO_BLOCCO + BL_FLAG) = 0x03;
}

/* 0x00FA: avvio di un comando. de punta a terne «blocco, parola» chiuse
 * da FF. Per ogni blocco: se è del secondo strato (>= 9) copre il blocco
 * -9 (bit 2); flag = (flag & 4) | 1; +01/+02 = parola - 1; +03 = 1;
 * +04, +08, +09, +0A, +16, +17 = 0. Letture e scritture passano per
 * aud_leggi/aud_scrivi: la voce del comando 40 punta fuori dalla ROM (7A09)
 * e l'originale scorre la memoria non mappata (00) fino a FFFF, latch
 * compreso, scrivendo anche fuori dalla RAM. */
static void aud_00fa(uint16_t de)
{
    if (de == 0) return;                         /* ld a,e; or d; ret z */
    for (;;) {
        uint8_t n = aud_leggi(de);               /* 00FD: ld a,(de) */
        if (n == 0xFF) return;                   /* inc a; ret z */
        uint16_t hl = (uint16_t)(AUD_BLOCCHI + n * AUD_PASSO_BLOCCO);
        if (n >= 9) {                            /* cp 09h; jp c,011Ch */
            uint16_t sotto = (uint16_t)(hl - 0x120);
            aud_scrivi(sotto, aud_leggi(sotto) | 0x04);  /* set 2,(hl) sul blocco -9 */
        }
        aud_scrivi(hl + BL_FLAG, (aud_leggi(hl + BL_FLAG) & 0x04) | 0x01);
        uint16_t parola = (uint16_t)(aud_leggi(de + 1) | (aud_leggi(de + 2) << 8));
        de += 3;
        parola--;                                /* dec bc */
        aud_scrivi(hl + BL_PUNT, parola & 0xFF);
        aud_scrivi(hl + BL_PUNT + 1, parola >> 8);
        aud_scrivi(hl + BL_DURATA, 0x01);
        aud_scrivi(hl + BL_04, 0x00);
        /* ld a,4; add a,l; ld l,a: senza riporto, i blocchi sono allineati */
        aud_scrivi(hl + BL_VOLUME, 0x00);
        aud_scrivi(hl + BL_ATTENUA, 0x00);
        aud_scrivi(hl + BL_RUMORE, 0x00);
        aud_scrivi(hl + BL_16, 0x00);
        aud_scrivi(hl + BL_17, 0x00);
    }                                            /* jp 00FDh */
}

/* 0x00E4: smistatore dei comandi. c = (4243) = latch + 1; 0 = nessun
 * comando; azzera 4243; latch 00 → 00D6; 41-FE ignorati; 01-40 → voce del
 * comando nella tabella a 0AF3 (rst 8; rst 10h) e 00FA. */
void aud_00e4(void)
{
    uint8_t c = AM(AUD_COMANDO);
    if (c == 0) return;
    AM(AUD_COMANDO) = 0;
    c--;
    if (c == 0) {                                /* jr z,00D6h; il suo ret esce da qui */
        aud_00d6();
        return;
    }
    if (c >= 0x41) return;                       /* cp 41h; ret nc */
    uint16_t hl = aud_0008(AUD_TAB_COMANDI, c);  /* rst 8 */
    uint16_t de = aud_0010(&hl);                 /* rst 10h */
    aud_00fa(de);
}

/* 0x0038: IRQ (im 1). Salva i registri, ei, interprete e smistatore,
 * reti. In C i registri non servono. */
void aud_0038(void)
{
    aud_0147();
    aud_00e4();
}

/* 0x0066: NMI dal latch: (4243) = (8000) + 1, retn. */
void aud_0066(uint8_t latch)
{
    aud_latch = latch;                           /* il valore resta leggibile a 8000 */
    AM(AUD_COMANDO) = (uint8_t)(latch + 1);
}

/* 0x0008 (rst 8): hl += 2a con la somma a 8 bit di add a,a; d = 0, e = 2a. */
uint16_t aud_0008(uint16_t hl, uint8_t a)
{
    return (uint16_t)(hl + (uint8_t)(a + a));
}

/* 0x0010 (rst 10h): de = parola in (hl); hl avanza di 1 (punta al byte
 * alto, come lo lascia l'originale). */
uint16_t aud_0010(uint16_t *hl)
{
    uint8_t e = aud_leggi(*hl);
    (*hl)++;
    uint8_t d = aud_leggi(*hl);
    return (uint16_t)(e | (d << 8));
}

/* 0x0018 (rst 18h): il puntatore (ix+1/+2) avanza di 1; restituisce il
 * byte puntato. ix punta a un blocco in RAM. */
uint8_t aud_0018(uint16_t ix)
{
    uint16_t hl = (uint16_t)(AM(ix + 1) | (AM(ix + 2) << 8));
    hl++;
    AM(ix + 1) = hl & 0xFF;
    AM(ix + 2) = hl >> 8;
    return aud_leggi(hl);
}
