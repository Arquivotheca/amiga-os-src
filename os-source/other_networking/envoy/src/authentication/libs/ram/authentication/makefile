########################################
MAKEMETA=	../../../../tools/makemeta

OBJS=\
	AddGroup.obj\
	AddToGroup.obj\
	AddUser.obj\
	AllocUserProfile.obj\
	AuthenticateUser.obj\
	AllocGroup.obj\
	FreeGroup.obj\
	ChangePassWord.obj\
	ChooseAuthServer.obj\
	DeleteGroup.obj\
	DeleteUser.obj\
	AuthErrorText.obj\
	FindGroupID.obj\
	FindGroupName.obj\
	FindUserName.obj\
	FreeUserProfile.obj\
	ListGroups.obj\
	ListMembers.obj\
	ListUsers.obj\
	RemoveFromGroup.obj\
	MemberOf.obj\
	NoSecurity.obj

lib.timestamp:	asm.timestamp
	make objs ${MFLAGS} ${MARGS}
	cat *.obj >../authentication.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs:		${OBJS}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
