##########################################################################

MAKEFILE=	ch1.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta.new
SRCDIR=		setmap
SUBSYSNAME=	ch1
DISKPATH=	cli/devs/keymaps/ch1

AFILES=		ch1.asm
OFILES=		ch1.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
