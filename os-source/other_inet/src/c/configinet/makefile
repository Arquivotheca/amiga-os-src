# ---------------------------------------------------------------------------------
# MAKEFILE for CONFIGINET (SAS) 
#
# $Locker:  $
#
# $Id: makefile,v 1.0 92/07/09 15:19:41 bj Exp $
#
# $Revision: 1.0 $
#
# $Log:	makefile,v $
# Revision 1.0  92/07/09  15:19:41  bj
# Initial revision
# 
#
# $Header: AS225:src/c/configinet/RCS/makefile,v 1.0 92/07/09 15:19:41 bj Exp $
#
#-----------------------------------------------------------------------------------


OBJ     = configinet.o mysprintf.o
NETINC  = NETINC:

.asm.o: 
	casm -a $*.asm -cqvy -o $*.o

.c.o:
	lc -v -d -cfistr -rr -O -i$(NETINC) $*.c

all:	configinet

configinet: $(OBJ)
	blink from $(OBJ) to configinet SC SD ND

bump:
	bumprev 1 configinet_rev

