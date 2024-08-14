########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	IEEEDPFix.obj\
	IEEEDPFlt.obj\
	IEEEDPCmp.obj\
	IEEEDPTst.obj\
	IEEEDPAbs.obj\
	IEEEDPNeg.obj\
	IEEEDPAdd.obj\
	IEEEDPSub.obj\
	IEEEDPMul.obj\
	IEEEDPDiv.obj\
	IEEEDPFloor.obj\
	IEEEDPCeil.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../mathieeedoubbas.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
