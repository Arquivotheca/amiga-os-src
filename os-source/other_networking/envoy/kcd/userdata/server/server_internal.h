
#define ID_UDAT		MAKE_ID('U','D','A','T')
#define ID_GDAT		MAKE_ID('G','D','A','T')
#define ID_UHDR		MAKE_ID('U','H','D','R')
#define ID_GHDR		MAKE_ID('G','H','D','R')
#define	ID_USER		MAKE_ID('U','S','E','R')
#define ID_GRUP		MAKE_ID('G','R','U','P')

struct UserIFFHeader
{
	UWORD	Version;
	UWORD	Revision;
	UWORD	NextUserID;
	UBYTE	AdminPassword[32];
};

struct GroupIFFHeader
{
	UWORD	Version;
	UWORD	Revision;
	UWORD	NextGroupID;
};

struct UserIFFChunk
{
	UBYTE	UserName[32];		/* User's Name (31 chars max) */
	UBYTE	Password[32];		/* User's Password (31 chars max) */
	UWORD	UserID;			/* User's Numeric UID */
	UWORD	GroupID;		/* User's Primary Group ID */
	ULONG	Flags;			/* Various Flags */
};

struct GroupIFFChunk
{
	UBYTE	GroupName[32];
	UWORD	GroupID;
	UWORD	AdminID;
	ULONG	Flags;

	/* Followed by Users*2 bytes of UserID's */
};

struct ServerData
{
    struct Library	*sd_SysBase;
    struct Library	*sd_DOSBase;
    struct Library	*sd_NIPCBase;
    struct Library	*sd_UtilityBase;
    struct UserIFFHeader sd_UserHeader;
    struct GroupIFFHeader sd_GroupHeader;
    struct MinList	 sd_UserList;
    struct MinList	 sd_GroupList;
    BOOL		 sd_FlushUserDat;
    BOOL		 sd_FlushGroupDat;
};

typedef struct ServerData *ServerDataPtr;

#define DOSBase		sd->sd_DOSBase
#define NIPCBase	sd->sd_NIPCBase
#define UtilityBase	sd->sd_UtilityBase
