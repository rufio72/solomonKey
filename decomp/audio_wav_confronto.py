#!/usr/bin/env python3
"""Confronto di due wav di audio_wav (brief 22).

python3 audio_wav_confronto.py <a.wav> <b.wav> [da]

da = frame del primo campione (predefinito 14); un frame sono 735 campioni
(44100 Hz / 60). Stampa una riga con campioni diversi, primo e ultimo
campione diverso col loro frame, scarto massimo, frame diversi e rms della
differenza in dBFS (riferimento 32768). Uscita 0 se identici, 1 se no,
2 per errori di file.
"""
import array
import math
import sys
import wave

CAMPIONI_FRAME = 735


def leggi(percorso):
    with wave.open(percorso, 'rb') as w:
        if w.getsampwidth() != 2 or w.getnchannels() != 1:
            raise ValueError('%s: atteso 16 bit mono' % percorso)
        grezzi = w.readframes(w.getnframes())
    v = array.array('h')
    v.frombytes(grezzi)
    if sys.byteorder == 'big':
        v.byteswap()
    return v


def main():
    if len(sys.argv) not in (3, 4):
        print('uso: python3 audio_wav_confronto.py <a.wav> <b.wav> [da]',
              file=sys.stderr)
        return 2
    pa, pb = sys.argv[1], sys.argv[2]
    da = int(sys.argv[3]) if len(sys.argv) == 4 else 14
    try:
        a, b = leggi(pa), leggi(pb)
    except (OSError, ValueError, wave.Error) as e:
        print('ERRORE: %s' % e, file=sys.stderr)
        return 2
    na, nb = len(a), len(b)
    n = max(na, nb)
    diversi = 0
    primo = ultimo = -1
    scarto = 0
    somma = 0
    frame = set()
    for i in range(n):
        x = a[i] if i < na else 0
        y = b[i] if i < nb else 0
        if x == y:
            continue
        d = x - y
        diversi += 1
        if primo < 0:
            primo = i
        ultimo = i
        if abs(d) > scarto:
            scarto = abs(d)
        somma += d * d
        frame.add(i // CAMPIONI_FRAME)
    if diversi == 0 and na == nb:
        print('%s %s: identici, campioni %d' % (pa, pb, na))
        return 0
    if somma:
        rms = math.sqrt(somma / n) / 32768
        db = '%.1f' % (20 * math.log10(rms))
    else:
        db = '-inf'
    print('%s %s: campioni %d/%d, diversi %d, primo %d (frame %d), '
          'ultimo %d (frame %d), scarto massimo %d, frame diversi %d, '
          'rms differenza %s dBFS'
          % (pa, pb, na, nb, diversi, primo, da + primo // CAMPIONI_FRAME,
             ultimo, da + ultimo // CAMPIONI_FRAME, scarto, len(frame), db))
    return 1


if __name__ == '__main__':
    sys.exit(main())
