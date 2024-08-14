##########################################################################
#
# $Id: Makefile,v 36.4 90/05/25 13:57:14 rsbx Exp $
#
#
##########################################################################


#
# the name of the subsystem to make
#

SUBSYSGROUP=	resources
SUBSYSNAME=	battmem
SRCDIR=		battmem
SRCDIRPATH=	kickstart

#
# the include and link lines to use
#

LFLAGS=		map=$*.lmap
MAKEFILE=	Makefile

#
# make depend and make print uses the next lines.
# They must be maintained by hand.
# I sugest something like vi's "!!ls *.asm" to generate the lines.
#
AFILES= resource.asm endcode.asm

CFILES=

EXTERNIFILES=	battmem.i battmembitsamiga.i battmembitsamix.i battmembitsshared.i
IFILES=		asmsupp.i internal.i ${EXTERNIFILES}

EXTERNHFILES=	battmem.h battmembitsamiga.h battmembitsamix.h battmembitsshared.h
HFILES=		${EXTERNHFILES}

#
# OLFILES are the list of files that are linked in
#
FIRSTO= resource.obj
OFILES= endcode.obj


#
# The main dependency line: everything else follows from this
#
all: ${SUBSYSNAME}.ld ${SUBSYSNAME}.doc

.INCLUDE=/usr/commodore/amiga/V$(VERSION)/tools/makemeta
