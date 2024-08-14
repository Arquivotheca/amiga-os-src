
##########################################################################
#
# $Id: Makefile,v 36.7 90/07/13 15:01:28 jesup Exp $
#
# $Locker:  $
#
# $Log:	Makefile,v $
# Revision 36.7  90/07/13  15:01:28  jesup
# id
# 
# Revision 36.6  89/09/06  20:48:53  bryce
# *** empty log message ***
# 
# Revision 36.5  89/04/12  15:37:12  jesup
# removed disk_rev.{i,h}
# so makeinternal will work
# 
# Revision 36.4  89/03/08  20:23:38  jesup
# Changed V$(VERSION) to V36, otherwise it couldn't find makemeta
# 
# Revision 36.3  89/02/17  23:03:25  jesup
# removed endcode.asm
# 
# Revision 36.2  88/09/19  15:23:54  neil
# added our favorite: SRCDIRPATH
# 
# Revision 36.1  88/08/19  10:24:48  dale
# added $(VERSION) to allow multiple versions to work
# 
# Revision 32.1  86/01/03  23:03:25  neil
# include file path name made generic
# 
##########################################################################


#
# the name of the subsystem to make
#
V=	V36

SUBSYSNAME=	disk
SRCDIR=		disk
SRCDIRPATH=	kickstart

#
# the include and link lines to use
#
# AFLAGS=	-l $*.lis
# LFLAGS=	map=$*.lmap
MAKEFILE=Makefile

#
# make depend and make print uses the next lines.
# They must be maintained by hand.
# I sugest something like vi's "!!ls *.asm" to generate the lines.
#
AFILES= abs.asm resource.asm

CFILES=

EXTERNIFILES=	disk.i
IFILES=		asmsupp.i messages.i ${EXTERNIFILES}

EXTERNHFILES=	disk.h
HFILES=		${EXTERNHFILES}

SUBSYSGROUP=	resources

LFILES=		${VERDIR}/${TERNAL}/lib/amiga.lib


#
# OLFILES are the list of files that are linked in
#
FIRSTO= abs.obj
OFILES= resource.obj


#
# The main dependency line: everything else follows from this
#
all: ${SUBSYSNAME}.ld

.INCLUDE=/usr/commodore/amiga/$(V)/tools/makemeta
