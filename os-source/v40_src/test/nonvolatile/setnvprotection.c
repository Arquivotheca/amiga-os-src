/* SetNVProtection.c
 * 
 * 
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
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/nonvolatile_protos.h>
#include <stdlib.h>
#include <string.h>

#include <pragmas/nonvolatile_pragmas.h>
#include "nv.h"

/* Defines ------------------------------------------ */
#define PROGNAME "SetNVProtection"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "NAME=APPNAME/K,ITEM=ITEMNAME/K,FLAGS=HEXFLAGS/K,DELETE/S,NN=NULLNAME/S,NI=NULLITEM/S,KILLREQ/S"


/* Structs ------------------------------ */
struct Opts {
    STRPTR name;
    STRPTR item;
    STRPTR flags;
    LONG   delete;
    LONG   nullName;
    LONG   nullItem;
    LONG   killReq;
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
extern BOOL xStrToULong(STRPTR, ULONG *);

/* Globals --------------------------------------- */
struct Opts     opts;
struct RDArgs  *rdargs;
struct Library *NVBase;


VOID main(int argc, UBYTE *argv[]) {
    STRPTR name, item;
    LONG flags=0UL;
    BOOL snvErr, killReq;
    ULONG hexNo=0;
    int rc = RETURN_OK;

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    name = ((opts.name) ? opts.name : (STRPTR)DEFAULT_APP_NAME);
    item = ((opts.item) ? opts.item : (STRPTR)DEFAULT_ITEM_NAME);
    killReq = ((opts.killReq) ? TRUE : FALSE);

    if (opts.nullName) {
        name = NULL;
    }
    if (opts.nullItem) {
        item = NULL;
    }
    if (opts.delete) {
        flags = NVEF_DELETE;
    }

    if (opts.flags) {
        if (!xStrToULong(opts.flags, &hexNo)) {
            Printf("Bad hex number: '%s'!\n", opts.flags);
            GoodBye(RETURN_FAIL);
        }
        flags |= hexNo;
    }

    Printf("Calling SetNVProtection('%s', '%s', $%08lx)...\n",
            name, item, flags);

    snvErr = SetNVProtection(name, item, flags, killReq);
    if (snvErr == FALSE) {
        PrintFault(IoErr(), PROGNAME);
        Printf("SetNVProtection() returned failure, %ld!\n", (LONG)snvErr);
        rc = RETURN_ERROR;
    }
    else {
        PutStr("SetNVProtection() returned success.\n");
    }

    GoodBye(rc);
}

/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {

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

    NVBase = OpenLibrary("nonvolatile.library", 40L);
    if (!NVBase) {
        Printf(ERRMSG_LIBNOOPEN, "nonvolatile.library", 40L);
        return(FALSE);
    }

    return(TRUE);
}

