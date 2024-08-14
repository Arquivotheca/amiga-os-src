##########################################################################
#
# $Id: Makefile,v 37.1 91/05/10 15:13:52 eric Exp $
#
# $Locker:  $
#
# $Log:	Makefile,v $
# Revision 37.1  91/05/10  15:13:52  eric
# Command renamed to Lacer.
# 
# Revision 37.0  91/04/27  13:35:37  eric
# added to rcs
# 
#
##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V/tools/makemeta

SRCDIR=		lacer
SUBSYSNAME=	lacer
DISKPATH=

#STARTUP=	${LIBDIR}/startup.obj
#MYLIBS=	${LIBDIR}/debug.lib

AFILES=		lacer.asm
OFILES=		lacer.obj

HFILES=

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SYMBOLOPT=

strip:
		mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
		stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld

LINK=		blink
