# Handout for G1, ARK15, DIKU.
#
# Copyright (c) 2015 DIKU.
# Copenhagen Liberal License - v0.5 - September 6, 2015.
#
# To cross-compile all assembly files into executables, type
#
# $ make all
#
# To clean things up, type
#
# $ make clean
#
# Authors:
#		Annie Jane Pinder <anpi@di.ku.dk>
#  	Oleksandr Shturmov <oleks@oleks.info>

.PHONY: all clean

SRC=$(patsubst %.S,%.elf,$(wildcard *.S))

all: $(SRC)

%.elf: %.S
	mips-elf-as -mips32 -o $*.o --fatal-warnings $*.S
	mips-elf-ld -o $*.elf $*.o

clean:
	rm -f *.o
	rm -f *.elf
