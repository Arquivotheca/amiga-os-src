########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	OpenEngine.obj\
	CloseEngine.obj\
	SetInfoA.obj\
	SetInfo.obj\
	ObtainInfoA.obj\
	ObtainInfo.obj\
	ReleaseInfoA.obj\
	ReleaseInfo.obj\
	GetGlyphMap.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../bullet.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
