##########################################################################

MAKEFILE=	is.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta.new
SRCDIR=		setmap
SUBSYSNAME=	is
DISKPATH=	cli/devs/keymaps/is

AFILES=		is.asm
OFILES=		is.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
