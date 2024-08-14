##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta

SUBSYSNAME=	usa1
SRCDIRPATH=	workbench/devs/keymaps
SRCDIR=		usa1
DISKPATH=	Workbench/devs/keymaps/usa1

AFILES=		usa1.asm
OFILES=		usa1.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

strip:
		mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
		stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

