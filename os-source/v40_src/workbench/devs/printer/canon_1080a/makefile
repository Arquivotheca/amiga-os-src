VERSION=    36

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		canon_1080a
SUBSYSNAME=	canon_1080a
DISKPATH=	workbench/devs/printers/Canon_PJ-1080A

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm
CFILES=		data.c dospecial.c render.c transfer.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj transfer.obj

#CFLAGS=    -l $*.lis

all:		${SUBSYSNAME}.ld
	astrip ${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=     35
