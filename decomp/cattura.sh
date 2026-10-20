#!/bin/sh
# Cattura prima/dopo di una routine del Z80 principale in uno stato del gioco.
# Uso (da decomp/): sh cattura.sh <ENTRY> <stato> [N] [SUFFISSO]
#   ENTRY: indirizzo come 0x30EF; stato: titolo | gioco | tardo; N: occorrenza (1);
#   SUFFISSO: se presente il tag è <stato>_<SUFFISSO> (file
#   out/snap_<ENTRY>_<stato>_<SUFFISSO>_*). SNAP_COND dall'ambiente passa a
#   snap.lua la condizione del breakpoint (es. SNAP_COND='a>=#10').
# Prova i frame di partenza dello stato in ordine e si ferma al primo che
# produce out/snap_<ENTRY>_<tag>_regs.txt. MAME senza finestra né audio.
set -u
if [ $# -lt 2 ]; then
    echo "uso: sh cattura.sh <ENTRY> <stato> [N] [SUFFISSO]" >&2
    exit 2
fi
ENTRY=$1
STATO=$2
N=${3:-1}
SUFFISSO=${4:-}
TAG=$STATO
[ -n "$SUFFISSO" ] && TAG=${STATO}_$SUFFISSO
case "$STATO" in
    titolo) INGRESSI=nessuno; FRAMES="1000 650 0" ;;
    gioco)  INGRESSI=traccia; FRAMES="2400 2000 1700 1600 1500" ;;
    tardo)  INGRESSI=traccia; FRAMES="4000 3000" ;;
    *) echo "stato sconosciuto: $STATO (titolo, gioco, tardo)" >&2; exit 2 ;;
esac
ROMPATH="$(cd .. && pwd)/roms"
mkdir -p out
for F in $FRAMES; do
    rm -f out/snap_${ENTRY}_${TAG}_prima.bin out/snap_${ENTRY}_${TAG}_dopo.bin \
          out/snap_${ENTRY}_${TAG}_regs.txt
    SNAP_ENTRY=$ENTRY SNAP_TAG=$TAG SNAP_INGRESSI=$INGRESSI SNAP_DA_FRAME=$F SNAP_N=$N \
        mame solomon -rompath "$ROMPATH" -video none -sound none \
        -skip_gameinfo -nothrottle -debug -debugger none -autoboot_script snap.lua \
        > out/snap_${ENTRY}_${TAG}.log 2>&1
    if [ -f out/snap_${ENTRY}_${TAG}_regs.txt ]; then
        echo "CATTURA $ENTRY $TAG frame_da=$F OK"
        exit 0
    fi
done
echo "CATTURA $ENTRY $TAG FALLITA"
exit 1
