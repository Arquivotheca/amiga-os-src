########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	AddAppWindowA.obj\
	AddAppWindow.obj\
	RemoveAppWindow.obj\
	AddAppIconA.obj\
	AddAppIcon.obj\
	RemoveAppIcon.obj\
	AddAppMenuItemA.obj\
	AddAppMenuItem.obj\
	RemoveAppMenuItem.obj\
	WBInfo.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../wb.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
