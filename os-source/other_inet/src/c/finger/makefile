# ---------------------------------------------------------------------------------
# Makefile for: finger  Lattice 5.10
#
# $Locker:  $
#
# $Id: makefile,v 1.6 93/01/14 16:34:30 gregm Exp $
#
# $Revision: 1.6 $
#
# $Header: Hog:Other/inet/src/c/finger/RCS/makefile,v 1.6 93/01/14 16:34:30 gregm Exp $
#
#-----------------------------------------------------------------------------------

CLIB    = c32.lib
NETBIN  = NETBIN:C
NETINC  = NETINC:
NETLIB  = NETLIB:socket32.lib
BIN     = NETBIN:c
OBJ     = finger.l


.c.l:
        lc  -O -d1 -v -cfist -i$(NETINC) -o$*.l $*.c

all:    finger

finger: finger.l
        blink finger.l to finger SC SD ND lib lib:lc.lib

clean:
        delete #?.l

install: finger
        copy finger inet:c clone

bump:
        bumprev 37 finger_rev

release:
        copy finger finger_rev.rev finger_rev.h finger.c makefile hognet:src/c/finger

