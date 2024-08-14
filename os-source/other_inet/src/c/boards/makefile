# ---------------------------------------------------------------------------------
# Makefile for: BOARDS  (Manx 36)
#
# $Locker:  $
#
# $Id: makefile,v 1.2 90/11/12 16:45:50 martin Exp $
#
# $Revision: 1.2 $
#
# $Header: inet2:src/c/boards/RCS/makefile,v 1.2 90/11/12 16:45:50 martin Exp $
#
#-----------------------------------------------------------------------------------
NETBIN = NETBIN:c
NETINC = NETINC:
NETLIB = NETLIB:socket32.lib
CLIB   = c32.lib
BIN    = inet:c
OBJ    = boards.m

.c.m:
	cc -DAMIGA +l -I$(NETINC) -o$*.m $*.c

all:	boards

boards:	version.m
	ln +ss -m +q -o $@ version.m $(OBJ) $(NETLIB) $(CLIB)

version.m: $(OBJ)
	vers >version.c -v boards
	cc -oversion.m version.c

clean:
	delete #?.m

install:	boards
	copy boards $(BIN) clone
	copy boards $(NETBIN) clone

bump:
	bumprev 36 boards_rev
