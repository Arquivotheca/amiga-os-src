/**********************************************************
 *
 *               Test Device Handler
 *
 *
 *********************************************************/
 
/* mods 27 Apr 1993, J.W. Lockhart: port to 3.0/SAS6.2 setup */

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h> 
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include "dev.h"
#include "test.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include "testdevice_protos.h"
#include "packmoncommon_protos.h"

#define HOST_PORT_NAME   "testdevice"
#define RESTORE 1
#define MODIFY  2



struct Library          *SysBase;       /* EXEC library base                    */
struct DosLibrary       *DOSBase;      /* DOS library base for debug process   */

/* pragmas off of vars from this module */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

/* More globals --------------------------------------------------------------- */
struct Process          *DosProc;       /* Our Process                               */
struct DeviceNode       *DosNode;       /* Our DOS node.. created by DOS for us      */
struct DeviceList       *DevList;       /* Device List structure for our volume node */

struct MsgPort     *pid;                /* process indentifier (handler message port) */
struct DosPacket   *packet;
struct Message     *msg;

struct MsgPort *HandReplyPort=0;
struct MsgPort *HandDataPort=0;

struct HAND_MESSAGE *h_message;


struct MinList  LCBase;

int monitoring_packets;

ULONG generated_error;

ULONG real_Task;
ULONG real_Volume;

UWORD noLock = 0;

/* this thing doesn't have that "in-use" look...
    short error_trans[] = {
        0,
        ERROR_NOT_A_DOS_DISK,
        ERROR_OBJECT_NOT_FOUND,
        ERROR_OBJECT_NOT_FOUND,
        ERROR_NO_FREE_STORE,
        ERROR_OBJECT_IN_USE,
        ERROR_INVALID_LOCK,
        ERROR_NOT_A_DOS_DISK,
        ERROR_NO_FREE_STORE,
        ERROR_NOT_A_DOS_DISK,
        ERROR_NOT_A_DOS_DISK,
        ERROR_NOT_A_DOS_DISK,
        ERROR_NOT_A_DOS_DISK,
        ERROR_NOT_A_DOS_DISK,
        0,
        ERROR_NOT_A_DOS_DISK,
        ERROR_OBJECT_IN_USE,
        ERROR_NOT_A_DOS_DISK,
        ERROR_NO_MORE_ENTRIES,
    };
*/

/* Protos that aren't cared for elsewhere */
extern VOID kprintf(STRPTR,...);
extern VOID KPrintF(STRPTR,...);

/* mane ==============================================================================
 */
VOID mane(VOID) {
    register short error;
    char    Vol_name[36]; /* this is too short and should come from a big chunk I dynamically allocate */
    short   Vol_size;
    struct  MsgPort        *localreplyport;
    struct  StandardPacket *localpacket;
/*    ULONG   rxERR; */
    ULONG   handbit;
    ULONG   Dmask;
    ULONG   waitmask;
    

/*    KPrintF("Now in mane...\n"); */
    SysBase = *(struct Library **)4UL;
    DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 0);
    if (!DOSBase) {
        return;         /* proper?? */
    }
    DosProc = (struct Process *)FindTask(NULL);

    HandDataPort = FindPort("Test_Handler");
    HandReplyPort = createPort(0L,0L);

    h_message = AllocMem(sizeof(struct HAND_MESSAGE),MEMF_PUBLIC|MEMF_CLEAR);
    if (!h_message) {
        return;  /* proper ?? */
    }

    localreplyport = createPort(NULL, 0);
    localpacket = (struct StandardPacket *) AllocMem((long)sizeof(struct StandardPacket),MEMF_PUBLIC|MEMF_CLEAR);
    monitoring_packets =  0;
    

    h_message->mesI.mn_ReplyPort = HandReplyPort;
    h_message->mesI.mn_Length = sizeof(struct HAND_MESSAGE);

/* debug */
/*    kprintf("siz %ld \n", sizeof(struct HAND_MESSAGE)); */


    h_message->m_type = STARTUP;

    /* Buggy??  Put message, wait for reply, eat one reply */
    PutMsg(HandDataPort, (struct Message *)h_message);
    WaitPort(HandReplyPort);
    while(!GetMsg(HandReplyPort))
         ;   /* do nothing */

    execute_command(h_message);

    WaitPort(&DosProc->pr_MsgPort);         /*  Get Startup Packet  */

    msg = GetMsg(&DosProc->pr_MsgPort);
    packet = (struct DosPacket *)msg->mn_Node.ln_Name;

/* debug */
/*    kprintf("got startup packet...\n"); */

    /*
     *  Loading DosNode->dn_Task causes DOS *NOT* to startup a new
     *  instance of the device driver for every reference.  E.G. if
     *  you were writing a CON device you would want this field to
     *  be NULL.
     */

    if (DOSBase && HandDataPort && localreplyport && localpacket) {
            struct DosInfo *di = (struct DosInfo *) BADDR(((struct RootNode *)DOSBase->dl_Root)->rn_Info);
            register struct DeviceList *dl = dosalloc(sizeof(struct DeviceList));

            DosNode = (struct DeviceNode *) BADDR(packet->dp_Arg3);

        /*
         *  Create Volume node and add to the device list.  This will
         *  cause the WORKBENCH to recognize us as a disk.  If we don't
         *  create a Volume node, Wb will not recognize us.  
         *  
         */

            DevList = dl;
            dl->dl_Type = DLT_VOLUME;
            dl->dl_Task = &DosProc->pr_MsgPort;
            dl->dl_DiskType = ID_DOS_DISK;
            dl->dl_Name = (BSTR )DosNode->dn_Name;
            dl->dl_Next = di->di_DevInfo;
            di->di_DevInfo = (long)CTOB(dl);
    
            btos((STRPTR)(dl->dl_Name), Vol_name);      /* save volume name */
            Vol_size = *((char *)BADDR(dl->dl_Name)); /* sizeof string */ 
            Vol_name[Vol_size++] = ':';

        /*
         *  Set dn_Task field which tells DOS not to startup a new
         *  process on every reference.
         */

            DosNode->dn_Task = &DosProc->pr_MsgPort;
            packet->dp_Res1 = DOS_TRUE;
            packet->dp_Res2 = 0;
     }
     else {
          /*  couldn't open dos.library   */
/* debug */
/*            kprintf("failed1........"); */

            packet->dp_Res1 = DOS_FALSE;
            if (localreplyport) {
                deletePort(localreplyport);
            }
            if (localpacket) {
                FreeMem(packet, sizeof(struct StandardPacket));
            }

            returnpacket(packet);
/* debug */
/*            kprintf("failed2........"); */

            return;                     /*  exit process                */
            }

/* kprintf("about to returnpacket\n"); */
    returnpacket(packet);

/* KPrintF("init of LCBase...\n"); */
    newList((struct List *)&LCBase);

    handbit = 1L << HandDataPort->mp_SigBit;
    Dmask   = 1L << DosProc->pr_MsgPort.mp_SigBit;
    waitmask = Dmask | handbit; 

    /*
     *      Here begins the endless loop, waiting for requests over our
     *      message port and executing them.  Since requests are sent over
     *      our message port, this precludes being able to call DOS functions
     *      ourselves
     */


for (;;)
    {
    struct InfoData *id;
/* kprintf("in for  \n"); */


/*  Under normal conditions we are waiting on two signals. 
If all we wanted to do was drain both ports dry when a signal
was recieved, that would be easy.  But unfortunatly, we want 
to do things syncronously when we are monitoring packets.
(We'll wait for a rexx message before proceeding to the
 next phase of packet handling.)
When we switch to monitor mode we must remember that either
signal can be set at any time...
*/




    Wait(Dmask);

    while (msg = GetMsg(&DosProc->pr_MsgPort)) 
            {
/* kprintf("in while \n"); */
            packet = (struct DosPacket *)msg->mn_Node.ln_Name;

            h_message->m_type = SENDMG;
            h_message->pkt_pointer = packet;


/* kprintf("putmsg to $%08lx TESTDEV main loop packet = $%08lx\n", HandDataPort, packet); */

            PutMsg(HandDataPort, (struct Message *)h_message);
            WaitPort(HandReplyPort);



/*              Wait(handbit); */
/*kprintf("done waiting for reply TSTDEV \n");*/

            GetMsg(HandReplyPort);  

            execute_command(h_message);

            localpacket->sp_Msg.mn_Node.ln_Name = (char *)&(localpacket->sp_Pkt);
            localpacket->sp_Pkt.dp_Link         = &(localpacket->sp_Msg);
            localpacket->sp_Pkt.dp_Port         = localreplyport;
            localpacket->sp_Pkt.dp_Type         = packet->dp_Type;

/* Debug:
               kprintf("Packet: %3ld %08lx %08lx %08lx %08lx %08lx %10s\n ",
                    packet->dp_Type,
                    packet->dp_Arg1, 
                    packet->dp_Arg2,
                    packet->dp_Arg3,
                    packet->dp_Arg4,
                    packet->dp_Arg5,
                    typetostr(packet->dp_Type)
                    );
*/


            if (!generated_error) {   /* no error, send packet to real handler */
                {
                    long *args, *pargs, count;
                    struct DosPacket *localdos;

                    pargs = &(localpacket->sp_Pkt.dp_Arg1);
                    localdos = &(localpacket->sp_Pkt);
                    args  = &(packet->dp_Arg1);  
                    for (count=0; count < 7; count++) {
                            *pargs++ = *args++;
                    }
                    restoreLock(localdos);
/* Debug:
                   kprintf(" Sent Packet: %3ld %08lx %08lx %08lx %08lx %08lx %10s\n ",
                        localdos->dp_Type,
                        localdos->dp_Arg1, 
                        localdos->dp_Arg2,
                        localdos->dp_Arg3,
                        localdos->dp_Arg4,
                        localdos->dp_Arg5,
                        typetostr(localdos->dp_Type)
                        );
*/


                }  /* scope, NOT if */
                    PutMsg(pid, (struct Message *)localpacket); /* send packet to real handler*/
/* kprintf("sent packet to real dev \n"); */

                    WaitPort(localreplyport);

/* kprintf("got reply from real device \n"); */
                    GetMsg(localreplyport);

                    error = 0;
                    packet->dp_Res1 = localpacket->sp_Pkt.dp_Res1;
                    packet->dp_Res2 = localpacket->sp_Pkt.dp_Res2;

                    if (packet->dp_Res1)  { /*  don't modify results if they are bad */
/* KPrintF("In the Big Switch, type %lu ($%08lx)\n", (ULONG)packet->dp_Type, (ULONG)packet->dp_Type); */
                            switch(packet->dp_Type) {
                                case ACTION_DIE:            /*  attempt to die?                      */
                                    break;
                                case ACTION_OPENNEW:        /* FileHandle,Lock,Name        Bool      */
                                    break;
                                case ACTION_OPENRW:         /* FileHandle,Lock,Name        Bool      */
                                    break;
                                case ACTION_OPENOLD:        /* FileHandle,Lock,Name        Bool      */
                                    break;
                                case ACTION_READ:           /* FHArg1,CPTRBuffer,Length   ActLength  */
                                    break;
                                case ACTION_WRITE:          /* FHArg1,CPTRBuffer,Length   ActLength  */
                                    break;
                                case ACTION_CLOSE:          /* FHArg1                     Bool:TRUE  */
                                    break;
                                case ACTION_SEEK:           /* FHArg1,Position,Mode      OldPosition */
                                    break;
                                case ACTION_EXAMINE_NEXT:   /* Lock,Fib               Bool          */
                                    break;
                                case ACTION_EXAMINE_OBJECT: /* Lock,Fib                       Bool  */
                                    break;
                                case ACTION_INFO:           /* Lock, InfoData    Bool:TRUE          */
                                    id = (struct InfoData *) BADDR(packet->dp_Arg2);
                                    id->id_VolumeNode = (long)CTOB(DosNode);
                                    break;
                                case ACTION_DISK_INFO:      /* InfoData          Bool:TRUE          */

                                    /* We must replace real info with our INFO */
                                    {
                                    id = (struct InfoData *) BADDR(packet->dp_Arg1);
                                    id->id_VolumeNode = (long)CTOB(DosNode);
                                    }
                                    break;

                                case ACTION_PARENT:         /* Lock                       ParentLock */
                                    GetTestLock();
                                    break;
                                case ACTION_DELETE_OBJECT:  /* Lock,Name              Bool           */
                                    break;
                                case ACTION_CREATE_DIR:     /* Lock,Name                  Lock       */
                                    GetTestLock();
                                    break;
                                case ACTION_LOCATE_OBJECT:  /* Lock,Name,Mode             Lock       */
                                    GetTestLock();
                                    break;
                                case ACTION_COPY_DIR:       /* Lock,                      Lock       */
                                    GetTestLock();
                                    break;
                                case ACTION_FREE_LOCK:      /* Lock,                      Bool       */
                                    FreeTestLock(BADDR(packet->dp_Arg1));
                                    break;

                                case ACTION_SET_PROTECT:    /* -,Lock,Name,Mask          Bool        */
                                    break;
                                case ACTION_SET_COMMENT:    /* -,Lock,Name,Comment       Bool        */
                                    break;
                                case ACTION_RENAME_OBJECT:  /* SLock,SName,DLock,DName    Bool       */
                                    break;
                                case ACTION_INHIBIT:        /* Bool                       Bool       */
                                    break;

                                /* Under construction: */
                                case ACTION_SAME_LOCK:      /* BPTR lock1, BPTR lock2; LONG rc, CODE ioerr for lock_diff */
                                    break;
                                case ACTION_CHANGE_SIGNAL:
                                    break;
                                case ACTION_FORMAT:
                                    break;
                                case ACTION_MAKE_LINK:  /* BPTR lock for:, BSTR linkName, BPTR targetLock, LONG linkType; BOOL rc, CODE ioerr */
                                    break;
                                case ACTION_READ_LINK:  /* BPTR lock, CPTR path/name, APTR buf, LONG bufsize; LONG strlen, CODE ioerr */
                                    break;
                                case ACTION_FH_FROM_LOCK: /* BPTR fh, LOCK lock; BOOL succ, CODE ioerr */
                                    break;
                                case ACTION_IS_FILESYSTEM:
                                    break;
                                case ACTION_CHANGE_MODE:    /* LONG objType, BPTR object, LONG newMode; BOOL succ, CODE ioerr */
                                    break;
                                case ACTION_COPY_DIR_FH:
                                    break;
                                case ACTION_PARENT_FH:
                                    break;
                                case ACTION_EXAMINE_ALL: /* BPTR lock, APTR buf, LONG len, LONG type, APTR ctrl; LONG cont, CODE ioerr */
                                    break;
                                case ACTION_EXAMINE_FH:  /* BPTR lock, BPTR fib; BOOL rc, CODE ioerr */
                                    break;
                                case ACTION_LOCK_RECORD:
                                    break;
                                case ACTION_FREE_RECORD:
                                    break;
                                case ACTION_ADD_NOTIFY: /* BPTR notifyreq; BOOL success, CODE ioerr */
                                    break;
                                case ACTION_REMOVE_NOTIFY: /* BPTR notifyreq; BOOL success, CODE ioerr */
                                    break;
                                case ACTION_EXAMINE_ALL_END:
                                    break;
                                case ACTION_SET_OWNER:
                                    break;

                                /*
                                 *       A few other packet types which we do not support
                                 */
                                case ACTION_RENAME_DISK:    /* BSTR:NewName               Bool       */
                                case ACTION_MORECACHE:      /* #BufsToAdd                 Bool       */
                                case ACTION_WAIT_CHAR:      /* Timeout, ticks             Bool       */
                                case ACTION_FLUSH:          /* writeout bufs, disk motor off         */
                                case ACTION_RAWMODE:        /* Bool(-1:RAW 0:CON)         OldState   */

                                default:
                                        break;
                            }  /* switch */
/* KPrintF("Leaving the Big Switch...\n"); */
                        }   /* if packet->dp_res1 */
                    }  /* if !generated_error */
            else {
                    error = generated_error;
            }
            if (packet) 
                    {
                    if (error)
                            {
                               kprintf("!!!!!!! ERROR !!!!!!!!!!\nERR=%ld\n", error);


                            switch(packet->dp_Type) /* Special cases return -1 for error */
                                    {
                                    case ACTION_READ:
                                    case ACTION_SEEK:
                                            packet->dp_Res1 = -1;
                                            break;
                                    default:
                                            packet->dp_Res1 = DOS_FALSE;
                                    }

                            packet->dp_Res2 = error;
                            }
                    else
                            {
                               /* kprintf("RES=%06lx\n", packet->dp_Res1);  */
                            }
                    returnpacket(packet);
                    }


            }
    
/*kprintf("returned packet \n");*/

    }
}





/* FreeTestLock ===================================================
 */
VOID FreeTestLock(struct FileLock *lock) {
    Remove((struct Node *)(lock->fl_Link));                  /* unlink from list */
    FreeMem((APTR)(lock->fl_Link), sizeof(LOCKLINK));       /* free link node   */
    dosfree(lock);
}



/* GetTestLock =============================================================
 * tries to copy lock.  But shouldn't it be doing DupLock()?
 */
VOID GetTestLock(VOID) {
    struct FileLock *realLock;
    struct FileLock *lock;
    LOCKLINK *ln;


    realLock = (struct FileLock *) BADDR(packet->dp_Res1);

/*
    kprintf("REAL LOCK %08lx \n", realLock);
    kprintf("REAL LOCK link %08lx \n", realLock->fl_Link);
*/

    if (realLock) {
        ln = AllocMem(sizeof(LOCKLINK), MEMF_PUBLIC|MEMF_CLEAR);
        lock =  dosalloc(sizeof(struct FileLock));

        if (ln && lock) {
            AddHead((struct List *)&LCBase, (struct Node *)ln);
            ln->lock = lock;
            lock->fl_Link= (long)ln;
            lock->fl_Key = (long)realLock;
            lock->fl_Access = realLock->fl_Access;
            lock->fl_Task = &DosProc->pr_MsgPort;
            lock->fl_Volume = (BPTR)CTOB(DosNode);
            packet->dp_Res1 = (LONG)(CTOB(lock));
        }       
        else {
            if (ln) {
                FreeMem(ln,sizeof(LOCKLINK));
            }
            if (lock) {
                dosfree(lock);
            }
            packet->dp_Res1 = 0;  /* should be DOSFALSE, as getting the lock failed. */
        
        }
    }
}

/* getOLDlock ========================================================
    Actually, expects "struct FileLock *arg", not LONG, but
    the casting of the existing code is a pain.
 */
LONG getOLDlock(LONG LockArg) {
    struct FileLock *lock = (struct FileLock *)(BADDR(LockArg));

    if(!lock) {
        return(NULL);
    }

    return((LONG)(CTOB(lock->fl_Key)));

}





int restoreLock(struct DosPacket *ourpacket) {
    int return_code = !0;

    switch (ourpacket->dp_Type)  {
        case ACTION_DIE:            /*  attempt to die? */
                break;

        case ACTION_FINDOUTPUT:    /*   FileHandle,Lock,Name        Bool    */

                /*kprintf("NEWport %06lx",((struct FileHandle *)BADDR(ourpacket->dp_Arg1))->fh_Port);
                  kprintf("NEWType %06lx",((struct FileHandle *)BADDR(ourpacket->dp_Arg1))->fh_Type);
                 */
                ourpacket->dp_Arg2 = getOLDlock(ourpacket->dp_Arg2);
                break;

        case ACTION_FINDUPDATE:         /*      FileHandle,Lock,Name        Bool    */
                /* kprintf("RWport %ld",((struct FileHandle *)(ourpacket->dp_Arg1))->fh_Port);
                   kprintf("RWType %ld",((struct FileHandle *)(ourpacket->dp_Arg1))->fh_Type);
                 */
                ourpacket->dp_Arg2 = getOLDlock(ourpacket->dp_Arg2);
                break;

        case ACTION_FINDINPUT:    /*    FileHandle,Lock,Name        Bool    */
                /* kprintf("nnnnn");
                   kprintf("OLDport %06lx",((struct FileHandle *)BADDR(ourpacket->dp_Arg1))->fh_Port);
                   kprintf("OLDType %06lx",((struct FileHandle *)BADDR(ourpacket->dp_Arg1))->fh_Type);
                 */
                ourpacket->dp_Arg2 = getOLDlock(ourpacket->dp_Arg2);
                break;

        case ACTION_READ:                /*      FHArg1,CPTRBuffer,Length   ActLength  */
                break;

        case ACTION_WRITE:          /*   FHArg1,CPTRBuffer,Length   ActLength  */
                break;

        case ACTION_CLOSE:          /*   FHArg1                     Bool:TRUE  */
                break;

        case ACTION_SEEK:           /*   FHArg1,Position,Mode       OldPosition*/
                break;

        case ACTION_EXAMINE_NEXT: /*   Lock,Fib               Bool       */
                ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);
                break;

        case ACTION_EXAMINE_OBJECT: /*   Lock,Fib                       Bool       */
                ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);
                break;

        case ACTION_INFO:           /*  Lock, InfoData    Bool:TRUE    */
                ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);
                break;

        case ACTION_DISK_INFO:  /*      InfoData          Bool:TRUE    */
                break;

        case ACTION_PARENT:     /*       Lock                       ParentLock */
                ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);
                break;

        case ACTION_DELETE_OBJECT: /*Lock,Name              Bool       */
                ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);
                break;

        case ACTION_CREATE_DIR: /*       Lock,Name                  Lock       */
                ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);
                break;

        case ACTION_LOCATE_OBJECT:      /*   Lock,Name,Mode             Lock       */
                ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);
                break;

        case ACTION_COPY_DIR:   /*       Lock,                      Lock       */
                ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);
                break;

        case ACTION_FREE_LOCK:  /*       Lock,                      Bool       */
                ourpacket->dp_Arg1  = getOLDlock(ourpacket->dp_Arg1);
                break;

        case ACTION_SET_PROTECT:/*       -,Lock,Name,Mask          Bool       */
                ourpacket->dp_Arg2 = getOLDlock(ourpacket->dp_Arg2);
                break;

        case ACTION_SET_COMMENT:/*       -,Lock,Name,Comment       Bool       */
                ourpacket->dp_Arg2  = getOLDlock(ourpacket->dp_Arg2);
                break;

        case ACTION_RENAME_OBJECT:/* SLock,SName,DLock,DName    Bool       */
                ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);
                ourpacket->dp_Arg3 = getOLDlock(ourpacket->dp_Arg3);
                break;

        case ACTION_INHIBIT:    /*       Bool                       Bool       */
                break;

        case ACTION_FH_FROM_LOCK:  /* BPTR fh, LOCK lock; BOOL succ, CODE ioerr */
                ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);
                break;

        case ACTION_MAKE_LINK:  /* BPTR lock for:, BSTR linkName, BPTR targetLock, LONG linkType; BOOL rc, CODE ioerr */
                ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);
                break;
        case ACTION_SAME_LOCK:      /* BPTR lock1, BPTR lock2; LONG rc, CODE ioerr for lock_diff */
                return_code = 0;
                break;

        /* Under construction: */
        case ACTION_CHANGE_SIGNAL:
            break;
        case ACTION_FORMAT:
            break;
        case ACTION_READ_LINK:  /* BPTR lock, CPTR path/name, APTR buf, LONG bufsize; LONG strlen, CODE ioerr */
            break;
        case ACTION_IS_FILESYSTEM:
            break;
        case ACTION_CHANGE_MODE:    /* LONG objType, BPTR object, LONG newMode; BOOL succ, CODE ioerr */
            break;
        case ACTION_COPY_DIR_FH:
            break;
        case ACTION_PARENT_FH:
            break;
        case ACTION_EXAMINE_ALL: /* BPTR lock, APTR buf, LONG len, LONG type, APTR ctrl; LONG cont, CODE ioerr */
            ourpacket->dp_Arg1 = getOLDlock(ourpacket->dp_Arg1);        /* kludge guess */
            break;
        case ACTION_EXAMINE_FH:  /* BPTR lock, BPTR fib; BOOL rc, CODE ioerr */
            break;
        case ACTION_LOCK_RECORD:
            break;
        case ACTION_FREE_RECORD:
            break;
        case ACTION_ADD_NOTIFY: /* BPTR notifyreq; BOOL success, CODE ioerr */
            break;
        case ACTION_REMOVE_NOTIFY: /* BPTR notifyreq; BOOL success, CODE ioerr */
            break;
        case ACTION_EXAMINE_ALL_END:
            break;
        case ACTION_SET_OWNER:
            break;


        /* old stuff */
        case ACTION_RENAME_DISK:
        case ACTION_MORECACHE:  
        case ACTION_WAIT_CHAR:  
        case ACTION_FLUSH:      
        case ACTION_RAWMODE:    
        case ACTION_SET_FILE_SIZE:

        default:
                break;
    }
    return(return_code);
}




/*
 *  PACKET ROUTINES.    Dos Packets are in a rather strange format as you
 *  can see by this and how the PACKET structure is extracted in the
 *  GetMsg() of the main routine.
 */

VOID returnpacket(register struct DosPacket *packet) {
    register struct Message *mess;
    register struct MsgPort *replyport;

    replyport                = packet->dp_Port;
    mess                     = packet->dp_Link;
    packet->dp_Port          = &DosProc->pr_MsgPort;
    mess->mn_Node.ln_Name    = (char *)packet;
    mess->mn_Node.ln_Succ    = NULL;
    mess->mn_Node.ln_Pred    = NULL;
    PutMsg(replyport, mess);

    packet = 0;
}

/*
 *  Are there any packets queued to our device?
 *  [they won't be detected by this; it's never called! :-)]
 */

BOOL packetsqueued(VOID) {

   return((BOOL)(!IsMsgPortEmpty(&(DosProc->pr_MsgPort))));
   /* ie, is lh_TailPred == &mp_MsgList? */

/* old code (below) looks kinda bogus.  rewrite is above.
    return ((BOOL)((VOID *)DosProc->pr_MsgPort.mp_MsgList.lh_Head !=
            (VOID *)&DosProc->pr_MsgPort.mp_MsgList.lh_Tail));
*/
}


/*
 *  DOS MEMORY ROUTINES
 *
 *  DOS makes certain assumptions about LOCKS.  A lock must minimally be
 *  a FileLock structure, with additional private information after the
 *  FileLock structure.  The longword before the beginning of the structure
 *  must contain the length of structure + 4.
 *
 *  NOTE!!!!! The workbench does not follow the rules and assumes it can
 *  copy lock structures.  This means that if you want to be workbench
 *  compatible, your lock structures must be EXACTLY sizeof(struct FileLock).
 */

VOID *dosalloc(register ULONG bytes) {
    register ULONG *ptr;

    bytes += 4;
    ptr = AllocMem(bytes, MEMF_PUBLIC|MEMF_CLEAR);
    *ptr = bytes;
    return(ptr+1);
}

VOID dosfree(register VOID *aptr) {
    ULONG *ptr = (ULONG *)aptr;

    --ptr;
    FreeMem(ptr, *ptr);
}

/*
 *  Convert a BSTR into a normal string.. copying the string into buf.
 *  I use normal strings for internal storage, and convert back and forth
 *  when required.
 */

VOID btos(UBYTE *bstr, UBYTE *buf) {

    bstr = (UBYTE *) BADDR(bstr);
    CopyMem(bstr+1,buf,*bstr);
    buf[*bstr] = 0;
}



struct MsgPort *setup_port(VOID) {

   struct MsgPort *the_port;
   Forbid();

   /* look for someone else that looks just like us! */
   if (FindPort("Test_Handler"))
   {
     Permit();
     return(NULL);
   }

   /* allocate the port */
   the_port = createPort("Test_Handler", 0L);

   Permit();

   return(the_port);
}

VOID shutdown_port(struct MsgPort *the_port) {
   deletePort(the_port);
}




VOID execute_command(struct HAND_MESSAGE *h_message) {

    switch(h_message->m_type) {
        case STARTUP:
            pid = (struct MsgPort *)(h_message->pid);  /* proper cast? */
            /* kprintf("in T DEV startup %ld\n",pid);*/
            break;

        case SENDMG:
            generated_error = h_message->error;
            break;
    }
}

/*  Matt's debug stuff... */ 

STRPTR typetostr(int ty) {
    switch(ty) {
        case ACTION_DIE:              return("DIE");
        case ACTION_OPENRW:           return("OPEN-RW");
        case ACTION_OPENOLD:          return("OPEN-OLD");
        case ACTION_OPENNEW:          return("OPEN-NEW");
        case ACTION_READ:             return("READ");
        case ACTION_WRITE:            return("WRITE");
        case ACTION_CLOSE:            return("CLOSE");
        case ACTION_SEEK:             return("SEEK");
        case ACTION_EXAMINE_NEXT:     return("EXAMINE NEXT");
        case ACTION_EXAMINE_OBJECT:   return("EXAMINE OBJ");
        case ACTION_EXAMINE_ALL:      return("EXAMINE ALL");
        case ACTION_EXAMINE_ALL_END:  return("EXAMINE ALL END");
        case ACTION_EXAMINE_FH:       return("EXAMINE FH");
        case ACTION_INFO:             return("INFO");
        case ACTION_DISK_INFO:        return("DISK INFO");
        case ACTION_PARENT:           return("PARENTDIR");
        case ACTION_DELETE_OBJECT:    return("DELETE");
        case ACTION_CREATE_DIR:       return("CREATEDIR");
        case ACTION_LOCATE_OBJECT:    return("LOCK");
        case ACTION_COPY_DIR:         return("DUPLOCK");
        case ACTION_FREE_LOCK:        return("FREELOCK");
        case ACTION_SERIALIZE_DISK:   return("SERIALIZE DISK");
        case ACTION_SET_PROTECT:      return("SETPROTECT");
        case ACTION_SET_COMMENT:      return("SETCOMMENT");
        case ACTION_SET_OWNER:        return("SETOWNER");
        case ACTION_RENAME_OBJECT:    return("RENAME");
        case ACTION_INHIBIT:          return("INHIBIT");
        case ACTION_RENAME_DISK:      return("RENAME DISK");
        case ACTION_MORECACHE:        return("MORE CACHE");
        case ACTION_WAIT_CHAR:        return("WAIT FOR CHAR");
        case ACTION_FLUSH:            return("FLUSH");
        case ACTION_RAWMODE:          return("RAWMODE");
        default:                      return("---------UNKNOWN-------");
    }
}

