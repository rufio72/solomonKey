#!/bin/sh
# Disassembly statico dell'immagine 48 KB con z80dasm (etichette e indirizzi).
set -e
cd "$(dirname "$0")"
z80dasm -a -l -g 0 out/main48.bin > out/main48.dasm
z80dasm -a -l -g 0 out/main64.bin > out/main64.dasm
