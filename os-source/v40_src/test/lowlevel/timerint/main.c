/****** CDGS/TimerInt **********************************************************
*
*   NAME
*       TimerInt -- test program for lowlevel.library/AddTimerInt(),
*                lowlevel.library/RemTimerInt(), lowlevel.library/StartTimerInt(),
*                and lowlevel.library/StopTimerInt().
*
*   SYNOPSIS
*       TimerInt
*
*   FUNCTION
*
*       Test program for lowlevel.library/AddTimerInt(),
*       lowlevel.library/RemTimerInt(), lowlevel.library/StartTimerInt(),
*       and lowlevel.library/StopTimerInt().
*
*       To terminate this program, select the close gadget.
*
*   INPUTS
*       None
*
*   RESULT
*       RETURN_OK (0)       -   success
*       RETURN_WARN (5)     -   warning
*       RETURN_ERROR (10)   -   error
*       RETURN_FAIL (20)    -   failure
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*       If timer interrupt deviates from timing with timer.device by one second
*       or more, an incorrect deviation will be reported because the deviation
*       is measured in microseconds only.
*
*       Hopefully very few.
*
*   SEE ALSO
*
******************************************************************************
*
*/

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

/*
 * Local includes
 */

#define MAIN

#include "timerint.h"

/****** TimerInt/main ******************************************
*
*   NAME
*       main -- main entry point
*
*   SYNOPSIS
*       main(argc,argv);
*
*       void main(int argc,char *argv[]);
*
*   FUNCTION
*       Main entry point.
*
*   INPUTS
*       argc -- argument count
*       argv -- argument value arrraay
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*       If there are any, you can get to 'em from here!
*
*   SEE ALSO
*       shutdown()
*
******************************************************************************
*
*/
void main(int argc,char *argv[])
{

    BYTE error;

    /*
     * Open libraries
     */

    /* Open intuition.library */
    IntuitionBase=OpenLibrary("intuition.library",KICKSTART_VERSION);
    if (!IntuitionBase) {
        Printf("%s: Error opening intuition.library V%ld\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    /* Open gadtool.library */
    GadtoolsBase=OpenLibrary("gadtools.library",KICKSTART_VERSION);
    if (!GadtoolsBase) {
        Printf("%s: Error opening gadtools.library V%ld\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    /* Open lowlevel.library */
    LowLevelBase=OpenLibrary("lowlevel.library",KICKSTART_VERSION);
    if (!LowLevelBase) {
        Printf("%s: Error opening lowlevel.library V%d\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    /*
     * Open timer.device
     */

    /* Create timer.device reply port */
    timerPort=CreateMsgPort();
    if (!timerPort) {
        Printf("%s: Error creating timer.device reply port\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Create timer.device I/O request */
    timerRequest=CreateIORequest(timerPort,sizeof(struct timerequest));
    if (!timerRequest) {
        Printf("%s: Error creating timer.device I/O request\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Open timer.device microhertz timer */
    error=OpenDevice("timer.device",UNIT_MICROHZ,timerRequest,NULL);
    if (error) {
        Printf("%s: Error %lu opening timer.device\n",PROGRAM_NAME,error);
        goodbye(RETURN_FAIL);
    }

    /* Fetch timer.device library base; timer.device I/O request is not
       actually used for timer functions */
    TimerBase=(struct Library *) timerRequest->tr_node.io_Device;

    /*
     * Set-up timer interrupt handler
     */

    /* Allocate timer interrupt signal bit */
    timerSignal=AllocSignal(-1);
    if (timerSignal==-1) {
        Printf("%s: Error allocating signal bit for timer interrupt\n",
            PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Find main task */
    mainTask=FindTask(NULL);
    if (!mainTask) {
        Printf("%s: Error finding main task\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Add timer interrupt handler */
    timerIntHandle=AddTimerInt(timerInterrupt,(APTR) TIMERINT_COOKIE);
    if (!timerIntHandle) {
        Printf("%s: Error adding timer interrupt handler\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /*
     * GUI
     */

    /* Open GUI */
    if (!guiOpen()) {
        Printf("%s: Error opening user interface\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Process GUI */
    guiLoop();

    /*
     * Termination
     */

    /* If any bad data to keyboard interrupt handler ... */
    if (timerIntBadData) {
        /* Warning */
        Printf("%s: %lu calls to timer interrupt handler with bad data\n",
            PROGRAM_NAME,timerIntBadData);
    }

    /* Exit */
    goodbye(timerIntBadData?RETURN_WARN:RETURN_OK);

}

/****** TimerInt/goodbye ******************************************
*
*   NAME
*       goodbye -- terminate program
*
*   SYNOPSIS
*       goodbye(returnCode);
*
*       void goodbye(int returnCode);
*
*   FUNCTION
*       Terminate program.
*
*   INPUTS
*       returnCode -- return code (from dos/dos.h RETURN_*)
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
*       main()
*
******************************************************************************
*
*/
void goodbye(int returnCode)
{

    /*
     * Close GUI
     */

    /* Close GUI */
    guiClose();

    /*
     * Remove timer interrupt handler
     */

    /* Remove timer interrupt handler */
    if (timerIntHandle) {
        RemTimerInt(timerIntHandle);
    }

    /*
     * Close timer.device
     */

    /* Close timer.device */
    if (TimerBase) {
        CloseDevice(timerRequest);
    }

    /* Delete timer.device I/O request */
    if (timerRequest) {
        DeleteIORequest(timerRequest);
    }

    /* Delete timer.device message port */
    if (timerPort) {
        DeleteMsgPort(timerPort);
    }

    /*
     * Close libraries
     */

    /* Close lowlevel.library */
    if (LowLevelBase) {
        CloseLibrary(LowLevelBase);
    }

    /* Close gadtools.library */
    if (GadtoolsBase) {
        CloseLibrary(GadtoolsBase);
    }

    /* Close intuition.library */
    if (IntuitionBase) {
        CloseLibrary(IntuitionBase);
    }

    /*
     * Exit
     */

    /* Exit */
    exit(returnCode);

}
