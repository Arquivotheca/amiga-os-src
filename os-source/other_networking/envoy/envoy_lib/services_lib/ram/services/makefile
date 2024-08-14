########################################
MAKEMETA=	../../../../tools/makemeta

OBJS=\
	FindServiceA.obj\
	FindService.obj\
	LoseService.obj

lib.timestamp:	asm.timestamp
	make objs ${MFLAGS} ${MARGS}
	cat *.obj >../services.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs:		${OBJS}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
