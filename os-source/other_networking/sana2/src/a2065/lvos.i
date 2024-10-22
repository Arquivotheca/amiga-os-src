	EXTERN_LIB	AddHead
	EXTERN_LIB	AddIntServer
	EXTERN_LIB	AddTail
	EXTERN_LIB	AllocMem
	EXTERN_LIB	AllocSignal
	EXTERN_LIB	Cause
	EXTERN_LIB	CloseDevice
	EXTERN_LIB	CloseLibrary
	EXTERN_LIB	CopyMem
	EXTERN_LIB	Disable
	EXTERN_LIB	DisplayAlert
	EXTERN_LIB	DoIO
	EXTERN_LIB	Enable
	EXTERN_LIB	FindConfigDev
	EXTERN_LIB	FindPort
	EXTERN_LIB	FindTask
	EXTERN_LIB	Forbid
	EXTERN_LIB	FreeMem
	EXTERN_LIB	FreeSignal
	EXTERN_LIB	OpenDevice
	EXTERN_LIB	OpenLibrary
	EXTERN_LIB	Permit
	EXTERN_LIB	ReplyMsg
	EXTERN_LIB	Remove
	EXTERN_LIB	RemHead
	EXTERN_LIB	RemIntServer
	EXTERN_LIB	SetFunction
	EXTERN_LIB	Wait

	XREF	_AbsExecBase

CALL	MACRO
	jsr	_LVO\1(a6)
	ENDM


	EXTERN_LIB	IntAllocSegment
	EXTERN_LIB	AllocSegment
	EXTERN_LIB	IntFreeSegment
	EXTERN_LIB	FreeSegment
	EXTERN_LIB	SplitNetBuff
	EXTERN_LIB	TrimNetBuff
	EXTERN_LIB	CopyToNetBuff
	EXTERN_LIB	CopyFromNetBuff
	EXTERN_LIB	CopyNetBuff
	EXTERN_LIB	CompactNetBuff
	EXTERN_LIB	ReadyNetBuff
	EXTERN_LIB	IsContiguous

