######################################################################
# $Header: /usr.MC68010/ghostwheel/darren/V38/clipboard/RCS/Makefile,v 36.8 92/04/14 09:46:44 darren Exp $
MAKEMETA=	/usr/commodore/amiga/V38/tools/makemeta
MAKEFILE=	Makefile
SRCDIRPATH=	workbench/devs
SRCDIR=		clipboard
SUBSYSNAME=	clipboard
SUBSYSGROUP=	devices
DISKPATH=	devs/clipboard.device

#MYLIBS=		${LIBDIR}/debug.lib
LFLAGS=		map=$*.lmap

AFILES=		residenttag.asm adev.asm
CFILES=		dev.c
EXTERNIFILES=	clipboard.i
EXTERNHFILES=	clipboard.h
IFILES=		${EXTERNIFILES}
HFILES=		${EXTERNHFILES}
FIRSTO=		residenttag.obj
OFILES=		dev.obj adev.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
