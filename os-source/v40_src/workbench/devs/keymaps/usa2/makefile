##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta

SUBSYSNAME=	usa2
SRCDIRPATH=	workbench/devs/keymaps
SRCDIR=		usa2
DISKPATH=	workbench/devs/keymaps/usa2

AFILES=		usa2.asm
OFILES=		usa2.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

strip:
		mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
		stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

