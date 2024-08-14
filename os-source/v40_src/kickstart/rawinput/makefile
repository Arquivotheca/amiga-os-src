######################################################################
MAKEMETA=	/usr/commodore/amiga/V40/tools/makemeta

SRCDIRPATH=	kickstart
SRCDIR=		rawinput
SUBSYSNAME=	rawinput
SUBSYSGROUP=	devices
MAKEFILE=	Makefile
SUBMAKES=	keyboard.mk gameport.mk input.mk

#
# the following have defaults in the system makefile
# use these lines to add user options
#
LFLAGS=		map=$*.lmap

MYLIBS=		keyboard.lib gameport.lib input.lib

AFILES=		residenttags.asm \
		beginio.asm dev.asm stop.asm unsupported.asm msgport.asm \
		endmodule.asm

EXTERNIFILES=	keyboard.i gameport.i input.i inputevent.i
IFILES=		stddevice.i kbdata.i gpdata.i iddata.i macros.i \
		${EXTERNIFILES}

EXTERNHFILES=	keyboard.h gameport.h input.h inputevent.h
HFILES=		${EXTERNHFILES}

FIRSTO=		residenttags.obj
OFILES=		beginio.obj dev.obj stop.obj unsupported.obj msgport.obj \
		endmodule.obj

all:		${SUBMAKES} ${SUBSYSNAME}.ld 

rawinput_rev.i: rawinput_rev.rev
		${BUMPREV} ${VERNUM} rawinput_rev
		rm -f rawinput_rev.h
		sed "s/rawinput //" rawinput_rev.i > rawinput_rev.i.TMP
		mv rawinput_rev.i.TMP rawinput_rev.i

doc:
		rm -f ${SUBSYSNAME}.cat ${SUBSYSNAME}.doc

.INCLUDE=${MAKEMETA}
