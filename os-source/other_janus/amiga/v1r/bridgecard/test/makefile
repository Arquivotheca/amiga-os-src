MAKEFILE=	Makefile
MAKEMETA=       /usr/commodore/amiga/V/tools/makemeta

SUBSYSNAME=	lpt1
MYLIBS=		${LIBDIR}/debug.lib

AFILES= 	parallel.asm	
CFILES=		mainlpt1.c
OFILES=		mainlpt1.obj parallel.obj
STARTUP=        ${LIBDIR}/startup.obj

#LFLAGS=		map=$*.lmap xref=$*.xref

all:		lpt1.ld
SYMBOLOPT=
.INCLUDE=${MAKEMETA}
