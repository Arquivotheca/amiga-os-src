/* TCTest.c
 * 
 * Test TimeCmp.c & associated asm functions.
 * 
 * 
 */

/* Includes --------------------------------------------- */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <devices/timer.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/timer_protos.h>
#include <stdlib.h>
#include <string.h>


/* Defines ------------------------------------------ */
#define PROGNAME "TCTest"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "TICKS/A/N"


/* Structs ------------------------------ */
struct Opts {
    LONG *delaysecs;
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
extern STRPTR timeCmp(struct EClockVal *, struct EClockVal *, ULONG);

/* Globals --------------------------------------- */
struct Opts     opts;
struct RDArgs  *rdargs;
struct Library *TimerBase;
struct timerequest timeReq, *tr;

VOID main(int argc, UBYTE *argv[]) {
    struct EClockVal start,stop;
    ULONG ticks;
    STRPTR timeStr;

    tr = &timeReq;

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    ticks = ReadEClock(&start);
    Delay(*opts.delaysecs);
    ReadEClock(&stop);

    if (timeStr = timeCmp(&stop,&start, ticks)) {
        Printf("Elapsed time was %s seconds.\n", timeStr);
        FreeVec(timeStr);
    }
    else {
        PutStr("timeCmp failed!\n");
    }

    GoodBye(RETURN_OK);
}

/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {
    if (rdargs) {
        FreeArgs(rdargs);
    }

    if (TimerBase) {
        CloseDevice((struct IORequest *)tr);
        TimerBase = NULL;
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

    if (OpenDevice(TIMERNAME,UNIT_ECLOCK,(struct IORequest*)tr,0)) {
        return(FALSE);
    }
    TimerBase = (struct Library *)(tr->tr_node.io_Device);

    return(TRUE);
}

