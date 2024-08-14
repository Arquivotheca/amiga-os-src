/* StoreNV.c
 * 
 * Test of nonvolatile.library/StoreNV()
 * 
 * 
 */

/* Includes --------------------------------------------- */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <libraries/nonvolatile.h>
#include <utility/tagitem.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/nonvolatile_protos.h>
#include <stdlib.h>
#include <string.h>

#include <pragmas/nonvolatile_pragmas.h>

#include "nv.h"   /* local defaults for many programs */

/* Defines ------------------------------------------ */
#define PROGNAME "StoreNV"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define ERRMSG_MX_OPTS   "Only one of DATA/K,FILE/K,BUFSIZE/K/N may be specified!\n"
#define TEMPLATE "NAME=APPNAME/K,ITEM=ITEMNAME/K,DATA/K,FILE=DATAFILE/K,BUFSIZE/K/N,BUFFILL/N,NULL=NULLTEST/S,ZERO=ZEROTEST/S,KILLREQ/S,TRUEBUFSIZE=BETRUE/S,NOLIE=NOROUNDTONV/S"


/* Structs ------------------------------ */
struct Opts {
    STRPTR name;                /* application name, default NVTest      */
    STRPTR item;                /* data nv-storage name, default Test01  */
    STRPTR data;                /* string of data to be stored           */
    STRPTR file;                /* file containing data to be stored     */
    ULONG *bufsize;             /* buffer size for optional data pattern */
    LONG  *buffill;             /* buffer pattern                        */
                                /* Note that only one of data|file|bufsize may be specified */
    LONG nullTest;              /* feed StoreNV a null data pointer        */
    LONG zeroTest;              /* feed StoreNV valid pointer, but 0 bytes */
    LONG killReq;               /* kill requesters for "nv" disk...        */
    LONG beTrue;                /* do NOT do roundup by 10, probably crash */
    LONG noLieAboutSize;        /* don't round size of buffer upwards for feed to StoreNV() */
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
static LONG findFileSize(BPTR);
static ULONG lieAboutLength(ULONG);
static ULONG roundUp(ULONG);

/* Globals --------------------------------------- */
struct Opts     opts;
struct RDArgs  *rdargs;
struct Library *NVBase;
BPTR            fh;
STRPTR          buf;


VOID main(int argc, UBYTE *argv[]) {
    LONG fileSize, amtRead;
    STRPTR item, name;
    STRPTR storePtr=NULL;        /* ptr to data to store */
    ULONG storeSize=0UL;         /* byte length of data, fed to StoreNV */
    UWORD snvErr;
    int rc=RETURN_OK, i;
    LONG ioErr;
    BOOL killReq;
    ULONG allocSize;             /* for memory allocations */

    if (!doInit()) {
        /* handles mutual exclude of file|data|buffer, too... */
        GoodBye(RETURN_FAIL);
    }

    killReq = ((opts.killReq) ? TRUE : FALSE);

    /* File options.... read file and put it into a buffer. */
    if (opts.file) {
        fh = Open(opts.file, MODE_OLDFILE);
        if (!fh) {
            PrintFault(IoErr(),PROGNAME);
            Printf("Cannot open file '%s'!\n", opts.file);
            GoodBye(RETURN_FAIL);
        }
        fileSize = findFileSize(fh);
        if (fileSize < 0L) {
            Printf("Cannot use file with size %ld!\n", fileSize);
            GoodBye(RETURN_FAIL);
        }
        allocSize = ((opts.beTrue) ? (ULONG)fileSize : roundUp(fileSize));
        buf = AllocVec(allocSize, MEMF_CLEAR);
        if (!buf) {
            PrintFault(IoErr(),PROGNAME);
            Printf("Cannot allocate %lu bytes of RAM for buffer!\n", allocSize);
            GoodBye(RETURN_FAIL);
        }
        amtRead = Read(fh, buf, fileSize);
        if (amtRead < 0L) {
            PrintFault(IoErr(),PROGNAME);
            PutStr("Error reading; will use storage size of zero.\nHit Control-C to abort.\n");
            Delay(100L);
            if (CheckSignal(SIGBREAKF_CTRL_C)) {
                GoodBye(RETURN_WARN);
            }
        }
        Printf("Read %ld bytes into buffer.\n", amtRead);
        storeSize = (ULONG)((amtRead >= 0L) ? amtRead : 0UL);
        storePtr = buf;
    }

    if (opts.data) {
        /* user has provided string to store */
        storeSize = strlen(opts.data) + 1L;   /* include the NULL */
        allocSize = ((opts.beTrue) ? (ULONG)storeSize : roundUp(storeSize));
        buf = AllocVec(allocSize, MEMF_CLEAR);
        if (!buf) {
            PrintFault(IoErr(),PROGNAME);
            Printf("Cannot allocate %lu bytes of RAM for buffer!\n", allocSize);
            GoodBye(RETURN_FAIL);
        }
        for (i = 0; i < storeSize; i++) {
            buf[i] = opts.data[i];
        }
        storePtr = buf;
    }
    if (opts.bufsize) {
        allocSize = ((opts.beTrue) ? *opts.bufsize : roundUp(*opts.bufsize));
        buf = AllocVec(allocSize, MEMF_CLEAR);
        if (!buf) {
            PrintFault(IoErr(),PROGNAME);
            Printf("Cannot allocate %lu bytes of RAM for buffer!\n", allocSize);
            GoodBye(RETURN_FAIL);
        }
        if (opts.buffill) {
            memset(buf,(UBYTE)(*opts.buffill), allocSize);
        }
        storeSize = *opts.bufsize;   /* not allocSize! */
        storePtr  = buf;
    }

    /* nvlib likes its data length in *tens* of bytes.  I have no clue
       how they expect this to not cause enforcer hits.
     */
    if ((!(opts.beTrue)) && (!(opts.noLieAboutSize))) {
        /* this rounds up storeSize, then divides it by 10 */
        storeSize = lieAboutLength(storeSize);
    }

    item = ((opts.item) ? opts.item : (STRPTR)DEFAULT_ITEM_NAME);
    name = ((opts.item) ? opts.name : (STRPTR)DEFAULT_APP_NAME);


    if (opts.nullTest) {
        storePtr = NULL;
    }
    if (opts.zeroTest) {
        storeSize = 0UL;
    }

    Printf("Calling StoreNV(%s, %s, $%08lx, %lu, %lu)...\n",
            name, item, storePtr, storeSize, (ULONG)killReq);

    Delay(10L);  /* if it's gonna crash, let it print first! */

    /* StoreNV() call ---------------------------------------- */
    snvErr = StoreNV(name, item, storePtr, storeSize, killReq);
    ioErr = IoErr();

    if (!snvErr) {
        PutStr("StoreNV() reported success.\n");
    }
    else {
        rc = RETURN_ERROR;
        Printf("StoreNV() reported error %lu:\n", (ULONG)(snvErr));
        switch (snvErr) {
            case NVERR_BADNAME:
                PutStr("  Error in AppName or ItemName.\n");
                break;
            case NVERR_WRITEPROT:
                PutStr("  Nonvolatile storage is read-only.\n");
                break;
            case NVERR_FAIL:
                PutStr("  Failure in writing data.\n");
                break;
            case NVERR_FATAL:
                PutStr("  Fatal error, possible loss of previously-saved data.\n");
                break;
            default:
                PutStr("  Unknown error!\n");
                break;
        }
    }
    Printf("IoErr was %ld  ", ioErr);
    PrintFault(ioErr,NULL);
    if (!ioErr) {
        PutStr("\n");  /* in this case, PrintFault() didn't do a newline... */
    }


    GoodBye(rc);
}

/* roundUp ======================================================
    round a number up to the closest one evenly divisible by 10.
    intended for memory allocations.
 */
static ULONG roundUp(ULONG oldSize) {

    if (!(oldSize % 10))
        return(oldSize);

    return(oldSize + (oldSize % 10));
}

/* findFileSize ==================================================
   Examine()s the file to find its size (and whether it's a file).
   Returns -1 for error.
*/
static LONG findFileSize(BPTR f) {
    struct FileInfoBlock *fib;
    LONG result = -1L;
    
    if (fib = AllocDosObjectTags(DOS_FIB, TAG_DONE)) {
        if (ExamineFH(fh, fib)) {
            if (fib->fib_DirEntryType < 0L) {
                /* ie, it's a file */
                result = fib->fib_Size;
            }
            else {
                PutStr("Please specify a *file*, not a directory!\n");
            }
        }
        else {
            PutStr("Couldn't ExamineFH() the file!\n");
        }
        FreeDosObject(DOS_FIB, fib);
    }
    else {
        PutStr("Couldn't allocate RAM for FileInfoBlock!\n");
    }
    
    return(result);
}

/* lieAboutLength ===============================================================
   simply round off the given number up to the next number evenly divisible by 10.
   Return that number divided by ten.
 */
static ULONG lieAboutLength(ULONG syze) {
    SHORT odd;

    if (odd = syze % 10) {
        syze += (ULONG)(10 - odd);
    }
    return(syze/10UL);
}

/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {

    if (fh) {
        Close(fh);
    }

    if (buf) {
        FreeVec(buf);
    }

    if (NVBase) {
        CloseLibrary(NVBase);
    }

    if (rdargs) {
        FreeArgs(rdargs);
    }

    exit(rc);
}

/* doInit =============================================
 * Open libraries, call ReadArgs() if necessary.
 * Returns TRUE for success, FALSE otherwise.
 */
static LONG doInit(VOID) {
    int mx=0;

    rdargs = ReadArgs(TEMPLATE, (LONG *)&opts, NULL);
    if (!rdargs) {
        PrintFault(IoErr(), PROGNAME);
        return(FALSE);
    }

    NVBase = OpenLibrary("nonvolatile.library", 40L);
    if (!NVBase) {
        Printf(ERRMSG_LIBNOOPEN, "nonvolatile.library", 40L);
        return(FALSE);
    }

    /* check mutual-exclude options */
    if (opts.data) {
        mx++;
    }
    if (opts.file) {
        mx++;
        if (mx >= 2L) {
            PutStr(ERRMSG_MX_OPTS);
            return(FALSE);
        }
    }
    if (opts.bufsize) {
        mx++;
        if (mx >= 2L) {
            PutStr(ERRMSG_MX_OPTS);
            return(FALSE);
        }
    }

    if ((!mx) && (!opts.nullTest)) {
        PutStr("You must specify a data source (NULL for none).\n");
        return(FALSE);
    }

    return(TRUE);
}

