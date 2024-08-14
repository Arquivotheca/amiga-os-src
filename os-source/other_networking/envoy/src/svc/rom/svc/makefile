########################################
MAKEMETA=	../../../../tools/makemeta

OBJS=\
	RexxReserved.obj\
	StartService.obj\
	GetServiceAttrsA.obj\
	GetServiceAttrs.obj

lib.timestamp:	asm.timestamp
	make objs ${MFLAGS} ${MARGS}
	cat *.obj >../svc.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs:		${OBJS}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
