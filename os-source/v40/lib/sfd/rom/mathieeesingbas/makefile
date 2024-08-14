########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	IEEESPFix.obj\
	IEEESPFlt.obj\
	IEEESPCmp.obj\
	IEEESPTst.obj\
	IEEESPAbs.obj\
	IEEESPNeg.obj\
	IEEESPAdd.obj\
	IEEESPSub.obj\
	IEEESPMul.obj\
	IEEESPDiv.obj\
	IEEESPFloor.obj\
	IEEESPCeil.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../mathieeesingbas.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
