
MAKEFILE=	Makefile
SUBSYSNAME=	snoop

SRCDIRPATH=	Toolkit
SRCDIR=		snoop
DISKPATH=	Toolkit/c/snoop

#LFLAGS=		map=${SUBSYSNAME}.lmap
MYLIBS=		${LIBDIR}/debug.lib

AFILES= 	newalloc.asm
CFILES=		$(SUBSYSNAME).c
OFILES=		$(SUBSYSNAME).obj newalloc.obj

STARTUP=	${LIBDIR}/startup.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=/usr/commodore/amiga/V/tools/makemeta

install:
	cp ${SUBSYSNAME}.doc $(VERDIR)/internal/doc/Toolkit

strip:
	mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
	stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

