CC := gcc
CFLAGS := -Werror -Wall -Wextra -pedantic -std=c11

.PHONY: clean
.PHONY: all
.PHONY: run
.PHONY: debug

all: sim

debug: CFLAGS += -ggdb
debug: sim

sim: elf.o mem.o pipes.o sim.c
	$(CC) $(CFLAGS) -o sim elf.o mem.o pipes.o sim.c

mem.o: mem.c mem.h
	$(CC) $(CFLAGS) -c mem.c mem.h

pipes.o: mem.o pipes.c pipes.h
	$(CC) $(CFLAGS) -c pipes.c pipes.h

elf.o: elf.c elf.h
	$(CC) $(CFLAGS) -c elf.c elf.h

clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.gch
	rm sim

run: sim
	./sim
