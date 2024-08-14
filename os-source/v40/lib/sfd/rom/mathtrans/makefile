########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	SPAtan.obj\
	SPSin.obj\
	SPCos.obj\
	SPTan.obj\
	SPSincos.obj\
	SPSinh.obj\
	SPCosh.obj\
	SPTanh.obj\
	SPExp.obj\
	SPLog.obj\
	SPPow.obj\
	SPSqrt.obj\
	SPTieee.obj\
	SPFieee.obj\
	SPAsin.obj\
	SPAcos.obj\
	SPLog10.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../mathtrans.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
