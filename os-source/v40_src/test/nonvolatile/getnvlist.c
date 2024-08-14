/* GetNVList.c
 * 
 * Test nonvolatile.library/GetNVList()
 * 
 * 
 */

/* Includes --------------------------------------------- */
#include <exec/types.h>
#include <exec/lists.h>
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

#include "nv.h"       /* default names, etc. */

/* Defines ------------------------------------------ */
#define PROGNAME "GetNVList"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "NAME=APPNAME/K,NULL=NULLTEST/S,KILLREQ/S,NOBREAK/S"

/* #define DEBUG 1 */

/* Structs ------------------------------ */
struct Opts {
    STRPTR name;
    LONG nullTest;
    LONG killReq;
    LONG noBreak;
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
static VOID dumpNVE(struct NVEntry *);
#ifdef DEBUG
extern VOID KPrintF(STRPTR,...);
static VOID kDumpNVE(struct NVEntry *);
#endif

/* Globals --------------------------------------- */
struct Opts          opts;
struct RDArgs       *rdargs;
struct Library      *NVBase;
struct MinList      *ml;
UBYTE  blankStr[] = "";

VOID main(int argc, UBYTE *argv[]) {
    STRPTR name, prName;
    struct NVEntry *nve;
    int rc = RETURN_OK;
    BOOL killReq;

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    name = ((opts.name) ? opts.name : (STRPTR)DEFAULT_APP_NAME);
    if (opts.nullTest) {
        name = NULL;
    }

    prName = ((name) ? name : blankStr);  /* originally for debug, needed by strcmp below */

    killReq = ((opts.killReq) ? TRUE : FALSE);

    Printf("Calling GetNVList('%s', %ld)...\n", prName, (LONG)killReq);
#ifdef DEBUG
    KPrintF("Calling GetNVList('%s', %ld)...\n", prName, (LONG)killReq);
#endif
    
    if (ml = GetNVList(name, killReq)) {
#ifdef DEBUG
        KPrintF("GetNVList returned $%08lx\n", ml);
#endif
        Printf("Got NVList at $%08lx\n", ml);
        nve = (struct NVEntry *)(ml->mlh_Head);
        if (!nve) {
            PutStr("GetNVList() returned a Headless List!!\n");
            rc = RETURN_ERROR;
        }
        while (nve) {
            if (CheckSignal(SIGBREAKF_CTRL_C)) {
                GoodBye(rc);
            }

#ifdef DEBUG
            KPrintF("size: %ld, nvename %08lx, name %08lx, break %08lx\n",
                     (LONG)(nve->nve_Size),
                     nve->nve_Name,
                     name,
                     opts.noBreak);
#endif
            if (!TypeOfMem(nve->nve_Name)) {
                if (nve->nve_Node.mln_Succ) {
                    Printf("Bogus name pointer detected: $%08lx\n", nve->nve_Name);
#ifdef DEBUG
                    KPrintF("Bogus name pointer detected: $%08lx\n", nve->nve_Name);
#endif
                }
                else {
                    Printf("End of list detected, name: $%08lx\n", nve->nve_Name);
#ifdef DEBUG
                    KPrintF("End of list detected, name: $%08lx\n", nve->nve_Name);
#endif
                }
            }

            /*  (size==0 && nve_name==prName) && (nobreak unspecified) */
            if (((nve->nve_Size == 0UL) && (!strcmp(nve->nve_Name,prName))) && (!opts.noBreak)) {
                break;
            }
            
            /* and the last entry in the list tends to be bogus... */
            if ((NULL == nve->nve_Node.mln_Succ) && (!opts.noBreak)) {
                break;
            }

            /* print out contents of the nve we found */
#ifdef DEBUG
            kDumpNVE(nve);
#endif
            dumpNVE(nve);

            nve = (struct NVEntry *)(nve->nve_Node.mln_Succ);
        }
        PutStr("Calling FreeNVData()...\n");
        FreeNVData(ml);
        ml = NULL;
    }
    else {
        Printf("GetNVList('%s', %ld) returned NULL!\n", prName, killReq);
    }


    GoodBye(rc);
}

/* dumpNVE ==============================================
   print out the contents of a struct NVEntry
 */
static VOID dumpNVE(struct NVEntry *nv) {

    if (!nv) {
        PutStr("dumpNVE() received a null pointer for printing!\n");
        return;
    }
    if (!TypeOfMem(nv)) {
#ifdef DEBUG
        KPrintF("dumpNVE() received a bogus pointer for printing!\n");
#endif
        PutStr("dumpNVE() received a bogus pointer for printing!\n");
    }

    Printf("NVEntry at $%08lx:  Succ: $%08lx  Pred: $%08lx\n", 
            nv,
            nv->nve_Node.mln_Succ,
            nv->nve_Node.mln_Pred);
    if ((nv->nve_Node.mln_Succ) && (TypeOfMem(nv->nve_Node.mln_Succ))) {
        Printf("  Name: '%s' ", nv->nve_Name);
    }
    else {
        Printf("  Name: $%08lx ", nv->nve_Name);
    }
    Printf("  Size:  %4lu Prot:  $%lx\n\n", nv->nve_Size, nv->nve_Protection);

    return;
}
#ifdef DEBUG
/* kDumpNVE ==================================================================
   kprintf version of dumpnve 
 */
static VOID kDumpNVE(struct NVEntry *nv) {

    if (!nv) {
        KPrintF("dumpNVE() received a null pointer for printing!\n");
        return;
    }
    if (!TypeOfMem(nv)) {
        KPrintF("dumpNVE() received a bogus pointer for printing!\n");
    }
    KPrintF("NVEntry at $%08lx:  Succ: $%08lx\n     Pred: $%08lx\n", 
             nv,
            nv->nve_Node.mln_Succ,
            nv->nve_Node.mln_Pred);
    if ((nv->nve_Node.mln_Succ) && (TypeOfMem(nv->nve_Node.mln_Succ))) {
        KPrintF("  Name: '%s' ", nv->nve_Name);
    }
    else {
        KPrintF("  Name: $%08lx ", nv->nve_Name);
    }
    KPrintF("  Size:  %4lu  Prot:  $%lx\n\n", 
               nv->nve_Size, nv->nve_Protection);

    return;
}
#endif

/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {

    if ((ml) && (TypeOfMem(ml))) {
        Printf("Calling FreeNVData(ml=$%08lx)...\n", ml);
        FreeNVData(ml);
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

