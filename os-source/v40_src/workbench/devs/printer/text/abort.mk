##########################################################################
#
##########################################################################

MAKEFILE=	abort.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIR=		test
SRCDIRPATH=	printer
SUBSYSNAME=	abort
DISKPATH=	qa/pt_abort

LFLAGS=

MYLIBS=	${VERDIR}/internal/lib/debug.lib

STARTUP=	${VERDIR}/${TERNAL}/lib/startup.obj
AFILES=
CFILES=		abort.c
OFILES=		abort.obj

CFLAGS=		-O

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SYMBOLOPT=
