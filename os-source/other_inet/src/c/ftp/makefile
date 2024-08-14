# ----------------------------------------------------------------------
# makefile for ftp (SAS 5.10 )
#
# $Locker:  $
#
# $Id: makefile,v 1.2 91/08/06 15:46:54 martin Exp $
#
# $Revision: 1.2 $
#
#----------------------------------------------------------------------

NETINC = NETINC:
BIN	= inet:c
OBJ	= main.o ftp.o cmds.o cmdtab.o glob.o domacro.o exit.o \
	ruserpass.o password.o gettimeofday.o

.c.o:
	lc -fi -v -cfist -i$(NETINC) $*.c

.a.o:
	asm -u -iINCLUDE: $*.a

.asm.o:
	casm -A $*.asm -o $*.o


all:	ftp


ftp:	$(OBJ) makefile inet:lib/unixS.lib
	blink from lib:c.o $(OBJ) to $@ lib inet:lib/unixS.lib lib:lcmieee.lib lib:lc.lib lib:amiga.lib SC SD ND

clean:
	delete #?.o ftp

install:	ftp
	copy ftp $(BIN) clone

bump:
	bumprev 2 ftp_rev
