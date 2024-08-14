
VERSION=	38

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V38/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		xerox_4020
SUBSYSNAME=	xerox_4020
DISKPATH=	workbench/devs/printers/Xerox_4020

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm
CFILES=		data.c dospecial.c render.c transfer.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj transfer.obj

all:		${SUBSYSNAME}.ld
	astrip $(SUBSYSNAME).ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=     35
