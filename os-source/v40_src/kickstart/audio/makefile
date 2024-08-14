#
# $Id: Makefile,v 36.3 91/02/27 21:00:54 mks Exp $
#
# $Log:	Makefile,v $
# Revision 36.3  91/02/27  21:00:54  mks
# Needed to use debug.lib for a while
# 
# Revision 36.2  90/08/29  17:48:53  mks
# Changed revision to 36...  Changed ownership to MKS...
#
######################################################################
MAKEMETA=	/usr/commodore/amiga/V${VERSION}/tools/makemeta

MAKEFILE=	Makefile
SRCDIRPATH=	kickstart
SRCDIR=		audio
SUBSYSNAME=	audio
SUBSYSGROUP=	devices
DISKPATH=	devs/audio.device
MISCSRC=	script cppapp

AFLAGS=		-c Q,C,S
CFLAGS=		-X36 -X108
LFLAGS=		map=$*.lmap ROM
#MYLIBS=		/usr/commodore/amiga/V/lib/debug.lib

AFILES=		residenttag.asm \
		device.asm commands.asm interrupt.asm sysbase.asm \
		endmodule.asm
CFILES=		cdevice.c cbeginio.c

EXTERNIFILES=	audio.i
IFILES=		macros.i device.i \
		${EXTERNIFILES}

EXTERNHFILES=	audio.h
HFILES=		device.h \
		${EXTERNHFILES}

FIRSTO=		residenttag.obj
OFILES=		device.obj interrupt.obj sysbase.obj \
		cdevice.obj cbeginio.obj \
		endmodule.obj

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

CAPP=		cppapp
