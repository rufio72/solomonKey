# Hash dei file dell'oracolo video, per confrontare due corse.
# Uso: python3 -I verifica_video.py [DIR]   (predefinito out/video)
# PNG: dimensioni e sha1 dei soli pixel RGB (i metadati possono differire).
import hashlib
import os
import sys

from PIL import Image

cartella = sys.argv[1] if len(sys.argv) > 1 else "out/video"
for nome in sorted(os.listdir(cartella)):
    p = os.path.join(cartella, nome)
    if nome.endswith(".png"):
        im = Image.open(p).convert("RGB")
        print(f"{nome[:-4]} {im.width}x{im.height} {hashlib.sha1(im.tobytes()).hexdigest()}")
    elif nome.endswith(".bin"):
        d = open(p, "rb").read()
        print(f"{nome} {len(d)} {hashlib.sha1(d).hexdigest()}")
