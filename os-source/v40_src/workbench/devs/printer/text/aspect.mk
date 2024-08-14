##########################################################################
#
##########################################################################

MAKEFILE=	aspect.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIR=		test
SRCDIRPATH=	printer
SUBSYSNAME=	aspect
DISKPATH=	qa/pt_aspect

LFLAGS=

MYLIBS=	${VERDIR}/internal/lib/debug.lib

STARTUP=	${VERDIR}/${TERNAL}/lib/startup.obj
AFILES=
CFILES=		aspect.c pio.c cdio.c
OFILES=		aspect.obj pio.obj cdio.obj

CFLAGS=		-O

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SYMBOLOPT=
