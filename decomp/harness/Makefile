CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O0
SORGENTI = harness.c routine.c fondale.c porte.c

harness: $(SORGENTI) ram.h e603.h rst20_tables.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI)

clean:
	rm -f harness
