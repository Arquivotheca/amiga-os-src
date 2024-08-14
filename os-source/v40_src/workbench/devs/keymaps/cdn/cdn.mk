##########################################################################

MAKEFILE=	cdn.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta.new
SRCDIR=		setmap
SUBSYSNAME=	cdn
DISKPATH=	cli/devs/keymaps/cdn

AFILES=		cdn.asm
OFILES=		cdn.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
