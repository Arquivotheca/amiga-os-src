
VERSION=    37

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		generic
SUBSYSNAME=	generic
DISKPATH=	workbench/devs/printers/generic

AFILES=		printertag.asm init.asm
CFILES=		data.c dospecial.c render.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj

all:		${SUBSYSNAME}.ld
	astrip ${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=     35
