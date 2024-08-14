########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	CreateArgstring.obj\
	DeleteArgstring.obj\
	LengthArgstring.obj\
	CreateRexxMsg.obj\
	DeleteRexxMsg.obj\
	ClearRexxMsg.obj\
	FillRexxMsg.obj\
	IsRexxMsg.obj\
	LockRexxBase.obj\
	UnlockRexxBase.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../rexxsyslib.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
