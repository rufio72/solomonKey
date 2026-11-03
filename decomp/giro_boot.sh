#!/bin/sh
# Oracolo del giro dal boot (brief 44, da replay.sh con giro_boot.lua): partita
# con gli ingressi convenzionali, impronta della RAM a ogni NMI e registro
# dello spedidore. Dal brief 45 arriva al frame 2411 e scarica anche il dump
# del frame 2410 (out/giroboot_2410.bin, per il confronto finale di H4b3).
# Uso (da decomp/): sh giro_boot.sh
# Variabili passate a replay.lua: REPLAY_FINE (2411), REPLAY_F0 (602),
# REPLAY_DUMPS (dump completi oltre a F0: 2410; separati da virgola), REPLAY_NOME
# (giroboot). MAME senza finestra né audio, come cattura.sh; circa 45 secondi.
# Tastiera, mouse, joystick e pistola dell'host sono spenti: altrimenti un
# tasto o un joystick toccato mentre MAME gira entra nelle porte (e600 = 02
# ai frame 1692-1725 in una delle registrazioni) e l'oracolo cambia.
set -u
REPLAY_NOME=${REPLAY_NOME:-giroboot}
export REPLAY_NOME
export REPLAY_FINE=${REPLAY_FINE:-2411}
export REPLAY_F0=${REPLAY_F0:-602}
export REPLAY_DUMPS=${REPLAY_DUMPS:-2410}
ROMPATH="$(cd .. && pwd)/roms"
mkdir -p out
rm -f out/${REPLAY_NOME}_oracolo.txt out/${REPLAY_NOME}_disp.txt
SDL_VIDEODRIVER=dummy \
    mame solomon -rompath "$ROMPATH" -video none -sound none \
    -keyboardprovider none -mouseprovider none -joystickprovider none \
    -lightgunprovider none \
    -skip_gameinfo -nothrottle -debug -debugger none -autoboot_script giro_boot.lua \
    > out/${REPLAY_NOME}.log 2>&1
if [ -s out/${REPLAY_NOME}_oracolo.txt ]; then
    echo "REPLAY $REPLAY_NOME: $(wc -l < out/${REPLAY_NOME}_oracolo.txt) righe F, \
$(grep -c '^D' out/${REPLAY_NOME}_disp.txt) spedizioni"
    exit 0
fi
echo "REPLAY $REPLAY_NOME FALLITO (vedi out/${REPLAY_NOME}.log)"
exit 1
