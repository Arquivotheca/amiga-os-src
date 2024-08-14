# -------------------------------------------------------------------------
# Makefile for: PASSWD lattice
#
# $Locker:  $
#
# $Id: makefile,v 1.3 91/09/24 13:32:59 bj Exp $
#
# $Revision: 1.3 $
#
# $Header: NW:src/c/passwd/RCS/makefile,v 1.3 91/09/24 13:32:59 bj Exp $
#
#-------------------------------------------------------------------------

NETINC  = NETINC:
BIN	= inet:c
OBJ	= passwd.o mysprintf.o bcmp.o mod.o

.asm.o:
	casm -a $*.asm -cqvy -o $*.o

.c.o:
	lc -v -b1 -O -cfist -i$(NETINC) $*.c

all:	passwd

passwd:	$(OBJ)
	blink from $(OBJ) to passwd SC SD ND

clean:
	delete #?.o

bump:
	bumprev 37 passwd_rev
