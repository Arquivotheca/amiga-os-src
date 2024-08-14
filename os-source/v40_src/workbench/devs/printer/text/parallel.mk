##########################################################################
#
##########################################################################

MAKEFILE=	parallel.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIR=		test
SRCDIRPATH=	printer
SUBSYSNAME=	parallel
DISKPATH=	qa/pt_parallel

MYLIBS=		${LIBDIR}/debug.lib
STARTUP=	${LIBDIR}/startup.obj
CFILES=		parallel.c
OFILES=		parallel.obj

CFLAGS=		-O

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SYMBOLOPT=
