##########################################################################
#
##########################################################################

MAKEFILE=	cancel.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIR=		test
SRCDIRPATH=	printer
SUBSYSNAME=	cancel
DISKPATH=	qa/pt_cancel

LFLAGS=

MYLIBS=	${VERDIR}/internal/lib/debug.lib

STARTUP=	${VERDIR}/${TERNAL}/lib/startup.obj
AFILES=
CFILES=		cancel.c pio.c
OFILES=		cancel.obj pio.obj

CFLAGS=		-O

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SYMBOLOPT=
