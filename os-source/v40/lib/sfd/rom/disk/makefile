########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	AllocUnit.obj\
	FreeUnit.obj\
	GetUnit.obj\
	GiveUnit.obj\
	GetUnitID.obj\
	ReadUnitID.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../disk.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
