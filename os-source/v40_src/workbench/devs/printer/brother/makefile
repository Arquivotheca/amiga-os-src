
VERSION=	36

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		brother
SUBSYSNAME=	brother
DISKPATH=	workbench/devs/printers/Brother_HR-15XL

AFILES=		printertag.asm init.asm
CFILES=		data.c dospecial.c render.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj

all:		${SUBSYSNAME}.ld
	astrip ${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=	35
