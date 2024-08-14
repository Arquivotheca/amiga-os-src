/* LibMon.c  $VER: Libmon_c 2.8 (27.05.93) by J.W. Lockhart
 * 
 * KPrintf's whenever a library is opened or closed.  May optionally deny libopens.
 * 
 *
 * Note: due to some stack operations, the new OpenLibrary
 * call, myOpenLib(), is written in assembly.  
 * This routine records the stack pointer of its caller, and
 * also must swap the stack.
 *
 * Why a StackSwap()?  Well, locale.library bloats the stack
 * usage of RawDoFmt, which is required by KPrintF, which is
 * how we do our printing.  So if the RAM task (or any other
 * where stack usage is tight) tries to execute this without
 * the swap, many things get munged.  Growl!
 *
 *  OPTIONS:
 *    LIBS  - libraries in question
 *    ADD   - pay attention to these libraries, or add them to the report/ignore list.
 *    DEL   - ignore these libraries, or remove them from the report/ignore list.
 *    DENY  - fail any OpenLibrary() for these libraries
 *    STET  - do not convert library names to lower case
 *    LIST  - tell what type of list is being used, and list its contents
 *    QUIT  - terminate program
 *
 * "LIB=LIBS/K/M,ONLY=ADD/S,IGNORE=DEL/S,DENY/S,STET=NOLOWER/S,LIST/S,QUIT/S"
 *
 *  DEFAULTS:
 *    If run with no args:
 *       Reports opens/closes of all libraries.  
 *    If run only with LIBS:
 *       Reports only about those specific libraries.  Names are put in lowercase.
 *    If the Ignore/Only/Deny list becomes empty, LibMon will revert to
 *    reporting on all libraries; ADDing or DELeting libraries will have the
 *    same effect as if you had started it with the ONLY option.
 *
 *  EXAMPLES:
 *     1.  Using LibMon to Monitor Only Certain Libraries
 *           libmon libs intuition.library graphics.library only
 *           --> reports about opens/closes of graphics and intuition
 *           libmon libs gadtools.library add
 *           --> also reports about opens/closes of gadtools
 *           libmon libs graphics.library del
 *           --> now only reports on intuition and gadtools
 *
 *     2.  Using LibMon to Ignore Only Certain Libraries
 *           libmon libs dos.library ignore
 *           --> reports on all libraries except for dos
 *           libmon libs exec.library add
 *           --> reports on everything but dos and exec
 *           libmon libs dos.library del
 *           --> reports on everything but exec
 *
 *     3.  Using LibMon to Deny Access to Only Certain Libraries
 *           libmon libs gadtools.library deny
 *           --> fails all opens of gadtools (all versions)
 *           libmon libs utility.library add
 *           --> fails all opens of gadtools and utility (all versions)
 *           libmon libs gadtools.library utility.library del
 *           --> removes all openlibrary restrictions.
 *         In Denial Mode, all OpenLibrary()/CloseLibrary attempts are reported.      
 *
 *
 *
 *
 *
 * 
 */


/* Includes --------------------------------------------- */
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <workbench/startup.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/alib_protos.h>

#include <stdlib.h>
#include <string.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include <clib/wb2cli_protos.h>

/* Defines ------------------------------------------ */
#define PROGNAME "LibMon"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "LIB=LIBS/K/M,ONLY=ADD/S,IGNORE=DEL/S,DENY/S,STET=NOLOWER/S,LIST/S,QUIT/S"
#define PORTNAME "LibMon_Port"
#define OPENLIB_OFFSET   -552L
#define CLOSELIB_OFFSET  -414L
#define MYSTACKSIZE     20256UL
#define MYSTACK_STARTP  17728UL   /* (stacksize + startP) % 32 == 0 */
#define ASM __asm
#define REG(x) register __ ## x
#define DENIAL_LIST (NT_USER - 1)     /* list type: fail any libname in list        */
#define ONLY_LIST   (NT_USER - 2)     /* list type: report if name in list          */
#define IGNORE_LIST (NT_USER - 3)     /* list type: ignore if name in list          */
#define LM_CMD_QUIT   1     /* kill off old LibMon                                  */
#define LM_CMD_LIST   2     /* return list pointer, wait til done                   */
#define LM_CMD_ADD    3     /* add to list, or make an 'only' list                  */
#define LM_CMD_DEL    4     /* delete from current list, or make 'ignore' list      */
#define LM_CMD_DENY   5     /* if list is empty, change it to a 'deny' list         */
#define LM_CMD_NOLOW  6     /* do not lowercase incoming libnames                   */
#define LM_CMD_DONE   7     /* done with list pointer, safe to write to it again    */
#define LM_REPLY_OK   1     /* successful command */
#define LM_REPLY_FAIL 2     /* unsuccessful cmd   */
#define LM_REPLY_QUIT 3     /* ok, am quitting    */
#define NOT_RELEVANT  0     /* open library without reporting or denying */
#define IS_RELEVANT   1     /* open library and report it                */
#define MUST_DENY     2     /* don't open library, but report it         */
#define STBUF_SIZE    100   /* name buffer for segtracker string */

/* TypeDefs ----------------------------------------------------------- */
typedef char (*__asm SegTrack (register __a0 ULONG Address, register __a1 ULONG * SegNum, register __a2 ULONG * Offset));

/* Structs ------------------------------ */
struct Opts {
    STRPTR *libs;       /* libraries to be affected              */
    LONG    add;        /* add to   library list                 */
    LONG    del;        /* del from library list                 */
    LONG    deny;       /* deny all opens of this library        */
    LONG    stet;       /* do not convert libnames to lower case */
    LONG    list;       /* report on list type and contents      */
    LONG    quit;       /* quit out                              */
};

struct SegSem {                  /* SegTracker Semaphore Support */
    struct SignalSemaphore       seg_Semaphore;
    SegTrack                    *seg_Find;
};

struct lm_Msg {
    struct Message msg;         /* message for exec's use    */
    ULONG          cmd;         /* command or reply from LM  */
    STRPTR        *libs;        /* libraries to be affected  */
    struct List   *list;        /* list pointer for printing */
};

/* Protos ------------------------------------------ */
static VOID  GoodBye(int);
static LONG  doInit(VOID);
static APTR  allocLAligned(ULONG);
struct Library * ASM __saveds openLibGuts(REG(a2) STRPTR, REG(d2) ULONG, REG(a3) ULONG *);
VOID             ASM __saveds closeLibGuts(register __a2 struct Library *, register __a3 ULONG *);
extern ULONG     ASM __saveds myOpenLib(register __a1 STRPTR, register __d0 ULONG);
extern ULONG     ASM __saveds myCloseLib(register __a1 struct Library *);
extern VOID   KPrintF(STRPTR,...);
extern STRPTR getTaskName(VOID);
static BOOL handleMsg(struct Message *);
BOOL delLibs(STRPTR *theLibs);
BOOL addLibs(STRPTR *theLibs);
BOOL isRelevant(STRPTR libName, STRPTR theTask);
static VOID kPrintList(struct List *lml);
static VOID strLCpy(STRPTR destStr, STRPTR srcStr);



/* Globals --------------------------------------- */
extern struct Library    *SysBase;
extern struct DosLibrary *DOSBase;
VOID * (*ASM oldOpenLib)(register __a1 STRPTR, register __d0 ULONG);
VOID * (*ASM oldCloseLib)(register __a1 struct Library *);
struct Library            *UtilityBase;                 /* only server need open it */
struct RDArgs             *rdargs;
struct MsgPort            *mp;
struct SegSem             *segTrackSem;
struct StackSwapStruct    *myStackP;
struct StackSwapStruct    myStack;
struct Opts               opts;
APTR                      stackRAM;
BOOL                      useDeleteMsgPort, doNotLowerCase=FALSE, denyOpens=FALSE;
struct List               list, *l;
UBYTE specialTask[] =     PROGNAME;       /* if this is the task, ignore things such as denying OpenLibrary() */
UBYTE segTrackBuf[STBUF_SIZE];            /* for SegTracker name-copying */

/* Main ===========================================================
   The way we handle the list is rather unusual, so I'll explain it here.
   A single list is kept, and it may either be of libraries to report,
   or libraries to ignore.  The first use of the list determines which.
 */
VOID main(int argc, UBYTE *argv[]) {
    ULONG tmpPtr;
    struct MsgPort *t_mp;
    struct lm_Msg msg, *rmsg;
    int rc = RETURN_OK, inForbid=FALSE;
    BOOL done = FALSE;


    if (!argc) {
        /* just in case we really need to be a CLI if started from WB... */
        WB2CLI((struct WBStartup *)argv, 8192UL, DOSBase);
    }

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    if ((opts.del + opts.add + opts.deny) > 1L) {
        KPrintF("You may only specify one of:  ADD DEL DENY!\n");
        GoodBye(RETURN_FAIL);
    }

    l = &list;
    NewList(l);
    list.lh_Type = IGNORE_LIST;     /* ignore nothing :-) */

    Forbid();
    inForbid = TRUE;
    t_mp = FindPort(PORTNAME);
    if (t_mp) {       /* We must signal an existing instance of LibMon --------------------------------- */
        /* must have (Quit, List, or Stet) OR (Libs AND (Add, Del, or Deny)), otherwise it's pointless */
        if ((opts.quit) || (opts.list) || (opts.stet) || 
            ((opts.libs) && ((opts.add) || (opts.del) || (opts.deny)))) {
            mp = CreateMsgPort();
            if (!mp) {
                KPrintF("Cannot signal %s!\n", PROGNAME);
                GoodBye(RETURN_FAIL);
            }
            useDeleteMsgPort = TRUE;
            msg.msg.mn_ReplyPort = mp;
            msg.msg.mn_Length = (sizeof(struct lm_Msg));
            msg.list = NULL;   /* not needed by all */
            msg.libs = NULL;   /* not needed by all */

            if (opts.quit) {
                msg.cmd  = LM_CMD_QUIT;
                PutMsg(t_mp, (struct Message *)&msg);
                if (inForbid) {
                    Permit();
                    inForbid = FALSE;
                }
                WaitPort(mp);  /* wait for reply */
                rmsg = (struct lm_Msg *)GetMsg(mp);
                if (rmsg->cmd == LM_REPLY_QUIT) {
                    KPrintF("LibMon was shut down okay.\n");
                }
                else {
                    KPrintF("LibMon ignored the Quit message!!\n");
                    rc = RETURN_ERROR;
                }
                GoodBye(rc);
            }
            if (opts.del) {
                msg.cmd = LM_CMD_DEL;
                msg.libs = opts.libs;
                PutMsg(t_mp, (struct Message *)&msg);
                if (inForbid) {
                    Permit();
                    inForbid = FALSE;
                }
                WaitPort(mp);  /* wait for reply */
                rmsg = (struct lm_Msg *)GetMsg(mp);
                if (rmsg->cmd == LM_REPLY_OK) {
                    KPrintF("Libs deleted from list.\n");
                }
                else {
                    KPrintF("Error deleting libs from list!\n");
                    rc = RETURN_ERROR;
                }
            }
            if ((opts.add) || ((opts.libs) && (!((opts.deny) || (opts.del))))) {
                /* assume ADD if only LIBS specified */
                msg.cmd = LM_CMD_ADD;
                msg.libs = opts.libs;
                PutMsg(t_mp, (struct Message *)&msg);
                if (inForbid) {
                    Permit();
                    inForbid = FALSE;
                }
                WaitPort(mp);  /* wait for reply */
                rmsg = (struct lm_Msg *)GetMsg(mp);
                if (rmsg->cmd == LM_REPLY_OK) {
                    KPrintF("Added libs to list.\n");
                }
                else {
                    KPrintF("Error adding libs to list!\n");
                    rc = RETURN_ERROR;
                }
            }
            if (opts.deny) {
                msg.cmd = LM_CMD_DENY;
                msg.libs = opts.libs;
                PutMsg(t_mp, (struct Message *)&msg);
                if (inForbid) {
                    Permit();
                    inForbid = FALSE;
                }
                WaitPort(mp);  /* wait for reply */
                rmsg = (struct lm_Msg *)GetMsg(mp);
                if (rmsg->cmd == LM_REPLY_OK) {
                    KPrintF("Added libs to Deny list.\n");
                }
                else {
                    KPrintF("Error adding libs to Deny list!\n");
                    rc = RETURN_ERROR;
                }
            }
            if (opts.list) {
                msg.cmd = LM_CMD_LIST;
                PutMsg(t_mp, (struct Message *)&msg);
                if (inForbid) {
                    Permit();
                    inForbid = FALSE;
                }
                WaitPort(mp);  /* wait for reply */
                rmsg = (struct lm_Msg *)GetMsg(mp);
                if (rmsg->cmd == LM_REPLY_OK) {
                    /* Server MUST wait for our LM_CMD_DONE message! */
                    kPrintList(rmsg->list);
                    msg.cmd = LM_CMD_DONE;
                    PutMsg(t_mp, (struct Message *)&msg);
                    WaitPort(mp);
                    GetMsg(mp);
                }
                else {
                    KPrintF("Error obtaining List!\n");
                    rc = RETURN_ERROR;
                }
            }
            if (opts.stet) {
                msg.cmd = LM_CMD_NOLOW;
                PutMsg(t_mp, (struct Message *)&msg);
                if (inForbid) {
                    Permit();
                    inForbid = FALSE;
                }
                WaitPort(mp);  /* wait for reply */
                rmsg = (struct lm_Msg *)GetMsg(mp);
                if (rmsg->cmd == LM_REPLY_OK) {
                    KPrintF("Turned off lowercase-only.\n");
                }
                else {
                    KPrintF("Error turning off lowercase!\n");
                    rc = RETURN_ERROR;
                }
            }
        }
        else {
            Permit();
            KPrintF("Invalid options!  Must have Quit|List|Stet OR Libs+(Add|Del|Deny)!\n");
            rc = RETURN_FAIL;
        }
        GoodBye(rc);
    }
    else {   /* We're the LibMon Server/Workhorse ----------------------------------------- */
        UtilityBase = OpenLibrary("utility.library", 37L);
        if (!UtilityBase) {
            KPrintF(ERRMSG_LIBNOOPEN, "utility.library", 37L);
            GoodBye(RETURN_FAIL);
        }
        mp = CreatePort(PORTNAME, 0);
        if (!mp) {
            KPrintF("Can't create MessagePort!\n");
            GoodBye(RETURN_FAIL);
        }
        Permit();
        useDeleteMsgPort = FALSE;
    }

    /* deal with setup commands */
    /* set list type */
    if ((opts.add) && (opts.libs)) {
        /* list only specific libraries */
        list.lh_Type = ONLY_LIST;
    }
    if (opts.del) {
        /* ignore specfied (or emptylist==none) libraries */
        list.lh_Type = IGNORE_LIST;
    }
    if ((opts.deny) && (opts.libs)) {
        /* deny opens of specified libraries */
        list.lh_Type = DENIAL_LIST;
    }
    if (opts.libs) {
        if ((opts.deny + opts.add + opts.del) == 0L) {
            list.lh_Type = ONLY_LIST;
        }
        if (!addLibs(opts.libs)) {
            KPrintF("Trouble adding specified libraries; check listing.\n");
        }
    }
    if (opts.stet) {
        doNotLowerCase = TRUE;
    }
    if (opts.quit) {
        /* user is a moron */
        GoodBye(RETURN_WARN);
    }
    /* opts.list is quietly ignored by the server */

    memset(stackRAM, 0, MYSTACKSIZE);  /* paranoia */

    /* set the stack pointer to a place in our ~20K stack */
    tmpPtr = ((ULONG)stackRAM + MYSTACK_STARTP);
    tmpPtr += (32UL - (tmpPtr % 32UL));

    /* set up the stackswap structure */
    myStackP = &myStack;
    myStack.stk_Lower = stackRAM;
    myStack.stk_Upper = ((ULONG)stackRAM + MYSTACKSIZE);
    myStack.stk_Pointer = (APTR)tmpPtr;

    /* Find SegTracker semaphore */
    Forbid();
    segTrackSem = (struct SegSem *)FindSemaphore("SegTracker");
    /* do the setfunctions */
    oldOpenLib  = (VOID *(* ASM)(REG(a1) STRPTR, REG(d0) ULONG))SetFunction(SysBase, OPENLIB_OFFSET,  (ULONG (*) ()) myOpenLib);
    oldCloseLib = (VOID *(* ASM)(struct Library *))SetFunction(SysBase, CLOSELIB_OFFSET, (ULONG (*) ()) myCloseLib);
    Permit();

    /* wait til we're told to quit */
    while (!done) {
        WaitPort(mp);
        done = handleMsg(GetMsg(mp));
    }

    /* then restore the old library functions */
    Forbid();
    SetFunction(SysBase, OPENLIB_OFFSET,  (ULONG (*) ()) oldOpenLib);
    SetFunction(SysBase, CLOSELIB_OFFSET, (ULONG (*) ()) oldCloseLib);
    Permit();

    /* outta here */
    GoodBye(RETURN_OK);
}

/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {
    struct Node *n;

    while (n = RemHead(l)) {
        if ((n->ln_Name) && (TypeOfMem(n->ln_Name))) {   /* mungwall protection */
            FreeVec(n->ln_Name);
        }
        FreeVec(n);
    }

    if (UtilityBase) {
        CloseLibrary(UtilityBase);
    }

    if (rdargs) {
        FreeArgs(rdargs);
    }

    if (stackRAM) {
        FreeMem(stackRAM, MYSTACKSIZE);
    }

    if (mp) {
        if (useDeleteMsgPort) {
            DeleteMsgPort(mp);
        }
        else {
            DeletePort(mp);
        }
    }

    exit(rc);
}

/* doInit =============================================
 * Open libraries, call ReadArgs() if necessary.
 * Returns TRUE for success, FALSE otherwise.
 */
static LONG doInit(VOID) {
    LONG err;

    if (SysBase->lib_Version < 37L) {
        KPrintF("You need AmigaDOS 2.04 (V37) or better!\n");
        GoodBye(RETURN_FAIL);
    }

    /* theoretically, WB2CLI() should make this return as if there 
       were no options, if we were started from Workbench... 
     */
    rdargs = ReadArgs(TEMPLATE, (LONG *)&opts, NULL);
    if (!rdargs) {
        err = IoErr();
        PrintFault(err, PROGNAME);
        KPrintF("ReadArgs problem, err = %ld\n", err);
        return(FALSE);
    }

    stackRAM = allocLAligned(MYSTACKSIZE);
    if (!stackRAM) {
        PrintFault(IoErr(),PROGNAME);
        KPrintF("No RAM for alternate stack!\n");
        return(FALSE);
    }

    return(TRUE);
}


/* allocLAligned ================================================
   allocates longword-aligned RAM.  Needed for stack allocation.
   You must FreeMem(ptr,ramsize) when done.
   Returns ptr upon success or NULL upon failure.
 */
static APTR allocLAligned(ULONG size) {
    APTR ret = NULL, tmp;
    ULONG tmp2;
    
    if (!size) {
        return(NULL);
    }
    Forbid();   /* so nobody steals ram in between allocations */
    if (tmp = AllocVec((size+64UL),MEMF_ANY)) {
        tmp2 = ((ULONG)tmp + (32UL - ((ULONG)tmp % 32UL)));
        FreeVec(tmp);
        ret = AllocAbs(size, (APTR)tmp2);
    }
    Permit();
    return(ret);
}

/* openLibGuts ======================================================================================
   Called by asm routine myOpenLib().
 */
struct Library * ASM __saveds openLibGuts(REG(a2) STRPTR lib, REG(d2) ULONG ver, REG(a3) ULONG *stack) {
    struct Library *newLib=NULL;
    LONG            newLibVer=-1L;
    LONG            newLibCnt=-1L;
    STRPTR          segName = " ";
    STRPTR          taskName;
    ULONG           hunk=0UL, offset=0UL;
    SHORT           has_Relevance;


    taskName = getTaskName();
    segTrackBuf[0] = '\0';

/*
    KPrintF("OLG(%s, %lu, $%08lx) ($%08lx)\n", lib, ver, stack, *stack);
*/

    has_Relevance = isRelevant(lib, taskName);  /* Might be NULL taskName, but that's okay. */


    if (has_Relevance < MUST_DENY) {
        if (lib) {
            newLib = (*oldOpenLib)(lib, ver);
        }
        else {
            KPrintF("OpenLibrary(NULL, %ld) was called!\n", (LONG)ver);
        }
    }

    if (has_Relevance) {
        /* report it */
        if (newLib) {
            newLibVer = newLib->lib_Version;
            newLibCnt = newLib->lib_OpenCnt;
        }
        if (segTrackSem) {
            if (segTrackSem->seg_Find) {
                Forbid();
                if (segName = (*(segTrackSem->seg_Find))(*stack, &hunk, &offset)) {
                    stccpy(segTrackBuf, segName, STBUF_SIZE);
                }
                else {
                    segTrackBuf[0] = '\0';
                }
                Permit();
            }
        }
        KPrintF("%lc: %s V%ld (v: %ld) (c: %ld) (t: %s)\n   (a: $%08lx) (s: %s) (h: $%04lx) (o: $%08lx)\n",
                ((has_Relevance == MUST_DENY) ? ((LONG)'D') : ((LONG)'O')),
                lib, 
                ver, 
                newLibVer, 
                newLibCnt, 
                ((taskName) ? taskName : (STRPTR)" "),
                *stack,
                segTrackBuf,
                hunk,
                offset);
    }
    if (taskName) {
        if (TypeOfMem(taskName)) {
            /* mungwall protection */
            FreeVec(taskName);
        }
    }
    
    return(newLib);  /* possibly NULL */
}

/* closeLibGuts ======================================================================================
   Called by asm routine myCloseLib().  (which does a Forbid()/Permit() around this routine)
 */
VOID ASM __saveds closeLibGuts(REG(a2) struct Library *lib, REG(a3) ULONG *stack) {
    LONG            newLibVer=-1L;
    LONG            newLibCnt=-1L;
    STRPTR          segName = " ";
    STRPTR          taskName;
    STRPTR          libName = " ";
    ULONG           hunk=0UL, offset=0UL;
    SHORT           validLib = FALSE, badPtr = FALSE;

    taskName = getTaskName();
    segTrackBuf[0] = '\0';

    if (lib) {
        if (TypeOfMem(lib)) {
            libName = lib->lib_Node.ln_Name;
            validLib = TRUE;
        }
        else {
            badPtr = TRUE;
        }
    }

    /* who called us?  We need to know, in case of bad lib ptr or need to report */
    if (segTrackSem) {
        if (segTrackSem->seg_Find) {
            Forbid();
            if (segName = (*(segTrackSem->seg_Find))(*stack, &hunk, &offset)) {
                stccpy(segTrackBuf, segName, STBUF_SIZE);
            }
            else {
                segTrackBuf[0] = '\0';
            }
            Permit();
        }

    }

    if (validLib) {
        if (isRelevant(libName, taskName)) {
            newLibVer = lib->lib_Version;
            newLibCnt = ((LONG)(lib->lib_OpenCnt) - 1L);
            KPrintF("C: %s (v: %ld) (c: %ld) (t: %s)\n   (a: $%08lx) (s: %s) (h: $%04lx) (o: $%08lx)\n",
                    libName, 
                    newLibVer, 
                    newLibCnt, 
                    ((taskName) ? taskName : (STRPTR)" "),
                    *stack,
                    segTrackBuf,
                    hunk,
                    offset);
        }
    }
    else {
        if (badPtr) {
            KPrintF("***Bad CloseLibrary(%08lx)!\n  (t: %s) (a: $%08lx) (s: %s) (h: $%04lx) (o: $%08lx)\n",
                     lib,
                     ((taskName) ? taskName : (STRPTR)" "),
                     *stack,
                     segTrackBuf,
                     hunk,
                     offset);

        }
        else {
            KPrintF("CloseLibrary(NULL) from (t: %s)\n  (a: $%08lx) (s: %s) (h: $%04lx) (o: $%08lx)\n",
                     ((taskName) ? taskName : (STRPTR)" "),
                     *stack,
                     segTrackBuf,
                     hunk,
                     offset);
        }
    }

    if (taskName) {
        FreeVec(taskName);
    }


    /* get around to closing the library :-) */
    if (validLib) {
        (*oldCloseLib)(lib);
    }

    return;
}


/* handleMsg ==============================================================================
   server function for handling incoming msgs (only kind of msg, as a matter of fact)
   returns TRUE if a Quit command was received; FALSE otherwise.
*/
static BOOL handleMsg(struct Message *amsg) {
    struct lm_Msg *msg1, *msg2;
    BOOL doneList = FALSE;

    msg1 = (struct lm_Msg *)amsg;

    switch (msg1->cmd) {
        case LM_CMD_QUIT:
            msg1->cmd = LM_REPLY_QUIT;
            ReplyMsg(amsg);
            Forbid();
            while (msg2 = (struct lm_Msg *)GetMsg(mp)) {
                msg2->cmd = LM_REPLY_FAIL;
                ReplyMsg((struct Message *)msg2);
            }
            DeletePort(mp);
            mp = NULL;
            Permit();
            return(TRUE);
            break;

        case LM_CMD_ADD:
            if (IsListEmpty(l)) {                    /* macro from exec/lists.h */
                l->lh_Type = ONLY_LIST;
                denyOpens = FALSE;
/* debug        KPrintF("Changing list type to ONLY, denyOpens=False\n"); */
            }
            if (addLibs(msg1->libs)) {
                msg1->cmd = LM_REPLY_OK;
            }
            else {
/* debug        KPrintF("CMD_ADD: addLibs() failed!\n"); */
                msg1->cmd = LM_REPLY_FAIL;
            }
            ReplyMsg(amsg);
            break;

        case LM_CMD_DENY:
            if (IsListEmpty(l)) {
                /* request is to make a deny list and add libs to it */
                l->lh_Type = DENIAL_LIST;
                denyOpens = TRUE;
/* debug        KPrintF("Changing list type to Denial, denyOpens=True\n"); */
            }
            if (l->lh_Type == DENIAL_LIST) {
                /* only add to it if it's a deny list */
                if (addLibs(msg1->libs)) {
                    msg1->cmd = LM_REPLY_OK;
                }
                else {
/* debug            KPrintF("CMD_DENY: addLibs() failed!\n"); */
                    msg1->cmd = LM_REPLY_FAIL;
                }
            }
            else {
/* debug        KPrintF("List type is %ld, not DENIAL_LIST!\n", (LONG)(l->lh_Type)); */
                msg1->cmd = LM_REPLY_FAIL;
            }
            ReplyMsg(amsg);
            break;

        case LM_CMD_DEL:
            if (IsListEmpty(l)) {
                /* request is to add libs to an ignore list... */
                l->lh_Type = IGNORE_LIST;
                denyOpens = FALSE;
/* debug        KPrintF("Changing list type to IGNORE, denyOpens=False\n"); */
                if (addLibs(msg1->libs)) {
                    msg1->cmd = LM_REPLY_OK;
                }
                else {
/* debug            KPrintF("CMD_DEL: addLibs() failed!\n"); */
                    msg1->cmd = LM_REPLY_FAIL;
                }
            }
            else {
                /* request is to delete libs from list */
                if (delLibs(msg1->libs)) {  
                    /* delLibs will reset to IGNORE_LIST if list becomes empty */
                    msg1->cmd = LM_REPLY_OK;
                }
                else {
/* debug            KPrintF("CMD_DEL: delLibs() failed!\n"); */
                    msg1->cmd = LM_REPLY_FAIL;
                }
            }
            ReplyMsg(amsg);
            break;

        case LM_CMD_NOLOW:
            doNotLowerCase = TRUE;
            msg1->cmd = LM_REPLY_OK;
            ReplyMsg(amsg);
            break;

        case LM_CMD_LIST:
            /* this one is tricky.  Hand out list pointer, then Ignore ALL Msgs *BUT* LM_CMD_DONE. */
            msg1->list = l;
            msg1->cmd = LM_REPLY_OK;
            ReplyMsg(amsg);
            while (!doneList) {
                WaitPort(mp);
                msg2 = (struct lm_Msg *)GetMsg(mp);
                if (msg2->cmd == LM_CMD_DONE) {
                    msg2->cmd = LM_REPLY_OK;
                    doneList = TRUE;
                }
                else {
                    msg2->cmd = LM_REPLY_FAIL;
                }
                ReplyMsg((struct Message *)msg2);
            }
            break;

        default:
            msg1->cmd = LM_REPLY_FAIL;
            ReplyMsg(amsg);
            break;
    }
    return(FALSE);
}

/* delLibs ===========================================================================================
   delete libraries from list 
 */
BOOL delLibs(STRPTR *theLibs) {
    UBYTE abuf[80];
    int i;
    struct Node *n;
    BOOL retVal = TRUE;

    if (!theLibs)
        return(FALSE);

    for (i = 0; theLibs[i] != NULL; i++) {
        memset(abuf,0,80);
        if (!doNotLowerCase) {
            strLCpy(abuf, theLibs[i]);
        }
        else {
            strcpy(abuf, theLibs[i]);
        }
        if (n = FindName(l, abuf)) {
            Remove(n);
            if ((n->ln_Name) && TypeOfMem(n->ln_Name)) {  /* mungwall protection again */
                FreeVec(n->ln_Name);
            }
            FreeVec(n);
        }
        else {
            retVal = FALSE;
/* debug    KPrintF("Can't delete %s: not found!\n", abuf); */
        }
    }

    if (IsListEmpty(l)) {
        l->lh_Type = IGNORE_LIST;
/* debug  KPrintF("delLibs: empty list changed to IGNORE.\n"); */
    }

    return(retVal);
}

/* addLibs ==================================================================================================
   add libraries to list; allocate new string, too 
 */
BOOL addLibs(STRPTR *theLibs) {
    BOOL retVal = TRUE;
    int i;
    ULONG strLen;
    STRPTR newStr;
    struct Node *newNode;



    if (!theLibs) {
/* debug  KPrintF("addLibs(NULL) happened!!!\n"); */
        return(FALSE);
    }

    for (i = 0; theLibs[i] != NULL; i++) {
        strLen = (ULONG)strlen(theLibs[i]) + 1UL;
        if (newStr = AllocVec(strLen, MEMF_CLEAR)) {
            if (!doNotLowerCase) {
                strLCpy(newStr, theLibs[i]);
            }
            else {
                strcpy(newStr, theLibs[i]);
            }

            if (FindName(l, newStr)) {
                /* Do not add a duplicate name.  But duplicates are not errors. */
/* debug        KPrintF("Duplicate not added to list: %s\n", newStr);  */
                FreeVec(newStr);
            }
            else {
                if (newNode = AllocVec(sizeof(struct Node), MEMF_CLEAR)) {
                    newNode->ln_Type = l->lh_Type;
                    newNode->ln_Name = newStr;
                    AddHead(l, newNode);
/* debug            KPrintF("Added %s to list\n", newStr); */
                    /* retVal remains TRUE */
                }
                else {
                    FreeVec(newStr);
                    retVal = FALSE;
/* debug            KPrintF("addLibs: Couldn't allocate new node!\n"); */
                }
            }
        }
        else {
            retVal = FALSE;
/* debug    KPrintF("addLibs: Couldn't allocate new string!\n"); */
        }
    }

    return(retVal);
}

/* strLCpy ====================================================================================
   string copy, converting to lowercase.  Does NOT check validity of pointers or length
   of destination.  So, be sure to feed in valid pointers, with strlen(dest) >= strlen(src)!
*/
static VOID strLCpy(STRPTR destStr, STRPTR srcStr) {
    STRPTR s1 = destStr, s2 = srcStr;
    
    while (*s2) {
        *s1++ = ToLower(*s2++);
    }
    
    return;
}

/* isRelevant ===============================================================================
   determine if libName & task are relevant to current operation
   0 = irrelevant   (NOT_RELEVANT)
   1 = relevant     (IS_RELEVANT)
   2 = relevant     (MUST_DENY)  do NOT open library unless theTask==specialTask
*/
SHORT isRelevant(STRPTR libName, STRPTR theTask) {


    if (FindName(l, libName)) {
        /* it's listed */
        switch (l->lh_Type) {
            case DENIAL_LIST:
                if (!theTask) {
                    return(MUST_DENY);
                }
                if (!Stricmp(theTask, specialTask)) {
                    return(IS_RELEVANT);                /* cheat so we don't deny LibMon :-) */
                }
                else {
                    return(MUST_DENY);
                }
                break;
            case ONLY_LIST:
                return(IS_RELEVANT);
                break;
            case IGNORE_LIST:
                return(NOT_RELEVANT);
                break;
            default:
                return(IS_RELEVANT);
                break;
        }
    }


    /* If it wasn't on our list, we still have to find out what to do... */
    switch (l->lh_Type) {
        case ONLY_LIST:
            return(NOT_RELEVANT);
            break;
        case DENIAL_LIST:
        case IGNORE_LIST:
        default:
            return(IS_RELEVANT);
            break;
    }

    return(IS_RELEVANT);       /* but we shouldn't ever get here */
}


/* kPrintList =========================================================================
   dump out the current available list
 */
static VOID kPrintList(struct List *lml) {
    struct Node *n;
    
    if (!lml)
        return;

    switch (lml->lh_Type) {
        case DENIAL_LIST:
            KPrintF("\nList of libraries to %s:\n", "DENY");
            break;
        case ONLY_LIST:
            KPrintF("\nList of libraries to %s:\n", "REPORT");
            break;
        case IGNORE_LIST:
            KPrintF("\nList of libraries to %s:\n", "IGNORE");
            break;
        default:
            KPrintF("\nList of libraries (type unknown(!)):\n");
            break;
    }
    
    n = lml->lh_Head;
    while (n) {
        if ((n->ln_Succ) && (n->ln_Name)) {
            KPrintF("    %s\n", n->ln_Name);
        }
        n = n->ln_Succ;
    }
    KPrintF("End of Library Listing\n\n");
    
    return;
}
