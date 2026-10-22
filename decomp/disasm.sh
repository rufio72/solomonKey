#!/bin/sh
# Disassembly statico con z80dasm (etichette e indirizzi): Z80 principale
# (immagini 48 e 64 KB) e Z80 audio (ROM 1.3jk, 16 KB).
set -e
cd "$(dirname "$0")"
z80dasm -a -l -g 0 out/main48.bin > out/main48.dasm
z80dasm -a -l -g 0 out/main64.bin > out/main64.dasm
z80dasm -a -l -g 0 ../roms/ext/solomon/1.3jk > out/audio.dasm
