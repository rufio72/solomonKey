#!/usr/bin/env python3
"""Joystick virtuale per provare `gioca --joystick` senza hardware (lotto H7).

Crea con /dev/uinput un SOLO joystick (mai tastiera o mouse) di nome
«solomon-test»: assi X/Y, hat 0, otto pulsanti. Attende 1,5 s (così la
diagnostica lo vede arrivare: COLLEGATO), poi invia asse X a +32767 per
0,3 s, hat a destra per 0,3 s, pulsante 0 giù e su, e lo distrugge
(SCOLLEGATO). Nient'altro cambia nel sistema.

Uso, in parallelo con la diagnostica:
    SDL_VIDEODRIVER=dummy harness/gioca --joystick -t 6 &
    sleep 0.5; python3 prova_joystick_uinput.py

Uscita 0 a prova inviata, 77 (= saltata) se /dev/uinput non è scrivibile.
Solo libreria standard: os, fcntl, struct, time.
"""
import fcntl
import os
import struct
import sys
import time

# costanti di linux/input-event-codes.h e linux/uinput.h
EV_SYN, EV_KEY, EV_ABS = 0x00, 0x01, 0x03
SYN_REPORT = 0
ABS_X, ABS_Y, ABS_HAT0X, ABS_HAT0Y = 0x00, 0x01, 0x10, 0x11
PULSANTI = list(range(0x130, 0x138))       # BTN_SOUTH ... BTN_TR
UI_SET_EVBIT, UI_SET_KEYBIT, UI_SET_ABSBIT = 0x40045564, 0x40045565, 0x40045567
UI_DEV_CREATE, UI_DEV_DESTROY = 0x5501, 0x5502
BUS_USB = 0x03
NOME = b"solomon-test"


def evento(fd, tipo, codice, valore):
    """Scrive un input_event (timeval a zero: lo riempie il kernel)."""
    os.write(fd, struct.pack("llHHi", 0, 0, tipo, codice, valore))


def invia(fd, tipo, codice, valore):
    evento(fd, tipo, codice, valore)
    evento(fd, EV_SYN, SYN_REPORT, 0)


def crea():
    try:
        fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
    except PermissionError:
        print("SALTATA: /dev/uinput non scrivibile; serve sudo -S o il gruppo input")
        sys.exit(77)
    except FileNotFoundError:
        print("SALTATA: /dev/uinput non esiste (modulo uinput non caricato)")
        sys.exit(77)
    fcntl.ioctl(fd, UI_SET_EVBIT, EV_KEY)
    fcntl.ioctl(fd, UI_SET_EVBIT, EV_ABS)
    for b in PULSANTI:
        fcntl.ioctl(fd, UI_SET_KEYBIT, b)
    for a in (ABS_X, ABS_Y, ABS_HAT0X, ABS_HAT0Y):
        fcntl.ioctl(fd, UI_SET_ABSBIT, a)
    # struct uinput_user_dev: nome[80], input_id, ff_effects_max, 4 x 64 s32
    absmax, absmin = [0] * 64, [0] * 64
    for a in (ABS_X, ABS_Y):
        absmin[a], absmax[a] = -32767, 32767
    for a in (ABS_HAT0X, ABS_HAT0Y):
        absmin[a], absmax[a] = -1, 1
    dev = struct.pack("80sHHHHI", NOME, BUS_USB, 0x1234, 0x5678, 1, 0)
    dev += struct.pack("64i", *absmax) + struct.pack("64i", *absmin)
    dev += struct.pack("64i", *([0] * 64)) + struct.pack("64i", *([0] * 64))
    os.write(fd, dev)
    fcntl.ioctl(fd, UI_DEV_CREATE)
    return fd


def main():
    fd = crea()
    print("creato joystick virtuale «solomon-test»", flush=True)
    try:
        time.sleep(1.5)
        invia(fd, EV_ABS, ABS_X, 32767)
        time.sleep(0.3)
        invia(fd, EV_ABS, ABS_X, 0)
        time.sleep(0.2)
        invia(fd, EV_ABS, ABS_HAT0X, 1)
        time.sleep(0.3)
        invia(fd, EV_ABS, ABS_HAT0X, 0)
        time.sleep(0.2)
        invia(fd, EV_KEY, PULSANTI[0], 1)
        time.sleep(0.2)
        invia(fd, EV_KEY, PULSANTI[0], 0)
        time.sleep(0.5)
    finally:
        fcntl.ioctl(fd, UI_DEV_DESTROY)
        os.close(fd)
    print("joystick virtuale rimosso", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
