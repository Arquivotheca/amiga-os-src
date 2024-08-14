
VERSION=    36

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		paintjet
SUBSYSNAME=	paintjet
DISKPATH=	workbench/devs/printers/HP_PaintJet

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm transfer.asm
CFILES=		data.c dospecial.c render.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj transfer.obj

#AFLAGS=		-l $*.lis
#CAFLAGS=	-l $*.lis

all:		${SUBSYSNAME}.ld
	astrip ${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=     35
