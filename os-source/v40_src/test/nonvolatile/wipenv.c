/* WipeNV.c
 * 
 * Delete everything in NV-RAM.
 * 
 * 
 */

/* Includes --------------------------------------------- */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <libraries/nonvolatile.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/nonvolatile_protos.h>
#include <stdlib.h>
#include <string.h>

#include <pragmas/nonvolatile_pragmas.h>

/* Defines ------------------------------------------ */
#define PROGNAME "WipeNV"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define APPNAMESIZE 80

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);

/* Globals --------------------------------------- */
struct Library *NVBase;
int rc;

VOID main(int argc, UBYTE *argv[]) {
    struct MinList *ml;
    struct MinNode *mn;
    struct NVEntry *nve;
    UBYTE appName[APPNAMESIZE];

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    rc = RETURN_OK;

    if (ml = GetNVList(NULL,FALSE)) {
        mn = ml->mlh_Head;
        while (mn) {
            if (mn->mln_Succ) {
                nve = (struct NVEntry *)mn;
                if (!(nve->nve_Size)) {
                    /* tis name to be copied for use in deletion */
                    stccpy(appName, nve->nve_Name, APPNAMESIZE);
                }
                else {
                    if (nve->nve_Protection) {
                        SetNVProtection(appName, nve->nve_Name, 0, FALSE);
                    }
                    Printf("Deleting %s %s ...", appName, nve->nve_Name);
                    if (DeleteNV(appName, nve->nve_Name, FALSE)) {
                        PutStr("done.\n");
                    }
                    else {
                        PutStr("error!\n");
                        rc = RETURN_ERROR;
                    }
                }
            }
            mn = mn->mln_Succ;
            if (CheckSignal(SIGBREAKF_CTRL_C)) {
                FreeNVData(ml);
                GoodBye(rc);
            }
        }
        FreeNVData(ml);
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

    exit(rc);
}

/* doInit =============================================
 * Open libraries, call ReadArgs() if necessary.
 * Returns TRUE for success, FALSE otherwise.
 */
static LONG doInit(VOID) {
    NVBase = OpenLibrary("nonvolatile.library", 39L);
    if (!NVBase) {
        Printf(ERRMSG_LIBNOOPEN, "nonvolatile.library", 39L);
        return(FALSE);
    }

    return(TRUE);
}

