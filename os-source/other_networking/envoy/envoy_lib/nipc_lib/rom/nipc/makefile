########################################
MAKEMETA=	../../../../tools/makemeta

OBJS=\
	StartMonitor.obj\
	StopMonitor.obj\
	AddRoute.obj\
	DeleteRoute.obj\
	StartStats.obj\
	EndStats.obj\
	SetSvcsBase.obj\
	AllocTransactionA.obj\
	AllocTransaction.obj\
	FreeTransaction.obj\
	CreateEntityA.obj\
	CreateEntity.obj\
	DeleteEntity.obj\
	FindEntity.obj\
	LoseEntity.obj\
	DoTransaction.obj\
	BeginTransaction.obj\
	GetTransaction.obj\
	ReplyTransaction.obj\
	CheckTransaction.obj\
	AbortTransaction.obj\
	WaitTransaction.obj\
	WaitEntity.obj\
	GetEntityName.obj\
	GetHostName.obj\
	NIPCInquiryA.obj\
	NIPCInquiry.obj\
	PingEntity.obj

lib.timestamp:	asm.timestamp
	make objs ${MFLAGS} ${MARGS}
	cat *.obj >../nipc.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs:		${OBJS}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
