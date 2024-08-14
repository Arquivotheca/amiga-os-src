/* TimeCmp.c - return string showing the amount of time (seconds.fractions)
   between two EClockVals passed in, based on the number of EClockTics/Sec
   also passed in.

   Does NOT change the EClockVals passed in.

    REQUIREMENTS:
        Open SysBase [WORKS ONLY on 020s or better!]
        Open TimerBase

 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <devices/timer.h>
#include <clib/exec_protos.h>
#include <clib/timer_protos.h>

#include <stdlib.h>
#include <string.h>

/* DB */
#include <clib/dos_protos.h>

#include <pragmas/timer_pragmas.h>

#define NINE_DIGIT_BIG_NUMBER 1000000000UL   /* 1,000,000,000UL, actually */

extern struct ExecBase *SysBase;
extern struct Library  *TimerBase;

STRPTR timeCmp(struct EClockVal *, struct EClockVal *, ULONG);
extern BOOL  UDiv6432(ULONG, ULONG, ULONG, ULONG *, ULONG *);  /* asm, set up for C */
extern ULONG giveFract(ULONG, ULONG, ULONG);                   /* asm, set up for C */
extern VOID  bsprintf(STRPTR,STRPTR,...);                      /* asm, errrpt.lib   */
extern VOID  subETime(struct EClockVal *, struct EClockVal *); /* asm */
extern VOID  SubETime(struct EClockVal *, struct EClockVal *); /* C */


/****** jwl.lib/timeCmp ******************************************
*
*   NAME
*       timeCmp -- compares two EClockVals, returning string with difference.
*
*   SYNOPSIS
*       timeCmp(eclock_stop, eclock1_start, eclockTics)
*
*       STRPTR timeCmp(struct EClockVal *, struct EClockVal *, ULONG);
*
*   FUNCTION
*       Return string showing the amount of time which elapsed between
*       the two EClockVals, formatted as Seconds.FractionalSecs.
*
*   INPUTS
*       eclock_stop  - ptr to last struct you did a ReadEClock() on.
*       eclock_start - ptr to first struct you did a ReadEClock() on.
*       eclockTics   - number of EClock ticks per second, also returned
*                      by timer.device/ReadEClock().
*
*   RESULT
*       STRPTR if all went well, NULL otherwise.  You MUST call
*              exec.library/FreeVec() on this pointer when you are
*              done with it!
*              
*
*   EXAMPLE
*
*   NOTES
*       This function works ONLY on 68020's or better, but may be
*       called on any CPU (returns failure if SysBase->AttnFlags
*       does not have AFF_68020 set).
*
*       Also returns NULL if you try to call it on a pre-V37 system.
*
*       Requires open TimerBase (timerequest.tr_node.io_Device)
*       and SysBase (ExecBase).
*
*       Neither eclock_stop nor eclock_start is modified in any way.
*
*       Passing in any NULLs will result in a NULL return.
*
*   BUGS
*
*   SEE ALSO
*       timer.device/CmpTime(),    timer.device/ReadEClock(),
*       exec.library/FreeVec(),    jwl.lib/UDiv6432(), jwl.lib/giveFract(),
*       exec.library/OpenDevice(), ErrRpt.lib/bsprintf(),
*       exec.library/RawDoFmt()
*
******************************************************************************
*
*/

STRPTR timeCmp(struct EClockVal *stopOrig, struct EClockVal *start, ULONG tps) {
    struct EClockVal stop;
    STRPTR result=NULL;
    ULONG quot=0, rem=0, fracSecs;

    /* Need 2.04 or better AmigaDOS! */
    if (SysBase->LibNode.lib_Version < 37L) {
        return(NULL);
    }

    /* Need 68020 or better CPU! */
    if (!(SysBase->AttnFlags & AFF_68020)) {
        return(NULL);
    }

    /* need open Timer... */
    if (!TimerBase) {
        return(NULL);
    }

    /* idiot-proofing */
    if (!stopOrig || !start || !tps) {
        return(NULL);
    }

    stop.ev_hi = stopOrig->ev_hi;
    stop.ev_lo = stopOrig->ev_lo;

/*    SubTime((struct timeval *)&stop, (struct timeval *)start); */
    subETime(&stop, start);   /* subETime is asm, SubETime is C. */


/* DB   Printf("Math:  $%08lx %08lx\n   -   $%08lx %08lx\n       ==================\n       $%08lx %08lx\n",
                  stopOrig->ev_hi,
                  stopOrig->ev_lo,
                  start->ev_hi,
                  start->ev_lo,
                  stop.ev_hi,
                  stop.ev_lo);
*/

    if (UDiv6432(stop.ev_hi, stop.ev_lo, tps, &quot, &rem)) {
/* DB    Printf("hi=%lu, lo=%lu, tps=%lu, quot=%lu, rem=%lu\n",
                stop.ev_hi, stop.ev_lo, tps, quot, rem);
*/

        /* this very large number (>9 digits) will give us a 9-digit number back */
        fracSecs = giveFract(rem, tps, NINE_DIGIT_BIG_NUMBER);

/* DB    Printf("fracSecs = %lu\n", fracSecs);
*/



        /* We now want to allocate a buffer to hold the following
           type of beast:
                "%lu.%04lu"
                ^a  ^b ^c ^d
                a = 10 digits max
                b =  1 decimal point
                c =  9 digits, perhaps 10
                d =  1 terminating NULL
                ==========================
                    21 bytes total.  Make it 24, since we'll get that much anyhow.
        */

        if (result = AllocVec(24L, MEMF_CLEAR)) {
            bsprintf(result, "%lu.%09lu", quot, fracSecs);
            /* and we're done... */
        }
    }

    return(result);
}

