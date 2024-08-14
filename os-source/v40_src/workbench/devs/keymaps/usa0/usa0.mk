##########################################################################

MAKEFILE=	usa0.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta.new
SRCDIR=		setmap
SUBSYSNAME=	usa0
DISKPATH=	cli/devs/keymaps/usa0

AFILES=		usa0.asm
OFILES=		usa0.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
