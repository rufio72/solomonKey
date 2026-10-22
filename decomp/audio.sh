#!/bin/sh
# Corse di osservazione del Z80 audio (brief 18): codici forzati nel latch,
# oracolo AY, RAM audio, copertura dalla traccia del solo :audiocpu.
# Uso (da decomp/): mkdir -p out/audio; sh audio.sh [blocco] > out/audio/audio.log 2>&1
# Blocchi: avvio | codici | ignorati | lunghi | gioco; senza argomento tutti in ordine.
# Ogni corsa scrive out/audio/<nome>_{ay,latch,ram}.txt (audio.lua) e, se ha
# traccia, coverage.py --cpu audio ne ricava out/audio/<nome>_*.txt
# cancellando la traccia. In coda la copertura delle call statiche dell'audio.
set -u
cd "$(dirname "$0")"
PATH="$PATH:/usr/games"
BLOCCO="${1:-tutto}"
MOPT="-rompath $(cd .. && pwd)/roms -video none -sound none -skip_gameinfo -nothrottle"
PASSO=90
mkdir -p out/audio

# corsa NOME: le variabili AUDIO_* arrivano dall'ambiente del chiamante
corsa() {
    nome="$1"
    echo "=== corsa $nome ($(date +%H:%M:%S))"
    AUDIO_NOME="$nome" SDL_VIDEODRIVER=dummy mame solomon $MOPT -debug -debugger none -autoboot_script audio.lua
    traccia="out/audio/${nome}_trace.txt"
    if [ -f "$traccia" ]; then
        python3 coverage.py --cpu audio --traccia "$traccia" --prefisso "audio/$nome" --cancella
    elif [ -n "${AUDIO_TRACE_DA:-}" ]; then
        echo "ERRORE: traccia di $nome assente"
    fi
}

# avvio dal reset (frame 0-30), traccia dal frame 0: copre 0000-0007,
# 0072-009D e 00A4, che le altre corse non tracciano
avvio() {
    rm -f out/audio/avvio_*
    AUDIO_CODICI= AUDIO_TRACE_DA=0 AUDIO_TRACE_A=30 AUDIO_FINE=30 corsa avvio
}

# gruppi di 8 codici (00-3f) più il 40, PASSO frame ciascuno, traccia intera
codici() {
    rm -f out/audio/cod_[0-3]* out/audio/cod_40_*
    for g in 00 08 10 18 20 28 30 38; do
        a=$((0x$g))
        b=$(printf '%02x' $((a + 7)))
        fine=$((120 + 8 * PASSO))
        AUDIO_CODICI="$g-$b" AUDIO_PASSO=$PASSO AUDIO_TRACE_DA=120 AUDIO_TRACE_A=$fine \
            corsa "cod_${g}_$b"
    done
    AUDIO_CODICI=40 AUDIO_PASSO=$PASSO AUDIO_TRACE_DA=120 AUDIO_TRACE_A=$((120 + PASSO)) \
        corsa cod_40
}

# codici 41-ff, ignorati dallo spedidore (cp 041h; ret nc): passo 2 frame
ignorati() {
    rm -f out/audio/cod_41_ff_*
    fine=$((120 + 191 * 2))
    AUDIO_CODICI=41-ff AUDIO_PASSO=2 AUDIO_TRACE_DA=120 AUDIO_TRACE_A=$fine \
        corsa cod_41_ff
}

# codici con cambi AY negli ultimi 10 frame della loro finestra di PASSO frame
# nelle corse cod_*: il suono non è finito, corsa singola da 900 frame
elenco_lunghi() {
    for f in out/audio/cod_[0-3]*_ay.txt out/audio/cod_40_ay.txt; do
        [ -f "$f" ] || continue
        awk -v passo=$PASSO '
            /^# codice / { f0 = $3; cc = $4; next }
            /^#/ { next }
            cc != "" && $1 >= f0 + passo - 10 && $1 < f0 + passo { print cc }
        ' "$f"
    done | sort -u
}

lunghi() {
    rm -f out/audio/lungo_*
    l=$(elenco_lunghi)
    echo "codici lunghi: $(echo $l)"
    for cc in $l; do
        AUDIO_CODICI=$cc AUDIO_PASSO=900 AUDIO_TRACE_DA=120 AUDIO_TRACE_A=1020 \
            corsa "lungo_$cc"
    done
}

# gioco normale (moneta, start, input casuali), traccia 1500-2200
gioco() {
    rm -f out/audio/gioco_*
    AUDIO_GIOCO=1 AUDIO_CODICI= AUDIO_TRACE_DA=1500 AUDIO_TRACE_A=2200 AUDIO_FINE=2200 \
        corsa gioco
}

case "$BLOCCO" in
    avvio) avvio ;;
    codici) codici ;;
    ignorati) ignorati ;;
    lunghi) lunghi ;;
    gioco) gioco ;;
    tutto) avvio; codici; ignorati; lunghi; gioco ;;
    *) echo "ERRORE: blocco sconosciuto $BLOCCO (avvio, codici, ignorati, lunghi, gioco o nessuno)"; exit 2 ;;
esac

# copertura delle destinazioni statiche di call sotto 4000 (unione esclusa dal glob)
python3 copertura_call.py --dasm out/audio.dasm --limite 0x4000 --uscite out/audio \
    --classifica out/audio/*_eseguiti.txt
echo "=== fine ($(date +%H:%M:%S))"
