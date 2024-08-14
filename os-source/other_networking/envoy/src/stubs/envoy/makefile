########################################
MAKEMETA=	../../../../tools/makemeta

OBJS=\
	HostRequestA.obj\
	HostRequest.obj

lib.timestamp:	asm.timestamp
	make objs ${MFLAGS} ${MARGS}
	cat *.obj >../envoy.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs:		${OBJS}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
