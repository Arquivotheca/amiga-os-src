# -----------------------------------------------------------------------
# makefile for SHOWMOUNT (manx 36) 
#
# $Locker:  $
#
# $Id: makefile,v 1.4 90/11/12 16:34:22 martin Exp $
#
# $Revision: 1.4 $
#
# $Header: inet2:src/c/showmount/RCS/makefile,v 1.4 90/11/12 16:34:22 martin Exp $
#
#------------------------------------------------------------------------

CLIB    = c32.lib

NETINC  = NETINC:
NETBIN  = NETBIN:c
NETLIB  = NETLIB:socket32.lib
RPCLIB  = NETLIB:rpc32.lib
BIN     = inet:c
OBJ     = showmount.m

.c.m:
	cc +l -I$(NETINC) $(CFLAGS) -o $*.m $*.c

all:	showmount

showmount: version.m
	ln +ss -m +q -o $@ version.m $@.m $(RPCLIB) $(NETLIB) $(CLIB) \
				$(RPCLIB) $(NETLIB) $(CLIB)

version.m: $(OBJ)
	vers >version.c -v showmount
	cc -o version.m version.c

clean:
	delete #?.m

install: showmount
	copy showmount $(NETBIN) clone
	copy showmount $(BIN) clone

bump:
	bumprev 36 showmount_rev

