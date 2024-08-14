##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta

SUBSYSNAME=	ch1
SRCDIRPATH=	workbench/devs/keymaps
SRCDIR=		ch1
DISKPATH=	Workbench/devs/keymaps/ch1

AFILES=		ch1.asm
OFILES=		ch1.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

strip:
		mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
		stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

