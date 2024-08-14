##########################################################################

MAKEFILE=	usa2.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta
SRCDIR=		setmap
SUBSYSNAME=	usa2
DISKPATH=	cli/devs/keymaps/usa2

AFILES=		usa2.asm
OFILES=		usa2.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
