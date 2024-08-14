######################################################################
MAKEMETA=	/usr/commodore/amiga/V40/tools/makemeta

SRCDIRPATH=	kickstart
SRCDIR=		keymap
SUBSYSNAME=	keymap
SUBSYSGROUP=	devices
MAKEFILE=	Makefile
MISCSRC=	mapansi.c.doc
#SUBMAKES=	test.mk

LFLAGS=		map=$*.lmap ROM

EXTERNIFILES=	keymap.i
IFILES=		kldata.i ${EXTERNIFILES}
EXTERNHFILES=	keymap.h
HFILES=		${EXTERNHFILES}

AFILES=		residenttag.asm library.asm defaultmap.asm \
		maprawkey.asm mapansi.asm usamap.asm endtag.asm

FIRSTO=		residenttag.obj
OFILES=		library.obj defaultmap.obj \
		maprawkey.obj mapansi.obj usamap.obj endtag.obj

all:		${SUBSYSNAME}.ld

doc:
		rm -f ${SUBSYSNAME}.cat

.INCLUDE=${MAKEMETA}
