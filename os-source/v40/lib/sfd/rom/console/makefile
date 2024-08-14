########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	CDInputHandler.obj\
	RawKeyConvert.obj\
	GetConSnip.obj\
	SetConSnip.obj\
	AddConSnipHook.obj\
	RemConSnipHook.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../console.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
