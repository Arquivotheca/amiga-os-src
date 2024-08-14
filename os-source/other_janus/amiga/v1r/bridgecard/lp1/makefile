
all:		lpt1.ld

SUBSYSNAME=	lpt1

AFILES= parallel.asm	

CFILES=mainlpt1.c

OFILES=mainlpt1.obj parallel.obj


#MYLIBS=		${LIBDIR}/janus.lib
MAKEFILE=	Makefile
SUBSYSGROUP=	
EXTERNIFILES=
IFILES=		${EXTERNIFILES}
HFILES=		${EXTERNHFILES}
LFLAGS=		map=$*.lmap xref=$*.xref

STARTUP=	/usr/commodore/amiga/V33/internal/lib/startup.obj

CFLAGS=

.INCLUDE=makemeta
SYMBOLOPT=
