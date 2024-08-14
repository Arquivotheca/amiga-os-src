# ---------------------------------------------------------------------------------
# Makefile for: ARP  Manx36
#
# $Locker:  $
#
# $Id: makefile,v 1.4 93/12/13 17:18:39 kcd Exp $
#
# $Revision: 1.4 $
#
# $Header: Hog:Other/inet/src/c/arp/RCS/makefile,v 1.4 93/12/13 17:18:39 kcd Exp $
#
#-----------------------------------------------------------------------------------



NETBIN  = AS225:bin/c
NETSRC	= AS225:src/c/arp
NETINC  = NETINC:
CLIB	= c32.lib
BIN	= inet:c

.c.o:
	lc -DAMIGA -v -j88 -O -i$(NETINC) -o$*.o $*.c

all:	arp

arp:	arp.o nlist.o
	blink lib:c.o arp.o nlist.o to arp lib lib:lc.lib lib:amiga.lib ND NOICONS

clean:
	delete \#?.o

install:	arp
	copy arp $(BIN) clone

release:
	copy arp $(NETBIN) clone
	copy arp.c nlist.c makefile arp_rev.rev arp_rev.h $(NETSRC)

bump:
	bumprev 37 arp_rev
