########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	AddConfigDev.obj\
	AddBootNode.obj\
	AllocBoardMem.obj\
	AllocConfigDev.obj\
	AllocExpansionMem.obj\
	ConfigBoard.obj\
	ConfigChain.obj\
	FindConfigDev.obj\
	FreeBoardMem.obj\
	FreeConfigDev.obj\
	FreeExpansionMem.obj\
	ReadExpansionByte.obj\
	ReadExpansionRom.obj\
	RemConfigDev.obj\
	WriteExpansionByte.obj\
	ObtainConfigBinding.obj\
	ReleaseConfigBinding.obj\
	SetCurrentBinding.obj\
	GetCurrentBinding.obj\
	MakeDosNode.obj\
	AddDosNode.obj\
	ExpansionReserved26.obj\
	WriteExpansionWord.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../expansion.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
