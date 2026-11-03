/* La chiave (compito 39), seconda parte: i gestori della tabella 0D18 da
 * 0F6B a 11D8 più 0D38. IX = oggetto chiave (c4ce), IY = parola di stato
 * (c299). Si riproducono l'effetto sulla RAM e i registri A, BC, DE, HL,
 * IX, IY all'uscita (F no); le tabelle dati si leggono con leggi(). Il
 * dispatcher 0CFD e i gestori 0D3C-0EDF sono in z80_chiave_a.c; il blocco
 * è in out/chiave_n39.dasm. */
#include <stddef.h>

#include "routine.h"

/* Campi dell'oggetto (offsetti da IX) e della parola di stato (da IY). */
#define OGG_Y        0x0e
#define OGG_X        0x15
#define OGG_STATO    0x1d
#define OGG_DIR      0x1e
#define ST_USCITA_4  0x04
#define ST_USCITA_5  0x05

#define IX(n) M(z->ix + (n))
#define IY(n) M(z->iy + (n))

/* 0x0D38 (indice F): nel Z80 è `ld (0000),a; ret`, una scrittura in ROM
 * senza effetto in MAME; in C non si scrive (M(0) sarebbe fuori array). */
void sub_0d38(struct z80 *z)
{
    (void)z;
}

/* 0x0F6B (indice 5): bit 3 di Y spento -> 0FA9, acceso -> 1172. */
void sub_0f6b(struct z80 *z)
{
    if (!(IX(OGG_Y) & 0x08))                           /* 0F6B-0F6F */
        sub_0fa9(z);
    else
        sub_1172(z);                                   /* 0F72 */
}

/* 0x0F75 (indice 6): stato 1 -> 0F8A (direzione = bit 1 ? 2 : 3, stato 0);
 * altrimenti esce se il nibble di X < 0C e prende la direzione da
 * (IY+05). Poi X = (X & F0) + 0B e (IY+05) = 1. */
void sub_0f75(struct z80 *z)
{
    z->a = IX(OGG_STATO);                              /* 0F75 */
    z->a--;                                            /* 0F78 */
    if (!z->a)
        goto l0f8a;                                    /* 0F79 */
    z->a = z->e & 0x0f;                                /* 0F7B */
    if (z->a < 0x0c)                                   /* 0F7E */
        return;                                        /* 0F80 */
    z->a = IY(ST_USCITA_5);                            /* 0F81 */
    IX(OGG_DIR) = z->a;                                /* 0F84 */
    goto l0f9c;                                        /* 0F87 */
l0f8a:
    z->c = (IX(OGG_DIR) & 0x02) ? 2 : 3;               /* 0F8A-0F93 */
    IX(OGG_STATO) = 0;                                 /* 0F95 */
    IX(OGG_DIR) = z->c;                                /* 0F99 */
l0f9c:
    z->a = (uint8_t)((0xf0 & z->e) + 0x0b);            /* 0F9C-0F9F */
    IX(OGG_X) = z->a;                                  /* 0FA1 */
    IY(ST_USCITA_5) = 1;                               /* 0FA4 */
}

/* 0x0FA9 (indice 7, o da 0F6B): B = stato - 1 e i suoi bit 0-2 sono flag:
 * bit 1 = Y da riposizionare, bit 0 = X da riposizionare (i flag si
 * spengono se il nibble ha già superato la soglia), bit 2 = aggiorna la
 * direzione da (IY+05).
 * Con stato 1 (B = 0) sceglie direzione e (IY+05) dal bit 0 della
 * direzione. Y = (Y & F0) + 3, X = (X & F0) + 0B. */
void sub_0fa9(struct z80 *z)
{
    z->b = IX(OGG_STATO);                              /* 0FA9 */
    z->b--;                                            /* 0FAC */
    if (z->b)
        goto l0fc8;                                    /* 0FAD */
    z->c = 2;                                          /* 0FB0 */
    z->a = 1;                                          /* 0FB2 */
    if (IX(OGG_DIR) & 0x01) {                          /* 0FB4 */
        z->c = 0;                                      /* 0FBA */
        z->a = 3;                                      /* 0FBC */
    }
    IX(OGG_STATO) = 0;                                 /* 0FBE */
    IX(OGG_DIR) = z->c;                                /* 0FC2 */
    IY(ST_USCITA_5) = z->a;                            /* 0FC5 */
l0fc8:
    if (!(z->b & 0x02))                                /* 0FC8 */
        goto l0fd5;
    z->a = z->d & 0x0f;                                /* 0FCC */
    if (z->a < 4)                                      /* 0FCF */
        goto l0fdd;
    z->b &= (uint8_t)~0x02;                            /* 0FD3 */
l0fd5:
    z->a = (uint8_t)((0xf0 & z->d) + 3);               /* 0FD5-0FD8 */
    IX(OGG_Y) = z->a;                                  /* 0FDA */
l0fdd:
    if (!(z->b & 0x01))                                /* 0FDD */
        goto l0fea;
    z->a = z->e & 0x0f;                                /* 0FE1 */
    if (z->a < 0x0c)                                   /* 0FE4 */
        goto l0ff2;
    z->b &= (uint8_t)~0x01;                            /* 0FE8 */
l0fea:
    z->a = (uint8_t)((0xf0 & z->e) + 0x0b);            /* 0FEA-0FED */
    IX(OGG_X) = z->a;                                  /* 0FEF */
l0ff2:
    if (!(z->b & 0x04))                                /* 0FF2 */
        return;                                        /* 0FF4 */
    z->a = 3 & z->b;                                   /* 0FF5-0FF7 */
    if (z->a == 3)                                     /* 0FF8 */
        return;                                        /* 0FFA */
    z->a = IY(ST_USCITA_5);                            /* 0FFC (dopo rra) */
    IX(OGG_DIR) = z->a;                                /* 0FFF */
    if (z->b & 0x01) {                                 /* 1002: riporto di rra */
        IY(ST_USCITA_5) = 3;                           /* 1004 */
        return;
    }
    IY(ST_USCITA_5) = 1;                               /* 1009 */
}

/* 0x100E (indice 8): come 0D8E con E-2: ramo alto (soglia 6) tabella 1274
 * su Y, ramo basso tabella 127E (nibble di Y < 0A) su X. (IY+04) = 3. */
void sub_100e(struct z80 *z)
{
    int riporto;
    z->a = (uint8_t)((z->e - 2) & 0x0f);               /* 100E-1011 */
    if (z->a < 6)                                      /* 1013 */
        goto l1057;
    z->a -= 6;                                         /* 1017 */
    SET_HL(z, 0x1274);                                 /* 1019 */
    z->c = z->a;                                       /* 101C */
    SET_HL(z, (uint16_t)(HL(z) + BC(z)));              /* 101D */
    z->a = z->d & 0x0f;                                /* 101E */
    z->e = z->a;                                       /* 1021 */
    z->a = leggi(HL(z));                               /* 1022 */
    riporto = z->a < z->e;                             /* 1023 */
    z->e = z->a;                                       /* 1024 */
    z->a = IX(OGG_STATO);                              /* 1025 */
    if (!riporto)
        goto l102f;                                    /* 1028 */
    z->a--;                                            /* 102A */
    if (z->a)
        return;                                        /* 102B */
    goto l1033;                                        /* 102C */
l102f:
    z->a--;                                            /* 102F */
    if (z->a)
        goto l103d;                                    /* 1030 */
l1033:
    if (IX(OGG_DIR) & 0x02)                            /* 1033 */
        goto l104f;
    goto l1043;                                        /* 103A */
l103d:
    z->a = IX(OGG_DIR);                                /* 103D */
    z->a--;                                            /* 1040 */
    if (!z->a)
        goto l104f;                                    /* 1041 */
l1043:
    IY(ST_USCITA_4) = 3;                               /* 1043 */
    IX(OGG_STATO) = 1;                                 /* 1047 */
    IX(OGG_DIR) = 3;                                   /* 104B */
l104f:
    z->a = (uint8_t)((0xf0 & z->d) | z->e);            /* 104F-1052 */
    IX(OGG_Y) = z->a;                                  /* 1053 */
    return;                                            /* 1056 */
l1057:
    z->a = z->d & 0x0f;                                /* 1057 */
    if (z->a >= 0x0a)                                  /* 105A */
        return;                                        /* 105C */
    SET_HL(z, 0x127e);                                 /* 105D */
    z->c = z->a;                                       /* 1060 */
    SET_HL(z, (uint16_t)(HL(z) + BC(z)));              /* 1061 */
    z->a = 0x0f & z->e;                                /* 1062-1064 */
    z->d = z->a;                                       /* 1065 */
    z->a = leggi(HL(z));                               /* 1066 */
    riporto = z->a < z->d;                             /* 1067 */
    z->d = z->a;                                       /* 1068 */
    z->a = IX(OGG_STATO);                              /* 1069 */
    if (!riporto)
        goto l1073;                                    /* 106C */
    z->a--;                                            /* 106E */
    if (z->a)
        return;                                        /* 106F */
    goto l1077;                                        /* 1070 */
l1073:
    z->a--;                                            /* 1073 */
    if (z->a)
        goto l1081;                                    /* 1074 */
l1077:
    if (IX(OGG_DIR) & 0x02)                            /* 1077 */
        goto l1094;
    goto l1088;                                        /* 107E */
l1081:
    z->a = IX(OGG_DIR);                                /* 1081 */
    if (z->a == 3)                                     /* 1084 */
        goto l1094;
l1088:
    IY(ST_USCITA_4) = 3;                               /* 1088 */
    IX(OGG_STATO) = 1;                                 /* 108C */
    IX(OGG_DIR) = 2;                                   /* 1090 */
l1094:
    z->a = (uint8_t)((0xf0 & z->e) | z->d);            /* 1094-1097 */
    IX(OGG_X) = z->a;                                  /* 1098 */
}

/* 0x109C (indice 9): come 0EAC su E: con stato 1 -> direzione = bit 1 ? 3
 * : 2, stato 0; altrimenti esce se il nibble di X >= 5 e prende la
 * direzione da (IY+05). Poi X = (X & F0) + 5 e (IY+05) = 0. */
void sub_109c(struct z80 *z)
{
    z->a = IX(OGG_STATO);                              /* 109C */
    z->a--;                                            /* 109F */
    if (!z->a)
        goto l10b1;                                    /* 10A0 */
    z->a = z->e & 0x0f;                                /* 10A2 */
    if (z->a >= 5)                                     /* 10A5 */
        return;                                        /* 10A7 */
    z->a = IY(ST_USCITA_5);                            /* 10A8 */
    IX(OGG_DIR) = z->a;                                /* 10AB */
    goto l10c2;                                        /* 10AE */
l10b1:
    z->c = (IX(OGG_DIR) & 0x02) ? 3 : 2;               /* 10B1-10BA */
    IX(OGG_STATO) = 0;                                 /* 10BB */
    IX(OGG_DIR) = z->c;                                /* 10BF */
l10c2:
    z->a = (uint8_t)((0xf0 & z->e) + 5);               /* 10C2-10C5 */
    IX(OGG_X) = z->a;                                  /* 10C7 */
    IY(ST_USCITA_5) = 0;                               /* 10CA */
}

/* 0x10CF (indice A): bit 3 di Y spento -> 10D9, acceso -> 11D8. */
void sub_10cf(struct z80 *z)
{
    if (!(IX(OGG_Y) & 0x08))                           /* 10CF-10D3 */
        sub_10d9(z);
    else
        sub_11d8(z);                                   /* 10D6 */
}

/* 0x10D9 (indice B, o da 10CF): come 0FA9 sui valori X = (X & F0) + 5 e
 * Y = (Y & F0) + 3; il flag 0 è X, il flag 1 è Y. */
void sub_10d9(struct z80 *z)
{
    z->b = IX(OGG_STATO);                              /* 10D9 */
    z->b--;                                            /* 10DC */
    if (z->b)
        goto l10f6;                                    /* 10DD */
    z->c = 2;                                          /* 10DF */
    z->a = 0;                                          /* 10E1 */
    if (IX(OGG_DIR) & 0x01) {                          /* 10E3 */
        z->c--;                                        /* 10E9 */
        z->a = 3;                                      /* 10EA */
    }
    IX(OGG_STATO) = 0;                                 /* 10EC */
    IX(OGG_DIR) = z->c;                                /* 10F0 */
    IY(ST_USCITA_5) = z->a;                            /* 10F3 */
l10f6:
    if (!(z->b & 0x01))                                /* 10F6 */
        goto l1103;
    z->a = z->e & 0x0f;                                /* 10FA-10FC */
    if (z->a >= 5)                                     /* 10FD */
        goto l110b;
    z->b &= (uint8_t)~0x01;                            /* 1101 */
l1103:
    z->a = (uint8_t)((0xf0 & z->e) + 5);               /* 1103-1106 */
    IX(OGG_X) = z->a;                                  /* 1108 */
l110b:
    if (!(z->b & 0x02))                                /* 110B */
        goto l1118;
    z->a = z->d & 0x0f;                                /* 110F-1111 */
    if (z->a < 4)                                      /* 1112 */
        goto l1120;
    z->b &= (uint8_t)~0x02;                            /* 1116 */
l1118:
    z->a = (uint8_t)((0xf0 & z->d) + 3);               /* 1118-111B */
    IX(OGG_Y) = z->a;                                  /* 111D */
l1120:
    if (!(z->b & 0x04))                                /* 1120 */
        return;                                        /* 1122 */
    z->a = 3 & z->b;                                   /* 1123-1125 */
    if (z->a == 3)                                     /* 1126 */
        return;                                        /* 1128 */
    z->a = IY(ST_USCITA_5);                            /* 112A (dopo rra) */
    IX(OGG_DIR) = z->a;                                /* 112D */
    if (z->b & 0x01) {                                 /* 1130: riporto di rra */
        IY(ST_USCITA_5) = 3;                           /* 1132 */
        return;
    }
    IY(ST_USCITA_5) = 0;                               /* 1137 */
}

/* 0x113C (indice C): come 0EAC su Y con soglia 0D (esce se il nibble di Y
 * >= 0D): stato 1 -> direzione = bit 1 ? 1 : 0; Y = (Y & F0) + 0D,
 * (IY+05) = 2. */
void sub_113c(struct z80 *z)
{
    z->a = IX(OGG_STATO);                              /* 113C */
    z->a--;                                            /* 113F */
    if (!z->a)
        goto l1155;                                    /* 1140 */
    z->a = z->d & 0x0f;                                /* 1142 */
    if (z->a >= 0x0d)                                  /* 1145 */
        return;                                        /* 1147 */
    z->a = IY(ST_USCITA_5);                            /* 1148 */
    IX(OGG_DIR) = z->a;                                /* 114B */
    IY(ST_USCITA_5) = 2;                               /* 114E */
    goto l1165;                                        /* 1152 */
l1155:
    z->c = (IX(OGG_DIR) & 0x02) ? 1 : 0;               /* 1155-115D */
    IX(OGG_STATO) = 0;                                 /* 115E */
    IX(OGG_DIR) = z->c;                                /* 1162 */
l1165:
    z->a = (uint8_t)((0xf0 & z->d) + 0x0d);            /* 1165-1168 */
    IX(OGG_Y) = z->a;                                  /* 116A */
    IY(ST_USCITA_5) = 2;                               /* 116D */
}

/* 0x1172 (indice D, o da 0F6B): come 0FA9 su X = (X & F0) + 5 e Y =
 * (Y & F0) + 0D. 11A5 è un'etichetta interna (flag 1). */
void sub_1172(struct z80 *z)
{
    z->b = IX(OGG_STATO);                              /* 1172 */
    z->b--;                                            /* 1175 */
    if (z->b)
        goto l118f;                                    /* 1176 */
    z->c = 3;                                          /* 1178 */
    z->a = 0;                                          /* 117A */
    if (!(IX(OGG_DIR) & 0x01)) {                       /* 117B */
        z->c = 1;                                      /* 1181 */
        z->a = 2;                                      /* 1183 */
    }
    IX(OGG_STATO) = 0;                                 /* 1185 */
    IX(OGG_DIR) = z->c;                                /* 1189 */
    IY(ST_USCITA_5) = z->a;                            /* 118C */
l118f:
    if (!(z->b & 0x01))                                /* 118F */
        goto l119d;
    z->a = z->e & 0x0f;                                /* 1193-1195 */
    if (z->a >= 5)                                     /* 1196 */
        goto l11a5;
    z->b &= (uint8_t)~0x01;                            /* 119B */
l119d:
    z->a = (uint8_t)((0xf0 & z->e) + 5);               /* 119D-11A0 */
    IX(OGG_X) = z->a;                                  /* 11A2 */
l11a5:
    if (!(z->b & 0x02))                                /* 11A5 */
        goto l11b3;
    z->a = z->d & 0x0f;                                /* 11A9-11AB */
    if (z->a >= 0x0d)                                  /* 11AC */
        goto l11bb;
    z->b &= (uint8_t)~0x02;                            /* 11B1 */
l11b3:
    z->a = (uint8_t)((0xf0 & z->d) + 0x0d);            /* 11B3-11B6 */
    IX(OGG_Y) = z->a;                                  /* 11B8 */
l11bb:
    if (!(z->b & 0x04))                                /* 11BB */
        return;                                        /* 11BD */
    z->a = 3 & z->b;                                   /* 11BE-11C0 */
    if (z->a == 3)                                     /* 11C1 */
        return;                                        /* 11C3 */
    z->a = IY(ST_USCITA_5);                            /* 11C5 (dopo rra) */
    IX(OGG_DIR) = z->a;                                /* 11C8 */
    if (z->b & 0x01) {                                 /* 11CB: riporto di rra */
        IY(ST_USCITA_5) = 2;                           /* 11CD */
        return;
    }
    IY(ST_USCITA_5) = 0;                               /* 11D2 */
}

/* 0x11D8 (indice E, o da 10CF): come 10D9 su Y = (Y & F0) + 0D e X =
 * (X & F0) + 0B; il flag 1 è Y, il flag 0 è X. */
void sub_11d8(struct z80 *z)
{
    z->b = IX(OGG_STATO);                              /* 11D8 */
    z->b--;                                            /* 11DB */
    if (z->b)
        goto l11f6;                                    /* 11DC */
    z->c = 3;                                          /* 11DE */
    z->a = 1;                                          /* 11E0 */
    if (!(IX(OGG_DIR) & 0x01)) {                       /* 11E2 */
        z->c = 0;                                      /* 11E8 */
        z->a = 2;                                      /* 11EA */
    }
    IX(OGG_STATO) = 0;                                 /* 11EC */
    IX(OGG_DIR) = z->c;                                /* 11F0 */
    IY(ST_USCITA_5) = z->a;                            /* 11F3 */
l11f6:
    if (!(z->b & 0x02))                                /* 11F6 */
        goto l1203;
    z->a = z->d & 0x0f;                                /* 11FA-11FC */
    if (z->a >= 0x0d)                                  /* 11FD */
        goto l120b;
    z->b &= (uint8_t)~0x02;                            /* 1201 */
l1203:
    z->a = (uint8_t)((0xf0 & z->d) + 0x0d);            /* 1203-1206 */
    IX(OGG_Y) = z->a;                                  /* 1208 */
l120b:
    if (!(z->b & 0x01))                                /* 120B */
        goto l1218;
    z->a = z->e & 0x0f;                                /* 120F-1211 */
    if (z->a < 0x0c)                                   /* 1212 */
        goto l1220;
    z->b &= (uint8_t)~0x01;                            /* 1216 */
l1218:
    z->a = (uint8_t)((0xf0 & z->e) + 0x0b);            /* 1218-121B */
    IX(OGG_X) = z->a;                                  /* 121D */
l1220:
    if (!(z->b & 0x04))                                /* 1220 */
        return;                                        /* 1222 */
    z->a = 3 & z->b;                                   /* 1223-1225 */
    if (z->a == 3)                                     /* 1226 */
        return;                                        /* 1228 */
    z->a = IY(ST_USCITA_5);                            /* 122A (dopo rra) */
    IX(OGG_DIR) = z->a;                                /* 122D */
    if (z->b & 0x01) {                                 /* 1230: riporto di rra */
        IY(ST_USCITA_5) = 2;                           /* 1232 */
        return;
    }
    IY(ST_USCITA_5) = 1;                               /* 1237 */
}
