# ---------------------------------------------------------------------------------
# Makefile for: LAST (Manx36)
#
# $Locker:  $
#
# $Id: makefile,v 1.2 90/11/12 17:01:08 martin Exp $
#
# $Revision: 1.2 $
#
# $Header: inet2:src/c/last/RCS/makefile,v 1.2 90/11/12 17:01:08 martin Exp $
#
#-----------------------------------------------------------------------------------
#
NETBIN = NETBIN:c
NETINC = NETINC:
NETLIB = NETLIB:socket32.lib
CLIB	= c32.lib
BIN	= inet:c
OBJ	= last.m

.c.m:
	cc -DAMIGA +L -I$(NETINC) -o $*.m $*.c

all:	last

last:	version.m
	ln -m +q -o $@ version.m $(OBJ) $(NETLIB) $(CLIB)

version.m: $(OBJ)
	vers >version.c -v last
	cc -o version.m version.c

clean:
	delete #?.m

install:	last
	copy last $(BIN) clone
	copy last $(NETBIN) clone

bump:
	bumprev 36 last_rev
