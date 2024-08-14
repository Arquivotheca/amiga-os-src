/* StuffNV.c
 * 
 * Purpose: fill NVRam as quickly as possible.
 * Protect each item unless directed not to.
 * 
 */

/* Includes --------------------------------------------- */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <libraries/nonvolatile.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/nonvolatile_protos.h>

#include <stdlib.h>
#include <string.h>

#include <pragmas/nonvolatile_pragmas.h>

/* Defines ------------------------------------------ */
#define PROGNAME "StuffNV"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "NAME/K,ITEMS/K/N,BYTES=BYTESPERITEM/K/N,NOPROTECT/S,OVERRUN/S"
#define INAME_SIZE 80     /* size of itemName buffer */
#define DEFAULT_APPNAME PROGNAME
#define ITEM_ROOT_NAME "TestItem"

/* Structs ------------------------------ */
struct Opts {
    STRPTR name;            /* appnameroot to use */
    LONG *items;            /* number of items */
    LONG *bytes;            /* bytes per item */
    LONG  noProtect;        /* don't protect? */
    LONG  overrun;          /* try to store too much? */
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
extern VOID bsprintf(STRPTR,STRPTR,...);  /* errrpt.lib; sprintf() using RawDoFmt() */
static VOID storeItem(STRPTR, ULONG, ULONG);
static VOID protectItem(STRPTR, STRPTR);
static ULONG findItemCount(ULONG);

/* Globals --------------------------------------- */
struct Opts     opts;
struct RDArgs  *rdargs;
struct Library *NVBase;
UBYTE  itemName[INAME_SIZE];
UBYTE  appName[INAME_SIZE];
UBYTE  tmpBuf[INAME_SIZE];
int rc = RETURN_OK;
ULONG  freeStore=10UL;
STRPTR myData;

VOID main(int argc, UBYTE *argv[]) {
    ULONG itemCount=0;
    ULONG bytesPerItem;
    BOOL doProtect=TRUE;
    int i;

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    myData = NULL;

    /* do we protect or not? */
    if (opts.noProtect) {
        doProtect = FALSE;
    }

    /* use user's idea of a good name? */
    if (opts.name) {
        stccpy(appName, opts.name, INAME_SIZE-1);
    }
    else {
        stccpy(appName, DEFAULT_APPNAME, INAME_SIZE-1);
    }

    /* user's item count, or fill to order? */
    if (opts.bytes) {
        bytesPerItem = *opts.bytes;
    }
    else {
        bytesPerItem = 10UL;
    }

    if (opts.items) {
        itemCount = *opts.items;
    }
    else {
        itemCount = findItemCount(bytesPerItem);
    }

    if (opts.overrun) {
        itemCount += 2UL;
    }


    strcat(appName, "1");
    for (i = 0; i < itemCount-2; i++) {
        storeItem(appName, i, bytesPerItem);  /* changes itemName */
        if (doProtect) {
            protectItem(appName, itemName);
        }
        if (CheckSignal(SIGBREAKF_CTRL_C)) {
            GoodBye(rc);
        }
    }

    appName[(strlen(appName)-1)] = '2';
    storeItem(appName, 1, bytesPerItem);
    if (doProtect) {
        protectItem(appName, itemName);
    }

    GoodBye(rc);
}
/* storeItem ====================================================
   feed it appname, itemnumber, bytesperitem
   If the buffers doesn't already exist, create it;
   use existing one otherwise.  This is an ugly kludge, added
   in solely for additional speed between calls to StoreNV().

   WE RELY ON "bytes" BEING THE SAME FROM CALL TO CALL!
   (Yuck; should've written things differently in the 1st place...)

*/
static VOID storeItem(STRPTR aName, ULONG num, ULONG bytes) {
    UWORD err;
    ULONG roundedBytes=bytes, nvBytes10;

    bsprintf(itemName, "%s%ld", ITEM_ROOT_NAME, num);
    if (bytes % 10UL) {
        roundedBytes += (10UL - (bytes % 10UL));
    }
    nvBytes10 = roundedBytes / 10UL;

    if (!myData) {
        if (NULL == (myData = AllocVec(roundedBytes, MEMF_ANY))) {
            Printf("Couldn't allocate RAM for item %ld!\n", num);
            return;
        }
    }

    memset(myData,(UBYTE)num, roundedBytes);
    err = StoreNV(aName, itemName, myData, nvBytes10, FALSE);
    Printf("StoreNV(%s,%s,%08lx,%lu) returned %lu\n", 
            aName, itemName, myData, nvBytes10, (ULONG)err);
    if (err) {
        rc = RETURN_ERROR;
    }

    return;
}

/* protectItem ====================================================
   feed it appname and itemname
*/
static VOID protectItem(STRPTR aName, STRPTR iName) {
    BOOL success;

    success = SetNVProtection(aName, itemName, NVEF_DELETE, FALSE);
    Printf("Protect of %s returned %ld\n", itemName, (LONG)success);
    if (!success) {
        rc = RETURN_ERROR;
    }
    return;    
}

/* findItemCount ===================================================
   freeStore/bytesPerItem
 */
static ULONG findItemCount(ULONG bytes) {
    struct NVInfo *ni;
    ULONG itemcount;

    if (!bytes) {
        PutStr("Bad, bad user.  ByteCount must be non-zero!\n");
        GoodBye(RETURN_FAIL);
    }

    if (ni = GetNVInfo(FALSE)) {
        freeStore = ni->nvi_FreeStorage * 10UL;  /* we work in real bytes here :-) */
        FreeNVData(ni);
    }
    else {
        PutStr("Couldn't GetNVInfo()!\n");
        GoodBye(RETURN_FAIL);
    }
    if (freeStore < 1UL) {
        PutStr("Fewer than 10 bytes free!\n");
        GoodBye(RETURN_FAIL);
    }
    itemcount = freeStore/bytes;

    return(itemcount);    
}


/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {

    if (myData) {
        FreeVec(myData);
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
    rdargs = ReadArgs(TEMPLATE, (LONG *)&opts, NULL);
    if (!rdargs) {
        PrintFault(IoErr(), PROGNAME);
        return(FALSE);
    }

    NVBase = OpenLibrary("nonvolatile.library", 37L);
    if (!NVBase) {
        Printf(ERRMSG_LIBNOOPEN, "nonvolatile.library", 37L);
        return(FALSE);
    }

    return(TRUE);
}

