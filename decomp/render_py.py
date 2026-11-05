"""Renderizzatore software del video in python (compito 60).

Stessa logica di harness/video_rendi.c (lotto H2), con numpy: schermo 32x32
celle 8x8, righe di schermo 16-239 visibili, BG sotto, FG sopra (pen 0
trasparente), sprite 16x16 dalla voce 31 alla 0.
"""
import os

import numpy as np

RIGA_PRIMA = 16                   # prima riga di schermo visibile
VID_LARGO, VID_ALTO = 256, 224

# offset nel dump c000-ffff di MAME (indirizzo - 0xc000) e lunghezze
BLOCCHI_DUMP = (('fg_colore', 0x1000, 1024), ('fg_video', 0x1400, 1024),
                ('bg_colore', 0x1800, 1024), ('bg_video', 0x1c00, 1024),
                ('spriteram', 0x2000, 128), ('palette', 0x2400, 512))


def _leggi(cartella, nome):
    with open(os.path.join(cartella, nome), 'rb') as f:
        return f.read()


def _pen_tile(dati):
    """Tile 8x8 a 4 bit: 32 byte per tile, 4 per riga, nibble alto se x pari."""
    b = np.frombuffer(dati, dtype=np.uint8).reshape(-1, 8, 4)
    pen = np.empty((b.shape[0], 8, 8), dtype=np.uint8)
    pen[:, :, 0::2] = b >> 4
    pen[:, :, 1::2] = b & 15
    return pen


def _pen_sprite(piani):
    """Sprite 16x16 da quattro piani: piano 0 = bit 3 del pen."""
    pen = None
    for i, dati in enumerate(piani):
        b = np.frombuffer(dati, dtype=np.uint8).reshape(-1, 4, 8)  # quadranti 8x8
        bit = np.unpackbits(b[:, :, :, None], axis=3).reshape(-1, 4, 8, 8)
        parte = bit.astype(np.uint8) << (3 - i)
        pen = parte if pen is None else pen | parte
    n = pen.shape[0]
    out = np.empty((n, 16, 16), dtype=np.uint8)
    # quadrante q: +16 se y>=8, +8 se x>=8
    out[:, 0:8, 0:8] = pen[:, 0]
    out[:, 0:8, 8:16] = pen[:, 1]
    out[:, 8:16, 0:8] = pen[:, 2]
    out[:, 8:16, 8:16] = pen[:, 3]
    return out


def carica_grafica(cartella_rom):
    """(car, fon, spr) come in video.h: byte concatenati dei ROM."""
    car = _leggi(cartella_rom, '12.3t') + _leggi(cartella_rom, '11.3r')
    fon = _leggi(cartella_rom, '10.3p') + _leggi(cartella_rom, '9.3m')
    spr = [_leggi(cartella_rom, n) for n in ('2.5lm', '3.6lm', '4.7lm', '5.8lm')]
    return car, fon, spr


def _decodifica(grafica):
    """Cache dei pen decodificati, una volta per oggetto grafica."""
    chiave = id(grafica)
    if _decodifica.cache.get(chiave, (None,))[0] is not grafica:
        car, fon, spr = grafica
        _decodifica.cache[chiave] = (grafica, _pen_tile(car), _pen_tile(fon),
                                     _pen_sprite(spr))
    return _decodifica.cache[chiave][1:]


_decodifica.cache = {}


def colori_palette(palette):
    """256 voci da 2 byte little-endian -> array (256, 3) RGB."""
    w = np.frombuffer(palette, dtype='<u2').astype(np.uint16)
    rgb = np.stack([(w & 15), (w >> 4) & 15, (w >> 8) & 15], axis=1) * 17
    return rgb.astype(np.uint8)


def rendi_frame(fg_colore, fg_video, bg_colore, bg_video, spriteram, palette, grafica):
    """Frame 224x256 RGB, come rendi_frame() di video_rendi.c."""
    car, fon, spr = _decodifica(grafica)
    col = colori_palette(palette)
    indice = np.zeros((256, 256), dtype=np.uint16)       # schermo intero
    for i in range(1024):
        a = bg_colore[i]
        t = fon[bg_video[i] + 256 * (a & 7)]
        if a & 0x80:
            t = t[:, ::-1]
        if a & 0x08:
            t = t[::-1, :]
        y, x = (i // 32) * 8, (i % 32) * 8
        indice[y:y + 8, x:x + 8] = 128 + 16 * ((a >> 4) & 7) + t
    for i in range(1024):
        a = fg_colore[i]
        t = car[fg_video[i] + 256 * (a & 7)]
        y, x = (i // 32) * 8, (i % 32) * 8
        zona = indice[y:y + 8, x:x + 8]
        np.copyto(zona, 16 * ((a >> 4) & 7) + t, where=t != 0)
    vis = indice[RIGA_PRIMA:RIGA_PRIMA + VID_ALTO].copy()
    for v in range(31, -1, -1):
        e = spriteram[4 * v:4 * v + 4]
        t = spr[e[0] + ((e[1] & 0x10) << 4)]
        if e[1] & 0x40:
            t = t[:, ::-1]
        if e[1] & 0x80:
            t = t[::-1, :]
        base = 16 * ((e[1] >> 1) & 7)
        sx, sy = e[3], 241 - e[2] - RIGA_PRIMA
        x0, x1 = max(sx, 0), min(sx + 16, VID_LARGO)
        y0, y1 = max(sy, 0), min(sy + 16, VID_ALTO)
        if x0 >= x1 or y0 >= y1:
            continue
        parte = t[y0 - sy:y1 - sy, x0 - sx:x1 - sx]
        np.copyto(vis[y0:y1, x0:x1], base + parte.astype(np.uint16), where=parte != 0)
    return col[vis]


def ram_da_dump(dump):
    """Sei blocchi (fg_colore, fg_video, bg_colore, bg_video, spriteram,
    palette) dal dump c000-ffff."""
    return tuple(bytes(dump[a:a + n]) for _, a, n in BLOCCHI_DUMP)


def scrivi_ppm(percorso, rgb):
    with open(percorso, 'wb') as f:
        f.write(b'P6\n%d %d\n255\n' % (rgb.shape[1], rgb.shape[0]))
        f.write(np.ascontiguousarray(rgb, dtype=np.uint8).tobytes())
