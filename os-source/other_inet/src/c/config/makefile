# ---------------------------------------------------------------------------------
# MAKEFILE for CONFIG (SAS) using shared socket library
#
# $Locker: bj $
#
# $Id: makefile,v 1.4 91/09/09 12:24:24 bj Exp Locker: bj $
#
# $Revision: 1.4 $
#
# $Log:	makefile,v $
# Revision 1.4  91/09/09  12:24:24  bj
# changed to reflect lack of startup code being used iin the
# executable.
# 
#
# $Header: Hog:Other/inet/src/c/config/RCS/makefile,v 1.4 91/09/09 12:24:24 bj Exp Locker: bj $
#
#-----------------------------------------------------------------------------------


BIN     = NETBIN:c
OBJ     = config.o
NETINC  = NETINC:

.asm.o: 
	casm -a $*.asm -cqvy -o $*.o

.c.o:
	lc -v -d -cfistr -rr -O -i$(NETINC) $*.c

all:	config

config: $(OBJ)
	blink from $(OBJ) to config SC SD ND

bump:
	bumprev 37 config_rev

