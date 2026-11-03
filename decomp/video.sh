#!/bin/sh
# Oracolo video (lotto H1): per ogni stato, dump di video RAM/sprite RAM/palette
# e PNG di MAME dello stesso frame in out/video (o in $VIDEO_OUT).
# Uso (da decomp/): sh video.sh [stato...]   (senza argomenti: tutti e sei)
# Stati: titolo gioco stanza10 bonus finale finale2 (inneschi come cattura.sh).
# MAME senza finestra né audio, con i quattro provider di input a none.
set -u
ROMPATH="$(cd .. && pwd)/roms"
DIR=${VIDEO_OUT:-out/video}
mkdir -p "$DIR"
[ $# -eq 0 ] && set -- titolo gioco stanza10 bonus finale finale2
POKE_F=2250:0xc19d:0,2250:0xc19e:0,2250:0xc24b:0x32,2250:0xc24c:0x32,2250:0xc24d:0x32

lancia() {
    S=$1
    ROOM=; POKE=; POSTA=
    case "$S" in
        titolo)  FR=1000; ING=nessuno ;;
        gioco)   FR=2400; ING=traccia ;;
        stanza10) FR=2100; ING=traccia; ROOM=10 ;;
        bonus)   FR=2400; ING=traccia; ROOM=4
                 POKE=2250:0xc19d:0,2250:0xc19e:0,2250:0xc24a:0; POSTA=2300:0x1c ;;
        finale)  FR=2400; ING=traccia; POKE=$POKE_F; POSTA=2300:0x1c ;;
        finale2) FR=4500; ING=traccia; POKE=$POKE_F; POSTA=2300:0x1c ;;
        *) echo "stato sconosciuto: $S" >&2; return 2 ;;
    esac
    VIDEO_STATO=$S VIDEO_DIR=$DIR VIDEO_FRAME=$FR VIDEO_INGRESSI=$ING \
    VIDEO_ROOM=$ROOM VIDEO_POKE=$POKE VIDEO_POSTA=$POSTA SDL_VIDEODRIVER=dummy \
        mame solomon -rompath "$ROMPATH" -video none -sound none \
        -keyboardprovider none -mouseprovider none -joystickprovider none \
        -lightgunprovider none -skip_gameinfo -nothrottle \
        -snapshot_directory "$DIR" -snapname "$S" -autoboot_script video.lua \
        > "$DIR/$S.log" 2>&1
}

for S in "$@"; do lancia "$S" & done
wait

RC=0
for S in "$@"; do
    N=0
    for F in bg_colorram bg_videoram fg_colorram fg_videoram spriteram palette; do
        [ -f "$DIR/${S}_$F.bin" ] && N=$((N + 1))
    done
    [ -f "$DIR/$S.png" ] && N=$((N + 1))
    if [ $N -eq 7 ]; then echo "VIDEO $S OK"; else echo "VIDEO $S FALLITO"; RC=1; fi
    rm -f "$DIR/$S.log"
done
exit $RC
