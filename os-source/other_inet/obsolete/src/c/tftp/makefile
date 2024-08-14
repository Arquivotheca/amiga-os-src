# -----------------------------------------------------------------------
# makefile for TFTP  (Manx 36 )
#
# $Locker$
#
# $Id$
#
# $Revision$
#
# $Header$
#
#------------------------------------------------------------------------
#

NETBIN = NETBIN:c
NETINC = NETINC:
NETLIB = NETLIB:socket32.library
CLIB   = c32.lib
BIN    = inet:c
OBJ    = tftp.m main.m tftpsubs.m

.c.m:
	cc -DAMIGA +l -I$(NETINC) -o $*.m $*.c

all:	tftp

tftp:	version.m
	ln +ss -m +q -o $@ version.m $(OBJ) $(NETLIB) $(CLIB)

version.m: $(OBJ)
	vers >version.c -v tftp
	cc -o version.m version.c

clean:
	delete #?.m

install:	tftp
	copy tftp $(BIN) clone
	copy tftp $(NETBIN) clone

bump:
	bumprev 36 tftp_rev

