##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V38/tools/makemeta

SUBSYSNAME=	f
SRCDIRPATH=	workbench/devs/keymaps
SRCDIR=		f
DISKPATH=	workbench/devs/keymaps/f

AFILES=		f.asm
OFILES=		f.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

strip:
		mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
		stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

