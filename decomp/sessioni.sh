#!/bin/sh
# Sessioni guidate per la copertura del Z80 principale (brief 04).
# Uso (da decomp/): sh sessioni.sh [lotto12] > out/sessioni.log 2>&1
# Senza argomento esegue tutto (sessioni storiche, poi il blocco lotto12);
# con argomento lotto12 solo le sessioni del brief 16 (porta, stanze
# nascoste, finale, oggetti, attract completa, attività mai spedite).
# Ogni sessione traccia solo le sue finestre e poi coverage.py ne ricava
# out/<nome>_*.txt cancellando la traccia.
set -u
cd "$(dirname "$0")"
PATH="$PATH:/usr/games"
BLOCCO="${1:-tutto}"
export DISPLAY="${DISPLAY:-:1}"
# provider di input a none come replay.sh: niente tastiera vera (brief 27)
MOPT="-rompath $(cd .. && pwd)/roms -video none -sound none -keyboardprovider none -mouseprovider none -joystickprovider none -lightgunprovider none -skip_gameinfo -nothrottle"

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

storiche() {
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
}

# Brief 16: inneschi con forzature di RAM (SESS_POKE) e poste di attività
# (SESS_POSTA); c19d/c19e (passo e sotto-passo degli spedidori a passi)
# azzerati prima di postare 1C o 24. Le finestre seguono le stampe FASE di
# corse di prova senza traccia.
lotto12() {
    # le corse att_XX dipendono dall'unione: si ripartono da zero a ogni giro
    rm -f out/att_*_*.txt
    # a) porta: sequenza 1C (3F40) -> 4000, avanzamento alla stanza 1
    SESS_ROOM= SESS_INPUT=casuale SESS_CASUALE_DA=1600 \
    SESS_POKE=2250:0xc19d:0,2250:0xc19e:0 SESS_POSTA=2300:0x1c \
    SESS_TRACE_DA=2250 SESS_TRACE_A=3300 sessione porta

    # b) stanze nascoste: record 4 voce 0 = 0x85 (stanza 5), record 42 = 0xAB (43)
    SESS_ROOM=4 SESS_INPUT=casuale SESS_CASUALE_DA=1600 \
    SESS_POKE=2250:0xc19d:0,2250:0xc19e:0,2250:0xc24a:0 SESS_POSTA=2300:0x1c \
    SESS_TRACE_DA=2250 SESS_TRACE_A=3300 sessione bonus_04
    SESS_ROOM=42 SESS_INPUT=casuale SESS_CASUALE_DA=1600 \
    SESS_POKE=2250:0xc19d:0,2250:0xc19e:0 SESS_POSTA=2300:0x1c \
    SESS_TRACE_DA=2250 SESS_TRACE_A=3300 sessione bonus_42

    # c) finale: candidate tutte 0x32, 4032 posta da solo la 24 (frame 2578);
    # la partita nella stanza finale prosegue (16 a 5269, 06/18 fino al 6800,
    # di nuovo 1C a 7788, poi 06 23 07 0F fino al 9300): finale2 la copre
    POKE_FIN=2250:0xc24b:0x32,2250:0xc24c:0x32,2250:0xc24d:0x32,2250:0xc19d:0,2250:0xc19e:0
    SESS_ROOM= SESS_INPUT=casuale SESS_CASUALE_DA=1600 \
    SESS_POKE=$POKE_FIN SESS_POSTA=2300:0x1c \
    SESS_TRACE_DA=2250 SESS_TRACE_A=4500 sessione finale
    SESS_ROOM= SESS_INPUT=casuale SESS_CASUALE_DA=1600 \
    SESS_POKE=$POKE_FIN SESS_POSTA=2300:0x1c \
    SESS_TRACE_DA=4500,7700 SESS_TRACE_A=6900,9400 sessione finale2

    # d) oggetti speciali: coda c05b (3 slot da 2 byte: oggetto, parametro E)
    # e posta 22 (12D6). 12F0 salta per oggetto-8 nella tabella a 130A, valida
    # per gli oggetti 08-1B: tre oggetti per posta, una posta ogni 100 frame
    # (il brief prevedeva solo 08, 0B, 0E: lasciavano scoperte 1471/1474/147F/1533)
    POKE_OGG=
    POSTA_OGG=
    f=2250
    for terna in "08 09 0a" "0b 0c 0d" "0e 0f 10" "11 12 13" "14 15 16" "17 18 19" "1a 1b 00"; do
        a=$((0xc05b))
        for o in $terna; do
            POKE_OGG="$POKE_OGG$f:$(printf 0x%x $a):0x$o,$f:$(printf 0x%x $((a + 1))):0,"
            a=$((a + 2))
        done
        POSTA_OGG="$POSTA_OGG$((f + 50)):0x22,"
        f=$((f + 100))
    done
    SESS_ROOM= SESS_INPUT=casuale SESS_CASUALE_DA=1600 \
    SESS_POKE=$POKE_OGG SESS_POSTA=$POSTA_OGG \
    SESS_TRACE_DA=2250 SESS_TRACE_A=3300 sessione oggetti

    # e) attract completa senza moneta (attività 16 a 6496)
    SESS_ROOM= SESS_INPUT=nessuno SESS_MONETA=0 SESS_TRACE_DA=7400,19400 SESS_TRACE_A=8200,19900 \
        sessione attract2

    # f) attività il cui ingresso resta fuori dall'unione dopo a-e (in più del
    # brief la 16: la attract completa non la spedisce, finale2 la posta a 5269
    # ma 6496 non viene eseguita)
    for voce in 0F:640E 12:426A 13:6480 16:6496 19:3792 1A:411D 1B:4111 1D:3F39 25:3B77 26:3B63 27:62B6; do
        cod=${voce%%:*}
        ing=${voce##*:}
        # unione ricalcolata a ogni giro: sessione() la riscrive col solo file nuovo
        python3 copertura_call.py $(ls out/*_eseguiti.txt | grep -v unione) | tail -1
        if grep -qx "$ing" out/eseguiti_unione.txt; then
            echo "attività $cod ($ing) già eseguita: nessuna corsa dedicata"
            continue
        fi
        SESS_ROOM= SESS_INPUT=casuale SESS_CASUALE_DA=1600 \
        SESS_POKE=2250:0xc19d:0,2250:0xc19e:0 SESS_POSTA=2300:0x$cod \
        SESS_TRACE_DA=2250 SESS_TRACE_A=2800 sessione "att_$cod"
    done
    python3 copertura_call.py $(ls out/*_eseguiti.txt | grep -v unione) | tail -1
}

case "$BLOCCO" in
    lotto12) lotto12 ;;
    tutto) storiche; lotto12 ;;
    *) echo "ERRORE: blocco sconosciuto $BLOCCO (lotto12 o nessun argomento)"; exit 2 ;;
esac

echo "=== fine ($(date +%H:%M:%S))"
