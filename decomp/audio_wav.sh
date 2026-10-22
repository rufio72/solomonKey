#!/bin/sh
# Wav dei suoni del driver audio (brief 22), da decomp/:
#   sh audio_wav.sh codici   comandi 01-3F dal C -> out/wav/codici/<cc>.wav
#   sh audio_wav.sh oracoli  i 44 oracoli MAME e il C -> out/wav/oracoli/
#                            <nome>_mame.wav e <nome>_c.wav, confronto con cmp
# Esclusi dai codici: 00 (arresto, silenzio), 40 (fuori modello, mai inviato
# dal gioco), 41-FF (ignorati, nessun cambio). Non lancia MAME: gli oracoli
# in out/audio/ si leggono soltanto.
set -e
cd "$(dirname "$0")"

FRAME_CODICE=120
FRAME_TETTO=2000
CODA=30

make -s -C harness audio_banco audio_suona audio_wav

# frame dell'ultima riga di dati di un flusso (0 se nessuna)
ultimo_frame() {
    awk '!/^#/ && NF == 4 { f = $1 } END { print f + 0 }' "$1"
}

codici() {
    mkdir -p out/wav/flussi out/wav/codici
    for n in $(seq 1 63); do
        cc=$(printf '%02X' "$n")
        AUDIO_DATI=estratti ./harness/audio_suona "$cc" "$FRAME_TETTO" \
            "out/wav/flussi/${cc}_ay.txt" 2>/dev/null
        a=$(( $(ultimo_frame "out/wav/flussi/${cc}_ay.txt") + CODA ))
        [ "$a" -gt "$FRAME_TETTO" ] && a=$FRAME_TETTO
        [ "$a" -lt "$FRAME_CODICE" ] && a=$FRAME_CODICE
        ./harness/audio_wav "out/wav/flussi/${cc}_ay.txt" \
            "out/wav/codici/${cc}.wav" "$FRAME_CODICE" "$a"
        LC_ALL=C awk -v c="$cc" -v d="$FRAME_CODICE" -v a="$a" \
            'BEGIN { printf "%s %d %d %.2f\n", c, d, a, (a - d + 1) / 60 }'
    done
}

oracoli() {
    mkdir -p out/wav/oracoli
    : > out/wav/eccezioni.txt
    uguali=0
    diversi=0
    for o in out/audio/*_ay.txt; do
        nome=$(basename "$o" _ay.txt)
        [ "$nome" = foto_01_40 ] && continue
        AUDIO_DATI=estratti ./harness/audio_banco "$nome" >/dev/null 2>&1 || true
        case "$nome" in
            cod_40|lungo_40) a=119 ;;      # comando 40 fuori modello
            *) a=$(ultimo_frame "$o") ;;
        esac
        [ "$a" -lt 14 ] && a=$FRAME_CODICE
        ./harness/audio_wav "$o" "out/wav/oracoli/${nome}_mame.wav" 14 "$a"
        ./harness/audio_wav "out/audio/banco/${nome}_ay.txt" \
            "out/wav/oracoli/${nome}_c.wav" 14 "$a"
        if cmp -s "out/wav/oracoli/${nome}_mame.wav" "out/wav/oracoli/${nome}_c.wav"; then
            echo "$nome uguali"
            uguali=$((uguali + 1))
        else
            echo "$nome diversi"
            diversi=$((diversi + 1))
            python3 audio_wav_confronto.py "out/wav/oracoli/${nome}_c.wav" \
                "out/wav/oracoli/${nome}_mame.wav" 14 \
                | tee -a out/wav/eccezioni.txt || true
        fi
    done
    echo "uguali $uguali diversi $diversi"
}

case "$1" in
    codici) codici ;;
    oracoli) oracoli ;;
    *) echo "uso (da decomp/): sh audio_wav.sh codici|oracoli" >&2; exit 2 ;;
esac
