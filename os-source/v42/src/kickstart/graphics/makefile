##########################################################################
#
# $Id: makefile,v 42.1 93/07/21 14:39:14 chrisg Exp $
#
# $Locker:  $
#
##########################################################################

SUBSYSNAME=
SUBSYSTEMS=a c d gels text fonts
AFILES=pwrup.asm
INCLUDEDIR=
ASMOPTS=
CFILES=
TEMPDIR=
HOMEDIR=
HOMEINCL=
LINKEROPTS=

INFILES= copper.i display.i gels.i gfx.i gfxbase.i gfxnodes.i rpattr.i \
	monitor.i rastport.i regions.i sprite.i view.i coerce.i modeid.i \
	displayinfo.i videocontrol.i bitmap_internal.i
# text/scale.i text/text.i

OTHERFILES= \
makefile_master \
makefile \
cia8520.i \
sane_names.i \
macros.i \
displayinfo_internal.i \
cia8520.h \
sane_names.h \
macros.h \
displayinfo_internal.h \
vp_internal.i \
vp_internal.h \
graphics_lib.sfd

NONRCS_FILES=setrev

HFILES= collide.h copper.h display.h displayinfo.h gels.h gfx.h rpattr.h \
	gfxbase.h gfxmacros.h gfxnodes.h graphint.h monitor.h coerce.h modeid.h \
	rastport.h regions.h sprite.h view.h videocontrol.h bitmap_internal.h
# text/scale.h text/text.h

LINKEROPTS=	SMALLCODE SMALLDATA ADDSYM  XREF $*.xref MAP $*.lmap XOHSFL PLAIN
LINKERLIBS= v$(VERSION):lib/rom.lib v$(VERSION):lib/debug.lib

PRE_COMPILE=
PRE_INTERNAL=rev 
POST_INTERNAL=revlabel

# stupid slink complains about libcode. use blink instead
LINKER=blink
.include makefile_master
.include lib:makemeta

gfxpragmas:
	@if nexists (t:temp)
	  makedir t:temp
	@endif
	@echo >t:x1 copy graphics_lib.sfd t:temp nopro
	@echo >>t:x1 pushcd
	@echo >>t:x1 cd t:temp
	@echo >>t:x1	sfd -h graphics_lib.sfd
	@echo >>t:x1	popcd
	@echo >>t:x1 copy t:temp/fd/graphics_lib.fd graphics_lib.fd nopro
	@echo >>t:x1 copy t:temp/pragmas/graphics_pragmas.h gfxpragmas.h nopro
	@echo >>t:x1 "splat -o *"GfxBase *" *"GBASE gfx_*" gfxpragmas.h"
	@protect t:x1 +s
	@runwsh t:x1


revlabel:
	@runwsh setrev
