CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O0
SORGENTI = harness.c routine.c fondale.c porte.c
SORGENTI_SCHED = sched.c sched_proto.c

all: harness sched_proto

harness: $(SORGENTI) ram.h e603.h rst20_tables.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI)

sched_proto: $(SORGENTI_SCHED) sched.h ram.h
	$(CC) $(CFLAGS) -o $@ $(SORGENTI_SCHED)

clean:
	rm -f harness sched_proto

.PHONY: all clean
