#!/bin/sh
# Oracolo video (lotto H1): per ogni stato, dump di video RAM/sprite RAM/palette
# e PNG di MAME dello stesso frame in out/video (o in $VIDEO_OUT).
# Uso (da decomp/): sh video.sh [stato...]   (senza argomenti: tutti e sei)
# Stati: titolo gioco stanza10 bonus finale finale2 (inneschi come cattura.sh);
# prove (sintetico, solo se nominato): titolo con poke di flip, bit FG e sprite.
# MAME senza finestra né audio, con i quattro provider di input a none.
set -u
ROMPATH="$(cd .. && pwd)/roms"
DIR=${VIDEO_OUT:-out/video}
mkdir -p "$DIR"
[ $# -eq 0 ] && set -- titolo gioco stanza10 bonus finale finale2
POKE_P=995:0xd942:0x0a,995:0xd943:0x0a,995:0xd944:0x0a,995:0xd945:0x0a,995:0xd946:0x0a,995:0xd947:0x0a,995:0xd948:0x0a,995:0xd949:0x0a,995:0xd94a:0x0a,995:0xd94b:0x0a,995:0xd2aa:0x80,995:0xd2ab:0x80,995:0xd2ac:0x08,995:0xd2ad:0x08,995:0xe000:0x88,995:0xe001:0x80,995:0xe002:141,995:0xe003:100,995:0xe004:0x88,995:0xe005:0x00,995:0xe006:141,995:0xe007:250,995:0xe008:0x88,995:0xe009:0xc0,995:0xe00a:120,995:0xe00b:60,995:0xe00c:0x88,995:0xe00d:0,995:0xe00e:5,995:0xe00f:30
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
        prove)   FR=1000; ING=nessuno; POKE=$POKE_P ;;
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
