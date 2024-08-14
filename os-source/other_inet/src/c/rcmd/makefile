# ---------------------------------------------------------------
# makefile for rcmd  (SAS 5.10)
#
# $Locker:  $
#
# $Id: makefile,v 1.1 92/04/06 14:20:44 dlarson Exp $
#
# $Revision: 1.1 $
#
# $Header: hog:Other/inet/src/c/rcmd/RCS/makefile,v 1.1 92/04/06 14:20:44 dlarson Exp $
#
#----------------------------------------------------------------

NETBIN = inet:c
NETINC = NETINC:

.c.o:
	lc -v -cfist -ma -i$(NETINC) -O  $*.c

all:	rcmd

rcmd:	rsh.o 
	blink FROM rsh.o TO $@ SC SD ND

clean:
	delete #?.o rcmd

install:	rcmd
	protect rcmd +p
	copy rcmd $(NETBIN) clone

bump:
	bumprev 1 rcmd_rev


