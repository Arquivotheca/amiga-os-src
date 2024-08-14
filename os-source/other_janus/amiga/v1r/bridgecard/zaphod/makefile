
all:		zaphod.ld

SUBSYSNAME=	zaphod

AFILES=	blast.asm curse.asm drawtext.asm eglobal.asm header.asm \
newlines.asm putmsg.asm uline.asm ztext.asm

CFILES=\
alert.c autosize.c auxtask.c border.c color.c colordat.c crt.c \
cstask.c disptask.c fileio.c global.c imtask.c info.c init.c \
lock.c mainc.c menuhand.c menus.c misc.c open.c recalc.c \
region.c render.c scroll.c selmatic.c stuff.c super.c taskinit.c \
blast.c curse.c drawtext.c newlines.c uline.c ztext.c

OFILES=\
alert.obj autosize.obj auxtask.obj border.obj color.obj colordat.obj crt.obj \
cstask.obj disptask.obj fileio.obj global.obj imtask.obj info.obj init.obj \
lock.obj mainc.obj menuhand.obj menus.obj misc.obj open.obj recalc.obj \
region.obj render.obj scroll.obj selmatic.obj stuff.obj super.obj taskinit.obj \
blast.obj curse.obj drawtext.obj newlines.obj uline.obj ztext.obj



#MYLIBS=		${LIBDIR}/janus.lib
MAKEFILE=	Makefile
SUBSYSGROUP=	
EXTERNIFILES=
IFILES=		${EXTERNIFILES}
EXTERNHFILES=	zaphod.h
HFILES=		${EXTERNHFILES}
LFLAGS=		map=$*.lmap xref=$*.xref

STARTUP=	/usr/commodore/amiga/V33/internal/lib/startup.obj

CFLAGS=

.INCLUDE=makemeta
SYMBOLOPT=


