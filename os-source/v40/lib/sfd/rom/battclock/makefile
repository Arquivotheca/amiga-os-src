########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	ResetBattClock.obj\
	ReadBattClock.obj\
	WriteBattClock.obj\
	ReadBattClockMem.obj\
	WriteBattClockMem.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../battclock.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
