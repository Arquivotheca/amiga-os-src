##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta

SUBSYSNAME=	ch2
SRCDIRPATH=	workbench/devs/keymaps
SRCDIR=		ch2
DISKPATH=	Workbench/devs/keymaps/ch2

AFILES=		ch2.asm
OFILES=		ch2.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

strip:
		mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
		stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

