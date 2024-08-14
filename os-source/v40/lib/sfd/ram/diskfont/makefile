########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	OpenDiskFont.obj\
	AvailFonts.obj\
	NewFontContents.obj\
	DisposeFontContents.obj\
	NewScaledDiskFont.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../diskfont.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
