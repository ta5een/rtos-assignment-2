# vim:tabstop=2

CC=gcc
CFLAGS=-lpthread -lrt -Wall

OUT=out
EXE=$(OUT)/a2

.PHONY: default run clean

default: $(EXE)

$(EXE): main.c
	$(CC) $< -o $(EXE) $(CFLAGS)

run: $(EXE)
	$(EXE)

clean:
	rm -f $(EXE)
