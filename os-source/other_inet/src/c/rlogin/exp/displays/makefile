#
# Makefile for rlogin (Lattice)
#         
# -----------------------------------------------------------------------------
# lattice linker library on the server

NETINC  = NETINC:
OBJL    = rl.o mysprintf.o
 
.asm.o:
        casm -a $*.asm -cqvy -o $*.o

.c.o:
	lc -v -cfist -b1 -dAMIGA -O -i$(NETINC) $*.c

all:	rl


rl: $(OBJL)
	blink from  $(OBJL) \
			lib lib:lc.lib lib:amiga.lib to rl SC SD ND

clean:
	delete *.o

