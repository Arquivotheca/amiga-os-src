########################################
MAKEMETA=	../../../../tools/makemeta

OBJS=\
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
	PingEntity.obj\
	GetEntityAttrsA.obj\
	GetEntityAttrs.obj\
	SetEntityAttrsA.obj\
	SetEntityAttrs.obj\
	AllocNIPCBuff.obj\
	AllocNIPCBuffEntry.obj\
	CopyNIPCBuff.obj\
	CopyToNIPCBuff.obj\
	CopyFromNIPCBuffer.obj\
	FreeNIPCBuff.obj\
	FreeNIPCBuffEntry.obj\
	NIPCBuffLength.obj\
	AppendNIPCBuff.obj\
	NIPCBuffPointer.obj

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
