##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V38/tools/makemeta

SUBSYSNAME=	po
SRCDIRPATH=	workbench/devs/keymaps
SRCDIR=		po
DISKPATH=	Workbench/devs/keymaps/po

AFILES=		po.asm
OFILES=		po.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

strip:
		stripa from ${SUBSYSNAME}.ld to ${SUBSYSNAME}.ld.strip

