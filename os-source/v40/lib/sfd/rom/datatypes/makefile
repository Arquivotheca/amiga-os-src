########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	RLDispatch.obj\
	ObtainDataTypeA.obj\
	ObtainDataType.obj\
	ReleaseDataType.obj\
	NewDTObjectA.obj\
	NewDTObject.obj\
	DisposeDTObject.obj\
	SetDTAttrsA.obj\
	SetDTAttrs.obj\
	GetDTAttrsA.obj\
	GetDTAttrs.obj\
	AddDTObject.obj\
	RefreshDTObjectA.obj\
	RefreshDTObjects.obj\
	DoAsyncLayout.obj\
	DoDTMethodA.obj\
	DoDTMethod.obj\
	RemoveDTObject.obj\
	GetDTMethods.obj\
	GetDTTriggerMethods.obj\
	PrintDTObjectA.obj\
	PrintDTObject.obj\
	ObtainDTDrawInfoA.obj\
	ObtainDTDrawInfo.obj\
	DrawDTObjectA.obj\
	DrawDTObject.obj\
	ReleaseDTDrawInfo.obj\
	GetDTString.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../datatypes.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
