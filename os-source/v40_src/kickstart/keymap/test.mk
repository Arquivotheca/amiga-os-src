######################################################################
MAKEMETA=	/usr/commodore/amiga/V${VERSION}/tools/makemeta

MAKEFILE=	test.mk
SRCDIRPATH=	kickstart
SRCDIR=		keymap
SUBSYSNAME=	test

CFILES=		test.c

STARTUP=	${LIBDIR}/startup.obj
OFILES=		test.obj

all:		${SUBSYSNAME}.ld

doc:
		rm test.cat test.doc

.INCLUDE=${MAKEMETA}
