#!/bin/sh
# Oracolo del giro dal boot (brief 44, da replay.sh con giro_boot.lua): partita
# con gli ingressi convenzionali, impronta della RAM a ogni NMI e registro
# dello spedidore. Dal brief 48 (H4c) arriva al frame 5499 (REPLAY_FINE 5500) e scarica i dump
# dei frame campione (13 per gli hash video, più 2410 per la saldatura di H4b3).
# Uso (da decomp/): sh giro_boot.sh
# Variabili passate a replay.lua: REPLAY_FINE (5500), REPLAY_F0 (602),
# REPLAY_DUMPS (dump completi oltre a F0: i 14 del brief 48; separati da virgola), REPLAY_NOME
# (giroboot). MAME senza finestra né audio, come cattura.sh; circa 13 secondi.
# Tastiera, mouse, joystick e pistola dell'host sono spenti: altrimenti un
# tasto o un joystick toccato mentre MAME gira entra nelle porte (e600 = 02
# ai frame 1692-1725 in una delle registrazioni) e l'oracolo cambia.
set -u
REPLAY_NOME=${REPLAY_NOME:-giroboot}
export REPLAY_NOME
export REPLAY_FINE=${REPLAY_FINE:-5500}
export REPLAY_F0=${REPLAY_F0:-602}
export REPLAY_DUMPS=${REPLAY_DUMPS:-700,1000,1300,1600,1900,2100,2400,2410,3000,3500,4000,4500,5000,5499}
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
