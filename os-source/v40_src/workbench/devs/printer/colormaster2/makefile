
VERSION=	38

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V38/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		colormaster2
SUBSYSNAME=	colormaster2
DISKPATH=	workbench/devs/printers/CalComp_ColorMaster2

MYLIBS=		${LIBDIR}/debug.lib

AFILES=		printertag.asm init.asm transfer.asm
CFILES=		data.c dospecial.c render.c prtready.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj transfer.obj \
		prtready.obj

#AFLAGS=	-l $*.lis
#CAFLAGS=	-l $*.lis

all:		${SUBSYSNAME}.ld
	astrip $(SUBSYSNAME).ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=	35
