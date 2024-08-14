########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	ObtainBattSemaphore.obj\
	ReleaseBattSemaphore.obj\
	ReadBattMem.obj\
	WriteBattMem.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../battmem.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
