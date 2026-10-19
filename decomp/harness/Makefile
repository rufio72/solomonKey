CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O0

harness: harness.c routine.c ram.h
	$(CC) $(CFLAGS) -o $@ harness.c routine.c

clean:
	rm -f harness
