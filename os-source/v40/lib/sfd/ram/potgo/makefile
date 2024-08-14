########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	AllocPotBits.obj\
	FreePotBits.obj\
	WritePotgo.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../potgo.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
