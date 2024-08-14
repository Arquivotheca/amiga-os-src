##########################################################################
#
# $Id: Makefile,v 37.1 91/01/13 21:22:11 mks Exp $
#
# $Log:	Makefile,v $
# Revision 37.1  91/01/13  21:22:11  mks
# Fixed to use version from command line
# 
# Revision 1.12  90/08/30  15:51:40  mks
# Needed checkin for make internal...
#
##########################################################################


#
# Amiga IEEE singbas Math Library
#
V=		V$(VERSION)

MAKEFILE=    Makefile
SRCDIR=		 mathieeesingbas.3000
SUBSYSNAME=  mathieeesingbas
SUBSYSGROUP= libraries
SRCDIRPATH=	kickstart

#
# the following have defaults in the system makefile
# use these lines to add user options
#

#AFLAGS=        -l $*.lis

#	fmul020.obj


FIRSTO=	residenttag.obj

OFILES=	library.obj vec_table.obj\
	ffloorceil.obj\
	fabs.obj fneg.obj\
	fcmp.obj\
	ftst.obj\
	ieee68881.obj endmodule.obj


#
# make depend and make print uses the next lines.
# They are maintained by hand.
#
#   AFILES are all the assembler source files
#
AFILES=	residenttag.asm library.asm vec_table.asm\
	ffloorceil.asm midb.asm\
	fabs.asm fneg.asm\
	fdiv.asm\
	fcmp.asm\
	fmul.asm\
	fmul020.asm\
	fdiv020.asm\
	ftst.asm\
	faddsub.asm cxren4.asm cxnrm4dale.asm cxadj4.asm\
	float.asm fix.asm cxspc4.asm traps.asm\
	ieee68881.asm iovec_table.asm endmodule.asm

#
#   EXTERNIFILES are all the assembler include files released to others in
#   ${VERDIR}/include/${SUBSYSGROUP}/
#
EXTERNIFILES=
#
#   IFILES are all the assembler include files used by this code, except
#   any _rev.i files
#
IFILES= macros.i float.i\
	nulca.i mathresource.i mathlibrary.i\
	privatemathlibrary.i

#
#   EXTERNHFILES are all the C include files released to others in
#   ${VERDIR}/include/${SUBSYSGROUP}/
#
EXTERNHFILES=
#
#   HFILES are all the C include files used by this code, except any
#   _rev.h files
#
HFILES=


#
# The main dependency line for a vanilla make
#
all:	${SUBSYSNAME}.ld ${SUBSYSNAME}.doc
#all:	${SUBSYSNAME}.ld

install:
	stripa mathieeesingbas.ld to /usr2/a2001/libs/mathieeesingbas.library

#
# Include the system makefile
#
.INCLUDE=/usr/commodore/amiga/$(V)/tools/makemeta
#MYLIBS=${LIBDIR}/debug.lib

MYLIBS=	${VERDIR}/lib/rom.lib

APP=spp
#LINK=blink
