##########################################################################
##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIRPATH=	workbench/l
SRCDIR=		speak-handler
DISKPATH=	workbench/l/speak-handler
SUBSYSNAME=	speak-handler

#MYLIBS=		${LIBDIR}/debug.lib

AFILES=		hstartup.asm
CFILES=		speak.c misc.c strfuncs.c speech.c

FIRSTO=		hstartup.obj
OFILES=		speak.obj misc.obj strfuncs.obj speech.obj

HFILES=		handler.h

all:		${SUBSYSNAME}.ld
#		rcp speak-handler.ld minnow:ram:speak-handler

.INCLUDE=${MAKEMETA}
SYMBOLOPT=
