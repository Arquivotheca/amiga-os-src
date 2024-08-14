##########################################################################
#
# $Id: Makefile,v 37.1 91/01/21 11:10:25 mks Exp $
#
##########################################################################


#
# Amiga IEEE Floating Point Math Library
#
V=		V$(VERSION)

MAKEFILE=    Makefile
SRCDIR=		 mathieeedoubbas
SUBSYSNAME=  mathieeedoubbas
SUBSYSGROUP= libraries
SRCDIRPATH=	workbench/libs

#
# the following have defaults in the system makefile
# use these lines to add user options
#

AFLAGS=        -l $*.lis

#	fmul020.obj\



#FIRSTO=	residenttag.obj

OFILES=	residenttag.obj library.obj vec_table.obj\
	ffloorceil.obj midb.obj\
	fabs.obj fneg.obj\
	fdiv.obj\
	fcmp.obj\
	fmul.obj\
	cxnrm5dale.obj\
	cxadj5dale.obj\
	ftst.obj\
	faddsub.obj cxren5.obj cxnrm5.obj\
	float.obj fix.obj cxspc5.obj traps.obj\
	ieee68881.obj iovec_table.obj endmodule.obj


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
	cxnrm5dale.asm\
	cxadj5dale.asm\
	ftst.asm\
	faddsub.asm cxren5.asm cxnrm5.asm\
	float.asm fix.asm cxspc5.asm traps.asm\
	ieee68881.asm iovec_table.asm endmodule.asm

#
#   EXTERNIFILES are all the assembler include files released to others in
#   ${VERDIR}/include/${SUBSYSGROUP}/
#
EXTERNIFILES= mathlibrary.i mathresource.i
#
#   IFILES are all the assembler include files used by this code, except
#   any _rev.i files
#
IFILES= macros.i\
	privatemathlibrary.i float.i nulca.i \
	mathieeedoubbas_lib.i\
	$(EXTERNIFILES)

#
#   EXTERNHFILES are all the C include files released to others in
#   ${VERDIR}/include/${SUBSYSGROUP}/
#
EXTERNHFILES=	mathlibrary.h mathresource.h mathieeedp.h
#
#   HFILES are all the C include files used by this code, except any
#   _rev.h files
#
HFILES= privatemathlibrary.h\
	$(EXTERNHFILES)

# ************ manual make internal massaging!! *************
#The following files need to be moved from the include/libraries directory
#to the include/resources directory.
#mathresource.i
#mathresource.h
#Remember to do the same for the include.strip/resources directory

#
# The main dependency line for a vanilla make
#
all:	${SUBSYSNAME}.ld ${SUBSYSNAME}.doc

install:
	stripa mathieeedoubbas.ld to /usr2/a2001/libs/mathieeedoubbas.library

#
# Include the system makefile
#
.INCLUDE=/usr/commodore/amiga/$(V)/tools/makemeta
#MYLIBS=${LIBDIR}/debug.lib

APP=spp
#LINK=blink
