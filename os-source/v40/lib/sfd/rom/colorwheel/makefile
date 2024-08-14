########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	ConvertHSBToRGB.obj\
	ConvertRGBToHSB.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../colorwheel.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
