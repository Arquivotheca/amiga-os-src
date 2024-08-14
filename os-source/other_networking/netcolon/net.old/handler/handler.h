/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                                          BBS:      */
/* | o  | ||   John Toebes     Dave Baker     John Mainwaring                */
/* |  . |//                                                                  */
/* ======                                                                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
**      $Filename: handler.h $ 
**      $Author: Doug $
**      $Revision: 1.18 $
**      $Date: 91/01/17 01:10:42 $
**      $Log:	handler.h,v $
 * Revision 1.18  91/01/17  01:10:42  Doug
 * Handle rebooting
 * 
 * Revision 1.17  91/01/15  03:03:27  J_Toebes
 * Add version string information.
 * 
 * Revision 1.16  91/01/15  02:57:29  Doug
 * Add fields to HPACKET and GLOBAL structures to support timer
 * requests and packet aging
 * 
 * Revision 1.15  91/01/11  10:39:32  Doug
 * Add support for network login packets.
 * 
 * Revision 1.14  91/01/11  00:42:35  Doug
 * *** empty log message ***
 * 
 * Revision 1.12  90/12/31  15:30:53  Doug
 * add protos for unimplemented funcs
 * 
 * Revision 1.11  90/12/30  15:49:22  Doug
 * Add protos for several ACTFNs
 * 
**
**/

#ifndef TESTING
#define TESTING 1
#endif

#include "netcomm.h"
#include "network-handler_rev.h"

#define VERSTRING VERSTAG "\0© 1991 SAS Institute Inc.  All Rights Reserved."

#define HAN_NETBUFSIZE 8192

#define PERMHPACK 10  /* Number of permanent HPACKETs */

/* These defines tell RemotePacket which of the ArgN values are NPTRS  */
/* If there is an error with the connection, the NPTRS must be checked */
/* for errors before sending the packet.  These are for the NPFlags    */
/* field of the HPACKET structure                                       */
#define NP1 0x01
#define NP2 0x02
#define NP3 0x04
#define NP4 0x08


/* Defines for the hp_State field */
#define HPS_FREE 0  /* Available                                    */
#define HPS_NEW  1  /* Just allocated to match application request  */
#define HPS_OLD  2  /* Has been sent to a driver at least once      */
#define HPS_PEND 3  /* Pending on I/O                               */
#define HPS_ERR  4  /* Tried to send to a driver but got an error   */
#define HPS_RETRY 5 /* Call RemotePacket again on this one          */
#define HPS_RELOCK 6 /* Lock being reestablished */

/* defines for NetNode status field */
#define NODE_DEAD    0     /* The node has never been heard from       */
#define NODE_UP      1     /* The node is up and responding normally   */
#define NODE_CRASHED 2     /* The node was responding, but is now down */
#define NODE_RELOCK  3     /* In the process of reestablishing locks   */

/* There is one NetNode structure for each node in the network.  Within*/
/* the NetNode structure is a definition for the NetPtr structure.  A  */
/* NetPtr structure completely describes a FileHandle or Lock on a     */
/* remote system: The NetNode pointer points to the NetNode struct for */
/* the appropriate node;  the RDevice field gives the address ON THE   */
/* REMOTE NODE of the AmigaDOS device to communicate with;  the RPtr   */
/* field gives the address ON THE REMOTE NODE of the FileHandle or Lock*/
/* we are dealing with.  To handle the NULL lock, we put an instance of*/
/* the NetPtr struct in the NetNode struct.  The NetNode field points  */
/* to the top of the struct, the RDevice field is normal and the RNPTR */
/* field is NULL.                                                      */
/* The ioptr field of the NetNode contains information specific to the */
/* communications driver for the node.  I will eventually also put a   */
/* pointer to the MsgPort for the driver once I support multiple       */
/* drivers.                                                            */
struct NetNode
{
   /* The NetPtr instance here is a special case - its NetNode pointer */
   /* points to the root of its own struct, its RPtr pointer is NULL.  */
   /* It is used when a 'template' NETPTR is needed for the net node.  */
   struct NetPtr
   {
      struct NetNode *NetNode;   /* Ptr to network address information     */
      RNPTR RPtr;                /* Remote file system's lock/filehandle   */
      short incarnation;         /* Validity check                         */
      char *objname;             /* NULL-terminated name of remote object  */
                                 /* NOTE: objname is only initialized if   */
                                 /*       network config parms say so and  */
                                 /*       it is a lock!                    */
   } RootLock;
   struct NetNode *next;      /* Next Net Node in chain                */
   int status;                /* Last known status - see above         */
   char *name;                /* Name of remote node                   */
   char devname[RNAMELEN+2];  /* TEMPORARY - name of remote device     */
   short spacefiller;         /* unused                                */
   APTR ioptr;                /* Driver-defined pointer                */
   DRIVER *driver;            /* Driver that owns this node            */
   long bufsize;              /* Maximum allowable read/write size     */
   struct FileLock *lkc;      /* Lock chain, linked by fl_Link         */
   struct FileLock *rlk;      /* Current lock in lock chain, if reestablishing */
   struct HandlerPacket *hpc; /* Chain of HPackets waiting to be sent  */
};


/* Simple check to see if a NPTR is valid */
#define CheckNPTR(np) ((np) != NULL && ((struct NetPtr *)(np))->incarnation != \
                          ((struct NetPtr *)(np))->NetNode->RootLock.incarnation)

typedef long SESSION;

/* The following typedef is used to represent both remote locks and    */
/* remote filehandles, since the same information is needed for both.  */
/* Using a single struct allows us to use it as a parm to RemotePacket */
/* whether we have a filehandle or a lock.                             */

typedef struct NetPtr *NETPTR;


/* Defines for the 'flags' field of GLOBAL */
#define NGF_RELOCK   0x00000001       /* Reestablish shared locks */
#define NGF_NORELOCK 0x00000002       /* Attempting to reestablish lock */

typedef struct global {
   struct NetGlobal      n;          /* Globals in common with server      */
   struct DeviceNode    *node;       /* our device node                    */
   struct DeviceList    *volume;     /* currently mounted volume           */
   struct NetNode       netchain;    /* Head of NetNode struct chain       */
   struct NetNode      *lastnode;    /* Last node packet was posted to     */
   struct HandlerPacket *donepkts;   /* HPACKETs that are ready to be done */
   struct HandlerPacket *tmphpkt;    /* List of temporary HPACKETS         */
   struct HandlerPacket *qpkts;      /* List of pkts waiting on replies    */
   struct HandlerPacket *prmhpkt;    /* Pointer to list of permanent hpkts */
   DRIVER *drivers;                  /* List of active drivers             */
   int    numnodes;                  /* Number of nodes in the chain       */
   int    upnodes;                   /* Number of up nodes in the chain    */
   long   unitnum;                   /* Unit number we are opened as       */
   long   flags;                     /* See defines above                  */
   ULONG  sigbits;                   /* Signal bits available to the driver*/
   ULONG  waitbits;                  /* Signal bits to wait on             */
   char                *work;        /* Workspace for constructing packets */
   struct FileLock     *devslock;    /* Lock on devs:networks              */
   struct TimerPacket  *timerpkt;    /* Pointer to packet for timer.device */
}  *HGLOBAL;

#undef GLOBAL
#define GLOBAL HGLOBAL

typedef struct HandlerPacket
{
   struct HandlerPacket *hp_Next;     /* Next handler packet on the chain     */
   struct HandlerPacket *hp_WNext;    /* Next packet in wait chain            */
   struct NetNode   *hp_NNode;        /* NetNode associated with this pkt     */
   struct DosPacket *hp_Pkt;          /* DOS Packet sent to us by application */
   struct RPacket   *hp_RP;           /* RPacket allocated from driver        */
   DRIVER           *hp_Driver;       /* Driver that owns this action         */
   void (*hp_Func)(GLOBAL, struct HandlerPacket *);
                                      /* Function to call when pkt returns    */
   short            hp_State;         /* State of packet; 0 == available      */
   short            hp_NPFlags;       /* Which args are NetPtrs               */
   short            hp_PktFlags;      /* Which DOS packet args are BPTRS      */
   short            hp_Age;           /* Number of timer pkts since transmit  */
} HPACKET;

typedef void (*ifuncp)(GLOBAL, HPACKET *);

#define HPACKET_PERM ((HPACKET *)-1)

#define HPQ(g,h) (h)->hp_WNext=(g)->donepkts, (g)->donepkts=(h);

#define HPAGEOUT  5  /* Number of timer packets before we check connection */
#define HPTICK    2  /* Length of one timer packet in seconds */

/* Each packet type has a corresponding Act??? function */
/* All of them take the same parameters - thus, use a macro to define them */
#define ACTFN(name) void name (GLOBAL global, HPACKET *hpkt)

/* file.c */
ACTFN( ActDelete        );
ACTFN( ActRename        );
ACTFN( ActSetComment    );
ACTFN( ActSetProtection );
ACTFN( ActSetFileDate   );

/* io.c */
ACTFN( ActFindwrite  );
ACTFN( ActFindwrite2 );
#define ActFindin ActFindwrite
#define ActFindout ActFindwrite
ACTFN( ActEnd        );
ACTFN( ActEnd2       );
ACTFN( ActRead       );
ACTFN( ActRead2      );
ACTFN( ActWrite      );
ACTFN( ActWrite2     );
ACTFN( ActSeek       );

/* dir.c */
ACTFN( ActCreateDir  );
ACTFN( ActCreateDir2 );
ACTFN( ActExamine    );
ACTFN( ActExamine2   );
#define ActExNext ActExamine
ACTFN( ActParent     );
ACTFN( ActParent2    );
ACTFN( ActMakeLink   );
ACTFN( ActReadLink   );
ACTFN( ActReadLink2  );

/* main.c */
ACTFN( ActSetDebug   );
ACTFN( ActNetPwd     );
ACTFN( ActNetHello   );
ACTFN( ActNetHello2  );
void freehpkt(GLOBAL global, HPACKET *hpkt);
void KillHPackets(GLOBAL, struct NetNode *, int);

/* lock.c */
struct FileLock *CreateLock (GLOBAL, NETPTR nlock,RNPTR RLock, LONG Access,
                             struct RPacket *RP);
void FreeLock   (GLOBAL, struct FileLock *);
ACTFN( ActLock       );
ACTFN( ActLock2      );
ACTFN( ActDupLock    );
ACTFN( ActDupLock2   );
ACTFN( ActUnLock     );
ACTFN( ActUnLock2     );
int ParseName   (GLOBAL, char *, NETPTR *, char *);
struct NetNode *FindNode (GLOBAL, char *);

/* Process.c */
ACTFN( ActDie         );
ACTFN( ActInhibit    );
ACTFN( ActFlush );
ACTFN( ActTimer );

/* volume.c */
ACTFN( ActCurentVol  );
ACTFN( ActRenameDisk );
ACTFN( ActDiskInfo   );
ACTFN( ActInfo       );
ACTFN( ActInfo2      );
ACTFN( ActNetKludge  );
ACTFN( ActDiskChange );
ACTFN( ActIsFS       );
ACTFN( ActNetInit    );
ACTFN( ActNetStart   );
ACTFN( ActNetTerm    );

/* unimp.c */
ACTFN( ActFormat        );
ACTFN( ActFHFromLock    );
ACTFN( ActChangeMode    );
ACTFN( ActCopyDirFH     );
ACTFN( ActParentFH      );
ACTFN( ActExamineAll    );
ACTFN( ActExamineFH     );
ACTFN( ActSameLock      );

/* device.c */
int GetDevice  (GLOBAL, struct FileSysStartupMsg *);
int InitDevice (GLOBAL);
int TermDevice (GLOBAL);
struct NetNode *AddNode (GLOBAL, char *);
void ParseConfig(GLOBAL);

/* Devio.c */
int RemotePacket (GLOBAL, HPACKET *);
int CheckLock(GLOBAL, NETPTR, struct RPacket *);

/* inhibit.c */
int inhibit  (struct MsgPort *, long);
long sendpkt (struct MsgPort *, long, long*, long);

/* mount.c */
void Mount         (GLOBAL, char *);
void DisMount  (GLOBAL);

/* Requester routine */
int  request         (GLOBAL, int, char *);
#define REQ_MUST    0
#define REQ_ERROR   1
#define REQ_GENERAL 2

/* devio.c */
void NetRestart (GLOBAL, NETPTR);
HPACKET *GetHPacket(GLOBAL);
void FreeHPacket(GLOBAL, HPACKET *);

/* timer.c */
ACTFN(ActTimer);

/* Following used throughout handler to do two things: */
/* - check to see if a lock is a lock on the root of NET: */
/* - assign the 'nlock' value to the 'n' parameter  */
#define TESTROOT(f,n) (!f || (n=(NETPTR)(f->fl_Key))->RPtr == (RNPTR)1)

struct RPacket *AllocRPacket(struct NetNode *nnode, long size);
void FreeRPacket(APTR drglobal, struct RPacket *RP);

#include "proto.h"

#include "memwatch.h"
