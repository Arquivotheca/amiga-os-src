
##########################################################################
#
# $Header: /sd0g/V39/src/kickstart/trackdisk/RCS/Makefile,v 36.5 90/06/01 23:20:25 jesup Exp $
#
# $Locker:  $
#
# $Log:	Makefile,v $
# Revision 36.5  90/06/01  23:20:25  jesup
# *** empty log message ***
# 
# Revision 36.4  89/09/06  19:50:10  jesup
# Added VERSION support
# 
# Revision 36.3  89/04/12  15:52:36  jesup
# remove trackdisk_rev.{i,h} so makeinternal will work
# 
# Revision 36.2  88/09/14  10:29:48  neil
# added that beloved V36 addition to science, SRCDIRPATH
# 
# Revision 36.1  88/09/14  10:28:34  neil
# bumped version number to 36
# 
# Revision 32.2  86/01/03  19:50:47  neil
# Added reset catching code
# 
# Revision 32.1  85/12/23  17:17:51  neil
# Added rawread/rawwrite
# 
# Revision 30.3  85/10/09  00:36:17  neil
# Another internal (for 1.01)
# 
# Revision 30.2  85/10/08  16:38:03  neil
# commented out debug.lib
# 
##########################################################################

MAKEMETA= /usr/commodore/amiga/V${VERSION}/tools/makemeta

#
# the name of the subsystem to make
#

SUBSYSNAME=	trackdisk
SRCDIR=		trackdisk
SRCDIRPATH=	kickstart

#
# the include and link lines to use
#
#AFLAGS=	-l $*.lis
LFLAGS=	map=$*.lmap xref=$*.xref
MAKEFILE=Makefile

#
# make depend and make print uses the next lines.
# They must be maintained by hand.
# I sugest something like vi's "!!ls *.asm" to generate the lines.
#
AFILES= abs.asm changed.asm driver.asm floppy.asm io.asm mfm.asm \
	resource.asm task.asm track.asm \
	endcode.asm blit.asm user.asm

CFILES=

EXTERNIFILES= trackdisk.i
IFILES=		internal.i messages.i asmsupp.i ${EXTERNIFILES}

EXTERNHFILES=	trackdisk.h
HFILES=		${EXTERNHFILES}

SUBSYSGROUP=	devices

LFILES=		${LIBDIR}/amiga.lib
#MYLIBS=	$(LIBDIR)/debug.lib

#
# OLFILES are the list of files that are linked in
#
FIRSTO= abs.obj

# endcode MUST be last
OFILES= changed.obj driver.obj floppy.obj io.obj mfm.obj \
	resource.obj task.obj track.obj blit.obj user.obj \
	endcode.obj


#
# The main dependency line: everything else follows from this
#
all: ${SUBSYSNAME}.ld

.INCLUDE=${MAKEMETA}

