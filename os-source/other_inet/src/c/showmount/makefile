# -----------------------------------------------------------------------
# makefile for inet:c/showmount (AS225)   SAS
#
# $Locker:  $
#
# $Id: makefile,v 1.8 92/10/19 15:52:15 bj Exp $
#
# $Revision: 1.8 $
#
# $Log:	makefile,v $
# Revision 1.8  92/10/19  15:52:15  bj
# removed RCS checkin and checkout of *rev files in the
# 'bump' lines
# 
# Revision 1.7  92/10/19  15:46:39  bj
# Fixes RCS header in previous checkin.
# 
# Revision 1.6  92/10/19  15:26:17  bj
# RCS header and altered to mne own assigns.
# 
#
# $Header: Hog:Other/inet/src/c/showmount/RCS/makefile,v 1.8 92/10/19 15:52:15 bj Exp $
#
#------------------------------------------------------------------------
#
#
# 'as225' = heartofgold:/Other/networking/as225
#

NETLIB	= as225:bin/lib/rpcS.lib
CLIB	= lib:amiga.lib lib:lc.lib
BIN	= inet:c
OBJ	= showmount.o

.c.o:
	lc -v -O -cfist -DAMIGA -inetinc: $*.c

.a.o:
	asm -u -iINCLUDE: $*.a

all:	showmount

showmount: $(OBJ) c.o
	blink FROM c.o $(OBJ) TO $@ LIB $(NETLIB) $(CLIB) SC SD ND

clean:
	delete #?.o showmount

install:	showmount
	copy showmount $(BIN) clone

bump:
	bumprev 2 showmount_rev


