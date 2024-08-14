######################################################################
MAKEMETA=	/usr/commodore/amiga/V40/tools/makemeta.sub

SRCDIRPATH=	kickstart
SRCDIR=		rawinput
SUBSYSNAME=	keyboard
MAKEFILE=	keyboard.mk

AFILES=		keydev.asm keycmds.asm keyint.asm

OFILES=		keydev.obj keycmds.obj keyint.obj

all:		${SUBSYSNAME}.lib

doc:
		rm -f ${SUBSYSNAME}.cat

.INCLUDE=${MAKEMETA}
