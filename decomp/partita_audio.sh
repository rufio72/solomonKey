#!/bin/sh
# Oracolo audio della partita (lotto H6, brief 50) e confronto con gioca.
# Uso (da decomp/):
#   sh partita_audio.sh             genera out/audio/partita_latch.txt e partita_ay.txt
#   sh partita_audio.sh confronta   gioca (fase scelta) contro l'oracolo: atteso esatto
# MAME senza finestra né audio, ingressi dell'host spenti (come giro_boot.sh).
set -u
FASE=1
# valori misurati con la fase scelta (insiemi diversi, stati diversi, primo frame)
ATT_INSIEMI=604
ATT_STATI=312
ATT_PRIMO=1190
if [ "${1:-}" = "confronta" ]; then
    set -e
    [ -s out/audio/partita_ay.txt ] || { echo "manca l'oracolo: sh partita_audio.sh"; exit 1; }
    harness/gioca -n 5434 --oracolo out/giroboot_oracolo.txt out/giroboot_disp.txt \
        --riproduci out/ingressi_replay.txt --fase-audio $FASE \
        --flusso-ay out/partita_c_ay.txt --registra-latch out/partita_c_latch.txt \
        > out/partita_c.log
    cmp out/partita_c_latch.txt out/audio/partita_latch.txt && echo "latch: sequenza uguale"
    set +e
    python3 partita_audio_confronta.py out/audio/partita_ay.txt out/partita_c_ay.txt 14 5499 \
        | tee out/partita_c_conf.txt
    ins=$(sed -n 's/^frame con insieme di cambi diverso //p' out/partita_c_conf.txt)
    sta=$(sed -n 's/^frame con stato di fine frame diverso //p' out/partita_c_conf.txt)
    pri=$(sed -n 's/^primo frame diverso //p' out/partita_c_conf.txt)
    if [ "$ins" = "$ATT_INSIEMI" ] && [ "$sta" = "$ATT_STATI" ] && [ "$pri" = "$ATT_PRIMO" ]; then
        echo "CONFRONTA AUDIO: come atteso (insiemi $ins, stati $sta, primo $pri)"; exit 0
    fi
    echo "CONFRONTA AUDIO: DIVERSO dall'atteso ($ATT_INSIEMI/$ATT_STATI/$ATT_PRIMO): $ins/$sta/$pri"
    exit 1
fi
export REPLAY_FINE=${REPLAY_FINE:-5500}
ROMPATH="$(cd .. && pwd)/roms"
mkdir -p out/audio
rm -f out/audio/partita_latch.txt out/audio/partita_ay.txt
SDL_VIDEODRIVER=dummy \
    mame solomon -rompath "$ROMPATH" -video none -sound none \
    -keyboardprovider none -mouseprovider none -joystickprovider none \
    -lightgunprovider none \
    -skip_gameinfo -nothrottle -debug -debugger none -autoboot_script partita_audio.lua \
    > out/audio/partita.log 2>&1
nl=$(grep -vc '^#' out/audio/partita_latch.txt 2>/dev/null)
na=$(wc -l < out/audio/partita_ay.txt 2>/dev/null)
if [ "${nl:-0}" -gt 0 ] && [ "${na:-0}" -gt 1 ]; then
    echo "PARTITA latch $nl ay $na"
    exit 0
fi
echo "PARTITA FALLITA (vedi out/audio/partita.log)"
exit 1
