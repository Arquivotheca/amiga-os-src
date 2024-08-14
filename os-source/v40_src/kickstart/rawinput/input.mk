######################################################################
MAKEMETA=	/usr/commodore/amiga/V40/tools/makemeta.sub

SRCDIRPATH=	kickstart
SRCDIR=		rawinput
SUBSYSNAME=	input
MAKEFILE=	input.mk

AFILES=		inputdev.asm inputcmds.asm inputtask.asm

OFILES=		inputdev.obj inputcmds.obj inputtask.obj

all:		${SUBSYSNAME}.lib

doc:
		rm -f ${SUBSYSNAME}.cat

.INCLUDE=${MAKEMETA}
