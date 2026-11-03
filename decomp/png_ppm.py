#!/usr/bin/env python3
"""Converte un PNG in PPM P6 (usato dall'oracolo video, lotto H2).

Uso: python3 -I png_ppm.py <in.png> <out.ppm>
"""
import sys

from PIL import Image


def main():
    if len(sys.argv) != 3:
        sys.exit('uso: png_ppm.py <in.png> <out.ppm>')
    im = Image.open(sys.argv[1]).convert('RGB')
    with open(sys.argv[2], 'wb') as f:
        f.write(b'P6\n%d %d\n255\n' % im.size)
        f.write(im.tobytes())


main()
