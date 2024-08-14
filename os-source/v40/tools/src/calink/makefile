GCC= cc
CC=cc
CFLAGS=  -O -DSUN -DDALE
CAFLAGS = -S -X70 -DAMIGA -DSUN
BIN = /usr/Metacomco/bin/
INCL = /usr/commodore/amiga/V33/internal/include
INCLL = $(INCL)/lattice
LIB = /usr/commodore/amiga/V33/internal/lib
LSTART = $(LIB)/Lstartup.obj

calink: resident.o pass1.o pass2.o init.o \
        map.o blib.o var.o sblib.o
	$(CC)   blib.o resident.o pass1.o pass2.o \
           init.o map.o var.o sblib.o -o calink 

all:	atom calink aatom aobjdump objdump

atom :  atom.o
	$(GCC)  atom.o -o atom
	rm atom.o

aatom:	atom.c
	echo -DAMIGA -DSUN atom.c:
	metacc $(CAFLAGS) -I -I$(INCLL) -I$(INCL) atom.c
	assem atom.asm -o atom.obj -c q,a,S 
	rm -f atom.asm
	alink $(LSTART),atom.obj library $(LIB)/lc.lib,$(LIB)/amiga.lib to aatom

aobjdump: objdump.c blib.c resident.c
	metacc -S -X70 -DAMIGA -I$(INCLL) -I$(INCL) objdump.c
	assem objdump.asm -o objdump.obj -c q,a,S
	rm -f objdump.asm
	metacc -S -X70 -DAMIGA -I$(INCLL) -I$(INCL) blib.c
	assem blib.asm -o blib.obj -c q,a,S
	rm -f blib.asm
#	metacc -S -X70 -DSUN -I$(INCLL) -I$(INCL) resident.c
#	assem resident.asm -o resident.obj -c q,a,S
#	rm -f resident.asm
#	alink $(LSTART),blib.obj,objdump.obj,resident.obj library $(LIB)/lc.lib,$(LIB)/amiga.lib to aobjdump
	alink $(LSTART),blib.obj,objdump.obj library $(LIB)/lc.lib,$(LIB)/amiga.lib to aobjdump

objdump: objdump.o
	$(GCC)  blib.o objdump.o -o objdump 
	rm objdump.o

blib.o: blib.c libhdr.h
	$(CC)   $(CFLAGS) -c blib.c 

sblib.o: sblib.c libhdr.h
	$(GCC)   $(CFLAGS) -c sblib.c 

resident.o: resident.c 
	$(GCC)   $(CFLAGS) -c resident.c 

pass1.o: pass1.c 
	$(GCC)   $(CFLAGS) -c pass1.c 

pass2.o: pass2.c
	$(GCC)   $(CFLAGS) -c pass2.c 

map.o: map.c 
	$(GCC)   $(CFLAGS) -c map.c 

init.o: init.c
	$(GCC)   $(CFLAGS) -c init.c 

var.o:  var.c
	$(GCC)   $(CFLAGS) -c var.c 

printa.o: printa.c
	$(GCC)   $(CFLAGS) -c printa.c

clean:
	rm -f *.o calink atom objdump

