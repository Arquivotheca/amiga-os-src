
VERSION=    39

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V39/tools/makemeta.daveb
SRCDIRPATH=	workbench/devs/printer
SRCDIR=		printer
SUBSYSNAME=	printer
SUBSYSGROUP=	devices
DISKPATH=	workbench/devs/printer.device

MYLIBS=		${LIBDIR}/debug.lib ${LIBDIR}/ddebug.lib

AFILES=		residenttag.asm beginio.asm device.asm stop.asm \
		unsupported.asm wtask.asm pwrite.asm \
		write.asm rawwrite.asm prtcommand.asm dumprport.asm \
		wait.asm rpa.asm scale.asm requester.asm endmodule.asm

CFILES=		awrite.c cdefault.c cdumprport.c copen.c cwrite.c \
		yexe.c yexr.c yrxe.c yrxr.c pixel.c error.c fixcolors.c \
		mrp.c query.c floyd.c alias.c time.c

EXTERNIFILES=	printer.i prtbase.i prtgfx.i
INTERNIFILES=	printer_rev.i

IFILES=		${EXTERNIFILES} internal.i macros.i submacs.i

EXTERNHFILES=	printer.h prtbase.h prtgfx.h
INTERNHFILES=	printer_rev.h

HFILES=		${EXTERNHFILES} internal.h timing.h

FIRSTO=		residenttag.obj

OFILES=		beginio.obj device.obj stop.obj \
		unsupported.obj wtask.obj pwrite.obj \
		write.obj rawwrite.obj prtcommand.obj dumprport.obj \
		awrite.obj cdefault.obj cdumprport.obj copen.obj cwrite.obj \
		wait.obj yexe.obj yexr.obj yrxe.obj yrxr.obj pixel.obj \
		error.obj fixcolors.obj rpa.obj time.obj scale.obj mrp.obj \
		query.obj floyd.obj alias.obj requester.obj endmodule.obj

#AFLAGS=	-l $*.lis
#CAFLAGS=	-l $*.lis
#LFLAGS=	map=$*.lmap xref=$*.xref

all:		${SUBSYSNAME}.ld
	astrip $(SUBSYSNAME).ld

.INCLUDE=${MAKEMETA}
SPECIALVERNUM=     39
