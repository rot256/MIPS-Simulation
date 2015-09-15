CC=gcc
CFLAGS=-Werror -Wall -Wextra -pedantic -std=c11

.PHONY: clean
.PHONY: all
.PHONY: run

all: sim

sim: mips32.h elf.o sim.c
	$(CC) $(CFLAGS) -o sim.elf elf.o sim.c

elf.o: elf.c elf.h
	$(CC) $(CFLAGS) -c elf.c elf.h

clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.gch

run: sim
	./sim
