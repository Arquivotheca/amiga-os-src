##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta

SUBSYSNAME=	d
SRCDIRPATH=	workbench/devs/keymaps
SRCDIR=		d
DISKPATH=	Workbench/devs/keymaps/d

AFILES=		d.asm
OFILES=		d.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

strip:
		mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
		stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

