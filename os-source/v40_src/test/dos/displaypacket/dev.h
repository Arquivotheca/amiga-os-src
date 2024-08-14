
#define ACTION_OPENRW       1004
#define ACTION_OPENOLD      1005
#define ACTION_OPENNEW      1006
#define ACTION_CLOSE        1007
#define ACTION_SEEK         1008
#define ACTION_RAWMODE      994
#define ACTION_MORECACHE    18


#define NONE                            0
#define DONE                            1000
#define a_ACTION_LOCATE_OBJECT          2001
#define SEND                            2002
#define a_ACTION_EXAMINE_OBJECT         2003
#define a_OPEN                          2004
#define GOT_OPEN_LOCK                   2005
#define a_ACTION_READ                   2006
#define a_PARENT                        2007
#define a_ACTION_SEEK                   2008
#define a_ACTION_CLOSE                  2009
#define OPEN_GT_ONE                     2010
#define a_ACTION_WRITE                  2011

#define TYPE_DIR    1
#define TYPE_FILE   -1


#define PC_LOCK                         0
#define PC_EXAMINE                      1
#define PC_EXNEXT                       2
#define PC_OPEN                         3
#define PC_READ                         4
#define PC_PARENT                       5
#define PC_SEEK                         6
#define PC_AJAR                         7
#define PC_OPEN_NXT                     8
#define PC_CLOSE                        9
#define PC_WRITE                        10
#define PC_CREAT                        11

#define RWBUFFSIZ                       512



#define CTOB(x) (void *)(((long)(x))>>2)    /*  BCPL conversion */


#define DOS_FALSE   0
#define DOS_TRUE    -1


#define LOCKLINK    struct _LL

typedef struct FileLock         LOCK;       /*  See LOCKLINK    */
typedef struct DeviceList       DEVLIST;
typedef struct DosInfo          DOSINFO;
typedef struct RootNode         ROOTNODE;
typedef struct FileHandle       FH;
typedef struct MsgPort          PORT;
typedef struct Message          MSG;
typedef struct MinList          LIST;
typedef struct MinNode          NODE;
typedef struct DateStamp        STAMP;
typedef struct InfoData         INFODATA;
typedef struct DosLibrary       DOSLIB;


/*
 *  We use this structure to link locks together in a list for internal
 *  usage.  I could have use the link field in the lock structure as a
 *  real linked list, but didn't want to have to sequentially search the
 *  list to remove a node.
 *
 *  NOTE:   You CANNOT simply extend the FileLock (LOCK) structure.  Some
 *  programs assume it is sizeof(LOCK) big and break.  I found this out the
 *  hard way.
 */

LOCKLINK {
    NODE    node;
    LOCK    *lock;
        };

struct PChandle
        {
        struct Node node;
        short filehandle;       /* MS-Dos handle associated with file */
        struct FileLock *lock;  
        };


struct PClock
        {
        struct Node node;
        short number_of_locks;  /* < 0  is exclusive lock */
        short type;
        short path_length;
        short filehandle0;      /* PC filehandle obtained when I opened the
                                   file. This is how I obtain a "lock". */
        short HandleAvailable; /* The "lock" Filehandle is used for the first
                                  OPEN.  If this handle has been used 
                                  HandleAvailable is FALSE else it's TRUE */
        };

struct PCDosServ
        {
        short function;
        short error;
        short filehandle;
        short access;
        long offset;
        char  Xfer[43];
        char name[512];
        };


extern void *AbsExecBase;

/* Dude, protos, right?
    extern void *AllocMem(), *RemHead(), *CreatePort(), *GetMsg();
    extern void *FindTask(), *Open(), *OpenLibrary();
 */
extern void    returnpacket(),btos(),*dosalloc();
extern STRPTR typetostr(int);
extern char *strupr();
char *pervert_path();
short translate();
struct PClock *GetPClock();
char *GetLockPath();
struct PClock *MakeDirLock();
struct FileLock *syslock();
