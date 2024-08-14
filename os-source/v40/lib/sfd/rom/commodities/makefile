########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	CreateCxObj.obj\
	CxBroker.obj\
	ActivateCxObj.obj\
	DeleteCxObj.obj\
	DeleteCxObjAll.obj\
	CxObjType.obj\
	CxObjError.obj\
	ClearCxObjError.obj\
	SetCxObjPri.obj\
	AttachCxObj.obj\
	EnqueueCxObj.obj\
	InsertCxObj.obj\
	RemoveCxObj.obj\
	CxReserved0.obj\
	SetTranslate.obj\
	SetFilter.obj\
	SetFilterIX.obj\
	ParseIX.obj\
	CxMsgType.obj\
	CxMsgData.obj\
	CxMsgID.obj\
	DivertCxMsg.obj\
	RouteCxMsg.obj\
	DisposeCxMsg.obj\
	InvertKeyMap.obj\
	AddIEvents.obj\
	CopyBrokerList.obj\
	FreeBrokerList.obj\
	BrokerCommand.obj\
	MatchIX.obj\
	CxReserved1.obj\
	CxReserved2.obj\
	CxReserved3.obj\
	CxReserved4.obj\
	CxReserved5.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../commodities.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
