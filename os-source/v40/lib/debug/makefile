######################################################################
MAKEMETA=	../../tools/makemeta

SUBSYSNAME=	debug_lib

RESULTFILE=	debug.lib

AFILES= 	tty.asm kcmpstr.asm kgetnum.asm
IFILES=		assembly.i

OFILES= 	tty.obj kcmpstr.obj kgetnum.obj

all quick:	${SUBSYSNAME}.lib doc
		cp ${SUBSYSNAME}.lib ../${RESULTFILE}
		mv ${SUBSYSNAME}.doc ..

internal::
	cp ${SUBSYSNAME}.doc ${DOCDIR}

.INCLUDE=${MAKEMETA}
