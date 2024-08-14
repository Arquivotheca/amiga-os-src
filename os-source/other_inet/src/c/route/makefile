# -------------------------------------------------------------------------
# makefile for ROUTE
#
# $Locker:  $
#
# $Id: makefile,v 1.6 92/10/20 17:02:52 bj Exp $
#
# $Revision: 1.6 $
#
# $Header: Hog:Other/inet/src/c/route/RCS/makefile,v 1.6 92/10/20 17:02:52 bj Exp $
#
#---------------------------------------------------------------------------

NETBIN = NETBIN:c
NETINC = NETINC:
BIN	= inet:c
OBJ	= route.o

.c.o:
	lc -DAMIGA -v -cfist -O -i$(NETINC) -o$*.o $*.c

.asm.o:
	casm -a $*.asm -cqvy -o $*.o -iwork1:cape/includes

.a.o:
	asm -u -iINCLUDE: $*.a

all:	route

route:	$(OBJ) makefile c.o
	blink from c.o $(OBJ) to $@ lib lib:amiga.lib lib:lc.lib SC SD ND

clean:
	delete #?.o route

install:	route
	copy route $(BIN) clone
	copy route $(NETBIN) clone

bump:
	bumprev 2 route_rev

