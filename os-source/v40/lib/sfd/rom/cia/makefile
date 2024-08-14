########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	AddICRVector.obj\
	RemICRVector.obj\
	AbleICR.obj\
	SetICR.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../cia.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
