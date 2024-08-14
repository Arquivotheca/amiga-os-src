######################################################################
VERSION=	38
MAKEMETA=	/usr/commodore/amiga/V${VERSION}/tools/makemeta

MAKEFILE=	Makefile
SRCDIRPATH=	kickstart
SRCDIR=		layers
SUBSYSNAME=	layers

#DEBUG=		${LIBDIR}/debug.lib

MYLIBS=		${LIBDIR}/rom.lib ${DEBUG}

#AFLAGS= -Q
#CFLAGS=	-X71 -X72 -X73 -DNEWCLIPRECTS_1_1
# -X36, kamikaze for switch/case statements
# -X91, use slower register allocation routines
# new flags for test
#CFLAGS=	-X36 -X105 -X107 -DNEWCLIPRECTS_1_1 -DGFX_RASTPORT_1_2
CFLAGS=		-X36 -X105 -X107 -X108 -DNEWCLIPRECTS_1_1 -DGFX_RASTPORT_1_2
#CAFLAGS=	-l $*.lis
LFLAGS=		XREF=$*.xref MAP=$*.lmap ROM
CAPP=		sed -e 's/JSR	_/BSR	_/g'
APP=		spp

AFILES=		layerface.asm liblayer.asm \
		search_for_cr.asm abort.asm WhichLayer.asm \
		LockLayer.asm layertable.asm BackFill.asm

CFILES=		addobs.c behind.c copyrect.c copylayer.c deletelayer.c \
		layerop.c layerstartup.c lock.c newlayer.c refreshclean.c \
		rsplit.c screenlock.c movelayer.c screenswap.c \
		scrolllayer.c sizelayer.c update.c upfront.c \
		layers.c layersubs.c moreregions.c localclipblit.c \
		dedice.c tomiddle.c \
		layersallocmem.c cleanup.c usercliprects.c movesizelayer.c \
		fullbackfill.c rebuildcliprects.c hook.c

EXTERNIFILES=
IFILES=		layersbase.i

EXTERNHFILES=
HFILES=		layersbase.h macros.h

FIRSTO=		layerstartup.obj
OFILES=		layerface.obj layertable.obj layers.obj rsplit.obj \
		layersubs.obj newlayer.obj layerop.obj copylayer.obj \
		addobs.obj copyrect.obj screenswap.obj behind.obj upfront.obj \
		movelayer.obj sizelayer.obj refreshclean.obj \
		scrolllayer.obj deletelayer.obj update.obj \
		lock.obj screenlock.obj \
		liblayer.obj WhichLayer.obj moreregions.obj localclipblit.obj \
		dedice.obj tomiddle.obj search_for_cr.obj \
		layersallocmem.obj abort.obj cleanup.obj LockLayer.obj \
		usercliprects.obj movesizelayer.obj BackFill.obj \
		fullbackfill.obj rebuildcliprects.obj hook.obj

#all:		layers.ld doc
all:		layers.ld

.INCLUDE=${MAKEMETA}

#CC=	tmetacc
#ASM=	./cassem
