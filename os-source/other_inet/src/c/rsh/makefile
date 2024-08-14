# ---------------------------------------------------------------
# makefile for rsh  (SAS 5.10)
#
# $Locker:  $
#
# $Id: makefile,v 3.2 92/09/10 17:18:33 bj Exp $
#
# $Revision: 3.2 $
#
# $Header: Hog:Other/inet/src/c/rsh/RCS/makefile,v 3.2 92/09/10 17:18:33 bj Exp $
#
#----------------------------------------------------------------

NETBIN = inet:c
NETINC = NETINC:

.c.o:
	lc -v -cfist -ma -i$(NETINC) -O  $*.c

all:	rsh

rsh:	rsh.o 
	blink FROM rsh.o TO $@ SC SD ND

clean:
	delete #?.o rsh

install:	rsh
	protect rsh +p
	copy rsh $(NETBIN) clone

bump:
	bumprev 1 rsh_rev


