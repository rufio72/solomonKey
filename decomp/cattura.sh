#!/bin/sh
# Cattura prima/dopo di una routine del Z80 principale in uno stato del gioco.
# Uso (da decomp/): sh cattura.sh <ENTRY> <stato> [N] [SUFFISSO]
#   ENTRY: indirizzo come 0x30EF; stato: titolo | gioco | tardo | porta | bonus |
#   finale; N: occorrenza (1);
#   SUFFISSO: se presente il tag è <stato>_<SUFFISSO> (file
#   out/snap_<ENTRY>_<stato>_<SUFFISSO>_*). SNAP_COND dall'ambiente passa a
#   snap.lua la condizione del breakpoint (es. SNAP_COND='a>=#10').
# Prova i frame di partenza dello stato in ordine e si ferma al primo che
# produce out/snap_<ENTRY>_<tag>_regs.txt. MAME senza finestra né audio.
# Stati del brief 16 (inneschi come le sessioni porta/bonus/finale di
# sessioni.sh, sovrascrivibili con SNAP_POKE/SNAP_POSTA dall'ambiente):
#   porta  c19d/c19e = 0 al 2250, posta 1C al 2300 (sequenza della porta)
#   bonus  in più c24a = 0 e c24b = 0x85: la porta porta alla stanza nascosta 5
#   finale in più c24b-c24d = 0x32: 4032 posta l'attività 24 (finale)
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
    porta)  INGRESSI=traccia; FRAMES="2400 2600 3000"
            POKE_DEF=2250:0xc19d:0,2250:0xc19e:0 ;;
    bonus)  INGRESSI=traccia; FRAMES="2400 2600 3000"
            POKE_DEF=2250:0xc19d:0,2250:0xc19e:0,2250:0xc24a:0,2250:0xc24b:0x85 ;;
    finale) INGRESSI=traccia; FRAMES="2400 2600 3000"
            POKE_DEF=2250:0xc19d:0,2250:0xc19e:0,2250:0xc24b:0x32,2250:0xc24c:0x32,2250:0xc24d:0x32 ;;
    *) echo "stato sconosciuto: $STATO (titolo, gioco, tardo, porta, bonus, finale)" >&2; exit 2 ;;
esac
case "$STATO" in
    porta|bonus|finale)
        SNAP_POKE=${SNAP_POKE:-$POKE_DEF}
        SNAP_POSTA=${SNAP_POSTA:-2300:0x1c} ;;
    *)
        SNAP_POKE=${SNAP_POKE:-}
        SNAP_POSTA=${SNAP_POSTA:-} ;;
esac
export SNAP_POKE SNAP_POSTA
ROMPATH="$(cd .. && pwd)/roms"
mkdir -p out
for F in $FRAMES; do
    rm -f out/snap_${ENTRY}_${TAG}_prima.bin out/snap_${ENTRY}_${TAG}_dopo.bin \
          out/snap_${ENTRY}_${TAG}_regs.txt
    SNAP_ENTRY=$ENTRY SNAP_TAG=$TAG SNAP_INGRESSI=$INGRESSI SNAP_DA_FRAME=$F SNAP_N=$N \
        SDL_VIDEODRIVER=dummy \
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
