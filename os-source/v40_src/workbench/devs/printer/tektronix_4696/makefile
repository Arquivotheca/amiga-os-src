
VERSION=    36

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		tektronix_4696
SUBSYSNAME=	tektronix_4696
DISKPATH=	workbench/devs/printers/Tektronix_4696

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm
CFILES=		data.c dospecial.c render.c transfer.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj transfer.obj

all:		${SUBSYSNAME}.ld
	astrip ${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=     35
