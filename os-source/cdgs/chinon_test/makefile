# Makefile for chinon test code

MAKE = smake

.SUFFIXES: .asm .o

SOURCES = load.asm
OBJECTS = load.o

.asm.o:
	hx68 -A $*.asm -o$@ -iinclude: -c=QS

.c.o:
	sc $*.c

all:	kickfile maketest test.ld

kickfile: test.ld
	DoGS
	touch kickfile

maketest: maketest.o
	sc LINK $*.o PROGRAMNAME=$@

test.ld: load.o
	SLINK from $*.o TO $@ library lib:amiga.lib
	
load.o:	load.asm loadseg.asm test_code.asm load_rev.i
	hx68 -A $*.asm -o$@  -iinclude: -c=QS

test_code.asm: chinon_test
	maketest chinon_test

# this is used instead of chinon_test to test load.asm
test_prog: test_prog.c
	sc LINK $*.c NOSTACKCHECK code=far data=far nostartup

version:
	bumprev 40 load_rev

clean:
	-delete \#?.o
	-delete \#?.ld
	-delete kick.CDGS.\#?
	-delete test_code.asm
	-delete maketest
	-delete test_prog
