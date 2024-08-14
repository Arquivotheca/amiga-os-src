##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta

SUBSYSNAME=	cdn
SRCDIRPATH=	workbench/devs/keymaps
SRCDIR=		cdn
DISKPATH=	workbench/devs/keymaps/cdn

AFILES=		cdn.asm
OFILES=		cdn.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

strip:
		mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
		stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

