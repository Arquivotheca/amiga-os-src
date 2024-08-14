##########################################################################

MAKEFILE=	f.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIR=		setmap
SUBSYSNAME=	f
DISKPATH=	cli/devs/keymaps/f

AFILES=		f.asm
OFILES=		f.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
