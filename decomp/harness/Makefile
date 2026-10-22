CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O0
SORGENTI = harness.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c $(wildcard z80_*.c)
SORGENTI_SCHED = sched.c sched_proto.c
SORGENTI_DATI = $(wildcard audio_dati*.c)
INTESTAZIONI_DATI = $(wildcard audio_dati*.h)
SORGENTI_AUDIO = audio_banco.c audio_piattaforma.c audio_ossatura.c audio_registri.c audio_interprete.c $(SORGENTI_DATI)

all: harness sched_proto audio_banco audio_immagine

harness: $(SORGENTI) ram.h routine.h e603.h rst20_tables.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI)

sched_proto: $(SORGENTI_SCHED) sched.h ram.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_SCHED)

audio_banco: $(SORGENTI_AUDIO) audio.h $(INTESTAZIONI_DATI)
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_AUDIO)

# andata e ritorno dei dati estratti (estrai_dati.py)
audio_immagine: audio_immagine.c $(SORGENTI_DATI) $(INTESTAZIONI_DATI)
	$(CC) $(CFLAGS) -o $@ audio_immagine.c $(SORGENTI_DATI)

clean:
	rm -f harness sched_proto audio_banco audio_immagine

.PHONY: all clean
