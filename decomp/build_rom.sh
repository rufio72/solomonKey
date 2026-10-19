#!/bin/sh
# Costruisce le immagini del Z80 principale di Solomon's Key (set solomon).
# main48.bin = 6.3f + seconda metà di 7.3h + prima metà di 7.3h (0000-bfff)
# main64.bin = main48 + 12288 byte zero + primi 4096 byte di 8.3jk (f000-ffff)
set -e
cd "$(dirname "$0")"
CHIP=../roms/ext/solomon
mkdir -p out
{
  cat "$CHIP/6.3f"
  dd if="$CHIP/7.3h" bs=16384 skip=1 count=1 2>/dev/null
  dd if="$CHIP/7.3h" bs=16384 count=1 2>/dev/null
} > out/main48.bin
{
  cat out/main48.bin
  head -c 12288 /dev/zero
  head -c 4096 "$CHIP/8.3jk"
} > out/main64.bin
md5sum out/main48.bin out/main64.bin
