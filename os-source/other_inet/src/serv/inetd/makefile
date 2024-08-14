# -----------------------------------------------------------------------
# makefile for inetd  (as225 inet:serv/inetd  R2)
#
# $Locker:  $
#
# $Id: makefile,v 1.3 92/07/17 15:31:49 bj Exp $
#
# $Revision: 1.3 $
#
# $Log:	makefile,v $
# Revision 1.3  92/07/17  15:31:49  bj
# Minor changes. Added RCS header file.
# 
#
# $Header: AS225:src/serv/inetd/RCS/makefile,v 1.3 92/07/17 15:31:49 bj Exp $
#
#------------------------------------------------------------------------
# Makefile for inetd
#

NETINC = NETINC:
BIN	= inet:serv
OBJ	= inetd.o syslog.o

.c.o:
	lc -v -cfist -O -i$(NETINC) -o$@ $*.c

.asm.o:
	casm -a $*.asm -cqvy -o $*.o -iwork1:cape/includes

all:	inetd

inetd:	$(OBJ)
#	blink FROM c.o $(OBJ) LIB lib:amiga.lib lib:lc.lib TO $@ MAP SC SD ND 
	blink FROM c.o $(OBJ) LIB lib:amiga.lib lib:lc.lib lib:debug.lib TO $@ MAP SC SD ND 

clean:
	delete #?.o

install:	inetd
	copy inetd $(BIN) clone

bump:
	bumprev 1 inetd_rev
