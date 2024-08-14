# -----------------------------------------------------------------------
# makefile for rshd 
#
# $Locker:  $
#
# $Id: makefile,v 1.3 92/08/07 16:22:41 bj Exp $
#
# $Revision: 1.3 $
#
# $Log:	makefile,v $
# Revision 1.3  92/08/07  16:22:41  bj
# added "sprintf.o" to OBJ list
# 
# Revision 1.2  92/07/10  17:44:23  bj
# Added RCS header.  Changed reference from work1: to work:.
# 
#
# $Header: AS225:src/serv/rshd/RCS/makefile,v 1.3 92/08/07 16:22:41 bj Exp $
#
#------------------------------------------------------------------------
#
#
# Makefile for rshd
#

CLIB	= lib:debug.lib lib:amiga.lib lib:lc.lib
BIN	= inet:serv
OBJ	= sprintf.o rshd.o syslog.o async.o

.c.o:
	lc -inetinc: -v  -cfist $*.c
#	lc -inetinc: -v -d1 -cfist -O $*.c

.asm.o:
	casm -a $*.asm -cqvy -o $*.o -iwork:cape/includes

all:	rshd

rshd:	$(OBJ) 
	blink from c.o $(OBJ) TO $@ LIB $(CLIB) SC SD ND

clean:
	delete *.o rshd

install:	rshd
	copy rshd $(BIN) clone

bump:
	bumprev 1 rshd_rev
