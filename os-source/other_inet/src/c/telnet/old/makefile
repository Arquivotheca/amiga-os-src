# ---------------------------------------------------------------------------------
# Makefile for TELNET (Manx)
#
# $Locker:  $
#
# $Id: makefile,v 1.4 90/11/12 19:42:01 martin Exp $
#
# $Revision: 1.4 $
#
# $Header: inet2:src/c/telnet/RCS/makefile,v 1.4 90/11/12 19:42:01 martin Exp $
#
#-----------------------------------------------------------------------------------
# manx 3.6 library (32 bit)

NETBIN = NETBIN:

NETLIB	= NETLIB:socket32.lib
CLIB	= c32.lib
BIN	= inet:c
OBJ	= telnet.m commands.m tmain.m network.m ring.m terminal.m utilities.m
TLIB	= telnet.lib
TELOBJ	= sys_amiga.m

.c.m:
	cc -DAMIGA=1 +l -INETINC: -o $*.m $*.c

all:	telnet

telnet:	version.m
	ln -m +q -o $@ version.m $(TELOBJ) $(TLIB) $(NETLIB) $(CLIB)

telnet.lib: $(OBJ)
	lb telnet.lib $(OBJ)

version.m: $(TLIB) $(TELOBJ)
	vers >version.c -v telnet
	cc -o version.m version.c

clean:
	delete #?.m

install:	telnet
	copy telnet $(BIN) clone
	copy telnet $(NETBIN) clone

bump:
	bumprev 36 telnet_rev

 