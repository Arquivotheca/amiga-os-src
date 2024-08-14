##############################################################
#
#	$Id: Makefile,v 36.12 91/02/25 14:54:30 bryce Exp $
#
#
SUBSYSNAME=	misc
SUBSYSGROUP=	resources
SRCDIR=		misc
SRCDIRPATH=	kickstart
MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta

#AFLAGS=	-l $*.lis
#LFLAGS=	map=$*.lmap

AFILES=		abs.asm resource.asm endcode.asm

EXTERNIFILES=	misc.i
IFILES=		asmsupp.i messages.i miscprivate.i ${EXTERNIFILES}

EXTERNHFILES=	misc.h
HFILES=		${EXTERNHFILES}

FIRSTO=		abs.obj
OFILES=		resource.obj endcode.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
