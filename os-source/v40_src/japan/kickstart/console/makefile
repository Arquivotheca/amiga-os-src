##########################################################################
#
# $Id: Makefile,v 36.22 92/03/16 13:01:33 darren Exp $
#
# $Locker:  $
#
##########################################################################
MAKEMETA=	/usr/commodore/amiga/V39/tools/makemeta

MAKEFILE=	Makefile
SRCDIRPATH=	kickstart
SRCDIR=		console
SUBSYSNAME=	console
SUBSYSGROUP=	devices

#SUBMAKES=	debug.mk
#MYLIBS=		${LIBDIR}/debug.lib

LFLAGS=	ROM

AFILES=	residenttag.asm \
	dev.asm beginio.asm unsupported.asm stop.asm \
	able.asm buffer.asm clear.asm clip.asm cursor.asm \
	highlight.asm inputhandler.asm keymap.asm \
	map.asm open.asm rawconvert.asm read.asm refresh.asm \
	scroll.asm snip.asm tab.asm task.asm wreset.asm write.asm \
	endmodule.asm

CFILES=	pack.c

EXTERNIFILES=	console.i conunit.i
IFILES=		cddata.i debug.i macros.i conmap.i ${EXTERNIFILES}

EXTERNHFILES=	console.h conunit.h
HFILES=		conmap.h ${EXTERNHFILES}

FIRSTO=	residenttag.obj
OFILES=	dev.obj beginio.obj unsupported.obj stop.obj \
	able.obj buffer.obj clear.obj clip.obj cursor.obj \
	highlight.obj inputhandler.obj keymap.obj \
	map.obj open.obj pack.obj rawconvert.obj read.obj refresh.obj \
	scroll.obj snip.obj tab.obj task.obj wreset.obj write.obj \
	endmodule.obj

all:	${SUBMAKES} ${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
