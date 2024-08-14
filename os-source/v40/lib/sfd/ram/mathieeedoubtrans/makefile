########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	IEEEDPAtan.obj\
	IEEEDPSin.obj\
	IEEEDPCos.obj\
	IEEEDPTan.obj\
	IEEEDPSincos.obj\
	IEEEDPSinh.obj\
	IEEEDPCosh.obj\
	IEEEDPTanh.obj\
	IEEEDPExp.obj\
	IEEEDPLog.obj\
	IEEEDPPow.obj\
	IEEEDPSqrt.obj\
	IEEEDPTieee.obj\
	IEEEDPFieee.obj\
	IEEEDPAsin.obj\
	IEEEDPAcos.obj\
	IEEEDPLog10.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../mathieeedoubtrans.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
