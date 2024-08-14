/* DeleteNV.c
 * 
 * test nonvolatile.library/DeleteNV()
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

#include "nv.h"         /* default app/item names, etc. */

/* Defines ------------------------------------------ */
#define PROGNAME "DeleteNV"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "NAME=APPNAME/K,ITEM=ITEMNAME/K,KILLREQ/S"


/* Structs ------------------------------ */
struct Opts {
    STRPTR name;
    STRPTR item;
    LONG   killReq;
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);

/* Globals --------------------------------------- */
struct Opts     opts;
struct RDArgs  *rdargs;
struct Library *NVBase;


VOID main(int argc, UBYTE *argv[]) {
    STRPTR app, item;
    BOOL success, killReq;
    LONG ioErr;
    int rc = RETURN_OK;

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    app  = ((opts.name) ? opts.name : (STRPTR)DEFAULT_APP_NAME);
    item = ((opts.item) ? opts.item : (STRPTR)DEFAULT_ITEM_NAME);
    killReq = ((opts.killReq) ? TRUE : FALSE);

    Printf("Calling DeleteNV(%s, %s)...\n", app, item);

    success = DeleteNV(app, item, killReq);
    ioErr = IoErr();
    if (!success) {
        PutStr("DeleteNV returned failure.\n");
        PrintFault(ioErr, PROGNAME);
        rc = RETURN_ERROR;
    }
    else {
        Printf("DeleteNV returned success.  IOErr was %ld  \n", ioErr);
        PrintFault(ioErr, NULL);
    }

    GoodBye(RETURN_OK);
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

