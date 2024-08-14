/* getdevproc() test, SAS/C 6.2 */

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdlib.h>
#include <string.h>



#ifdef DEBUG
#define Printf KPrintF
#define PutStr KPutStr
extern VOID KPrintF(STRPTR, ...);
extern VOID KPutStr(STRPTR);
#endif

/* DVPF_(UNLOCK|ASSIGN) are interesting in the dvp_Flags field */

#define PROGNAME "GetDevProc"
#define TEMPLATE "NAME/A,DELAY/N,ALL/S"
#define BUFSIZE 4096
struct Opts {
    STRPTR name;            /* name to get info on */
    LONG  *delay;           /* seconds to delay in exit */
    LONG   all;             /* loop on name */
};

/* well, SAS/C 6.2 seems to have trouble with this sucker inside the
   union within DosList.  So we'll try putting it here.
 */
struct dol_handler {
    BSTR    dol_Handler;    /* file name to load if seglist is null */
    LONG    dol_StackSize;  /* stacksize to use when starting process */
    LONG    dol_Priority;   /* task priority when starting process */
    ULONG   dol_Startup;    /* startup msg: FileSysStartupMsg for disks */
    BPTR    dol_SegList;    /* already loaded code for new task */
    BPTR    dol_GlobVec;    /* BCPL global vector to use when starting
                             * a process. -1 indicates a C/Assembler
                             * program. */
};

struct RDArgs *rdargs;
struct Opts opts;
struct DevProc *dp, *dp2, *origDP;
UBYTE  buf[BUFSIZE];

static VOID GoodBye(int);
static VOID displayHandler(struct dol_handler *);
static VOID displayDP(struct DevProc *);

VOID main(int argc, UBYTE *argv[]) {
    LONG ioerr;

    origDP = dp = dp2 = NULL;

    rdargs = ReadArgs(TEMPLATE, (LONG *)&opts, NULL);
    if (!rdargs) {
        PrintFault(IoErr(), PROGNAME);
        PutStr("Usage:  gdproc <name> [all, means each part of multiple assign]\n");
        GoodBye(RETURN_WARN);
    }

    Printf("Src: dp ($%08lx) = GetDeviceProc(opts.name='%s', dp2=$%08lx)\n",
            dp, opts.name, dp2);
    while (dp = GetDeviceProc(opts.name, dp2)) {
        ioerr = IoErr();
        Printf("Called GetDeviceProc(), IoErr() is now %ld\n", ioerr);
        if (!dp) {
            PrintFault(ioerr, PROGNAME);
            Printf("No info on '%s', ioerr=%ld!\n", opts.name, ioerr);
            GoodBye(RETURN_FAIL);
        }

        if (!origDP)
            origDP = dp;

        Printf("\n------\nGot DevProc $%08lx\nIoErr contains %ld --", dp, ioerr);
        if (ioerr)
            PrintFault(ioerr,NULL);
        else
            PutStr("\n");

        /* display interesting info about returned DevProc structure */
        displayDP(dp);

        /* break out of loop unless user said 'ALL' */
        if (!(opts.all))
            break;

        /* break out of program if user hits Ctrl-C */
        if (CheckSignal(SIGBREAKF_CTRL_C))
            GoodBye(RETURN_WARN);

        /* get ready for next time 'round */
        if (dp2) {
            Printf("Thought about freeing dp2 ($%08lx)\n", dp2);
            /* FreeDeviceProc(dp2); */
        }
        dp2 = dp;
/*         dp = NULL; */
        Printf("Src: dp ($%08lx) = GetDeviceProc(opts.name='%s', dp2=$%08lx)\n",
                dp, opts.name, dp2);
    }
    ioerr = IoErr();
    Printf("Out of loop, ioerr=%ld\n", ioerr);
/*
    if (dp) {
        PutStr("\n********** Info on DP:\n");
        displayDP(dp);
    }
*/
/*
    if (dp2) {
        PutStr("\n********** Info on DP2:\n");
        displayDP(dp);
    }
*/
/*
    if (origDP) {
        PutStr("\n********** Info on origDP:\n");
        displayDP(origDP);
    }
*/
    PutStr("=======================\n");
    if (ioerr) {
        PrintFault(ioerr, PROGNAME);
        Printf("No more info on '%s', ioerr=%ld!\n", opts.name, ioerr);
        if ((ioerr != ERROR_NO_MORE_ENTRIES) && (ioerr != RETURN_OK))
            GoodBye(RETURN_FAIL);
    }

    GoodBye(RETURN_OK);
}

/* displayHandler ==============================================================
 */
static VOID displayHandler(struct dol_handler *dh) {
    STRPTR fName;
    LONG fNameLen;

    PutStr("Info on handler:\n");    
    if (dh->dol_Handler) {
        /* Another evil BSTR.  Print the sucker. */
        fName = BADDR(dh->dol_Handler);
        fNameLen = (LONG)(*fName);
        fName++;
        PutStr("  FileName to load if SegList is NULL:  '");
        Flush(Output());
        Write(Output(), fName, fNameLen);
        Flush(Output());
        PutStr("'\n");
    }
    else
        PutStr("  No handler name info.\n");

    Printf("  StackSize:  $%08lx\n   Priority:  $%08lx\n    Startup:  $%08lx\n    SegList:  $%08lx\n    GlobVec:  $%08lx\n",
        dh->dol_StackSize,
        dh->dol_Priority,
        dh->dol_Startup,
        BADDR(dh->dol_SegList),
        BADDR(dh->dol_GlobVec));
}


static VOID GoodBye(int rc) {
    Printf("In Exit; we have the following:\n    %s %s %s\n",
           ((rdargs) ? "rdargs" : "no_rdargs"),
           ((dp)     ? "dp"     : "no_dp"),
           ((dp2)    ? "dp2"    : "no_dp2"));
    if (rdargs) {
        if (opts.delay) {
            Printf("Delaying %ld seconds...\n", *opts.delay);
            Delay(*opts.delay * 50L);
        }
        Printf("Freeing ReadArgs $%08lx\n",rdargs);
        FreeArgs(rdargs);
        opts.name = NULL;
    }
    if (dp) {
        Printf("Freeing DevProc1 $%08lx:\n", dp);
        displayDP(dp);
        FreeDeviceProc(dp);
    }
#ifdef CRASH_THE_BLOODY_SYSTEM
    if (dp2 && (dp2 != (struct DevProc *)0xDEADBEEF)) {
        Printf("Freeing DevProc2 $%08lx\n", dp2);
        displayDP(dp2);
        FreeDeviceProc(dp2);

    }
#endif /* CRASH_THE_BLOODY_SYSTEM */
    Printf("Exiting with rc=%ld\n", rc);
    exit(rc);
}

#define MAGICBYTES (sizeof(struct DevProc) / sizeof(LONG))

static VOID displayDP(struct DevProc *dpr) {
    struct DosList *dl;
    STRPTR dlName, optName;
    LONG dlNameLen;
    ULONG i, *dprContents;


    optName = (opts.name ? opts.name : (STRPTR)"UNKNOWN!");

    if (!dpr) {
        Printf("  Structure contents for '%s' at $%08lx were freed.\n", optName, dpr);
        return;
    }

    /* MAJOR LEAGUE ANTI-MUNGWALL KLUDGE */
    dprContents = (ULONG *)dpr;
    for (i = 0; i < (ULONG)MAGICBYTES; i++) {
        if (dprContents[i] == (ULONG)0xDEADBEEF) {
            Printf("  Structure contents for '%s' at $%08lx were freed.\n", optName, dpr);
            return;
        }
        else {
            ; /* yes, blank.  Otherwise, compiler generates crashing code! */
        }
    }

    Printf("Info on '%s' (struct at $%08lx):\n", optName, dpr);
    Printf("   Port:  $%08lx\n   Lock:  $%08lx\n  Flags:  $%08lx\n   Node:  $%08lx\n",
              dpr->dvp_Port,
              BADDR(dpr->dvp_Lock),
              dpr->dvp_Flags,
              dpr->dvp_DevNode);
    PutStr("  Flags include:  ");
    if (dpr->dvp_Flags & DVPF_UNLOCK)
        PutStr("Unlock ");
    if (dpr->dvp_Flags & DVPF_ASSIGN)
        PutStr("Assign ");
    if (!(dpr->dvp_Flags))
        PutStr("None! ");
    PutStr("\n");
    if (dpr->dvp_Lock) {
        if (DOSTRUE == NameFromLock(dpr->dvp_Lock, buf, BUFSIZE))
            Printf("  NameFromLock: '%s'\n", buf);
    }

    /* anti-Mungwall kludge, too */
    if (dpr->dvp_DevNode && (dpr->dvp_DevNode != (struct DosList *)0xDEADBEEF)) {
        dl = dpr->dvp_DevNode;
        if (dl->dol_Name) {
            /* this revolting beast is a BPTR to a BString...
               so we must make it a cptr, and then get its length from the first
               "character" of the string.  The remnants are the real string, not
               necessarily null-terminated.  Want a membership in the ever-expanding
               "I hate BCPL!" club?
             */
            dlName = BADDR(dl->dol_Name);
            dlNameLen = (LONG)(*dlName);
            dlName++;  /* NOTE: NOT NULL-Terminated! */
            PutStr("  DosList Name:  ");
            Flush(Output());
            Write(Output(), dlName, dlNameLen);
            Flush(Output());
            PutStr("\n");
        }
        PutStr("  DosList Type:  ");
        switch (dl->dol_Type) {
            case DLT_DEVICE:
                PutStr("Device\n");
                displayHandler(&(dl->dol_misc.dol_handler));
                break;
            case DLT_DIRECTORY:
                PutStr("Directory\n");
                Printf("Assigned name: '%s'\n", dl->dol_misc.dol_assign.dol_AssignName);
                break;
            case DLT_VOLUME:
                PutStr("Volume\n");
                Printf("Disk Type: $%08lx\n", dl->dol_misc.dol_volume.dol_DiskType);
                break;
            case DLT_LATE:
                PutStr("Late Assign\n");
                Printf("Assigned name: '%s'\n", dl->dol_misc.dol_assign.dol_AssignName);
                break;
            case DLT_NONBINDING:
                PutStr("Non-binding Assign\n");
                Printf("Assigned name: '%s'\n", dl->dol_misc.dol_assign.dol_AssignName);
                break;
            case DLT_PRIVATE:
                PutStr("Private (hands-off!)\n");
                break;
            default:
                Printf("Unknown ($%08lx)!!\n", (ULONG)(dl->dol_Type));
                break;
        }
    }
}
