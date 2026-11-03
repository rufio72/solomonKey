CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O0
SORGENTI_BASE = harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c $(wildcard z80_*.c)
# i moduli del motore dei nemici (nemici*.c, brief 25) entrano anche nel banco
# (brief 27), con i corpi delle attività da cui dipendono (sub_4b98 e altri);
# lo spedidore sched.c non serve al banco
SORGENTI_NEMICI = $(wildcard nemici*.c) attivita.c attivita_01_10.c
SORGENTI = $(SORGENTI_BASE) $(SORGENTI_NEMICI)
SORGENTI_SCHED = sched.c sched_proto.c
# giro di gioco del replay (brief 23): il banco senza harness.c, più spedidore e replay.c
SORGENTI_REPLAY = $(filter-out harness.c,$(SORGENTI)) sched.c replay.c
SORGENTI_DATI = $(wildcard audio_dati*.c)
INTESTAZIONI_DATI = $(wildcard audio_dati*.h)
SORGENTI_DRIVER = audio_piattaforma.c audio_ossatura.c audio_registri.c audio_interprete.c $(SORGENTI_DATI)
SORGENTI_AUDIO = audio_banco.c $(SORGENTI_DRIVER)

# strato video SDL2 (lotto H3): i bersagli SDL esistono solo se sdl2-config risponde
SDL2_CONFIG ?= sdl2-config
HA_SDL := $(shell $(SDL2_CONFIG) --version 2>/dev/null)
ifneq ($(HA_SDL),)
BERSAGLI_SDL = mostra_oracolo mostra_replay
SDL_CFLAGS := $(shell $(SDL2_CONFIG) --cflags)
SDL_LIBS := $(shell $(SDL2_CONFIG) --libs)
endif

all: harness sched_proto replay gioco_libero audio_banco audio_immagine audio_suona audio_wav rendi_oracolo video_immagine $(BERSAGLI_SDL)

harness: $(SORGENTI) ram.h routine.h e603.h rst20_tables.h nemici.h rst38_tables.h sched.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI)

sched_proto: $(SORGENTI_SCHED) sched.h ram.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_SCHED)

replay: $(SORGENTI_REPLAY) ram.h routine.h sched.h nemici.h e603.h rst20_tables.h rst38_tables.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_REPLAY)

# giro dal boot (lotto H4a): macchina, NMI una per frame, ingressi registrati
SORGENTI_LIBERO = $(filter-out harness.c,$(SORGENTI)) sched.c macchina.c gioco_libero.c

gioco_libero: $(SORGENTI_LIBERO) ram.h routine.h sched.h nemici.h e603.h rst20_tables.h rst38_tables.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_LIBERO)

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

# strato video (lotto H2): renderizzatore, grafica estratta da estrai_grafica.py
SORGENTI_GRAFICA = $(wildcard video_dati_*.c)

rendi_oracolo: rendi_oracolo.c video_rendi.c $(SORGENTI_GRAFICA) video.h
	$(CC) $(CFLAGS) -o $@ rendi_oracolo.c video_rendi.c $(SORGENTI_GRAFICA)

# andata e ritorno della grafica estratta
video_immagine: video_immagine.c $(SORGENTI_GRAFICA) video.h
	$(CC) $(CFLAGS) -o $@ video_immagine.c $(SORGENTI_GRAFICA)

# finestra SDL2 (lotto H3): stato dell'oracolo e giro replay
mostra_oracolo: mostra_oracolo.c video_sdl.c video_sdl.h video_rendi.c $(SORGENTI_GRAFICA) video.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ mostra_oracolo.c video_sdl.c video_rendi.c $(SORGENTI_GRAFICA) $(SDL_LIBS)

replay_lib.o: replay.c ram.h routine.h sched.h nemici.h e603.h rst20_tables.h rst38_tables.h
	$(CC) $(CFLAGS) -Dmain=replay_main -c -o $@ replay.c

mostra_replay: mostra_replay.c video_sdl.c video_sdl.h video_rendi.c $(SORGENTI_GRAFICA) replay_lib.o $(filter-out replay.c,$(SORGENTI_REPLAY)) video.h ram.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ mostra_replay.c video_sdl.c video_rendi.c $(SORGENTI_GRAFICA) replay_lib.o $(filter-out replay.c,$(SORGENTI_REPLAY)) $(SDL_LIBS)

clean:
	rm -f harness sched_proto replay gioco_libero audio_banco audio_immagine audio_suona audio_wav rendi_oracolo video_immagine mostra_oracolo mostra_replay replay_lib.o

.PHONY: all clean
