########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	AllocIFF.obj\
	OpenIFF.obj\
	ParseIFF.obj\
	CloseIFF.obj\
	FreeIFF.obj\
	ReadChunkBytes.obj\
	WriteChunkBytes.obj\
	ReadChunkRecords.obj\
	WriteChunkRecords.obj\
	PushChunk.obj\
	PopChunk.obj\
	EntryHandler.obj\
	ExitHandler.obj\
	PropChunk.obj\
	PropChunks.obj\
	StopChunk.obj\
	StopChunks.obj\
	CollectionChunk.obj\
	CollectionChunks.obj\
	StopOnExit.obj\
	FindProp.obj\
	FindCollection.obj\
	FindPropContext.obj\
	CurrentChunk.obj\
	ParentChunk.obj\
	AllocLocalItem.obj\
	LocalItemData.obj\
	SetLocalItemPurge.obj\
	FreeLocalItem.obj\
	FindLocalItem.obj\
	StoreLocalItem.obj\
	StoreItemInContext.obj\
	InitIFF.obj\
	InitIFFasDOS.obj\
	InitIFFasClip.obj\
	OpenClipboard.obj\
	CloseClipboard.obj\
	GoodID.obj\
	GoodType.obj\
	IDtoStr.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../iffparse.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
