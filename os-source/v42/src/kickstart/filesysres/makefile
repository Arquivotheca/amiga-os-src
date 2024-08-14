######################################################################
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta

MAKEFILE=	Makefile
SRCDIRPATH=	kickstart
SRCDIR=		filesysres
SUBSYSNAME=	filesysres
SUBSYSGROUP=	resources

AFILES=		filesysres.asm

EXTERNIFILES=	filesysres.i
IFILES=		${EXTERNIFILES}

EXTERNHFILES=	filesysres.h
HFILES=		${EXTERNHFILES}

FIRSTO=		filesysres.obj

all:		${SUBSYSNAME}.ld

doc:
		rm -f ${SUBSYSNAME}.cat

.INCLUDE=${MAKEMETA}
