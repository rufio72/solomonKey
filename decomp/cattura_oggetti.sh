#!/bin/sh
# Cattura prima/dopo nello stato «oggetti» (brief 30): lo stato gioco di
# cattura.sh con gli oggetti 08-1B in coda c05b e la posta 22, come la
# sessione «oggetti» di sessioni.sh, ripetuti due volte (14 terne, una posta
# ogni 100 frame da 2300 a 3600). Uso (da decomp/):
#   sh cattura_oggetti.sh <ENTRY> [N] [SUFFISSO]
# I file sono out/snap_<ENTRY>_gioco_oggetti[_SUFFISSO]_*.
set -u
POKE=
POSTA=
f=2250
for terna in "08 09 0a" "0b 0c 0d" "0e 0f 10" "11 12 13" "14 15 16" "17 18 19" "1a 1b 00" \
             "08 09 0a" "0b 0c 0d" "0e 0f 10" "11 12 13" "14 15 16" "17 18 19" "1a 1b 00"; do
    a=$((0xc05b))
    for o in $terna; do
        POKE="$POKE$f:$(printf 0x%x $a):0x$o,$f:$(printf 0x%x $((a + 1))):0,"
        a=$((a + 2))
    done
    POSTA="$POSTA$((f + 50)):0x22,"
    f=$((f + 100))
done
export SNAP_POKE="$POKE" SNAP_POSTA="$POSTA"
exec sh cattura.sh "$1" gioco "${2:-1}" "oggetti${3:+_$3}"
