# ---------------------------------------------------------------
# makefile for RCS  (SAS 5.10)
#
# $Locker:  $
#
# $Id: makefile,v 1.1 92/04/06 14:19:41 dlarson Exp $
#
# $Revision: 1.1 $
#
# $Header: hog:Other/inet/src/c/rsh/RCS/makefile,v 1.1 92/04/06 14:19:41 dlarson Exp $
#
#----------------------------------------------------------------

NETLIB = NETLIB:socket32.lib
NETBIN = inet:c
NETINC = NETINC:


.c.o:
	lc -v -cfist -ma -i$(NETINC) -O  $*.c

all:	rsh

rsh:	rsh.o async.o
	blink FROM c.o async.o rsh.o TO $@ LIB lib:amiga.lib lib:lc.lib SC SD ND

clean:
	delete #?.o rsh

install:	rsh
	copy rsh $(NETBIN) clone

bump:
	bumprev 3 rsh_rev


