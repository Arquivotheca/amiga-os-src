
VERSION=	38

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V38/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		imagewriter
SUBSYSNAME=	imagewriter
DISKPATH=	workbench/devs/printers/ImagewriterII

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm transfer.asm
CFILES=		data.c dospecial.c render.c density.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj transfer.obj \
		density.obj

#CAFLAGS=	-l $*.lis

all:		${SUBSYSNAME}.ld
	astrip ${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=     35
