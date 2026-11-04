#!/bin/sh
# Catture dei corpi di attività (brief 52): legge corpi_catture.txt (una riga
# per cattura: «modo cod entry tag N poke», vedi l'intestazione del file) e
# lancia cattura.sh nello stato att<cod>, sei alla volta (xargs -P6). Con modo
# corpo esporta SNAP_CORPO=1 (snap.lua aspetta il ritorno in [SP+2]); poke si
# aggiunge ai predefiniti 2250:0xc19d:0,2250:0xc19e:0 dello stato attNN.
# Con modo stato (brief 54) cod è il nome di uno stato di cattura.sh (gioco),
# senza SNAP_CORPO né poke predefiniti: si passano i soli poke della riga.
# Rieseguibile: cancella e rifà out/snap_<entry>_att<cod>_<tag>_* (con modo
# stato out/snap_<entry>_<cod>_<tag>_*) per le sole righe dell'elenco. Ultima riga: «CATTURE OK n su m».
# Uso (da decomp/): sh cattura_corpi.sh [elenco]   (predefinito corpi_catture.txt)
set -u
cd "$(dirname "$0")"
ELENCO=${1:-corpi_catture.txt}
ESITI=$(mktemp)
trap 'rm -f "$ESITI"' EXIT
# una riga dell'elenco per processo: modo cod entry tag N poke
grep -v '^[[:space:]]*#' "$ELENCO" | grep -v '^[[:space:]]*$' |
    xargs -P6 -L1 sh -c '
        modo=$1; cod=$2; entry=$3; tag=$4; n=$5; poke=$6
        st=att$cod
        [ "$modo" = stato ] && st=$cod
        rm -f out/snap_${entry}_${st}_${tag}_prima.bin out/snap_${entry}_${st}_${tag}_dopo.bin \
              out/snap_${entry}_${st}_${tag}_regs.txt out/snap_${entry}_${st}_${tag}.log
        p=2250:0xc19d:0,2250:0xc19e:0
        [ "$poke" != "-" ] && p=$p,$poke
        if [ "$modo" = stato ]; then
            p=
            [ "$poke" != "-" ] && p=$poke
            SNAP_POKE=$p sh cattura.sh "$entry" "$cod" "$n" "$tag"
        elif [ "$modo" = corpo ]; then
            SNAP_CORPO=1 SNAP_POKE=$p sh cattura.sh "$entry" "att$cod" "$n" "$tag"
        else
            SNAP_POKE=$p sh cattura.sh "$entry" "att$cod" "$n" "$tag"
        fi' _ | tee "$ESITI"
M=$(grep -v '^[[:space:]]*#' "$ELENCO" | grep -c -v '^[[:space:]]*$')
K=$(grep -c '^CATTURA .* OK$' "$ESITI")
echo "CATTURE OK $K su $M"
