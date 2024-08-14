/* Requires MemServer.  Simple program to copy between two blocks of RAM. */

/* Pragmas ------------------------------------------------------------------- */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

/* Docs -----------------------------------------------------------------------
FROM=FROMBOARD/A:  Board on which source mempool is located.
TO=TOBOARD/A:      Board on which destination memory is located.
BUF=BUFSIZE/K/N:   Size of source and destination memory pools.
REPS/K/N:          Number of repetitions of test
PAT/K/N:           Pattern to write to RAM
VER=VERIFY/S:      Whether to verify that the write was accurate.
COPY/S:            Use an existing pool of memory if possible.  If existing memory
                   is used, a pattern will not be written.

What you need to run this code:
    MemServer - the A2091 memory server.
    AmigaDOS 2.0x; use the shell to launch this.
    A system with a few memory boards that need testing. (or bus, or whatever)

What you need to compile this code:
    SAS C compiler, 5.10b or better
    Cape Assembler, V2.5 or better
    memcpy.c, bsprintf.asm, bsprintf2.asm, common_h.c, lmkfile
    AmigaDOS 2.0x includes/libs/etc, release 37.4 or better.
    AmigaDOS 2.0x if you want to say "lmk debug_all" and have it work.

*/


/* Defines ------------------------------------------------------------------- */
#define PORT_TEMPLATE "MemCopy_"
#define PORT_TEMP_LEN 11         /* length of template + # of digits of retries + null */
#define TEMPLATE "FROM=FROMBOARD/A,TO=TOBOARD/A,BUF=BUFSIZE/K/N,REPS/K/N,PAT/K/N,PRI=PRIORITY/K/N,VER=VERIFY/S,COPY/S"
#define OPT_FROM 0
#define OPT_TO   1
#define OPT_BUF  2
#define OPT_REPS 3
#define OPT_PAT  4
#define OPT_PRI  5
#define OPT_VER  6
#define OPT_CPY  7
#define NUM_OPTS 8
#define PROGNAME "MemCopy"
#define DEFAULT_REPS 30
#define DEFAULT_PAT 'A'
#define DEFAULT_BUF 512 * 1024
#define MAX_PORTNAME_RETRIES 999
#define LOG_BUF_SIZE 128               /* max size of log message */
#define DEFAULT_PRIORITY 2             /* our default task priority */

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
    struct memRequest mreq1;
    struct memRequest mreq2;
};

extern struct ExecBase *SysBase;


/* Function Protos ----------------------------------------------------------- */
extern VOID bsprintf(UBYTE *target, UBYTE *format, ULONG args, ...);  /* asm */
VOID GoodBye(struct Globals *g);
BOOL sendMsg(struct memRequest *mr, struct Globals *g);
struct MsgPort *createClientPort(STRPTR portName);
struct MsgPort *findServerPort(VOID);
VOID dumpMemReq(struct memRequest *mr);
VOID aprintf(STRPTR fmt, LONG argv, ...);
BOOL servLog(struct memRequest *mr, struct Globals *g, STRPTR fmt, LONG argv, ...);
BOOL servAccess(LONG new, struct memRequest *mr, struct Globals *g);
BOOL servAlloc(ULONG bytes, LONG mode, struct memRequest *mr, struct Globals *g);
BOOL servFree(struct memRequest *mr, struct Globals *g);
VOID showValidNames(VOID);
VOID setBoard(STRPTR name, struct memRequest *mr);


/* disable SAS break handling */
NUKE_SAS_BREAK
REALLY_NUKE_IT

VOID main(int argc, UBYTE *argv[]) {
    LONG Opts[NUM_OPTS];
    struct Globals g;
    struct MsgPort *sp = NULL;          /* server's message port */
    struct memRequest *mr1, *mr2;
    ULONG i, reps, index, memBufSize;
    SHORT pat;
    BOOL verify = FALSE;
    UBYTE *mem1, *mem2;
    LONG protect = OK_WRITE, pri=DEFAULT_PRIORITY;

    EXIT_IF_ANCIENT();
    EXIT_IF_WB();

    memset(&g, 0, sizeof(g));
    memset(Opts, 0, sizeof(Opts));

    g.rc = RETURN_FAIL;
    mr1 = &(g.mreq1);
    mr2 = &(g.mreq2);
    mr1->memPtr = NULL;
    mr2->memPtr = NULL;

    if (!(g.rdargs = ReadArgs(TEMPLATE,Opts,NULL))) {
        PrintFault(IoErr(),PROGNAME);
        GoodBye(&g);
    }
    
    sp = findServerPort();
    if (!sp) {
        PutStr("MemServer isn't running!  Go start it first.\n");
        GoodBye(&g);
    }
    g.mp = createClientPort(g.portName);
    if (!(g.mp)) {
        PutStr("Couldn't create a message port!\n");
        GoodBye(&g);
    }

    /* Note:  we now have a more specific program name: g.portName .... */

    /* Get the rest of the ReadArgs options set up */

    /* Set up number of repetitions for test */
    if (Opts[OPT_REPS])
        reps = *(ULONG *)Opts[OPT_REPS];
    else
        reps = DEFAULT_REPS;

    /* Set up size of memory blocks to test */
    if (Opts[OPT_BUF]) {
        memBufSize = *(ULONG *)Opts[OPT_BUF];
    }
    else {
        memBufSize = DEFAULT_BUF;
    }

    /* Set up pattern to write to RAM */
    if (Opts[OPT_PAT]) {
        pat = *(SHORT *)Opts[OPT_PAT];
    }
    else {
        pat = DEFAULT_PAT;
    }

    /* Set up Verify option */
    if (Opts[OPT_VER])
        verify = TRUE;

    if (Opts[OPT_CPY]) {
        protect = OK_READ;
    }

    /* Initialize client names for logging purposes */
    mr1->clientName = g.portName;
    mr2->clientName = mr1->clientName;

    /* Get Board specifications, set 'em, and check for validity */
    setBoard((STRPTR)Opts[OPT_FROM], mr1);
    setBoard((STRPTR)Opts[OPT_TO], mr2);
    if (mr1->board < 0) {
        aprintf("%s: Invalid board specification: '%s'\n", (LONG)(g.portName),Opts[OPT_FROM]);
        GoodBye(&g);
    }
    if (mr2->board < 0) {
        aprintf("%s: Invalid board specification: '%s'\n", (LONG)(g.portName),Opts[OPT_TO]);
        GoodBye(&g);
    }

    /* First allocation */
    if (!servAlloc(memBufSize, protect, mr1, &g)) {
        aprintf("%s: Error with first memory allocation attempt!\n", (LONG)(g.portName));
        D(dumpMemReq(mr1));
        GoodBye(&g);
    }

    /* Second allocation */
    if (!servAlloc(memBufSize, OK_WRITE, mr2, &g)) {
        aprintf("%s: Error with second memory allocation attempt!\n", (LONG)(g.portName));
        D(dumpMemReq(mr1));
        GoodBye(&g);
    }
    if (Opts[OPT_PRI]) {
        pri = *(LONG *)Opts[OPT_PRI];
        if (abs(pri) > 5) {
            aprintf("%s: Invalid priority, %ld, using %ld\n",(LONG)(g.portName),pri,DEFAULT_PRIORITY);
            pri = DEFAULT_PRIORITY;
        }
    }

    /* Return OK unless something nasty happens later on */
    g.rc = RETURN_OK;

    /* Set our priority */
    SetTaskPri(FindTask(NULL),pri);

    /* Perform Test */


    mem1 = (UBYTE *)(mr1->memPtr);
    mem2 = (UBYTE *)(mr2->memPtr);

    D(aprintf("%s: RAM 0x%08lx (%ld), 0x%08lx (%ld)\n", (ULONG)(g.portName), (ULONG)(mr1->memPtr), (ULONG)(mr1->memSize), (ULONG)(mr2->memPtr), (ULONG)(mr2->memSize)));
    D(aprintf("%s: RAM 0x%08lx (%ld), 0x%08lx (%ld)\n", (ULONG)(g.portName), (ULONG)(mem1), (ULONG)(mr1->memSize), (ULONG)(mem2), (ULONG)(mr2->memSize)));

    /* Write pattern */
    if (protect == OK_WRITE) {
        for (index = 0; index < mr1->memSize; index++) {
            mem1[index] = pat;
        }
        /* We're done writing to mem1, set it to READ protection              */
        /* For this test, we don't particularly care if the mode change fails */
        if (!servAccess(OK_READ, mr1, &g)) {
            aprintf("%s: Couldn't change from WRITE to READ protection.\n", 
              (LONG)(g.portName));
        }
    }
    D(aprintf("\nMemCpy: Msg should refer to 0x%08lx and 0x%08lx\n", (LONG)mem1, (ULONG)mem2));

    servLog(mr1, &g, "Beginning Write Tests between 0x%08lx and 0x%08lx ", (LONG)mem1, (ULONG)mem2);

    for (index = 0; index < reps; index++) {
        for (i = 0; i < mr1->memSize; i++) {
            mem2[i] = mem1[i];
        }
        if (verify) {
            for (i = 0; i < mr1->memSize; i++) {
                if (mem2[i] != mem1[i]) {
                    servLog(mr1, &g, "Verify Failure: loc. 0x%08lx vs 0x%08lx, pass %ld of %ld",
                             (LONG)(&mem2[i]), &mem1[i],index+1L,reps);
                }
            }
        }
        if (USER_HIT_CTRL_C) {
            g.rc = RETURN_WARN;
            GoodBye(&g);
        }
    }
    servLog(mr1, &g, "Completed Memory Writes%s", (((verify) ? (LONG)" and verification" : 0L)));
    g.rc = RETURN_OK;
    GoodBye(&g);
}

/* aprintf =================================================================== */
/* To avoid absolute reference to DOSBase in amiga.lib... */
VOID aprintf(STRPTR fmt, LONG argv, ...) {
    VPrintf(fmt, &argv);
}

/* GoodBye ================================================================== */
VOID GoodBye(struct Globals *g) {
    struct Message *msg = NULL;
    SHORT rc;

    rc = g->rc;

    /* Free first RAM block, if necessary */
    if (g->mreq1.memPtr)
        servFree(&(g->mreq1), g);

    /* Free second RAM block, if necessary */
    if (g->mreq2.memPtr)
        servFree(&(g->mreq2), g);

    /* Tell Server we're going away */
    g->mreq1.cmd = CMD_ABEO;
    sendMsg(&g->mreq1, g);

    /* Clean up message port */
    if (g->mp) {
        while ((msg = GetMsg(g->mp)))
            /* do nothing; these should be replies */
            ;
    }
    DeletePort(g->mp);
    g->mp = NULL;

    /* Release ReadArgs structure */
    if (g->rdargs) {
        FreeArgs(g->rdargs);
        g->rdargs = NULL;
    }
    exit(rc);
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
