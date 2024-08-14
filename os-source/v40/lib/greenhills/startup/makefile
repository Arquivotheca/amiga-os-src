VERSION=	35
V=		V35

MAKEFILE=	Makefile
SUBSYSNAME=	startup
LIBNAME=	/
SRCDIR=		startup

AFILES=		gh_startup.asm
OFILES=		gh_startup.obj

#AFLAGS=	-l $*.lis

all quick:	${OFILES}

asm:
		@echo "startup does not have auto generated .asm files"

install:
	cp *.obj ..

.INCLUDE=/usr/commodore/amiga/$(V)/tools/makemeta
