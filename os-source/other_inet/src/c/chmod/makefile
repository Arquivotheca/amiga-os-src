# -----------------------------------------------------------------------
# makefile for chmod  AS225 Release 2
#
# $Locker:  $
#
# $Id: makefile,v 1.4 92/11/24 15:59:26 bj Exp $
#
# $Revision: 1.4 $
#
# $Log:	makefile,v $
# Revision 1.4  92/11/24  15:59:26  bj
# new for binary 37.2
# 
# Added RCS header and updated to standard make form.
# 
#
# $Header: Hog:Other/inet/src/c/chmod/RCS/makefile,v 1.4 92/11/24 15:59:26 bj Exp $
#
#------------------------------------------------------------------------

NETINC  = NETINC:
OBJL    = chmod.o unixprotect.o
 
.asm.o:
        casm -a $*.asm -cqvy -o $*.o

.c.o: 
	lc -v -cst -b1 -O $(CFLAGS) -i$(NETINC) -o$*.o $*.c

all: chmod


chmod: $(OBJL) chmod.h makefile
#	blink from lib:c.o $(OBJL) lib lib:lc.lib lib:amiga.lib to chmod SC SD ND
	blink from $(OBJL) to chmod SC SD ND

clean:
	delete *.o

bump:
	bumprev 37 chmod_rev
