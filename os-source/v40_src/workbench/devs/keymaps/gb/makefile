##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta

SUBSYSNAME=	gb
SRCDIRPATH=	workbench/devs/keymaps
SRCDIR=		gb
DISKPATH=	Workbench/devs/keymaps/gb

AFILES=		gb.asm
OFILES=		gb.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

strip:
		mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
		stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

