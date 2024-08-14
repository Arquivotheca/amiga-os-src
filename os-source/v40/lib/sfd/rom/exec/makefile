########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	Supervisor.obj\
	ExitIntr.obj\
	Schedule.obj\
	Reschedule.obj\
	Switch.obj\
	Dispatch.obj\
	Exception.obj\
	InitCode.obj\
	InitStruct.obj\
	MakeLibrary.obj\
	MakeFunctions.obj\
	FindResident.obj\
	InitResident.obj\
	Alert.obj\
	Debug.obj\
	Disable.obj\
	Enable.obj\
	Forbid.obj\
	Permit.obj\
	SetSR.obj\
	SuperState.obj\
	UserState.obj\
	SetIntVector.obj\
	AddIntServer.obj\
	RemIntServer.obj\
	Cause.obj\
	Allocate.obj\
	Deallocate.obj\
	AllocMem.obj\
	AllocAbs.obj\
	FreeMem.obj\
	AvailMem.obj\
	AllocEntry.obj\
	FreeEntry.obj\
	Insert.obj\
	AddHead.obj\
	AddTail.obj\
	Remove.obj\
	RemHead.obj\
	RemTail.obj\
	Enqueue.obj\
	FindName.obj\
	AddTask.obj\
	RemTask.obj\
	FindTask.obj\
	SetTaskPri.obj\
	SetSignal.obj\
	SetExcept.obj\
	Wait.obj\
	Signal.obj\
	AllocSignal.obj\
	FreeSignal.obj\
	AllocTrap.obj\
	FreeTrap.obj\
	AddPort.obj\
	RemPort.obj\
	PutMsg.obj\
	GetMsg.obj\
	ReplyMsg.obj\
	WaitPort.obj\
	FindPort.obj\
	AddLibrary.obj\
	RemLibrary.obj\
	OldOpenLibrary.obj\
	CloseLibrary.obj\
	SetFunction.obj\
	SumLibrary.obj\
	AddDevice.obj\
	RemDevice.obj\
	OpenDevice.obj\
	CloseDevice.obj\
	DoIO.obj\
	SendIO.obj\
	CheckIO.obj\
	WaitIO.obj\
	AbortIO.obj\
	AddResource.obj\
	RemResource.obj\
	OpenResource.obj\
	RawIOInit.obj\
	RawMayGetChar.obj\
	RawPutChar.obj\
	RawDoFmt.obj\
	GetCC.obj\
	TypeOfMem.obj\
	Procure.obj\
	Vacate.obj\
	OpenLibrary.obj\
	InitSemaphore.obj\
	ObtainSemaphore.obj\
	ReleaseSemaphore.obj\
	AttemptSemaphore.obj\
	ObtainSemaphoreList.obj\
	ReleaseSemaphoreList.obj\
	FindSemaphore.obj\
	AddSemaphore.obj\
	RemSemaphore.obj\
	SumKickData.obj\
	AddMemList.obj\
	CopyMem.obj

OBJS1=\
	CopyMemQuick.obj\
	CacheClearU.obj\
	CacheClearE.obj\
	CacheControl.obj\
	CreateIORequest.obj\
	DeleteIORequest.obj\
	CreateMsgPort.obj\
	DeleteMsgPort.obj\
	ObtainSemaphoreShared.obj\
	AllocVec.obj\
	FreeVec.obj\
	CreatePool.obj\
	DeletePool.obj\
	AllocPooled.obj\
	FreePooled.obj\
	AttemptSemaphoreShared.obj\
	ColdReboot.obj\
	StackSwap.obj\
	ChildFree.obj\
	ChildOrphan.obj\
	ChildStatus.obj\
	ChildWait.obj\
	CachePreDMA.obj\
	CachePostDMA.obj\
	AddMemHandler.obj\
	RemMemHandler.obj\
	ObtainQuickVector.obj\
	ExecReserved04.obj\
	ExecReserved05.obj\
	ExecReserved06.obj\
	TaggedOpenLibrary.obj\
	ReadGayle.obj\
	ExecReserved08.obj

lib.timestamp:	asm.timestamp
	make objs0 objs1 ${MFLAGS} ${MARGS}
	cat *.obj >../exec.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}

objs1:		${OBJS1}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
