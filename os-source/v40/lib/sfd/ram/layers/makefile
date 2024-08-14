########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	InitLayers.obj\
	CreateUpfrontLayer.obj\
	CreateBehindLayer.obj\
	UpfrontLayer.obj\
	BehindLayer.obj\
	MoveLayer.obj\
	SizeLayer.obj\
	ScrollLayer.obj\
	BeginUpdate.obj\
	EndUpdate.obj\
	DeleteLayer.obj\
	LockLayer.obj\
	UnlockLayer.obj\
	LockLayers.obj\
	UnlockLayers.obj\
	LockLayerInfo.obj\
	SwapBitsRastPortClipRect.obj\
	WhichLayer.obj\
	UnlockLayerInfo.obj\
	NewLayerInfo.obj\
	DisposeLayerInfo.obj\
	FattenLayerInfo.obj\
	ThinLayerInfo.obj\
	MoveLayerInFrontOf.obj\
	InstallClipRegion.obj\
	MoveSizeLayer.obj\
	CreateUpfrontHookLayer.obj\
	CreateBehindHookLayer.obj\
	InstallLayerHook.obj\
	InstallLayerInfoHook.obj\
	SortLayerCR.obj\
	DoHookClipRects.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../layers.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
