
VERSION=	38

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V38/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		seiko_5300a
SUBSYSNAME=	seiko_5300a
DISKPATH=	workbench/devs/printers/Seiko_5300a

#MYLIBS=	${LIBDIR}/debug.lib /usr/maryann/daveb/V/dprintf/ddebug.lib
MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm transfer.asm
CFILES=		data.c dospecial.c render.c density.c prtready.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj transfer.obj \
		density.obj prtready.obj

#AFLAGS=	-l $*.lis
#CAFLAGS=	-l $*.lis

all:		${SUBSYSNAME}.ld
	astrip $(SUBSYSNAME).ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=     35
