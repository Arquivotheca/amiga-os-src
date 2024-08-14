# -----------------------------------------------------------------------
# makefile for nfsmgr (as225) 
#
# $Locker:  $
#
# $Id: makefile,v 1.6 94/03/24 22:14:47 jesup Exp $
#
# $Revision: 1.6 $
#
# $Log:	makefile,v $
# Revision 1.6  94/03/24  22:14:47  jesup
# BJ didn't check this in
# 
# Revision 1.5  92/12/01  15:39:08  bj
# binary 37.2
# 
# New header.  New paths to libs. 
# Converted to SAS from Manx.
# 
#
# $Header: Hog:Other/inet/src/c/nfsmgr/RCS/makefile,v 1.6 94/03/24 22:14:47 jesup Exp $
#
#------------------------------------------------------------------------
#

NETINC	= netinc:
RPCLIB	= as225:bin/lib/rpcS.lib
BIN	= inet:c
OBJ	= util.o y.tab.o
CFLAGS	=  -DAMIGA -v -cfist -O

.c.o:
	lc $(CFLAGS) -i$(NETINC) $*.c

all:	nfsmgr

nfsmgr:	$(OBJ)
#	blink FROM lib:c.o $(OBJ) to $@ LIB $(RPCLIB) lib:lc.lib SC SD ND
	blink FROM lib:c.o $(OBJ) to $@ LIB $(RPCLIB) lib:lc.lib lib:amiga.lib lib:debug.lib SC SD ND

y.tab.c:	nfsmgr.y
	yacc nfsmgr.y

y.tab.o: y.tab.c
	lc $(CFLAGS) -i$(NETINC) y.tab.c

y.tab.o: y.tab.c

clean:
	delete #?.o y.tab.c

bump:
	bumprev 37 nfsmgr_rev

install:	nfsmgr
	copy nfsmgr $(BIN) clone
