##########################################################################

MAKEFILE=	e.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta.new
SRCDIR=		setmap
SUBSYSNAME=	e
DISKPATH=	cli/devs/keymaps/e

AFILES=		e.asm
OFILES=		e.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
