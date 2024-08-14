##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta

SUBSYSNAME=	usa0
SRCDIRPATH=	workbench/devs/keymaps
SRCDIR=		usa0
DISKPATH=	Workbench/devs/keymaps/usa0

AFILES=		usa0.asm
OFILES=		usa0.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

strip:
		mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
		stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

