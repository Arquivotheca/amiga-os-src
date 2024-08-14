##########################################################################

MAKEFILE=	ch2.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta.new
SRCDIR=		setmap
SUBSYSNAME=	ch2
DISKPATH=	cli/devs/keymaps/ch1

AFILES=		ch2.asm
OFILES=		ch2.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
