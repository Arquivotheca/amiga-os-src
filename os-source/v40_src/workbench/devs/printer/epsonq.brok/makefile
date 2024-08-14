##########################################################################
#
# $Header: Makefile,v 1.0 85/04/02 19:29:23 kodiak Exp $
#
# $Locker:  $
#
# $Log:	Makefile,v $
#
##########################################################################

VERSION=    34
VERNUM=     35

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIRPATH=	workbench/printer
SRCDIR=		epsonQ
SUBSYSNAME=	epsonQ
DISKPATH=	workbench/devs/printers/EpsonQ

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm transfer.asm
CFILES=		data.c dospecial.c render.c density.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj density.obj \
			transfer.obj

#AFLAGS=	-l $*.lis
#CAFLAGS=	-l $*.lis

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SYMBOLOPT=NULL
