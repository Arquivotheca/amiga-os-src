MAKEFILE=	Makefile
VERSION= 36
V=/usr/commodore/amiga/V$(VERSION)

SUBSYSNAME= db

LIBC=		${VERDIR}/lib/lc.lib
CSTRINGS=	${VERDIR}/lib/cstrings.lib
DEBUG=		${VERDIR}/lib/debug.lib
MYLIBS=		$(LIBC) $(CSTRINGS) $(DEBUG)
#CFLAGS=	-DDEBUG

STARTUP=	${VERDIR}/lib/Lstartup.obj

AFILES=		
CFILES=		db.c 

OFILES=		db.obj

all:	${SUBSYSNAME}.ld

.INCLUDE=$V/tools/makemeta
