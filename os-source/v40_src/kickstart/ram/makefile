######################################################################
# NATIVE BUILD ONLY
MAKEMETA=	/usr/commodore/amiga/V36/tools/makemeta

MAKEFILE=	Makefile
SRCDIRPATH=	kickstart
SRCDIR=		ram
SUBSYSNAME=	ram
SUBSYSGROUP=	libraries

# if I use dos.ld, Makemeta does annoying things
all:
		@if test ! -f ram-handler; then ( \
			echo "Can't find dos.ld!  Please make it."; \
			false; ); \
		fi;

.INCLUDE=${MAKEMETA}
SYMBOLOPT=

