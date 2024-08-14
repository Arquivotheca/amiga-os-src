##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta

SUBSYSNAME=	dk
SRCDIRPATH=	workbench/devs/keymaps
SRCDIR=		dk
DISKPATH=	Workbench/devs/keymaps/dk

AFILES=		dk.asm
OFILES=		dk.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

strip:
		mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
		stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

