########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	LockRealTime.obj\
	UnlockRealTime.obj\
	CreatePlayerA.obj\
	CreatePlayer.obj\
	DeletePlayer.obj\
	SetPlayerAttrsA.obj\
	SetPlayerAttrs.obj\
	SetConductorState.obj\
	ExternalSync.obj\
	NextConductor.obj\
	FindConductor.obj\
	GetPlayerAttrsA.obj\
	GetPlayerAttrs.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../realtime.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
