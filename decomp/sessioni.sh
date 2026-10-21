#!/bin/sh
# Sessioni guidate per la copertura del Z80 principale (brief 04).
# Uso (da decomp/): sh sessioni.sh > out/sessioni.log 2>&1
# Ogni sessione traccia solo le sue finestre e poi coverage.py ne ricava
# out/<nome>_*.txt cancellando la traccia. Le sessioni bonus e finale sono
# fuori perimetro (rinviate).
set -u
cd "$(dirname "$0")"
export DISPLAY="${DISPLAY:-:1}"
MOPT="-rompath $(cd .. && pwd)/roms -video none -sound none -skip_gameinfo -nothrottle"

# sessione NOME: le variabili SESS_* arrivano dall'ambiente del chiamante
sessione() {
    nome="$1"
    echo "=== sessione $nome ($(date +%H:%M:%S))"
    SESS_NOME="$nome" SDL_VIDEODRIVER=dummy mame solomon $MOPT -debug -debugger none -autoboot_script sessione.lua
    if [ -f "out/${nome}_trace.txt" ]; then
        python3 coverage.py --traccia "out/${nome}_trace.txt" --prefisso "$nome" --cancella
        python3 copertura_call.py "out/${nome}_eseguiti.txt"
    else
        echo "ERRORE: traccia di $nome assente"
    fi
}

# stanze 0..51 tranne 49 (indice 0-based; la stanza 50 non carica)
n=0
while [ $n -le 51 ]; do
    if [ $n -ne 49 ]; then
        nn=$(printf '%02d' $n)
        SESS_ROOM=$n SESS_INPUT=casuale SESS_CASUALE_DA=1600 \
        SESS_TRACE_DA=1500 SESS_TRACE_A=2200 sessione "stanza_$nn"
    fi
    n=$((n + 1))
done

# partita senza input fino al game over (codice 15 fra i frame 7300 e 7400)
SESS_ROOM= SESS_INPUT=nessuno SESS_TRACE_DA=7000 SESS_TRACE_A=7700 sessione gameover

# test RAM, titolo e demo iniziale
SESS_ROOM= SESS_INPUT=nessuno SESS_TRACE_DA=0 SESS_TRACE_A=1500 sessione titolo

# attract senza moneta: fasi 07/16 dopo la demo
SESS_ROOM= SESS_INPUT=nessuno SESS_MONETA=0 SESS_TRACE_DA=7400,19400 SESS_TRACE_A=8200,19900 \
    sessione attract

echo "=== fine ($(date +%H:%M:%S))"
