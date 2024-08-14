########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	GetCopyNV.obj\
	FreeNVData.obj\
	StoreNV.obj\
	DeleteNV.obj\
	GetNVInfo.obj\
	GetNVList.obj\
	SetNVProtection.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../nonvolatile.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
