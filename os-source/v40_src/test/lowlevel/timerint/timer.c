/*
 * System includes
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <libraries/gadtools.h>

#include <devices/timer.h>

#include <libraries/lowlevel.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/timer_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/alib_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/lowlevel_pragmas.h>

/*
 * ANSI includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * Local includes
 */

#include "timerint.h"


/****** timerInt/timerStart ******************************************
*
*   NAME
*       timerStart -- start timer interrupt
*
*   SYNOPSIS
*       timerStart();
*
*       void timerStart(void);
*
*   FUNCTION
*       Start timer interrupt.
*
*   INPUTS
*       None
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       timerStop()
*
******************************************************************************
*
*/
void timerStart(void)
{

    /* Clear deviation */
    deviation=0L;
    GT_SetGadgetAttrs(deviationGadget,window,NULL,
        GTNM_Number, deviation,
        TAG_DONE);

    /* Get starting system time */
    GetSysTime(&startTime);

    /* Start timer interrupt */
    StartTimerInt(timerIntHandle,timerInterval,timerContinuous);

}


/****** timerInt/timerStop ******************************************
*
*   NAME
*       timerStop -- stop timer interrupt
*
*   SYNOPSIS
*       timerStop();
*
*       void timerStop(void);
*
*   FUNCTION
*       Stop timer interrupt.
*
*   INPUTS
*       None
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       timerStart()
*
******************************************************************************
*
*/
void timerStop(void)
{

    /* Stop timer interrupt */
    StopTimerInt(timerIntHandle);

}

/****** timerInt/timerInterrupt ******************************************
*
*   NAME
*       timerInterrupt -- timer interrupt handler
*
*   SYNOPSIS
*       timerInterrupt(intData);
*
*       void __asm __interrupt __saveds timerInterrupt(register __a1 APTR intData);
*
*   FUNCTION
*       Timer interrupt handler.
*
*   INPUTS
*       intData -- interrupt data
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

void __asm __interrupt __saveds timerInterrupt(register __a1 APTR intData)
{

    static struct timeval elapsedTime, oldStartTime;
    LONG thisDeviation;

    /* Test interrupt data */
    if (intData!=(APTR) TIMERINT_COOKIE) {
        timerIntBadData++;
    }

    /* Get ending system time */
    GetSysTime(&stopTime);

    /* Store start time */
    oldStartTime=startTime;

    /* If in continuous mode ... */
    if (timerContinuous) {
        /* Get starting system time for next interval */
        GetSysTime(&startTime);
    }

    /* Compute elapsed time */
    elapsedTime=stopTime;
    SubTime(&elapsedTime,&oldStartTime);

    /* Compute deviation */
    thisDeviation=timerInterval-elapsedTime.tv_micro;

    /* If deviation greater than previous maximum ... */
    if (abs(thisDeviation)>abs(deviation)) {
        /* Update deviation */
        deviation=thisDeviation;
    }

    /* Signal main task */
    Signal(mainTask,1<<timerSignal);

}
