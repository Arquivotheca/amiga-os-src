##########################################################################
#
##########################################################################

MAKEFILE=	serial.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIR=		test
SRCDIRPATH=	printer
SUBSYSNAME=	serial
DISKPATH=	qa/pt_serial

LFLAGS=

MYLIBS=	${VERDIR}/internal/lib/debug.lib

STARTUP=	${VERDIR}/${TERNAL}/lib/startup.obj
AFILES=
CFILES=		serial.c
OFILES=		serial.obj

CFLAGS=		-O

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SYMBOLOPT=
