# ---------------------------------------------------------------------------------
# Makefile for: lance-test
#
# $Locker:  $
#
# $Id: makefile,v 1.5 90/12/04 18:25:20 martin Exp $
#
# $Revision: 1.5 $
#
# $Header: inet2:src/c/lance-test/RCS/makefile,v 1.5 90/12/04 18:25:20 martin Exp $
#
#-----------------------------------------------------------------------------------

NETINC = NETINC:
NETBIN = NETBIN:c
NETLIB = NETLIB:socketS.lib
LIB	= lib:lc.lib lib:amiga.lib
OBJ	= version.l lance-test.l ffs.l inet_up.l
BIN = inet:c

.c.l: 
	lc -v -i$(NETINC) -o$*.l $*.c

all: lance-test

lance-test:  makefile $(OBJ) 
	Blink from lib:c.o $(OBJ)  \
	       lib $(LIB) to lance-test SC SD NODEBUG 
	delete version.*       	       
	       
version.l: 
	vers >version.c -v lance-test
	lc -oversion.l version.c

bump:
	bumprev 36 lance-test_rev

install: lance-test
	copy lance-test $(BIN) clone
	copy lance-test $(NETBIN) clone

clean:
	delete #?.l
	delete lance-test


