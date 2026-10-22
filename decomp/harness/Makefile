CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O0
SORGENTI = harness.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c $(wildcard z80_*.c)
SORGENTI_SCHED = sched.c sched_proto.c
SORGENTI_AUDIO = audio_banco.c audio_piattaforma.c audio_ossatura.c audio_registri.c audio_interprete.c

all: harness sched_proto audio_banco

harness: $(SORGENTI) ram.h routine.h e603.h rst20_tables.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI)

sched_proto: $(SORGENTI_SCHED) sched.h ram.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_SCHED)

audio_banco: $(SORGENTI_AUDIO) audio.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_AUDIO)

clean:
	rm -f harness sched_proto audio_banco

.PHONY: all clean
