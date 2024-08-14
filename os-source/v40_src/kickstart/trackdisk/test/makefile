
##########################################################################
#
# $Header: Makefile,v 30.3 85/10/09 00:36:17 neil Exp $
#
# $Locker: neil $
#
# $Log:	Makefile,v $
# Revision 30.3  85/10/09  00:36:17  neil
# Another internal (for 1.01)
# 
# Revision 30.2  85/10/08  16:38:03  neil
# commented out debug.lib
# 
# Revision 30.1  85/08/29  12:55:26  neil
# V1 upgrade
# 
# Revision 29.2  85/08/08  16:23:30  neil
# upgrade to V29
# 
# Revision 29.1  85/08/02  11:28:54  neil
# Upgrade to V29
# 
# Revision 28.2  85/07/10  01:10:40  neil
# address change for V28
# 
# Revision 28.1  85/07/02  22:39:39  neil
# updated to V28
# 
# Revision 27.1  85/06/24  13:38:49  neil
# Upgrade to V27
# 
#
##########################################################################


#
# the name of the subsystem to make
#

SUBSYSNAME=	tracktest
SRCDIR=		tracktest

#
# the include and link lines to use
#
#AFLAGS=	-l $*.lis
LFLAGS=	map=$*.lmap
MAKEFILE=Makefile

#
# make depend and make print uses the next lines.
# They must be maintained by hand.
# I sugest something like vi's "!!ls *.asm" to generate the lines.
#
STARTUP=	$(LIBDIR)/startup.obj
AFILES= test.asm

CFILES= fsup.c track.c fixup.c

EXTERNIFILES=
IFILES=		${EXTERNIFILES}

EXTERNHFILES=
HFILES=		${EXTERNHFILES}

SUBSYSGROUP=	devices

LFILES=		${LIBDIR}/amiga.lib
MYLIBS=	$(LIBDIR)/debug.lib

OFILES= fsup.obj test.obj track.obj fixup.obj


#
# The main dependency line: everything else follows from this
#
all: ${SUBSYSNAME}.ld

.INCLUDE=/usr/commodore/amiga/V/tools/makemeta
