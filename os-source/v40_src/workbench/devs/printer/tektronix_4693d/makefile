
VERSION=	36

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		tektronix_4693D
SUBSYSNAME=	tektronix_4693D
DISKPATH=	workbench/devs/printers/Tektronix_4693D

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm transfer.asm
CFILES=		data.c dospecial.c render.c tek.c prtready.c density.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj transfer.obj \
		tek.obj prtready.obj density.obj

all:		${SUBSYSNAME}.ld
	astrip ${SUBSYSNAME}.ld
	
.INCLUDE=${MAKEMETA}
SPECIALVERNUM=		35
