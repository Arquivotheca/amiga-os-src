# ----------------------------------------------------------------------------------
# Makefile for vers (Lattice)
#
# $Header: inet2:src/c/vers/RCS/makefile,v 1.7 90/11/06 14:19:30 bj Exp Locker: martin $
#
# $Locker: martin $
#
# $Id: makefile,v 1.7 90/11/06 14:19:30 bj Exp Locker: martin $
#
# $Revision: 1.7 $
#
# $Date: 90/11/06 14:19:30 $
#
# $Log:	makefile,v $
# Revision 1.7  90/11/06  14:19:30  bj
# More updates to new network group standard assigns. All
# Lattice stuff for this version. Manx does not exists for
# VERS right now.
# 
# Revision 1.5  90/11/05  16:50:39  bj
# no changes. RCS date test.
# 
# Revision 1.4  90/11/05  16:38:10  bj
#  (1) changed server root from INET2: to NW:
#  (2) added NETINC: reference for calling server based
#      includes.
#  (3) fixed bump: to the right make keyword (I had 'version:')
# 
# Revision 1.3  90/11/05  14:45:59  bj
# RCS adjustments. Nothing functional is changed.
# 
#
#
# ----------------------------------------------------------------------------------
# lattice linker library on server (not actually used in this!! )
HOGLIBLAT = NW:bin/lib/socketS.library


BIN     = inet:c
OBJ     = vers.l
INCLUDE = NETINC:
NETBIN  = NETBIN:c

.c.o:
	cc -DAMIGA +l -I//include $*.c

.c.l:
	lc -b1 -O $(CFLAGS) -i$(INCLUDE) -o$*.l $*.c

all:	vers

### must be linked in this order - first 'version.l' then 'vers.l'  !!!
###^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ !!!

vers: makefile version.l vers.l
	blink from lib:c.o version.l vers.l \
		lib	lib:lc.lib lib:amiga.lib to vers NODEBUG

version.l: $(OBJ)
	vers >version.c -v vers
	lc -oversion.l version.c

bump:
	bumprev 36 vers_rev

clean:              
	delete #?.l

install:  vers
	copy vers $(BIN) clone
	copy vers $(NETBIN) clone

