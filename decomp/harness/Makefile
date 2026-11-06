CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O0 -I. -I$(TECMO)
# tabelle Tecmo del Z80 principale (estrai_rom.py, brief 62): ricompongono rom[]
TECMO = ../../tecmo
SORGENTI_ROM = $(wildcard $(TECMO)/rom_dati_*.c) $(TECMO)/rom_voci.c
SORGENTI_BASE = harness.c comune.c routine.c fondale.c porte.c nmi.c nmi_oggetti.c routine_tab.c $(wildcard z80_*.c) $(SORGENTI_ROM)
# i moduli del motore dei nemici (nemici*.c, brief 25) entrano anche nel banco
# (brief 27), con i corpi delle attività da cui dipendono (sub_4b98 e altri);
# lo spedidore sched.c non serve al banco
SORGENTI_NEMICI = $(wildcard nemici*.c) attivita.c attivita_01_10.c attivita_attract.c attivita_attract07.c attivita_attract07a.c attivita_attract07b.c attivita_attract07c.c attivita_attract07d.c attivita_attract08.c attivita_piccoli.c attivita_12_13.c attivita_18.c attivita_1c.c attivita_22.c attivita_25.c attivita_26.c attivita_15.c attivita_16.c attivita_16b.c attivita_24.c attivita_27.c attivita_06b.c raccolta_13xx.c banco_corpi.c
SORGENTI = $(SORGENTI_BASE) $(SORGENTI_NEMICI)
SORGENTI_SCHED = sched.c sched_proto.c
# giro di gioco del replay (brief 23): il banco senza harness.c, più spedidore e replay.c
SORGENTI_REPLAY = $(filter-out harness.c,$(SORGENTI)) sched.c replay.c
# dati audio estratti in tecmo/ (estrai_dati.py), serializzatore e macro in harness/;
# codice del driver 0000-05C5 in tecmo/audio_driver.c (estrai_rom.py, brief 63)
SORGENTI_DATI = audio_dati.c $(wildcard $(TECMO)/audio_dati_*.c)
INTESTAZIONI_DATI = audio_dati.h $(wildcard $(TECMO)/audio_dati_*.h)
SORGENTI_DRIVER = audio_piattaforma.c audio_ossatura.c audio_registri.c audio_interprete.c $(SORGENTI_DATI) $(TECMO)/audio_driver.c
SORGENTI_AUDIO = audio_banco.c $(SORGENTI_DRIVER)

# strato video SDL2 (lotto H3): i bersagli SDL esistono solo se sdl2-config risponde
SDL2_CONFIG ?= sdl2-config
HA_SDL := $(shell $(SDL2_CONFIG) --version 2>/dev/null)
ifneq ($(HA_SDL),)
BERSAGLI_SDL = mostra_oracolo mostra_replay gioca
SDL_CFLAGS := $(shell $(SDL2_CONFIG) --cflags)
SDL_LIBS := $(shell $(SDL2_CONFIG) --libs)
endif

all: harness sched_proto replay gioco_libero audio_banco audio_immagine audio_suona audio_wav rendi_oracolo video_immagine rom_immagine $(BERSAGLI_SDL)

harness: $(SORGENTI) ram.h $(TECMO)/rom_dati.h routine.h e603.h rst20_tables.h nemici.h rst38_tables.h sched.h attivita_libera.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI)

sched_proto: $(SORGENTI_SCHED) sched.h ram.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_SCHED)

replay: $(SORGENTI_REPLAY) ram.h $(TECMO)/rom_dati.h routine.h sched.h nemici.h e603.h rst20_tables.h rst38_tables.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_REPLAY)

# giro dal boot (lotto H4a): macchina, NMI una per frame, ingressi registrati
# (H4c: con il modulo video per gli hash dei framebuffer)
SORGENTI_LIBERO = $(filter-out harness.c,$(SORGENTI)) sched.c macchina.c gioco_libero.c giro_video.c video_rendi.c $(SORGENTI_GRAFICA)

gioco_libero: $(SORGENTI_LIBERO) ram.h $(TECMO)/rom_dati.h routine.h sched.h nemici.h e603.h rst20_tables.h rst38_tables.h video.h giro_video.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_LIBERO)

audio_banco: $(SORGENTI_AUDIO) audio.h $(INTESTAZIONI_DATI) $(TECMO)/audio_driver.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_AUDIO)

# andata e ritorno dell'immagine di 1.3jk (driver e dati di tecmo/)
audio_immagine: audio_immagine.c $(SORGENTI_DATI) $(TECMO)/audio_driver.c $(INTESTAZIONI_DATI) $(TECMO)/audio_driver.h
	$(CC) $(CFLAGS) -o $@ audio_immagine.c $(SORGENTI_DATI) $(TECMO)/audio_driver.c

# un comando dal C del driver, senza oracolo (brief 22)
audio_suona: audio_suona.c $(SORGENTI_DRIVER) audio.h $(INTESTAZIONI_DATI) $(TECMO)/audio_driver.h
	$(CC) $(CFLAGS) -o $@ audio_suona.c $(SORGENTI_DRIVER)

# renderizzatore dei flussi AY in wav con emu2149 (brief 22)
audio_wav: audio_wav.c emu2149/emu2149.c emu2149/emu2149.h
	$(CC) $(CFLAGS) -o $@ audio_wav.c emu2149/emu2149.c

# strato video (lotto H2): renderizzatore, grafica estratta da estrai_grafica.py in tecmo/
SORGENTI_GRAFICA = $(wildcard $(TECMO)/video_dati_*.c)

rendi_oracolo: rendi_oracolo.c video_rendi.c $(SORGENTI_GRAFICA) video.h
	$(CC) $(CFLAGS) -o $@ rendi_oracolo.c video_rendi.c $(SORGENTI_GRAFICA)

# andata e ritorno delle tabelle Tecmo (tecmo/) contro out/main64.bin
rom_immagine: rom_immagine.c $(SORGENTI_ROM) $(TECMO)/rom_dati.h
	$(CC) $(CFLAGS) -o $@ rom_immagine.c $(SORGENTI_ROM)

# andata e ritorno della grafica estratta
video_immagine: video_immagine.c $(SORGENTI_GRAFICA) video.h
	$(CC) $(CFLAGS) -o $@ video_immagine.c $(SORGENTI_GRAFICA)

# finestra SDL2 (lotto H3): stato dell'oracolo e giro replay
mostra_oracolo: mostra_oracolo.c video_sdl.c video_sdl.h video_rendi.c $(SORGENTI_GRAFICA) video.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ mostra_oracolo.c video_sdl.c video_rendi.c $(SORGENTI_GRAFICA) $(SDL_LIBS)

replay_lib.o: replay.c ram.h $(TECMO)/rom_dati.h routine.h sched.h nemici.h e603.h rst20_tables.h rst38_tables.h
	$(CC) $(CFLAGS) -Dmain=replay_main -c -o $@ replay.c

mostra_replay: mostra_replay.c video_sdl.c video_sdl.h video_rendi.c $(SORGENTI_GRAFICA) replay_lib.o $(filter-out replay.c,$(SORGENTI_REPLAY)) video.h ram.h $(TECMO)/rom_dati.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ mostra_replay.c video_sdl.c video_rendi.c $(SORGENTI_GRAFICA) replay_lib.o $(filter-out replay.c,$(SORGENTI_REPLAY)) $(SDL_LIBS)

# il gioco si gioca (lotto H5): macchina libera o con oracolo, tastiera, DSW
SORGENTI_GIOCA = $(filter-out gioco_libero.c,$(SORGENTI_LIBERO)) gioca.c gioca_tasti.c gioca_joy.c gioca_diag.c gioca_opzioni.c gioca_aiuto.c gioca_dsw.c macchina_libera.c video_sdl.c scheda_audio.c scheda_pcm.c gioca_audio.c $(SORGENTI_DRIVER) emu2149/emu2149.c

gioca: $(SORGENTI_GIOCA) macchina.h ram.h $(TECMO)/rom_dati.h routine.h sched.h nemici.h e603.h rst20_tables.h rst38_tables.h video.h video_sdl.h giro_video.h scheda_audio.h audio.h gioca_joy.h gioca_opzioni.h $(INTESTAZIONI_DATI) $(TECMO)/audio_driver.h emu2149/emu2149.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ $(SORGENTI_GIOCA) $(SDL_LIBS)

# prova dei pixel delle scanlines (addendum 57C): fuori da all, si lancia
# con SDL_VIDEODRIVER=dummy ./prova_scanlines
prova_scanlines: prova_scanlines.c video_sdl.c video_sdl.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ prova_scanlines.c $(SDL_LIBS)

# make gioco compila gioca e basta: nessuna ROM a runtime né in compilazione
# (Z80 principale, driver, musica e grafica in tecmo/).
ROM_DIR = ../../roms/ext/solomon
ifeq ($(HA_SDL),)
gioco:
	@echo "ERRORE: SDL2 non trovato. Installa libsdl2-dev (sudo apt install libsdl2-dev) e riprova." >&2; exit 2
else
gioco: gioca
	@echo "Pronto: lancia decomp/harness/gioca (gioca --aiuto per le opzioni)"
endif

# strumento di sviluppo per chi ha le ROM: estrae roms/solomon.zip e
# costruisce out/main64.bin (servono a rom_immagine e agli estrattori di decomp/)
rom:
	@if [ ! -f $(ROM_DIR)/6.3f ] || [ ! -f $(ROM_DIR)/1.3jk ]; then \
	  if [ -f ../../roms/solomon.zip ]; then \
	    echo "estraggo roms/solomon.zip"; mkdir -p $(ROM_DIR) && \
	    unzip -o -q -d $(ROM_DIR) ../../roms/solomon.zip || exit 2; \
	  else echo "ERRORE: mancano le ROM: metti solomon.zip in roms/" >&2; exit 2; fi; \
	fi; \
	if [ ! -s ../out/main64.bin ]; then echo "costruisco out/main64.bin"; sh ../build_rom.sh >/dev/null || exit 2; fi

clean:
	rm -f harness sched_proto replay gioco_libero audio_banco audio_immagine audio_suona audio_wav rendi_oracolo video_immagine rom_immagine mostra_oracolo mostra_replay gioca prova_scanlines replay_lib.o

.PHONY: all clean gioco rom
