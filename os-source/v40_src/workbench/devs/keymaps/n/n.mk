##########################################################################

MAKEFILE=	n.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta.new
SRCDIR=		setmap
SUBSYSNAME=	n
DISKPATH=	cli/devs/keymaps/n

AFILES=		n.asm
OFILES=		n.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
