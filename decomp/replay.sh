#!/bin/sh
# Oracolo del replay (brief 23): partita con gli ingressi convenzionali,
# impronta della RAM a ogni NMI e registro dello spedidore (replay.lua).
# Uso (da decomp/): sh replay.sh
# Variabili passate a replay.lua: REPLAY_FINE (5500), REPLAY_F0 (2402),
# REPLAY_DUMPS (frame con dump completo, separati da virgola), REPLAY_NOME
# (replay). MAME senza finestra né audio, come cattura.sh; circa 90 secondi.
# Tastiera, mouse, joystick e pistola dell'host sono spenti: altrimenti un
# tasto o un joystick toccato mentre MAME gira entra nelle porte (e600 = 02
# ai frame 1692-1725 in una delle registrazioni) e l'oracolo cambia.
set -u
REPLAY_NOME=${REPLAY_NOME:-replay}
export REPLAY_NOME
export REPLAY_FINE=${REPLAY_FINE:-5500}
export REPLAY_F0=${REPLAY_F0:-2402}
export REPLAY_DUMPS=${REPLAY_DUMPS:-}
ROMPATH="$(cd .. && pwd)/roms"
mkdir -p out
rm -f out/${REPLAY_NOME}_oracolo.txt out/${REPLAY_NOME}_disp.txt
SDL_VIDEODRIVER=dummy \
    mame solomon -rompath "$ROMPATH" -video none -sound none \
    -keyboardprovider none -mouseprovider none -joystickprovider none \
    -lightgunprovider none \
    -skip_gameinfo -nothrottle -debug -debugger none -autoboot_script replay.lua \
    > out/${REPLAY_NOME}.log 2>&1
if [ -s out/${REPLAY_NOME}_oracolo.txt ]; then
    echo "REPLAY $REPLAY_NOME: $(wc -l < out/${REPLAY_NOME}_oracolo.txt) righe F, \
$(grep -c '^D' out/${REPLAY_NOME}_disp.txt) spedizioni"
    exit 0
fi
echo "REPLAY $REPLAY_NOME FALLITO (vedi out/${REPLAY_NOME}.log)"
exit 1
