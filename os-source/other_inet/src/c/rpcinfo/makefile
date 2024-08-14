# ----------------------------------------------------------------------
# 
#
# $Locker:  $
#
# $Id: makefile,v 1.2 90/11/12 16:40:34 martin Exp $
#
# $Revision: 1.2 $
#
# $Header: inet2:src/c/rpcinfo/RCS/makefile,v 1.2 90/11/12 16:40:34 martin Exp $
#
#-----------------------------------------------------------------------
#

CLIB	= c32.lib
NETLIB	= NETLIB:socket32.lib
NETINC  = NETINC:
NETBIN  = NETBIN:c
RPCLIB 	= NETLIB:rpc32.lib

BIN	= NETBIN:c
OBJ	= rpcinfo.m 

.c.m:
	cc -n +l -I$(NETINC)  -o $*.m $*.c

all:	rpcinfo

rpcinfo: version.m 
	ln -m +q -o $@ version.m $(OBJ) $(RPCLIB) $(NETLIB) $(CLIB)

version.m: $(OBJ)
	vers >version.c -v rpcinfo
	cc -o version.m version.c

clean:
	delete #?.m

install: rpcinfo
	copy rpcinfo $(BIN) clone
bump:
	bumprev 36 rpcinfo_rev

