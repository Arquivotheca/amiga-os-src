
#include    <exec/types.h>
#include    <dos/dos.h>
#include    <dos/dosextens.h>
#include    <envoy/nipc.h>
#include    <envoy/errors.h>
#include    <envoy/services.h>
#include    <envoy/accounts.h>
#include    <clib/accounts_protos.h>
#include    <pragmas/accounts_pragmas.h>

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
};

struct CLock
{
    struct FileLock     CLock_FileLock;
    struct FileLock    *CLock_ServerLock;
    ULONG               CLock_ConnectionNumber; /* This lock is valid to this connection # */
    UBYTE              *CLock_FullServerPath;
};

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
};

struct TPacket
{
    APTR        tp_ServerMFSI;  /* Uniquely identifies this mount (unless somebody crashes) */
    APTR        tp_DosPacket;   /* The Client DOSPacket that this represents */
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
    struct DosPacket * tp_Owner; /* DosPacket owner on client side deleteme!  Not used anymore. */

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
#define FLAGSMASK               (MFSIF_NOBACKDROP|MFSIF_NODISKINFO)


#define EVF_SnapshotOK          1
#define EVF_LeftoutOK           2
#define EVF_Full                4

/* Greg's "new" dospackets
 * To handle uid-gid string conversion
 *
 */
#define ACTION_Dummy                20000
#define ACTION_USERNAME_TO_UID      (ACTION_Dummy+0)
#define ACTION_GROUPNAME_TO_GID     (ACTION_Dummy+1)
#define ACTION_UID_TO_USERINFO      (ACTION_Dummy+2)
#define ACTION_GID_TO_GROUPINFO     (ACTION_Dummy+3)

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


