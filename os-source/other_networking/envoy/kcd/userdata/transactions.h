
/* Commands Used by the UserData Server */

#define UD_AddUser	100		/* Add a new user */
#define UD_RemUser	101		/* Remove a user */
#define UD_AddGroup	102		/* Add a group */
#define UD_RemGroup	103		/* Remove a group */
#define UD_AddMember	104		/* Add a user to a group */
#define UD_RemoveMember	105		/* Remove a user from a group */
#define UD_NextUser	106		/* For listing Users */
#define UD_NextMember	107		/* For listing Group members */
#define UD_NextGroup	108		/* For listing Groups */
#define UD_IDToUser	109		/* UID To Username */
#define UD_NameToUser	110		/* Username to UID */
#define UD_IDToGroup	111		/* GID To GroupName */
#define UD_NameToGroup	112		/* Groupname to GID */
#define UD_VerifyUser	113		/* Verify a User */
#define UD_AdminUser	114		/* Modify a User */
#define UD_ModifyGroup	115		/* Modify a Group */
#define UD_MemberOf	116		/* Check if a user is a member of a group */

struct AuthorityData
{
	UBYTE	UserName[32];		/* User Name of the Authority */
	UBYTE	Password[32];		/* Password of the Authority */
	UBYTE	AuthPW[32];		/* UserData Password */
};

typedef struct AuthorityData AuthorityData;

struct UserData
{
	UBYTE	UserName[32];		/* User's Name */
	UBYTE	Password[32];		/* User's Password */
	UWORD	UserID;			/* UID */
	UWORD	PrimaryGroupID;		/* Primary GID */
	ULONG	Flags;
};

typedef struct UserData UserData;

struct GroupData
{
	UBYTE	GroupName[32];		/* The Group's Name */
	UWORD	GroupID;		/* The Group's GID */
	UWORD	AdminID;		/* The Group Administrator's UID */
	ULONG	Flags;			/* Misc flags */
};

typedef struct GroupData GroupData;

struct AdminUserRequest
{
	AuthorityData	Authority;	/* The Authority */
	UserData	User;		/* The User to administrate */
};

typedef struct AdminUserRequest AdminUserReq;



struct ModifyGroupRequest
{
	AuthorityData	Authority;	/* The Authority */
	GroupData	Group;		/* The Group */
	UserData	User;		/* The (possibly Empty) User */
};

typedef struct ModifyGroupRequest ModifyGroupReq;

struct NextMemberRequest
{
	GroupData	Group;		/* Group we're seaching through */
	UserData	User;		/* Current User */
};

typedef struct NextMemberRequest NextMemberReq;

struct AdminGroupRequest
{
	AuthorityData	Authority;	/* The Authority */
	GroupData	Group;		/* The group to remove */
};

typedef struct AdminGroupRequest AdminGroupReq;

struct UserNode
{
	struct Node	Link;
	UWORD		Pad0;
	UserData	Data;
};

typedef struct UserNode UserNode;

struct GroupNode
{
	struct Node	Link;
	UWORD		Pad0;
	GroupData	Data;
	struct MinList	UserList;
};

typedef struct GroupNode GroupNode;

struct MinUserNode
{
	struct MinNode	Link;
	struct UserNode *User;
};

typedef struct MinUserNode MinUserNode;

/* Currently Defined Flags for the User Flags data */

#define UFLAGB_AdminSelf	0	/* Whether or not this User may change his/her own data */
#define	UFLAGB_AdminGroups	1	/* Whether or not this User may create/delete groups */
#define UFLAGB_AdminAll		2	/* User may do anything */

#define UFLAGF_AdminSelf	(1 << UFLAGB_AdminSelf)
#define UFLAGF_AdminGroups	(1 << UFLAGB_AdminGroups)
#define UFLAGF_AdminAll		(1 << UFLAGB_AdminAll)

/* Error codes for the security server */

#define	SECERROR_NORESOURCES	100
#define SECERROR_NOPRIVS	101
#define SECERROR_NOAUTHORITY	102
#define SECERROR_UNKNOWNUSER	103
#define SECERROR_UNKNOWNGROUP	104
#define SECERROR_LASTUSER	105
#define SECERROR_LASTGROUP	106
#define SECERROR_LASTMEMBER	107
#define SECERROR_GROUPEXISTS	108
#define SECERROR_NOFREEGROUPS	109
#define SECERROR_UNKNOWNMEMBER	110
