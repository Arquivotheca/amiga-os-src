# -----------------------------------------------------------------------
# makefile     for rlogin v38.xx
#
# $Locker:  $
#
# $Id: makefile,v 38.0 93/04/13 11:30:35 bj Exp $
#
# $Revision: 38.0 $
#
# $Log:	makefile,v $
# Revision 38.0  93/04/13  11:30:35  bj
# makefile for V38 rlogin (complete rewrite)
# 
#
# $Header: AS225:src/c/rlogin/RCS/makefile,v 38.0 93/04/13 11:30:35 bj Exp $
#
#------------------------------------------------------------------------
#

OBJ	  = main.o mainloop.o mysprintf.o support.o console.o getprefs.o sockets.o clip.o newlist.o menus.o

.asm.o:
	casm -a $*.asm -cqvy -iinc:  -o $*.o

.c.o:
	lc -v -b1 -O -cfist -iNETINC: $*.c

all: rlogin

rlogin:	$(OBJ) makefile rl.h
	blink from $(OBJ) to rlogin SC SD ND

clean:
	delete #?.o

bump:
	bumprev 38 rlogin_rev

