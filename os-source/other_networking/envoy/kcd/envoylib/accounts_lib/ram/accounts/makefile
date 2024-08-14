########################################
MAKEMETA=	../../../../tools/makemeta

OBJS=\
	AllocUserInfo.obj\
	AllocGroupInfo.obj\
	FreeUserInfo.obj\
	FreeGroupInfo.obj\
	VerifyUser.obj\
	MemberOf.obj\
	NameToUser.obj\
	NameToGroup.obj\
	IDToUser.obj\
	IDToGroup.obj\
	NextUser.obj\
	NextGroup.obj\
	NextMember.obj\
	ECrypt.obj\
	VerifyUserCrypt.obj

lib.timestamp:	asm.timestamp
	make objs ${MFLAGS} ${MARGS}
	cat *.obj >../accounts.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs:		${OBJS}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
