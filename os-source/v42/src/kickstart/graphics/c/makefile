##########################################################################
#
# $Id: makefile,v 42.0 93/06/16 11:17:37 chrisg Exp $
#
##########################################################################

SUBSYSNAME=c
SUBSYSTEMS=
ASMOPTS=OPTIMON 0xfdc0000 ADDSYM DEFINE VBLANK_BUG=1
INFILES=
AFILES=	\
	minterms.asm\
	romgfxint.asm misc.asm ffill2asm.asm \
	coppermover.asm downellipse.asm \
	interface.asm update.asm\
	downregions.asm

CFILES=\
	blitplate.c cbump.c copper.c flood.c cpwrup.c \
	setrast.c colorconvert.c \
	ffill.c ffill2.c ffillsubrs.c g.c cdraw.c \
	makevp.c loadview.c  \
	clipline.c drawvect.c clipdraw.c  \
	areaend.c  \
	scrollraster.c sync_sbitmap.c copy_sbitmap.c waitbovp.c \
	blitplate2.c  \
	freevportcoplists.c freecprlist.c \
	scrollvp.c newgenblit.c bltrastport.c \
	bltbitmask.c clipstuff.c  \
	memory.c monitor.c vlimit.c setrpattr.c \
	modeid.c buildvp.c great.c bestid.c coerce.c

INFILES=	areafill.i cdrawinfo.i ffill.i gfxblit.i
OTHERFILES=	makefile areafill.h cdrawinfo.h ffill.h gfxblit.h gs_info.h tune.h gfxprivate.h \
			romgfxint.i

TEMPDIR=
HOMEDIR=
# defaults to Vxx/src/moduletype/system/subsysname

HOMEINCL=
LINKEROPTS=
ASM_PREPROCESSOR=spp
PROTOINC=#include *"/a/a.protos*"
PROTOINC2=#include *"romgfxint.i*"

.include /makefile_master
.include lib:makemeta

