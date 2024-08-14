##########################################################################
#
##########################################################################

MAKEFILE=	dave.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIR=		test
SRCDIRPATH=	printer
SUBSYSNAME=	dave
DISKPATH=	qa/pt_dave

LFLAGS=

MYLIBS=	${VERDIR}/internal/lib/debug.lib

STARTUP=	${VERDIR}/${TERNAL}/lib/startup.obj
AFILES=
CFILES=		dave.c pio.c cdio.c
OFILES=		dave.obj pio.obj cdio.obj

CFLAGS=		-O

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SYMBOLOPT=
