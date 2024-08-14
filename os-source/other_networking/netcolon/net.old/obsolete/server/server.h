/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                                          BBS:      */
/* | o  | ||   John Toebes     Dave Baker     John Mainwaring                */
/* |  . |//                                                                  */
/* ======                                                                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netcomm.h"
#include "sdn_pragmas.h"

#define PERMSPACK   10
#define MAXNODENAME 32


/* Private action numbers for state information                                 */
#define ACTION_RETURN           5558
#define ACTION_RETRW            5559
#define ACTION_RWMORE1          5560
#define ACTION_RWMORE2          5561
#define ACTION_RWMORE3          5562
#define ACTION_RETOPEN          5563
#define ACTION_RETEND           5564
#define ACTION_RETLOCKNAME      5565
#define ACTION_RETPARENT        5566
#define ACTION_RETNAMEEX        5567
#define ACTION_RETNAMELOCK      5568
#define ACTION_RETNAMEUNLOCK    5569
#define ACTION_FINDINFO         5570
#define ACTION_RETINFO          5571
#define ACTION_RETLOCK          5572

#define DEFICON "\x16icons/__DEFAULT__.info"

#define CLASS_READONLY 0
#define CLASS_SHARED   1
#define CLASS_EXCLUDE  2
#define CLASS_ASSIGN   0x80     /* This is a bit mask setting over the others   */

#define MAXPW 32

struct ACCESS {
   struct ACCESS *next;
   int class;                   /* One of the CLASS values above.  Note that a  */
                                /* a bit setting of CLASS_ASSIGN is given, we   */
                                /* will store the assign name just after the    */
                                /* real name in the name array.                 */
   char name[1];
};

struct DEVLIST {
   struct DEVLIST *next;        /* Next entry on the list                       */
   short  type;                 /* The type.  One of:                           */
                                /*    2 = ST_USERDIR  a directory               */
                                /*    4 = ST_LINKDIR  an assign to a directory  */
                                /*   -3 = ST_FILE     a file                    */
                                /*   -4 = ST_LINKFILE an assign to a file       */
   short  protect;              /* Access protection bits for the entry         */
/* struct MsgPort *port;        ** Message port associated with the entry       */
   BPTR lock;                   /* Lock for any assigned item.                  */
   char len;                    /* Length of the name (make this a BSTR)        */
   char name[1];                /* The actual name itself                       */
};

struct LCHAIN {
   struct LCHAIN *next;
   void          *data;
};

struct FCHAIN {
   struct FileHandle fh;
   struct FCHAIN *next;
};

/* Indicator of the current session */
typedef struct session
{
   struct session *next;     /* Next active session                             */
   DRIVER         *driver;   /* Driver handle for communicating with session    */
   struct LCHAIN  *locks;    /* List of active locks                            */
   struct FCHAIN  *fhs;      /* List of active file handles                     */
   char            name[MAXNODENAME]; /* Our name as we know it                 */
   struct DEVLIST *devices;  /* List of devices we have access to               */
} SESSION;

typedef struct ServerPacket {
   struct Message        sp_Msg;        /* These two fields make it look like   */
   struct DosPacket      sp_Pkt;        /*   a normal StandardPacket            */
   struct ServerPacket  *sp_Next;       /* Next server packet on the chain.     */
                                        /*  == SPACKET_PERM for permanent packet*/
   struct RPacket       *sp_RP;         /* RPacket to respond to at completion  */
   DRIVER               *sp_Driver;     /* Driver which initiated the action    */
   SESSION              *sp_Ses;        /* Session associated with packet       */
   short                 sp_Action;     /* Action code to be processed          */
   short                 sp_State;      /* Action state value                   */
                                        /*   Note that 0 indicates an inactive  */
                                        /*   packet that is available for reuse */
} SPACKET;
#define SPACKET_PERM ((SPACKET *)-1)
typedef struct global
   {
   struct NetGlobal       n;            /* Globals in common with handler       */
   long                   sigbits;      /* signal bits available to the driver  */
   long                   waitbits;     /* Bits to wait on.                     */
   long                   replymask;    /* Signal bit of our port               */
   int                    reply;        /* Flag to indicate time to reply       */
   DRIVER                *drivers;      /* Points to list of active drivers     */
   SPACKET               *pspacket;     /* Array of permanent SPackets          */
   SPACKET               *tempspkt;     /* List of temporary SPackets           */
   SESSION               *sessions;     /* List of all active sessions          */
   struct FCHAIN         *fhs;          /* List of all open files               */
   struct MsgPort        *dosport;      /* Msgport of last dos device           */
   BPTR                   portlock;     /* Relative lock for operation          */
   BPTR                   configlock;   /* Lock on the networks/devs directory  */
   }* SGLOBAL;

#undef GLOBAL
#define GLOBAL SGLOBAL
#define ROOTLOCK ((BPTR)-2)

void checkdebug U_ARGS((void));

#if DEBUG
#define BUGCHECK() checkdebug();
#define BUGINIT()  initdebug(NULL);
#else
#define BUGCHECK()
#define BUGINIT()
#endif

/* Each packet type has a corresponding Act??? function */
/* All of them take the same parameters - thus, use a macro to define them */
#define ACTFN(name) void name (GLOBAL global, SPACKET *spkt)

/* main.c */
ACTFN(RmtSetDebug    );

/* file.c */
ACTFN(RmtDelete        );
ACTFN(RmtRename        );
ACTFN(RmtSetComment    );
ACTFN(RmtSetProtection );
ACTFN(RmtSetFileDate   );

/* io.c */
ACTFN(RmtRW        );
ACTFN(RmtRWRet     );
ACTFN(RmtRWMore    );
ACTFN(RmtRWMore1   );
ACTFN(RmtRWMore2   );
ACTFN(RmtRWMore3   );
ACTFN(RmtSeek      );
ACTFN(RmtOpen      );
ACTFN(RmtOpenRet   );
ACTFN(RmtEnd       );
ACTFN(RmtEndRet    );

/* dir.c */
ACTFN(RmtCreateDir );
ACTFN(RmtNameLockRet);
ACTFN(RmtFindInfo  );
ACTFN(RmtDoneInfo  );
ACTFN(RmtExamine   );
#define RmtExNext RmtExamine
ACTFN(RmtParent    );
ACTFN(RmtParentRet );

/* lock.c */
ACTFN(RmtLock      );
void NameNode     U_ARGS((GLOBAL, SPACKET *, BPTR));
struct MsgPort *GetRDevice   U_ARGS((GLOBAL, SPACKET *, BPTR, char *));
ACTFN(RmtRetLock    );
ACTFN(RmtNameExamine);
ACTFN(RmtNameLock  );
ACTFN(RmtNameUnlock);
ACTFN(RmtDupLock   );
ACTFN(RmtUnLock    );
void chainlock    U_ARGS((GLOBAL, SPACKET *, BPTR));
void unchainlock  U_ARGS((GLOBAL, SPACKET *, BPTR));

/* volume.c */
ACTFN(RmtInfo       );
ACTFN(RmtNetLogin   );
ACTFN(RmtNetStart   );
ACTFN(RmtNetStop    );

/* device.c */
void ParseConfig U_ARGS((GLOBAL));
SPACKET *GetSPacket U_ARGS((GLOBAL));
void FreeSPacket U_ARGS((GLOBAL, SPACKET *));
ACTFN(RmtDie        );

/* Dispatch.c */
void Dispatch    U_ARGS((GLOBAL, SPACKET *, int, struct MsgPort *));
ACTFN(RmtReturn     );

/* inhibit.c */
int inhibit  U_ARGS((struct MsgPort *, long));
long sendpkt U_ARGS((struct MsgPort *, long, long*, long));

/* volume.c */
ACTFN(RmtInfo       );
SESSION *NewSession U_ARGS((GLOBAL));
struct ACCESS *loadaccess U_ARGS((GLOBAL global, char *name));
void freeaccess U_ARGS((GLOBAL global, struct ACCESS *access));
void freedevlist U_ARGS((GLOBAL global, struct DEVLIST *devlist));
struct DEVLIST *builddevlist U_ARGS((GLOBAL global, struct ACCESS *access));

void __stdargs kprintf(char *, ...);

#include "proto.h"
