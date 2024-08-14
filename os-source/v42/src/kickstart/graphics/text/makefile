##########################################################################
#
# $Id: makefile,v 42.0 93/06/16 11:25:31 chrisg Exp $
#
##########################################################################

SYSTEM=graphics
SUBSYSNAME=text
MODULETYPE=kickstart
SUBSYSTEMS=
AFILES=bitscale.asm bltcopy.asm clear.asm font.asm \
		length.asm write.asm
VERSION=42
INCLUDEDIR=include:
ASMOPTS=OPTIMON 0xfdc00000 ADDSYM
OTHERFILES=makefile
INFILES=text.i scale.i txtdata.i macros.i
HFILES=text.h scale.h
CFILES=
LINKEROPTS=
TEMPDIR=

.include /makefile_master
.include lib:makemeta

