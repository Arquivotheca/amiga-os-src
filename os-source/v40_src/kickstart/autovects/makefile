######################################################################

MAKEMETA=       /usr/commodore/amiga/V${VERSION}/tools/makemeta
MAKEFILE=       Makefile
SRCDIRPATH=     kickstart
SRCDIR=         autovects
MISCSRC=	autovects.srec

all:
		if test ! -s autovects.srec; then co autovects.srec; fi

.INCLUDE=${MAKEMETA}

depend::
		rm -f 36.depend
