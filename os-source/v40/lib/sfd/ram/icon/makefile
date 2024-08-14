########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	FreeFreeList.obj\
	AddFreeList.obj\
	GetDiskObject.obj\
	PutDiskObject.obj\
	FreeDiskObject.obj\
	FindToolType.obj\
	MatchToolValue.obj\
	BumpRevision.obj\
	GetDefDiskObject.obj\
	PutDefDiskObject.obj\
	GetDiskObjectNew.obj\
	DeleteDiskObject.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../icon.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
