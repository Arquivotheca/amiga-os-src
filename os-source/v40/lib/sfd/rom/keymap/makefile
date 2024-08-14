########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	SetKeyMapDefault.obj\
	AskKeyMapDefault.obj\
	MapRawKey.obj\
	MapANSI.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../keymap.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
