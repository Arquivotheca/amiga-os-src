########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	IEEESPAtan.obj\
	IEEESPSin.obj\
	IEEESPCos.obj\
	IEEESPTan.obj\
	IEEESPSincos.obj\
	IEEESPSinh.obj\
	IEEESPCosh.obj\
	IEEESPTanh.obj\
	IEEESPExp.obj\
	IEEESPLog.obj\
	IEEESPPow.obj\
	IEEESPSqrt.obj\
	IEEESPTieee.obj\
	IEEESPFieee.obj\
	IEEESPAsin.obj\
	IEEESPAcos.obj\
	IEEESPLog10.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../mathieeesingtrans.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
