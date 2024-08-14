
/* Commands Used by the UserData Server */

#define ACMD_AddUser		100	/* Add a new user */
#define ACMD_RemUser		101	/* Remove a user */
#define ACMD_AddGroup		102	/* Add a group */
#define ACMD_RemGroup		103	/* Remove a group */
#define ACMD_AddMember		104	/* Add a user to a group */
#define ACMD_RemoveMember	105	/* Remove a user from a group */
#define ACMD_NextUser		106	/* For listing Users */
#define ACMD_NextMember		107	/* For listing Group members */
#define ACMD_NextGroup		108	/* For listing Groups */
#define ACMD_IDToUser		109	/* UID To Username */
#define ACMD_NameToUser		110	/* Username to UID */
#define ACMD_IDToGroup		111	/* GID To GroupName */
#define ACMD_NameToGroup	112	/* Groupname to GID */
#define ACMD_VerifyUser		113	/* Verify a User */
#define ACMD_AdminUser		114	/* Modify a User */
#define ACMD_ModifyGroup	115	/* Modify a Group */
#define ACMD_MemberOf		116	/* Check if a user is a member of a group */

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

#define UFLAGB_AdminName	0	/* Whether or not this User may change his/her own name */
#define UFLAGB_AdminPassword	1	/* Whether or not this User may change his/her own password */
#define UFLAGB_NeedsPassword	2	/* Whether or not a password is required for this user */
#define	UFLAGB_AdminGroups	3	/* Whether or not this User may create/delete groups */
#define UFLAGB_AdminAll		4	/* User may do anything he wants. */

#define UFLAGF_AdminName	(1 << UFLAGB_AdminName)
#define UFLAGF_AdminPassword	(1 << UFLAGB_AdminPassword)
#define UFLAGF_NeedsPassword	(1 << UFLAGB_NeedsPassword)
#define UFLAGF_AdminGroups	(1 << UFLAGB_AdminGroups)
#define UFLAGF_AdminAll		(1 << UFLAGB_AdminAll)

