########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	FindTagItem.obj\
	GetTagData.obj\
	PackBoolTags.obj\
	NextTagItem.obj\
	FilterTagChanges.obj\
	MapTags.obj\
	AllocateTagItems.obj\
	CloneTagItems.obj\
	FreeTagItems.obj\
	RefreshTagItemClones.obj\
	TagInArray.obj\
	FilterTagItems.obj\
	CallHookPkt.obj\
	Amiga2Date.obj\
	Date2Amiga.obj\
	CheckDate.obj\
	SMult32.obj\
	UMult32.obj\
	SDivMod32.obj\
	UDivMod32.obj\
	Stricmp.obj\
	Strnicmp.obj\
	ToUpper.obj\
	ToLower.obj\
	ApplyTagChanges.obj\
	SMult64.obj\
	UMult64.obj\
	PackStructureTags.obj\
	UnpackStructureTags.obj\
	AddNamedObject.obj\
	AllocNamedObjectA.obj\
	AllocNamedObject.obj\
	AttemptRemNamedObject.obj\
	FindNamedObject.obj\
	FreeNamedObject.obj\
	NamedObjectName.obj\
	ReleaseNamedObject.obj\
	RemNamedObject.obj\
	GetUniqueID.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../utility.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
