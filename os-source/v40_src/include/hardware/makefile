######################################################################
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta

MAKEFILE=	Makefile
SRCDIRPATH=	include
SRCDIR=		hardware
SUBSYSGROUP=	hardware

EXTERNHFILES=	adkbits.h blit.h cia.h custom.h dmabits.h intbits.h
INTERNHFILES=
HFILES=		${EXTERNHFILES} ${INTERNHFILES}

EXTERNIFILES=	adkbits.i blit.i cia.i custom.i dmabits.i intbits.i
INTERNIFILES=	a3000_hardware.i
IFILES=		${EXTERNIFILES} ${INTERNIFILES}

all:
		@echo "(${SRCDIRPATH}/${SRCDIR}: nothing to make)"

.INCLUDE=${MAKEMETA}
