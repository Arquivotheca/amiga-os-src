
#include    <exec/types.h>
#include    <dos/dos.h>
#include    <dos/dosextens.h>
#include    <envoy/nipc.h>
#include    <envoy/errors.h>
#include    <envoy/services.h>
#include    <envoy/accounts.h>
#include    <clib/accounts_protos.h>
#include    <pragmas/accounts_pragmas.h>

#define TRANS_SIZE	512

#define BITFLIP(x,m) (( (x) & ~((LONG)m)) | ( ~(x) & ((LONG)m) ))

struct ResourcesUsed
{
    struct    Node  ru_link;
    APTR            ru_Resource;
};

struct CFH
{
    struct FileHandle  *CFH_FH;
    APTR                CFH_ServerFH;
    ULONG               CFH_Pos;
    ULONG               CFH_Access;
    ULONG               CFH_ConnectionNumber;
    UBYTE              *CFH_FullServerPath;
    ULONG		CFH_Flags;
    struct MinList	CFH_Packets;	// packets waiting for a read/write to finish
};
#define CFHF_RW_ACTIVE	1		// a read or write is active with more transactions to send
					// if active, hold ALL packets for the FH until the R/W is finished
					// we must do this because we can't allow the transaction of the
					// r/w to mix with the transactions of later packets for the FH.
struct SFH
{
	struct FileHandle  sfh_FH;
	ULONG		   sfh_Flags;
};
#define SFHF_NOWRITE	1		// don't allow writes to the filehandle!


struct ExNextBlock
{
    struct MinNode      Node;
    UBYTE		Available;
    UBYTE		Type;
    UBYTE		Error;
    UBYTE		pad;
    ULONG		Res2;
    UBYTE		buffer[TRANS_SIZE];
};

struct ExNextData
{
    struct ExAllData   *CurrentEntry;
    struct DosPacket   *WaitingPacket;
    struct ExNextBlock *CurrentBlock;
    struct MinList	BlockList;
    struct DosPacket   *FakePacket;
};

// server structure for LRU list of ExAllControl structures
struct EAC_Node {
    struct MinNode	en_Node;
    BPTR		en_Lock;
    struct ExAllControl *en_EAC;
    ULONG		en_Type;	// 0 or highest type that should be used for this exall
};

struct CLock
{
    struct FileLock     CLock_FileLock;
    struct FileLock    *CLock_ServerLock;
    ULONG               CLock_ConnectionNumber; /* This lock is valid to this connection # */
    UBYTE              *CLock_FullServerPath;
    struct ExNextData  *ExNextData;
    UBYTE		Examined;
    UBYTE		Zombie;			// free this on transaction end
};

// 0 means it's not a zombie
#define ZOMBIE_FREE	1			// free it when possible
#define ZOMBIE_NOFREE	2			// free only exnext data when possible, leave clock

struct MountedFSInfo
{
    struct MinNode      mfsi_Link;         /* Link to the next one */
    UBYTE              *mfsi_LocalPath;    /* correctly delimited base of this mount */
    struct Entity      *mfsi_SourceEntity; /* Client */
    struct MinList      mfsi_Locks;        /* List of all of the locks belonging to this mount */
    struct MinList      mfsi_FileHandles;  /* List of all of the filehandles belonging to this mount */
    struct MinList      mfsi_Etc;          /* Record locks, etc. */
    struct FileLock    *mfsi_BaseLock;     /* The Base Lock for this mount */
    APTR                mfsi_ServerMFSI;   /* Filled in only on the client */
    UBYTE               mfsi_HostName[80]; /* Name of the client */
    UBYTE               mfsi_ClientMount[256]; /* Full client mount name */
    UBYTE               mfsi_UserName[33];
    UBYTE               mfsi_Password[33];
    STRPTR              mfsi_VolumeName;
    UWORD               mfsi_UID;          /* User ID and Group ID for this user */
    UWORD               mfsi_GID;
    ULONG               mfsi_UserFlags;
    ULONG               mfsi_Flags;
    APTR                mfsi_FIB;           /* Temp fib */
};

struct MountedFSInfoClient
{
    struct MinNode      mfsi_Link;         /* Link to the next one */
    struct Entity      *mfsi_SourceEntity; /* Client */
    struct MinList      mfsi_Locks;        /* List of all of the locks belonging to this mount */
    struct MinList      mfsi_FileHandles;  /* List of all of the filehandles belonging to this mount */
    struct FileLock    *mfsi_BaseLock;     /* The Base Lock for this mount */
    APTR                mfsi_ServerMFSI;   /* Filled in only on the client */
    struct Transaction *mfsi_ConnectTrans; // transaction used for connect/reconnect
    ULONG               mfsi_ConnectionNumber;
    struct MsgPort     *mfsi_LocalPort;
    APTR                mfsi_Volume;
    struct Entity      *mfsi_Source;
    struct Entity      *mfsi_Destination;
    UBYTE              *mfsi_Machine;      /* name of machine on which this fs exists */
    UBYTE              *mfsi_LocalPath;    /* Local mountname of this mount */
    UBYTE              *mfsi_RemotePath;   /* Remote mountpoint */
    UBYTE              *mfsi_UserName;
    UBYTE              *mfsi_Password;
    ULONG               mfsi_Flags;         /* Filter flags, etc. */
    UBYTE               mfsi_VolumeName[64];
    UWORD               mfsi_DynamicDelay;
};

struct TPacket
{
    struct MountedFSInfoClient *tp_ServerMFSI;  /* Uniquely identifies this mount (unless somebody crashes) */
    struct DosPacket *tp_DosPacket;   /* The Client DOSPacket that this represents */
    LONG        tp_Type;
    LONG        tp_Res1;
    LONG        tp_Res2;
    LONG        tp_Arg1;
    LONG        tp_Arg2;
    LONG        tp_Arg3;
    LONG        tp_Arg4;
    LONG        tp_Arg5;
    LONG        tp_Arg6;
    LONG        tp_Arg7;
    ULONG	tp_Unused; /* _MUST_ stay here! Or compatibility will be broken! */
};

struct RequestMounts
{
    UBYTE       User[64];
    UBYTE       Password[64];
};

struct IndividualMount
{
    UBYTE       MountName[64];
};


#define FSCMD_MOUNT      1
#define FSCMD_UNMOUNT    2
#define FSCMD_DOSPACKET  3
#define FSCMD_SHOWMOUNTS 4

#define FSERR_REJECTED_USER     32768       /* user/password pair not adequate */
#define FSERR_REJECTED_NOMOUNT  32769       /* Given mountpoint not allowed */

#define MFSIF_NOBACKDROP        1L          /* Filter .backdrop files */
#define MFSIF_NODISKINFO        2L          /* Don't show or let them snapshot the stuff */
#define MFSIF_SECURITYON        4L          /* Use GID/UID/Permissions bits */
#define MFSIF_NOSECURITY        8L          /* No security whatsoever */
#define MFSIF_NOREQUESTERS      16L         /* When something goes bad, no requesters, please. */
#define MFSIF_V37_FS		32L	    // this filesystem doesn't support ED_OWNER
#define MFSIF_COMMENT_BSTR	64L	    // this filesystem is a V37 ROM fs, returns comments as BSTRs
#define FLAGSMASK               (MFSIF_NOBACKDROP|MFSIF_NODISKINFO)


#define EVF_SnapshotOK          1
#define EVF_LeftoutOK           2
#define EVF_Full                4
#define EVF_NoSecurity          8

#define MAX_RW_PACKETS	2		// How many read/write packets active at a time.
#define MAXSIZE		32768		// Maximum size for a read/write transaction
#define PACKETTIMEOUT	6		// seconds before trying to reconnect

/* Greg's "new" dospackets
 * To handle uid-gid string conversion
 *
 */
#define ACTION_Dummy                20000
#define ACTION_USERNAME_TO_UID      (ACTION_Dummy+0)
#define ACTION_GROUPNAME_TO_GID     (ACTION_Dummy+1)
#define ACTION_UID_TO_USERINFO      (ACTION_Dummy+2)
#define ACTION_GID_TO_GROUPINFO     (ACTION_Dummy+3)

#define ACTION_ENVOY_EXAMINE_ALL    5678	// internal only

/* Private EFS packet below */
#define ACTION_EFS_INFO 64000			// internal only
#define ERROR_NETWORK_FAILED ERROR_SEEK_ERROR	// silly

/* Etc types -- resources other than Filehandles and locks that
 * need to be freed when a mount disappears.
 */

#define ETCTYPE_RecordLock          1

struct LockedRecord
{
    BPTR        rl_FileHandle;
    ULONG       rl_Position;
    ULONG       rl_Length;
};


#define UFLAGB_AdminName        0       /* Whether or not this User may change his/her own name */
#define UFLAGB_AdminPassword    1       /* Whether or not this User may change his/her own password */
#define UFLAGB_NeedsPassword    2       /* Whether or not a password is required for this user */
#define UFLAGB_AdminGroups      3       /* Whether or not this User may create/delete groups */
#define UFLAGB_AdminAll         4       /* User may do anything he wants. */

#define UFLAGF_AdminName        (1 << UFLAGB_AdminName)
#define UFLAGF_AdminPassword    (1 << UFLAGB_AdminPassword)
#define UFLAGF_NeedsPassword    (1 << UFLAGB_NeedsPassword)
#define UFLAGF_AdminGroups      (1 << UFLAGB_AdminGroups)
#define UFLAGF_AdminAll         (1 << UFLAGB_AdminAll)

#define BPF_USER    1
#define BPF_GROUP   2
#define BPF_OTHER   4

#define OWNERBITS	(FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE)
#define GROUPBITS	(FIBF_GRP_READ|FIBF_GRP_WRITE|FIBF_GRP_EXECUTE|FIBF_GRP_DELETE)
#define OTHERBITS	(FIBF_OTR_READ|FIBF_OTR_WRITE|FIBF_OTR_EXECUTE|FIBF_OTR_DELETE)
#define ALLBITS		(OWNERBITS|GROUPBITS|OTHERBITS)
