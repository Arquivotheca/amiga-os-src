######################################################################
# $Id: Makefile,v 36.3 92/02/03 09:45:20 eric Exp $

MAKEMETA=	/usr/commodore/amiga/V${VERSION}/tools/makemeta
MAKEFILE=	Makefile
SRCDIRPATH=	workbench/libs
SRCDIR=		translator
SUBSYSNAME=	translator
DISKPATH=	libs/translator.library

LFLAGS=		map=$*.lmap

AFILES=		residenttag.asm library.asm translator.asm endtag.asm
IFILES=		ltrrules.i

FIRSTO=		residenttag.obj

OFILES=		library.obj translator.obj endtag.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
