# ---------------------------------------------------------------
# makefile for timed  (SAS 5.10)
#
# $Locker:  $
#
# $Id: makefile,v 1.1 92/04/06 14:17:10 dlarson Exp $
#
# $Revision: 1.1 $
#
# $Header: hog:Other/inet/src/serv/timed/RCS/makefile,v 1.1 92/04/06 14:17:10 dlarson Exp $
#
#----------------------------------------------------------------

NETINC = NETINC:
BIN    = inet:serv

.c.o:
	lc -v -cfist -O -i$(NETINC) -o$*.o $*.c

all:	timed

timed:	timed.o
	blink FROM c.o timed.o LIB lib:amiga.lib lib:lc.lib TO $@ SC SD ND

clean:
	delete #?.o timed

install:	timed
	copy timed $(BIN) clone

bump:
	bumprev 1 timed_rev


