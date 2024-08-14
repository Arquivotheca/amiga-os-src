########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	SPFix.obj\
	SPFlt.obj\
	SPCmp.obj\
	SPTst.obj\
	SPAbs.obj\
	SPNeg.obj\
	SPAdd.obj\
	SPSub.obj\
	SPMul.obj\
	SPDiv.obj\
	SPFloor.obj\
	SPCeil.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../mathffp.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
