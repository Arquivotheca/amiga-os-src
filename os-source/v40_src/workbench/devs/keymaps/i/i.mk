##########################################################################

MAKEFILE=	i.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIR=		setmap
SUBSYSNAME=	i
DISKPATH=	cli/devs/keymaps/i

AFILES=		i.asm
OFILES=		i.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
