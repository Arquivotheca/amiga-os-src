######################################################################
MAKEMETA=	/usr/commodore/amiga/V40/tools/makemeta.sub

SRCDIRPATH=	kickstart
SRCDIR=		rawinput
SUBSYSNAME=	gameport
MAKEFILE=	gameport.mk

AFILES=		gamedev.asm gamecmds.asm gameint.asm

OFILES=		gamedev.obj gamecmds.obj gameint.obj

all:		${SUBSYSNAME}.lib

doc:
		rm -f ${SUBSYSNAME}.cat

.INCLUDE=${MAKEMETA}
