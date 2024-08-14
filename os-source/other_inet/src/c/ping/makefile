# ----------------------------------------------------------------------
# makefile for PING  (SAS 5.10 )
#
# $Locker: bj $
#
# $Id: makefile,v 1.3 93/01/15 13:58:00 bj Exp Locker: bj $
#
# $Revision: 1.3 $
#
#----------------------------------------------------------------------

NETINC = NETINC:
BIN	= inet:c
OBJ	= ping.o

.c.o:
	lc -inetinc: -v -cfist -fi -O $*.c

all:	ping

ping:	$(OBJ) makefile
	blink from lib:c.o $(OBJ) to $@ lib lib:lcmieee.lib lib:lc.lib lib:amiga.lib SC SD ND

clean:
	delete #?.o ping

install:	ping
	copy ping $(BIN) clone

bump:
	bumprev 3 ping_rev
