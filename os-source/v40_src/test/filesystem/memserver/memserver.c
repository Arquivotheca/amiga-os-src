/* MemServer.c       John W. Lockhart       Rel 1, February 1992
   This code is Copyright 1992 by Commodore International Services Corporation.

   Memory Server and Recorder
   compile:  lc -HCommon_h.pre -t -cs MemServer.c

   Purpose In Life:
        To provide a means of allocating RAM on any given expansion 
        board, or on the motherboard.  Also, to allow multiple programs
        to share blocks of RAM, and provide a means where those programs
        can write coherently to a logfile.

   Current Bugs:
        Does not handle motherboard Fast RAM properly.  (ie, thinks that all
        motherboard RAM is Chip RAM)  [Only for V1.x.]
        Believed fixed in V2.x.

   Use:  run memserver <logfile> [safe] [pri] [NoSer] [NoClientQuit]
   Where:
        logfile is the name of the file you want to use to record the activities
                of the MemServer's clients.  It is humbly suggested that you
                choose someplace on a scratch disk drive, or serial port,
                so that you don't munge anything if you crash.  (Crashes should
                be due to hardware or firmware failures, btw :-)
        safe    is whether to close the logfile between writes.  While it
                adds some security and peace of mind, it also slows down the
                recordkeeping (ie, Open/Seek/Close all the time).  And if you're
                writing out the serial port, you don't really want this option,
                unless you like Seek()ing on SER:...  Default: unsafe.
        pri     Priority at which to run.  Default is 4.
        noser   Suppress KPrintF() messages out SER:.  Default: ON at 9600 baud.
                Special bell-ringing and inverse video is supported for errors.
        NCQ     Don't tell clients to quit upon severe error.

   --------------------------------------------------------------------------------

   Misc useful note:
   Note that er_Type and cd_BoardSize are vital to figuring out
   exactly where the memory you want is.

    NOTA BENE!!
        Memory boards ONLY are counted.  Thus, if you have
             A2630              MemBoardID = 1
             A2091 0K
             A2058 2MB          MemBoardID = 2
             A2065
             A2091 1MB          MemBoardID = 3
       (Although in expansion.library, A2630=1, A2091=2,
        A2058 = 3, A2065 = 4, A2091 = 5 AND 6.)

    This code is not residentable.
 */


/* Includes ------------------------------------------------------------ */
/* NB: There are really a TON of includes; they're hiding in common_h.c,
   and are precompiled for the compiling programmer's convenience.
 */
#include "server.h"

/* DEFINES -------------------------------------------------------------- */
#define TEMPLATE "LOGFILE/A,PRI=PRIORITY/K/N,SAFE=SAFE_SAVES/S,NOSER/S,NCQ=NOCLIENTQUIT/S"
#define OPT_LOG  0   /* logfile for results - file, con:, ser:, whatever */
#define OPT_PRI  1   /* priority at which to run                         */
#define OPT_SAFE 2   /* Open/Close/Seek logfile for each write           */
#define OPT_NSER 3   /* turn off kprintf's to ser:                       */
#define OPT_NCQ  4   /* whether to tell client to quit                   */
#define NUM_OPTS 5

#define FUDGE 256   /* FUDGE is the factor involved in how much memory you need
                       in a memchunk to allocate X bytes.  So, to get an X-byte allocation,
                       we look for X+FUDGE free bytes in a memchunk. */

#define GRANULARITY 256  /* Granularity is how close to &MemChunk 
                            we want to re-try an allocation */

#define ROM_LOCATION 0x00F80000    /* default ROM location; other may be KICKIT */
#define KICKIT_LOCATION 0x00200000
#define PROGNAME SERVER_PORT
#define DEFAULT_PRIORITY 4         /* default priority for this task */

/* DEBUG DEFINES and declarations ----------------------------------------- */
#ifdef DEBUG
#define D(x) (x);
#else
#define D(x) ;
#endif

#ifdef DEBUG2
/* For debugging allocRange() routine */
#define D2(x) (x);
struct dbmem {
    struct MemHeader mmh;
    struct MemChunk mmc;
    ULONG mcLoc;
    ULONG attempt;
    SHORT minfo;
};
struct dbmem dbm;
#define BOFFO 0x000B0FF0
#else
#define D2(x) ;
#endif

/* Nuke the SAS Break-C handling ------------------------------------------- */

/* Get rid of SAS Break handling.  We have our own exit routine & method. */
/* (from jwldefs.h)  */
NUKE_SAS_BREAK
REALLY_NUKE_IT


/* GLOBALS -----------------------------------------------------------------*/
struct RDArgs *rdargs = NULL;
struct Library *ExpansionBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct MsgPort *servPort = NULL;
BPTR   fh = NULL;
SHORT  rc = RETURN_FAIL;
struct MinList *listPtr=NULL;     /* for emergency shutdown, removal of allocated list */
STRPTR logFileName = NULL;        /* kludge for logMsg() */
ULONG romStart = ROM_LOCATION;    /* where ROM starts                   */
BOOL   safe_saves = FALSE;        /* Open, Seek, Close the logfile on each write */
BOOL   quiet_ser = FALSE;
BOOL   clientQuit = CMD_QUIT;
extern struct ExecBase *SysBase;



/* Function protos ----------------------------------------------------- */
SHORT getNumOfBoards(VOID);
VOID GoodBye(VOID);
VOID printBoardInfo(BPTR fileHandle, struct boardInfo *bi);  /* for debugging */
VOID stuffBoards(struct boardInfo *bi);
VOID findMBRam(struct boardInfo *bi);
VOID afprintf(STRPTR fmt, LONG argv, ...);
VOID aprintf(STRPTR fmt, LONG argv, ...);
VOID *allocRange(ULONG hi, ULONG lo, ULONG mem_size);
VOID delMemItem (struct MinList *l, VOID *ptr);
VOID addMemItem (struct MinList *l, struct memItem *mi);
BOOL allocMoreRam(struct memRequest *mr, struct MinList *pool, struct boardInfo *b);
BOOL protections_ok(struct memItem *mi, struct memRequest *mr, BOOL change_prot);
/* VOID logMsg(STRPTR fmt, struct DateTime *dt, STRPTR client, LONG arg1, ...); */
VOID logMsg(STRPTR fmt, struct memRequest *mr, LONG arg1, ...);
struct memItem *matchRequest(struct memRequest *mr, struct MinList *pool, BOOL changeProt);
struct memItem *okToFree(struct memRequest *mr, struct MinList *l);
VOID unAccess(struct memItem *mi, struct memRequest *mr, struct MinList *l);
BOOL changeAccess(struct memItem *mi, struct memRequest *mr, struct MinList *l);
struct memItem *findMemPtr(struct memRequest *mr, struct MinList *l);
VOID dumpDBM(BOOL toScreen, BOOL toLog);
VOID bsprintf2(STRPTR dest, STRPTR fmt, LONG argv, ...);  /* asm */

/* ======================= MAIN ============================================== */
VOID main (int argc, UBYTE *argv[]) {
    LONG Opts[NUM_OPTS];                 /* for readargs()                     */
    struct boardInfo boards[MAX_BOARDS]; /* server.h, for expansion board info */
    struct DateTime dt;                  /* datetime struct for real string    */
    UBYTE dateFmtName[LEN_DATSTRING];    /* DD-MMM-YY str, dos/datetime.h      */
    UBYTE timeFmtName[LEN_DATSTRING];    /* Time of day str, dos/datetime.h    */
    struct memRequest *mr = NULL;        /* ptr to memreq, used variously      */
    struct memItem *mi;                  /* ptr to MemItem, used variously     */
    struct MinList pool;                 /* list of allocated RAM              */
    int i;                               /* index counter                      */
    LONG pri = DEFAULT_PRIORITY;         /* task priority                      */


    EXIT_IF_WB();              /* jwldefs.h */
    EXIT_IF_ANCIENT();         /* jwldefs.h */

    /* incidentally, we most likely need dos.library V37 as well... */    
    if (!(ExpansionBase = OpenLibrary(EXPANSIONNAME, 37))) {
        PutStr("Couldn't open expansion library!\n");
        GoodBye();
    }
    if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 36))) {
        PutStr("Couldn't open Intuition!\n");
        GoodBye();
    }
    if (!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 36))) {
        PutStr("Couldn't open Graphics.library!\n");
        GoodBye();
    }

    /* Go get a message port for this MemServer */    
    Forbid();
    servPort = FindPort(SERVER_PORT);
    if (servPort) {
        Permit();
        PutStr("Already running MemServer; goodbye!\n");
        servPort = NULL;  /* otherwise we DeletePort it in GoodBye()... */
        GoodBye();
    }
    else {
        /* uses amiga.lib's CreatePort() rather than exec/CreateMsgPort() */
        servPort = (struct MsgPort *)CreatePort(SERVER_PORT, PORT_PRIORITY);
    }
    Permit();
    if (!servPort) {
        PutStr("Couldn't create server message port!\n");
        GoodBye();
    }

    memset(Opts,0,sizeof(Opts));

    if (!(rdargs = ReadArgs(TEMPLATE,Opts,NULL))) {
        PrintFault(IoErr(),PROGNAME);
        PutStr("You must supply a log file name!\n");
        GoodBye();
    }

    /* NB: Assumes that OPT_LOG is an /A template item!! */
    logFileName = (UBYTE *)Opts[OPT_LOG];
    if (!(fh = Open((UBYTE *)Opts[OPT_LOG],MODE_NEWFILE))) {
        PrintFault(IoErr(),PROGNAME);
        GoodBye();
    }
    else {
        FPuts(fh, "Server initializing...\n\n");
    }

    if (Opts[OPT_SAFE])
        safe_saves = TRUE;

    if (Opts[OPT_NSER])
        quiet_ser = TRUE;

    if (Opts[OPT_NCQ])
        clientQuit = CMD_CONTINUE;

    /* Go get date and time information */
    memset(&dt,0,sizeof(dt));
    memset(dateFmtName,0,sizeof(dateFmtName));
    memset(timeFmtName,0,sizeof(timeFmtName));

    dt.dat_Format = FORMAT_DOS;
    DateStamp(&(dt.dat_Stamp));
    dt.dat_StrDate = dateFmtName;
    dt.dat_StrTime = timeFmtName;
    DateToStr(&dt);


    /* kludge to support reads from Kickit machine ROMs */
    if (((ULONG)(SysBase->LibNode.lib_IdString) & 0xFFFF0000) == KICKIT_LOCATION)
        romStart = KICKIT_LOCATION;


    /* Initialize Boards array */
    for (i = 0; i < MAX_BOARDS; i++)
        boards[i].size = 0UL;

    /* Initialize Memory List */
    pool.mlh_Head = NULL;
    pool.mlh_Tail = NULL;
    pool.mlh_TailPred = NULL;
    listPtr = &pool;

    /* Record our presence for posterity */    
    aprintf("%s started at %s on %s\n",(LONG)PROGNAME,timeFmtName,dateFmtName); 
    afprintf("%s started at %s on %s\n",(LONG)PROGNAME,timeFmtName,dateFmtName); 

    /* we're OK until further notice */
    rc = RETURN_OK;

    /* Set our task's priority... we want to be higher than clients/WB/whatever */
    if (Opts[OPT_PRI]) {
        pri = *(LONG *)Opts[OPT_PRI];
        if (abs(pri) > 5) {
            aprintf("Invalid priority '%ld', setting to '%ld'", pri, DEFAULT_PRIORITY);
            pri = DEFAULT_PRIORITY;
        }
    }
    SetTaskPri(FindTask(NULL), pri);

    /* go get info about memory boards */
    stuffBoards(boards);
    findMBRam(boards);
    printBoardInfo(fh, boards);

    /* If the user has requested safe_saves (OPT_SAFE), we
       now close the fh so that we can write safely and hopefully avoid munging
       our logfile if we crash later on.
     */
    if (safe_saves) {
        Close(fh);
        fh = NULL;
    }


    /* MAIN EVENT LOOP 같같같같같같같같같같같같같같같같같� */
    while (TRUE) {
        WaitPort(servPort);
        /* process message */
        while ((mr = (struct memRequest *)GetMsg(servPort)) != NULL) {
            switch (mr->cmd) {
                case CMD_MEMREQ:   /* client wants memory */
                    D(aprintf("Got CMD_MEMREQ from %s\n",(LONG)mr->clientName));
                    D(Delay(20));
                    /* find permissible memory block, if possible */
                    if ((mi = matchRequest(mr, &pool, TRUE)) != NULL) {
                        mr->memPtr = mi->memPtr;
                        mr->status = STATUS_DONE;
                        mr->rc  = RETURN_OK;
                        mr->rc2 = MEM_OK;
                        logMsg("Gained access to 0x%08lx\n", mr,
                                (ULONG)(mr->memPtr));
                    }
                    else {
                        /* if old pool was requested and failed, fail request */
                        if (mr->memPtr) {
                            mr->status = STATUS_NOT_DONE;
                            mr->rc  = RETURN_ERROR;
                            /* mr->rc2 = set by matchMemRequest() */
                            logMsg("Existing memory at 0x%08lx DENIED\n",
                                    mr,
                                    (ULONG)(mr->memPtr));
                        }
                        else {
                        /* else, allocate more ram for the client */
                            /* allocMoreRam deals with updating the mr->status, etc, too */
                            if (allocMoreRam(mr, &pool, boards)) {
                                logMsg("Allocated %ld bytes at 0x%08lx (bd %ld)\n",
                                    mr,
                                    (ULONG)(mr->memSize),
                                    mr->memPtr,
                                    mr->board);
                            }
                            else {  /* allocation failed */
                                logMsg("Failed alloc size %ld (board/loc %ld)\n",
                                    mr,
                                    (ULONG)(mr->memSize),
                                    (LONG)(mr->board));
                                D2(dumpDBM(TRUE,TRUE));
                            }
                        }
                    }
                    /* ReplyMsg at end of switch */
                    break;

                case CMD_LOGIT:    /* client wants to write to log */
                    D(aprintf("Got CMD_LOGIT from %s\n",(LONG)mr->clientName));
                    D(Delay(20));
                    logMsg("Sent Message:\n  %s\n\n", mr, (LONG)(mr->log1));
                    if (mr->status == STATUS_SEVERE_ERR) {
                        mr->status =  STATUS_CHECK_CMD;
                        mr->cmd    = clientQuit;   /* cmd_quit or cmd_continue, cmdline option */
                    }
                    else {
                        mr->status = STATUS_DONE;
                    }
                    mr->rc  = RETURN_OK;
                    mr->rc2 = 0;
                    break;

                case CMD_FREE_MEM:  /* client wants to free memory */
                    D(aprintf("Got CMD_FREEMEM from %s\n",(LONG)mr->clientName));
                    D(Delay(20));
                    if ((mi = okToFree(mr, &pool)) != NULL) {
                        logMsg("Freed RAM at %08lx, size %ld\n",mr,
                            (ULONG)(mr->memPtr),mi->byteSize);
                        delMemItem(&pool, mi->memPtr); 
                        mr->status = STATUS_DONE;
                        mr->rc  = RETURN_OK;
                        mr->rc2 = MEM_FREED;
                    }
                    else {
                        logMsg("RAM not freed: %08lx, size %ld -- IN USE\n",
                                mr,
                                (ULONG)(mr->memPtr), mr->memSize);
                        mr->status = STATUS_NOT_DONE;
                        mr->rc  = RETURN_ERROR;
                        mr->rc2 = MEM_LOCKED;
                    }
                    break;

                /* CMD_LOCKIT and CMD_UNLOCKIT are now obsolete, replaced by: */
                case CMD_NEW_ACCESS:
                    D(aprintf("Got CMD_NEW_ACCESS from %s\n",(LONG)mr->clientName));
                    D(Delay(50));
                    if ((mi = findMemPtr(mr, &pool)) != NULL) {
                        if (changeAccess(mi, mr, &pool)) {
                            /* change worked */
                            mr->rc = RETURN_OK;
                            mr->rc2 = MEM_OK;
                            mr->status = STATUS_DONE;
                            logMsg("Changed access from %ld to %ld on 0x%08lx\n",
                                mr,
                                (LONG)(mr->access),
                                (LONG)(mr->newAccess),
                                mr->memPtr); 
                            mr->access = mr->newAccess;
                        }
                        else {
                            /* change failed */
                            mr->rc = RETURN_WARN;
                            mr->rc2 = MEM_LOCKED;  /* best guess */
                            mr->status = STATUS_NOT_DONE;
                            logMsg("Failed to changed access from %ld to %ld on 0x%08lx\n",
                                mr,
                                (LONG)(mr->access),
                                (LONG)(mr->newAccess),
                                mr->memPtr); 
                        }
                    }
                    else {
                        /* couldn't find referenced memory block */
                            mr->rc = RETURN_ERROR;
                            mr->rc2 = MEM_NOT_VALID;
                            mr->status = STATUS_NOT_DONE;
                            logMsg("Tried to change bogus RAM at 0x%08lx\n",
                                mr,
                                (ULONG)(mr->memPtr));
                    }
                    break;

                case CMD_ABEO:  /* client is exiting (Latin: ab + eo = 'I go away') */
                    /* NB:  NO MEMORY FREEING IS DONE! */
                    D(aprintf("Got CMD_ABEO from %s\n",(LONG)mr->clientName));
                    D(Delay(30));
                    logMsg("Exiting.\n", mr, 0L);
                    mr->rc  = RETURN_OK;
                    mr->rc2 = 0;
                    mr->status = STATUS_DONE;
                    break;

                case CMD_QUIT:
                    logMsg("sent QUIT command; server complying...\n",mr, 0L);
                    mr->rc  = RETURN_OK;
                    mr->rc2 = RC2_EXITING;
                    mr->status = STATUS_DONE;
                    ReplyMsg((struct Message *)mr);
                    GoodBye();
                    break;

                default:
                    logMsg("Unknown cmd 0x%08lx sent\n", mr, (LONG)mr->cmd);
                    mr->rc = RETURN_FAIL;
                    mr->rc2 = MEM_NOT_VALID;
                    mr->status = STATUS_NOT_DONE;
                    break;

            }  /* end switch */
            ReplyMsg((struct Message *)mr);
        }   /* end while */
    } /* end of main event loop (while TRUE) */

    GoodBye();
}

/* GoodBye ======================= clean exit routine ==================== */
VOID GoodBye(VOID) {
    struct memRequest *m;

    if (fh) {
        Close(fh);
        fh = NULL;
    }
    if (rdargs) {
        FreeArgs(rdargs);
        rdargs = NULL;
    }
    /* nuke entire memlist, protections or no.  This means that any clients
       which haven't quit are in DIRE TROUBLE if they try to use the ram... */
    if (listPtr) {
        if (listPtr->mlh_Head != NULL) {
            struct memItem *mi = NULL;
            struct MinNode *mn = NULL;
            for (mn = listPtr->mlh_Head; mn != NULL; mn = listPtr->mlh_Head) {
                mi = (struct memItem *)mn;
                delMemItem(listPtr, mi->memPtr);
            }
        }
    }
    if (servPort) {
        /* get rid of pending messages */
        while ((m = (struct memRequest *)GetMsg(servPort)) != NULL) {
            m->status = STATUS_EXIT;
            m->rc     = MEM_DENIED;
            m->rc2    = RC2_EXITING;
            ReplyMsg((struct Message *)m);
        }
        /* get rid of port, using amiga.lib DeletePort() */
        DeletePort(servPort);
        servPort = NULL;
    }
    if (GfxBase) {
        CloseLibrary((struct Library *)GfxBase);
        GfxBase = NULL;
    }
    if (IntuitionBase) {
        CloseLibrary((struct Library *)IntuitionBase);
        IntuitionBase = NULL;
    }
    if (ExpansionBase) {
        CloseLibrary(ExpansionBase);
        ExpansionBase = NULL;
    }
/*    memt2 = AvailMem(MEMF_ANY|MEMF_TOTAL); */
/*    Printf("Mem Start: %ld, End: %ld, Diff: %ld\n",memt1,memt2,(LONG)(memt1-memt2)); */
    exit(rc);
}

/* =========================== a small residentable printf ================ */
VOID aprintf(STRPTR fmt, LONG argv, ...) {
    VPrintf(fmt, &argv);
}

/* =================== a small fprintf; uses global filehandle "fh" ======= */
VOID afprintf(STRPTR fmt, LONG argv, ...) {
    VFPrintf(fh, fmt, &argv);
}

/* BoardInfo:
   16 slots = 1MB, 1 slot = 64KB.
 */

/* Print out BoardInfo =============================================== */
VOID printBoardInfo(BPTR fileHandle, struct boardInfo *bi) {
    SHORT i;

    for (i = 0; i < MAX_BOARDS; i++) {
        if (bi[i].size != 0) {
            aprintf("For Memory Board %ld:\n",(LONG)i+1L);
            aprintf("    Start = 0x%08lx\n",bi[i].start);
            aprintf("    End   = 0x%08lx\n",bi[i].end);
            aprintf("    Size  = 0x%08lx  (%lu bytes)\n",bi[i].size, bi[i].size);
        }
    }

    if (fileHandle != NULL) {
        for (i = 0; i < MAX_BOARDS; i++) {
            if (bi[i].size != 0) {
                afprintf("For Memory Board %ld:\n",(LONG)i+1L);
                afprintf("    Start = 0x%08lx\n",bi[i].start);
                afprintf("    End   = 0x%08lx\n",bi[i].end);
                afprintf("    Size  = 0x%08lx  (%lu bytes)\n\n",bi[i].size, bi[i].size);
            }
        }
    }
}

/* stuffBoards ======================================================= */
VOID stuffBoards(struct boardInfo *bi) {
    struct ConfigDev *cdev = NULL;
    LONG i = 0;

    while(cdev=FindConfigDev(cdev,-1L,-1L)) {
        if ((cdev->cd_Rom.er_Type) & ERTF_MEMLIST) {
            bi[i].start = (ULONG)(cdev->cd_BoardAddr);
            bi[i].size  = (ULONG)(cdev->cd_BoardSize);
            bi[i].end   = (ULONG)(((bi[i].size)-1)+(LONG)(cdev->cd_BoardAddr));
            i++;
        }
    }
}

/* findMBRam ==================================================================
   walks system memlist to find Fast RAM that isn't on the expansion bus       
   NB: assumes that expansion RAM is contiguous, or at least that you won't
   find a block of Fast Ram in the midst of the expansion RAM.
 */
VOID findMBRam(struct boardInfo *bi) {
    ULONG minLoc=0, maxLoc=0, mhLo, mhHi;
    SHORT i;
    struct MemHeader *mh;
    struct Node *n;

    /* find the minimum and maximum expansion board info */
    for (i = 0; i < MOTHER_BOARD; i++) {
        if (bi[i].size != 0) {
            if (minLoc > 0)
                minLoc = min(bi[i].start,minLoc);
            else
                minLoc = bi[i].start;
            maxLoc = max(bi[i].end, maxLoc);
        }
    }
    /* find some Fast RAM */
    Forbid();
        for (n = SysBase->MemList.lh_Head; n != NULL; n = n->ln_Succ) {
            mh = (struct MemHeader *)n;
            if (mh->mh_Attributes & MEMF_FAST) {
                mhLo = (ULONG)(mh->mh_Lower);
                mhHi = (ULONG)(mh->mh_Upper);
                if (mhLo < minLoc) {
                    bi[MOTHER_BOARD].start = mhLo;
                    bi[MOTHER_BOARD].end = min(mhHi - 1L, minLoc);
                    Permit();
                    bi[MOTHER_BOARD].size = bi[MOTHER_BOARD].end - bi[MOTHER_BOARD].start;
                    return;
                }
                if (mhLo >= maxLoc) {
                    bi[MOTHER_BOARD].start = mhLo;
                    bi[MOTHER_BOARD].end = mhHi - 1L;
                    Permit();
                    bi[MOTHER_BOARD].size = bi[MOTHER_BOARD].end - bi[MOTHER_BOARD].start;
                    return;
                }
                if (mhHi > maxLoc) {
                    bi[MOTHER_BOARD].start = max(maxLoc, mhLo);
                    bi[MOTHER_BOARD].end = mhHi - 1L;
                    Permit();
                    bi[MOTHER_BOARD].size = bi[MOTHER_BOARD].end - bi[MOTHER_BOARD].start;
                    return;
                }
            }
        }
    Permit();
    /* if we didn't find anything, bi[MOTHER_BOARD].size is still 0 ... */
    return;
}

/* allocRange ========================================================================
   allocRange(ULONG hi, ULONG lo, ULONG size) - return ptr to requested
   memory, or 0 if AllocAbs() failed.
   NOTE:  THIS CODE DOES A Forbid() and Permit() and ILLEGALLY walks
   the system memory lists.  If you've a better way to do it, let me know.
 */
VOID *allocRange(ULONG hi, ULONG lo, ULONG mem_size) {
    struct ExecBase *eb = NULL;   /* SysBase; */
    struct MemHeader *mh, *first;
    struct MemChunk *mc;
    VOID *myMem = NULL;
    ULONG allocLocation = 0UL;
    ULONG ibaseLock = NULL;
    ULONG memc_addr, memc_size;
    BOOL broke_forbid = FALSE;
    struct ViewPort *vp = NULL;
    struct Screen *scr = NULL;

    D(aprintf("allocRange:  hi=0x%08lx, lo=0x%08lx, size=0x%08lx\n",hi,lo,mem_size));

    /* Opening ExecBase is unnecessary, but seems to work better -- fewer
       retries are necessary...
     */
    eb = (struct ExecBase *)OpenLibrary("exec.library",0);
    if (!eb)
        return(NULL);

    /* To be anywhere near safe, we must
       WaitBOVP(struct ViewPort *frontMostScreen'sViewPort)
       so as not to interfere with the system VBLANK, which would be
       disasterous. 
       
       Well, this was for a Disable()/Enable() pair, which proved
       to be unnecessary.  But the code works now, and I don't want
       to take it out and re-test right now.
     */

    ibaseLock = LockIBase(0UL);
    scr = IntuitionBase->FirstScreen;
    vp = &(scr->ViewPort);
    WaitBOVP(vp);
    UnlockIBase(ibaseLock);

    /* A Few Reference Comments:  */
    /* eb->MemList is a struct List:
                        struct Node *lh_Head;
                        struct Node *lh_Tail;
                        struct Node *lh_TailPred;
                        UBYTE lh_Type;
                        UBYTE l_pad;
    */

    /* lh_Head should be a MemHeader:
                struct Node mh_Node;
                UWORD mh_Attributes;
                struct MemChunk *mh_First;
                APTR mh_Lower;
                APTR mh_Upper;
                ULONG mh_Free;
     */

    D2(dbm.attempt = 0);
    D2(dbm.mcLoc = (ULONG)BOFFO);

    Forbid();      /* Forbid ON ------------------- */

    first = (struct MemHeader *)(eb->MemList.lh_Head);
    mh = first;
    for (mh = first; mh != NULL; mh = (struct MemHeader *)mh->mh_Node.ln_Succ) {
        if ((min((ULONG)(mh->mh_Upper), hi) >= max((ULONG)(mh->mh_Lower), lo))
                && (mh->mh_Free >= mem_size)) {

            for (mc = mh->mh_First; mc != NULL; mc = mc->mc_Next) {
                memc_addr = (ULONG)&mc;
                memc_size = (ULONG)(mc->mc_Bytes);
                if (broke_forbid) {
                    CloseLibrary((struct Library *)eb);
                    D(Printf("Failed:   %8ld                           at 0x%08lx\n",mem_size,allocLocation));
                    D(Printf("MemChunk:         %ld bytes at 0x%08lx\n",memc_size,memc_addr));
                    return(NULL);
                }
                memc_addr = (ULONG)mc;    /* WAS: &mc */
                memc_size = (ULONG)(mc->mc_Bytes);
                /* Save starting memory location and Pre-round it */
                allocLocation = (ULONG)(max(memc_addr, lo));
                allocLocation += (allocLocation & (ULONG)MEM_BLOCKMASK);
                /* mem1 = allocLocation;  / debug */
                while (!myMem) {
                    if ((memc_addr + memc_size) >= (allocLocation + mem_size + FUDGE)) {
                        myMem = AllocAbs(mem_size,(APTR)allocLocation);
                        Permit();
                        broke_forbid = TRUE;
                        if (myMem) {
                            /* growl_growl = mem_size; / debug */
                            if (!broke_forbid) {
                                Permit();
                            }
                            CloseLibrary((struct Library *)eb);
                            return((VOID *)myMem);
                        }
                        else {
/*                            D(Printf("Failed:   %8ld                           at 0x%08lx\n",mem_size,allocLocation));
                            D(Printf("MemChunk:         %ld bytes at 0x%08lx\n",memc_size,memc_addr));
*/
                            allocLocation += (LONG)GRANULARITY;
                            D2(dbm.mmh = *mh);
                            D2(dbm.mmc = *mc);
                            D2(dbm.mcLoc = memc_addr);
                            D2(dbm.attempt++);
                        }
                    }
                    else
                        break;
                }
            }
        }
    }

    if (!broke_forbid) {
        Permit();
    }

    CloseLibrary((struct Library *)eb);
    return(NULL);
}

/* MinList: 3 MinNodes: *mlh_Head, *mlh_Tail, *mlh_TailPred */
/* MinNode: 2 MinNodes: *mln_Succ, *mln_Pred                */

/* addMemItem ======================================================================== */
/* add memory request to head of internal memory list (LIFO) */
/* Works with empty list, without care as to where in list   */
/* Note that TailPred is never used                          */
VOID addMemItem (struct MinList *l, struct memItem *mi) {
    struct MinNode *mn, *old;

    D(PutStr("Entering addMemItem\n"));
    D(Delay(30));
    mn = (struct MinNode *)mi;

    if (l->mlh_Head == NULL) {
        l->mlh_Head = mn;
        l->mlh_Tail = mn;
    }
    else {
        old = l->mlh_Head;
        l->mlh_Head = mn;
        mn->mln_Succ = old;
        old->mln_Pred = mn;
    }
    D(PutStr("Added a memItem\n"));
}


/* delMemItem ============================================================== */
/* Takes memory list and Frees the given block of RAM, fixing list */
/* Note that TailPred is never used                                */
/* Arguments:  l = memory list, ptr = block of ram to free         */
VOID delMemItem (struct MinList *l, VOID *ptr) {
    struct MinNode *mn, *tmp;
    struct memItem *mi;

    D(PutStr("Entering delMemItem\n"));
    D(Delay(50));
    for (mn = l->mlh_Head; mn != NULL; mn = mn->mln_Succ) {
        mi = (struct memItem *)mn;
        if (mi->memPtr == ptr) {
            /* found block to free */
            
            /* Deallocate RAM as per instructions */
            /* check for ptr validity; may be NULL in debugging */
            if (ptr != NULL) {
                if (mi->deAlloc == DEALLOC_FREEMEM) {
                    FreeMem(ptr, mi->byteSize);
                }
                else if (mi->deAlloc != DEALLOC_DONT) {
                    FreeVec(ptr);
                }
            }

            /* MinList: 3 MinNodes: *mlh_Head, *mlh_Tail, *mlh_TailPred */
            /* MinNode: 2 MinNodes: *mln_Succ, *mln_Pred                */

            /* Patch memlist */
            if (mn == l->mlh_Head) {
                /* nuked first node in list... need new */
                tmp = mn->mln_Succ;
                l->mlh_Head = tmp;
                if (tmp)
                    tmp->mln_Pred = NULL;
                if (mn == l->mlh_Tail) {
                    /* nuked node was both head and tail */
                    l->mlh_Tail = NULL;
                }
            }
            else if (mn == l->mlh_Tail) {
                /* assumes we have valid head, tail, tailpred */
                /* nuked last node in list; need new listEnd */
                /* But what if tail==head==tailpred or some combo? */
                /* if tail==head, we found it above.  tail can't = tailpred. */
                tmp = mn->mln_Pred;
                l->mlh_Tail = tmp;
                tmp->mln_Succ = NULL;
            }
            else {
                /* nuked node in midst of list */
                tmp = mn->mln_Pred;
                tmp->mln_Succ = mn->mln_Succ;
                mn->mln_Succ->mln_Pred = tmp;
            }
            /* Done patching lists, now free it */
            FreeVec((VOID *)mn);
            return;  /* we're done */
        }
    }
    /* quietly return if memory not found (client screwed up) */
}


/* matchRequest =================================================================== */
/* tries to match up memory request with existing item in pool.  Returns NULL if    */
/* none is available for the access mode requested.  Client should wait and try     */
/* again later if subsequent allocation attempts fail.                              */
/* Select by:
        board(mr) / location(i)  (if ptr, used as a substitute)
        memptr (null if by board)
 */
struct memItem *matchRequest(struct memRequest *mr, struct MinList *pool, BOOL changeProt) {
    struct MinNode *mn;
    struct memItem *mi;
    LONG tmpboard;

    D(PutStr("Entering matchRequest\n"));
    D(Delay(10));
    /* incoming request is [1..x], storage is [0..x-1] */
    if ((mr->board <= LOC_MOTHERBOARD) && (mr->board > 0))
        tmpboard = mr->board - 1;
    else
        tmpboard = mr->board;  /* LOC_CHIP, FAST, ANY */

    for (mn = pool->mlh_Head; mn != NULL; mn = mn->mln_Succ) {
        mi = (struct memItem *)mn;
        if (mr->memPtr) {  /* wants an old pointer */
            if (mi->memPtr == mr->memPtr) {
                /* found requested block, check access */
                if (protections_ok(mi,mr,changeProt)) {
                    return(mi);
                }
            }
            /* else keep looking */
        }
        else {   /* wants a memory board or region */
            if ((tmpboard == mi->location) && (mi->byteSize >= mr->memSize)) {
                /* found a region */
                if (protections_ok(mi,mr,changeProt))
                    return(mi);
            }
            /* else keep looking */
        }
    }
    return(NULL);
}

/* allocMoreRam ==================================================================== */
/* tries to allocate more ram matching request.  Returns TRUE for success, FALSE     */
/* otherwise.  Memory pointer and means of deallocation are stored in a newly        */
/* allocated memItem, with the memory pointer copied back to the request for         */
/* the eventual reply back to the client.                                            */
/* Foo Bar:  right now, doesn't know how to deal with motherboard ram specifically   */
BOOL allocMoreRam(struct memRequest *mr, struct MinList *pool, struct boardInfo *b) {
    struct memItem *new = NULL;
    SHORT boardnum = -1;

    D(PutStr("Entering allocMoreRam\n"));
    D(Delay(30));
    /* initialize memPtr so we can check the allocation later */
    mr->memPtr = NULL;
    mr->rc  = RETURN_OK;
    mr->rc2 = 0;  /* checked and set below */

    /* get new node to store allocation information */
    new = (struct memItem *)AllocVec(sizeof(struct memItem),MEMF_CLEAR);
    if (!new) {
        /* fail the RAM request, we obviously have none */
        mr->status = STATUS_DONE;
        mr->rc  = RETURN_FAIL;
        mr->rc2 = MEM_NOT_AVAIL;
        return(FALSE);
    }
    else {  /* OK to go get new RAM */
            /* remember to indicate means of freeing it */
        D(aprintf("allocMoreRam:  Board %ld\n",mr->board));
        switch (mr->board) {
            case LOC_CHIP:
                mr->memPtr = AllocVec(mr->memSize, MEMF_CHIP|MEMF_CLEAR);
                new->deAlloc = DEALLOC_FREEVEC;
                break;
            case LOC_FAST:
                mr->memPtr = AllocVec(mr->memSize, MEMF_FAST|MEMF_CLEAR);
                new->deAlloc = DEALLOC_FREEVEC;
                break;
            case LOC_ANY:
                mr->memPtr = AllocVec(mr->memSize, MEMF_PUBLIC|MEMF_CLEAR);
                new->deAlloc = DEALLOC_FREEVEC;
            case LOC_ROM:  /* kludgy */
                if (mr->memSize <= ROM_SIZE) {
                    if ((mr->access == OK_READ) || (mr->access == NC_READ)) {
                        mr->memPtr   = (VOID *)romStart;
                        new->deAlloc = DEALLOC_DONT;
                    }
                    else
                        mr->rc2 = MEM_READ_ONLY;
                }
                /* else we don't fill in mr->memPtr and thus look like a failed allocation */
                break;
            case LOC_MOTHERBOARD:
            default:
                boardnum = mr->board;  /* NB: client: board[1..n]; server: board[0..n-1] */
                /* cpu boards are always zero */
                if ((boardnum > 0) && (boardnum <= LOC_MOTHERBOARD))
                    boardnum--;
                /* quick paranoid validity check */
                if ((boardnum < MAX_BOARDS) && (b[boardnum].size != 0)) {
                    D(aprintf("Trying to allocRange with Board #%ld\n",(LONG)boardnum));
                    D(Delay(50));
                    mr->memPtr = allocRange(b[boardnum].end, b[boardnum].start, mr->memSize);
                    new->deAlloc = DEALLOC_FREEMEM;
                }
                else {
                    D(PutStr("allocMoreRam:  Invalid RAM request\n"));
                    D(aprintf("Board: %ld, Max: %ld, BSize: %ld\n", (LONG)boardnum,
                              MAX_BOARDS, b[boardnum].size));
                    mr->rc2 = MEM_NOT_VALID;
                }
                break;
        }

        /* Check success of allocation and process accordingly */
        if (mr->memPtr) {
            D(PutStr("allocMoreRam:  Got RAM!\n"));
            /* set up the new memItem */
            new->memPtr = mr->memPtr;
            if (mr->board != LOC_ROM) {
                new->byteSize  = mr->memSize;
                new->permitted = ~0;  /* everything is permissible to start */
            }
            else {
                new->byteSize  = ROM_SIZE;           /* it has only one size :-)  */
                new->permitted = READ_OK|NCREAD_OK;  /* no write/ncwrite to ROM!  */
            }

            /* NB: intelligent location storage, corresponds with boards[] if applicable. */
            /* boardnum was set up under default, or is -1. */
            if (boardnum >= 0)
                new->location = boardnum;
            else
                new->location = mr->board;
            new->owner = (ULONG)(mr->clientName);
            
            /* Add memItem to list/pool */
            addMemItem(pool,new);

            /* Set protection bits according to request in mr */
            protections_ok(new,mr,TRUE);

            /* Prepare Success in memRequest */            
            mr->status = STATUS_DONE;
            mr->rc = RETURN_OK;
            mr->rc2 = MEM_OK;            
            /* Done.  Return. */
            return(TRUE);
        }
        else {  /* allocation failed; FreeVec(new) & fail the request */
            D(PutStr("allocMoreRam:  failed ram request!\n"));
            FreeVec(new);
            mr->status = STATUS_NOT_DONE;
            mr->rc  = RETURN_ERROR;
            if (!(mr->rc2))                     /* may be mem_not_valid already */
                mr->rc2 = MEM_NOT_AVAIL;
        }

    }

    return(FALSE);
}

/* protections_ok ================================================================== */
/* find out if requested access matches existing access for a given memItem          */
/* if change_prot==TRUE, change the protections to reflect new values                */
/* memitem is busy if:
      opencount > 0  or   / read  or ncwrite /
      locked == TRUE      / write /
      readcount >0
      writecount >0
   *Any* access bumps the opencount;
   "Locks" are indicated by readcount and writecount.
   rc2 is set to MEM_LOCKED if we get ACCESS_DENIED
   This routine changes protections if change_prot is set, otherwise it'll
   just tell you if access is permitted.

   This routine is RECURSIVE if you ask for a read lock on ROM, and will change
   your request for an exclusive lock to a "no care" lock.  ROM ain't gonna be
   changing while your reading from it, or any other time!
*/
BOOL protections_ok(struct memItem *mi, struct memRequest *mr, BOOL change_prot) {

    D(PutStr("Entering protections_ok\n"));
    D(Delay(20));
    /* yes, OK_READ is different from READ_OK, and so on */
    switch (mr->access) {
        case OK_READ:
            if (mr->board == LOC_ROM) {
                /* you shouldn't ask for an exclusive read lock on ROM! */
                mr->access = NC_READ;
                if (protections_ok(mi,mr,change_prot))
                    return(TRUE);
                else
                    return(FALSE);
            }
            if (mi->permitted & READ_OK) {
                if (change_prot) {
                    mi->permitted &= ~((LONG)WRITE_OK);   /* turn off writes   */
                    mi->permitted &= ~((LONG)NCWRITE_OK); /* turn off ncwrites */
                    mi->openCount++;
                    mi->readCount++;  /* so we can tell when WRITEs are OK again */
                }
                if (mi->memPtr)  /* paranoia */
                    return(TRUE);
                else {
                    mr->rc2 = MEM_NOT_AVAIL;
                    return(FALSE);
                }
            }
            else {  /* reads are verboten */
                mr->rc2 = MEM_LOCKED;
                return(FALSE);
            }
            break;
        case OK_WRITE:
            if (mi->permitted & WRITE_OK) {
                if (change_prot) {
                    mi->locked = TRUE;
                    mi->lock_owner = (ULONG)(mr->clientName);
                    mi->permitted &= ~((LONG)WRITE_OK);     /* turn off writes   */
                    mi->permitted &= ~((LONG)NCWRITE_OK);   /* turn off ncwrites */
                    mi->permitted &= ~((LONG)READ_OK);      /* turn off reads    */
                    mi->openCount++;
                    mi->writeCount++;
                }
                if (mi->memPtr)  /* paranoia */
                    return(TRUE);
                else {
                    mr->rc2 = MEM_NOT_AVAIL;
                    return(FALSE);
                }
            }
            else {
                mr->rc2 = MEM_LOCKED;
                return(FALSE);
            }
            break;
        case NC_READ:  /* the least picky of the lot */
            if (mi->permitted & NCREAD_OK) {
                if (change_prot) {
                    mi->openCount++;   /* so we don't deallocate it while reading */
                    /* no read lock for a nc_read */
                }
                if (mi->memPtr)  /* paranoia */
                    return(TRUE);
                else {
                    mr->rc2 = MEM_NOT_AVAIL;
                    return(FALSE);
                }
            }
            else {
                mr->rc2 = MEM_LOCKED;
                return(FALSE);
            }
            break;
        case NC_WRITE:
            if (mi->permitted & NCWRITE_OK) {
                if (change_prot) {
                    mi->permitted &= ~((LONG)WRITE_OK);     /* turn off writes   */
                    mi->permitted &= ~((LONG)READ_OK);      /* turn off reads    */
                    mi->openCount++;
                    mi->writeCount++;  /* so we can tell when to set OK for locks */
                }
                if (mi->memPtr)  /* paranoia */
                    return(TRUE);
                else {
                    mr->rc2 = MEM_NOT_AVAIL;
                    return(FALSE);
                }
            }
            else {
                mr->rc2 = MEM_LOCKED;
                return(FALSE);
            }
            break;
        default:
            mr->rc2 = MEM_NOT_VALID;  /* well, INVALID_ACCESS_MODE, were it defined */
            return(FALSE);
            break;
    }
    mr->rc2 = MEM_NOT_VALID;  /* well, *something's* wrong if we got here */
    return(FALSE);
}

/* logMsg =========================================================================
   Purpose:  put a message into the logfile, printf-style
   Opens and closes the file each time in hopes of avoiding corruption
   Please note that HERE is where CMD_QUIT may be given to the client, as the
   client may send STATUS_CLIENT_ERR along with the CMD_LOGIT message.  If CMD_QUIT
   is sent, STATUS_CHECK_CMD will be set in mr->status rather than STATUS_DONE.
 */
VOID logMsg(STRPTR fmt, struct memRequest *mr, LONG arg1, ...) {
    UBYTE dateStr[LEN_DATSTRING];    /* DD-MMM-YY str, dos/datetime.h      */
    UBYTE timeStr[LEN_DATSTRING];    /* Time of day str, dos/datetime.h    */
    UBYTE *tmpDate, *tmpTime;
    struct DateTime *dt; 
    STRPTR client;
    UBYTE  hasErr;

    client = mr->clientName;
    dt = mr->dt;
    D(PutStr("Entering LogMessage\n"));
    if (safe_saves) {
        if (!(fh = Open(logFileName, MODE_OLDFILE))) {
            PrintFault(IoErr(),PROGNAME);
            PutStr("Server Error!  Can't write to LogFile!!\n");
            return;
        }

        Seek(fh, 0L, OFFSET_END);
    }
    /* else the filehandle is already open and ready to use */

    /* preserve original date/time buffers */    
    tmpDate = dt->dat_StrDate;
    tmpTime = dt->dat_StrTime;

    dt->dat_StrDate = dateStr;
    dt->dat_StrTime = timeStr;

    /* convert existing date/time stamps to real strings */    
    DateToStr(dt);

    /* references global filehandle in both calls: */    
    afprintf("%s on %s (%s):  ", (LONG)timeStr, dateStr, client);
    VFPrintf(fh, fmt, &arg1);

    hasErr = ((mr->status == STATUS_CLIENT_ERR) || (mr->status == STATUS_SEVERE_ERR));

    if (!quiet_ser) {
        UBYTE buf[256];
        if (hasErr)
            KPrintF("%lc%s",BELL_CHAR,INVERSE_VIDEO_STRING);

        KPrintF("%s on %s (%s):  ", (LONG)timeStr, dateStr, client);

        bsprintf2(buf, fmt, (LONG)&arg1);
        KPutStr(buf);
        if (hasErr)
            KPutStr(NORMAL_VIDEO_STRING);
    }

    if (safe_saves) {
        Close(fh);
        fh = NULL;
    }

    /* restore datetime pointers */
    dt->dat_StrDate = tmpDate;
    dt->dat_StrTime = tmpTime;

    /* DEBUG ONLY */
    D(Delay(10));

}

/* okToFree =========================================================================
   Removes access to indicated RAM, and if it's OK to deallocate it then,
   returns the appropriate memItem.  Returns NULL if the memRequest didn't match
   an existing item (oops), or if it isn't ok to deallocate the RAM in question.
 */
struct memItem *okToFree(struct memRequest *mr, struct MinList *l) {
    struct memItem *mi = NULL;
    
    mi = findMemPtr(mr, l);
    if (!mi)
        return(NULL);

    /* remove current access (mr->access) */
    unAccess(mi, mr, l);

    if (mi->location == LOC_ROM)
        return(NULL);

    if ((mi->openCount == 0) && (mi->locked == FALSE) && 
        (mi->readCount == 0) && (mi->writeCount == 0)) {
        return(mi);
    }

    return(NULL);
}


/* unAccess ========================================================================
   Remove request to access given memItem.  Readjust protections.
   Access to be removed is given in mr->access.  Hopefully it's valid.
 */
VOID unAccess(struct memItem *mi, struct memRequest *mr, struct MinList *l) {

    switch (mr->access) {
        case OK_READ:
        /* Read locks prevent Write and NCWrite access. */
            if (mi->readCount > 0)
                mi->readCount--;
            mi->openCount--;
            if (mi->readCount == 0) {
                if ((mi->locked == FALSE) && (mr->board != LOC_ROM)) {
                    mi->permitted |= WRITE_OK;
                    mi->permitted |= NCWRITE_OK;
                }
            }
            break;

        case OK_WRITE:
        /* Write locks prevent Write, NCWrite, and Read access */
            if (mi->locked == TRUE) {
                mi->writeCount--;
                mi->openCount--;
                mi->locked = FALSE;
                mi->lock_owner = NULL;
            }
            if (mi->writeCount == 0) {
                mi->permitted |= READ_OK;
                mi->permitted |= WRITE_OK;
                mi->permitted |= NCWRITE_OK;
            }
            break;

        case NC_READ:
        /* NC_Read locks don't affect anything. */
            mi->openCount--;
            break;

        case NC_WRITE:
        /* NCWrite locks affect Read and Write access. */
            mi->openCount--;
            mi->writeCount--;
            if ((mi->writeCount == 0) && (mi->locked == FALSE)) {
                mi->permitted |= READ_OK;
                mi->permitted |= WRITE_OK;
                mi->permitted |= NCWRITE_OK;
            }
            break;

        default:
            PutStr("Error in Server unAccess() routine!\n");
            aprintf("Was fed 0x%08x for Access code.\n",mr->access);
            break;
    }
    /* Sanity check */
    if (mi->openCount < 0) {
        aprintf("Server error: %sCount less than zero!  Resetting to 0...\n",(LONG)"open");
        mi->openCount = 0;
    }
    if (mi->readCount < 0) {
        aprintf("Server error: %sCount less than zero!  Resetting to 0...\n",(LONG)"read");
        mi->readCount = 0;
    }
    if (mi->writeCount < 0) {
        aprintf("Server error: %sCount less than zero!  Resetting to 0...\n",(LONG)"write");
        mi->writeCount = 0;
    }

}

/* changeAccess =====================================================================
   Changes access to memitem from mr->access to mr->newAccess.  Fails if 
   requester didn't HAVE the original access.  If failure occurs after old 
   access is removed, cA() tries to restore old access.  In any case, returns
   TRUE for successful access change, FALSE otherwise.  If FALSE, original
   access is probably still there.

   Does not change access or newAccess fields; should be changed in Main() if
   it's appropriate.
*/

BOOL changeAccess(struct memItem *mi, struct memRequest *mr, struct MinList *l) {
    LONG oldAccess;
    BOOL hadAccess = FALSE;

    /* save old access in case we need to restore it */
    oldAccess = mr->access;

    /* This checking code is VERY iffy, and easy to fool. :-(  But
       it's the best that the current data structures allow. 
     */
    switch (mr->access) {
        case OK_READ:
            if (mi->readCount > 0)
                hadAccess = TRUE;  /* not definite, but no other way to tell */
            break;
        case OK_WRITE:
            if (mi->lock_owner == (ULONG)(mr->clientName))
                hadAccess = TRUE;
            break;
        case NC_READ:
            if (mi->openCount > 0)
                hadAccess = TRUE;
            break;
        case NC_WRITE:
            if (mi->writeCount > 0)
                hadAccess = TRUE;
            break;
        default:
                hadAccess = FALSE;
            break;
    }

    /* remove old access */
    unAccess(mi,mr,l);

    /* update mr->access */
    mr->access = mr->newAccess;

    /* try to obtain new access */
    if (protections_ok(mi, mr, TRUE)) {
        mr->access = oldAccess;   /* For accurate logMsgs, make change in main() */
        return(TRUE);
    }
    else {
        /* if new access isn't available, try to restore old access */
        mr->access = oldAccess;
        /* The following is RISKY.  We assume that we can get the old
           access back, because the status of mi can't have changed in
           the mean time.  If this fails for some weirdo reason, though,
           we are none the wiser; therein lies the risk.
         */
        protections_ok(mi,mr,TRUE);
        return(FALSE);
    }
}

/* findMemPtr ========================================================================
   Find the memItem which contains the requested memPtr.  No protections necessary
   or wanted.  Returns NULL if the memPtr can't be found.
*/
struct memItem *findMemPtr(struct memRequest *mr, struct MinList *l) {
    struct MinNode *mn = NULL;
    struct memItem *mi;

    for (mn = l->mlh_Head; mn != NULL; mn = mn->mln_Succ) {
        mi = (struct memItem *)mn;
        if (mr->memPtr == mi->memPtr)
            return(mi);
    }
    return(NULL);
}

/* dumpDBM =========================================================================
   (DEBUGGING WITH DEBUG2 DEFINED *ONLY* - counts on global struct dbmem dbm present.)
   Prints out contents of dbMem structure, in nauseating detail.
                         struct dbmem {
                             struct MemHeader mmh;
                             struct MemChunk mmc;
                             ULONG mcLoc;
                             ULONG attempt;
                             SHORT minfo;
                         };
*/

#ifdef DEBUG2
VOID dumpDBM(BOOL toScreen, BOOL toLog) {

    if (dbm.mcLoc == BOFFO)   /* Had nothing to report */
        return;

    if (toScreen) {
       PutStr ("MemHeader:\n");
       PutStr ("     Node:\n");
       aprintf("            Succ:  0x%08lx\n",(LONG)dbm.mmh.mh_Node.ln_Succ);
       aprintf("            Pred:  0x%08lx\n",(LONG)dbm.mmh.mh_Node.ln_Pred);
       aprintf("            Type:  0x%08lx\n",(LONG)dbm.mmh.mh_Node.ln_Type);
       aprintf("             Pri:  0x%08lx\n",(LONG)dbm.mmh.mh_Node.ln_Pri);
       aprintf("            Name:  0x%08lx\n",(LONG)dbm.mmh.mh_Node.ln_Name);
       aprintf("   Attrib:  0x%08lx\n", (LONG)dbm.mmh.mh_Attributes);
       aprintf(" MemChunk:  0x%08lx\n", (LONG)dbm.mmh.mh_First);
       aprintf("    Lower:  0x%08lx\n", (LONG)dbm.mmh.mh_Lower);
       aprintf("    Upper:  0x%08lx\n", (LONG)dbm.mmh.mh_Upper);
       aprintf("     Free:  0x%08lx\n", (LONG)dbm.mmh.mh_Free);
       PutStr ("MemChunk:\n");
       aprintf("     Next:  0x%08lx\n", (LONG)dbm.mmc.mc_Next);
       aprintf("    Bytes:  0x%08lx\n", (LONG)dbm.mmc.mc_Bytes);
       aprintf("MemChunk Location:  0x%08lx\n", dbm.mcLoc);
       aprintf("Allocation Attempt: %ld\n", dbm.attempt);
    }
    if (toLog) {
       if (safe_saves) {
           if (!(fh = Open(logFileName, MODE_OLDFILE))) {
               PrintFault(IoErr(),PROGNAME);
               PutStr("Server Error in dumpDBM!  Can't write to LogFile!!\n");
               return;
           }
           Seek(fh, 0L, OFFSET_END);
       }
       afprintf("MemHeader:\n",0);
       afprintf("       Node:\n",0);
       afprintf("              Succ:  0x%08lx\n",(LONG)dbm.mmh.mh_Node.ln_Succ);
       afprintf("              Pred:  0x%08lx\n",(LONG)dbm.mmh.mh_Node.ln_Pred);
       afprintf("              Type:  0x%08lx\n",(LONG)dbm.mmh.mh_Node.ln_Type);
       afprintf("               Pri:  0x%08lx\n",(LONG)dbm.mmh.mh_Node.ln_Pri);
       afprintf("              Name:  0x%08lx\n",(LONG)dbm.mmh.mh_Node.ln_Name);
       afprintf("     Attrib:  0x%08lx\n", (LONG)dbm.mmh.mh_Attributes);
       afprintf("   MemChunk:  0x%08lx\n", (LONG)dbm.mmh.mh_First);
       afprintf("      Lower:  0x%08lx\n", (LONG)dbm.mmh.mh_Lower);
       afprintf("      Upper:  0x%08lx\n", (LONG)dbm.mmh.mh_Upper);
       afprintf("       Free:  0x%08lx\n", (LONG)dbm.mmh.mh_Free);
       afprintf("MemChunk:\n",0);
       afprintf("       Next:  0x%08lx\n", (LONG)dbm.mmc.mc_Next);
       afprintf("      Bytes:  0x%08lx\n", (LONG)dbm.mmc.mc_Bytes);
       afprintf("MemChunk  Location:  0x%08lx\n", dbm.mcLoc);
       afprintf("Allocation Attempt:    %8ld\n", dbm.attempt);

       if (safe_saves) {
           Close(fh);
           fh = NULL;
       }
    }
}
#endif
