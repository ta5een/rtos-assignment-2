# vim:tabstop=2

# Compilation variables
CC=gcc
CFLAGS=-lpthread -lrt -Wall

# Executable variables
EXE_DIR=out
EXE=$(EXE_DIR)/a2

# Runtime variables
DATA_FILE=./data.txt
OUTPUT_FILE=./output.txt

.PHONY: default run clean

default: $(EXE)

$(EXE): main.c
	$(CC) $< -o $(EXE) $(CFLAGS)

run: $(EXE)
	$(EXE) $(DATA_FILE) $(OUTPUT_FILE)

clean:
	rm -f $(EXE)
