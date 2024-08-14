########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	AddTime.obj\
	SubTime.obj\
	CmpTime.obj\
	ReadEClock.obj\
	GetSysTime.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../timer.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
