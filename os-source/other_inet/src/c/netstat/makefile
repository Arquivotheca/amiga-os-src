# ---------------------------------------------------------------------------------
# Makefile for: ARP  Manx36
#
# $Locker:  $
#
# $Id: makefile,v 1.4 93/01/29 22:41:28 kcd Exp $
#
# $Revision: 1.4 $
#
# $Header: AS225:src/c/netstat/RCS/makefile,v 1.4 93/01/29 22:41:28 kcd Exp $
#
#-----------------------------------------------------------------------------------



NETBIN  = AS225:bin/c
NETSRC	= AS225:src/c/netstat
NETINC  = NETINC:
CLIB	= c32.lib
BIN	= inet:c

.c.o:
	lc -DAMIGA -v -j85 -O -i$(NETINC) -o$*.o $*.c

all:	netstat

netstat:	main.o if.o mbuf.o inet.o route.o nlist.o index.o
	blink lib:c.o main.o if.o mbuf.o inet.o route.o nlist.o index.o to netstat lib lib:lc.lib lib:amiga.lib ND NOICONS

clean:
	delete \#?.o

install: netstat
	copy netstat $(BIN) clone

release:
	copy netstat $(NETBIN) clone
	copy main.c if.c mbuf.c inet.c route.c makefile netstat_rev.rev netstat_rev.h $(NETSRC)

bump:
	bumprev 37 netstat_rev
