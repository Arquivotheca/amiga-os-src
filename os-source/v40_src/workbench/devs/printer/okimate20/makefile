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
SRCDIR=		okimate20
SUBSYSNAME=	okimate20
DISKPATH=	workbench/devs/printers/Okimate_20

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm
CFILES=		data.c dospecial.c render.c transfer.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj transfer.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SYMBOLOPT=NULL
