########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	LockAmigaGuideBase.obj\
	UnlockAmigaGuideBase.obj\
	OpenAmigaGuideA.obj\
	OpenAmigaGuide.obj\
	OpenAmigaGuideAsyncA.obj\
	OpenAmigaGuideAsync.obj\
	CloseAmigaGuide.obj\
	AmigaGuideSignal.obj\
	GetAmigaGuideMsg.obj\
	ReplyAmigaGuideMsg.obj\
	SetAmigaGuideContextA.obj\
	SetAmigaGuideContext.obj\
	SendAmigaGuideContextA.obj\
	SendAmigaGuideContext.obj\
	SendAmigaGuideCmdA.obj\
	SendAmigaGuideCmd.obj\
	SetAmigaGuideAttrsA.obj\
	SetAmigaGuideAttrs.obj\
	GetAmigaGuideAttr.obj\
	LoadXRef.obj\
	ExpungeXRef.obj\
	AddAmigaGuideHostA.obj\
	AddAmigaGuideHost.obj\
	RemoveAmigaGuideHostA.obj\
	RemoveAmigaGuideHost.obj\
	GetAmigaGuideString.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../amigaguide.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
