##########################################################################

MAKEFILE=	s.mk
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta.new
SRCDIR=		setmap
SUBSYSNAME=	s
DISKPATH=	cli/devs/keymaps/s

AFILES=		s.asm
OFILES=		s.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
