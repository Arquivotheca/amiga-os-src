##########################################################################

MAKEFILE=	dk.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta.new
SRCDIR=		setmap
SUBSYSNAME=	dk
DISKPATH=	cli/devs/keymaps/dk

AFILES=		dk.asm
OFILES=		dk.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
