#!/bin/sh
# Confronto del renderizzatore C con l'oracolo video (lotto H2).
# Uso (da decomp/): sh rendi_video.sh [stato...]
# Senza argomenti: titolo gioco stanza10 bonus finale finale2 prove.
# Richiede harness/rendi_oracolo (make in harness/) e gli stati in out/video.
set -u
DIR=${VIDEO_OUT:-out/video}
[ $# -eq 0 ] && set -- titolo gioco stanza10 bonus finale finale2 prove
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

N=0; K=0
for S in "$@"; do
    N=$((N + 1))
    if python3 -I png_ppm.py "$DIR/$S.png" "$TMP/$S.ppm" &&
       harness/rendi_oracolo "$DIR" "$S" "$TMP/$S.ppm"; then
        K=$((K + 1))
    fi
done
echo "RENDI $K/$N identici"
[ "$K" -eq "$N" ]
