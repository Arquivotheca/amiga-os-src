########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	ReadJoyPort.obj\
	GetLanguageSelection.obj\
	SetLanguageSelection.obj\
	GetKey.obj\
	QueryKeys.obj\
	AddKBInt.obj\
	RemKBInt.obj\
	SystemControlA.obj\
	SystemControl.obj\
	AddTimerInt.obj\
	RemTimerInt.obj\
	StopTimerInt.obj\
	StartTimerInt.obj\
	ElapsedTime.obj\
	AddVBlankInt.obj\
	RemVBlankInt.obj\
	KillReq.obj\
	RestoreReq.obj\
	SetJoyPortAttrsA.obj\
	SetJoyPortAttrs.obj\
	Reserved2.obj\
	Reserved3.obj\
	Reserved4.obj\
	Reserved5.obj\
	Reserved6.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../lowlevel.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
