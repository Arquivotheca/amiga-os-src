#
# Makefile for inetd (Lattice)
#
# $Locker:  $
#
# $Log:	makefile,v $
#
# $Header: HOG:Other/inet/src/c/rlogin/RCS/makefile,v 1.8 90/12/05 16:12:16 bj Exp $
#         
# -----------------------------------------------------------------------------
# lattice linker 

NETINC  = NETINC:
NETLIB	= NETLIB:socketS.lib
CLIB	= c32.lib
BIN 	= inet:c
OBJ		= inetd.o config.o mysprintf.o
 
.c.o:
	lc -v -cst -b1 -O $(CFLAGS) -i$(NETINC) -o$*.o $*.c

all:	inetd

.asm.o:
	casm -A $*.asm -cqvy -o $*.o -i work2:cape/includes

inetd: $(OBJ)
	blink from lib:c.o $(OBJ) \
			lib lib:lc.lib lib:amiga.lib lib:debug.lib to inetd.test SC SD ND

clean:
	delete #?.l

install:	inetd
	copy inetd inet:serv clone

bump: 
	bumprev 37 inetd_rev

