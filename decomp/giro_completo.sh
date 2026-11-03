#!/bin/sh
# Regressione del giro completo dal boot, frame 0-5499 (lotto H4c, brief 48).
# Uso (da decomp/): sh giro_completo.sh        con REGEN=1 rigenera prima l'oracolo
# Esegue gioco_libero sull'oracolo esteso, stampa le righe di sintesi e la riga
# GIRO COMPLETO, pretende esattamente MEZZE_ATTESE NMI divergenti a metà attività
# (le 30 attese del boot e dei corpi non ancora a punti; 33 prima del punto 4632
# di H4c) e restituisce l'uscita del giro. Uscita 2 se manca l'oracolo.
MEZZE_ATTESE=30
[ "${REGEN:-0}" = 1 ] && sh giro_boot.sh
if [ "$(grep -c '^F ' out/giroboot_oracolo.txt 2>/dev/null)" -lt 5434 ] 2>/dev/null \
   || [ ! -s out/giroboot_oracolo.txt ]; then
    echo "oracolo mancante o corto: sh giro_boot.sh (oppure REGEN=1 sh giro_completo.sh)"
    exit 2
fi
harness/gioco_libero out/giroboot_oracolo.txt out/giroboot_disp.txt out/ingressi_replay.txt \
    > out/giro_completo.txt
rc=$?
grep -E '^(SALDATURA|NMI confrontate|dopo il boot|GIRO COMPLETO)' out/giro_completo.txt
[ "$RIGHE" = 1 ] && grep '^MEZZA' out/giro_completo.txt
mezze=$(grep -c '^MEZZA' out/giro_completo.txt)
if [ "$rc" = 0 ] && [ "$mezze" != "$MEZZE_ATTESE" ]; then
    echo "M diverso dall'atteso: $mezze invece di $MEZZE_ATTESE"
    exit 1
fi
exit $rc
