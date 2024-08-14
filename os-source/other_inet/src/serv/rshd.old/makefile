# ---------------------------------------------------------------------------------
# Makefile for: RSHD  Manx 36
#
# $Locker:  $
#
# $Id: makefile,v 1.6 90/11/26 15:58:16 bj Exp $
#
# $Revision: 1.6 $
#
# $Log:	makefile,v $
# Revision 1.6  90/11/26  15:58:16  bj
# *** empty log message ***
# 
# Revision 1.5  90/11/20  17:15:20  bj
# grrrr.  
# 
#
# $Header: HOG:Other/inet/src/serv/rshd/RCS/makefile,v 1.6 90/11/26 15:58:16 bj Exp $
#
#-----------------------------------------------------------------------------------

NETLIB	= NETBIN:lib/socket32.lib
CLIB	= c32.lib
BIN	= NETBIN:serv
OBJ	= _main.o rshd.o my_popen.o _exit.o

.c.o:
	cc -DAMIGA +l -Inetinc: $*.c

all:	rshd

rshd:	version.o 
	ln -m +q -o $@ version.o $(OBJ) $(NETLIB) $(CLIB)

version.o: $(OBJ)
	vers >version.c -v rshd
	cc version.c

clean:
	delete #?.o

install:	rshd
	copy rshd $(BIN) clone
