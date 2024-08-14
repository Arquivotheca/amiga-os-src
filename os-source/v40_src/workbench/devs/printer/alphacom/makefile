
VERSION=	36	# use V36 includes, libraries, etc.

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		alphacom
SUBSYSNAME=	alphacom
DISKPATH=	workbench/devs/printers/Alphacom_Alphapro_101

AFILES=		printertag.asm init.asm
CFILES=		data.c dospecial.c render.c
FIRSTO=		printertag.obj
OFILES=		init.obj data.obj dospecial.obj render.obj

all:		${SUBSYSNAME}.ld
	astrip ${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=	35	# what gets sent to bumprev
