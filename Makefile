CC=gcc
CFLAGS=-Werror -Wall -Wextra -pedantic -std=c11

.PHONY: clean
.PHONY: all
.PHONY: run

all: sim

sim: mips32.h consts.h elf.o pipes.o sim.c 
	$(CC) $(CFLAGS) -o sim elf.o sim.c

pipes.o: consts.h
	$(CC) $(CFLAGS) -c pipes.c pipes.h

elf.o: elf.c elf.h
	$(CC) $(CFLAGS) -c elf.c elf.h

clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.gch

run: sim
	./sim
