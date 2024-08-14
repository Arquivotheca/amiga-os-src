#
# Makefile for rlogin (Lattice)
#
# $Locker: bj $
#
# $Log:	makefile,v $
# Revision 1.11  91/08/06  13:21:28  bj
# details (again)
# 
# Revision 1.10  91/08/05  13:53:59  bj
# Details. 
# 
# Revision 1.9  91/04/24  15:16:54  bj
# bumprev to 37
# 
# Revision 1.8  90/12/05  16:12:16  bj
# details
# 
# Revision 1.7  90/12/02  21:12:31  martin
# *** empty log message ***
# 
# Revision 1.6  90/12/02  06:10:12  martin
# *** empty log message ***
# 
# Revision 1.5  90/12/02  06:07:20  martin
# *** empty log message ***
# 
# Revision 1.4  90/12/01  22:03:16  martin
# fixed to copy executable to NETBIN
# 
# Revision 1.3  90/11/28  00:31:36  bj
# Debug lines added (commented out) for using CodeProbe
# 
# Revision 1.2  90/11/13  15:38:08  bj
# fixed library name.
# 
# Revision 1.1  90/11/11  22:06:25  bj
# Initial revision
# 
#
# $Header: HOG:Other/inet/src/c/rlogin/RCS/makefile,v 1.11 91/08/06 13:21:28 bj Exp Locker: bj $
#         
# -----------------------------------------------------------------------------
# lattice linker library on the server

NETINC  = NETINC:
NETBIN  = NETBIN:c
NETLIB  = NETLIB:socketS.lib
CLIB    = c32.lib
BIN     = inet:c
OBJL    = _main.o rlogin.o console.o mysprintf.o
 
.asm.o:
        casm -a $*.asm -cqvy -o $*.o

.c.o:
	lc -v -cst -b1 -O $(CFLAGS) -i$(NETINC) -o$*.o $*.c

all:	rlogin


rlogin: $(OBJL) version.o
	blink from lib:c.o $(OBJL) version.o \
			lib lib:lc.lib lib:amiga.lib to rlogin SC SD ND

version.o: $(OBJL)
	vers >version.c -v rlogin
	lc -oversion.o version.c


clean:
	delete *.o

install:	rlogin
	copy rlogin $(NETBIN) clone

bump: 
	bumprev 37 rlogin_rev

