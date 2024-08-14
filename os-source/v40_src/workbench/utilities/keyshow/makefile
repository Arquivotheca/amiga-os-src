##########################################################################
#
# $Id: Makefile,v 1.4 91/01/16 12:00:48 eric Exp $
#
# $Locker:  $
#
# $Log:	Makefile,v $
# Revision 1.4  91/01/16  12:00:48  eric
# Changed version from V36 to V37
# 
# Revision 1.3  90/11/08  13:55:51  eric
# Added revision.c|o and changed startup to Qstartup.obj
# 
# Revision 1.2  90/11/07  15:58:57  eric
# Changed startup to Astartup.
# 
# Revision 1.1  90/04/19  15:23:09  eric
# Initial revision
# 
#
##########################################################################

MAKEFILE=	Makefile
MAKEMETA=	/usr/commodore/amiga/V37/tools/makemeta

SUBSYSNAME=	keyshow
SRCDIR=		${SUBSYSNAME}
SRCDIRPATH=	workbench/utilities
DISKPATH=	workbench/utilities/${SUBSYSNAME}

#MYLIBS=         ${LIBDIR}/debug.lib
STARTUP=	${LIBDIR}/Qstartup.obj
CFILES=		revision.c main.c init.c drawboard.c images.c
OFILES=		revision.obj main.obj init.obj drawboard.obj images.obj

HFILES=		keytoy.h keyboard.h

all:		${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}
SYMBOLOPT=

strip:
	mv ${SUBSYSNAME}.ld ${SUBSYSNAME}.nostrip
	stripa from ${SUBSYSNAME}.nostrip to ${SUBSYSNAME}.ld
