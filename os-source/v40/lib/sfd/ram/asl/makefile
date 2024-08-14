########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	AllocFileRequest.obj\
	FreeFileRequest.obj\
	RequestFile.obj\
	AllocAslRequest.obj\
	AllocAslRequestTags.obj\
	FreeAslRequest.obj\
	AslRequest.obj\
	AslRequestTags.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../asl.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
