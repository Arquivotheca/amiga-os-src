##########################################################################

MAKEFILE=	gb.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIR=		setmap
SUBSYSNAME=	gb
DISKPATH=	cli/devs/keymaps/gb

AFILES=		gb.asm
OFILES=		gb.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
