######################################################################
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta

MAKEFILE=	Makefile
SRCDIRPATH=	kickstart
SRCDIR=		potgo
SUBSYSNAME=	potgo
SUBSYSGROUP=	resources

AFILES=		potgo.asm

EXTERNIFILES=	potgo.i
IFILES=		${EXTERNIFILES}

EXTERNHFILES=	potgo.h
HFILES=		${EXTERNHFILES}

FIRSTO=		potgo.obj

all:		${SUBSYSNAME}.ld

doc:
		rm -f ${SUBSYSNAME}.cat

.INCLUDE=${MAKEMETA}
