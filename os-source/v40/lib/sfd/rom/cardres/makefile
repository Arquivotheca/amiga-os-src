########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	OwnCard.obj\
	ReleaseCard.obj\
	GetCardMap.obj\
	BeginCardAccess.obj\
	EndCardAccess.obj\
	ReadCardStatus.obj\
	CardResetRemove.obj\
	CardMiscControl.obj\
	CardAccessSpeed.obj\
	CardProgramVoltage.obj\
	CardResetCard.obj\
	CopyTuple.obj\
	DeviceTuple.obj\
	IfAmigaXIP.obj\
	CardForceChange.obj\
	CardChangeCount.obj\
	CardInterface.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../cardres.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
