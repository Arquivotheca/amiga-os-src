/* ReadFM.c
   This thingie just reads the given file into a block of RAM,
   x number of times.  It's smart enough to deal with files larger
   than the RAM size, and will do the appropriate Seek()s for proper
   verification (if specified).  Also, creates the file if necessary
   using SetFileSize().
 */

/* CAUTION:  THIS PROGRAM IS QUITE CAPABLE OF TRASHING DISK DRIVES.

    09 Mar 92  Fixed problem with deleting files, and added better error report.
    12 Mar 92  Added mechanism for better error handling; complies with new
               MemServer's requirements (memserver v2.x).  Will now follow
               server's advice when Read() fails, for instance.
*/


/* Use pre-compiled headers in common_h.c */

/* Pragmas ------------------------------------------------------------------- */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "client2server.h"

/* Defines ------------------------------------------------------------------- */
#define DOS_TRUE DOSTRUE
#define DOS_FALSE DOSFALSE
#define PORT_TEMPLATE "ReadFM_"          /* a number is inserted after the underscore */
#define PORT_TEMP_LEN 11                 /* length of template + # of digits of retries + null */
#define PROGNAME "ReadFM"                /* used with PrintFault... */
#define DEF_MEMTYPE LOC_ANY
#define DEFAULT_MEMSIZE 256*1024
#define DEFAULT_PRIORITY 2
#define DB_MF1           0xFF
#define DB_MF2           0xEE

/* ReadArgs Template Defines --------------------------------------------------- */
#define TEMPLATE "FILE/A,MS=MEMSIZE/K/N,MT=MEMTYPE/K,REPS/K/N,FS=FILESIZE/K/N,PRI=PRIORITY/K/N,VER=VERIFY/S,RM=DELETE/S"
#define OPT_FILE    0
#define OPT_MEM     1
#define OPT_MEMTYPE 2
#define OPT_REPS    3
#define OPT_FSIZE   4
#define OPT_PRI     5
#define OPT_VER     6
#define OPT_DEL     7
#define NUM_OPTS    8

/* Where:
    FILE      - file to work with
    MEMSIZE   - Memory size
    MEMTYPE   - Memory type/location/board
    REPS      - times to repeat test.
    FS        - new size for file; implies file creation
    VER       - whether to verify info (requires 2*RAM)
    DELETE    - Delete file when finished?  Default = NO.
*/


#ifdef DEBUG
#define D(x) (x);
#else
#define D(x) ;
#endif


/* Globals.  This Code Shall Be Residentable --------------------------------- */
struct Globals {
    /* Things common to all clients */
    struct RDArgs *rdargs;
    struct MsgPort *mp;
    struct DateTime dt;
    UBYTE portName[PORT_TEMP_LEN];
    SHORT rc;

    /* Things specific to this client */
    struct memRequest mr;   /* regular buffer */
    struct memRequest mr2;  /* for verify     */
    BPTR   fh;

};

extern struct ExecBase *SysBase;

#include "client2server.c"

/* disable SAS break handling */
NUKE_SAS_BREAK
REALLY_NUKE_IT

VOID main(int argc, UBYTE *argv[]) {
    LONG Opts[NUM_OPTS];
    struct Globals *g = NULL;           /* make it dynamic; otherwise we blow up the default stack */
    struct MsgPort *sp = NULL;          /* server's message port */
    struct memRequest *mr, *mr2;
    ULONG reps;
    STRPTR fileName;
    BOOL   verify = FALSE;
    LONG oldpos=0, curpos=0;            /* file positions for Seek() */
    LONG protection = NC_WRITE;
    LONG i, fileSize, amtRead = 0;
    LONG bytesLeft, pri=DEFAULT_PRIORITY;
    BPTR lock = NULL;

    EXIT_IF_ANCIENT();
    EXIT_IF_WB();

    g = AllocVec(sizeof(struct Globals), MEMF_CLEAR);
    if (!g) {
        PrintFault(IoErr(),PROGNAME);
        exit(RETURN_FAIL);
    }

    g->rc = RETURN_FAIL;  /* we'll change it after we succeed with setup */
    mr  = &(g->mr);
    mr2 = &(g->mr2);
    mr2->board = LOC_ANY;

    memset(Opts, 0, sizeof(Opts));  /* prepare for ReadArgs */

    /* Call ReadArgs */
    if (!(g->rdargs = ReadArgs(TEMPLATE,Opts,NULL))) {
        PrintFault(IoErr(),PROGNAME);
        GoodBye(g);
    }

    /* Find the server */    
    sp = findServerPort();
    if (!sp) {
        PutStr("MemServer isn't running!  Go start it first.\n");
        GoodBye(g);
    }

    /* Make a Port */
    g->mp = createClientPort(g->portName);
    if (!(g->mp)) {
        PutStr("Couldn't create a message port!\n");
        GoodBye(g);
    }

    /* Note:  we now have a more specific program name: g->portName .... */

    /* Get the rest of the ReadArgs options set up */
    fileName = (STRPTR)Opts[OPT_FILE];   /* assumes /A switch */

    if (Opts[OPT_VER]) {
        verify = TRUE;
        protection = OK_WRITE;
    }

    if (Opts[OPT_MEM]) {
        mr->memSize = *(ULONG *)Opts[OPT_MEM];
    }
    else {
        mr->memSize = DEFAULT_MEMSIZE;
    }

    /* Do board location */
    if (Opts[OPT_MEMTYPE]) {
        setBoard((STRPTR)Opts[OPT_MEMTYPE], mr);
    }
    else {
        mr->board = LOC_ANY;
    }
    /* check that it was valid */
    if (mr->board < 0) {
        aprintf("%s: Invalid board specification: '%s'\n", (LONG)(g->portName),Opts[OPT_MEMTYPE]);
        GoodBye(g);
    }

    /* Set up number of repetitions for test */
    if (Opts[OPT_REPS])
        reps = *(ULONG *)Opts[OPT_REPS];
    else
        reps = DEFAULT_REPS;

    /* Initialize client names for logging purposes */
    mr->clientName  = g->portName;
    mr2->clientName = g->portName;

    /* Memory allocation
       Get block of RAM to feed file to.
     */
    if (!servAlloc(mr->memSize, protection, mr, g)) {
        aprintf("%s: Memory allocation failed, exiting...\n",(LONG)(g->portName));
        GoodBye(g);
    }

    /* See if the file exists, find its size, and open it. */
    lock = Lock(fileName,SHARED_LOCK);
    if (lock) {
        struct FileInfoBlock *fib = NULL;  /* temp variable */
        struct TagItem ti[1] = {TAG_DONE,TAG_DONE};

        /* Go out and find out how large the file is... */
        /* get a fileInfoBlock and check that we did */
        fib = AllocDosObject(DOS_FIB, ti);
        if (!fib) {
            mr->status = STATUS_SEVERE_ERR;
            servLog(mr, g, "Couldn't allocate a FileInfoBlock, exiting...", 0L);
            UnLock(lock);
            GoodBye(g);
        }
        /* Examine the file */
        if (Examine(lock, fib) == DOS_TRUE) {
            fileSize = fib->fib_Size;
        }
        else {
            mr->status = STATUS_SEVERE_ERR;
            servLog(mr, g, "Couldn't Examine() file '%s', exiting...", (LONG)fileName);
            UnLock(lock);
            GoodBye(g);
        }
        /* Clean up our Examine() excursion */
        FreeDosObject(DOS_FIB, fib);
        UnLock(lock);

        /* Open file as existing file */
        g->fh = Open(fileName, MODE_OLDFILE);
        if (!(g->fh)) {
            mr->status = STATUS_SEVERE_ERR;
            servLog(mr,g,"Couldn't open file %s!",(LONG)fileName);
            GoodBye(g);
        }
    }
    else {
        if (!Opts[OPT_FSIZE]) {
            /* We can' find it and we're not supposed to create it */
            mr->status = STATUS_SEVERE_ERR;
            servLog(mr, g, "File '%s' doesn't exist!  Exiting...", (LONG)fileName);
            GoodBye(g);
        }
    }

    if (Opts[OPT_FSIZE]) {
        /* If we have an FSIZE, we're supposed to create the file, or size it. */
        if (!(g->fh)) {
            g->fh = Open(fileName, MODE_NEWFILE);
        }
        fileSize = *(LONG *)Opts[OPT_FSIZE];
        if (g->fh) {
            servLog(mr, g, "Sizing/Creating file '%s', size %ld", (LONG)fileName, fileSize);
            /* this call really should be checked... */
            SetFileSize(g->fh, fileSize, OFFSET_BEGINNING);
            Seek(g->fh,0,OFFSET_BEGINNING);
        }
        else {
            mr->status = STATUS_SEVERE_ERR;
            servLog(mr, g, "Can't create file '%s'", (LONG)fileName);
            GoodBye(g);
        }
    }

    /* go allocate buffer for verification, if requested */
    if (Opts[OPT_VER]) {
        if (!servAlloc(min((ULONG)fileSize, mr->memSize), OK_WRITE, mr2, g)) {
            aprintf("%s: Memory allocation for Verify failed, exiting...\n",(LONG)(g->portName));
            GoodBye(g);
        }
    }
    if (Opts[OPT_PRI]) {
        pri = *(LONG *)Opts[OPT_PRI];
        if (abs(pri) > 5) {
            aprintf("%s: Invalid priority %ld, using %ld\n", (LONG)(g->portName),pri,DEFAULT_PRIORITY);
            pri = DEFAULT_PRIORITY;
        }
    }

    /* Return OK now... */
    g->rc = RETURN_OK;

    /* Set our priority */
    SetTaskPri(FindTask(NULL),pri);

    /* Perform Test .................................... */
    /* Think about the design here.  
       We are reading disk-to-RAM continuously, sometimes w/verify (ie double read)
       All of this makes a difference...
     */

    servLog(mr,g,"Begin FileReads: '%s' (%ld) to 0x%08lx (%ld)",
            (LONG)fileName,fileSize,mr->memPtr,mr->memSize);
    for (i = 0; i < reps; i++) {
        servLog(mr,g,"FileRead %ld of %ld: '%s' (%ld) to 0x%08lx (%ld)",
                (LONG)i+1L, reps, (LONG)fileName,fileSize,mr->memPtr,mr->memSize);
        if (fileSize > mr->memSize) {       /* must read the file memSize at a time */
            D(PutStr("Reading file memSize at a time...\n"));
            bytesLeft = fileSize;
            while (bytesLeft > 0) {
/* debug */
                    memset(mr->memPtr,(LONG)DB_MF1,mr->memSize);  /* nuke previous contents */
/* end debug */
                oldpos = Seek(g->fh, 0, OFFSET_CURRENT);
                oldpos = Seek(g->fh, 0, OFFSET_CURRENT);
                amtRead = Read(g->fh, mr->memPtr, mr->memSize);
                if (amtRead < 0) {
                    mr->status = STATUS_SEVERE_ERR;
                    servLog(mr, g, "Error reading '%s' (%ld) - IoErr %ld", 
                             (LONG)fileName, amtRead, IoErr());
                    if (mr->status == STATUS_CHECK_CMD) {
                        if (mr->cmd == CMD_QUIT) {
                            GoodBye(g);
                        }
                    }
                }
                bytesLeft -= amtRead;
                if (USER_HIT_CTRL_C)
                    GoodBye(g);
                curpos = Seek(g->fh, 0, OFFSET_CURRENT);
                curpos = Seek(g->fh, 0, OFFSET_CURRENT);
/* debug */
/*                servLog(mr,g,"Curpos %ld, Oldpos %ld, Diff %ld, amtRead %ld, BL %ld",curpos,oldpos,curpos-oldpos,amtRead,bytesLeft); */
/* end debug */
                D(aprintf("%s: Read %ld, %ld left, cur %ld, old %ld\n", (LONG)(g->portName), amtRead, bytesLeft, curpos, oldpos));
                if (verify) {
                    UBYTE *src, *dest;
                    LONG j, firstErr=0L,errCount=0L,ovpos,cvpos,amtRead2;

                    src  = (UBYTE *)(mr->memPtr);
                    dest = (UBYTE *)(mr2->memPtr);
                    ovpos = Seek(g->fh, oldpos, OFFSET_BEGINNING);
                    cvpos = Seek(g->fh, 0, OFFSET_CURRENT);
/* debug */
/*                    servLog(mr,g,"2-Curpos %ld, Oldpos %ld, OVPos %ld, CVPos %ld",curpos,oldpos,ovpos,cvpos); */
                    memset(mr2->memPtr,(LONG)DB_MF2,mr2->memSize);  /* nuke previous contents */
/* end debug */
                    amtRead2 = Read(g->fh, mr2->memPtr, mr2->memSize);
/* debug */
                    servLog(mr,g,"%ld: Verifying range 0x%08lx to 0x%08lx",i+1L,oldpos,oldpos+amtRead2);
/* end debug */
                    D(aprintf("%s: tmppos %ld, curpos %ld, oldpos %ld, amtRead2 %ld\n", (LONG)(g->portName),tmppos,curpos,oldpos,amtRead2));
                    if (amtRead2 < 0) {
                        mr->status = STATUS_SEVERE_ERR;
                        servLog(mr, g, "Read Error on Verify of '%s' (%ld) - IoErr %ld", 
                                 (LONG)fileName, (LONG)amtRead2, (LONG)IoErr());
                        if (mr->status == STATUS_CHECK_CMD) {
                            if (mr->cmd == CMD_QUIT)
                                GoodBye(g);
                        }
                    }
                    for (j = 0; j < amtRead2; j++) {
                        if (src[j] != dest[j]) {
                            errCount++;
                            mr->status = STATUS_CLIENT_ERR;
                            servLog(mr,g," **Verify Error at offset %12ld (0x%08lx): 0x%02lx != 0x%02lx (%ld)",cvpos+j,cvpos+j,src[j],dest[j],i+1L);
                            if (!firstErr)
                                firstErr = j;
                        }
                    }
                    if (errCount) {
                        servLog(mr, g, "%ld Errs in verify %ld of %ld on %s, First = %ld, Range: %ld to %ld\n  OrigBufRead: %ld, VerRead: %ld (err)",
                                errCount, i+1L, reps, (LONG)fileName, firstErr, oldpos, oldpos+amtRead2,amtRead,amtRead2);
                    }
                    errCount = 0; 
                    firstErr = 0;
                }
            } /* end while */
            oldpos = Seek(g->fh,0,OFFSET_BEGINNING);  /* or close/open the file */
        }

        else {    /* we can read the whole file at once */
            D(PutStr("Reading whole file at once\n"));
            amtRead = Read(g->fh, mr->memPtr, fileSize);
            if (amtRead < 0) {
                mr->status = STATUS_SEVERE_ERR;
                servLog(mr, g, "Error reading '%s' (%ld) - IoErr %ld", 
                         (LONG)fileName, amtRead, IoErr());
                if (mr->status == STATUS_CHECK_CMD) {
                    if (mr->cmd == CMD_QUIT)
                        GoodBye(g);
                }
            }
            Seek(g->fh, 0, OFFSET_BEGINNING);
            if (verify) {
                ULONG j, errCount=0UL, firstErr=0UL;
                UBYTE *src, *dest;

                servLog(mr, g, "Beginning Verify %ld of %s",i+1L,(LONG)fileName);
                src  = (UBYTE *)(mr->memPtr);
                dest = (UBYTE *)(mr2->memPtr);
                /* re-read and verify */
                amtRead = Read(g->fh, mr2->memPtr, fileSize);
                if (amtRead < 0) {
                    mr->status = STATUS_SEVERE_ERR;
                    servLog(mr, g, "Error in VerifyRead of '%s' (%ld) - IoErr %ld", 
                             (LONG)fileName, amtRead, IoErr());
                    if (mr->status == STATUS_CHECK_CMD) {
                        if (mr->cmd == CMD_QUIT)
                            GoodBye(g);
                    }
                }
                Seek(g->fh, 0, OFFSET_BEGINNING);
                for (j = 0; j < amtRead; j++) {
                    if (src[j] != dest[j]) {
                        errCount++;
                        mr->status = STATUS_CLIENT_ERR;
                        servLog(mr,g," **Verify Error at offset %ld: 0x%02lx != 0x%02lx",j,src[j],dest[j]);
                        if (!firstErr)
                            firstErr = j;
                    }
                }
                if (errCount) {
                    servLog(mr, g, "%ld Errors in verify %ld of %ld on %s, First = %ld",
                            errCount, i+1L, reps, (LONG)fileName, firstErr);
                }
                servLog(mr, g, "Completed Verify %ld of %ld on %s",
                        i+1L, reps, (LONG)fileName);
            }
        }
    }

    if (Opts[OPT_DEL]) {
        Close(g->fh);
        g->fh = NULL;
        DeleteFile(fileName);
        lock = Lock(fileName, SHARED_LOCK);
        if (lock) {
            mr->status = STATUS_CLIENT_ERR;
            servLog(mr, g, "Error deleting file '%s'", (LONG)fileName);
            UnLock(lock);
        }
        else {
            servLog(mr, g, "Deleted file '%s'", (LONG)fileName);
        }
    }

    /* Tell server we finished tests successfully, then exit */
    servLog(mr, g, "Completed %ld FileReads of '%s'", reps, fileName);

    g->rc = RETURN_OK;
    GoodBye(g);
}

/* GoodBye ================================================================== */
VOID GoodBye(struct Globals *g) {
    struct Message *msg = NULL;
    SHORT rc = RETURN_FAIL;

    if (g) {
        /* In case you wanted to allocate g dynamically and will free it here... */
        rc = g->rc;                                   /* ...copy the returncode. */

        /* Free all RAM blocks, if necessary.              */
        /* ptr to relevant memrequest should be in Globals */
        if (g->mr.memPtr) {
            D(PutStr("Freeing mr\n"));
            servFree(&(g->mr), g);
        }
        if (g->mr2.memPtr) {
            D(PutStr("Freeing mr2\n"));
            servFree(&(g->mr2), g);
        }

        /* Close file if it's open */
        if (g->fh) {
            D(PutStr("Closing file\n"));
            Close(g->fh);
            g->fh = NULL;
        }

        /* Tell Server we're going away  */
        D(PutStr("Informing Server of exit\n"));
        g->mr.cmd = CMD_ABEO;
        sendMsg(&(g->mr), g);

        /* Clean up message port */
        if (g->mp) {
            D(PutStr("Cleaning up msgport\n"));
            while ((msg = GetMsg(g->mp)))
                /* do nothing; these should be replies */
                ;
            DeletePort(g->mp);
            g->mp = NULL;
        }

        /* Release ReadArgs structure */
        if (g->rdargs) {
            D(PutStr("Freeing RDArgs structure\n"));
            FreeArgs(g->rdargs);
            g->rdargs = NULL;
        }
        D(PutStr("Freeing Globals structure\n"));
        FreeVec(g);
        g = NULL;
    }
    D(PutStr("Final Exit...\n"));
    exit(rc);  /* ...that's the news and we're outta here... */
}

/* All this stuff is in client2server.c, but is copied and #ifdef'd out for reference */
#ifdef BLOW_UP_COMPILER
/* aprintf =================================================================== */
/* To avoid absolute reference to DOSBase in amiga.lib... */
VOID aprintf(STRPTR fmt, LONG argv, ...) {
    VPrintf(fmt, &argv);
}



/* findServerPort =========================================================== 
   Depends on a Common_h.c define of SERVER_PORT
 */
struct MsgPort *findServerPort(VOID) {
    struct MsgPort *mp = NULL;
    
    Forbid();
        mp = FindPort(SERVER_PORT);
    Permit();
        if (!mp) {
            PutStr("Can't find server!!\n");
            return(NULL);
        }
    return(mp);
}

/* createClientPort ========================================================= */
struct MsgPort *createClientPort(STRPTR portName) {
    SHORT i;
    struct MsgPort *mp = NULL;

    for (i = 0; i < MAX_PORTNAME_RETRIES; i++) {
        bsprintf(portName, "%s%ld", (LONG)PORT_TEMPLATE, (LONG)i);
        Forbid();
            if (!FindPort(portName)) {
                mp = CreatePort(portName,PORT_PRIORITY);
                Permit();
                return(mp);
            }
        Permit();
    }
    return(NULL);
}

/* sendMsg =====================================================================
   Given a memRequest and Globals, sends it safely off to the server
   and waits for a single reply.  Globals must contain a struct DateTime
   and appropriate messageport information.

   Returns TRUE if successful, FALSE for error.

   FALSE basically means that the server has gone away, and is a very nasty
   (and fatal) error.  It could also mean that we don't have a MsgPort,
   which means we were probably called in an exit routine.
*/
BOOL sendMsg(struct memRequest *mr, struct Globals *g) {
    struct MsgPort *sp = NULL;
    struct Message *msg = NULL;
   
    if (!(g->mp))
        return(FALSE);

    g->dt.dat_Format = FORMAT_DOS;
    mr->dt = &(g->dt);
    mr->msg.mn_Node.ln_Type = NT_MESSAGE;
    mr->msg.mn_Length = sizeof(mr);
    mr->msg.mn_ReplyPort = g->mp;

    /* DateStamp just before sending */
    DateStamp(&(g->dt.dat_Stamp));    

    /* Find the serverport and send the message */
    Forbid();
        if ((sp = FindPort(SERVER_PORT)) != NULL)
            PutMsg(sp, (struct Message *)mr);
    Permit();

    /* Check to see that we found the server */
    if (!sp) {
        /* We didn't send anything */
        return(FALSE);
    }

    /* Wait for our reply */
    WaitPort(g->mp);
    msg = GetMsg(g->mp);

    /* A bit of paranoia... we should get *our* message back */
    if (msg != (struct Message *)mr) {
        aprintf("%s: returning message has different address than outgoing!\n",
                (LONG)(g->portName));
    }

    return(TRUE);
}

/* declarations useful for looking up valid board specs */
struct lookUp {
    STRPTR name;
    LONG val;
};

static struct lookUp vals[] = {
    { "MB",     LOC_MOTHERBOARD },
    { "CPU",    LOC_CPU_BOARD },
    { "1",      LOC_BUS_1 },
    { "2",      LOC_BUS_2 },
    { "3",      LOC_BUS_3 },
    { "4",      LOC_BUS_4 },
    { "5",      LOC_BUS_5 },
    { "6",      LOC_BUS_6 },
    { "CHIP",   LOC_CHIP },
    { "FAST",   LOC_FAST },
    { "ANY",    LOC_ANY },
    { "ROM",    LOC_ROM },
    { NULL,     -1},
};


/* setBoard ================================================================
   Given a strptr to a name, tries to match it with known good names.
 */

VOID setBoard(STRPTR name, struct memRequest *mr) {
    SHORT i;

    for (i = 0; vals[i].name; i++) {
        if (stricmp(name, vals[i].name) == 0) {
            mr->board = vals[i].val;
            return;
        }
    }
    mr->board = -2;
}

/* showValidNames ================================================================
   Prints a list of valid board names to the console
*/
VOID showValidNames(VOID) {
    SHORT i;

    PutStr("Valid Board specifications:\n");
    for (i = 0; vals[i].name; i++) {
        aprintf("%s ", (LONG)(vals[i].name));
    }
    PutStr("\n\n");
}

/* dumpMemReq ===================================================================
    Prints contents of memRequest structure.  Good for debugging.
 */
VOID dumpMemReq (struct memRequest *mr) {
    aprintf("memPtr: 0x%08lx\n", (LONG)mr->memPtr);
    aprintf("memSize: 0x%08lx (%ld)\n",mr->memSize,mr->memSize);
    aprintf("memType:  0x%08lx\n", (LONG)mr->memType);
    aprintf("memUpper: 0x%08lx\n", (LONG)mr->memUpper);
    aprintf("memLower: 0x%08lx\n", (LONG)mr->memLower);
    aprintf("board:     %ld\n", (LONG)mr->board);
    aprintf("access:    %ld\n", (LONG)mr->access);
    aprintf("newAccess: %ld\n", (LONG)mr->newAccess);
    aprintf("dt:  0x%08lx\n", (LONG)mr->dt);
    aprintf("log1:  '%s'\n", (LONG)mr->log1);
    aprintf("log2:  '%s'\n", (LONG)mr->log2);
    aprintf("clientName:  '%s'\n", (LONG)mr->clientName);
    aprintf("status:  %ld\n", (LONG)mr->status);
    aprintf("cmd:  %ld\n", (LONG)mr->cmd);
    aprintf("rc:   %ld\n", (LONG)mr->rc);
    aprintf("rc2:  %ld\n", (LONG)mr->rc2);
}


/* servAlloc ====================================================================
   Request new memory block from server.
   You must first set up the memRequest to indicate how you want to get this RAM:
   by board, range, existing pointer, or whatever.  MemSize, Access, and Cmd 
   fields are set up for you, however.
   Calls exit routine GoodBye() if a server communications error occurrs.
*/
BOOL servAlloc(ULONG bytes, LONG mode, struct memRequest *mr, struct Globals *g) {

    /* Initialize for a memory allocation */
    mr->memSize = bytes;
    mr->access = mode;
    mr->cmd = CMD_MEMREQ;

    /* Send Allocation Request */
    if (!sendMsg(mr, g)) {
        aprintf("%s: Server communications error!\n", (LONG)(g->portName));
        g->rc = RETURN_ERROR;
        GoodBye(g);
    }

    /* Check return for validity */
    if (mr->rc != RETURN_OK)
        return(FALSE);
    if (mr->memPtr == NULL)
        return(FALSE);

    return(TRUE);
}

/* servFree =====================================================================
   Request server to free a block of RAM.  memRequest must be pre-initialized,
   other than mr->cmd.  mr->memPtr is set to NULL upon success.
   TRUE is returned if server freed the RAM; otherwise FALSE is returned. 
   Does NOT call GoodBye() if server port has vanished, so that this routine
   is safe to call from GoodBye(). 
 */
BOOL servFree(struct memRequest *mr, struct Globals *g) {

    mr->cmd = CMD_FREE_MEM;
    /* Send FreeMem Request */
    if (!sendMsg(mr, g)) {
        aprintf("%s: Server communications error!\n", (LONG)(g->portName));
        g->rc = RETURN_ERROR;
        /* We intentionally don't call GoodBye here; we might be called by GoodBye()
           ourselves!
         */
    }

    /* Check return for validity */
    if (mr->rc != RETURN_OK)
        return(FALSE);
    if (mr->status != STATUS_DONE)
        return(FALSE);

    /* Zero out memory ptr if it was really freed */
    mr->memPtr = NULL;

    /* Successful free of RAM */
    return(TRUE);
}


/* servAccess ===================================================================
   Request change in memory access permission from server.
   Calls exit routine GoodBye() if a server communications error occurrs.
   see other servXX cmds for caveats.
 */
BOOL servAccess(LONG new, struct memRequest *mr, struct Globals *g) {

    /* initialize memRequest */
    mr->newAccess = new;
    mr->cmd = CMD_NEW_ACCESS;

    /* Send Access Change Request */
    if (!sendMsg(mr, g)) {
        aprintf("%s: Server communications error!\n", (LONG)(g->portName));
        g->rc = RETURN_ERROR;
        GoodBye(g);
    }

    /* Check return for validity */
    if (mr->rc != RETURN_OK)
        return(FALSE);
    if (mr->status != STATUS_DONE)
        return(FALSE);

    /* Successful change of access mode */
    return(TRUE);
}


/* servLog ======================================================================
   Request server to log a message, printf-style.
   Calls exit routine GoodBye() if a server communications error occurrs.
 */
BOOL servLog(struct memRequest *mr, struct Globals *g, STRPTR fmt, LONG argv, ...) {
    UBYTE tmpBuf[LOG_BUF_SIZE];

    /* Historical note:  bsprintf2 is just like the sprintf() in the Autodocs,
       see exec.library/RawDoFmt(), except the lea.l for the datastream has
       been changed to a move.l.
         bsprintf is exactly like the RawDoFmt() sprintf.
         (Well, both bsprintfs tend to use an EQU for RawDoFmt to be residentable.)
     */

    bsprintf2(tmpBuf, fmt, (LONG)&argv);
    D(aprintf("ServLog: '%s'\n\n", (LONG)tmpBuf)); 
    mr->cmd = CMD_LOGIT;
    mr->log1 = tmpBuf;

    /* Send Request */
    if (!sendMsg(mr, g)) {
        aprintf("%s: Server communications error!\n", (LONG)(g->portName));
        g->rc = RETURN_ERROR;
        GoodBye(g);
    }

    /* Check return for validity */
    if (mr->rc != RETURN_OK)
        return(FALSE);
    if (mr->status != STATUS_DONE)
        return(FALSE);

    /* Successful request */
    return(TRUE);
}
#endif
