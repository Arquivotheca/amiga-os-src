# -----------------------------------------------------------------------
# makefile    for nfsc  (as225)   SAS 
#
# $Locker:  $
#
# $Id: makefile,v 1.5 92/10/07 13:13:27 bj Exp $
#
# $Revision: 1.5 $
#
# $Log:	makefile,v $
# Revision 1.5  92/10/07  13:13:27  bj
# changed so that networking link libs are pulled from the server.
# 
#
# $Header: Hog:Other/inet/src/c/nfsc/RCS/makefile,v 1.5 92/10/07 13:13:27 bj Exp $
#
#------------------------------------------------------------------------
#
#
# Makefile for AmigaNFS nfs client
#

.c.o:
	lc -v -cfist -inetinc: -O -o$*.o $*.c

.a.o:
	asm -u -iINCLUDE: $*.a

BIN	= inet:c
NFSOBJ	= main.o mount.o ptofh.o lk.o utils.o xdr.o fhcache.o\
	  volume.o examine.o perm.o

#
# Program rules
#

all:	nfsc


nfsc: $(NFSOBJ) fs.h c.o
	blink from c.o $(NFSOBJ)\
			lib as225:bin/lib/rpcsvcS.lib \
			lib as225:bin/lib/rpcS.lib \
			lib:debug.lib lib:amiga.lib lib:lc.lib to nfsc SC SD ND


install: nfsc
	copy nfsc $(BIN) clone

clean:
	delete #?.o
	delete nfsc 

bump:
	bumprev 3 nfsc_rev
