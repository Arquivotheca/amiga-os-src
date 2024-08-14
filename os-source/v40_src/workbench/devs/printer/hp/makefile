
VERSION=	38

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V38/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		hp
SUBSYSNAME=	hp
DISKPATH=	workbench/devs/printers/HP_LaserJet

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm transfer.asm
CFILES=		data.c dospecial.c render.c density.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj density.obj \
		transfer.obj

all:		${SUBSYSNAME}.ld
	astrip ${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=	35
