# ---------------------------------------------------------------------------------
# Makefile for: portmapd
#
# $Locker$
#
# $Id$
#
# $Revision$
#
# $Header$
#
#-----------------------------------------------------------------------------------

NETLIB	= NETLIB:socket32.lib
RPCLIB	= NETLIB:rpc32.lib
NETBIN  = NETBIN:serv
NETINC  = NETINC:
CLIB	= c32.lib
BIN		= inet:serv
OBJ		= portmapd.m _main.m _myexit.m

.c.m:	
	cc +l -DAMIGA -I$(NETINC) -o$*.m $*.c

all:	portmapd

portmapd: _main.m _myexit.m version.m 
	ln +ss +q -o portmapd $(OBJ) version.m $(RPCLIB) $(NETLIB) $(CLIB) \
			$(RPCLIB) $(NETLIB) $(CLIB)

version.m: $(OBJ)
	vers >version.c -v portmapd
	cc -oversion.m version.c

clean:
	delete #?.m

install:	portmapd
	copy portmapd $(BIN) clone
	copy portmapd $(NETBIN) clone

bump:
	bumprev 36 portmapd_rev

