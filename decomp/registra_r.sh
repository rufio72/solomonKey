#!/bin/sh
# Registrazione del registro R (brief 45): come giro_boot.sh, con registra_r.lua.
# Produce out/regr_oracolo.txt, out/regr_disp.txt, i dump e out/regr_r.txt,
# una riga «R <frame> <PC del ld a,r> <A>» per ogni lettura di R ai siti
# 2D1F 2D22 430A 430D 54C5 54C8 59E5 59E8 5DDF 5DE2 (o REGR_SITI, esadecimali
# separati da spazio). Con gli ingressi registrati le letture sono 0 fino al
# frame 5499: il giro in C si ferma se ne incontra una (nemici_r_gancio).
# Uso (da decomp/): sh registra_r.sh
# Variabili: REPLAY_FINE (2411), REPLAY_F0 (602), REPLAY_DUMPS, REPLAY_NOME
# (regr), REGR_SITI. MAME senza finestra né audio e con gli ingressi
# dell'host spenti, come giro_boot.sh; circa 45 secondi per 2411 frame.
set -u
REPLAY_NOME=${REPLAY_NOME:-regr}
export REPLAY_NOME
export REPLAY_FINE=${REPLAY_FINE:-2411}
export REPLAY_F0=${REPLAY_F0:-602}
export REPLAY_DUMPS=${REPLAY_DUMPS:-}
ROMPATH="$(cd .. && pwd)/roms"
mkdir -p out
rm -f out/${REPLAY_NOME}_oracolo.txt out/${REPLAY_NOME}_disp.txt out/${REPLAY_NOME}_r.txt
SDL_VIDEODRIVER=dummy \
    mame solomon -rompath "$ROMPATH" -video none -sound none \
    -keyboardprovider none -mouseprovider none -joystickprovider none \
    -lightgunprovider none \
    -skip_gameinfo -nothrottle -debug -debugger none -autoboot_script registra_r.lua \
    > out/${REPLAY_NOME}.log 2>&1
if [ -s out/${REPLAY_NOME}_oracolo.txt ]; then
    echo "R: $(wc -l < out/${REPLAY_NOME}_r.txt) letture in out/${REPLAY_NOME}_r.txt"
    exit 0
fi
echo "REGISTRA_R $REPLAY_NOME FALLITO (vedi out/${REPLAY_NOME}.log)"
exit 1
