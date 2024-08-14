########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	ReadJoyPort.obj\
	GetLanguageSelection.obj\
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
	SetJoyPortAttrsA.obj\
	SetJoyPortAttrs.obj

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
