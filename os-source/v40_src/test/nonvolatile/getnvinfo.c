/* GetNVInfo.c
 * 
 * test nonvolatile.library/GetNVInfo()
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
#define PROGNAME "GetNVInfo"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "KILLREQ/S"

struct Opts {
    LONG killReq;
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);

/* Globals --------------------------------------- */
struct Library *NVBase;
struct NVInfo *nvi;
struct RDArgs *rdargs;
struct Opts opts;

VOID main(int argc, UBYTE *argv[]) {
    BOOL killReq;

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    killReq = ((opts.killReq) ? TRUE : FALSE);

    if (nvi = GetNVInfo(killReq)) {
        Printf("Got NVInfo at $%08lx:\n", nvi);
        Printf("  nvi_MaxStorage  = %lu\n", nvi->nvi_MaxStorage);
        Printf("  nvi_FreeStorage = %lu\n", nvi->nvi_FreeStorage);
    }
    else {
        PrintFault(IoErr(), PROGNAME);
        PutStr("GetNVInfo() failed!\n");
    }

    GoodBye(RETURN_OK);
}

/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {

    if (nvi) {
        PutStr("Calling FreeNVData(nvi)...\n");
        FreeNVData(nvi);
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
        PrintFault(IoErr(),PROGNAME);
        return(FALSE);
    }

    NVBase = OpenLibrary("nonvolatile.library", 40L);
    if (!NVBase) {
        Printf(ERRMSG_LIBNOOPEN, "nonvolatile.library", 40L);
        return(FALSE);
    }

    return(TRUE);
}

