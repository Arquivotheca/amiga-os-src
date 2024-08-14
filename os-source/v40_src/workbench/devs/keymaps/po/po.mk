##########################################################################

MAKEFILE=	d.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIR=		setmap
SUBSYSNAME=	d
DISKPATH=	cli/devs/keymaps/d

AFILES=		d.asm
OFILES=		d.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
