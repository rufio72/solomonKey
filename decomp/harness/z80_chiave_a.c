/* La chiave (compito 39), prima parte: lo smistamento 0CFD sulla tabella
 * ROM 0D18 e i gestori da 0D3C a 0EDF. IX = oggetto chiave (c4ce, slot da
 * 0x30 byte), IY = parola di stato della chiave (c299). Si riproducono
 * l'effetto sulla RAM e i registri A, BC, DE, HL, IX, IY all'uscita (le
 * bandiere F no): anche i registri «sporchi» seguono il disassemblato riga
 * per riga. Le tabelle dati (0D6E, 123C, 1246, 124F, 1258, 1261, 126A) si
 * leggono dalla ROM con leggi(). Il blocco 0CE6-1287 è in
 * out/chiave_n39.dasm; i gestori 0F6B-11D8 sono in z80_chiave_b.c. */
#include <stddef.h>

#include "routine.h"

/* Campi dell'oggetto (offsetti da IX) e della parola di stato (da IY). */
#define OGG_Y        0x0e   /* Y pixel (il nibble basso è la cella) */
#define OGG_X        0x15   /* X pixel */
#define OGG_TIPO     0x1a   /* il nibble basso sceglie il gestore */
#define OGG_STATO    0x1d
#define OGG_DIR      0x1e   /* bit di direzione/fase */
#define ST_USCITA_4  0x04
#define ST_USCITA_5  0x05

#define IX(n) M(z->ix + (n))
#define IY(n) M(z->iy + (n))

/* I 16 gestori nell'ordine della tabella ROM 0D18:
 * 0D3C 0D8E 0E1F 0EAC 0EDF 0F6B 0F75 0FA9 100E 109C 10CF 10D9 113C 1172
 * 11D8 0D38. */
static void (*const gestori_0d18[16])(struct z80 *) = {
    sub_0d3c, sub_0d8e, sub_0e1f, sub_0eac,
    sub_0edf, sub_0f6b, sub_0f75, sub_0fa9,
    sub_100e, sub_109c, sub_10cf, sub_10d9,
    sub_113c, sub_1172, sub_11d8, sub_0d38,
};

/* 0x0CFD: A = tipo & 0F, HL = parola ROM a 0D18 + 2A (l'indirizzo del
 * gestore), E = X, D = Y, B = 0, poi `jp (hl)`: qui la chiamata del
 * gestore nella tabella. Uscita: registri del gestore. */
void sub_0cfd(struct z80 *z)
{
    z->a = IX(OGG_TIPO) & 0x0f;                        /* 0CFD-0D01 */
    uint16_t voce = (uint16_t)(0x0d18 + 2 * z->a);     /* 0D02-0D0A */
    uint16_t gest = (uint16_t)(leggi(voce) | (leggi(voce + 1) << 8));
    SET_DE(z, (uint16_t)(voce + 1));                   /* E,D letti in 0D0B-0D0D */
    SET_HL(z, gest);                                   /* 0D0E: ex de,hl */
    z->e = IX(OGG_X);                                  /* 0D0F */
    z->d = IX(OGG_Y);                                  /* 0D12 */
    z->b = 0;                                          /* 0D15 */
    gestori_0d18[z->a](z);                             /* 0D17: jp (hl) */
}

/* 0x0D3C (tabella 0D18, indice 0): con lo stato diverso da 1 esce (B =
 * stato - 1). Altrimenti C = (IY+04) ruotato con il confronto del nibble
 * di Y (soglia 6, o 0A se il bit 1 di (IY+04)) e il bit 0 della direzione,
 * poi shift: BC = indice nella tabella 0D6E (16 coppie); stato = 0,
 * direzione = byte, (IY+05) = byte successivo. HL = voce + 1. */
void sub_0d3c(struct z80 *z)
{
    z->b = IX(OGG_STATO);                              /* 0D3C */
    z->b--;                                            /* 0D3F */
    if (z->b)
        return;                                        /* 0D40 */
    z->c = IY(ST_USCITA_4);                            /* 0D41 */
    z->b = 6;                                          /* 0D44 */
    if (z->c & 0x02)                                   /* 0D46 */
        z->b = 0x0a;                                   /* 0D4A */
    z->a = 0x0f & z->d;                                /* 0D4C-0D4E */
    int riporto = z->a < z->b;                         /* 0D4F */
    int vecchio7 = z->c >> 7;
    z->c = (uint8_t)((z->c << 1) | riporto);           /* 0D50: rl c */
    uint8_t dir = IX(OGG_DIR);                         /* 0D52 */
    z->a = (uint8_t)((dir >> 1) | (vecchio7 << 7));    /* 0D55: rra */
    riporto = dir & 1;
    z->c = (uint8_t)((z->c << 1) | riporto);           /* 0D56: rl c */
    z->b = 0;                                          /* 0D58 */
    z->c = (uint8_t)(z->c << 1);                       /* 0D5A: sla c */
    SET_HL(z, (uint16_t)(0x0d6e + BC(z)));             /* 0D5C-0D5F */
    z->a = leggi(HL(z));                               /* 0D60 */
    IX(OGG_STATO) = 0;                                 /* 0D61 */
    IX(OGG_DIR) = z->a;                                /* 0D65 */
    SET_HL(z, (uint16_t)(HL(z) + 1));                  /* 0D68 */
    z->a = leggi(HL(z));                               /* 0D69 */
    IY(ST_USCITA_5) = z->a;                            /* 0D6A */
}

/* 0x0D8E (indice 1): confronto su E, poi su D. Ramo alto: A = (E-2)&0F >=
 * 6, tabella 123C (indice A-6) con il nibble di Y; ramo basso: nibble di
 * Y >= 7, tabella 1246 con il nibble di X. Stato 1 (o fase da cambiare):
 * (IY+04) = 0, stato 1, direzione 0 (ramo alto) o 1 (basso). 0E0B cade
 * dentro questo gestore (etichetta l0e0b). */
void sub_0d8e(struct z80 *z)
{
    int riporto;
    z->a = (uint8_t)((z->e - 2) & 0x0f);               /* 0D8E-0D91 */
    if (z->a < 6)                                      /* 0D93 */
        goto l0dd6;
    z->a -= 6;                                         /* 0D97 */
    SET_HL(z, 0x123c);                                 /* 0D99 */
    z->c = z->a;                                       /* 0D9C */
    SET_HL(z, (uint16_t)(HL(z) + BC(z)));              /* 0D9D */
    z->a = z->d & 0x0f;                                /* 0D9E */
    z->l = leggi(HL(z));                               /* 0DA1 */
    riporto = z->a < z->l;                             /* 0DA2 */
    z->a = IX(OGG_STATO);                              /* 0DA3 */
    if (!riporto)
        goto l0dae;                                    /* 0DA6 */
    z->a--;                                            /* 0DA9 */
    if (z->a)
        return;                                        /* 0DAA */
    goto l0db2;                                        /* 0DAB */
l0dae:
    z->a--;                                            /* 0DAE */
    if (z->a)
        goto l0dbc;                                    /* 0DAF */
l0db2:
    if (!(IX(OGG_DIR) & 0x02))                         /* 0DB2 */
        goto l0dce;
    goto l0dc2;                                        /* 0DB9 */
l0dbc:
    z->a = IX(OGG_DIR);                                /* 0DBC */
    z->a--;                                            /* 0DBF */
    if (!z->a)
        goto l0dce;                                    /* 0DC0 */
l0dc2:
    IY(ST_USCITA_4) = 0;                               /* 0DC2 */
    IX(OGG_STATO) = 1;                                 /* 0DC6 */
    IX(OGG_DIR) = 0;                                   /* 0DCA */
l0dce:
    z->a = (uint8_t)((0xf0 & z->d) | z->l);            /* 0DCE-0DD1 */
    IX(OGG_Y) = z->a;                                  /* 0DD2 */
    return;                                            /* 0DD5 */
l0dd6:
    z->a = z->d & 0x0f;                                /* 0DD6 */
    if (z->a < 7)                                      /* 0DD9 */
        return;                                        /* 0DDB */
    z->a -= 7;                                         /* 0DDC */
    SET_HL(z, 0x1246);                                 /* 0DDE */
    z->c = z->a;                                       /* 0DE1 */
    z->b = 0;                                          /* 0DE2 */
    SET_HL(z, (uint16_t)(HL(z) + BC(z)));              /* 0DE4 */
    z->a = 0x0f & z->e;                                /* 0DE5-0DE7 */
    z->d = z->a;                                       /* 0DE8 */
    z->a = leggi(HL(z));                               /* 0DE9 */
    riporto = z->a < z->d;                             /* 0DEA */
    z->d = z->a;                                       /* 0DEB */
    z->a = IX(OGG_STATO);                              /* 0DEC */
    if (!riporto)
        goto l0df6;                                    /* 0DEF */
    z->a--;                                            /* 0DF1 */
    if (z->a)
        return;                                        /* 0DF2 */
    goto l0dfa;                                        /* 0DF3 */
l0df6:
    z->a--;                                            /* 0DF6 */
    if (z->a)
        goto l0e04;                                    /* 0DF7 */
l0dfa:
    if (!(IX(OGG_DIR) & 0x02))                         /* 0DFA */
        goto l0e17;
    goto l0e0b;                                        /* 0E01 */
l0e04:
    z->a = IX(OGG_DIR);                                /* 0E04 */
    if (z->a == 2)                                     /* 0E07 */
        goto l0e17;
l0e0b:
    IY(ST_USCITA_4) = 0;                               /* 0E0B */
    IX(OGG_STATO) = 1;                                 /* 0E0F */
    IX(OGG_DIR) = 1;                                   /* 0E13 */
l0e17:
    z->a = (uint8_t)((0xf0 & z->e) | z->d);            /* 0E17-0E1A */
    IX(OGG_X) = z->a;                                  /* 0E1B */
}

/* 0x0E1F (indice 2): come 0D8E con E-8: ramo alto (soglia 7) tabella 124F
 * su Y; ramo basso tabella 1258 su X. (IY+04) = 1, direzione 2 o 3. */
void sub_0e1f(struct z80 *z)
{
    int riporto;
    z->a = (uint8_t)((z->e - 8) & 0x0f);               /* 0E1F-0E22 */
    if (z->a < 7)                                      /* 0E24 */
        goto l0e67;
    z->a -= 7;                                         /* 0E28 */
    SET_HL(z, 0x124f);                                 /* 0E2A */
    z->c = z->a;                                       /* 0E2D */
    SET_HL(z, (uint16_t)(HL(z) + BC(z)));              /* 0E2E */
    z->a = z->d & 0x0f;                                /* 0E2F */
    z->e = leggi(HL(z));                               /* 0E32 */
    riporto = z->a < z->e;                             /* 0E33 */
    z->a = IX(OGG_STATO);                              /* 0E34 */
    if (!riporto)
        goto l0e3f;                                    /* 0E37 */
    z->a--;                                            /* 0E3A */
    if (z->a)
        return;                                        /* 0E3B */
    goto l0e43;                                        /* 0E3C */
l0e3f:
    z->a--;                                            /* 0E3F */
    if (z->a)
        goto l0e4d;                                    /* 0E40 */
l0e43:
    if (IX(OGG_DIR) & 0x02)                            /* 0E43 */
        goto l0e5f;
    goto l0e53;                                        /* 0E4A */
l0e4d:
    z->a = IX(OGG_DIR);                                /* 0E4D */
    if (z->a == 0)                                     /* 0E50 */
        goto l0e5f;
l0e53:
    IY(ST_USCITA_4) = 1;                               /* 0E53 */
    IX(OGG_STATO) = 1;                                 /* 0E57 */
    IX(OGG_DIR) = 2;                                   /* 0E5B */
l0e5f:
    z->a = (uint8_t)((0xf0 & z->d) | z->e);            /* 0E5F-0E62 */
    IX(OGG_Y) = z->a;                                  /* 0E63 */
    return;                                            /* 0E66 */
l0e67:
    z->a = z->d & 0x0f;                                /* 0E67 */
    if (z->a < 7)                                      /* 0E6A */
        return;                                        /* 0E6C */
    z->a -= 7;                                         /* 0E6D */
    SET_HL(z, 0x1258);                                 /* 0E6F */
    z->c = z->a;                                       /* 0E72 */
    SET_HL(z, (uint16_t)(HL(z) + BC(z)));              /* 0E73 */
    z->a = 0x0f & z->e;                                /* 0E74-0E76 */
    z->d = leggi(HL(z));                               /* 0E77 */
    riporto = z->a < z->d;                             /* 0E78 */
    z->a = IX(OGG_STATO);                              /* 0E79 */
    if (!riporto)
        goto l0e83;                                    /* 0E7C */
    z->a--;                                            /* 0E7E */
    if (z->a)
        return;                                        /* 0E7F */
    goto l0e87;                                        /* 0E80 */
l0e83:
    z->a--;                                            /* 0E83 */
    if (z->a)
        goto l0e91;                                    /* 0E84 */
l0e87:
    if (IX(OGG_DIR) & 0x02)                            /* 0E87 */
        goto l0ea4;
    goto l0e98;                                        /* 0E8E */
l0e91:
    z->a = IX(OGG_DIR);                                /* 0E91 */
    if (z->a == 2)                                     /* 0E94 */
        goto l0ea4;
l0e98:
    IY(ST_USCITA_4) = 1;                               /* 0E98 */
    IX(OGG_STATO) = 1;                                 /* 0E9C */
    IX(OGG_DIR) = 3;                                   /* 0EA0 */
l0ea4:
    z->a = (uint8_t)((0xf0 & z->e) | z->d);            /* 0EA4-0EA7 */
    IX(OGG_X) = z->a;                                  /* 0EA8 */
}

/* 0x0EAC (indice 3): con lo stato 1 salta a 0EC1 (direzione = bit 1
 * ? 0 : 1, stato 0); altrimenti esce se il nibble di Y < 4, e prende la
 * direzione da (IY+05). Poi Y = (Y & F0) + 3 e (IY+05) = 3. */
void sub_0eac(struct z80 *z)
{
    z->a = IX(OGG_STATO);                              /* 0EAC */
    z->a--;                                            /* 0EAF */
    if (!z->a)
        goto l0ec1;                                    /* 0EB0 */
    z->a = z->d & 0x0f;                                /* 0EB2 */
    if (z->a < 4)                                      /* 0EB5 */
        return;                                        /* 0EB7 */
    z->a = IY(ST_USCITA_5);                            /* 0EB8 */
    IX(OGG_DIR) = z->a;                                /* 0EBB */
    goto l0ed2;                                        /* 0EBE */
l0ec1:
    z->c = (IX(OGG_DIR) & 0x02) ? 0 : 1;               /* 0EC1-0EC9 */
    IX(OGG_STATO) = 0;                                 /* 0ECB */
    IX(OGG_DIR) = z->c;                                /* 0ECF */
l0ed2:
    z->a = (uint8_t)((0xf0 & z->d) + 3);               /* 0ED2-0ED5 */
    IX(OGG_Y) = z->a;                                  /* 0ED7 */
    IY(ST_USCITA_5) = 3;                               /* 0EDA */
}

/* 0x0EDF (indice 4): come 0D8E con E-8: ramo alto (soglia 7) tabella 1261
 * su Y, ramo basso tabella 126A (nibble di Y < 0A) su X. (IY+04) = 2. */
void sub_0edf(struct z80 *z)
{
    int riporto;
    z->a = (uint8_t)((z->e - 8) & 0x0f);               /* 0EDF-0EE2 */
    if (z->a < 7)                                      /* 0EE4 */
        goto l0f28;
    z->a -= 7;                                         /* 0EE8 */
    SET_HL(z, 0x1261);                                 /* 0EEA */
    z->c = z->a;                                       /* 0EED */
    SET_HL(z, (uint16_t)(HL(z) + BC(z)));              /* 0EEE */
    z->a = z->d & 0x0f;                                /* 0EEF */
    z->e = z->a;                                       /* 0EF2 */
    z->a = leggi(HL(z));                               /* 0EF3 */
    riporto = z->a < z->e;                             /* 0EF4 */
    z->e = z->a;                                       /* 0EF5 */
    z->a = IX(OGG_STATO);                              /* 0EF6 */
    if (!riporto)
        goto l0f00;                                    /* 0EF9 */
    z->a--;                                            /* 0EFB */
    if (z->a)
        return;                                        /* 0EFC */
    goto l0f04;                                        /* 0EFD */
l0f00:
    z->a--;                                            /* 0F00 */
    if (z->a)
        goto l0f0e;                                    /* 0F01 */
l0f04:
    if (!(IX(OGG_DIR) & 0x02))                         /* 0F04 */
        goto l0f20;
    goto l0f14;                                        /* 0F0B */
l0f0e:
    z->a = IX(OGG_DIR);                                /* 0F0E */
    if (z->a == 0)                                     /* 0F11 */
        goto l0f20;
l0f14:
    IY(ST_USCITA_4) = 2;                               /* 0F14 */
    IX(OGG_STATO) = 1;                                 /* 0F18 */
    IX(OGG_DIR) = 1;                                   /* 0F1C */
l0f20:
    z->a = (uint8_t)((0xf0 & z->d) | z->e);            /* 0F20-0F23 */
    IX(OGG_Y) = z->a;                                  /* 0F24 */
    return;                                            /* 0F27 */
l0f28:
    z->a = z->d & 0x0f;                                /* 0F28 */
    if (z->a >= 0x0a)                                  /* 0F2B */
        return;                                        /* 0F2D */
    SET_HL(z, 0x126a);                                 /* 0F2E */
    z->c = z->a;                                       /* 0F31 */
    SET_HL(z, (uint16_t)(HL(z) + BC(z)));              /* 0F32 */
    z->a = 0x0f & z->e;                                /* 0F33-0F35 */
    z->d = leggi(HL(z));                               /* 0F36 */
    riporto = z->a < z->d;                             /* 0F37 */
    z->a = IX(OGG_STATO);                              /* 0F38 */
    if (!riporto)
        goto l0f42;                                    /* 0F3B */
    z->a--;                                            /* 0F3D */
    if (z->a)
        return;                                        /* 0F3E */
    goto l0f46;                                        /* 0F3F */
l0f42:
    z->a--;                                            /* 0F42 */
    if (z->a)
        goto l0f50;                                    /* 0F43 */
l0f46:
    if (!(IX(OGG_DIR) & 0x02))                         /* 0F46 */
        goto l0f63;
    goto l0f57;                                        /* 0F4D */
l0f50:
    z->a = IX(OGG_DIR);                                /* 0F50 */
    if (z->a == 3)                                     /* 0F53 */
        goto l0f63;
l0f57:
    IY(ST_USCITA_4) = 2;                               /* 0F57 */
    IX(OGG_STATO) = 1;                                 /* 0F5B */
    IX(OGG_DIR) = 0;                                   /* 0F5F */
l0f63:
    z->a = (uint8_t)((0xf0 & z->e) | z->d);            /* 0F63-0F66 */
    IX(OGG_X) = z->a;                                  /* 0F67 */
}
