/* Client_Template
   This is the complete outline for writing your own memory clients for
   the memory server.  It implements all kinds of nifty functions to
   deal with said server.
   
   Suggested trick:  Use one struct memRequest per memory block controlled
   by the server.  Keep this in Globals so everything can get at it.
   (No, 'tis not really global, just a pointer passed to just about everything.)
   This will make life much easier in the long run...
 */


/* Pragmas ------------------------------------------------------------------- */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

/* Docs -----------------------------------------------------------------------
What you need to run this code:
    MemServer - the A2091 memory server.
    AmigaDOS 2.0x; use the shell to launch this.
    A system with a few memory boards that need testing. (or bus, or whatever)

What you need to compile this code:
    SAS C compiler, 5.10b or better
    Cape Assembler, V2.5 or better
    this file, bsprintf.asm, bsprintf2.asm, common_h.c, lmkfile
    AmigaDOS 2.0x includes/libs/etc, release 37.4 or better.
    AmigaDOS 2.0x if you want to say "lmk debug_all" and have it work.

*/




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

    bsprintf2(tmpBuf, fmt, &argv);
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
