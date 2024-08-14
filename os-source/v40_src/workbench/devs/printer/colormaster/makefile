
VERSION=	38

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V38/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		colormaster
SUBSYSNAME=	colormaster
DISKPATH=	workbench/devs/printers/CalComp_ColorMaster

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm transfer.asm
CFILES=		data.c dospecial.c render.c prtready.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj transfer.obj \
		prtready.obj

all:		${SUBSYSNAME}.ld
		astrip $(SUBSYSNAME).ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=	35
