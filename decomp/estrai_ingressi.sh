#!/bin/sh
# Ingressi registrati (brief 44): dalle righe F dell'oracolo del replay
# estrae <frame> <e600> <e601> <e602> (esadecimali) per ogni NMI.
# I frame senza riga valgono 00 00 00. Uso (da decomp/): sh estrai_ingressi.sh
set -u
ORACOLO=out/replay_oracolo.txt
if [ ! -s "$ORACOLO" ]; then
    echo "ESTRAI_INGRESSI: $ORACOLO assente o vuoto (sh replay.sh)"
    exit 1
fi
awk '$1=="F"{print $2, $3, $4, $5}' "$ORACOLO" > out/ingressi_replay.txt
echo "ESTRAI_INGRESSI: $(wc -l < out/ingressi_replay.txt) righe in out/ingressi_replay.txt"
