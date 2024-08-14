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
#define TEMPLATE "TICKS/A/N,TEST/A/N"


struct testGroup {
    ULONG start_hi;
    ULONG start_lo;
    ULONG stop_hi;
    ULONG stop_lo;
    ULONG diff_hi;
    ULONG diff_lo;
};





#define NUM_TESTS 5
struct testGroup tests[] = {
    { 0x00000040, 0x00000002, 0x00000042, 0x00000001, 0x00000001, 0xFFFFFFFF },
    { 0x00000040, 0xFFFFFFFF, 0x00000041, 0x00000000, 0x00000000, 0x00000001 },
    { 0x00000000, 0x03333333, 0x00000002, 0x00000020, 0x00000001, 0xFCCCCCED },
    { 0x00000000, 0x000FFF0F, 0x00000001, 0x000A1801, 0x00000000, 0xFFFA18F2 },
    { 0x00000000, 0x000FFF0F, 0x00000001, 0x000A1802, 0x00000000, 0xFFFA18F3 },
/*     start.HI     start.LO     stop.HI     stop.LO    diff.hI     diff.LO       */
    NULL,
};

/* 
    So SubTime must do

    Test 0
             0x00000042 00000001
          -  0x00000040 00000002
             ===================
             0x00000001 FFFFFFFF

    Test 1

             0x00000041 00000000
          -  0x00000040 FFFFFFFF
             ===================
             0x00000000 00000001

    Test 2
             0x00000002 00000020
          -  0x00000000 03333333
             ===================
             0x00000001 FCCCCCED

    Test 3
             0x00000001 000A1801
          -  0x00000000 000FFF0F
             ===================
             0x00000000 FFFA18F2

    Test 4
             0x00000001 000A1802
          -  0x00000000 000FFF0F
             ===================
             0x00000000 FFFA18F3

    Does IT??
*/

/* Structs ------------------------------ */
struct Opts {
    LONG *delaytics;
    LONG *testNum;
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
extern STRPTR timeCmp(struct EClockVal *, struct EClockVal *, ULONG);
extern BOOL UDiv6432(ULONG,ULONG,ULONG,ULONG *,ULONG *);

/* Globals --------------------------------------- */
struct Opts     opts;
struct RDArgs  *rdargs;
struct Library *TimerBase;
struct timerequest timeReq, *tr;

VOID main(int argc, UBYTE *argv[]) {
    struct EClockVal start,stop;
    ULONG ticks, quot=0, rem=0;
    STRPTR timeStr;
    struct testGroup *tg;

    tr = &timeReq;


    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    if ((*opts.testNum < 0L) || (*opts.testNum >= NUM_TESTS)) {
        Printf("Bad Test Number!  Valid test numbers are 0 to %ld\n", (LONG)NUM_TESTS-1L);
        GoodBye(RETURN_FAIL);
    }

    tg = &tests[*opts.testNum];

    PutStr("This does a BOGUS test, trying to find a bug in timpCmp()...\n");
    Printf("Start time is %lu %lu, and stop time is %lu %lu.\n", tg->start_hi, tg->start_lo,
            tg->stop_hi, tg->stop_lo);


    ticks = ReadEClock(&start);
    start.ev_hi = tg->start_hi;
    start.ev_lo = tg->start_lo;
    Delay(*opts.delaytics);
    ReadEClock(&stop);
    stop.ev_hi = tg->stop_hi;
    stop.ev_lo = tg->stop_lo;

    if (timeStr = timeCmp(&stop, &start, ticks)) {
        Printf("Elapsed time was %s seconds.\n", timeStr);
        FreeVec(timeStr);
    }
    else {
        PutStr("timeCmp failed!\n");
    }

    if (UDiv6432(tg->diff_hi, tg->diff_lo, ticks, &quot, &rem)) {
        Printf("The real answer is %lu and %lu/%lu seconds\n", quot, rem, ticks);
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

