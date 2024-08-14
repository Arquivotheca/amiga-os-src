
VERSION=    36

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		diablo_c
SUBSYSNAME=	diablo_c
DISKPATH=	workbench/devs/printers/Diablo_C-150

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm
CFILES=		data.c dospecial.c render.c transfer.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj transfer.obj

all:		${SUBSYSNAME}.ld
	astrip ${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=     35
