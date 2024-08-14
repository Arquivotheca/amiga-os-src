########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	AllocMiscResource.obj\
	FreeMiscResource.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../misc.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
