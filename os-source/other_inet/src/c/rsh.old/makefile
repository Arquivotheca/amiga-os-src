# ---------------------------------------------------------------
# makefile for RCS  (manx 36)
#
# $Locker:  $
#
# $Id: makefile,v 1.2 90/11/12 19:16:54 martin Exp $
#
# $Revision: 1.2 $
#
# $Header: inet2:src/c/rsh/RCS/makefile,v 1.2 90/11/12 19:16:54 martin Exp $
#
#----------------------------------------------------------------

NETLIB = NETLIB:socket32.lib
NETBIN = NETBIN:c
NETINC = NETINC:

CLIB   = c32.lib
BIN    = inet:c
OBJ    = rsh.m

.c.m:
	cc +l -I$(NETINC) -o $*.m $*.c

all:	rsh

rsh:	version.m
	ln +ss -m +q -o $@ version.m $(OBJ) $(NETLIB) $(CLIB)

version.m: $(OBJ)
	vers >version.c -v rsh
	cc -o version.m version.c

clean:
	delete #?.m

install:	rsh
	copy rsh $(NETBIN) clone

bump:
	bumprev 36 rsh_rev

