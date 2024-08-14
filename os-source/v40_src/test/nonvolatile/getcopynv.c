/* GetCopyNV.c
 * 
 * 
 * test nonvolatile.library/GetCopyNV()
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
#define PROGNAME "GetCopyNV"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "LENGTH/A/N,NAME=APPNAME/K,ITEM=ITEMNAME/K,NN=NULLNAME/S,NI=NULLITEM/S,HEX=HEXDUMP/S,KILLREQ/S"


/* Structs ------------------------------ */
struct Opts {
    ULONG   *length;        /* length of data.  Use GetNVList test prog to get this */
    STRPTR  name;           /* app name */
    STRPTR  item;           /* item name */
    LONG    nullName;       /* test with null app name */
    LONG    nullItem;       /* test with null item name */
    LONG    hexDump;        /* do hex dump instead of ascii dump */
    LONG    killReq;        /* eliminate requesters */
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
static VOID dumpHex(STRPTR,ULONG);
static VOID dumpData(STRPTR,ULONG);

/* Globals --------------------------------------- */
struct Opts     opts;
struct RDArgs  *rdargs;
struct Library *NVBase;
STRPTR          data;   /* returned data from GetCopyNV() */

VOID main(int argc, UBYTE *argv[]) {
    STRPTR name, item;        /* appname, itemname */
    int rc = RETURN_OK;
    BOOL killReq;

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

    Printf("Calling GetCopyNV('%s', '%s')\n", name, item);
    if (data = GetCopyNV(name, item, killReq)) {
        if (opts.hexDump) {
            dumpHex(data, *opts.length);
        }
        else {
            dumpData(data, *opts.length);
        }
    }
    else {
        PrintFault(IoErr(), NULL);
        PutStr("GetCopyNV() returned NULL!\n");
        rc = RETURN_WARN;
    }
    

    GoodBye(rc);
}

/* dumpHex ==============================================
    do hex dump of data, 1 char at a time...
    should really do it a ulong at a time, but it doesn't matter much.
*/
static VOID dumpHex(STRPTR s, ULONG slen) {
    ULONG i;

    if (!s || !slen)
        return;

    for (i = 0; i < slen; i++) {
        Printf("%02lx", (ULONG)(s[i]));
        if (!(i+1 % 8)) {
            PutStr(" ");
        }
        if (!(i+1 % 32)) {
            PutStr("\n");
        }
        if (CheckSignal(SIGBREAKF_CTRL_C)) {
            return;
        }
    }
    PutStr("\n");

    return;
}


/* dumpData ==============================================
    do dump of data (consider it non-null-terminated ascii)
*/
static VOID dumpData(STRPTR s, ULONG len) {

    if (!s || !len)
        return;

    PutStr("\"");
    Flush(Output());

    Write(Output(), s, (LONG)len);

    Flush(Output());
    PutStr("\"\n");

    return;
}

/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {

    if (data) {
        PutStr("Calling FreeNVData(data)...\n");
        FreeNVData(data);
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

    NVBase = OpenLibrary("nonvolatile.library", 40L);
    if (!NVBase) {
        Printf(ERRMSG_LIBNOOPEN, "nonvolatile.library", 40L);
        return(FALSE);
    }

    return(TRUE);
}

