CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O0
SORGENTI = harness.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c $(wildcard z80_*.c)
SORGENTI_SCHED = sched.c sched_proto.c
SORGENTI_DATI = $(wildcard audio_dati*.c)
INTESTAZIONI_DATI = $(wildcard audio_dati*.h)
SORGENTI_DRIVER = audio_piattaforma.c audio_ossatura.c audio_registri.c audio_interprete.c $(SORGENTI_DATI)
SORGENTI_AUDIO = audio_banco.c $(SORGENTI_DRIVER)

all: harness sched_proto audio_banco audio_immagine audio_suona audio_wav

harness: $(SORGENTI) ram.h routine.h e603.h rst20_tables.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI)

sched_proto: $(SORGENTI_SCHED) sched.h ram.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_SCHED)

audio_banco: $(SORGENTI_AUDIO) audio.h $(INTESTAZIONI_DATI)
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_AUDIO)

# andata e ritorno dei dati estratti (estrai_dati.py)
audio_immagine: audio_immagine.c $(SORGENTI_DATI) $(INTESTAZIONI_DATI)
	$(CC) $(CFLAGS) -o $@ audio_immagine.c $(SORGENTI_DATI)

# un comando dal C del driver, senza oracolo (brief 22)
audio_suona: audio_suona.c $(SORGENTI_DRIVER) audio.h $(INTESTAZIONI_DATI)
	$(CC) $(CFLAGS) -o $@ audio_suona.c $(SORGENTI_DRIVER)

# renderizzatore dei flussi AY in wav con emu2149 (brief 22)
audio_wav: audio_wav.c emu2149/emu2149.c emu2149/emu2149.h
	$(CC) $(CFLAGS) -o $@ audio_wav.c emu2149/emu2149.c

clean:
	rm -f harness sched_proto audio_banco audio_immagine audio_suona audio_wav

.PHONY: all clean
